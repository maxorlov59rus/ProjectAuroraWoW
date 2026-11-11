#include "AuroraAuthLibrary.h"

#include "Auth/PacketCodec.h"
#include "Auth/Srp6.h"

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

namespace
{
	FSocket* OpenConnection(const FString& Host, int32 Port)
	{
		ISocketSubsystem* SocketSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		if (!SocketSub)
		{
			return nullptr;
		}

		FIPv4Address Address;
		if (!FIPv4Address::Parse(Host, Address))
		{
			return nullptr;
		}

		FSocket* Socket = SocketSub->CreateSocket(NAME_Stream, TEXT("AuroraAuth"), false);
		if (!Socket)
		{
			return nullptr;
		}

		Socket->SetNonBlocking(false);
		Socket->SetNoDelay(true);

		TSharedRef<FInternetAddr> Addr = SocketSub->CreateInternetAddr();
		Addr->SetIp(Address.Value);
		Addr->SetPort(Port);

		if (!Socket->Connect(*Addr))
		{
			Socket->Close();
			SocketSub->DestroySocket(Socket);
			return nullptr;
		}

		return Socket;
	}

	void CloseConnection(FSocket* Socket)
	{
		if (Socket)
		{
			Socket->Close();
			if (ISocketSubsystem* SocketSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
			{
				SocketSub->DestroySocket(Socket);
			}
		}
	}

	bool ReadExact(FSocket* Socket, uint8* Buffer, int32 Length)
	{
		int32 Total = 0;
		while (Total < Length)
		{
			int32 Read = 0;
			if (!Socket->Recv(Buffer + Total, Length - Total, Read) || Read <= 0)
			{
				return false;
			}
			Total += Read;
		}
		return true;
	}

	bool ReadExactArray(FSocket* Socket, int32 Length, TArray<uint8>& Out)
	{
		Out.SetNumUninitialized(Length);
		return Length == 0 || ReadExact(Socket, Out.GetData(), Length);
	}

	bool WriteExact(FSocket* Socket, const TArray<uint8>& Data)
	{
		int32 Sent = 0;
		return Socket->Send(Data.GetData(), Data.Num(), Sent) && Sent == Data.Num();
	}

	EAuthResult MapError(uint8 Code)
	{
		switch (Code)
		{
			case Aurora::Auth::Wire::RESULT_FAIL_PASSWORD:
			case Aurora::Auth::Wire::RESULT_FAIL_UNKNOWN:
				return EAuthResult::WrongCredentials;
			case Aurora::Auth::Wire::RESULT_FAIL_VERSION:
				return EAuthResult::ProtocolError;
			default:
				return EAuthResult::Blocked;
		}
	}
}

bool UAuthLibrary::PingAuthServer(const FString& Host, int32 Port)
{
	FSocket* Socket = OpenConnection(Host, Port);
	if (!Socket)
	{
		return false;
	}
	CloseConnection(Socket);
	return true;
}

EAuthResult UAuthLibrary::Login(
	const FString& Host,
	int32 Port,
	const FString& Username,
	const FString& Password,
	FAuthSession& OutSession)
{
	OutSession.Username.Reset();
	OutSession.SessionKey.Reset();

	const FString Upper = Username.ToUpper();
	if (Upper.Len() < 3 || Upper.Len() > 16)
	{
		return EAuthResult::ProtocolError;
	}

	FSocket* Socket = OpenConnection(Host, Port);
	if (!Socket)
	{
		return EAuthResult::ServerUnreachable;
	}

	// ---- 1. AUTH_LOGON_CHALLENGE -----------------------------------------
	{
		const TArray<uint8> Packet = Aurora::Auth::Wire::BuildLogonChallenge(Upper);
		if (!WriteExact(Socket, Packet))
		{
			CloseConnection(Socket);
			return EAuthResult::ServerUnreachable;
		}
	}

	// ---- 2. Чтение ответа ------------------------------------------------
	uint8 Header[2] = {0, 0};
	if (!ReadExact(Socket, Header, 2))
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	if (Header[0] != Aurora::Auth::Wire::CMD_LOGON_CHALLENGE)
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	if (Header[1] != Aurora::Auth::Wire::RESULT_SUCCESS)
	{
		CloseConnection(Socket);
		return MapError(Header[1]);
	}

	TArray<uint8> B;
	if (!ReadExactArray(Socket, 32, B))
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	uint8 GLen = 0;
	if (!ReadExact(Socket, &GLen, 1))
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}
	TArray<uint8> G;
	if (!ReadExactArray(Socket, GLen, G))
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	uint8 NLen = 0;
	if (!ReadExact(Socket, &NLen, 1))
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}
	TArray<uint8> N;
	if (!ReadExactArray(Socket, NLen, N))
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	TArray<uint8> Salt;
	if (!ReadExactArray(Socket, 32, Salt))
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	// CRC16 (16 байт) — сервер присылает его для сверки с версией клиента.
	TArray<uint8> Crc;
	if (!ReadExactArray(Socket, 16, Crc))
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	// ---- 3. SRP6 ---------------------------------------------------------
	TArray<uint8> UserPassHash;
	Aurora::Srp6::ComputeUserPasswordHash(Upper, Password, UserPassHash);

	TArray<uint8> A;
	TArray<uint8> PrivateA;
	Aurora::Srp6::GeneratePublicA(A, PrivateA);

	TArray<uint8> X;
	Aurora::Srp6::ComputeX(Salt, UserPassHash, X);

	TArray<uint8> U;
	Aurora::Srp6::ComputeU(A, B, U);

	TArray<uint8> S;
	Aurora::Srp6::ComputeS(B, PrivateA, U, X, S);

	TArray<uint8> UsernameBytes;
	{
		FTCHARToUTF8 Utf8(*Upper);
		const uint8* Raw = reinterpret_cast<const uint8*>(Utf8.Get());
		UsernameBytes.Append(Raw, Utf8.Length());
	}

	TArray<uint8> M1;
	Aurora::Srp6::ComputeM1(UsernameBytes, Salt, A, B, S, M1);

	TArray<uint8> SessionKey;
	Aurora::Srp6::ComputeSessionKey(S, SessionKey);

	// ---- 4. AUTH_LOGON_PROOF ---------------------------------------------
	{
		const TArray<uint8> Packet = Aurora::Auth::Wire::BuildLogonProof(A, M1, Crc);
		if (!WriteExact(Socket, Packet))
		{
			CloseConnection(Socket);
			return EAuthResult::ServerUnreachable;
		}
	}

	// ---- 5. Ответ на proof ------------------------------------------------
	// cmd(1) + error(1) + M2(20) + accountFlags(4) + surveyId(4) + loginFlags(2)
	uint8 ProofHeader[2] = {0, 0};
	if (!ReadExact(Socket, ProofHeader, 2))
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	if (ProofHeader[0] != Aurora::Auth::Wire::CMD_LOGON_PROOF)
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	if (ProofHeader[1] != Aurora::Auth::Wire::RESULT_SUCCESS)
	{
		CloseConnection(Socket);
		return MapError(ProofHeader[1]);
	}

	TArray<uint8> M2;
	if (!ReadExactArray(Socket, 20, M2))
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	// Сервер подтверждает подлинность: M2 должен совпасть с нашим расчётом.
	TArray<uint8> ExpectedM2;
	Aurora::Srp6::ComputeM2(A, M1, S, ExpectedM2);
	if (M2.Num() != ExpectedM2.Num())
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	bool bM2Ok = true;
	for (int32 i = 0; i < M2.Num(); ++i)
	{
		if (M2[i] != ExpectedM2[i])
		{
			bM2Ok = false;
			break;
		}
	}
	if (!bM2Ok)
	{
		CloseConnection(Socket);
		return EAuthResult::ProtocolError;
	}

	// Остаток пакета не нужен для входа в мир.
	TArray<uint8> Tail;
	ReadExactArray(Socket, 10, Tail);

	CloseConnection(Socket);

	OutSession.Username = Upper;
	OutSession.SessionKey = SessionKey;
	return EAuthResult::Success;
}

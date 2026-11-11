#include "Net/WorldConnection.h"

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

#include <chrono>
#include <random>

namespace Aurora::World
{
	namespace
	{
		constexpr uint32 SMSG_AUTH_CHALLENGE = 0x1EC;
		constexpr uint32 CMSG_AUTH_SESSION    = 0x1ED;
		constexpr uint32 SMSG_AUTH_RESPONSE   = 0x1EE;
		constexpr uint32 AUTH_OK              = 0x0C;
		constexpr uint32 AUTH_FAILED          = 0x0D;
		constexpr uint32 AUTH_WAIT_QUEUE      = 0x1B;
		constexpr int32  AUTH_TIMEOUT_MS      = 5000;

		bool IsAuthOk(uint32 Code)
		{
			// 0x0C — AUTH_OK, 0x0D..0x12 — разные «ок» с доп. данными.
			return Code == AUTH_OK;
		}

		FSocket* AsSocket(void* Handle)
		{
			return static_cast<FSocket*>(Handle);
		}
	}

	FWorldConnection::FWorldConnection() = default;

	FWorldConnection::~FWorldConnection()
	{
		Disconnect();
	}

	void FWorldConnection::Disconnect()
	{
		if (FSocket* Socket = AsSocket(SocketHandle))
		{
			Socket->Close();
			if (ISocketSubsystem* SocketSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
			{
				SocketSub->DestroySocket(Socket);
			}
		}
		SocketHandle = nullptr;
	}

	bool FWorldConnection::EnsureBytes(int32 Required, FString& OutError)
	{
		FSocket* Socket = AsSocket(SocketHandle);
		if (!Socket)
		{
			OutError = TEXT("Socket is closed");
			return false;
		}

		const auto Deadline = std::chrono::steady_clock::now()
			+ std::chrono::milliseconds(AUTH_TIMEOUT_MS);

		while (InBuffer.Num() < Required)
		{
			if (std::chrono::steady_clock::now() > Deadline)
			{
				OutError = TEXT("Timed out waiting for server data");
				return false;
			}

			uint8 Chunk[4096];
			int32 Read = 0;

			if (!Socket->Recv(Chunk, sizeof(Chunk), Read, true))
			{
				if (Socket->GetConnectionState() == SCS_Connected)
				{
					FPlatformProcess::Sleep(0.001f);
					continue;
				}
				OutError = TEXT("Connection lost while reading");
				return false;
			}

			if (Read <= 0)
			{
				FPlatformProcess::Sleep(0.001f);
				continue;
			}

			InBuffer.Append(Chunk, Read);
		}

		return true;
	}

	bool FWorldConnection::SendPacket(uint32 Opcode, const TArray<uint8>& Body)
	{
		FSocket* Socket = AsSocket(SocketHandle);
		if (!Socket)
		{
			return false;
		}

		const TArray<uint8> Encoded = Stream.EncodeClient(Opcode, Body);

		int32 Sent = 0;
		return Socket->Send(Encoded.GetData(), Encoded.Num(), Sent) && Sent == Encoded.Num();
	}

	bool FWorldConnection::ReceivePacket(FServerPacket& OutPacket, FString& OutError)
	{
		const auto Deadline = std::chrono::steady_clock::now()
			+ std::chrono::milliseconds(AUTH_TIMEOUT_MS);

		while (true)
		{
			int32 Offset = 0;
			int32 NeedMore = 0;

			if (Stream.TryDecodeServer(InBuffer, Offset, OutPacket, NeedMore))
			{
				InBuffer.RemoveAt(0, Offset, EAllowShrinking::No);
				return true;
			}

			if (std::chrono::steady_clock::now() > Deadline)
			{
				OutError = TEXT("Timed out waiting for a server packet");
				return false;
			}

			const int32 Want = FMath::Max(NeedMore, InBuffer.Num() + 1);
			if (!EnsureBytes(Want, OutError))
			{
				return false;
			}
		}
	}

	EWorldResult FWorldConnection::ConnectAndAuthenticate(
		const FString& Host,
		int32 Port,
		const FString& Username,
		const TArray<uint8>& SessionKey,
		uint32 Build,
		FString& OutError)
	{
		OutError.Reset();

		if (SessionKey.Num() != 40)
		{
			OutError = TEXT("Session key must be 40 bytes");
			return EWorldResult::ProtocolError;
		}

		ISocketSubsystem* SocketSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		if (!SocketSub)
		{
			OutError = TEXT("No socket subsystem");
			return EWorldResult::ConnectFailed;
		}

		FIPv4Address Address;
		if (!FIPv4Address::Parse(Host, Address))
		{
			OutError = TEXT("Invalid host address");
			return EWorldResult::ConnectFailed;
		}

		FSocket* Socket = SocketSub->CreateSocket(NAME_Stream, TEXT("AuroraWorld"), false);
		if (!Socket)
		{
			OutError = TEXT("Unable to create socket");
			return EWorldResult::ConnectFailed;
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
			OutError = TEXT("Unable to connect to WorldServer");
			return EWorldResult::ConnectFailed;
		}

		SocketHandle = Socket;
		InBuffer.Reset();
		Stream = FWorldStream();

		// ---- 1. SMSG_AUTH_CHALLENGE -------------------------------------
		FServerPacket Challenge;
		if (!ReceivePacket(Challenge, OutError))
		{
			Disconnect();
			return EWorldResult::TimedOut;
		}

		if (Challenge.Opcode != SMSG_AUTH_CHALLENGE || Challenge.Body.Num() < 4)
		{
			OutError = FString::Printf(
				TEXT("Expected SMSG_AUTH_CHALLENGE (0x1EC), got 0x%X"), Challenge.Opcode);
			Disconnect();
			return EWorldResult::ProtocolError;
		}

		uint32 ServerSeed = 0;
		FMemory::Memcpy(&ServerSeed, Challenge.Body.GetData(), 4);

		// ---- 2. Ключи ---------------------------------------------------
		const FSessionKeys Keys = DeriveKeys(SessionKey);

		// ---- 3. CMSG_AUTH_SESSION ---------------------------------------
		uint32 ClientSeed = 0;
		{
			std::mt19937 Rng(static_cast<uint32>(
				std::chrono::high_resolution_clock::now().time_since_epoch().count()));
			ClientSeed = Rng();
		}

		// digest = SHA1(USER | 0 | ClientSeed | ServerSeed | SessionKey)
		TArray<uint8> DigestInput;
		{
			FTCHARToUTF8 UserUtf8(*Username.ToUpper());
			const uint8* Raw = reinterpret_cast<const uint8*>(UserUtf8.Get());
			DigestInput.Append(Raw, UserUtf8.Length());
		}
		DigestInput.Add(0x00);
		{
			uint8 SeedBytes[8];
			FMemory::Memcpy(SeedBytes, &ClientSeed, 4);
			FMemory::Memcpy(SeedBytes + 4, &ServerSeed, 4);
			DigestInput.Append(SeedBytes, 8);
		}
		DigestInput.Append(SessionKey.GetData(), SessionKey.Num());

		uint8 Digest[20] = {0};
		FSHA1::HashBuffer(DigestInput.GetData(), static_cast<uint64>(DigestInput.Num()), Digest);

		TArray<uint8> Body;
		Body.SetNumZeroed(4);
		FMemory::Memcpy(Body.GetData(), &Build, 4);          // build
		Body.Add(0);                                         // loginServerType
		Body.Add(0x00); Body.Add(0x00); Body.Add(0x00); Body.Add(0x00); // loginServerId (0)
		{
			uint8 SeedBytes[4];
			FMemory::Memcpy(SeedBytes, &ClientSeed, 4);
			Body.Append(SeedBytes, 4);                       // clientSeed
		}
		Body.Add(0x00); Body.Add(0x00); Body.Add(0x00); Body.Add(0x00); // regionId
		Body.Add(0x00); Body.Add(0x00); Body.Add(0x00); Body.Add(0x00); // battlegroupId
		Body.Add(0x00); Body.Add(0x00); Body.Add(0x00); Body.Add(0x00); // realmId
		Body.Add(0x01); Body.Add(0x1E); Body.Add(0x19); Body.Add(0x57); // 0x5719191E — magic
		Body.Add(0x00);                                      // digestLength
		Body.Append(Digest, 20);
		Body.Add(0x00);                                      // addonInfo: 0
		Body.Append(SessionKey.GetData(), SessionKey.Num() < 40 ? SessionKey.Num() : 40);

		if (!SendPacket(CMSG_AUTH_SESSION, Body))
		{
			Disconnect();
			OutError = TEXT("Unable to send CMSG_AUTH_SESSION");
			return EWorldResult::ConnectFailed;
		}

		// ---- 4. SMSG_AUTH_RESPONSE ---------------------------------------
		FServerPacket Response;
		if (!ReceivePacket(Response, OutError))
		{
			Disconnect();
			return EWorldResult::TimedOut;
		}

		// Ответ может прийти в любом порядке с другими пакетами (например,
		// SMSG_ADDON_INFO). Ищем именно SMSG_AUTH_RESPONSE.
		int32 Guard = 0;
		while (Response.Opcode != SMSG_AUTH_RESPONSE && Guard++ < 16)
		{
			if (!ReceivePacket(Response, OutError))
			{
				Disconnect();
				return EWorldResult::TimedOut;
			}
		}

		if (Response.Opcode != SMSG_AUTH_RESPONSE)
		{
			OutError = TEXT("SMSG_AUTH_RESPONSE not received");
			Disconnect();
			return EWorldResult::ProtocolError;
		}

		// Ответ зашифрован: включаем крипто до разбора тела.
		Stream.EnableEncryption(Keys);

		if (Response.Body.Num() < 1)
		{
			OutError = TEXT("Empty SMSG_AUTH_RESPONSE");
			Disconnect();
			return EWorldResult::ProtocolError;
		}

		const uint8 Result = Response.Body[0];
		if (!IsAuthOk(Result))
		{
			OutError = FString::Printf(TEXT("Auth rejected, code 0x%X"), Result);
			if (Result == AUTH_FAILED || Result == AUTH_WAIT_QUEUE)
			{
				Disconnect();
				return EWorldResult::Rejected;
			}
			Disconnect();
			return EWorldResult::ProtocolError;
		}

		return EWorldResult::Success;
	}
}

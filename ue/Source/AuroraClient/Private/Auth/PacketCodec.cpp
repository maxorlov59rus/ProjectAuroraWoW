#include "Auth/PacketCodec.h"

namespace Aurora::Auth::Wire
{
	void WritePascalString(TArray<uint8>& Out, const FString& Text)
	{
		FTCHARToUTF8 Utf8(*Text);
		const int32 Len = FMath::Min(Utf8.Length(), 255);
		Out.Add(static_cast<uint8>(Len));
		const uint8* Bytes = reinterpret_cast<const uint8*>(Utf8.Get());
		for (int32 i = 0; i < Len; ++i)
		{
			Out.Add(Bytes[i]);
		}
	}

	bool ReadPascalString(const TArray<uint8>& Data, int32& Offset, FString& OutText)
	{
		uint8 Len = 0;
		if (!ReadUInt8(Data, Offset, Len))
		{
			return false;
		}
		if (Offset + Len > Data.Num())
		{
			return false;
		}

		OutText = FString(ANSI_TO_TCHAR(reinterpret_cast<const ANSICHAR*>(Data.GetData() + Offset))).Left(Len);
		Offset += Len;
		return true;
	}

	bool ReadBytes(const TArray<uint8>& Data, int32& Offset, int32 Count, TArray<uint8>& OutBytes)
	{
		if (Count < 0 || Offset + Count > Data.Num())
		{
			return false;
		}
		OutBytes.Reset();
		OutBytes.Append(Data.GetData() + Offset, Count);
		Offset += Count;
		return true;
	}

	bool ReadUInt8(const TArray<uint8>& Data, int32& Offset, uint8& OutValue)
	{
		if (Offset >= Data.Num())
		{
			return false;
		}
		OutValue = Data[Offset++];
		return true;
	}

	TArray<uint8> BuildLogonChallenge(const FString& Username)
	{
		TArray<uint8> Packet;
		Packet.Add(CMD_LOGON_CHALLENGE);
		Packet.Add(RESULT_SUCCESS);

		const int32 SizePos = Packet.Num();
		Packet.Add(0x00);
		Packet.Add(0x00);

		// game name "WoW\0"
		WritePascalString(Packet, TEXT("WoW"));

		// version 3.3.5 (major, minor, patch)
		Packet.Add(0x03);
		Packet.Add(0x03);
		Packet.Add(0x05);

		// build 12340 = 0x3034, little-endian
		Packet.Add(0x34);
		Packet.Add(0x30);

		WritePascalString(Packet, TEXT("68x"));   // platform x86
		WritePascalString(Packet, TEXT("niW"));   // os "Win" в обратном порядке, как в протоколе
		WritePascalString(Packet, TEXT("SUne"));  // locale "enUS" в обратном порядке

		// timezone bias (minutes west of UTC), little-endian int32
		Packet.Add(0x00);
		Packet.Add(0x00);
		Packet.Add(0x00);
		Packet.Add(0x00);

		// client IP: 127.0.0.1 в порядке байт big-endian
		Packet.Add(127);
		Packet.Add(0x00);
		Packet.Add(0x00);
		Packet.Add(0x01);

		const FString Upper = Username.ToUpper();
		WritePascalString(Packet, Upper);

		const uint16 Body = static_cast<uint16>(Packet.Num() - SizePos - 2);
		Packet[SizePos] = static_cast<uint8>(Body & 0xFF);
		Packet[SizePos + 1] = static_cast<uint8>(Body >> 8);

		return Packet;
	}

	TArray<uint8> BuildLogonProof(const TArray<uint8>& A, const TArray<uint8>& M1, const TArray<uint8>& Crc)
	{
		TArray<uint8> Packet;
		Packet.Add(CMD_LOGON_PROOF);
		Packet.Append(A.GetData(), FMath::Min(A.Num(), 32));
		Packet.Append(M1.GetData(), FMath::Min(M1.Num(), 20));
		Packet.Append(Crc.GetData(), FMath::Min(Crc.Num(), 20));
		Packet.Add(0x00); // numberOfKeys = 0
		Packet.Add(0x00); // security flags
		return Packet;
	}

	TArray<uint8> FromHex(const FString& Hex)
	{
		auto Nibble = [](TCHAR C) -> int32
		{
			if (C >= '0' && C <= '9') { return C - '0'; }
			if (C >= 'A' && C <= 'F') { return C - 'A' + 10; }
			if (C >= 'a' && C <= 'f') { return C - 'a' + 10; }
			return -1;
		};

		TArray<uint8> Out;
		for (int32 i = 0; i + 1 < Hex.Len(); i += 2)
		{
			const int32 Hi = Nibble(Hex[i]);
			const int32 Lo = Nibble(Hex[i + 1]);
			if (Hi < 0 || Lo < 0)
			{
				break;
			}
			Out.Add(static_cast<uint8>((Hi << 4) | Lo));
		}
		return Out;
	}

	FString ToHex(const TArray<uint8>& Bytes)
	{
		static const TCHAR Digits[] = TEXT("0123456789abcdef");
		FString Out;
		Out.Reserve(Bytes.Num() * 2);
		for (const uint8 Byte : Bytes)
		{
			Out.AppendChar(Digits[Byte >> 4]);
			Out.AppendChar(Digits[Byte & 0x0F]);
		}
		return Out;
	}
}

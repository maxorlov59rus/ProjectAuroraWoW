#include "Net/WorldStream.h"

namespace Aurora::World
{
	FWorldStream::FWorldStream()
	{
		HeaderDecrypt.Reserve(LARGE_SERVER_HEADER);
	}

	int32 FWorldStream::GetExpectedHeaderSize() const
	{
		return bEncrypted ? LARGE_SERVER_HEADER : SERVER_HEADER_SIZE;
	}

	uint32 FWorldStream::ReadUInt24(const uint8* Data)
	{
		return static_cast<uint32>(Data[0])
			| (static_cast<uint32>(Data[1]) << 8)
			| (static_cast<uint32>(Data[2]) << 16);
	}

	void FWorldStream::EnableEncryption(const FSessionKeys& Keys)
	{
		EncryptKey = Keys.EncryptKey;
		DecryptKey = Keys.DecryptKey;

		CryptOut.SetKey(EncryptKey);
		CryptIn.SetKey(DecryptKey);
		CryptIn.Drop(1024);

		ClientPacketCounter = 0;
		bEncrypted = true;
	}

	TArray<uint8> FWorldStream::EncodeClient(uint32 Opcode, const TArray<uint8>& Body)
	{
		const int32 BodySize = Body.Num();
		const bool bLarge = BodySize > 0x7FFF;

		TArray<uint8> Header;
		if (bLarge)
		{
			Header.SetNumZeroed(LARGE_CLIENT_HEADER);
			Header[0] = static_cast<uint8>((BodySize + LARGE_CLIENT_HEADER) & 0xFF);
			Header[1] = static_cast<uint8>(((BodySize + LARGE_CLIENT_HEADER) >> 8) & 0xFF);
			Header[2] = static_cast<uint8>(((BodySize + LARGE_CLIENT_HEADER) >> 16) & 0xFF);
			Header[3] = static_cast<uint8>(Opcode & 0xFF);
			Header[4] = static_cast<uint8>((Opcode >> 8) & 0xFF);
			Header[5] = static_cast<uint8>((Opcode >> 16) & 0xFF);
			Header[6] = static_cast<uint8>((Opcode >> 24) & 0xFF);
		}
		else
		{
			Header.SetNumZeroed(CLIENT_HEADER_SIZE);
			const uint16 Size = static_cast<uint16>(BodySize + CLIENT_HEADER_SIZE);
			Header[0] = static_cast<uint8>(Size & 0xFF);
			Header[1] = static_cast<uint8>((Size >> 8) & 0xFF);
			Header[2] = static_cast<uint8>(Opcode & 0xFF);
			Header[3] = static_cast<uint8>((Opcode >> 8) & 0xFF);
			Header[4] = static_cast<uint8>((Opcode >> 16) & 0xFF);
			Header[5] = static_cast<uint8>((Opcode >> 24) & 0xFF);
		}

		TArray<uint8> Out = Header;
		Out.Append(Body);

		if (bEncrypted)
		{
			// Шифруем всё, кроме 2 (или 3) байт размера.
			const int32 PlainLen = bLarge ? 3 : 1;
			const int32 Tail = Out.Num() - PlainLen;

			// ARC4 хранит состояние: «съедаем» открытую часть, чтобы ключевой
			// поток совпадал с тем, что ожидает сервер.
			uint8 Skip[3] = {Out[0], Out[1], bLarge ? Out[2] : 0};
			CryptOut.Process(Skip, bLarge ? 3 : 1);

			if (Tail > 0)
			{
				CryptOut.Process(Out.GetData() + PlainLen, Tail);
			}

			// HMAC-SHA1 от (заголовок | тело | счётчик пакета), 20 байт.
			uint8 Counter[4];
			Counter[0] = static_cast<uint8>(ClientPacketCounter & 0xFF);
			Counter[1] = static_cast<uint8>((ClientPacketCounter >> 8) & 0xFF);
			Counter[2] = static_cast<uint8>((ClientPacketCounter >> 16) & 0xFF);
			Counter[3] = static_cast<uint8>((ClientPacketCounter >> 24) & 0xFF);
			++ClientPacketCounter;

			TArray<uint8> HmacInput = Out;
			HmacInput.Append(Counter, 4);

			uint8 Digest[20];
			HmacSha1(
				EncryptKey.GetData(), EncryptKey.Num(),
				HmacInput.GetData(), HmacInput.Num(),
				Digest);

			Out.Append(Digest, 20);
		}

		return Out;
	}

	bool FWorldStream::TryDecodeServer(const TArray<uint8>& Buffer, int32& InOutOffset, FServerPacket& OutPacket, int32& OutNeedMore)
	{
		OutNeedMore = 0;

		const int32 Available = Buffer.Num() - InOutOffset;
		if (Available < 2)
		{
			OutNeedMore = 2;
			return false;
		}

		const uint8* Cursor = Buffer.GetData() + InOutOffset;

		int32 BodySize = 0;
		int32 HeaderLen = 0;

		if (!bEncrypted)
		{
			const int32 Total = static_cast<int32>(Cursor[0]) | (static_cast<int32>(Cursor[1]) << 8);
			HeaderLen = SERVER_HEADER_SIZE;
			BodySize = Total - HeaderLen;

			if (Available < HeaderLen)
			{
				OutNeedMore = HeaderLen;
				return false;
			}
			if (Available < Total)
			{
				OutNeedMore = Total;
				return false;
			}

			OutPacket.Opcode = static_cast<uint16>(Cursor[2]) | (static_cast<uint16>(Cursor[3]) << 8);
		}
		else
		{
			// Большой заголовок: 3 байта размера открыты, остальное зашифровано.
			const bool bLargeCandidate =
				((static_cast<uint32>(Cursor[0]) | (static_cast<uint32>(Cursor[1]) << 8) |
					(static_cast<uint32>(Cursor[2]) << 16)) & 0x800000u) == 0;

			HeaderLen = bLargeCandidate ? LARGE_SERVER_HEADER : SERVER_HEADER_SIZE;

			if (Available < HeaderLen + 20)
			{
				OutNeedMore = HeaderLen + 20;
				return false;
			}

			// Расшифровка заголовка: только «хвост» зашифрован, size — нет.
			HeaderDecrypt.Reset();
			HeaderDecrypt.Append(Cursor, HeaderLen);

			const int32 PlainLen = bLargeCandidate ? 3 : 1;
			uint8 Skip[3] = {Cursor[0], Cursor[1], bLargeCandidate ? Cursor[2] : 0};
			CryptIn.Process(Skip, PlainLen);

			if (HeaderLen - PlainLen > 0)
			{
				CryptIn.Process(HeaderDecrypt.GetData() + PlainLen, HeaderLen - PlainLen);
			}


			if (bLargeCandidate)
			{
				const uint32 Total = ReadUInt24(HeaderDecrypt.GetData());
				BodySize = static_cast<int32>(Total) - LARGE_SERVER_HEADER;
				OutPacket.Opcode = static_cast<uint16>(HeaderDecrypt[3]) |
					(static_cast<uint16>(HeaderDecrypt[4]) << 8);
				HeaderLen = LARGE_SERVER_HEADER;

				if (Available < static_cast<int32>(Total) + 20)
				{
					OutNeedMore = static_cast<int32>(Total) + 20;
					return false;
				}
			}
			else
			{
				const int32 Total = static_cast<int32>(HeaderDecrypt[0]) |
					(static_cast<int32>(HeaderDecrypt[1]) << 8);
				BodySize = Total - SERVER_HEADER_SIZE;
				OutPacket.Opcode = static_cast<uint16>(HeaderDecrypt[2]) |
					(static_cast<uint16>(HeaderDecrypt[3]) << 8);
				HeaderLen = SERVER_HEADER_SIZE;

				if (Available < Total + 20)
				{
					OutNeedMore = Total + 20;
					return false;
				}
			}
		}

		if (BodySize < 0)
		{
			OutNeedMore = 0;
			return false;
		}

		const int32 TotalPacket = HeaderLen + BodySize;

		InOutOffset += HeaderLen;
		if (BodySize > 0)
		{
			OutPacket.Body.SetNumUninitialized(BodySize);
			FMemory::Memcpy(OutPacket.Body.GetData(), Buffer.GetData() + InOutOffset, BodySize);
			InOutOffset += BodySize;
		}
		else
		{
			OutPacket.Body.Reset();
		}

		if (bEncrypted && OutPacket.Body.Num() > 0)
		{
			CryptIn.Process(OutPacket.Body);
		}

		if (bEncrypted)
		{
			// Пропускаем HMAC-подпись, приложенную к пакету.
			InOutOffset += 20;
		}

		(void)TotalPacket;
		return true;
	}
}

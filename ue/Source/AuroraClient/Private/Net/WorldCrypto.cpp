#include "Net/WorldCrypto.h"

#include "Misc/SecureHash.h"

namespace Aurora::World
{
	namespace
	{
		/** CRC32 (IEEE 802.3), бит за битом — таблицу держать смысла нет. */
		uint32 Crc32Bit(const uint8* Data, int32 Length)
		{
			uint32 Crc = 0xFFFFFFFFu;
			for (int32 i = 0; i < Length; ++i)
			{
				Crc ^= Data[i];
				for (int32 Bit = 0; Bit < 8; ++Bit)
				{
					if (Crc & 1u)
					{
						Crc = (Crc >> 1) ^ 0xEDB88320u;
					}
					else
					{
						Crc >>= 1;
					}
				}
			}
			return ~Crc;
		}

		TArray<uint8> HmacBytes(const TArray<uint8>& Key, const uint8* Data, int32 DataLen)
		{
			uint8 Digest[20];
			FSHA1::HMACBuffer(
				Key.GetData(), static_cast<uint32>(Key.Num()),
				Data, static_cast<uint64>(DataLen),
				Digest);

			TArray<uint8> Out;
			Out.Append(Digest, 20);
			return Out;
		}
	}

	void HmacSha1(
		const uint8* Key, int32 KeyLen,
		const uint8* Data, int32 DataLen,
		uint8* OutDigest)
	{
		FSHA1::HMACBuffer(
			Key, static_cast<uint32>(KeyLen),
			Data, static_cast<uint64>(DataLen),
			OutDigest);
	}

	uint32 ComputeCrc32(const uint8* Data, int32 Length)
	{
		return Crc32Bit(Data, Length);
	}

	namespace
	{
		/**
		 * Один блок из 20 байт в общей цепочке «key seed».
		 *
		 * AzerothCore разбивает session key на две половины по 20 байт и
		 * считает HMAC-SHA1, добавляя к lastContext исполненный блок:
		 * copyContext = lastContext ++ block (block = 0, 1, ...).
		 * Пять таких блоков дают ровно 60 байт ключевого материала.
		 */
		void DeriveBlock(const uint8* Seed, int32 SeedLen, uint8 Block, uint8* Out20)
		{
			const uint8 Tail = Block;
			uint8 Buffer[64];

			uint8 HmacKey[64];
			FMemory::Memzero(HmacKey, sizeof(HmacKey));

			// HMAC ключ = seed(40) | block(1) | нули, как это делает сервер.
			const int32 CopyLen = FMath::Min(SeedLen, 63);
			FMemory::Memcpy(HmacKey, Seed, CopyLen);
			HmacKey[CopyLen] = Tail;

			FSHA1::HMACBuffer(HmacKey, static_cast<uint32>(CopyLen + 1),
				reinterpret_cast<const uint8*>(""), 0, Out20);

			(void)Buffer;
		}
	}

	FSessionKeys DeriveKeys(const TArray<uint8>& SessionKey)
	{
		FSessionKeys Keys;

		if (SessionKey.Num() < SEED_LENGTH)
		{
			return Keys;
		}

		const uint8* Seed = SessionKey.GetData();
		const int32 SeedLen = SessionKey.Num();

		uint8 Digest[20];

		// Блок 0 -> encryptKey, блок 1 -> decryptKey (байты 20..39).
		DeriveBlock(Seed, SeedLen, 0, Digest);
		Keys.EncryptKey.Append(Digest, 20);

		DeriveBlock(Seed, SeedLen, 1, Digest);
		Keys.DecryptKey.Append(Digest, 20);

		DeriveBlock(Seed, SeedLen, 2, Digest);
		Keys.SeedKey.Append(Digest, 20);

		return Keys;
	}
}

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

	FSessionKeys DeriveKeys(const TArray<uint8>& SessionKey)
	{
		// 60 байт = три 20-байтовых ключа подряд.
		static const char* Trailing = "trailing";
		const int32 TrailingLen = 8;

		uint8 Digest[20];
		FSHA1::HMACBuffer(
			SessionKey.GetData(), static_cast<uint32>(SessionKey.Num()),
			reinterpret_cast<const uint8*>(Trailing), static_cast<uint64>(TrailingLen),
			Digest);

		FSessionKeys Keys;
		Keys.EncryptKey.Append(Digest, 20);

		// Второй блок: продолжение «trailing» с последним байтом ключа
		// сдвинутым на следующий счётчик, как это делает AzerothCore.
		uint8 Key2[20];
		FMemory::Memcpy(Key2, SessionKey.GetData(), 20);
		FMemory::Memcpy(Key2 + 20 - 1, SessionKey.GetData() + 19, 1);

		uint8 Digest2[20];
		FSHA1::HMACBuffer(
			SessionKey.GetData(), static_cast<uint32>(SessionKey.Num()),
			reinterpret_cast<const uint8*>(Trailing), static_cast<uint64>(TrailingLen),
			Digest2);

		Keys.DecryptKey.Append(Digest2, 20);

		uint8 Digest3[20];
		FSHA1::HMACBuffer(
			SessionKey.GetData(), static_cast<uint32>(SessionKey.Num()),
			reinterpret_cast<const uint8*>(Trailing), static_cast<uint64>(TrailingLen),
			Digest3);

		Keys.SeedKey.Append(Digest3, 20);
		(void)Key2;
		return Keys;
	}
}

#pragma once

#include "CoreMinimal.h"
#include "Net/Arc4.h"

/**
 * Криптография WorldServer 3.3.5a.
 *
 * Из 40-байтового session key (этап 2) выводятся три ключа HMAC-SHA1:
 *   encryptKey = HMAC(key, "trailing")[0..19]
 *   decryptKey = HMAC(key, "trailing")[20..39]
 *   authSeed   = HMAC(key, "trailing")[40..59]   (не нужен в AES-вариантах)
 *
 * Заголовки пакетов подписываются HMAC-SHA1 и шифруются ARC4; каждый
 * заголовок отодвигается на 2 байта назад по счётчику пакетов, и HMAC
 * снимает сервер уже с учётом этого сдвига.
 */
namespace Aurora::World
{
	constexpr int32 SERVER_HEADER_SIZE = 4;   // size(2) + opcode(2)
	constexpr int32 CLIENT_HEADER_SIZE = 6;   // size(2) + opcode(4)
	constexpr int32 LARGE_SERVER_HEADER = 5;  // size(3) + opcode(2)
	constexpr int32 LARGE_CLIENT_HEADER = 7;  // size(3) + opcode(4)
	constexpr int32 HMAC_KEY_LENGTH = 20;
	constexpr int32 SEED_LENGTH = 16;

	/** Три HMAC-ключа, выведенных из session key. */
	struct FSessionKeys
	{
		TArray<uint8> EncryptKey;
		TArray<uint8> DecryptKey;
		TArray<uint8> SeedKey;
	};

	FSessionKeys DeriveKeys(const TArray<uint8>& SessionKey);

	/** HMAC-SHA1. OutDigest получает 20 байт. */
	void HmacSha1(
		const uint8* Key, int32 KeyLen,
		const uint8* Data, int32 DataLen,
		uint8* OutDigest);

	/** CRC32 для CMSG_AUTH_SESSION. */
	uint32 ComputeCrc32(const uint8* Data, int32 Length);
}

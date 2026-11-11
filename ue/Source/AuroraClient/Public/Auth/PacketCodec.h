#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"

/**
 * Кодирование и разбор пакетов AuthServer 3.3.5a.
 *
 * Формат строки в протоколе — Pascal: 1 байт длины + ASCII-байты.
 * Всё little-endian.
 */
namespace Aurora::Auth::Wire
{
	constexpr uint8 CMD_LOGON_CHALLENGE = 0x00;
	constexpr uint8 CMD_LOGON_PROOF     = 0x01;
	constexpr uint8 CMD_RECONNECT_CHALLENGE = 0x02;
	constexpr uint8 CMD_RECONNECT_PROOF     = 0x03;

	constexpr uint8 RESULT_SUCCESS      = 0x00;
	constexpr uint8 RESULT_FAIL_UNKNOWN = 0x01;
	constexpr uint8 RESULT_FAIL_VERSION = 0x03;
	constexpr uint8 RESULT_FAIL_PASSWORD = 0x04;
	constexpr uint8 RESULT_FAIL_BANNED  = 0x03;

	/** Дописывает Pascal-строку (ASCII). */
	void WritePascalString(TArray<uint8>& Out, const FString& Text);

	/** Читает Pascal-строку, сдвигая Offset. */
	bool ReadPascalString(const TArray<uint8>& Data, int32& Offset, FString& OutText);

	/** Читает ровно N байт, сдвигая Offset. */
	bool ReadBytes(const TArray<uint8>& Data, int32& Offset, int32 Count, TArray<uint8>& OutBytes);

	/** Читает uint8, сдвигая Offset. */
	bool ReadUInt8(const TArray<uint8>& Data, int32& Offset, uint8& OutValue);

	/** Собирает 'AUTH_LOGON_CHALLENGE' с подстановкой размера. */
	TArray<uint8> BuildLogonChallenge(const FString& Username);

	/** Собирает 'AUTH_LOGON_PROOF'. */
	TArray<uint8> BuildLogonProof(const TArray<uint8>& A, const TArray<uint8>& M1, const TArray<uint8>& Crc);

	/** Преобразует hex-строку в массив байт. */
	TArray<uint8> FromHex(const FString& Hex);

	/** Из массива в hex (нижний регистр). */
	FString ToHex(const TArray<uint8>& Bytes);
}

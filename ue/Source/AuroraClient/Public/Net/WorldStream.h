#pragma once

#include "CoreMinimal.h"
#include "Net/Arc4.h"
#include "Net/WorldCrypto.h"

/**
 * Транспорт WorldServer 3.3.5a: разбор заголовков, шифрование ARC4,
 * HMAC-подпись.
 *
 * После SMSG_AUTH_CHALLENGE обе стороны инициализируют ARC4 из
 * decryptKey/encryptKey и начинают подписывать каждый пакет.
 */
namespace Aurora::World
{
	/** Один разобранный пакет сервера. */
	struct FServerPacket
	{
		uint16 Opcode = 0;
		TArray<uint8> Body;
	};

	class FWorldStream
	{
	public:
		FWorldStream();

		/** Включает шифрование после успешной авторизации сессии. */
		void EnableEncryption(const FSessionKeys& Keys);

		bool IsEncrypted() const { return bEncrypted; }

		/** Сериализует клиентский пакет (шифрование + HMAC, если включено). */
		TArray<uint8> EncodeClient(uint32 Opcode, const TArray<uint8>& Body);

		/**
		 * Разбирает один пакет сервера из потока.
		 *
		 * @return false, если данных недостаточно — дозапросите из сокета
		 */
		bool TryDecodeServer(const TArray<uint8>& Buffer, int32& InOutOffset, FServerPacket& OutPacket, int32& OutNeedMore);

		/** Сколько байт заголовка ещё нужно, чтобы понять размер пакета. */
		int32 GetExpectedHeaderSize() const;

	private:
		static uint32 ReadUInt24(const uint8* Data);

		bool bEncrypted = false;

		// Каждый heading шифруется ARC4, но его первые 2/3 байта (size)
		// передаются открытым текстом, поэтому ARC4 «проглатывает» их
		// сохранением состояния.
		FArc4 CryptIn;    // сервер -> клиент
		FArc4 CryptOut;   // клиент -> сервер

		TArray<uint8> HeaderDecrypt;
		TArray<uint8> EncryptKey;
		TArray<uint8> DecryptKey;

		uint32 ClientPacketCounter = 0;
	};
}

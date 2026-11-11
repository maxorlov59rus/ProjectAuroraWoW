#pragma once

#include "CoreMinimal.h"
#include "Net/WorldStream.h"

/**
 * Клиент WorldServer (порт 8085) для 3.3.5a.
 *
 * Этап 3: подключение, SMSG_AUTH_CHALLENGE, CMSG_AUTH_SESSION.
 * После успеха поток шифруется ARC4 и готов к charm/char-select (этап 4).
 *
 * Класс не потокобезопасен и блокирующий; вызывайте из рабочего потока.
 */
namespace Aurora::World
{
	enum class EWorldResult : uint8
	{
		Success,
		ConnectFailed,
		TimedOut,
		ProtocolError,
		Rejected,
	};

	class FWorldConnection
	{
	public:
		FWorldConnection();
		~FWorldConnection();

		FWorldConnection(const FWorldConnection&) = delete;
		FWorldConnection& operator=(const FWorldConnection&) = delete;

		/**
		 * Подключается и проходит AUTH_SESSION.
		 *
		 * @param Host        адрес WorldServer
		 * @param Port        8085 по умолчанию
		 * @param Username    логин (тот же, что в AuthServer)
		 * @param SessionKey  40-байтовый ключ из этапа 2
		 * @param Build       12340 для 3.3.5a
		 * @param OutError    текст ошибки для лога
		 */
		EWorldResult ConnectAndAuthenticate(
			const FString& Host,
			int32 Port,
			const FString& Username,
			const TArray<uint8>& SessionKey,
			uint32 Build,
			FString& OutError);

		/** Закрывает соединение. */
		void Disconnect();

		bool IsConnected() const { return SocketHandle != nullptr; }

		/** Шлёт готовый клиентский пакет (заголовок собирается здесь). */
		bool SendPacket(uint32 Opcode, const TArray<uint8>& Body);

		/** Читает один пакет сервера. */
		bool ReceivePacket(FServerPacket& OutPacket, FString& OutError);

	private:
		void* SocketHandle = nullptr;
		FWorldStream Stream;
		TArray<uint8> InBuffer;

		bool EnsureBytes(int32 Required, FString& OutError);
	};
}

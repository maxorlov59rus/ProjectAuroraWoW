#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuroraAuthLibrary.generated.h"

/** Результат попытки подключения к AuthServer. */
UENUM(BlueprintType)
enum class EAuthResult : uint8
{
	Success           UMETA(DisplayName = "Success"),
	WrongCredentials  UMETA(DisplayName = "Wrong credentials"),
	Banned            UMETA(DisplayName = "Banned"),
	ServerUnreachable UMETA(DisplayName = "Server unreachable"),
	ProtocolError     UMETA(DisplayName = "Protocol error"),
	Blocked           UMETA(DisplayName = "Blocked"),
};

/** Данные успешного входа, необходимые для подключения к WorldServer. */
USTRUCT(BlueprintType)
struct FAuthSession
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Aurora|Auth")
	FString Username;

	/** 40-байтовый ключ сессии: им подписываются пакеты WorldServer. */
	UPROPERTY(BlueprintReadOnly, Category = "Aurora|Auth")
	TArray<uint8> SessionKey;
};

/**
 * Полный вход на AuthServer (порт 3724) для 3.3.5a.
 *
 * Реализован цикл AUTH_LOGON_CHALLENGE -> AUTH_LOGON_PROOF вместе с SRP6.
 * На выходе — session key, с которого начинается этап 3 (WorldServer).
 *
 * Функция синхронная и блокирует игровой поток; вызывайте её из
 * concurrency-задачи или из тестового актёра в редакторе.
 */
UCLASS()
class AURORACLIENT_API UAuthLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Логинится на AuthServer.
	 *
	 * @param Host     адрес AuthServer
	 * @param Port     порт, обычно 3724
	 * @param Username логин (3–16 символов, латиница/цифры)
	 * @param Password пароль в открытом виде; на сервер не отправляется
	 * @param OutSession результат: ключ сессии для WorldServer
	 */
	UFUNCTION(BlueprintCallable, Category = "Aurora|Auth")
	static EAuthResult Login(
		const FString& Host,
		int32 Port,
		const FString& Username,
		const FString& Password,
		FAuthSession& OutSession);

	/** Проверяет, что AuthServer принимает TCP-соединение. */
	UFUNCTION(BlueprintCallable, Category = "Aurora|Auth")
	static bool PingAuthServer(const FString& Host, int32 Port);
};

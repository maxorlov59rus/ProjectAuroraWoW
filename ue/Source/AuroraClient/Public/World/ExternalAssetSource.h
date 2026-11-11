#pragma once

#include "CoreMinimal.h"
#include "World/WorldSource.h"

/**
 * Вариант 2: внешний источник ассетов.
 *
 * Читает геометрию и сущности из внешнего каталога (результат работы
 * конвертера данных карты). Ничего не выдумывает: нет файла — нет данных.
 *
 * ВАЖНО о правах. Загрузчик принимает только файлы, на которые у
 * пользователя есть права. Каждый файл сопровождается записью в
 * manifest.json с полем "license". Если поле отсутствует или равно
 * "unknown"/"blizzard", файл не загружается — код не помогает
 * распространять защищённый контент.
 *
 * Ожидаемая раскладка каталога:
 *   <RootDir>/manifest.json
 *   <RootDir>/terrain/<map>_<x>_<y>.json
 *   <RootDir>/entities/<map>.json
 */
namespace Aurora::World
{
	/** Допустимые значения поля license в manifest.json. */
	namespace ExternalAssetPolicy
	{
		/** Лицензии, которые загрузчик принимает. */
		inline bool IsAllowedLicense(const FString& License)
		{
			const FString Normalized = License.ToLower();

			return Normalized == TEXT("cc0")
				|| Normalized == TEXT("cc-by")
				|| Normalized == TEXT("cc-by-sa")
				|| Normalized == TEXT("proprietary-own")
				|| Normalized == TEXT("user-owned");
		}
	}

	class FExternalAssetSource final : public IWorldSource
	{
	public:
		explicit FExternalAssetSource(const FString& InRootDir)
			: RootDir(InRootDir) {}

		/** Читает manifest.json. Возвращает false, если манифест невалиден. */
		bool LoadManifest(FString& OutError);

		virtual bool GetTerrain(float CenterX, float CenterY, float Radius,
		                        TArray<FTerrainChunk>& OutChunks) override;

		virtual bool GetEntities(float CenterX, float CenterY, float Radius,
		                         TArray<FWorldEntity>& OutEntities) override;

		virtual FString GetName() const override { return TEXT("External"); }

		/** Доступные карты и их разрешённые чанки. */
		const TMap<FString, TArray<FString>>& GetChunks() const { return Chunks; }

	private:
		FString RootDir;

		/** map -> имена чанков, прошедших проверку лицензии. */
		TMap<FString, TArray<FString>> Chunks;

		/** map -> файл с сущностями, прошедший проверку. */
		TMap<FString, FString> EntityFiles;

		/** Читает JSON-файл и проверяет его лицензию по записи в манифесте. */
		bool TryReadLicensedFile(const FString& RelativePath, TSharedPtr<FJsonObject>& OutJson) const;
	};
}

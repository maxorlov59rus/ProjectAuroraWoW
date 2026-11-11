#pragma once

#include "CoreMinimal.h"
#include "World/WorldSource.h"

/**
 * Вариант 1: процедурный ландшафт.
 *
 * Высоты берутся из простой гладкой функции (не из данных игры), модели
 * заменяются плейсхолдерами. Юридически чисто: никакие ассеты Blizzard
 * не используются.
 *
 * Функция высот намеренно отделена от генерации сетки — её можно заменить
 * на чтение реальной карты высот, не меняя остальной код.
 */
namespace Aurora::World
{
	/** Параметры рельефа. */
	struct FProceduralTerrainParams
	{
		int32 Cells = 32;          // ячеек по стороне
		float CellSize = 25.f;     // размер ячейки в ярдах WoW
		float Amplitude = 40.f;    // амплитуда холмов
		float Frequency = 0.008f;  // частота шума
	};

	class FProceduralTerrainSource final : public IWorldSource
	{
	public:
		FProceduralTerrainSource() = default;
		explicit FProceduralTerrainSource(const FProceduralTerrainParams& InParams)
			: Params(InParams) {}

		virtual bool GetTerrain(float CenterX, float CenterY, float Radius,
		                        TArray<FTerrainChunk>& OutChunks) override;

		virtual bool GetEntities(float CenterX, float CenterY, float Radius,
		                         TArray<FWorldEntity>& OutEntities) override;

		virtual FString GetName() const override { return TEXT("Procedural"); }

		/** Высота в точке мира. Публична, чтобы её можно было переиспользовать. */
		float SampleHeight(float X, float Y) const;

	private:
		FProceduralTerrainParams Params;
	};
}

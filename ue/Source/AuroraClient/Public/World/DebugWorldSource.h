#pragma once

#include "CoreMinimal.h"
#include "World/WorldSource.h"

/**
 * Вариант 3: отладочный источник.
 *
 * Не содержит ни геометрии карты, ни моделей. Отдаёт плоскую сетку на
 * уровне земли и маркеры сущностей — этого достаточно, чтобы увидеть,
 * что поток мира живой и координаты приходят правильные.
 */
namespace Aurora::World
{
	class FDebugWorldSource final : public IWorldSource
	{
	public:
		FDebugWorldSource() = default;
		explicit FDebugWorldSource(const TArray<FWorldEntity>& InEntities)
			: Entities(InEntities) {}

		void SetEntities(const TArray<FWorldEntity>& InEntities)
		{
			Entities = InEntities;
		}

		virtual bool GetTerrain(float CenterX, float CenterY, float Radius,
		                        TArray<FTerrainChunk>& OutChunks) override;

		virtual bool GetEntities(float CenterX, float CenterY, float Radius,
		                         TArray<FWorldEntity>& OutEntities) override;

		virtual FString GetName() const override { return TEXT("Debug"); }

	private:
		TArray<FWorldEntity> Entities;

		/** Размер ячейки отладочной сетки в ярдах WoW. */
		static constexpr float GridStep = 10.f;
		static constexpr int32 GridCells = 16;
	};
}

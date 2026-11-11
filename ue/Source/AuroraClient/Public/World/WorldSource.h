#pragma once

#include "CoreMinimal.h"

/**
 * Этап 5: отрисовка мира.
 *
 * Три варианта, выбранных пользователем (1–3), живут за одним интерфейсом,
 * чтобы сцена не зависела от способа получения геометрии:
 *
 *  1. Procedural  — ландшафт генерируется из карты высот, модели заменены
 *                   простыми плейсхолдерами. Юридически чисто.
 *  2. External    — ассеты приходят из внешнего источника (конвертер),
 *                   с обязательной проверкой прав на файлы.
 *  3. Debug       — только сетка координат и маркеры, для проверки, что
 *                   поток мира живой.
 *
 * WorldSource ничего не знает о UE-акторах: он отдаёт данные, а акторы
 * решают, как их показать. Это позволяет тестировать источник без рендера.
 */
namespace Aurora::World
{
	/** Вершина ландшафта в координатах WoW. */
	struct FTerrainVertex
	{
		float X = 0.f;
		float Y = 0.f;
		float Z = 0.f;
	};

	/** Кусок ландшафта: сетка вершин rows x cols. */
	struct FTerrainChunk
	{
		int32 Rows = 0;
		int32 Cols = 0;
		TArray<FTerrainVertex> Vertices;
	};

	/** Что показать в точке мира. */
	enum class EEntityKind : uint8
	{
		Player,
		Npc,
		GameObject,
		Unknown,
	};

	struct FWorldEntity
	{
		uint64 Guid = 0;
		EEntityKind Kind = EEntityKind::Unknown;
		FString Name;
		float X = 0.f;
		float Y = 0.f;
		float Z = 0.f;
		float Orientation = 0.f;
	};

	/** Единый интерфейс источника данных мира. */
	class IWorldSource
	{
	public:
		virtual ~IWorldSource() = default;

		/** Террейн вокруг точки. Пустой массив означает «нет данных». */
		virtual bool GetTerrain(float CenterX, float CenterY, float Radius,
		                        TArray<FTerrainChunk>& OutChunks) = 0;

		/** Сущности для отрисовки в радиусе. */
		virtual bool GetEntities(float CenterX, float CenterY, float Radius,
		                         TArray<FWorldEntity>& OutEntities) = 0;

		/** Короткое имя источника для логов. */
		virtual FString GetName() const = 0;
	};
}

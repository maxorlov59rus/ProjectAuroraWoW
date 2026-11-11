#pragma once

#include "CoreMinimal.h"
#include "World/WorldSource.h"
#include "World/UpdateObject.h"

/**
 * Мир-состояние: словарь объектов по GUID.
 *
 * Принимает события от парсера SMSG_UPDATE_OBJECT и умеет отдать их в виде
 * FWorldEntity для отрисовки. Ничего не знает о UE-акторах, поэтому
 * тестируется без рендера.
 *
 * Потокобезопасность: класс не синхронизирован. Один поток обновляет
 * состояние (сетевой), другой читает (игровой) — вызывающая сторона
 * отвечает за сериализацию. Для этапа 6 достаточно обновления из тика.
 */
namespace Aurora::World
{
	struct FWorldObject
	{
		uint64 Guid = 0;
		EEntityKind Kind = EEntityKind::Unknown;
		FString Name;
		float X = 0.f;
		float Y = 0.f;
		float Z = 0.f;
		float Orientation = 0.f;
	};

	class FWorldState
	{
	public:
		/** Применяет пакет SMSG_UPDATE_OBJECT. Возвращает число изменений. */
		int32 ApplyUpdateObject(const TArray<uint8>& Body);

		/** Удаляет объект. */
		bool Remove(uint64 Guid);

		/** Полная очистка (при выходе из мира или смене карты). */
		void Clear();

		/** Объект по guid. */
		const FWorldObject* Find(uint64 Guid) const;

		/** Все известные объекты. */
		const TMap<uint64, FWorldObject>& GetObjects() const { return Objects; }

		/** Объекты в радиусе от точки, готовые для отрисовки. */
		void GetEntitiesNear(float CenterX, float CenterY, float Radius,
		                     TArray<FWorldEntity>& OutEntities) const;

		/** Количество объектов. */
		int32 Num() const { return Objects.Num(); }

	private:
		TMap<uint64, FWorldObject> Objects;
	};
}

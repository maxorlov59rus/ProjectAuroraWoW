#pragma once

#include "CoreMinimal.h"
#include "World/WorldSource.h"

/**
 * Разбор SMSG_UPDATE_OBJECT (0x00A9) для 3.3.5a.
 *
 * Пакет описывает появление и изменение объектов в мире. Он может
 * содержать несколько блоков подряд, и блоки разных типов чередуются:
 *
 *   OUT_OF_RANGE_OBJECTS   (0-2)      — guid'ы, ушедшие из видимости
 *   NEAR_OBJECTS           (0-1)      — guid'ы с обновлением «рядом»
 *   VALUES / MOVEMENT      (2, 3)     — полное описание или дельта объекта
 *   CREATE_OBJECT / CREATE_OBJECT2 (4, 5) — новый объект с блоками полей
 *
 * Мир-состояние (`FWorldState`) применяет блоки к словарю объектов, а
 * отрисовка берёт готовые позиции. Парсер не знает про UE-акторы.
 */
namespace Aurora::World
{
	enum class EUpdateBlock : uint8
	{
		OutOfRangeObjects,
		NearObjects,
		Values,
		Movement,
		CreateObject,
		CreateObject2,
		Unknown,
	};

	enum class EObjectTypeMask : uint16
	{
		Object       = 0x0001,
		Item         = 0x0002,
		Container    = 0x0004,
		Unit         = 0x0008,
		Player       = 0x0010,
		GameObject   = 0x0020,
		DynamicObject = 0x0040,
		Corpse       = 0x0080,
	};

	/** Одно событие из пакета: блок и его payload. */
	struct FUpdateEvent
	{
		EUpdateBlock Block = EUpdateBlock::Unknown;
		uint64 Guid = 0;
		uint16 TypeMask = 0;
		float X = 0.f;
		float Y = 0.f;
		float Z = 0.f;
		float Orientation = 0.f;
		FString Name;
	};

	/**
	 * Разбирает тело SMSG_UPDATE_OBJECT на события.
	 *
	 * Возвращает false, если данные усечены: поток не должен продолжать
	 * разбор пакета, потому что смещения после ошибки уже не восстановить.
	 *
	 * @param Body           тело пакета
	 * @param OutEvents      события в порядке следования
	 * @param OutDestroyed   guid'ы, вышедшие из видимости
	 */
	bool ParseUpdateObject(
		const TArray<uint8>& Body,
		TArray<FUpdateEvent>& OutEvents,
		TArray<uint64>& OutDestroyed);

	/** Расшифровывает маску типа объекта в вид сущности для отрисовки. */
	EEntityKind KindFromTypeMask(uint16 TypeMask);
}

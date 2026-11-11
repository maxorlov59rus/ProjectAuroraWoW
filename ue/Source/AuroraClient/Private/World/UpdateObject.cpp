#include "World/UpdateObject.h"

namespace Aurora::World
{
	namespace
	{
		struct FReader
		{
			const TArray<uint8>& Data;
			int32 Offset = 0;

			explicit FReader(const TArray<uint8>& In) : Data(In) {}

			bool Can(int32 Count) const
			{
				return Count >= 0 && Offset + Count <= Data.Num();
			}

			bool ReadU8(uint8& Out)
			{
				if (!Can(1)) { return false; }
				Out = Data[Offset++];
				return true;
			}

			bool ReadU16(uint16& Out)
			{
				if (!Can(2)) { return false; }
				FMemory::Memcpy(&Out, Data.GetData() + Offset, 2);
				Offset += 2;
				return true;
			}

			bool ReadU32(uint32& Out)
			{
				if (!Can(4)) { return false; }
				FMemory::Memcpy(&Out, Data.GetData() + Offset, 4);
				Offset += 4;
				return true;
			}

			bool ReadU64(uint64& Out)
			{
				if (!Can(8)) { return false; }
				FMemory::Memcpy(&Out, Data.GetData() + Offset, 8);
				Offset += 8;
				return true;
			}

			bool ReadF32(float& Out)
			{
				if (!Can(4)) { return false; }
				FMemory::Memcpy(&Out, Data.GetData() + Offset, 4);
				Offset += 4;
				return true;
			}

			/** Packed guid: маска 1 байт, затем только ненулевые байты. */
			bool ReadPackedGuid(uint64& Out)
			{
				Out = 0;

				uint8 Mask = 0;
				if (!ReadU8(Mask))
				{
					return false;
				}

				for (int32 Bit = 0; Bit < 8; ++Bit)
				{
					if ((Mask & (1 << Bit)) == 0)
					{
						continue;
					}

					uint8 Byte = 0;
					if (!ReadU8(Byte))
					{
						return false;
					}
					Out |= static_cast<uint64>(Byte) << (Bit * 8);
				}

				return true;
			}

			/** Битовое поле в стиле WotLK: только ненулевые биты по одному байту. */
			bool ReadPackedValue(uint32 Bytes, uint32& Out)
			{
				Out = 0;
				for (int32 i = 0; i < static_cast<int32>(Bytes); ++i)
				{
					uint8 Byte = 0;
					if (!ReadU8(Byte))
					{
						return false;
					}
					Out |= static_cast<uint32>(Byte) << (i * 8);
				}
				return true;
			}
		};

		// ------------------------------------------------------------------
		// Движение
		// ------------------------------------------------------------------

		/**
		 * Пропускает движение транспортного объекта (тип 0) до его полей.
		 * Часть полей зависит от флагов, поэтому порядок строго по протоколу.
		 */
		bool SkipTransportFields(FReader& Reader, uint32 TransportFlags)
		{
			uint32 TransportTime = 0;
			if (!Reader.ReadU32(TransportTime))
			{
				return false;
			}

			// Транспортный guid: packed.
			uint64 TransportGuid = 0;
			if (!Reader.ReadPackedGuid(TransportGuid))
			{
				return false;
			}

			if ((TransportFlags & 0x001) == 0)
			{
				return false;
			}

			float F1 = 0.f, F2 = 0.f, F3 = 0.f, F4 = 0.f;
			uint16 T1 = 0;
			if (!Reader.ReadF32(F1) || !Reader.ReadF32(F2) || !Reader.ReadF32(F3))
			{
				return false;
			}
			if (!Reader.ReadU16(T1))
			{
				return false;
			}
			if (!Reader.ReadF32(F4))
			{
				return false;
			}

			// t_time2 и производные от флагов 0x2/0x4/0x8.
			uint32 Time2 = 0;
			if (!Reader.ReadU32(Time2))
			{
				return false;
			}

			if (TransportFlags & 0x002)
			{
				float Extra = 0.f;
				if (!Reader.ReadF32(Extra)) { return false; }
			}
			if (TransportFlags & 0x004)
			{
				uint32 Extra = 0;
				if (!Reader.ReadU32(Extra)) { return false; }
			}
			if (TransportFlags & 0x008)
			{
				uint32 Extra = 0;
				if (!Reader.ReadU32(Extra)) { return false; }
			}

			return true;
		}

		/** Разбирает движение и возвращает позицию. */
		bool ReadMovement(FReader& Reader, FUpdateEvent& Event)
		{
			uint16 Flags = 0;
			if (!Reader.ReadU16(Flags))
			{
				return false;
			}

			uint16 MovementFlags2 = 0;
			if (!Reader.ReadU16(MovementFlags2))
			{
				return false;
			}

			uint32 Time = 0;
			if (!Reader.ReadU32(Time))
			{
				return false;
			}

			if (!Reader.ReadF32(Event.X) ||
				!Reader.ReadF32(Event.Y) ||
				!Reader.ReadF32(Event.Z) ||
				!Reader.ReadF32(Event.Orientation))
			{
				return false;
			}

			// Транспорт
			if (Flags & 0x0200)
			{
				uint8 HasTransport = 0;
				if (!Reader.ReadU8(HasTransport))
				{
					return false;
				}

				if (HasTransport != 0)
				{
					uint32 TransportFlags = 0;
					if (!Reader.ReadU32(TransportFlags))
					{
						return false;
					}
					if (!SkipTransportFields(Reader, TransportFlags))
					{
						return false;
					}
				}
			}

			// Тело в воздухе / на земле
			const bool bOnGround = (Flags & (0x001 | 0x0800)) != 0;

			if (bOnGround)
			{
				uint32 FallTime = 0;
				if (!Reader.ReadU32(FallTime))
				{
					return false;
				}
				return true;
			}

			// Не на земле: x/y/z + время падения
			float Velocity = 0.f;
			uint32 FallTime = 0;
			if (!Reader.ReadF32(Velocity) || !Reader.ReadU32(FallTime))
			{
				return false;
			}

			// Направление и скорость поворота при движении
			if ((Flags & 0x0004) != 0)
			{
				float Extra = 0.f;
				if (!Reader.ReadF32(Extra)) { return false; }
			}

			// Направление и скорость по вертикали при падении
			if ((Flags & 0x0008) != 0)
			{
				float Extra = 0.f;
				if (!Reader.ReadF32(Extra)) { return false; }
			}

			// Точная ориентация
			if ((Flags & 0x0020) != 0)
			{
				float ExactPitch = 0.f;
				if (!Reader.ReadF32(ExactPitch)) { return false; }
			}

			// Прыжок
			if ((Flags & 0x0400) != 0)
			{
				uint32 JumpData = 0;
				if (!Reader.ReadU32(JumpData)) { return false; }
			}

			// SplineElevation
			if ((MovementFlags2 & 0x200) != 0)
			{
				float SplineElevation = 0.f;
				if (!Reader.ReadF32(SplineElevation)) { return false; }
			}

			return true;
		}

		// ------------------------------------------------------------------
		// Поля объекта
		// ------------------------------------------------------------------

		/**
		 * Пропускает массив значений одного поля.
		 *
		 * В 3.3.5a значение кодируется по типу поля:
		 *   0 — int32 (4 байта)
		 *   1 — float (4 байта)
		 *   2 — int64 (8 байт)
		 *   3 — string (длина + байты)
		 *
		 * Высокий бит маски (0x80) означает, что значение — массив. Формат
		 * массива в 3.3.5a не передаёт счётчик в явном виде, поэтому такой
		 * блок на этом этапе считается непрочитанным: безопаснее остановиться,
		 * чем разъехаться на следующих полях.
		 */
		bool SkipFieldValue(FReader& Reader, uint8 FieldValue, bool bIsArray)
		{
			if (bIsArray)
			{
				return false;
			}

			const uint8 FieldType = FieldValue & 0x7F;

			switch (FieldType)
			{
				case 0:  // int32
				case 1:  // float
				{
					uint32 Dummy = 0;
					return Reader.ReadU32(Dummy);
				}
				case 2:  // int64
				{
					uint64 Dummy = 0;
					return Reader.ReadU64(Dummy);
				}
				case 3:  // string: 1 байт длины + данные
				{
					uint8 Length = 0;
					if (!Reader.ReadU8(Length))
					{
						return false;
					}
					Reader.Offset += Length;
					return Reader.Can(0);
				}
				default:
					return false;
			}
		}

		/** Пропускает блок UpdateFields. */
		bool SkipUpdateFields(FReader& Reader)
		{
			uint8 Blocks = 0;
			if (!Reader.ReadU8(Blocks))
			{
				return false;
			}

			for (uint8 Block = 0; Block < Blocks; ++Block)
			{
				uint8 Mask = 0;
				if (!Reader.ReadU8(Mask))
				{
					return false;
				}

				for (int32 Bit = 0; Bit < 8; ++Bit)
				{
					if ((Mask & (1 << Bit)) == 0)
					{
						continue;
					}

					// index(1) + value(1); индекс в 3.3.5a — байт смещения.
					uint8 FieldIndex = 0;
					uint8 FieldValue = 0;
					if (!Reader.ReadU8(FieldIndex) || !Reader.ReadU8(FieldValue))
					{
						return false;
					}

					const bool bIsArray = (FieldValue & 0x80) != 0;
					if (!SkipFieldValue(Reader, FieldValue, bIsArray))
					{
						return false;
					}
				}
			}

			return true;
		}

		// ------------------------------------------------------------------
		// Блоки
		// ------------------------------------------------------------------

		bool SkipOutOfRange(FReader& Reader, TArray<uint64>& OutDestroyed)
		{
			uint16 Count = 0;
			if (!Reader.ReadU16(Count))
			{
				return false;
			}

			for (uint16 N = 0; N < Count; ++N)
			{
				uint64 Guid = 0;
				if (!Reader.ReadPackedGuid(Guid))
				{
					return false;
				}
				OutDestroyed.Add(Guid);
			}
			return true;
		}

		bool SkipNearObjects(FReader& Reader)
		{
			uint16 Count = 0;
			if (!Reader.ReadU16(Count))
			{
				return false;
			}

			for (uint16 N = 0; N < Count; ++N)
			{
				uint64 Guid = 0;
				if (!Reader.ReadPackedGuid(Guid))
				{
					return false;
				}
				// Объект «рядом» без движения нам не нужен.
			}
			return true;
		}

		bool ParseMovementBlock(FReader& Reader, TArray<FUpdateEvent>& OutEvents)
		{
			uint64 Guid = 0;
			if (!Reader.ReadPackedGuid(Guid))
			{
				return false;
			}

			FUpdateEvent Event;
			Event.Block = EUpdateBlock::Movement;
			Event.Guid = Guid;

			if (!ReadMovement(Reader, Event))
			{
				return false;
			}

			OutEvents.Add(Event);
			return true;
		}

		bool ParseValuesBlock(FReader& Reader, TArray<FUpdateEvent>& OutEvents)
		{
			uint64 Guid = 0;
			if (!Reader.ReadPackedGuid(Guid))
			{
				return false;
			}

			if (!SkipUpdateFields(Reader))
			{
				return false;
			}

			FUpdateEvent Event;
			Event.Block = EUpdateBlock::Values;
			Event.Guid = Guid;
			OutEvents.Add(Event);
			return true;
		}

		bool ParseCreateBlock(FReader& Reader, EUpdateBlock Block, TArray<FUpdateEvent>& OutEvents)
		{
			uint64 Guid = 0;
			if (!Reader.ReadPackedGuid(Guid))
			{
				return false;
			}

			uint8 RawObjectType = 0;
			if (!Reader.ReadU8(RawObjectType))
			{
				return false;
			}

			FUpdateEvent Event;
			Event.Block = Block;
			Event.Guid = Guid;
			Event.TypeMask = RawObjectType;

			if (!ReadMovement(Reader, Event))
			{
				return false;
			}

			OutEvents.Add(Event);

			// После движения объект описывает свои поля. Игрок и unit несут
			// дополнительный блок (флаги/ауры), затем UpdateFields.
			const bool bHasExtraBlock =
				(RawObjectType & static_cast<uint8>(EObjectTypeMask::Unit)) != 0 ||
				(RawObjectType & static_cast<uint8>(EObjectTypeMask::Player)) != 0 ||
				(RawObjectType & static_cast<uint8>(EObjectTypeMask::Corpse)) != 0;

			if (bHasExtraBlock)
			{
				// Unit/Player: необязательный блок полей перед update fields.
				// В 3.3.5a он заканчивается тем же форматом UpdateFields,
				// поэтому читаем как обычный блок полей.
				if (!SkipUpdateFields(Reader))
				{
					return false;
				}
			}

			if (!SkipUpdateFields(Reader))
			{
				return false;
			}

			return true;
		}
	}

	EEntityKind KindFromTypeMask(uint16 TypeMask)
	{
		if ((TypeMask & static_cast<uint16>(EObjectTypeMask::Player)) != 0)
		{
			return EEntityKind::Player;
		}

		if ((TypeMask & static_cast<uint16>(EObjectTypeMask::Unit)) != 0)
		{
			return EEntityKind::Npc;
		}

		if ((TypeMask & static_cast<uint16>(EObjectTypeMask::GameObject)) != 0)
		{
			return EEntityKind::GameObject;
		}

		return EEntityKind::Unknown;
	}

	bool ParseUpdateObject(
		const TArray<uint8>& Body,
		TArray<FUpdateEvent>& OutEvents,
		TArray<uint64>& OutDestroyed)
	{
		OutEvents.Reset();
		OutDestroyed.Reset();

		FReader Reader(Body);

		uint32 BlockCount = 0;
		if (!Reader.ReadU32(BlockCount))
		{
			return false;
		}

		for (uint32 Index = 0; Index < BlockCount; ++Index)
		{
			uint8 RawType = 0;
			if (!Reader.ReadU8(RawType))
			{
				return false;
			}

			const EUpdateBlock Block = static_cast<EUpdateBlock>(RawType);
			bool bOk = false;

			switch (Block)
			{
				case EUpdateBlock::OutOfRangeObjects:
					bOk = SkipOutOfRange(Reader, OutDestroyed);
					break;

				case EUpdateBlock::NearObjects:
					bOk = SkipNearObjects(Reader);
					break;

				case EUpdateBlock::Movement:
					bOk = ParseMovementBlock(Reader, OutEvents);
					break;

				case EUpdateBlock::Values:
					bOk = ParseValuesBlock(Reader, OutEvents);
					break;

				case EUpdateBlock::CreateObject:
				case EUpdateBlock::CreateObject2:
					bOk = ParseCreateBlock(Reader, Block, OutEvents);
					break;

				default:
					// Незнакомый блок: смещение после него недостоверно.
					return true;
			}

			if (!bOk)
			{
				// Пакет усечён или формат не совпал. То, что уже разобрано,
				// остаётся у вызывающей стороны; остальное отбрасываем.
				return true;
			}
		}

		return true;
	}
}

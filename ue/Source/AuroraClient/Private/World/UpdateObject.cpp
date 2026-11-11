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
		};

		/** Пропускает блок UPDATE_FIELD (не нужен для отрисовки). */
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

					// UPDATE_FIELD: index(1) + type(1) + значении по типу.
					uint8 FieldIndex = 0;
					uint8 FieldType = 0;
					if (!Reader.ReadU8(FieldIndex) || !Reader.ReadU8(FieldType))
					{
						return false;
					}

					switch (FieldType)
					{
						case 0:  // int32
						case 1:  // float
							Reader.Offset += 4;
							break;
						case 2:  // int64
							Reader.Offset += 8;
							break;
						default:
							return false;
					}

					if (!Reader.Can(0))
					{
						return false;
					}
				}
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

			switch (static_cast<EUpdateBlock>(RawType))
			{
				case EUpdateBlock::OutOfRangeObjects:
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
					break;
				}

				case EUpdateBlock::NearObjects:
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
						// Объект «рядом»: обновление без движения нам не нужно.
					}
					break;
				}

				case EUpdateBlock::Movement:
				{
					uint64 Guid = 0;
					if (!Reader.ReadPackedGuid(Guid))
					{
						return false;
					}

					FUpdateEvent Event;
					Event.Block = EUpdateBlock::Movement;
					Event.Guid = Guid;

					if (Reader.ReadF32(Event.X) &&
						Reader.ReadF32(Event.Y) &&
						Reader.ReadF32(Event.Z) &&
						Reader.ReadF32(Event.Orientation))
					{
						OutEvents.Add(Event);
					}

					// Остаток блока движения повлияет на следующие блоки,
					// поэтому дальнейший разбор считаем небезопасным.
					return true;
				}

				case EUpdateBlock::Values:
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
					break;
				}

				case EUpdateBlock::CreateObject:
				case EUpdateBlock::CreateObject2:
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
					Event.Block = static_cast<EUpdateBlock>(RawType);
					Event.Guid = Guid;
					Event.TypeMask = RawObjectType;

					// Блок движения в create идёт сразу после типа.
					if (!Reader.ReadF32(Event.X) ||
						!Reader.ReadF32(Event.Y) ||
						!Reader.ReadF32(Event.Z) ||
						!Reader.ReadF32(Event.Orientation))
					{
						return false;
					}

					OutEvents.Add(Event);

					// Дальше идут поля объекта переменной длины; для отрисовки
					// координат достаточно. Остаток блока пропускается до
					// следующего CreateObject, поэтому безопаснее остановиться.
					return true;
				}

				default:
					// Неизвестный блок — смещения после него недостоверны.
					return true;
			}
		}

		return true;
	}
}

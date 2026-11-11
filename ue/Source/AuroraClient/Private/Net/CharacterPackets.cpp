#include "Net/CharacterPackets.h"

namespace Aurora::World
{
	namespace
	{
		struct FReader
		{
			const TArray<uint8>& Data;
			int32 Offset = 0;

			explicit FReader(const TArray<uint8>& In) : Data(In) {}

			bool ReadU8(uint8& Out)
			{
				if (Offset + 1 > Data.Num()) { return false; }
				Out = Data[Offset++];
				return true;
			}

			bool ReadU32(uint32& Out)
			{
				if (Offset + 4 > Data.Num()) { return false; }
				FMemory::Memcpy(&Out, Data.GetData() + Offset, 4);
				Offset += 4;
				return true;
			}

			bool ReadU64(uint64& Out)
			{
				if (Offset + 8 > Data.Num()) { return false; }
				FMemory::Memcpy(&Out, Data.GetData() + Offset, 8);
				Offset += 8;
				return true;
			}

			bool ReadF32(float& Out)
			{
				if (Offset + 4 > Data.Num()) { return false; }
				FMemory::Memcpy(&Out, Data.GetData() + Offset, 4);
				Offset += 4;
				return true;
			}

			bool ReadString(FString& Out)
			{
				uint8 Len = 0;
				if (!ReadU8(Len)) { return false; }
				if (Offset + Len > Data.Num()) { return false; }

				Out = FString(Len, reinterpret_cast<const ANSICHAR*>(Data.GetData() + Offset));
				Offset += Len;
				return true;
			}

			bool Skip(int32 Count)
			{
				if (Count < 0 || Offset + Count > Data.Num()) { return false; }
				Offset += Count;
				return true;
			}
		};
	}

	bool ParseCharEnum(const TArray<uint8>& Body, TArray<FCharacterSummary>& OutCharacters)
	{
		OutCharacters.Reset();

		FReader Reader(Body);

		uint8 Count = 0;
		if (!Reader.ReadU8(Count))
		{
			return false;
		}

		OutCharacters.Reserve(Count);

		for (uint8 Index = 0; Index < Count; ++Index)
		{
			FCharacterSummary Char;

			uint8 Skin = 0, Face = 0, HairStyle = 0, HairColor = 0, FacialHair = 0;

			if (!Reader.ReadU64(Char.Guid) ||
				!Reader.ReadString(Char.Name) ||
				!Reader.ReadU8(Char.Race) ||
				!Reader.ReadU8(Char.Class) ||
				!Reader.ReadU8(Char.Gender) ||
				!Reader.ReadU8(Skin) ||
				!Reader.ReadU8(Face) ||
				!Reader.ReadU8(HairStyle) ||
				!Reader.ReadU8(HairColor) ||
				!Reader.ReadU8(FacialHair) ||
				!Reader.ReadU8(Char.Level) ||
				!Reader.ReadU32(Char.ZoneId) ||
				!Reader.ReadU32(Char.MapId) ||
				!Reader.ReadF32(Char.X) ||
				!Reader.ReadF32(Char.Y) ||
				!Reader.ReadF32(Char.Z) ||
				!Reader.ReadU32(Char.GuildId) ||
				!Reader.ReadU32(Char.PlayerFlags) ||
				!Reader.ReadU32(Char.CustomizationFlags) ||
				!Reader.ReadU8(Char.FirstLogin) ||
				!Reader.ReadU32(Char.PetDisplayId) ||
				!Reader.ReadU32(Char.PetLevel) ||
				!Reader.ReadU32(Char.PetFamily))
			{
				return false;
			}

			// 19 видимых слотов экипировки в 3.3.5a:
			// itemDisplayInfo(4) + inventoryType(1) + enchantDisplayInfo(4) = 9 байт.
			if (!Reader.Skip(19 * 9))
			{
				return false;
			}

			// 4 байта «flags2» банка в некоторых сборках — читаем только если
			// они есть, иначе считаем, что пакет закончился на этом персонаже.
			if (Reader.Offset + 4 <= Body.Num())
			{
				uint32 BankFlags = 0;
				Reader.ReadU32(BankFlags);
			}

			OutCharacters.Add(Char);
		}

		return true;
	}

	bool ParseLoginVerifyWorld(
		const TArray<uint8>& Body,
		uint32& OutMapId,
		float& OutX,
		float& OutY,
		float& OutZ,
		float& OutO)
	{
		FReader Reader(Body);

		if (!Reader.ReadU32(OutMapId) ||
			!Reader.ReadF32(OutX) ||
			!Reader.ReadF32(OutY) ||
			!Reader.ReadF32(OutZ) ||
			!Reader.ReadF32(OutO))
		{
			return false;
		}

		return true;
	}
}

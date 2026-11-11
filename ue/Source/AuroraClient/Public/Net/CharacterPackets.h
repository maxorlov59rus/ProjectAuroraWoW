#pragma once

#include "CoreMinimal.h"

/**
 * Разбор SMSG_CHAR_ENUM (0x003B) — список персонажей на экране выбора.
 *
 * Формат 3.3.5a:
 *   uint8  count
 *   повтор count раз:
 *     uint64 guid
 *     PascalString name
 *     uint8  race
 *     uint8  class
 *     uint8  gender
 *     uint8  skin, face, hairStyle, hairColor, facialHair   (5 байт)
 *     uint8  level
 *     uint32 zoneId
 *     uint32 mapId
 *     float  x, y, z
 *     uint32 guildId
 *     uint32 flags
 *     uint32 customizationFlags
 *     uint8  firstLogin
 *     uint32 petDisplayId, petLevel, petFamily
 *     20 x (uint32 itemDisplayInfo + uint8 inventoryType + uint32 enchantDisplayInfo)
 *     (WotLK) uint32? — банковские слоты отсутствуют, но есть 4 байта padding
 *     в некоторых сборках; остаток тела дописывается как есть.
 */
namespace Aurora::World
{
	struct FCharacterSummary
	{
		uint64 Guid = 0;
		FString Name;
		uint8 Race = 0;
		uint8 Class = 0;
		uint8 Gender = 0;
		uint8 Level = 1;
		uint32 ZoneId = 0;
		uint32 MapId = 0;
		float X = 0.f;
		float Y = 0.f;
		float Z = 0.f;
		uint32 GuildId = 0;
		uint32 PlayerFlags = 0;
		uint32 CustomizationFlags = 0;
		uint8 FirstLogin = 0;
		uint32 PetDisplayId = 0;
		uint32 PetLevel = 0;
		uint32 PetFamily = 0;
	};

	/** Разбирает тело SMSG_CHAR_ENUM. Возвращает false при усечённых данных. */
	bool ParseCharEnum(const TArray<uint8>& Body, TArray<FCharacterSummary>& OutCharacters);

	/** Разбирает тело SMSG_LOGIN_VERIFY_WORLD (map, x, y, z, o). */
	bool ParseLoginVerifyWorld(
		const TArray<uint8>& Body,
		uint32& OutMapId,
		float& OutX,
		float& OutY,
		float& OutZ,
		float& OutO);
}

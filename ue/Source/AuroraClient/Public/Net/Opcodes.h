#pragma once

#include "CoreMinimal.h"

/**
 * Opcodes и константы протокола WorldServer для 3.3.5a (build 12340).
 *
 * Значения взяты из публичной таблицы opcodes AzerothCore; они
 * отличаются от более поздних патчей, поэтому менять их нельзя.
 */
namespace Aurora::World::Opcodes
{
	// --- подключение и вход ---------------------------------------------
	constexpr uint32 CMSG_AUTH_SESSION        = 0x01ED;
	constexpr uint32 SMSG_AUTH_CHALLENGE      = 0x01EC;
	constexpr uint32 SMSG_AUTH_RESPONSE       = 0x01EE;
	constexpr uint32 CMSG_CHAR_ENUM           = 0x0037;
	constexpr uint32 SMSG_CHAR_ENUM           = 0x003B;
	constexpr uint32 CMSG_PLAYER_LOGIN        = 0x003D;
	constexpr uint32 SMSG_LOGIN_VERIFY_WORLD  = 0x0236;
	constexpr uint32 SMSG_LOGIN_SETTIMESPEED  = 0x0042;
	constexpr uint32 SMSG_TUTORIAL_FLAGS      = 0x00FD;
	constexpr uint32 SMSG_ADDON_INFO          = 0x02EF;
	constexpr uint32 CMSG_LOGOUT_REQUEST      = 0x004B;
	constexpr uint32 SMSG_LOGOUT_COMPLETE     = 0x004C;

	// --- коды AuthSessionResult ------------------------------------------
	constexpr uint8 AUTH_OK                   = 0x0C;
	constexpr uint8 AUTH_FAILED               = 0x0D;
	constexpr uint8 AUTH_REJECT                = 0x0E;
	constexpr uint8 AUTH_BAD_SERVER_PROOF      = 0x0F;
	constexpr uint8 AUTH_UNAVAILABLE           = 0x10;
	constexpr uint8 AUTH_SYSTEM_ERROR          = 0x11;
	constexpr uint8 AUTH_BILLING_ERROR         = 0x12;
	constexpr uint8 AUTH_BILLING_EXPIRED       = 0x13;
	constexpr uint8 AUTH_VERSION_MISMATCH      = 0x14;
	constexpr uint8 AUTH_UNKNOWN_ACCOUNT       = 0x15;
	constexpr uint8 AUTH_INCORRECT_PASSWORD    = 0x16;
	constexpr uint8 AUTH_SESSION_EXPIRED       = 0x17;
	constexpr uint8 AUTH_SERVER_SHUTTING_DOWN  = 0x18;
	constexpr uint8 AUTH_ALREADY_LOGGING_IN    = 0x19;
	constexpr uint8 AUTH_LOGIN_SERVER_NOT_FOUND = 0x1A;
	constexpr uint8 AUTH_WAIT_QUEUE            = 0x1B;
	constexpr uint8 AUTH_BANNED                = 0x1C;
	constexpr uint8 AUTH_ALREADY_ONLINE        = 0x1D;
	constexpr uint8 AUTH_NO_TIME               = 0x1E;
	constexpr uint8 AUTH_DB_BUSY               = 0x1F;
	constexpr uint8 AUTH_SUSPENDED             = 0x20;

	/** Клиентская сборка 3.3.5a. */
	constexpr uint32 CLIENT_BUILD = 12340;

	/** Возвращает true для кодов, означающих успешный вход в мир. */
	inline bool IsAuthOk(uint8 Code)
	{
		return Code == AUTH_OK;
	}
}

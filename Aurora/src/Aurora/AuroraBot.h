/*
 * AuroraBotsModule - модуль системы активных ботов для AzerothCore (Wotlk)
 *
 * Предоставляет:
 *   - Команды чата и консоли: !createbot, !delbot, !listbots, !botai
 *   - Базовый класс AuroraBot (наследуется от Player)
 *   - Интеграция с Lua-скриптами через ScriptMgr AzerothCore
 *   - Хранение данных ботов в памяти (можно расширить БД)
 *
 * Модуль загружается через mekanism AzerothCore (как AuroraSkills).
 */

#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>

#include <AuroraPCH.h>

// Forward declarations for AzerothCore APIs (will be resolved at runtime)
struct WorldSession;
struct Player;
struct ChatHandler;
struct GuildMgr;

// Простая структура данных бота
struct BotInfo
{
    std::string name;
    std::string className;   // e.g. "WARRIOR", "MAGE"
    std::string race;        // e.g. "Human", "Orc"
    std::string luaScript;   // имя файла скрипта (без пути .lua)
    bool isRunning = false;
};

// Глобальный менеджер ботов (хранится в памяти)
class AuroraBotManager
{
public:
    static AuroraBotManager& Instance()
    {
        static AuroraBotManager inst;
        return inst;
    }

    // Создать нового бота. Возвращает Guid бота (или 0 при ошибке).
    // В реальном проекте здесь должен быть вызов CreatePlayer/GuidGenerator.
    uint64_t CreateBot(const std::string& name,
                       const std::string& className,
                       const std::string& race,
                       const std::string& luaScript = "");

    // Удалить бота по GUID.
    bool DeleteBot(uint64_t guid);

    // Получить информацию о боте.
    bool GetBot(uint64_t guid, BotInfo& out);

    // Список всех ботов.
    std::vector<BotInfo> ListBots() const;

    // Включить/выключить бота.
    bool SetRunning(uint64_t guid, bool running);

private:
    std::map<uint64_t, BotInfo> m_bots;
    uint64_t m_nextGuid = 1; // простая генерация GUID (в production - proper generator)
    std::mutex m_mutex;
};

//--------------------------------------------------------------------------
// Команды чата и консоли
//--------------------------------------------------------------------------

// Регистрирует команды в AzerothCore. Вызывается при загрузке модуля.
void RegisterBotCommands();

// Обработчик команды !createbot <name> <class> <race> [script]
// Пример: .createbot MyBot MAGE Human myscript
bool HandleCreateBotCommand(ChatHandler* handler, const std::string& args);

// Обработчик команды !delbot <guid>
bool HandleDelBotCommand(ChatHandler* handler, const std::string& args);

// Обработчик команды !listbots
bool HandleListBotsCommand(ChatHandler* handler, const std::string& args);

// Обработчик команды !botai <script_name>
// Устанавливает Lua-скрипт для бота (или сбрасывает: !botai "")
bool HandleBotAiCommand(ChatHandler* handler, const std::string& args);

//--------------------------------------------------------------------------
// Базовый класс бота
//--------------------------------------------------------------------------

/**
 * AuroraBot - базовый класс для управления ботом.
 * Наследуется от Player (AzerothCore) и переопределяет виртуальные методы AI.
 * Lua-скрипты могут регистрировать свои хуки через этот класс.
 */
class AuroraBot : public Player
{
public:
    AuroraBot() = default;
    ~AuroraBot() override = default;

    // Инициализация бота. Вызывается после создания в мире.
    bool Initialize(const std::string& name,
                    const std::string& className,
                    const std::string& race,
                    const std::string& luaScript);

    // Шutdown: остановка AI, сохранение состояния.
    void Shutdown();

    // Called by the server each update loop (обычно в UpdateWorld).
    void OnUpdate(uint32 diff);

    // Базовая логика движения/комбата (можно переопределять в Lua).
    virtual void OnMove(uint32 diff);
    virtual void OnSpellCast(uint32 diff);

    // Получить ссылку на информацию бота.
    const BotInfo& GetInfo() const { return m_info; }

private:
    BotInfo m_info;
    std::string m_luaScriptPath; // полный путь к .lua файлу
    bool m_initialized = false;
};

//--------------------------------------------------------------------------
// Lua-интеграция (обертка над ScriptMgr)
//--------------------------------------------------------------------------

/**
 * LoadBotScript - загружает Lua-скрипт для бота.
 * Скрипты ожидаются в папке scripts/bots/ относительно корня проекта.
 * Функции, которые скрипт должен реализовывать (как события):
 *   OnBotCreate(botGuid)
 *   OnBotUpdate(botGuid, diff_ms)
 *   OnBotCombatStart(botGuid, targetGuid)
 *   OnBotCombatEnd(botGuid)
 */
bool LoadBotScript(AuroraBot* bot);

/**
 * RunBotScriptEvent - выполняет событие из Lua-скрипта бота.
 * Вызывается из OnUpdate сервера.
 */
void RunBotScriptEvent(AuroraBot* bot, const std::string& eventName, uint32 diff);

/*extern "C"
{ // функции, которые AzerothCore ожидает видеть в модуле управления скриптами
// (упрощенно - на основе реальных Symbols AzerothCore)
// void __declspec(dllexport) RegisterBotScripts();
// }
*/
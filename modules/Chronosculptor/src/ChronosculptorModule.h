/*
 * ChronosculptorModule - "Sculptor of Time" for AzerothCore (WotLK)
 *
 * The player records a short route and, a moment later, an "echo" of the
 * player repeats it. Two echoes can exist at once and each lives for a limited
 * time, so a solo player can hold two pressure plates that would otherwise
 * need a group.
 *
 * The echo itself is a TempSummon spawned from Aurora.Chronosculptor.EchoEntry;
 * with EchoEntry = 0 the module stays in "virtual" mode and only simulates the
 * echo in memory (the creature entry does not exist until the matching SQL is
 * imported), which keeps the loop, the limits and the commands testable.
 */
#pragma once

#include "Define.h"
#include <string>
#include <vector>
#include <mutex>
#include <chrono>

class Player;

struct ChronoEcho
{
    uint32_t guid = 0;
    std::vector<float> xs, ys, zs, os; // recorded route
    uint64_t spawnDeadlineMs = 0;      // when the echo may appear
    uint64_t expireAtMs = 0;           // when the echo despawns
    bool spawned = false;
};
class ChronosculptorManager
{
public:
    static ChronosculptorManager& Instance();

    bool Initialize();

    bool StartRecord(Player* p);
    void CancelRecord(Player* p);
    // Called from the PlayerScript update hook while a recording is running.
    void OnPlayerUpdate(Player* p, uint32 diff);
    // Called from the WorldScript update hook to spawn/expire echoes.
    void Tick();

    bool IsEnabled() const { return m_enabled; }
    bool IsRecording() const { return m_recording; }
    uint32_t MaxEchoes() const { return m_maxEchoes; }
    uint32_t EchoLifetimeSecs() const { return m_echoLifetimeSecs; }
    std::vector<ChronoEcho> GetActiveEchoes() const;

private:
    ChronosculptorManager() = default;
    ~ChronosculptorManager() = default;

    void FinishRecord();

    bool m_enabled = true;
    uint32_t m_maxEchoes = 2;
    uint32_t m_echoLifetimeSecs = 30;
    uint32_t m_recordDurationMs = 3000;
    uint32_t m_echoDelayMs = 5000;
    uint32_t m_echoEntry = 0;

    bool m_recording = false;
    uint64_t m_recordStartMs = 0;
    uint64_t m_recordDeadlineMs = 0;
    std::vector<float> m_xs, m_ys, m_zs, m_os;

    std::vector<ChronoEcho> m_echoes;
    uint32_t m_nextEchoGuid = 1;
    mutable std::mutex m_mutex;
};
void AddChronosculptorScripts();
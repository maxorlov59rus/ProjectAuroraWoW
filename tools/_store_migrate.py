import io

p = 'modules/CampaignOfLight/src/CampaignOfLightModule.cpp'
s = io.open(p, encoding='utf-8').read()

pairs = [
    ('        static_cast<uint32>(m_npcs.size()),\n'
     '        static_cast<uint32>(m_quests.size()),\n'
     '        static_cast<uint32>(m_bosses.size()),\n'
     '        static_cast<uint32>(m_shops.size()));',
     '        m_store.NpcCount(), m_store.QuestCount(),\n'
     '        m_store.BossCount(), m_store.ShopCount());'),

    ('        m_npcs[npc.entry] = npc;',
     '        m_store.PutNpc(npc);'),

    ('    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    m_npcs[npc.entry] = npc;',
     '    m_store.PutNpc(npc);'),

    ('    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    return m_npcs.erase(entry) > 0;',
     '    return m_store.RemoveNpc(entry);'),

    ('CampaignNpc const* CampaignOfLightManager::GetNpc(uint32 entry) const\n'
     '{\n'
     '    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    auto it = m_npcs.find(entry);\n'
     '    return it != m_npcs.end() ? &it->second : nullptr;\n'
     '}',
     'std::optional<CampaignNpc> CampaignOfLightManager::GetNpc(uint32 entry) const\n'
     '{\n'
     '    return m_store.GetNpc(entry);\n'
     '}'),

    ('std::vector<CampaignNpc> CampaignOfLightManager::GetNpcs() const\n'
     '{\n'
     '    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    std::vector<CampaignNpc> list;\n'
     '    list.reserve(m_npcs.size());\n'
     '    for (auto const& [_, npc] : m_npcs)\n'
     '        list.push_back(npc);\n'
     '    return list;\n'
     '}',
     'std::vector<CampaignNpc> CampaignOfLightManager::GetNpcs() const\n'
     '{\n'
     '    return m_store.GetNpcs();\n'
     '}'),

    ('    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    m_dialogues[npcEntry][node.id] = node;',
     '    m_store.PutDialogue(npcEntry, node);'),

    ('    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '\n'
     '    auto npcIt = m_dialogues.find(npcEntry);\n'
     '    if (npcIt == m_dialogues.end())\n'
     '        return false;\n'
     '\n'
     '    auto nodeIt = npcIt->second.find(nodeId);\n'
     '    if (nodeIt == npcIt->second.end())\n'
     '        return false;\n'
     '\n'
     '    DialogueNode const& node = nodeIt->second;',
     '    auto Node = m_store.GetDialogue(npcEntry, nodeId);\n'
     '    if (!Node)\n'
     '        return false;\n'
     '\n'
     '    DialogueNode const& node = *Node;'),

    ('        m_quests[quest.questId] = quest;',
     '        m_store.PutQuest(quest);'),

    ('    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    m_quests[quest.questId] = quest;',
     '    m_store.PutQuest(quest);'),

    ('    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    auto it = m_quests.find(questId);\n'
     '    if (it == m_quests.end())\n'
     '        return false;\n'
     '\n'
     '    CampaignQuest const& quest = it->second;',
     '    auto Found = m_store.GetQuest(questId);\n'
     '    if (!Found)\n'
     '        return false;\n'
     '\n'
     '    CampaignQuest const& quest = *Found;'),

    ('CampaignQuest const* CampaignOfLightManager::GetQuest(uint32 questId) const\n'
     '{\n'
     '    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    auto it = m_quests.find(questId);\n'
     '    return it != m_quests.end() ? &it->second : nullptr;\n'
     '}',
     'std::optional<CampaignQuest> CampaignOfLightManager::GetQuest(uint32 questId) const\n'
     '{\n'
     '    return m_store.GetQuest(questId);\n'
     '}'),

    ('        m_bosses[boss.entry] = boss;',
     '        m_store.PutBoss(boss);'),

    ('    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    auto it = m_bosses.find(bossEntry);\n'
     '    if (it == m_bosses.end())\n'
     '        return false;\n'
     '\n'
     '    CampaignBoss const& boss = it->second;',
     '    auto Found = m_store.GetBoss(bossEntry);\n'
     '    if (!Found)\n'
     '        return false;\n'
     '\n'
     '    CampaignBoss const& boss = *Found;'),

    ('    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    return m_bosses.erase(bossEntry) > 0;',
     '    return m_store.RemoveBoss(bossEntry);'),

    ('    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    std::vector<CampaignBoss> list;\n'
     '    list.reserve(m_bosses.size());\n'
     '    for (auto const& [_, boss] : m_bosses)\n'
     '        list.push_back(boss);\n'
     '    return list;',
     '    return m_store.GetBosses();'),

    ('        m_shops[shop.vendorEntry] = shop;',
     '        m_store.PutShop(shop);'),

    ('    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    m_shops[shop.vendorEntry] = shop;',
     '    m_store.PutShop(shop);'),

    ('    std::lock_guard<std::mutex> lock(m_mutex);\n'
     '    auto it = m_shops.find(vendorEntry);\n'
     '    if (it == m_shops.end())\n'
     '        return false;\n'
     '\n'
     '    CampaignShop const& shop = it->second;',
     '    auto Found = m_store.GetShop(vendorEntry);\n'
     '    if (!Found)\n'
     '        return false;\n'
     '\n'
     '    CampaignShop const& shop = *Found;'),
]

done = 0
missing = []
for old, new in pairs:
    if old in s:
        s = s.replace(old, new, 1)
        done += 1
    else:
        missing.append(old.splitlines()[0][:60])

io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('replaced', done, 'of', len(pairs))
for m in missing:
    print('MISSING:', m)

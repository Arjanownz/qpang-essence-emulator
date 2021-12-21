#include "RoomSessionNpcManager.h"

#include "GCPvEHitNpcData.h"
#include "gc_pve_die_npc.hpp"
#include "gc_pve_hit_npc.hpp"
#include "gc_pve_npc_init.hpp"
#include "PatternedCatPveNpc.h"
#include "PveNpc.h"
#include "RoomSession.h"
#include "RoomSessionPlayer.h"

void RoomSessionNpcManager::initialize(const std::shared_ptr<RoomSession>& roomSession)
{
	m_roomSession = roomSession;

	spawnInitialNpcs();
}

void RoomSessionNpcManager::tick(const std::shared_ptr<RoomSession>& roomSession) const
{

}

void RoomSessionNpcManager::spawnInitialNpcs()
{
	// Stage 1.
	const std::vector<std::shared_ptr<PveNpc>> npcs
	{
		std::make_shared<PatternedCatPveNpc>(eNpcType::EASY_PATTERNED_CAT, Position{12.00f, 5.00f, -30.00}, 100),
	};

	for (auto& npc : npcs)
	{
		spawnNpc(npc);
	}
}

uint32_t RoomSessionNpcManager::spawnNpc(const std::shared_ptr<PveNpc>& npc)
{
	const auto roomSession = m_roomSession.lock();

	if (roomSession == nullptr)
	{
		return 0;
	}

	const auto npcUid = (m_npcs.size() + 1);

	npc->setUid(npcUid);

	roomSession->relayPlaying<GCPvENpcInit>(npc->getType(), npcUid, npc->getPosition());

	m_npcs[npcUid] = npc;

	return npcUid;
}

void RoomSessionNpcManager::killNpc(const uint32_t uid)
{
	const auto roomSession = m_roomSession.lock();

	if (roomSession == nullptr)
	{
		return;
	}

	if (const auto npc = findNpcByUid(uid); npc == nullptr)
	{
		return;
	}

	roomSession->relayPlaying<GCPvEDieNpc>(uid);
}

std::shared_ptr<PveNpc> RoomSessionNpcManager::findNpcByUid(const uint32_t uid)
{
	const auto& it = m_npcs.find(uid);

	if (it == m_npcs.end())
	{
		return nullptr;
	}

	return it->second;
}

std::vector<std::shared_ptr<PveNpc>> RoomSessionNpcManager::getAliveNpcs()
{
	std::vector<std::shared_ptr<PveNpc>> aliveNpcs{};

	for (const auto& [id, npc] : m_npcs)
	{
		if (!npc->isDead())
		{
			aliveNpcs.push_back(npc);
		}
	}

	return aliveNpcs;
}

#pragma region Event handlers

void RoomSessionNpcManager::onPlayerSync(const std::shared_ptr<RoomSessionPlayer>& session)
{
	for (const auto& npc : getAliveNpcs())
	{
		session->send<GCPvENpcInit>(npc->getType(), npc->getUid(), npc->getPosition());
	}
}

void RoomSessionNpcManager::onCGPvEHitNpc(const CGPvEHitNpcData& data)
{
	const auto roomSession = m_roomSession.lock();

	if (roomSession == nullptr)
	{
		return;
	}

	const auto playerId = data.roomSessionPlayer->getPlayer()->getId();
	const auto targetNpcUid = data.targetNpc->getUid();

	const auto damageDealt = data.targetNpc->takeDamage(data.weaponUsed.damage);
	const auto hasTargetDied = data.targetNpc->isDead();

	if (hasTargetDied)
	{
		killNpc(targetNpcUid);
	}

	const auto gcPvEHitNpcData = GCPvEHitNpcData
	{
		playerId,
		targetNpcUid,
		data.unk_03,
		data.impactPos,
		data.impactPosOffset,
		data.unk_10,
		data.unk_11,
		data.bodyPartId,
		data.weaponUsed.itemId,
		data.weaponCardId,
		data.weaponType,
		data.hitLocation,
		data.unk_18,
		data.unk_19,
		damageDealt,
		hasTargetDied,
		0
	};

	roomSession->relayPlaying<GCPvEHitNpc>(gcPvEHitNpcData);
}

#pragma endregion

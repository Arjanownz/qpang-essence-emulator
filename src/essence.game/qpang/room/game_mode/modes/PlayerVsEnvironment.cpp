#include "PlayerVsEnvironment.h"

#include "Room.h"
#include "RoomSession.h"

bool PlayerVsEnvironment::isMissionMode()
{
	return true;
}

bool PlayerVsEnvironment::isTeamMode()
{
	return true;
}

void PlayerVsEnvironment::onApply(const std::shared_ptr<Room> room)
{
	room->setIsPointsGame(false);
	room->setScoreTime(10);

	room->setMaxPlayers(4);

	room->setSkillsEnabled(false);

	GameMode::onApply(room);
}

void PlayerVsEnvironment::onStart(const std::shared_ptr<RoomSession> roomSession)
{
	GameMode::onStart(roomSession);
}

void PlayerVsEnvironment::onPlayerSync(const std::shared_ptr<RoomSessionPlayer> session)
{
	// Send all spawned objects/npc/items
	session->getRoomSession()->getObjectManager()->onPlayerSync(session);
	session->getRoomSession()->getNpcManager()->onPlayerSync(session);
	session->getRoomSession()->getPveItemManager()->onPlayerSync(session);

	GameMode::onPlayerSync(session);
}

void PlayerVsEnvironment::tick(const std::shared_ptr<RoomSession> roomSession)
{
	roomSession->getObjectManager()->tick();
	roomSession->getNpcManager()->tick();
	roomSession->getPveItemManager()->tick();

	roomSession->getPveRoundManager()->tick();

	GameMode::tick(roomSession);
}

void PlayerVsEnvironment::onPlayerKill(std::shared_ptr<RoomSessionPlayer> killer, std::shared_ptr<RoomSessionPlayer> target, const Weapon& weapon, uint8_t hitLocation)
{
	// Set can respawn to false, as the player should not suddenly respawn when not selecting a respawn option.
	// This will be overridden in the CGPvERespawnReq packet, when the target makes a choise.
	target->setCanRespawn(false);

	GameMode::onPlayerKill(killer, target, weapon, hitLocation);
}
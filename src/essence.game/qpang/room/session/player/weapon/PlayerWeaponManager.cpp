#include "PlayerWeaponManager.h"

#include "qpang/Game.h"
#include "qpang/room/session/RoomSession.h"

#include "qpang/room/tnl/net_events/server/gc_weapon.hpp"
#include "qpang/room/tnl/net_events/server/gc_respawn.hpp"
#include "qpang/room/tnl/net_events/server/gc_shoot.hpp"
#include "qpang/room/tnl/net_events/server/gc_game_item.hpp"
#include "qpang/room/tnl/net_events/server/gc_essence.hpp"

PlayerWeaponManager::PlayerWeaponManager() :
	m_selectedWeaponIndex(0)
{
}

void PlayerWeaponManager::initialize(std::shared_ptr<RoomSessionPlayer> player)
{
	m_player = player;
	auto* equipmentManager = player->getPlayer()->getEquipmentManager();

	auto itemIds = equipmentManager->getWeaponItemIdsByCharacter(player->getCharacter());
	auto hasPreSelectedWeapon = false;
	auto isMeleeOnly = player->getRoomSession()->getRoom()->isMeleeOnly();

	for (size_t i = 0; i < itemIds.size(); i++)
	{
		auto weapon = Game::instance()->getWeaponManager()->get(itemIds[i]);

		if (isMeleeOnly)
		{
			if (i == 3)
				m_weapons[3] = weapon;
		}
		else
			m_weapons[i] = weapon;

		auto& defaultAmmo = m_defaultAmmo[weapon.itemId];

		defaultAmmo.first = weapon.clipCount + equipmentManager->getExtraAmmoForWeaponIndex(i);
		defaultAmmo.second = weapon.clipSize;

		if (m_weapons[i].itemId != NULL && !hasPreSelectedWeapon)
		{
			m_selectedWeaponIndex = i;
			hasPreSelectedWeapon = true;
		}
	}
}

Weapon PlayerWeaponManager::getSelectedWeapon()
{
	return m_weapons[m_selectedWeaponIndex];
}

void PlayerWeaponManager::selectTagWeapon()
{
	const auto gunIndex = 0;

	m_previousSelectedWeaponIndex = m_selectedWeaponIndex;

	m_selectedWeaponIndex = gunIndex;
	m_currentGunWeapon = m_weapons[m_selectedWeaponIndex];

	const auto chainLightWeapon = Game::instance()->getWeaponManager()->get(1095368720);

	m_weapons[gunIndex] = chainLightWeapon;

	auto& defaultAmmo = m_defaultAmmo[chainLightWeapon.itemId];

	defaultAmmo.first = chainLightWeapon.clipCount;
	defaultAmmo.second = chainLightWeapon.clipSize;

	refillCurrentWeapon();

	switchWeapon(m_weapons[m_selectedWeaponIndex].itemId, false);
}

void PlayerWeaponManager::deselectTagWeapon()
{
	const auto gunIndex = 0;

	m_weapons[gunIndex] = m_currentGunWeapon;
	m_selectedWeaponIndex = m_previousSelectedWeaponIndex;

	switchWeapon(m_weapons[m_selectedWeaponIndex].itemId, false);

	reset();
}

void PlayerWeaponManager::reset()
{
	const auto player = m_player.lock();

	if (player == nullptr)
		return;

	for (auto& weapon : m_weapons)
	{
		auto pair = m_defaultAmmo[weapon.itemId];

		weapon.clipCount = pair.first;
		weapon.clipSize = pair.second;

		player->post(new GCGameItem(14, { {1191182337, 1} }, weapon.itemId));
	}
}

void PlayerWeaponManager::reload(uint32_t seqId)
{
	m_weapons[m_selectedWeaponIndex].clipCount--;
	m_weapons[m_selectedWeaponIndex].clipSize = m_defaultAmmo[getSelectedWeapon().itemId].second;

	if (const auto player = m_player.lock(); player != nullptr)
		player->getRoomSession()->relayPlaying<GCWeapon>(player->getPlayer()->getId(), 3, m_weapons[m_selectedWeaponIndex].itemId, seqId);
}

void PlayerWeaponManager::shoot(uint32_t entityId)
{
	m_weapons[m_selectedWeaponIndex].clipSize--;
}

bool PlayerWeaponManager::canReload()
{
	return m_weapons[m_selectedWeaponIndex].clipCount > 0;
}

bool PlayerWeaponManager::canShoot()
{
	if (isHoldingMelee())
	{
		return true;
	}

	return m_weapons[m_selectedWeaponIndex].clipSize > 0;
}

bool PlayerWeaponManager::hasWeapon(uint32_t weaponId)
{
	return std::find_if(m_weapons.cbegin(), m_weapons.cend(),
		[weaponId](const Weapon& weapon)
		{
			return weapon.itemId == weaponId;
		}
	) != m_weapons.cend();
}

void PlayerWeaponManager::switchWeapon(uint32_t weaponId, bool isReloadGlitchEnabled)
{
	if (!isReloadGlitchEnabled)
	{
		if (m_weapons[m_selectedWeaponIndex].itemId == weaponId)
		{
			// already chose this weapon
			return;
		}
	}

	for (size_t i = 0; i < m_weapons.size(); i++)
	{
		if (m_weapons[i].itemId == weaponId)
		{
			m_selectedWeaponIndex = i;
			break;
		}
	}

	if (const auto player = m_player.lock(); player != nullptr)
	{
		player->getRoomSession()->relayPlaying<GCWeapon>(player->getPlayer()->getId(), 0, m_weapons[m_selectedWeaponIndex].itemId, 0);
		player->post(new GCWeapon(player->getPlayer()->getId(), 5, m_weapons[m_selectedWeaponIndex].itemId, 0));
	}
}

void PlayerWeaponManager::refillWeapon(uint32_t weaponId)
{
	const auto pair = m_defaultAmmo[weaponId];

	m_weapons[m_selectedWeaponIndex].clipCount = pair.first;
	m_weapons[m_selectedWeaponIndex].clipSize = pair.second;

	if (const auto player = m_player.lock(); player != nullptr)
	{
		player->post(new GCGameItem(
			14,
			std::vector<GCGameItem::Item>({ GCGameItem::Item{1191182337, 0x01, 0x00, 0x00, 0x00} }),
			weaponId
		));
	}
}

void PlayerWeaponManager::refillCurrentWeapon()
{
	const auto itemId = getSelectedWeapon().itemId;

	refillWeapon(itemId);
}

std::array<uint32_t, 4> PlayerWeaponManager::getWeaponIds()
{
	std::array<uint32_t, 4> weaponIds;

	for (size_t i = 0; i < m_weapons.size(); i++)
		weaponIds[i] = m_weapons[i].itemId;

	return weaponIds;
}

std::array<Weapon, 4> PlayerWeaponManager::getWeapons()
{
	return m_weapons;
}

bool PlayerWeaponManager::isHoldingMelee()
{
	return m_selectedWeaponIndex == 3;
}


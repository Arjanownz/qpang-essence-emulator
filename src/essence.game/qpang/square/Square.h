#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <mutex>

#include "qpang/square/SquarePlayer.h"
#include "packets/SquareServerPacket.h"

#include "qpang/Game.h"
#include "qpang/square/SquareManager.h"
#include "qpang/square/SquarePlayer.h"

#include "packets/square/outgoing/SendJoinSquareSuccess.h"
#include "packets/square/outgoing/SendAddSquarePlayer.h"
#include "packets/square/outgoing/SendSquareRemovePlayer.h"
#include "packets/square/outgoing/SendUpdateSquareEntry.h"

class Player;

class Square : public std::enable_shared_from_this<Square>
{
public:
	using Ptr = std::shared_ptr<Square>;
	using Ptr = std::shared_ptr<Player>;


	Square(uint32_t id, std::u16string name, uint8_t capacity = 50);

	const uint32_t& GetRemoteIP(Player::Ptr& player);

	const uint32_t& isPlayerIdValid(Player::Ptr& player);

	bool add(std::shared_ptr<Player> player);

	void remove(uint32_t player);

	void sendPacket(const SquareServerPacket& packet);
	void sendPacketExcept(const SquareServerPacket& packet, uint32_t playerId);

	void setClosed(bool isClosed);

	std::vector<SquarePlayer::Ptr> listPlayers();
	SquarePlayer::Ptr getSquarePlayer(uint32_t playerId);

	uint32_t getId();
	std::u16string getName();
	uint8_t getCapacity();
	uint8_t getPlayerCount();
	uint8_t getState();
	bool isFull();
	bool isClosed();
private:

	std::unordered_map<uint32_t, SquarePlayer::Ptr> m_players;
	std::recursive_mutex m_playerMx;

	const uint32_t m_id;
	std::u16string m_name;
	uint8_t m_capacity;
	uint8_t m_state;
	bool m_isClosed;
	std::string getRemoteIP();
	bool isPlayerIdValid(uint32_t playerId);
	bool IsConnectedToLobby(Player::Ptr& player)
	{
		return isConnectedToLobby(player);
	}
	bool isConnectedToLobby(Player::Ptr& player);
	bool hasLobbyConnection(std::shared_ptr<Player> player);
	bool getRemoteIP(Player::Ptr& player);
	bool isSameRemoteIP(std::shared_ptr<Player> player);
	bool isInChannel(std::shared_ptr<Player> player);
	void setupConnection(Player::Ptr& player);
	void setupPlayerConnection(std::shared_ptr<Player> player);
};

#pragma once
#include "Square.h"

#include <future>
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <mutex>
#include <unordered_map>

#include "qpang/Game.h"
#include "qpang/square/SquareManager.h"
#include "qpang/square/SquarePlayer.h"

#include "packets/square/outgoing/SendJoinSquareSuccess.h"
#include "packets/square/outgoing/SendAddSquarePlayer.h"
#include "packets/square/outgoing/SendSquareRemovePlayer.h"
#include "packets/square/outgoing/SendUpdateSquareEntry.h"

Square::Square(uint32_t id, std::u16string name, uint8_t capacity) :
    m_id(id),
    m_name(std::move(name)),
    m_capacity(capacity),
    m_state(8),
    m_isClosed(false)
{
}


const uint32_t& Square::GetRemoteIP(Player::Ptr& player)
{
    return getRemoteIP(player);
}

const uint32_t& Square::isPlayerIdValid(Player::Ptr& player)
{
    return player->getId();
}

bool Square::add(std::shared_ptr<Player> player)
{
    assert(player != nullptr);

    if (!Square::isPlayerIdValid(player->getId()) || !Square::hasLobbyConnection(player) ||
        !Square::isSameRemoteIP(player) || !Square::isInChannel(player)) {
        std::cerr << "Connection checks failed for player ID: " << Square::isPlayerIdValid(player) << std::endl;
        return false; // Discard connection
    }

    if (getPlayerCount() >= m_capacity)
        return false;

    if (player->getRank() == 1 && m_isClosed)
        return false;

    const auto curr = shared_from_this();

    const auto squarePlayer = std::make_shared<SquarePlayer>(player, curr);

    m_playerMx.lock();
    m_players[player->getId()] = squarePlayer;
    m_playerMx.unlock();

    Game::instance()->getSquareManager()->broadcast(SendUpdateSquareEntry(curr, true));

    player->enterSquare(squarePlayer);

    sendPacketExcept(SendAddSquarePlayer(squarePlayer), player->getId());

    return true;
}

void Square::remove(uint32_t playerId)
{
    m_playerMx.lock();
    m_players.erase(playerId);
    m_playerMx.unlock();

    sendPacket(SendSquareRemovePlayer(playerId));

    const auto curr = shared_from_this();
    Game::instance()->getSquareManager()->broadcast(SendUpdateSquareEntry(curr, true));

    if (m_players.empty())
        Game::instance()->getSquareManager()->close(m_id);
}

void Square::sendPacket(const SquareServerPacket & packet)
{
    std::lock_guard<std::recursive_mutex> lg(m_playerMx);

    for (const auto& [id, sqPlayer] : m_players)
        sqPlayer->getPlayer()->send(packet);
}

void Square::sendPacketExcept(const SquareServerPacket & packet, uint32_t playerId)
{
    std::lock_guard<std::recursive_mutex> lg(m_playerMx);

    for (const auto& [id, sqPlayer] : m_players)
        if (sqPlayer->getPlayer()->getId() != playerId)
            sqPlayer->getPlayer()->send(packet);
}

void Square::setClosed(bool isClosed)
{
    m_isClosed = isClosed;
}

std::vector<SquarePlayer::Ptr> Square::listPlayers()
{
    std::lock_guard<std::recursive_mutex> lg(m_playerMx);

    std::vector<SquarePlayer::Ptr> players;

    for (const auto& [id, player] : m_players)
        players.push_back(player);

    return players;
}

SquarePlayer::Ptr Square::getSquarePlayer(uint32_t playerId)
{
    std::lock_guard<std::recursive_mutex> lg(m_playerMx);

    auto it = m_players.find(playerId);
    return it != m_players.cend() ? (*it).second : nullptr;
}

uint32_t Square::getId()
{
    return m_id;
}

std::u16string Square::getName()
{
    return m_name;
}

uint8_t Square::getCapacity()
{
    return m_capacity;
}

uint8_t Square::getPlayerCount()
{
    return m_players.size();
}

uint8_t Square::getState()
{
    return m_state;
}

bool Square::isFull()
{
    return m_players.size() >= m_capacity;
}

bool Square::isClosed()
{
    return m_isClosed;
}




// New functions added for player validation
bool Square::isPlayerIdValid(uint32_t playerId)
{
    return m_players.find(playerId) != m_players.end();
}

bool Square::isConnectedToLobby(Player::Ptr& player)
{
    return IsConnectedToLobby(player);
}

bool Square::hasLobbyConnection(std::shared_ptr<Player> player)
{
    // Implement logic to check if the player has a lobby connection
    return isConnectedToLobby(player);
}

bool Square::getRemoteIP(Player::Ptr& player)
{
    return getRemoteIP(player);
}

bool Square::isSameRemoteIP(std::shared_ptr<Player> player)
{
    // Implement logic to check if the player is from the same remote IP address
    return getRemoteIP(player);
}


void Square::setupConnection(Player::Ptr& player)
{
    return setupConnection(player);
}

void Square::setupPlayerConnection(std::shared_ptr<Player> player)
{
    // Implement logic to set up the player connection
    setupConnection(player);
}

std::string Square::getRemoteIP()
{
    // Implement logic to retrieve the remote IP address of the square
    return "127.0.0.1"; // Placeholder
}

#pragma once

#include "core/communication/packet/PacketEvent.h"
#include <packets/lobby/outgoing/trading/SendReceiveTradeRequestResponse.h>
#include <packets/lobby/outgoing/trading/SendReceiveTradeRequestError.h>
#include <packets/lobby/outgoing/trading/SendOpenTradeWindow.h>

class HandleReceiveTradeRequest final : public PacketEvent
{
public:
	enum TradeRequestErrorCode {
		ALREADY_TRADING = 380,
		DOES_NOT_WANT_TRADE_OFFERS = 877
	};

	void handle(const QpangConnection::Ptr conn, QpangPacket& packet) override
	{
		const auto player = conn->getPlayer();
		const auto playerId = player->getId();

		const auto targetPlayerId = packet.readInt();

		packet.readEmpty(4);

		const auto targetPlayer = Game::instance()->getOnlinePlayer(targetPlayerId);

		if (targetPlayer == nullptr)
		{
			conn->send(SendReceiveTradeRequestError(0));

			return;
		}

		TradeManager* tradeManager = Game::instance()->getTradeManager();

		// 1 = true, 2 = false.
		const auto hasAcceptedTradeRequestValue = packet.readByte();
		const auto hasAcceptedTradeRequest = (hasAcceptedTradeRequestValue == 1);

		if (!hasAcceptedTradeRequest)
		{
			targetPlayer->send(SendReceiveTradeRequestResponse(playerId, hasAcceptedTradeRequestValue));

			return;
		}

		// Check if the player you are going to trade with is already trading.
		if (tradeManager->isTrading(targetPlayerId))
		{
			conn->send(SendReceiveTradeRequestError(ALREADY_TRADING));

			return;
		}

		// Start the trading session.
		tradeManager->startTradeSession(targetPlayerId, playerId, false);
		tradeManager->startTradeSession(playerId, targetPlayerId, false);

		// Send the accept trade and open trade window packet to the trading buddy.
		targetPlayer->send(SendReceiveTradeRequestResponse(playerId, hasAcceptedTradeRequestValue));
		targetPlayer->send(SendOpenTradeWindow(playerId, hasAcceptedTradeRequestValue));

		// Send the accept trade and open trade window to the player.
		conn->send(SendReceiveTradeRequestResponse(targetPlayerId, hasAcceptedTradeRequestValue));
		conn->send(SendOpenTradeWindow(targetPlayerId, hasAcceptedTradeRequestValue));
	}
};
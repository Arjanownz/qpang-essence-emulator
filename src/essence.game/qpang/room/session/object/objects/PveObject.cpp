#include "PveObject.h"

#include "RoomSession.h"

#include <qpang/room/tnl/net_events/server/gc_pve_object_init.hpp>
#include <qpang/room/tnl/net_events/server/gc_pve_object_move.hpp>

PveObject::PveObject(uint32_t uid, eObjectType type, const Position& position, uint32_t linkedObjectId)
{
	m_uid = uid;
	m_type = type;
	m_position = position;
	m_linkedObjectId = linkedObjectId;
}

eObjectType PveObject::getType() const
{
	return m_type;
}

Position PveObject::getPosition() const
{
	return m_position;
}

void PveObject::tick(const std::shared_ptr<RoomSession>& roomSession)
{

}

void PveObject::onEvent(const std::shared_ptr<RoomSession>& roomSession)
{
	// Default is 0, not all objects have a linked object
	// TODO: would be more proper to check if the linked object is a gate, before opening
	if (m_linkedObjectId)
		roomSession->getObjectManager()->openGate(m_linkedObjectId);
}
#pragma once
#include "Packet.h"
#include <queue>
#include <mutex>
#include <memory>

class PacketManager
{
private:
	std::queue<std::shared_ptr<Packet>> m_queue_packets;
	std::mutex m_mutex_packets;
public:
	void Clear();
	bool HasPendingPackets();
	void Append(std::shared_ptr<Packet> p);
	std::shared_ptr<Packet> Retrieve();
};
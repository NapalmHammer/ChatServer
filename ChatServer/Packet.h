#pragma once
#include "PacketType.h"
#include <cstdint>
#include <vector>
#include <memory>
class Packet
{
public:
	Packet();
	Packet(const char* _buffer, const int _size);

	Packet(const PacketType _p);
	Packet(const std::shared_ptr<Packet> _p);
	void Append(const std::shared_ptr<Packet> _p);
	void Append(const PacketType _p);
	void Append(const std::int32_t int32_);
	void Append(const std::size_t _p);
	void Append(const Packet& p);
	void Append(const std::string& _str);
	void Append(const char* _buffer, const int _size);

	std::vector<int8_t> m_buffer;
};
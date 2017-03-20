#include "Server.h"
#include "PacketStructs.h"
#pragma comment(lib,"ws2_32.lib") //Required for WinSock

bool Server::GetAll(std::shared_ptr<Connection> _connection, char* _data, int _totalBytes)
{
	int bytesReceived = 0;
	while (bytesReceived < _totalBytes)
	{
		int retnCheck = recv(_connection->m_socket, _data + bytesReceived, _totalBytes - bytesReceived, NULL);
		if (retnCheck == SOCKET_ERROR || retnCheck == 0)
			return false;
		bytesReceived += retnCheck;
	}
	return true;
}

bool Server::SendAll(std::shared_ptr<Connection> _connection, const char* _data, const int _totalBytes)
{
	int bytesSent = 0;
	while (bytesSent < _totalBytes)
	{
		int retnCheck = send(_connection->m_socket, _data + bytesSent, _totalBytes - bytesSent, NULL);
		if (retnCheck == SOCKET_ERROR)
			return false;
		bytesSent += retnCheck;
	}
	return true;
}

bool Server::GetInt32_t(std::shared_ptr<Connection> _connection, std::int32_t &_int32_t)
{
	if (!GetAll(_connection, (char*)&_int32_t, sizeof(std::int32_t)))
		return false;
	_int32_t = ntohl(_int32_t); // - Convert from network-byte-order to host-byte-order
	return true; 
}

bool Server::GetPacketType(std::shared_ptr<Connection> _connection, PacketType &_packettype)
{
	int packettype_int;
	if (!GetInt32_t(_connection, packettype_int))
		return false; 
	_packettype = (PacketType)packettype_int;
	return true;
}

void Server::SendString(std::shared_ptr<Connection> _connection, const std::string &_string)
{
	PS::ChatMessage message(_string);
	_connection->m_pm.Append(message.toPacket());
}

bool Server::GetString(std::shared_ptr<Connection> _connection, std::string &_string)
{
	std::int32_t bufferlength;
	if (!GetInt32_t(_connection, bufferlength))
		return false; 
	if (bufferlength == 0) return true;
	_string.resize(bufferlength); //resize string to fit message
	return GetAll(_connection, &_string[0], bufferlength);
}

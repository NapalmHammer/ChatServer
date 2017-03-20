#pragma once

#include <winsock2.h>
#include <string>
#include "FileTransferData.h"
#include "PacketManager.h"
#include <vector>
#include <shared_mutex>

class Connection
{
public:
	Connection(SOCKET socket)
		:m_socket(socket)
	{
	}
	SOCKET m_socket;
	FileTransferData m_file;
	PacketManager m_pm;
	std::string m_Name;
	int m_ID = 0;
};

class Server
{
public:
	Server(int _PORT, bool _LoopBackToLocalHost = false, bool _takingNewUsers = false);
	~Server();
	bool ListenForNewConnection();
private:
	bool SendAll(std::shared_ptr<Connection> _connection, const char* _data, const int _totalBytes);
	bool GetAll(std::shared_ptr<Connection> _connection, char* data, int totalBytes);
	bool GetInt32_t(std::shared_ptr<Connection> _connection, int32_t &_int32_t);
	bool GetPacketType(std::shared_ptr<Connection> _connection, PacketType &_packettype);
	void SendString(std::shared_ptr<Connection> _connection, const std::string& _string);
	bool GetString(std::shared_ptr<Connection> _connection, std::string &_string);
	bool ProcessPacket(std::shared_ptr<Connection> _connection, PacketType _packettype);
	//-----
	bool RequestFile(std::shared_ptr<Connection> _connection, std::string FileName);
	//-----
	static void ClientHandlerThread(Server& _server, std::shared_ptr<Connection> _connection);
	static void PacketSenderThread(Server& server);	
	void DisconnectClient(std::shared_ptr<Connection> _connection);
private:
bool acceptingUsers;
std::vector<std::shared_ptr<Connection>> m_connections;
std::shared_mutex m_mutex_connectionMgr;
int m_IDCounter = 0;
SOCKADDR_IN m_addr; 
SOCKET m_sListen;
bool m_terminateThreads = false;
std::vector<std::thread*> m_threads; 
};
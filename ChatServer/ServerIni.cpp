#include "Server.h"
#include <WS2tcpip.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib") //Required for WinSock

// Edits for GitHub testing

Server::Server(int _PORT, bool _LoopBackToLocalHost, bool _takingNewUsers)
{
	// Winsock startup
	WSADATA wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		MessageBoxA(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	if (_LoopBackToLocalHost) //--------------------------- If constructor argument "BroadcastPublically" is == true
		inet_pton(AF_INET, "127.0.0.1", &m_addr.sin_addr.s_addr); //- Broadcast locally 
	else
		m_addr.sin_addr.s_addr = htonl(INADDR_ANY); //--- Broacasting publicly

	m_addr.sin_port = htons(_PORT);
	m_addr.sin_family = AF_INET; //----------- using IPv4

	m_sListen = socket(AF_INET, SOCK_STREAM, 0);

	if (bind(m_sListen, (SOCKADDR*)&m_addr, sizeof(m_addr)) == SOCKET_ERROR)
	{
		std::string ErrorMsg = "Failed to bind the address to our listening socket. Winsock error is: " + std::to_string(WSAGetLastError());
		MessageBoxA(0, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}
	if (listen(m_sListen, SOMAXCONN) == SOCKET_ERROR) //-------------------------- Places sListen socket in listening state. SOMAXCONN = Socket Outstanding Max connections
	{
		std::string ErrorMsg = "Failed to listen on socket. Winsock error is: " + std::to_string(WSAGetLastError());
		MessageBoxA(0, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}
	m_IDCounter = 0;
	std::thread PST(PacketSenderThread, std::ref(*this));
	PST.detach();
	m_threads.push_back(&PST);
	acceptingUsers = _takingNewUsers;
}

bool Server::ListenForNewConnection()
{
	int addrlen = sizeof(m_addr);
	SOCKET newConnectionSocket = accept(m_sListen, (SOCKADDR*)&m_addr, &addrlen);
	if (newConnectionSocket == 0)
	{
		std::cout << "Failed to accept the client's connection" << std::endl;
		return false;
	}
	else
	{
		std::lock_guard<std::shared_mutex> lock(m_mutex_connectionMgr);
		std::shared_ptr<Connection> newConnection(std::make_shared<Connection>(newConnectionSocket));
		m_connections.push_back(newConnection);
		newConnection->m_ID = m_IDCounter;
		newConnection->m_Name = std::to_string(m_IDCounter);
		m_IDCounter++;
		std::cout << "Client connected! ID:" << newConnection->m_ID << std::endl;
		std::thread CHT(ClientHandlerThread, std::ref(*this), newConnection);
		CHT.detach();
		m_threads.push_back(&CHT);
		return true;
	}
}

Server::~Server()
{
	m_terminateThreads = true;
	for (std::thread * t : m_threads)
	{
		t->join();
	}
}
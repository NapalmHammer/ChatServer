#include "Server.h"
#include <iostream>
#include "PacketStructs.h"
#pragma comment(lib,"ws2_32.lib") //Required for WinSock

void Server::ClientHandlerThread(Server & _server, std::shared_ptr<Connection> _connection) //ID = the index in the SOCKET connections array
{
	PacketType packettype;
	while (true)
	{
		if (_server.m_terminateThreads == true)
			break;
		if (!_server.GetPacketType(_connection, packettype)) //Get packet type
			break; //If there is an issue getting the packet type, exit this loop
		if (!_server.ProcessPacket(_connection, packettype)) //Process packet (packet type)
			break; //If there is an issue processing the packet, exit this loop
	}
	std::cout << "Lost connection to client ID: " << _connection->m_ID << std::endl;
	_server.DisconnectClient(_connection); //Disconnect this client and clean up the connection if possible
	return;
}

bool Server::RequestFile(std::shared_ptr<Connection> _connection, std::string _fileName)
{
	_connection->m_file.m_outfileStream.open(_fileName, std::ios::binary);
	_connection->m_file.m_fileName = _fileName;
	_connection->m_file.m_bytesWritten = 0;
	if (!_connection->m_file.m_outfileStream.is_open())
	{
		std::cout << "ERROR: Function(Server::Requestfile) - Unable to open file: " << _fileName << " for writing.\n";
		return false;
	}
	std::cout << "ID: " << _connection->m_ID << " Requesting to store file " << _fileName << " on server." << std::endl;

	Packet requestFilePacket;
	requestFilePacket.Append(PacketType::FileTransferRequestFile);
	requestFilePacket.Append(_connection->m_file.m_fileName.size());
	requestFilePacket.Append(_connection->m_file.m_fileName.c_str(), _connection->m_file.m_fileName.size());
	_connection->m_pm.Append(std::make_shared<Packet>(requestFilePacket));
	std::cout << "Receiving file: " << _fileName << std::endl;
	return true;
}

void Server::PacketSenderThread(Server & server) //Thread for all outgoing packets
{
	while (true)
	{
		if (server.m_terminateThreads == true)
			break;
		std::shared_lock<std::shared_mutex> lock(server.m_mutex_connectionMgr);
		for (auto conn : server.m_connections) //for each connection...
		{
			if (conn->m_pm.HasPendingPackets()) //If there are pending packets for this connection's packet manager
			{
				std::shared_ptr<Packet> p = conn->m_pm.Retrieve(); //Retrieve packet from packet manager
				if (!server.SendAll(conn, (const char*)(&p->m_buffer[0]), p->m_buffer.size())) //send packet to connection
				{
					std::cout << "Failed to send packet to ID: " << conn->m_ID << std::endl; //Print out if failed to send packet
				}
			}
		}
		Sleep(5);
	}
	std::cout << "Ending Packet Sender Thread..." << std::endl;
}

void Server::DisconnectClient(std::shared_ptr<Connection> _connection) //Disconnects a client and cleans up socket if possible
{
	std::lock_guard<std::shared_mutex> lock(m_mutex_connectionMgr); //Lock connection manager mutex since we are possible removing element(s) from the vector
	_connection->m_pm.Clear(); //Clear out all remaining packets in queue for this connection
	closesocket(_connection->m_socket); //Close the socket for this connection
	m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), _connection)); //Remove connection from vector of connections
	std::cout << "Cleaned up client: " << _connection->m_ID << "." << std::endl;
	std::cout << "Total connections: " << m_connections.size() << std::endl;
}
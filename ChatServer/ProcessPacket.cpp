#include "Server.h"
#include <iostream>
#include "PacketStructs.h"
#pragma comment(lib, "ws2_32.lib")

bool Server::ProcessPacket(std::shared_ptr<Connection> _connection, PacketType _packettype)
{
	switch (_packettype)
	{
	case PacketType::ChatMessage:
	{
		std::string message;
		if (!GetString(_connection, message))
			return false;

		PS::ChatMessage cm(_connection->m_Name + message);
		std::shared_ptr<Packet> msgPacket = std::make_shared<Packet>(cm.toPacket());
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex_connectionMgr);
			for (auto conn : m_connections)
			{
				if (conn == _connection)
					continue;
				conn->m_pm.Append(msgPacket);
			}
		}
		std::cout << "Processed chat message from ID: " << _connection->m_ID << " Sent message: " << message << std::endl;
		break;
	}
	case PacketType::FileTransferByteBuffer:
	{
		int32_t buffersize;
		if (!GetInt32_t(_connection, buffersize))
		{
			std::cout << "Uh oh: GETINT32, BUFFER" << std::endl;
			return false;
		}
		if (!GetAll(_connection, _connection->m_file.m_buffer, buffersize))
		{
			std::cout << "Uh oh: GETALL, BUFFER-BUFFERSIZE" << std::endl;
			return false;
		}
		_connection->m_file.m_outfileStream.write(_connection->m_file.m_buffer, buffersize);
		_connection->m_file.m_bytesWritten += buffersize;
		std::cout << buffersize << " Bytes received." << std::endl;
		_connection->m_pm.Append(std::make_shared<Packet>(Packet(PacketType::FileTransferRequestNextBuffer)));
		break;
	}
	case PacketType::FileTransferRequestFile:
	{
		std::string fileName;
		if (!GetString(_connection, fileName))
			return false;
		std::string errMsg;
		std::cout << "File name Received: " << fileName << std::endl;
		std::cout << "ID: " << _connection->m_ID << " Requesting to get file " << fileName << " on server." << std::endl;
		if (_connection->m_file.Initialize(fileName, errMsg)) //if filetransferdata successfully initialized
		{
			_connection->m_pm.Append(_connection->m_file.getOutgoingBufferPacket()); //Send first buffer from file
		}
		else //If initialization failed, send error message
		{
			SendString(_connection, errMsg);
			std::cout << errMsg << std::endl;
			std::cout << "Cancelling file send" << std::endl;			
			_connection->m_pm.Append(std::make_shared<Packet>(Packet(PacketType::CancelFileSend)));
		}
		break;
	}
	case PacketType::FileTransferRequestNextBuffer:
	{
		if (_connection->m_file.m_transferInProgress)
		{
			_connection->m_pm.Append(_connection->m_file.getOutgoingBufferPacket()); //get and send next buffer for file
		}
		break;
	}
	case PacketType::FileTransfer_EndOfFile:
	{
		std::cout << "File transfer completed, file received." << std::endl;
		std::cout << "File Name: " << _connection->m_file.m_fileName << std::endl;
		std::cout << "File size(bytes): " << _connection->m_file.m_bytesWritten << std::endl;
		_connection->m_file.m_bytesWritten = 0;
		_connection->m_file.m_outfileStream.close();
		break;
	}
	case PacketType::ClientSendFileRequest:
	{
		std::string FileName = "0";
		if (!GetString(_connection, FileName))
			return false;
		std::cout << FileName << std::endl;
		if (!RequestFile(_connection, FileName))
		{
			std::cout << "Uh oh: RequestFile" << std::endl;
			return false;
		}
		break;
	}
	case PacketType::ClientSetName:
	{
		GetString(_connection, _connection->m_Name);
		std::cout << "client " + std::to_string(_connection->m_ID) + " set their name to: " + _connection->m_Name << std::endl;
		break;
	}
	default:
		std::cout << "Unrecognizable packet: " << (int32_t)_packettype << std::endl;
		break;
	}
	return true;
}
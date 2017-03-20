#pragma once
#include "PacketType.h"
#include "Packet.h"
#include "FileTransferData.h"
#include <string>

namespace PS
{
	class ChatMessage
	{
	public:
		ChatMessage(const std::string _msg);
		std::shared_ptr<Packet> toPacket();
	private:
		std::string m_message;
	};

	class FileTransferRequestFile
	{
	public:
		FileTransferRequestFile(const std::string _fileName);
		std::shared_ptr<Packet> toPacket();
	private:
		std::string m_fileName;
	};

	class FileDataBuffer
	{
	public:
		std::shared_ptr<Packet> toPacket();
		char m_databuffer[FileTransferData::m_bufferSize];
		int m_size;
	};
}
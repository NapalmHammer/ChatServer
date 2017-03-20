#include "PacketStructs.h"
#include <Windows.h>

namespace PS
{
	ChatMessage::ChatMessage(const std::string _msg)
		:m_message(_msg)
	{
	}

	std::shared_ptr<Packet> ChatMessage::toPacket()
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(PacketType::ChatMessage);
		p->Append(m_message.size());
		p->Append(m_message);
		return p;
	}

	std::shared_ptr<Packet> FileDataBuffer::toPacket()
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(PacketType::FileTransferByteBuffer);
		p->Append(m_size);
		p->Append(m_databuffer, m_size);
		return p;
	}
	FileTransferRequestFile::FileTransferRequestFile(const std::string _fileName)
		:m_fileName(_fileName)
	{
	}
	std::shared_ptr<Packet> FileTransferRequestFile::toPacket()
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(PacketType::FileTransferRequestFile);
		p->Append(std::int32_t(htonl(m_fileName.size())));
		p->Append(m_fileName);
		return p;
	}
}

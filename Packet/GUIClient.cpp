#include "pch.h"
#include "GUIClient.h"
#include "Router.h"

GUIClient::GUIClient(const std::wstring& serverIP, uint16_t serverPort) :UDPClient(serverIP, serverPort)
{
}

GUIClient::~GUIClient()
{
	m_running = false;
	if (m_pingThread.joinable()) {
		m_pingThread.join();
	}
}

#pragma region Send
void GUIClient::Ping(uint32_t pid)
{
	m_pingThread = std::thread([this, pid]() {
		while (m_running) {
			std::vector<uint8_t> buffer;
			PacketScript::EncodeOp(buffer, Opcode::CP_Ping);
			PacketScript::Encode4(buffer, pid);
			bool ok = this->SendBuffer(buffer.data(), buffer.size());
			if (!ok) {
				DEBUG(L"Failed to ping");
			}
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
		});
}

void GUIClient::SendPacketInfo(PacketInfo& info)
{
	std::vector<uint8_t> buffer;
	PacketScript::EncodeOp(buffer, Opcode::CP_SendPacketInfo);
	info.Serialize(buffer);
	bool ok = this->SendBuffer(buffer.data(), buffer.size());
	if (!ok) {
		DEBUG(L"Failed to send packet info");
		return;
	}
}
#pragma endregion


#pragma region Receive
void GUIClient::HandleDatagram(const void* buf, size_t bufLen, const std::string& clientIP, uint16_t clientPort)
{
	const uint8_t* data = static_cast<const uint8_t*>(buf);
	std::vector<uint8_t> buffer(data, data + bufLen);
	if (buffer.size() >= 2) {
		size_t pos = 0;
		Opcode op = PacketScript::DecodeOp(buffer, pos);
		switch (op) {
		case Opcode::LP_SendPacketInfo: {
			PacketInfo info{};
			info.Deserialize(buffer, pos);
			handlePacketInfo(info);
			break;
		}
		case Opcode::LP_UpdateFilterOpcodeSet: {
			FilterOpcodeSet set{};
			set.Deserialize(buffer, pos);
			handleFilterOpcodesSet(set);
			break;
		}
		default:
			DEBUG(L"Unknown opcode received " + std::to_wstring(static_cast<uint16_t>(op)));
			break;
		}
	}
	else {
		DEBUG(L"Invaild udp packet size");
	}

}

void GUIClient::handlePacketInfo(PacketInfo& info)
{
	if (info.IsInPacket) {
		InPacket iPacket{};
		iPacket.m_nState = 2;
		iPacket.m_uDataLen = static_cast<uint16_t>(info.Payload.size());
		iPacket.m_uLength = Router::kHeaderLength + iPacket.m_uDataLen;
		iPacket.m_aRecvBuff = new uint8_t[iPacket.m_uLength];
		iPacket.m_uOffset = Router::kHeaderLength;
		memcpy(iPacket.m_aRecvBuff + Router::kHeaderLength, info.Payload.data(), info.Payload.size());
		Router::ProcessPacket(&iPacket);
		delete[] iPacket.m_aRecvBuff;
	}
	else {
#ifdef _WIN64
		OutPacket oPacket{};
		oPacket.m_uMaxBufferSize = 0x100;
		oPacket.m_uHeader = static_cast<uint16_t>(info.Payload[0] | info.Payload[1] << 8);
		oPacket.m_uOffset = static_cast<uint32_t>(info.Payload.size());
		memcpy(oPacket.m_aSendBuff, info.Payload.data(), info.Payload.size());
		COutPacket::SetActions(&oPacket, info.Actions);
		Router::SendPacket(&oPacket);
#else
		OutPacket oPacket{};
		oPacket.m_aSendBuff = new uint8_t[info.Payload.size()];
		oPacket.m_uOffset = static_cast<uint32_t>(info.Payload.size());
		memcpy(oPacket.m_aSendBuff, info.Payload.data(), info.Payload.size());
		COutPacket::SetActions(&oPacket, info.Actions);
		Router::SendPacket(&oPacket);
		delete[] oPacket.m_aSendBuff;
#endif
	}

}
void GUIClient::handleFilterOpcodesSet(FilterOpcodeSet& set)
{
	CInPacket::UpdateFilterOpcodeSet(set.CInPacketFilterOpcodes);
	COutPacket::UpdateFilterOpcodeSet(set.COutPacketFilterOpcodes);
}
#pragma endregion
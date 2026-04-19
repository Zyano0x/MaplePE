#include "pch.h"
#include "UDPServer.h"
#include "GUIServer.h"

GUIServer::GUIServer(IMainController* mainControllerImpl) :UDPServer()
{
	m_mainControllerImpl = mainControllerImpl;
	const Setting s = mainControllerImpl->GetSetting();
	this->Bind(s.GUIServerIP, static_cast<uint16_t>(s.GUIServerPort));
}

GUIServer::~GUIServer()
{
}

#pragma region Send

std::wstring GUIServer::sendPacket(uint32_t pid, std::vector<uint8_t> buffer)
{
	auto it = m_clientEndpointMap.find(pid);
	if (it == m_clientEndpointMap.end()) {
		return L"No client found for this PID " + std::to_wstring(pid);
	}
	const ClientEndpoint& clientEndpoint = it->second;
	bool ok = this->Notify(buffer.data(), buffer.size(), clientEndpoint.IP, clientEndpoint.Port);
	if (!ok) {
		m_clientEndpointMap.erase(pid);
		return L"Failed to send packet to PID " + std::to_wstring(pid);
	}
	return std::wstring();
}

std::wstring GUIServer::broadcastPacket(std::vector<uint8_t> buffer)
{
	if (m_clientEndpointMap.size() == 0) {
		return L"No active clients";
	}
	std::wstring err;
	std::vector<uint32_t> pids;
	for (const auto& pair : m_clientEndpointMap) {
		pids.push_back(pair.first);
	}
	for (uint32_t pid : pids) {
		std::wstring result = sendPacket(pid, buffer);
		if (!result.empty()) {
			err += result + L"\n";
		}
	}
	if (!err.empty()) {
		return err;
	}

	return std::wstring();
}

std::vector<uint8_t> GUIServer::Log2Buffer(const PacketLogModel& log)
{
	PacketInfo info{};
	info.PID = log.GetPID();
	info.IsInPacket = log.IsInPacket();
	info.Actions = log.GetActions();
	bool ok = PacketScript::Data2Buffer(log.GetData(), info.Payload);
	if (!ok) {
		return {};
	}
	std::vector<uint8_t> buffer;
	PacketScript::EncodeOp(buffer, Opcode::LP_SendPacketInfo);
	info.Serialize(buffer);
	return buffer;
}

std::wstring GUIServer::SendPacketInfo(const PacketLogModel& log)
{
	if (log.IsEmpty()) {
		return L"Invalid PID";
	}
	std::vector<uint8_t> buffer = Log2Buffer(log);
	if (buffer.empty()) {
		return L"Failed to send packet info";
	}
	return sendPacket(log.GetPID(), Log2Buffer(log));
}

std::wstring GUIServer::BroadcastPacketInfo(const PacketLogModel& log)
{
	if (!log.IsEmpty()) {
		return L"PID must be 0 if broadcasting";
	}
	std::vector<uint8_t> buffer = Log2Buffer(log);
	if (buffer.empty()) {
		return L"Failed to broadcast packet info";
	}
	return broadcastPacket(buffer);
}

std::wstring GUIServer::BroadcastFilterOpcodes(const std::wstring& inOpcodes, const std::wstring& outOpcodes)
{
	FilterOpcodeSet set{ inOpcodes,outOpcodes };
	std::vector<uint8_t> buffer;
	PacketScript::EncodeOp(buffer, Opcode::LP_UpdateFilterOpcodeSet);
	set.Serialize(buffer);
	return broadcastPacket(buffer);
}
#pragma endregion

#pragma region Receive
void GUIServer::HandleDatagram(const void* buf, size_t bufLen, const std::string& clientIP, uint16_t clientPort)
{
	const uint8_t* data = static_cast<const uint8_t*>(buf);
	std::vector<uint8_t> buffer(data, data + bufLen);
	size_t pos = 0;
	Opcode op = PacketScript::DecodeOp(buffer, pos);
	switch (op) {
	case Opcode::CP_Ping: {
		uint32_t pid = PacketScript::Decode4(buffer, pos);
		m_clientEndpointMap[pid] = ClientEndpoint{
			clientIP,
			clientPort,
		};
		break;
	}
	case Opcode::CP_SendPacketInfo: {
		PacketInfo info{};
		info.Deserialize(buffer, pos);
		handlePacketInfo(info);
		break;
	}
	default:
		std::wstring err = L"Unknown opcode received " + std::to_wstring(static_cast<uint16_t>(op));
		MessageBoxW(NULL, err.c_str(), L"Protocol Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
		break;
	}
}

void GUIServer::handlePacketInfo(PacketInfo& info)
{
	PacketLogModel log(info);
	m_mainControllerImpl->OnPacketLogModel(log);
}

#pragma endregion
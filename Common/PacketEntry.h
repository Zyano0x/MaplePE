#pragma once
#include "PacketScript.h"

#pragma region PacketInfo
enum PacketActionType {
	Decode1,
	Decode2,
	Decode4,
	Decode8,
	Skip8,
	DecodeStr,
	DecodeBuffer,
	Encode1,
	Encode2,
	Encode4,
	Encode8,
	EncodeStr,
	EncodeBuffer,
};

struct PacketAction {
	PacketActionType Type;
	size_t Size;
	ULONG_PTR RetAddr;
};

struct PacketInfo {
	uint32_t PID;
	uint32_t Index;
	bool IsInPacket;
	std::vector<uint8_t> Payload;
	std::vector<PacketAction> Actions;

	void Serialize(std::vector<uint8_t>& buffer) {
		PacketScript::Encode4(buffer, PID);
		PacketScript::Encode4(buffer, Index);
		PacketScript::Encode1(buffer, IsInPacket);
		PacketScript::Encode4(buffer, static_cast<uint32_t>(Payload.size()));
		buffer.insert(buffer.end(), Payload.begin(), Payload.end());
		PacketScript::Encode4(buffer, static_cast<uint32_t>(Actions.size()));
		for (const PacketAction& action : Actions) {
			PacketScript::Encode1(buffer, static_cast<uint8_t>(action.Type));
			PacketScript::Encode4(buffer, static_cast<uint32_t>(action.Size));
			PacketScript::Encode8(buffer, static_cast<uint64_t>(action.RetAddr));
		}
	}

	void Deserialize(std::vector<uint8_t>& buffer, size_t& pos) {
		PID = PacketScript::Decode4(buffer, pos);
		Index = PacketScript::Decode4(buffer, pos);
		IsInPacket = PacketScript::Decode1(buffer, pos);
		size_t payloadSize = PacketScript::Decode4(buffer, pos);
		Payload.assign(buffer.begin() + pos, buffer.begin() + pos + payloadSize);
		pos += payloadSize;
		size_t actionSize = PacketScript::Decode4(buffer, pos);
		for (size_t i = 0; i < actionSize; i++)
		{
			PacketAction action{
				PacketActionType(PacketScript::Decode1(buffer,pos)),
				PacketScript::Decode4(buffer, pos),
				static_cast<ULONG_PTR>(PacketScript::Decode8(buffer, pos)),
			};
			Actions.push_back(action);
		}
	}
};

#pragma endregion

#pragma region FilterOpcodeSet
struct FilterOpcodeSet {
	std::wstring CInPacketFilterOpcodes;
	std::wstring COutPacketFilterOpcodes;

	void Serialize(std::vector<uint8_t>& buffer) {
		PacketScript::EncodeStr(buffer, CInPacketFilterOpcodes);
		PacketScript::EncodeStr(buffer, COutPacketFilterOpcodes);
	}

	void Deserialize(std::vector<uint8_t>& buffer, size_t& pos) {
		CInPacketFilterOpcodes = PacketScript::DecodeStr(buffer, pos);
		COutPacketFilterOpcodes = PacketScript::DecodeStr(buffer, pos);
	}
};
#pragma endregion

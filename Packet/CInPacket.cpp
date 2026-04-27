#include "pch.h"
#include "CInPacket.h"

namespace {

	static std::set<uint16_t> gFilterOpcodeSet;

	static std::map<void*, std::vector<PacketAction>> gActionsMap;

	static std::map<void*, uint16_t> gOpcodeMap;

	static bool IsPayload(void* ecx) {
		InPacket* iPacket = static_cast<InPacket*>(ecx);
		return iPacket->m_nState == 2;
	}
}

namespace CInPacket {

	void UpdateFilterOpcodeSet(std::wstring& opcodes)
	{
		std::set<uint16_t> tempSet;
		std::wstringstream ss(opcodes);
		std::wstring token;

		while (std::getline(ss, token, L',')) {
			try {
				uint16_t opcode = static_cast<uint16_t>(std::stoul(token, nullptr, 0));
				tempSet.insert(opcode);
			}
			catch (const std::exception&) {
				DEBUG(L"Unknown filter opcodes")
			}
		}

		if (tempSet.empty()) {
			DEBUG(L"Failed to update CInPacketFilterOpcodeSet: no valid opcodes found");
			return;
		}

		gFilterOpcodeSet = std::move(tempSet);
	}


	bool IsFilterOpcode(uint16_t opcode)
	{
		return gFilterOpcodeSet.find(opcode) != gFilterOpcodeSet.end();
	}

	uint16_t GetOpcode(void* key)
	{
		auto it = gOpcodeMap.find(key);
		if (it == gOpcodeMap.end()) {
			return 0;
		}
		return it->second;
	}

	std::vector<PacketAction>* GetActions(void* key)
	{
		auto it = gActionsMap.find(key);
		if (it == gActionsMap.end()) {
			return nullptr;
		}
		return &it->second;
	}

	void DeleteActions(void* key)
	{
		gActionsMap.erase(key);
	}

	uint8_t(__thiscall* Decode1)(void* ecx) = nullptr;
	uint8_t __fastcall Decode1_Hook(void* ecx) {
		uint8_t result = Decode1(ecx);
		if (IsPayload(ecx)) {
			auto action = PacketAction{ PacketActionType::Decode1,1,(ULONG_PTR)_ReturnAddress() };
			auto actions = GetActions(ecx);
			if (actions == nullptr) {
				if (IsFilterOpcode(result)) {
					gActionsMap[ecx] = std::vector<PacketAction>{};
				}
				else {
					gActionsMap[ecx] = std::vector<PacketAction>{ action };
					gOpcodeMap[ecx] = result;
				}
			}
			else if (!actions->empty()) {
				actions->push_back(action);
			}
		}
		return result;
	}

	uint16_t(__thiscall* Decode2)(void* ecx) = nullptr;
	uint16_t __fastcall Decode2_Hook(void* ecx) {
		uint16_t result = Decode2(ecx);
		if (IsPayload(ecx)) {
			// CInPacket::AppendBuffer will call twice CInPacket::Decode2 as header in GMS95
			auto action = PacketAction{ PacketActionType::Decode2,2,(ULONG_PTR)_ReturnAddress() };
			auto actions = GetActions(ecx);
			if (actions == nullptr) {
				if (IsFilterOpcode(result)) {
					gActionsMap[ecx] = std::vector<PacketAction>{};
				}
				else {
					gActionsMap[ecx] = std::vector<PacketAction>{ action };
					gOpcodeMap[ecx] = result;
				}
			}
			else if (!actions->empty()) {
				actions->push_back(PacketAction{ action });
			}
		}
		return result;
	}

	uint32_t(__thiscall* Decode4)(void* ecx) = nullptr;
	uint32_t __fastcall Decode4_Hook(void* ecx) {
		if (IsPayload(ecx)) {
			// CInPacket::AppendBuffer will call once CInPacket::Decode4 as header in KMS1029
			auto actions = GetActions(ecx);
			if (actions != nullptr && !actions->empty()) {
				actions->push_back(PacketAction{ PacketActionType::Decode4,4,(ULONG_PTR)_ReturnAddress() });
			}
		}
		return Decode4(ecx);
	}

	uint64_t(__thiscall* Decode8)(void* ecx) = nullptr;
	uint64_t __fastcall Decode8_Hook(void* ecx) {
		if (IsPayload(ecx)) {
			auto actions = GetActions(ecx);
			if (actions != nullptr && !actions->empty()) {
				actions->push_back(PacketAction{ PacketActionType::Decode8,8,(ULONG_PTR)_ReturnAddress() });
			}
		}
		return Decode8(ecx);
	}

	extern uint64_t(__thiscall* Skip8)(void* ecx) = nullptr;
	uint64_t __fastcall Skip8_Hook(void* ecx)
	{
		if (IsPayload(ecx)) {
			auto actions = GetActions(ecx);
			if (actions != nullptr && !actions->empty()) {
				actions->push_back(PacketAction{ PacketActionType::Skip8,8,(ULONG_PTR)_ReturnAddress() });
			}
		}
		return Skip8(ecx);
	}

	char** (__thiscall* DecodeStr)(void* iPacket, char** result) = nullptr;
	char** __fastcall DecodeStr_Hook(void* ecx, FASTCALL_EDX_PADDING char** result) {
		if (IsPayload(ecx)) {
			auto actions = GetActions(ecx);
			if (actions != nullptr && !actions->empty()) {
				actions->push_back(PacketAction{ PacketActionType::DecodeStr,0,(ULONG_PTR)_ReturnAddress() });
			}
		}
		return DecodeStr(ecx, result);
	}

	void(__thiscall* DecodeBuffer)(void* ecx, uint8_t* p, size_t uSize) = nullptr;
	void __fastcall DecodeBuffer_Hook(void* ecx, FASTCALL_EDX_PADDING uint8_t* p, size_t uSize) {
		if (IsPayload(ecx)) {
			auto actions = GetActions(ecx);
			if (actions != nullptr && !actions->empty()) {
				actions->push_back(PacketAction{ PacketActionType::DecodeBuffer,uSize,(ULONG_PTR)_ReturnAddress() });
			}
		}
		return DecodeBuffer(ecx, p, uSize);
	}

}
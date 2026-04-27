#pragma once

#ifdef _WIN64
#pragma pack(push, 1)
typedef struct {
	uint32_t unk1;
	uint32_t m_bLoopback;
	uint32_t m_nState;
	uint32_t unk4;
	uint8_t* m_aRecvBuff; // Header(4) + Payload(Opcode(2) + Data(uint8_t...))
	uint16_t m_uLength;   // Payload length is kHeaderLength(4) + m_uDataLen(any)
	uint16_t unk7;
	uint16_t m_uRawSeq;
	uint8_t	 m_uPadding1;
	uint8_t	 m_uPadding2;
	uint32_t m_uDataLen;
	uint32_t m_uOffset;
} InPacket;
#pragma pack(pop)
#else
#pragma pack(push, 1)
typedef struct {
	uint32_t m_bLoopback;
	uint32_t m_nState;
	uint8_t* m_aRecvBuff; // Header(4) + Payload(Opcode(2) + Data(uint8_t...))
	uint16_t m_uLength;   // Payload length is kHeaderLength(4) + m_uDataLen(any)
	uint16_t m_uRawSeq;
	uint16_t m_uDataLen;
	uint8_t	 m_uPadding1;
	uint8_t	 m_uPadding2;
	uint32_t m_uOffset;
} InPacket;
#pragma pack(pop)
#endif

namespace CInPacket {

	void UpdateFilterOpcodeSet(std::wstring& opcodes);

	bool IsFilterOpcode(uint16_t opcode);

	uint16_t GetOpcode(void* key);

	std::vector<PacketAction>* GetActions(void* key);

	void DeleteActions(void* key);

	// GMS95 unsigned __int8 __thiscall CInPacket::Decode1(CInPacket *this)
	extern uint8_t(__thiscall* Decode1)(void* ecx);
	uint8_t __fastcall Decode1_Hook(void* ecx);

	// GMS95 int __thiscall CInPacket::Decode2(CInPacket *this)
	extern uint16_t(__thiscall* Decode2)(void* ecx);
	uint16_t __fastcall Decode2_Hook(void* ecx);

	// GMS95 unsigned int __thiscall CInPacket::Decode4(CInPacket *this)
	extern uint32_t(__thiscall* Decode4)(void* ecx);
	uint32_t __fastcall Decode4_Hook(void* ecx);

	// KMS1029 __int64 __thiscall CInPacket::Decode8(CInPacket *this)
	extern uint64_t(__thiscall* Decode8)(void* ecx);
	uint64_t __fastcall Decode8_Hook(void* ecx);

	// GMSCW1 __int64 __thiscall CInPacket::Skip8(CInPacket *this)
	extern uint64_t(__thiscall* Skip8)(void* ecx);
	uint64_t __fastcall Skip8_Hook(void* ecx);

	// GMS95 ZXString<char> *__thiscall CInPacket::DecodeStr(CInPacket *this, ZXString<char> *result)
	extern char** (__thiscall* DecodeStr)(void* ecx, char** result);
	char** __fastcall DecodeStr_Hook(void* ecx, FASTCALL_EDX_PADDING char** result);

	// GMS95 void __thiscall CInPacket::DecodeBuffer(CInPacket *this, unsigned __int8 *p, unsigned int uSize)
	extern void(__thiscall* DecodeBuffer)(void* ecx, uint8_t* p, size_t uSize);
	void __fastcall DecodeBuffer_Hook(void* ecx, FASTCALL_EDX_PADDING uint8_t* p, size_t uSize);

}
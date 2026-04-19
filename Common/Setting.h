#pragma once
#include <windows.h>
#include <string>

const std::wstring kSettingFilename = L"Setting.txt";

struct Setting {
	// Base info
	std::wstring GameProcessName = L"MapleStory.exe";
#ifdef _WIN64
	std::wstring PacketDllName = L"Packet64.dll";
#else
	std::wstring PacketDllName = L"Packet.dll";
#endif
	std::wstring GUIServerIP = L"127.0.0.1";
	ULONG_PTR GUIServerPort = 0xC0DE;
	bool IsTypeHeader1Byte = false;

	// Filter Opcodes
	std::wstring CInPacketFilterOpcodes = L"";
	std::wstring COutPacketFilterOpcodes = L"";

	// Function Addr
	ULONG_PTR CInPacketDecode1Addr = 0x00000000;
	ULONG_PTR CInPacketDecode2Addr = 0x00000000;
	ULONG_PTR CInPacketDecode4Addr = 0x00000000;
	ULONG_PTR CInPacketDecode8Addr = 0x00000000;
	ULONG_PTR CInPacketSkip8Addr = 0x00000000;
	ULONG_PTR CInPacketDecodeStrAddr = 0x00000000;
	ULONG_PTR CInPacketDecodeBufferAddr = 0x00000000;

	ULONG_PTR COutPacketEncode1Addr = 0x00000000;
	ULONG_PTR COutPacketEncode2Addr = 0x00000000;
	ULONG_PTR COutPacketEncode4Addr = 0x00000000;
	ULONG_PTR COutPacketEncode8Addr = 0x00000000;
	ULONG_PTR COutPacketEncodeStrAddr = 0x00000000;
	ULONG_PTR COutPacketEncodeBufferAddr = 0x00000000;
	ULONG_PTR COutPacketMakeBufferListAddr = 0x00000000;

	ULONG_PTR CClientSocketProcessPacketAddr = 0x00000000;
	ULONG_PTR CClientSocketSendPacketAddr = 0x00000000;
	ULONG_PTR SendPacketEHAddr = 0x00000000;

	// Template code
	std::wstring CInPacketDecode1GenCode = L"v%d := iPacket.Decode1()";
	std::wstring CInPacketDecode2GenCode = L"v%d := iPacket.Decode2()";
	std::wstring CInPacketDecode4GenCode = L"v%d := iPacket.Decode4()";
	std::wstring CInPacketDecode8GenCode = L"v%d := iPacket.Decode8()";
	std::wstring CInPacketSkip8GenCode = L"v%d := iPacket.Skip8()";
	std::wstring CInPacketDecodeStrGenCode = L"str%d := iPacket.DecodeStr()";
	std::wstring CInPacketDecodeBufferGenCode = L"buf%d := iPacket.DecodeBuffer(%d)";

	std::wstring COutPacketEncode1GenCode = L"oPacket.Encode1(%ls)";
	std::wstring COutPacketEncode2GenCode = L"oPacket.Encode2(%ls)";
	std::wstring COutPacketEncode4GenCode = L"oPacket.Encode4(%ls)";
	std::wstring COutPacketEncode8GenCode = L"oPacket.Encode8(%ls)";
	std::wstring COutPacketEncodeStrGenCode = L"oPacket.EncodeStr(\"%ls\")";
	std::wstring COutPacketEncodeBufferGenCode = L"oPacket.EncodeBuffer(make([]byte,%d))";
};

inline bool WSTR2BOOL(const std::wstring& value) {
	return (!value.empty() && value != L"0");
}

inline ULONG_PTR WSTR2ULONGPTR(const std::wstring& value, ULONG_PTR defaultValue = 0) {
	try {
		return static_cast<ULONG_PTR>(std::stoull(value, nullptr, 0));
	}
	catch (...) {
		return defaultValue;
	}
}

#define SET_BOOL_FIELD(name) \
    else if (key == L#name) s.name = WSTR2BOOL(value);

#define SET_ADDR_FIELD(name) \
    else if (key == L#name) s.name = WSTR2ULONGPTR(value);

#define SET_WSTR_FIELD(name) \
    else if (key == L#name) s.name = value;

std::wstring GetExecutableDir(HMODULE hModule);

bool CheckEmptyAddr(const Setting& s);

bool LoadSetting(std::wstring dir, Setting& s);

bool SaveSetting(std::wstring dir, const Setting& s);
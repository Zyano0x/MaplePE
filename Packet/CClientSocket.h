#pragma once
#include "CInPacket.h"
#include "COutPacket.h"

// KMS1029 void __cdecl SendPacket(COutPacket *oPacket)
extern void(__fastcall* SendPacket_EH)(void* packet);

namespace CClientSocket {

	// GMS95 void __thiscall CClientSocket::ProcessPacket(CClientSocket *this, CInPacket *iPacket)
	extern void(__thiscall* ProcessPacket)(void* ecx, void* packet);
	void __fastcall ProcessPacket_Hook(void* ecx, FASTCALL_EDX_PADDING void* packet);

	// GMS95 void __fastcall CClientSocket::SendPacket(CClientSocket *this, int a2, COutPacket *oPacket)
	// KMS1029 void __cdecl CClientSocket::SendPacket(COutPacket *oPacket)
	extern void(__fastcall* SendPacket)(void* ecx, FASTCALL_EDX_PADDING void* packet);
}
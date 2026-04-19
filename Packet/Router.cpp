#include "pch.h"
#include "Router.h"
#include "CClientSocket.h"
#include "GUIClient.h"

namespace {
	static GUIClient* gGUIClient = nullptr;
}

namespace Router {

	const uint32_t kPID = GetCurrentProcessId();
	uint32_t gPacketIndex = 0;
	void* gClientSocketPtr = nullptr;

	void Init(const std::wstring& serverIP, const uint16_t serverPort) {
		if (gGUIClient != nullptr) {
			DEBUG(L"GUIClient already initialized");
			return;
		}
		gGUIClient = new GUIClient(serverIP, serverPort);
		gGUIClient->Ping(kPID);
		gGUIClient->OnNotify();
	}

	void Free() {
		delete gGUIClient;
		gGUIClient = nullptr;
	}

	void SendPacketInfo(PacketInfo& info) {
		if (gGUIClient == nullptr) {
			return;
		}
		gGUIClient->SendPacketInfo(info);
	}

	void ProcessPacket(void* iPacket) {
		if (gClientSocketPtr == nullptr) {
			DEBUG(L"ClientSocket has not been specified");
			return;
		}
#ifdef _WIN64
		CClientSocket::ProcessPacket_Hook(gClientSocketPtr, iPacket);
#else
		CClientSocket::ProcessPacket_Hook(gClientSocketPtr, nullptr, iPacket);
#endif
	}

	void SendPacket(void* oPacket) {
		if (SendPacket_EH != nullptr) {
			SendPacket_EH(oPacket);
			return;
		}
		if (gClientSocketPtr == nullptr) {
			DEBUG(L"ClientSocket has not been specified");
			COutPacket::DeleteActions(oPacket);
			return;
		}
#ifdef _WIN64
		CClientSocket::SendPacket(gClientSocketPtr, oPacket);
#else
		CClientSocket::SendPacket(gClientSocketPtr, nullptr, oPacket);
#endif
	}
}
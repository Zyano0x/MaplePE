#pragma once
#include <functional>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define RECV_BUF_LEN 65536

static bool InitWSAData() {
	static bool initialized = [] {
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0) {
			return false;
		}
		return result == 0;
		}();
	return initialized;
}

static void StartListen(const std::wstring& name, SOCKET socket, std::function<void(const char*, int, const std::string&, uint16_t)> handler) {
	int iResult = 0;
	char RecvBuf[RECV_BUF_LEN];
	sockaddr_in saddr{};
	int saddrSize = 0;
	// Call the recvfrom function to receive datagrams	
	// on the bound socket.
	while (true) {
		saddrSize = sizeof(saddr);
		iResult = recvfrom(socket, RecvBuf, RECV_BUF_LEN, 0, (SOCKADDR*)&saddr, &saddrSize);
		if (iResult == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err == WSAEINTR || err == WSAENOTSOCK) {
				// Client process exited
				return;
			}
			if (err == WSAECONNRESET) {
				// Server process exited
				continue;
			}
			std::wstring msg = L"Failed to receive with error: " + std::to_wstring(err);
			MessageBox(nullptr, msg.c_str(), name.c_str(), MB_OK);
			continue;
		}
		if (iResult == 0) {
			std::wstring msg = L"Received empty UDP packet";
			MessageBox(nullptr, msg.c_str(), name.c_str(), MB_OK);
			continue;
		}
		char ipStr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &saddr.sin_addr, ipStr, sizeof(ipStr));
		uint16_t port = ntohs(saddr.sin_port);
		handler(RecvBuf, iResult, std::string(ipStr), port);
	}
}
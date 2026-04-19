#include "UDPClient.h"
#include "WSAData.h"

UDPClient::UDPClient(std::wstring serverIP, uint16_t serverPort)
{
	m_name = L"UDPClient";
	if (!InitWSAData()) {
		OutputDebugStringW(L"WSAStartup failed");
		return;
	}
	// Resolve the local address and port to be used by the client
	std::wstring portStr = std::to_wstring(serverPort);
	ADDRINFOW* result = nullptr;
	ADDRINFOW hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	int iResult = GetAddrInfoW(serverIP.c_str(), portStr.c_str(), &hints, &result);
	if (iResult != 0) {
		std::wstring err = L"GetAddrInfoW failed: " + std::to_wstring(iResult);
		OutputDebugStringW(err.c_str());
		return;
	}
	// Create a socket for sending data
	m_sendSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_sendSocket == INVALID_SOCKET) {
		std::wstring err = L"error at socket(): " + std::to_wstring(WSAGetLastError());
		OutputDebugStringW(err.c_str());
		FreeAddrInfoW(result);
		return;
	}
	memcpy(&m_serverAddr, result->ai_addr, result->ai_addrlen);
	// Bind to a random local port for receiving server message
	sockaddr_in localAddr = {};
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = INADDR_ANY;
	localAddr.sin_port = 0;
	if (bind(m_sendSocket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
		std::wstring err = L"bind() failed: " + std::to_wstring(WSAGetLastError());
		OutputDebugStringW(err.c_str());
		FreeAddrInfoW(result);
		closesocket(m_sendSocket);
		return;
	}
	FreeAddrInfoW(result);
}

UDPClient::~UDPClient()
{
	int iResult = closesocket(m_sendSocket);
	if (iResult == SOCKET_ERROR) {
		std::wstring err = L"closesocket failed with error: " + std::to_wstring(WSAGetLastError());
		OutputDebugStringW(err.c_str());
		m_sendSocket = INVALID_SOCKET;
	}
	if (m_thread.joinable()) {
		m_thread.join();
	}
	WSACleanup();
}

bool UDPClient::SendBuffer(const void* buf, size_t bufLen)
{
	int iResult = sendto(m_sendSocket, static_cast<const char*>(buf), static_cast<int>(bufLen), 0, (SOCKADDR*)&m_serverAddr, sizeof(m_serverAddr));
	if (iResult == SOCKET_ERROR) {
		int errCode = WSAGetLastError();
		switch (errCode) {
		case WSAENOTSOCK:
			OutputDebugStringW(L"sendto() failed: socket was already closed");
			break;
		case WSAEINTR:
			OutputDebugStringW(L"sendto() was interrupted, possibly due to thread shutdown");
			break;
		case WSAEMSGSIZE:
			OutputDebugStringW(L"Packet too large for UDP MTU");
		default:
			std::wstring err = L"Unknown err" + std::to_wstring(errCode);
			OutputDebugStringW(err.c_str());
		}
		return false;
	}
	return true;
}

void UDPClient::OnNotify()
{
	m_thread = std::thread([this]() {
		StartListen(m_name, m_sendSocket, [this](const char* data, int len, const std::string& ip, uint16_t port) {
			this->HandleDatagram(data, len, ip, port);
			});
		});
}
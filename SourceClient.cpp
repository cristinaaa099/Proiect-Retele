#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

constexpr int DEFAULT_BUFLEN = 512;
constexpr const char* DEFAULT_PORT = "980";
constexpr const char* LOOPBACK_ADDR = "localhost";

enum PACKET_TYPE
{
	COMANDA1 = 93232,
	COMANDA2,
	COMANDA3,
	COMANDA4
};

int Send(PACKET_TYPE packetType, char* sendBuf, int& iResult, SOCKET& ConnectSocket)
{
	iResult = send(ConnectSocket, _itoa(packetType, sendBuf, 10), (int)strlen(sendBuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("Eroare send: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Bytes trimisi: %ld => %s\n", iResult, sendBuf);
	return 0;
}

int main(int argc, char** argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("Eroare WSAStartup: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(LOOPBACK_ADDR, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("Eroare getaddrinfo: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("Eroare socket: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Nu se poate realiza conexiunea la server! Eroare: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	char sendbuf[DEFAULT_BUFLEN];
	ZeroMemory(sendbuf, DEFAULT_BUFLEN);
	_itoa(COMANDA1, sendbuf, 10);
	while (true)
	{
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			if (GetAsyncKeyState(0x31) & 1)//1
			{
				if (Send(COMANDA1, sendbuf, iResult, ConnectSocket))
				{
					return 1;
				}
			}
			if (GetAsyncKeyState(0x32) & 1)//2
			{
				if (Send(COMANDA2, sendbuf, iResult, ConnectSocket))
				{
					return 1;
				}
			}
			if (GetAsyncKeyState(0x33) & 1)//3
			{
				if (Send(COMANDA3, sendbuf, iResult, ConnectSocket))
				{
					return 1;
				}
			}
			if (GetAsyncKeyState(0x34) & 1)//4
			{
				if (Send(COMANDA4, sendbuf, iResult, ConnectSocket))
				{
					return 1;
				}
			}
			if (GetAsyncKeyState('K') & 1)
			{
				break;
			}
		}
		Sleep(100);
	}


	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("Eroare shutdown: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}


	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}
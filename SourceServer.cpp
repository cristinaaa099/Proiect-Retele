#undef UNICODE
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <thread>

#pragma comment (lib, "Ws2_32.lib")

constexpr int DEFAULT_BUFLEN = 512;
constexpr const char* DEFAULT_PORT = "980";
constexpr int NUM_CLIENTS = 10;

std::thread* clients[NUM_CLIENTS];

enum PACKET_TYPE
{
	COMANDA1 = 93232,
	COMANDA2,
	COMANDA3,
	COMANDA4
};

void createClient(int id);

void destroySocket(SOCKET& s, bool cleanup = true)
{
	closesocket(s);
	if (cleanup)
	{
		WSACleanup();
	}
}

class Server
{
	static SOCKET ListenSocket;

	static WSADATA wsaData;
	static int iResult;

	static struct addrinfo* result;
	static struct addrinfo hints;

public:
	static SOCKET getSocket()
	{
		return ListenSocket;
	}

	static void Init()
	{
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("Eroare WSAStartup: %d\n", iResult);
			return;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
		if (iResult != 0) {
			printf("Eroare getaddrinfo: %d\n", iResult);
			WSACleanup();
			return;
		}

		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) {
			printf("Eroare socket: %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return;
		}

		printf("Socket-ul a fost initializat\n");

		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			printf("Eroare bind: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			destroySocket(ListenSocket);
			return;
		}

		printf("Bind a fost initializat\n");

		freeaddrinfo(result);

		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			printf("Eroare listen: %d\n", WSAGetLastError());
			destroySocket(ListenSocket);
			return;
		}

		printf("Se face listening...\n");
	}
};

SOCKET Server::ListenSocket = INVALID_SOCKET;
struct addrinfo* Server::result = NULL;
WSADATA Server::wsaData;
int Server::iResult;
struct addrinfo Server::hints;

class ClientConnection
{
	bool initialised = false;
	static int totalClients;
	int currentClient;
	int iResult;

	SOCKET ClientSocket = INVALID_SOCKET;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	void CloseSocket(SOCKET s, bool cleanup)
	{
		destroySocket(s, cleanup);
		if (totalClients > 0)
		{
			totalClients--;
		}
		printf("Conexiunea pentru clientul %d s-a inchis! Au mai ramas %d clienti\n", currentClient, totalClients);
		initialised = false;
	}

public:
	void operator()(int id)
	{
		currentClient = id;
		Init();
	}

	void Init()
	{
		while (true)
		{
			Listen();
		}
	}

	void Listen()
	{
		SOCKADDR_IN client_info = { 0 };
		int addrsize = sizeof(client_info);

		ClientSocket = accept(Server::getSocket(), (struct sockaddr*)&client_info, &addrsize);
		if (ClientSocket == INVALID_SOCKET) {
			printf("(Client #%d) Eroare accept: %d\n", currentClient, WSAGetLastError());
			CloseSocket(ClientSocket, false);
			return;
		}
		getpeername(ClientSocket, (struct sockaddr*)&client_info, &addrsize);

		char* ip = inet_ntoa(client_info.sin_addr);
		auto port = client_info.sin_port;

		if (!initialised)
		{
			initialised = true;
			++totalClients;
		}
		printf("(Client #%d) S-a acceptat conexiunea socket IP: %s PORT: %d, Total clienti: %d\n", currentClient, ip, port, totalClients);

		do {

			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {

				printf("(Client #%d) Bytes primiti: %d => %s\n", currentClient, iResult, recvbuf);
				switch (atoi(recvbuf))
				{
				case COMANDA1:
				{
					printf("(Client #%d) Se executa comanda 1...\n", currentClient);
					break;
				}
				case COMANDA2:
				{
					printf("(Client #%d) Se executa comanda 2...\n", currentClient);
					break;
				}
				case COMANDA3:
				{
					printf("(Client #%d) Se executa comanda 3...\n", currentClient);
					break;
				}
				case COMANDA4:
				{
					printf("(Client #%d) Se executa comanda 4...\n", currentClient);
					break;
				}
				default:break;
				}
			}
			else if (iResult == 0)
				printf("(Client #%d) Conexiunea se inchide...\n", currentClient);
			else {
				int lastErr = WSAGetLastError();
				if (lastErr == WSAECONNRESET)
				{
					printf("(Client #%d) Conexiunea a fost inchisa fortat!\n", currentClient);
				}
				else
				{
					printf("(Client #%d) Eroare recv: %d\n", currentClient, WSAGetLastError());
				}
				CloseSocket(ClientSocket, false);
				return;
			}

			ZeroMemory(recvbuf, recvbuflen);
		} while (iResult > 0);


		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf("(Client #%d) Eroare shutdown: %d\n", WSAGetLastError(), currentClient);
			CloseSocket(ClientSocket, false);
			return;
		}

		CloseSocket(ClientSocket, false);
	}
};

void createClient(int id)
{
	clients[id] = new std::thread(ClientConnection(), id);
}

int ClientConnection::totalClients = 0;

int main()
{
	Server::Init();
	for (int i = 0; i < NUM_CLIENTS; ++i)
	{
		createClient(i);
	}

	for (int i = 0; i < NUM_CLIENTS; ++i)
	{
		clients[i]->join();
	}

	return 0;
}
#pragma once


#include "WsService.hpp"
#include "utils/WsPathUtils.hpp"
#include "utils/WsRequestUtils.hpp"
#include "utils/WsDataFrameUtils.hpp"
#include "WsSafeConList.hpp"

#include <iostream>
#include <string>
#include <deque>


#include <atomic>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>

using namespace std;

#define WS_BUFFER_SIZE 1024




struct WsSocketData
{
	WSAOVERLAPPED	m_overlapped;
	WSABUF			m_wsaBuffer;   //异步收发消息的buffer,只是一个地址，需要绑定自己的buffer
	SOCKET			m_clientSocket;

	char			m_buffer[WS_BUFFER_SIZE];
	string m_totalBuffer;
};

class WsServer
{
private:
	atomic_bool m_isStop;

	HANDLE m_iocpHandle;
	deque<thread> m_workThreadList;

private:
	int m_mostThreadCount;

	map<string, WsService*> m_pathServiceMap;
	map<string, WsSafeConList*> m_pathConlistMap;

public:
	WsServer()
	{
		m_isStop = false;
		m_iocpHandle = INVALID_HANDLE_VALUE;

		m_mostThreadCount = std::thread::hardware_concurrency();
	}

	~WsServer()
	{
		m_isStop = true;
		for (thread& workThread : m_workThreadList)
		{
			workThread.join();
		}

		for (auto& it : m_pathServiceMap)
		{
			delete it.second;
		}

		for (auto& it : m_pathConlistMap)
		{
			delete it.second;
		}
	}


public:
	void injectService(string path, WsService* service)
	{
		m_pathServiceMap[path] = service;
		m_pathConlistMap[path] = new WsSafeConList();
	}


public:
	void setMostThreadCount(int threadCount) { m_mostThreadCount = threadCount; }

	bool listen(string ip, int port)
	{
		createWorkerThread();

		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		SOCKET serverSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

		SOCKADDR_IN serverSocketAddr;
		serverSocketAddr.sin_family = PF_INET;
		serverSocketAddr.sin_port = htons(port);
		serverSocketAddr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

		if (::bind(serverSocket, (struct sockaddr*)&serverSocketAddr, sizeof(SOCKADDR_IN)))
		{
			return false;
		}
		if (::listen(serverSocket, 5))
		{
			return false;
		}

		m_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);


		SOCKADDR_IN clientScoketAddr;
		int addrLength = sizeof(SOCKADDR_IN);

		SOCKET clientSocket;
		DWORD receiveByteSize;
		DWORD flags = 0;

		while (true)
		{
			clientSocket = WSAAccept(serverSocket, (struct sockaddr*)&clientScoketAddr, &addrLength, NULL, NULL);

			WsSocketData* wsSocketData = new WsSocketData();
			wsSocketData->m_clientSocket = clientSocket;
			wsSocketData->m_wsaBuffer.len = WS_BUFFER_SIZE;
			wsSocketData->m_wsaBuffer.buf = wsSocketData->m_buffer;

			wsSocketData->m_totalBuffer.clear();

			m_iocpHandle = CreateIoCompletionPort((HANDLE)clientSocket, m_iocpHandle, (DWORD)wsSocketData, 0);


			WSARecv(clientSocket, &(wsSocketData->m_wsaBuffer), 1, &receiveByteSize, &flags,
				&(wsSocketData->m_overlapped), NULL);
		}

		WSACleanup();

		return true;
	}

//listen
private:
	void createWorkerThread()
	{
		for (int i = 0; i < m_mostThreadCount; i++)
		{
			m_workThreadList.emplace_back([this]() {

				DWORD sendByteSize = 0;
				DWORD dwFlags = 0;
				DWORD receiveByteSize = 0;
				WsSocketData* wsSocketData;
				WsSocketData* wsIocpKey;

				while (true)
				{
					if (this->m_isStop)
					{
						break;
					}
					if (this->m_iocpHandle == INVALID_HANDLE_VALUE)
					{
						continue;
					}

					BOOL isSuccess = GetQueuedCompletionStatus(this->m_iocpHandle, &receiveByteSize,
						(PULONG_PTR)&wsIocpKey, (LPOVERLAPPED*)&wsSocketData, INFINITE);

					if (isSuccess == FALSE)//The client disconnected the link
					{
						closeConnect(wsSocketData);

						continue;
					}

					if (receiveByteSize == 0)//Maybe the client sent an empty packet
					{
						closeConnect(wsSocketData);

						continue;
					}

					wsSocketData->m_totalBuffer.append(wsSocketData->m_wsaBuffer.buf, receiveByteSize);

					if (wsSocketData->m_totalBuffer[0] == 'G')
					{
						if (WsRequestUtils::isValidConRequest(wsSocketData->m_totalBuffer) && this->bindToConList(wsSocketData))
						{
							deque<WsConnect> curConList = this->getConnectList(wsSocketData->m_clientSocket)->getCurConnectList();
							
							WsRequestUtils::buildLink(wsSocketData->m_totalBuffer, wsSocketData->m_clientSocket);
							wsSocketData->m_totalBuffer.clear();

							this->getService(wsSocketData->m_clientSocket)->open(curConList, this->getConnect(wsSocketData->m_clientSocket));
						}
						else
						{
							closeConnect(wsSocketData);

							continue;
						}
					}
					else if ((receiveByteSize < WS_BUFFER_SIZE) || WsDataFrameUtils::isComplete(wsSocketData->m_totalBuffer))
					{
						if (this->isValidConnect(wsSocketData->m_clientSocket) == false)
						{
							closeConnect(wsSocketData);

							continue;
						}
						else
						{
							this->handleDataFrame(wsSocketData);
							wsSocketData->m_totalBuffer.clear();
						}
					}

					ZeroMemory(&(wsSocketData->m_overlapped), sizeof(OVERLAPPED));
					ZeroMemory(wsSocketData->m_wsaBuffer.buf, WS_BUFFER_SIZE);
					wsSocketData->m_wsaBuffer.len = WS_BUFFER_SIZE;

					dwFlags = 0;
					WSARecv(wsSocketData->m_clientSocket, &(wsSocketData->m_wsaBuffer), 1, &receiveByteSize, &dwFlags,
						(LPWSAOVERLAPPED)&(wsSocketData->m_overlapped), NULL);
				}
			});
		}
	}

//createWorkerThread
private:
	bool bindToConList(WsSocketData* requestSocketData)
	{
		string path = WsStringUtils::getStringUseCharStart(requestSocketData->m_totalBuffer, '/');
		path = "/" + WsStringUtils::getStringUseCharEnd(path, ' ');

		string bindPath = getBindPath(path);
		if (isValidPath(path) && m_pathConlistMap[bindPath]->isExistConnect(requestSocketData->m_clientSocket) == false)
		{
			WsConnect* wsConnect = new WsConnect(requestSocketData->m_clientSocket, WsPathUtils::getParam(path, bindPath));

			m_pathConlistMap[bindPath]->injectConnect(wsConnect);

			return true;
		}
		else
		{
			return false;
		}
	}

	WsConnect* getConnect(SOCKET socketHandle)
	{
		for (auto& it : m_pathConlistMap)
		{
			if (it.second->isExistConnect(socketHandle))
			{
				return it.second->getConnect(socketHandle);
			}
		}

		return nullptr;
	}

	WsSafeConList* getConnectList(SOCKET socketHandle)
	{
		for (auto& it : m_pathConlistMap)
		{
			if (it.second->isExistConnect(socketHandle))
			{
				return it.second;
			}
		}

		return nullptr;
	}

	WsService* getService(SOCKET socketHandle)
	{
		for (auto& it : m_pathConlistMap)
		{
			if (it.second->isExistConnect(socketHandle))
			{
				return m_pathServiceMap[it.first];
			}
		}

		return nullptr;
	}

	bool isValidConnect(SOCKET socketHandle)
	{
		for (auto& it : m_pathConlistMap)
		{
			if (it.second->isExistConnect(socketHandle))
			{
				return true;
			}
		}

		return false;
	}

	void handleDataFrame(WsSocketData* wsSocketData)//parse message type
	{
		deque<WsConnect> conList = getConnectList(wsSocketData->m_clientSocket)->getCurConnectList();

		int dataType = WsDataFrameUtils::getType(wsSocketData->m_totalBuffer);
		switch (dataType)
		{
		case 1:
		{
			string msg = WsDataFrameUtils::getMsg(wsSocketData->m_totalBuffer);
			getService(wsSocketData->m_clientSocket)->message(conList, getConnect(wsSocketData->m_clientSocket), msg);
			break;
		}
		case 2:
		{
			string msg = WsDataFrameUtils::getMsg(wsSocketData->m_totalBuffer);
			getService(wsSocketData->m_clientSocket)->message(conList, getConnect(wsSocketData->m_clientSocket), msg);
			break;
		}
		case 8:
		{
			getService(wsSocketData->m_clientSocket)->close(conList, getConnect(wsSocketData->m_clientSocket));
			break;
		}
		default:
			getService(wsSocketData->m_clientSocket)->typeError(conList, getConnect(wsSocketData->m_clientSocket));
			break;
		}
	}

	void closeConnect(WsSocketData* wsSocketData)
	{
		SOCKET socketHandle = wsSocketData->m_clientSocket;

		if (isValidConnect(socketHandle))
		{
			deque<WsConnect> conList = getConnectList(socketHandle)->getCurConnectList();
			getService(socketHandle)->disConnect(conList, getConnect(socketHandle));

			for (auto& it : m_pathConlistMap)
			{
				if (it.second->erase(socketHandle))
				{
					closesocket(socketHandle);
					delete wsSocketData;
					wsSocketData = nullptr;

					break;
				}
			}
		}
		else
		{
			closesocket(socketHandle);
			delete wsSocketData;
		}
	}



//bindToConList
private:
	bool isValidPath(string path)
	{
		for (auto& it : m_pathConlistMap)
		{
			if (WsPathUtils::isMatch(path, it.first))
			{
				return true;
			}
		}
		return false;
	}

	string getBindPath(string path)
	{
		for (auto& it : m_pathConlistMap)
		{
			if (WsPathUtils::isMatch(path, it.first))
			{
				return it.first;
			}
		}

		return "";
	}
};



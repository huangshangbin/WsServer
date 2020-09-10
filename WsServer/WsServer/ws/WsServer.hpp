#pragma once


#include "WsService.hpp"
#include "WsStringUtils.hpp"
#include "WsEnDecryptUtils.hpp"
#include "WsTypeConvertUtils.hpp"
#include "WsBitUtils.hpp"

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
						closesocket(wsSocketData->m_clientSocket);
						delete wsSocketData;
						continue;
					}

					if (receiveByteSize == 0)//Maybe the client sent an empty packet
					{
						closesocket(wsSocketData->m_clientSocket);
						delete wsSocketData;
						continue;
					}

					wsSocketData->m_totalBuffer.append(wsSocketData->m_wsaBuffer.buf, receiveByteSize);

					if (wsSocketData->m_totalBuffer[0] == 'G')
					{
						if (this->isValidConRequest(wsSocketData->m_totalBuffer) && this->bindToConList(wsSocketData))
						{
							this->buildLink(wsSocketData);
							wsSocketData->m_totalBuffer.clear();
						}
						else
						{
							closesocket(wsSocketData->m_clientSocket);
							delete wsSocketData;
							continue;
						}
					}
					else if ((receiveByteSize < WS_BUFFER_SIZE) || this->isCompleteDataFrame(wsSocketData->m_totalBuffer))
					{
						if (this->isValidConnect(wsSocketData->m_clientSocket) == false)
						{
							closesocket(wsSocketData->m_clientSocket);
							delete wsSocketData;
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
	bool isValidConRequest(string& requestData)
	{
		if (WsStringUtils::isExistStringInString(requestData, "GET") == false)
		{
			return false;
		}

		if (WsStringUtils::isExistStringInString(requestData, "Sec-WebSocket-Key") == false)
		{
			return false;
		}

		if (WsStringUtils::isExistStringInString(requestData, "Upgrade") == false)
		{
			return false;
		}

		if (WsStringUtils::isExistStringInString(requestData, "websocke") == false)
		{
			return false;
		}

		if (WsStringUtils::isExistStringInString(requestData, "Sec-WebSocket-Version") == false)
		{
			return false;
		}

		return true;
	}

	bool bindToConList(WsSocketData* requestSocketData)
	{
		string path = WsStringUtils::getStringUseCharStart(requestSocketData->m_totalBuffer, '/');
		path = "/" + WsStringUtils::getStringUseCharEnd(path, ' ');

		string bindPath = getBindPath(path);
		if (isValidPath(path) && m_pathConlistMap[bindPath]->isExistConnect(requestSocketData->m_clientSocket) == false)
		{
			WsConnect* wsConnect = new WsConnect(requestSocketData->m_clientSocket, getPathParam(path));

			m_pathConlistMap[bindPath]->injectConnect(wsConnect);

			return true;
		}
		else
		{
			return false;
		}
	}

	void buildLink(WsSocketData* requestSocketData)
	{
		string key = WsStringUtils::splitStringGetOneStr(requestSocketData->m_totalBuffer, "Sec-WebSocket-Key", 1);
		key = WsStringUtils::getStringUseCharStart(key, ' ');
		key = WsStringUtils::getStringUseCharEnd(key, '\r');

		string response = "HTTP/1.1 101 Switching Protocols\r\n"\
			"Upgrade:websocket\r\n"\
			"Connection:Upgrade\r\n"\
			"Sec-WebSocket-Accept:" + getAccept(key) + "\r\n"\
			"\r\n";

		::send(requestSocketData->m_clientSocket, response.c_str(), response.length(), 0);


	}

	bool isCompleteDataFrame(string& dataBuffer)
	{
		int msgLengthMark = WsBitUtils::getIntUseCharPos(dataBuffer[1], 1, 7);
		if (msgLengthMark < 126)
		{
			return dataBuffer.length() == (2 + 4 + msgLengthMark);
		}
		else if (msgLengthMark == 126)
		{
			int msgLength = WsBitUtils::getUnsignShort(dataBuffer[2], dataBuffer[3]);
			return dataBuffer.length() == (2 + 2 + 4 + msgLength);
		}
		else
		{
			int msgLength = WsBitUtils::getUnsignLongLong(dataBuffer[2], dataBuffer[3], dataBuffer[4], dataBuffer[5],
				dataBuffer[6], dataBuffer[7], dataBuffer[8], dataBuffer[9]);

			return dataBuffer.length() == (2 + 8 + 4 + msgLength);
		}
	}

	bool isValidConnect(SOCKET socketHandle)
	{
		// 是否在m_socketConMap里面，不在是非法的

		return true;
	}

	void handleDataFrame(WsSocketData* requestSocketData)
	{
		
	}

	void closeConnect(WsSocketData* wsSocketData)
	{
		//移除链接在map里面的记录，close socket。
	}



//bindToConList
private:
	bool isValidPath(string path)
	{
		for (auto& it : m_pathConlistMap)
		{
			if (isMatchPath(path, it.first))
			{
				return true;
			}
		}
		return false;
	}

	map<string, string> getPathParam(string path)
	{
		map<string, string> paramMap;
		return std::move(paramMap);
	}

	string getBindPath(string path)
	{
		for (auto& it : m_pathConlistMap)
		{
			if (isMatchPath(path, it.first))
			{
				return it.first;
			}
		}

		return "";
	}

//buildLink
private:
	string getAccept(string key)
	{
		string acceptStr = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		acceptStr = WsEnDecryptUtils::sha1Encrypt(acceptStr);

		char acceptArray[1000] = { 0 };
		int arrayIndex = 0;
		for (int i = 0; i <= acceptStr.length() - 2; i = i + 2)
		{
			acceptArray[arrayIndex] = (int)WsTypeConvertUtils::getLongUseHexStr(WsStringUtils::getStringUsePos(acceptStr, i, i + 1));
			arrayIndex++;
		}

		acceptStr = acceptArray;
		return WsEnDecryptUtils::base64Encode(acceptStr);
	}

//isValidPath
private:
	bool isMatchPath(string path, string restfulPath)
	{
		deque<string> pathList = WsStringUtils::splitString(path, "/");
		deque<string> restfulPathList = WsStringUtils::splitString(restfulPath, "/");
		if (pathList.size() != restfulPathList.size())
		{
			return false;
		}

		for (int i = 0; i < pathList.size(); i++)
		{
			if ((restfulPathList[i][0] == '{') && (restfulPathList[i][restfulPathList.size() - 1] == '}'))
			{
				continue;
			}

			if (pathList[i] != restfulPathList[i])
			{
				return false;
			}
		}

		return true;
	}

};



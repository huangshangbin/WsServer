#pragma once


#include <WinSock2.h>

#include <map>

class WsConnect
{
private:
	SOCKET m_socket;
	map<string, string> m_pathParamMap;

public:
	WsConnect(SOCKET socketHandle, map<string, string>&& pathParamMap)
	{
		m_socket = socketHandle;
		m_pathParamMap = std::move(pathParamMap);
	}

public:
	SOCKET getSocketHandle()
	{
		return m_socket;
	}

	string getUrlParam(string key)
	{
		return m_pathParamMap[key];
	}

public:
	void sendText(string& text) 
	{
		::send(m_socket, text.c_str(), text.length(), 0);//Synchronous transmission to avoid data copy
	}

};


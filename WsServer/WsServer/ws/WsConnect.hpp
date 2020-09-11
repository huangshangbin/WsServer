#pragma once


#include <WinSock2.h>

#include <map>
#include <deque>

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

	deque<string> getUrlParamKeyList()
	{
		deque<string> keyList;
		for (auto& it : m_pathParamMap)
		{
			keyList.push_back(it.first);
		}

		return std::move(keyList);
	}

public:
	void sendText(string& text) 
	{
		::send(m_socket, text.c_str(), text.length(), 0);//Synchronous transmission to avoid data copy
	}

	void sendClose()
	{

	}

private:

};


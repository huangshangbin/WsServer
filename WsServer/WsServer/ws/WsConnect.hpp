#pragma once


#include <WinSock2.h>

#include <map>
#include <deque>

#include <ws/utils/WsBitUtils.hpp>
#include <ws/utils/WsStringUtils.hpp>

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
	void sendText(string text) 
	{
		char byte1 = WsBitUtils::getChar("10000001");

		string sendData;
		WsStringUtils::addChar(sendData, byte1);

		if (text.length() < 126)
		{
			char byte2 = text.length();
			WsBitUtils::setBit(byte2, 0, 0);

			WsStringUtils::addChar(sendData, byte2);
		}
		else if (text.length() <= 65535)
		{
			char byte2 = 126;
			WsBitUtils::setBit(byte2, 0, 0);
			WsStringUtils::addChar(sendData, byte2);

			unsigned short dataLength = text.length();
			WsStringUtils::addChar(sendData, WsBitUtils::getUnsignedIntByte(dataLength, 0));
			WsStringUtils::addChar(sendData, WsBitUtils::getUnsignedIntByte(dataLength, 1));
		}
		else
		{
			char byte2 = 127;
			WsBitUtils::setBit(byte2, 0, 1);
			WsStringUtils::addChar(sendData, byte2);

			unsigned long dataLength = text.length();
			for (int i = 0; i < 8; i++)
			{
				WsStringUtils::addChar(sendData, WsBitUtils::getUnsignedLongByte(dataLength, 1));
			}
		}

		sendData = sendData + text;
		::send(m_socket, sendData.c_str(), sendData.length(), 0);
	}

	void sendClose()
	{
		char byte1 = WsBitUtils::getChar("10001000");

		string sendData;
		WsStringUtils::addChar(sendData, byte1);

		char byte2 = 0;
		WsBitUtils::setBit(byte2, 0, 0);
		WsStringUtils::addChar(sendData, byte2);
		
		::send(m_socket, sendData.c_str(), sendData.length(), 0);
	}

private:

};


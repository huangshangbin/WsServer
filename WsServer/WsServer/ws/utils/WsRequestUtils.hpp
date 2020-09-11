#pragma once

#include "WsStringUtils.hpp"
#include "WsEnDecryptUtils.hpp"
#include "WsTypeConvertUtils.hpp"



class WsRequestUtils
{
public:
	static bool isValidConRequest(string& requestData)
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

	static void buildLink(string& requestBuffer, SOCKET socketHandle)
	{
		string key = WsStringUtils::splitStringGetOneStr(requestBuffer, "Sec-WebSocket-Key", 1);
		key = WsStringUtils::getStringUseCharStart(key, ' ');
		key = WsStringUtils::getStringUseCharEnd(key, '\r');

		string response = "HTTP/1.1 101 Switching Protocols\r\n"\
			"Upgrade:websocket\r\n"\
			"Connection:Upgrade\r\n"\
			"Sec-WebSocket-Accept:" + getAccept(key) + "\r\n"\
			"\r\n";

		::send(socketHandle, response.c_str(), response.length(), 0);
	}


private:
	static string getAccept(string key)
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

};
#pragma once

#include "WsConnect.hpp"

#include <deque>
#include <string>

using namespace std;

class WsService
{
public:
	virtual void open(deque<WsConnect>& connectList, WsConnect* curConect)
	{
		deque<string> keyList = curConect->getUrlParamKeyList();
		for (auto& key : keyList)
		{
			cout << key << "  = " << curConect->getUrlParam(key) << "  ;";
		}

		cout << endl;
	}


	virtual void message(deque<WsConnect>& connectList, WsConnect* curConect, string& reqMessage)
	{
	}


	virtual void close(deque<WsConnect>& connectList, WsConnect* curConect) 
	{
		cout << curConect->getUrlParam("name") << " �ر�" << endl;
	}


public:
	virtual void disConnect(deque<WsConnect>& connectList, WsConnect* curConect)
	{
		cout << curConect->getUrlParam("name") << " ���ӶϿ�" << endl;
	}
};


#pragma once

#include "WsConnect.hpp"

#include <deque>
#include <string>

using namespace std;

class WsService
{
public:
	virtual void open(deque<WsConnect>& connectList, WsConnect* curConnect) {}
	virtual void message(deque<WsConnect>& connectList, WsConnect* curConnect, string& reqMessage) {}

	virtual void close(deque<WsConnect>& connectList, WsConnect* curConnect)
	{
		curConnect->sendClose();
	}

	virtual void typeError(deque<WsConnect>& connectList, WsConnect* curConnect) {}


public:
	virtual void disConnect(deque<WsConnect>& connectList, WsConnect* curConect) {}
};


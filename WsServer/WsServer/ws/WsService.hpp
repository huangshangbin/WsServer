#pragma once


#include "WsSafeConList.hpp"

#include <string>

using namespace std;

class WsService
{
public:
	virtual void open(WsSafeConList* connectList, WsConnect* curConect) {}
	virtual void message(WsSafeConList* connectList, WsConnect* curConect, string& reqMessage) {}
	virtual void close(WsSafeConList* connectList, WsConnect* curConect) {}
};


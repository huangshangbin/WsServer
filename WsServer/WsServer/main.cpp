#include <iostream>

using namespace std;

#include <ws/WsServer.hpp>



class StudentService : public WsService
{
public:
	void open(deque<WsConnect>& connectList, WsConnect* curConnect)
	{
		deque<string> keyList = curConnect->getUrlParamKeyList();
		for (auto& key : keyList)
		{
			cout << key << "  = " << curConnect->getUrlParam(key) << "  ;";
		}

		cout << endl;

		for (auto& con : connectList)
		{
			if (con.getSocketHandle() != curConnect->getSocketHandle())
			{
				con.sendText(curConnect->getUrlParam("name") + " login");
			}
		}
	}
	
	void message(deque<WsConnect>& connectList, WsConnect* curConnect, string& reqMessage)
	{
		cout << curConnect->getUrlParam("name") << " reqMessage:" << reqMessage << endl;
		curConnect->sendText("recevie msg");

		for (auto& con : connectList)
		{
			if (con.getSocketHandle() != curConnect->getSocketHandle())
			{
				con.sendText(curConnect->getUrlParam("name") + " new msg : " + reqMessage);
			}
		}
	}

	void close(deque<WsConnect>& connectList, WsConnect* curConnect)
	{
		cout << curConnect->getUrlParam("name") << " ¹Ø±Õ" << endl;
		curConnect->sendClose();
	}

	void disConnect(deque<WsConnect>& connectList, WsConnect* curConect)
	{
		cout << curConect->getUrlParam("name") << " Á´½Ó¶Ï¿ª" << endl;
	}

};



class InfoService : public WsService
{
public:
	void message(deque<WsConnect>& connectList, WsConnect* curConnect, string& reqMessage)
	{
		curConnect->sendText("info service");
	}

};



int main()
{
	WsServer wsServer;
	wsServer.setMostThreadCount(8);

	wsServer.injectService("/{name}/{age}", new StudentService());
	wsServer.injectService("/info", new InfoService());


	wsServer.listen("127.0.0.1", 4000);

	

	int a;
	cin >> a;
	return 0;
}
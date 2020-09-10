#include <iostream>

using namespace std;

#include <ws/WsServer.hpp>

int main()
{
	WsServer wsServer;
	wsServer.setMostThreadCount(8);

	wsServer.injectService("/", new WsService());


	wsServer.listen("127.0.0.1", 4000);

	

	int a;
	cin >> a;
	return 0;
}
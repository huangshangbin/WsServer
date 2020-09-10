#pragma once


#include "WsConnect.hpp"

#include <deque>
#include <mutex>


using namespace std;

class WsSafeConList
{
private:
	mutex m_mutex;

	deque<WsConnect*> m_conList;


public:
	void injectConnect(WsConnect* con)
	{
		lock_guard<mutex> lockGuard(m_mutex);

		m_conList.push_back(con);
	}

	WsConnect* getCon(int index)
	{
		lock_guard<mutex> lockGuard(m_mutex);

		return m_conList[index];
	}

	bool erase(SOCKET socketHandle)
	{
		lock_guard<mutex> lockGuard(m_mutex);

		for (auto it = m_conList.begin(); it != m_conList.end(); it++)
		{
			if ((*it)->getSocketHandle() == socketHandle)
			{
				WsConnect* con = *it;
				m_conList.erase(it);

				delete con;

				return true;
			}
		}

		return false;
	}

	bool isExistConnect(SOCKET socketHandle)
	{
		lock_guard<mutex> lockGuard(m_mutex);

		for (auto it = m_conList.begin(); it != m_conList.end(); it++)
		{
			if ((*it)->getSocketHandle() == socketHandle)
			{
				return true;
			}
		}

		return false;
	}
};
#pragma once

#include "WsStringUtils.hpp"

#include <deque>
#include <string>
using namespace std;

class WsPathUtils
{
public:
	static bool isMatch(string path, string restfulPath)
	{
		deque<string> pathList = WsStringUtils::splitString(path, "/");
		deque<string> restfulPathList = WsStringUtils::splitString(restfulPath, "/");
		if (pathList.size() != restfulPathList.size())
		{
			return false;
		}

		for (int i = 0; i < pathList.size(); i++)
		{
			if ((restfulPathList[i][0] == '{') && (restfulPathList[i][restfulPathList[i].size() - 1] == '}'))
			{
				continue;
			}

			if (pathList[i] != restfulPathList[i])
			{
				return false;
			}
		}

		return true;
	}

	static map<string, string> getParam(string path, string restfulPath)
	{
		map<string, string> paramMap;

		deque<string> pathList = WsStringUtils::splitString(path, "/");
		deque<string> restfulPathList = WsStringUtils::splitString(restfulPath, "/");

		for (int i = 0; i < pathList.size(); i++)
		{
			if ((restfulPathList[i][0] == '{'))
			{
				string key = WsStringUtils::getStringUsePos(restfulPathList[i], 1, restfulPathList[i].length() - 2);
				paramMap[key] = pathList[i];
			}
		}

		return std::move(paramMap);
	}
};
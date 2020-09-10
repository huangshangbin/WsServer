#pragma once

#include <deque>
#include <string>
#include <windows.h>

#include <iostream>
using namespace std;


class WsStringUtils
{
public:
	static deque<string> splitString(string srcStr, string splitStr)
	{
		deque<string> dataList;
		if (srcStr.length() == 0)
		{
			return std::move(dataList);
		}

		string str = srcStr;

		string::size_type pos1, pos2;
		pos2 = str.find(splitStr);
		pos1 = 0;
		while (string::npos != pos2)
		{
			dataList.push_back(str.substr(pos1, pos2 - pos1));

			pos1 = pos2 + splitStr.size();
			pos2 = str.find(splitStr, pos1);
		}
		dataList.push_back(str.substr(pos1));

		return std::move(dataList);
	}

	static string splitStringGetOneStr(string srcStr, string splitStr, int index)
	{
		deque<string> strList = splitString(srcStr, splitStr);
		if (index >= strList.size() || index < 0)
		{
			return "";
		}

		return strList[index];
	}

	static bool isExistStringInString(string srcStr, string existStr)
	{
		deque<string> srcStrList = splitString(srcStr, existStr);
		if (srcStrList.size() < 2)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	static bool isEqualString(string srcStr1, string srcStr2)
	{
		if (srcStr1 == srcStr2)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	static bool isUseStringEnd(string srcStr, string endStr)
	{
		string compareCS = getStringUsePos(srcStr, srcStr.length() - endStr.length(), srcStr.length());

		return isEqualString(compareCS, endStr);
	}

	static string getStringUsePos(string src, int startPos, int endPos)
	{
		return src.substr(startPos, endPos - startPos + 1);
	}

	static string getStringUseCharStart(string src, char startChar)
	{
		int index = -1;
		for (int i = 0; i < src.length(); i++)
		{
			if (src[i] == startChar)
			{
				index = i;
				break;
			}
		}
		if (index == -1)
		{
			return "";
		}

		return getStringUsePos(src, index + 1, src.length());
	}

	static string getStringUseCharEnd(string src, char endChar)
	{
		int index = -1;
		for (int i = 0; i < src.length(); i++)
		{
			if (src[i] == endChar)
			{
				index = i;
				break;
			}
		}
		if (index == -1)
		{
			return src;
		}

		return getStringUsePos(src, 0, index - 1);
	}

	static string getStringDeleteStartNone(string srcStr)
	{
		int index;
		for (int i = 0; i < srcStr.length(); i++)
		{
			if (srcStr[i] != ' ')
			{
				index = i;
				break;
			}
		}

		return getStringUsePos(srcStr, index, srcStr.length());
	}

	static void addChar(string& srcStr, char ch)
	{
		string tempStr(1, ch);

		srcStr = srcStr + tempStr;
	}

	static string getUtf8UseGbk(string content)
	{
		string strGBK = content;

		int len = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
		unsigned short * wszUtf8 = new unsigned short[len + 1];
		memset(wszUtf8, 0, len * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, (LPWSTR)wszUtf8, len);

		len = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)wszUtf8, -1, NULL, 0, NULL, NULL);
		char *szUtf8 = new char[len + 1];
		memset(szUtf8, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)wszUtf8, -1, szUtf8, len, NULL, NULL);

		content = szUtf8;

		delete[] szUtf8;
		delete[] wszUtf8;

		return content;
	}
};


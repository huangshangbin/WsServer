#pragma once

#include "WsStringUtils.hpp"
#include "WsBitUtils.hpp"

#include <deque>
#include <string>
using namespace std;


//request dataFrame
class WsDataFrameUtils
{
public:
	static bool isComplete(string& dataFrame)
	{
		int msgLengthMark = WsBitUtils::getIntUseCharPos(dataFrame[1], 1, 7);
		if (msgLengthMark < 126)
		{
			return dataFrame.length() == (2 + 4 + msgLengthMark);
		}
		else if (msgLengthMark == 126)
		{
			int msgLength = WsBitUtils::getUnsignShort(dataFrame[2], dataFrame[3]);
			return dataFrame.length() == (2 + 2 + 4 + msgLength);
		}
		else
		{
			int msgLength = WsBitUtils::getUnsignLongLong(dataFrame[2], dataFrame[3], dataFrame[4], dataFrame[5],
				dataFrame[6], dataFrame[7], dataFrame[8], dataFrame[9]);

			return dataFrame.length() == (2 + 8 + 4 + msgLength);
		}
	}

	static int getType(string& dataFrame)
	{
		return WsBitUtils::getIntUseCharPos(dataFrame[0], 4, 7);
	}

	static string getMsg(string& dataFrame)
	{
		int msgLengthMark = WsBitUtils::getIntUseCharPos(dataFrame[1], 1, 7);

		int maskIndex;

		if (msgLengthMark < 126)
		{
			maskIndex = 2;
		}
		else if (msgLengthMark == 126)
		{
			maskIndex = 4;
		}
		else
		{
			maskIndex = 10;
		}

		string mask = WsStringUtils::getStringUsePos(dataFrame, maskIndex, maskIndex + 3);
		string maskMsg = WsStringUtils::getStringUsePos(dataFrame, maskIndex + 4, dataFrame.length() - 1);

		string msg;
		for (int i = 0; i < maskMsg.length(); i++)
		{
			char msgNode = maskMsg[i] ^ (mask[i % 4]);
			WsStringUtils::addChar(msg, msgNode);
		}

		return msg;
	}
};
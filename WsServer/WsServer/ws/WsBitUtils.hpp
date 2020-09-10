#pragma once

#include <bitset>
using namespace std;

class WsBitUtils
{
public:
	static int getIntUseCharPos(char ch, int startPos, int endPos)//包括 startPos endPos下标
	{
		bitset<8> bitList(ch);

		bitList >>= (7 - endPos); //右移

		int setZeroCount = startPos + 7 - endPos;
		int zeroIndex = 7;
		for (int i = 0; i < setZeroCount; i++)
		{
			bitList.reset(zeroIndex);
			zeroIndex--;
		}

		return bitList.to_ulong();
	}

	static char getChar(string str)
	{
		bitset<8> bitList(str);

		return (int)bitList.to_ulong();
	}

	static unsigned short getUnsignShort(char byte1, char byte2)
	{
		unsigned short value;
		char* pValue = (char*)&value;

		*pValue = byte2;
		*(pValue + 1) = byte1;

		return value;
	}

	static unsigned int getUnsignInt(char byte1, char byte2, char byte3, char byte4)//unsign long is the same
	{
		unsigned int value;
		char* pValue = (char*)&value;

		*pValue = byte4;
		*(pValue + 1) = byte3;
		*(pValue + 2) = byte2;
		*(pValue + 3) = byte1;

		return value;
	}

	static unsigned long long getUnsignLongLong(char byte1, char byte2, char byte3, char byte4, char byte5,
		char byte6, char byte7, char byte8)//unsign long is the same
	{
		unsigned long long value;
		char* pValue = (char*)&value;

		*pValue = byte8;
		*(pValue + 1) = byte7;
		*(pValue + 2) = byte6;
		*(pValue + 3) = byte5;
		*(pValue + 4) = byte4;
		*(pValue + 5) = byte3;
		*(pValue + 6) = byte2;
		*(pValue + 7) = byte1;

		return value;
	}

	static unsigned char getUnsignedShortByte(unsigned short value, int index)
	{
		unsigned char* pValue = (unsigned char*)&value;
		return pValue[1 - index];
	}

	static unsigned char getUnsignedIntByte(unsigned int value, int index)//unsigned int转char可强转
	{
		unsigned char* pValue = (unsigned char*)&value;
		return pValue[3 - index];
	}

	static unsigned char getUnsignedLongByte(unsigned long value, int index)//unsigned int转char可强转
	{
		unsigned char* pValue = (unsigned char*)&value;
		return pValue[7 - index];
	}

	static void exchangeBit(bitset<8>& bitList, int index1, int index2)
	{
		int temp = bitList[index1];
		bitList[index1] = bitList[index2];
		bitList[index2] = temp;
	}

	static void setBit(char& ch, int index, int value)
	{
		index = 7 - index;
		if (value)
		{
			ch |= (1 << index);
		}
		else
		{
			ch &= ~(1 << index);
		}
	}

	static string getBitString(char ch)
	{
		bitset<8> bit1List(ch);
		return bit1List.to_string();
	}
};
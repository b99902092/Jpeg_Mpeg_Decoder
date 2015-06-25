#pragma once
#include <iostream>
#include <map>
#include <unordered_map>

using namespace std;

typedef unsigned long long int	Byte8;
typedef unsigned int			Byte4;
typedef unsigned short			Byte2;
typedef unsigned char			Byte;

template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std
{
	template<typename S, typename T> struct hash<pair<S, T>>
	{
		inline size_t operator()(const pair<S, T> & v) const
		{
			size_t seed = 0;
			::hash_combine(seed, v.first);
			::hash_combine(seed, v.second);
			return seed;
		}
	};
}

class TableHandler{
public:
	static int zigzagOrder[64];
	static Byte defaultIntraQuantizerMatrix[64];
	static Byte defaultNonIntraQuantizerMatrix[64];
	static unordered_map<int, double> pictureRateTable;
	static unordered_map<pair<int, int>, int> macroblockAddressingTable;		//2-B.1
	static unordered_map<pair<int, int>, Byte> macroblockTypeTable_I;		//2-B.2a
	static unordered_map<pair<int, int>, Byte> macroblockTypeTable_P;		//2-B.2b
	static unordered_map<pair<int, int>, Byte> macroblockTypeTable_B;		//2-B.2c
	static unordered_map<pair<int, int>, Byte> macroblockTypeTable_D;		//2-B.2d
	static unordered_map<pair<int, int>, Byte> codedBlockPatternTable;		//2-B.3
	static unordered_map<pair<int, int>, int> motionVectorTable;				//2-B.4
	static unordered_map<pair<int, int>, int> dcSizeLuminanceTable;			//2-B.5a
	static unordered_map<pair<int, int>, int> dcSizeChrominanceTable;		//2-B.5b
	static unordered_map<pair<int, int>, pair<int, int>> dctCoefficientTable;//2-B.5c~5f
	static unordered_map<pair<int, int>, int> dctCoefficientEscapeTable;  //2-B.5g

	static void initialize();
	static void initPictureRate();
	static void initMacroblockAddressing();
	static void initMacroblockType();
	static void initCodedBlockPattern();
	static void initMotinoVector();
	static void initDCSize();
	static void initDCTCoefficient();

	static bool checkPictureRateTable(int pictureRateCode, double &pictureRate);
	static bool checkMacroblockAddressingTable(int length, int code,int &address);
	static bool checkMacroblockTypeTable(int type,int length,int code,Byte& macroBlockType);
	static bool checkCodedBlockPatternTable(int length,int code,Byte& blockPattern);
	static bool checkMotionVectorTable(int length,int code,int &motionVector);
	static bool checkDcSize(int LC,int length,int code,Byte &dcSize);
	static bool checkDctCoefficientTable(int length,int code,int &run,int &level);
	static bool checkDctCoefficientEscapeTable(int length, int code, int &level);

	TableHandler();
	~TableHandler();
};


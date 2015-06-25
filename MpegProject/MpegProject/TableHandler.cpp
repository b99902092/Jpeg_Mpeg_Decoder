#include "TableHandler.h"

int TableHandler::zigzagOrder[64]{
		0, 1, 5, 6, 14, 15, 27, 28,
		2, 4, 7, 13, 16, 26, 29, 42,
		3, 8, 12, 17, 25, 30, 41, 43,
		9, 11, 18, 24, 31, 40, 44, 53,
		10, 19, 23, 32, 39, 45, 52, 54,
		20, 22, 33, 38, 46, 51, 55, 60,
		21, 34, 37, 47, 50, 56, 59, 61,
		35, 36, 48, 49, 57, 58, 62, 63
};

Byte TableHandler::defaultIntraQuantizerMatrix[64]{
		8, 16, 19, 22, 26, 27, 29, 34,
		16, 16, 22, 24, 27, 29, 34, 37,
		19, 22, 26, 27, 29, 34, 34, 38,
		22, 22, 26, 27, 29, 34, 37, 40,
		22, 26, 27, 29, 32, 35, 40, 48,
		26, 27, 29, 32, 35, 40, 48, 58,
		26, 27, 29, 34, 38, 46, 56, 69,
		27, 29, 35, 38, 46, 56, 69, 83
};

Byte TableHandler::defaultNonIntraQuantizerMatrix[64]{
		16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16
};

unordered_map<int, double> TableHandler::pictureRateTable;
unordered_map<pair<int, int>, int> TableHandler::macroblockAddressingTable;		//2-B.1
unordered_map<pair<int, int>, Byte> TableHandler::macroblockTypeTable_I;		//2-B.2a
unordered_map<pair<int, int>, Byte> TableHandler::macroblockTypeTable_P;		//2-B.2b
unordered_map<pair<int, int>, Byte> TableHandler::macroblockTypeTable_B;		//2-B.2c
unordered_map<pair<int, int>, Byte> TableHandler::macroblockTypeTable_D;		//2-B.2d
unordered_map<pair<int, int>, Byte> TableHandler::codedBlockPatternTable;		//2-B.3
unordered_map<pair<int, int>, int> TableHandler::motionVectorTable;				//2-B.4
unordered_map<pair<int, int>, int> TableHandler::dcSizeLuminanceTable;			//2-B.5a
unordered_map<pair<int, int>, int> TableHandler::dcSizeChrominanceTable;		//2-B.5b
unordered_map<pair<int, int>, pair<int, int>> TableHandler::dctCoefficientTable;//2-B.5c~5g
unordered_map<pair<int, int>, int> TableHandler::dctCoefficientEscapeTable;  //2-B.5g

TableHandler::TableHandler(){
}


TableHandler::~TableHandler(){
}

bool TableHandler::checkPictureRateTable(int pictureRateCode, double &pictureRate){
	unordered_map<int, double>::iterator iter = pictureRateTable.find(pictureRateCode);
	if (iter == pictureRateTable.end()){
		return false;
	}
	else{
		pictureRate = iter->second;
		return true;
	}
}

bool TableHandler::checkMacroblockAddressingTable(int length, int code, int &address){
	unordered_map<pair<int, int>, int>::iterator iter = macroblockAddressingTable.find(pair<int, int>(length, code));
	if (iter == macroblockAddressingTable.end()){
		return false;
	}
	else{
		address = iter->second;
		return true;
	}
}
bool TableHandler::checkMacroblockTypeTable(int type, int length, int code, Byte& macroBlockType){
	unordered_map<pair<int, int>, Byte>::iterator iter;
	switch (type){
		case 1:
			iter = macroblockTypeTable_I.find(pair<int, int>(length, code));
			if (iter == macroblockTypeTable_I.end()){
				return false;
			}
			break;
		case 2:
			iter = macroblockTypeTable_P.find(pair<int, int>(length, code));
			if (iter == macroblockTypeTable_P.end()){
				return false;
			}
			break;
		case 3:
			iter = macroblockTypeTable_B.find(pair<int, int>(length, code));
			if (iter == macroblockTypeTable_B.end()){
				return false;
			}
			break;
		case 4:
			iter = macroblockTypeTable_D.find(pair<int, int>(length, code));
			if (iter == macroblockTypeTable_D.end()){
				return false;
			}
			break;
		default:
			cerr << "In checking macroblock type table: something must went wrong!!\n";
			exit(1);
			break;
	}
	//(length,code) key is found
	macroBlockType = iter->second;
	return true;
}
bool TableHandler::checkCodedBlockPatternTable(int length, int code, Byte& blockPattern){
	unordered_map<pair<int, int>, Byte>::iterator iter = codedBlockPatternTable.find(pair<int, int>(length, code));
	if (iter == codedBlockPatternTable.end()){
		return false;
	}
	else{
		blockPattern = iter->second;
		return true;
	}
}
bool TableHandler::checkMotionVectorTable(int length, int code, int &motionVector){
	unordered_map<pair<int, int>, int>::iterator iter = motionVectorTable.find(pair<int, int>(length, code));
	if (iter == motionVectorTable.end()){
		return false;
	}
	else{
		motionVector = iter->second;
		return true;
	}
}
bool TableHandler::checkDcSize(int LC, int length, int code, Byte &dcSize){
	unordered_map<pair<int, int>, int>::iterator iter;
	if (LC == 0){
		//Luminance
		iter = dcSizeLuminanceTable.find(pair<int, int>(length, code));
		if (iter == dcSizeLuminanceTable.end()){
			return false;
		}
	}
	else{
		//Chrominance
		iter = dcSizeChrominanceTable.find(pair<int, int>(length, code));
		if (iter == dcSizeChrominanceTable.end()){
			return false;
		}
	}
	//success!
	dcSize = iter->second;
	return true;
}
bool TableHandler::checkDctCoefficientTable(int length, int code, int &run, int &level){
	unordered_map<pair<int, int>, pair<int, int>>::iterator iter = dctCoefficientTable.find(pair<int, int>(length, code));
	if (iter == dctCoefficientTable.end()){
		return false;
	}
	else{
		pair<int, int> value = iter->second;
		run = value.first;
		level = value.second;
		return true;
	}
}

bool TableHandler::checkDctCoefficientEscapeTable(int length, int code, int &level){
	unordered_map<pair<int, int>, int>::iterator iter = dctCoefficientEscapeTable.find(pair<int, int>(length, code));
	if (iter == dctCoefficientEscapeTable.end()){
		return false;
	}
	else{
		level = iter->second;
		return true;
	}
}

void TableHandler::initialize(){
	initPictureRate();
	initMacroblockAddressing();
	initMacroblockType();
	initCodedBlockPattern();
	initMotinoVector();
	initDCSize();
	initDCTCoefficient();
}

void TableHandler::initPictureRate(){
	pictureRateTable.insert(pair<int, double>(0x1, 23.976));
	pictureRateTable.insert(pair<int, double>(0x2, 24.0));
	pictureRateTable.insert(pair<int, double>(0x3, 25.0));
	pictureRateTable.insert(pair<int, double>(0x4, 29.97));
	pictureRateTable.insert(pair<int, double>(0x5, 30.0));
	pictureRateTable.insert(pair<int, double>(0x6, 50.0));
	pictureRateTable.insert(pair<int, double>(0x7, 59.94));
	pictureRateTable.insert(pair<int, double>(0x8, 60.0));
}

void TableHandler::initMacroblockAddressing(){
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(1, 1), 1));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(3, 3), 2));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(3, 2), 3));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(4, 3), 4));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(4, 2), 5));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(5, 3), 6));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(5, 2), 7));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(7, 7), 8));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(7, 6), 9));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 11), 10));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 10), 11));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 9), 12));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 8), 13));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 7), 14));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 6), 15));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 23), 16));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 22), 17));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 21), 18));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 20), 19));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 19), 20));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 18), 21));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 35), 22));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 34), 23));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 33), 24));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 32), 25));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 31), 26));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 30), 27));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 29), 28));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 28), 29));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 27), 30));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 26), 31));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 25), 32));
	macroblockAddressingTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 24), 33));
}
void TableHandler::initMacroblockType(){
	//I frame
	macroblockTypeTable_I.insert(pair<pair<int, int>, Byte>(pair<int, int>(1, 1), (Byte)1U));
	macroblockTypeTable_I.insert(pair<pair<int, int>, Byte>(pair<int, int>(2, 1), (Byte)17U));
	//P frame
	macroblockTypeTable_P.insert(pair<pair<int, int>, Byte>(pair<int, int>(1, 1), (Byte)10U));
	macroblockTypeTable_P.insert(pair<pair<int, int>, Byte>(pair<int, int>(2, 1), (Byte)2U));
	macroblockTypeTable_P.insert(pair<pair<int, int>, Byte>(pair<int, int>(3, 1), (Byte)8U));
	macroblockTypeTable_P.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 3), (Byte)1U));
	macroblockTypeTable_P.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 2), (Byte)26U));
	macroblockTypeTable_P.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 1), (Byte)18U));
	macroblockTypeTable_P.insert(pair<pair<int, int>, Byte>(pair<int, int>(6, 1), (Byte)17U));
	//B frame
	macroblockTypeTable_B.insert(pair<pair<int, int>, Byte>(pair<int, int>(2, 2), (Byte)12U));
	macroblockTypeTable_B.insert(pair<pair<int, int>, Byte>(pair<int, int>(2, 3), (Byte)14U));
	macroblockTypeTable_B.insert(pair<pair<int, int>, Byte>(pair<int, int>(3, 2), (Byte)4U));
	macroblockTypeTable_B.insert(pair<pair<int, int>, Byte>(pair<int, int>(3, 3), (Byte)6U));
	macroblockTypeTable_B.insert(pair<pair<int, int>, Byte>(pair<int, int>(4, 2), (Byte)8U));
	macroblockTypeTable_B.insert(pair<pair<int, int>, Byte>(pair<int, int>(4, 3), (Byte)10U));
	macroblockTypeTable_B.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 3), (Byte)1U));
	macroblockTypeTable_B.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 2), (Byte)30U));
	macroblockTypeTable_B.insert(pair<pair<int, int>, Byte>(pair<int, int>(6, 3), (Byte)26U));
	macroblockTypeTable_B.insert(pair<pair<int, int>, Byte>(pair<int, int>(6, 2), (Byte)22U));
	macroblockTypeTable_B.insert(pair<pair<int, int>, Byte>(pair<int, int>(6, 1), (Byte)17U));
	//D frame
	macroblockTypeTable_D.insert(pair<pair<int, int>, Byte>(pair<int, int>(1, 1), (Byte)1U));
}
void TableHandler::initCodedBlockPattern(){
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(3, 7), (Byte)60U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(4, 13), (Byte)4U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(4, 12), (Byte)8U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(4, 11), (Byte)16U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(4, 10), (Byte)32U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 19), (Byte)12U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 18), (Byte)48U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 17), (Byte)20U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 16), (Byte)40U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 15), (Byte)28U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 14), (Byte)44U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 13), (Byte)52U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 12), (Byte)56U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 11), (Byte)1U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 10), (Byte)61U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 9), (Byte)2U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(5, 8), (Byte)62U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(6, 15), (Byte)24U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(6, 14), (Byte)36U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(6, 13), (Byte)3U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(6, 12), (Byte)63U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(7, 23), (Byte)5U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(7, 22), (Byte)9U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(7, 21), (Byte)17U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(7, 20), (Byte)33U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(7, 19), (Byte)6U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(7, 18), (Byte)10U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(7, 17), (Byte)18U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(7, 16), (Byte)34U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 31), (Byte)7U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 30), (Byte)11U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 29), (Byte)19U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 28), (Byte)35U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 27), (Byte)13U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 26), (Byte)49U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 25), (Byte)21U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 24), (Byte)41U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 23), (Byte)14U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 22), (Byte)50U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 21), (Byte)22U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 20), (Byte)42U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 19), (Byte)15U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 18), (Byte)51U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 17), (Byte)23U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 16), (Byte)43U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 15), (Byte)25U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 14), (Byte)37U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 13), (Byte)26U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 12), (Byte)38U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 11), (Byte)29U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 10), (Byte)45U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 9), (Byte)53U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 8), (Byte)57U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 7), (Byte)30U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 6), (Byte)46U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 5), (Byte)54U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(8, 4), (Byte)58U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(9, 7), (Byte)31U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(9, 6), (Byte)47U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(9, 5), (Byte)55U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(9, 4), (Byte)59U));
	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(9, 3), (Byte)27U));

	codedBlockPatternTable.insert(pair<pair<int, int>, Byte>(pair<int, int>(9, 2), (Byte)39U));
}
void TableHandler::initMotinoVector(){
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 25), -16));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 27), -15));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 29), -14));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 31), -13));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 33), -12));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 35), -11));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 19), -10));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 21), -9));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 23), -8));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 7), -7));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 9), -6));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 11), -5));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(7, 7), -4));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(5, 3), -3));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(4, 3), -2));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(3, 3), -1));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(1, 1), 0));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(3, 2), 1));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(4, 2), 2));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(5, 2), 3));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(7, 6), 4));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 10), 5));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 8), 6));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 6), 7));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 22), 8));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 20), 9));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(10, 18), 10));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 34), 11));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 32), 12));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 30), 13));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 28), 14));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 26), 15));
	motionVectorTable.insert(pair<pair<int, int>, int>(pair<int, int>(11, 24), 16));
}
void TableHandler::initDCSize(){
	//Luminance Table
	dcSizeLuminanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(3, 4), 0));
	dcSizeLuminanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(2, 0), 1));
	dcSizeLuminanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(2, 1), 2));
	dcSizeLuminanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(3, 5), 3));
	dcSizeLuminanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(3, 6), 4));
	dcSizeLuminanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(4, 14), 5));
	dcSizeLuminanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(5, 30), 6));
	dcSizeLuminanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(6, 62), 7));
	dcSizeLuminanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(7, 126), 8));
	//Chrominance Table
	dcSizeChrominanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(2, 0), 0));
	dcSizeChrominanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(2, 1), 1));
	dcSizeChrominanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(2, 2), 2));
	dcSizeChrominanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(3, 6), 3));
	dcSizeChrominanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(4, 14), 4));
	dcSizeChrominanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(5, 30), 5));
	dcSizeChrominanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(6, 62), 6));
	dcSizeChrominanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(7, 126), 7));
	dcSizeChrominanceTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, 254), 8));
}
void TableHandler::initDCTCoefficient(){
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(2, 3), pair<int, int>(0, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(3, 3), pair<int, int>(1, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(4, 4), pair<int, int>(0, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(4, 5), pair<int, int>(2, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(5, 5), pair<int, int>(0, 3)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(5, 7), pair<int, int>(3, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(5, 6), pair<int, int>(4, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(6, 6), pair<int, int>(1, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(6, 7), pair<int, int>(5, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(6, 5), pair<int, int>(6, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(6, 4), pair<int, int>(7, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(7, 6), pair<int, int>(0, 4)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(7, 4), pair<int, int>(2, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(7, 7), pair<int, int>(8, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(7, 5), pair<int, int>(9, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(6, 1), pair<int, int>(-1, -1))); //escape
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(8, 38), pair<int, int>(0, 5)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(8, 33), pair<int, int>(0, 6)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(8, 37), pair<int, int>(1, 3)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(8, 36), pair<int, int>(3, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(8, 39), pair<int, int>(10, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(8, 35), pair<int, int>(11, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(8, 34), pair<int, int>(12, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(8, 32), pair<int, int>(13, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(10, 10), pair<int, int>(0, 7)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(10, 12), pair<int, int>(1, 4)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(10, 11), pair<int, int>(2, 3)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(10, 15), pair<int, int>(4, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(10, 9), pair<int, int>(5, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(10, 14), pair<int, int>(14, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(10, 13), pair<int, int>(15, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(10, 8), pair<int, int>(16, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 29), pair<int, int>(0, 8)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 24), pair<int, int>(0, 9)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 19), pair<int, int>(0, 10)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 16), pair<int, int>(0, 11)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 27), pair<int, int>(1, 5)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 20), pair<int, int>(2, 4)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 28), pair<int, int>(3, 3)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 18), pair<int, int>(4, 3)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 30), pair<int, int>(6, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 21), pair<int, int>(7, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 17), pair<int, int>(8, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 31), pair<int, int>(17, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 26), pair<int, int>(18, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 25), pair<int, int>(19, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 23), pair<int, int>(20, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(12, 22), pair<int, int>(21, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 26), pair<int, int>(0, 12)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 25), pair<int, int>(0, 13)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 24), pair<int, int>(0, 14)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 23), pair<int, int>(0, 15)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 22), pair<int, int>(1, 6)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 21), pair<int, int>(1, 7)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 20), pair<int, int>(2, 5)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 19), pair<int, int>(3, 4)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 18), pair<int, int>(5, 3)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 17), pair<int, int>(9, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 16), pair<int, int>(10, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 31), pair<int, int>(22, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 30), pair<int, int>(23, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 29), pair<int, int>(24, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 28), pair<int, int>(25, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(13, 27), pair<int, int>(26, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 31), pair<int, int>(0, 16)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 30), pair<int, int>(0, 17)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 29), pair<int, int>(0, 18)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 28), pair<int, int>(0, 19)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 27), pair<int, int>(0, 20)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 26), pair<int, int>(0, 21)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 25), pair<int, int>(0, 22)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 24), pair<int, int>(0, 23)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 23), pair<int, int>(0, 24)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 22), pair<int, int>(0, 25)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 21), pair<int, int>(0, 26)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 20), pair<int, int>(0, 27)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 19), pair<int, int>(0, 28)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 18), pair<int, int>(0, 29)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 17), pair<int, int>(0, 30)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(14, 16), pair<int, int>(0, 31)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 24), pair<int, int>(0, 32)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 23), pair<int, int>(0, 33)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 22), pair<int, int>(0, 34)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 21), pair<int, int>(0, 35)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 20), pair<int, int>(0, 36)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 19), pair<int, int>(0, 37)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 18), pair<int, int>(0, 38)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 17), pair<int, int>(0, 39)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 16), pair<int, int>(0, 40)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 31), pair<int, int>(1, 8)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 30), pair<int, int>(1, 9)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 29), pair<int, int>(1, 10)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 28), pair<int, int>(1, 11)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 27), pair<int, int>(1, 12)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 26), pair<int, int>(1, 13)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(15, 25), pair<int, int>(1, 14)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 19), pair<int, int>(1, 15)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 18), pair<int, int>(1, 16)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 17), pair<int, int>(1, 17)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 16), pair<int, int>(1, 18)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 20), pair<int, int>(6, 3)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 26), pair<int, int>(11, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 25), pair<int, int>(12, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 24), pair<int, int>(13, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 23), pair<int, int>(14, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 22), pair<int, int>(15, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 21), pair<int, int>(16, 2)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 31), pair<int, int>(27, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 30), pair<int, int>(28, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 29), pair<int, int>(29, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 28), pair<int, int>(30, 1)));
	dctCoefficientTable.insert(pair<pair<int, int>, pair<int, int>>(pair<int, int>(16, 27), pair<int, int>(31, 1)));

	int value = 1;
	int code;

	value = -255;
	for (code = 0x8001; code <= 0x8080; code++){
		dctCoefficientEscapeTable.insert(pair<pair<int, int>, int>(pair<int, int>(16, code), value));
		value++;
	}
	//now value should be -127
	for (code = 0x81;code <= 0xFF;code++){
		dctCoefficientEscapeTable.insert(pair<pair<int, int>, int>(pair<int, int>(8, code), value));
		value++;
	}
	//now value should be 0, but 0 is forbidden
	value++;
	for (code = 0x01; code <= 0x7F; code++){
		dctCoefficientEscapeTable.insert(pair<pair<int, int>, int>(pair<int, int>(8,code), value));
		value++;
	}
	//now value should be 128
	for (code = 0x0080; code <= 0x00FF; code++){
		dctCoefficientEscapeTable.insert(pair<pair<int, int>, int>(pair<int, int>(16, code), value));
		value++;
	}
	//now value should be 256
}
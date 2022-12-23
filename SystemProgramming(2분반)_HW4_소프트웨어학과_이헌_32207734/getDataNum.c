/**
 * getDataNum.c : file that include function which count Word(intstruction) Number in line
 * @author		: Lee Heon (32207734)
 * @email		: knife967@gmail,com
 * @version		: 1.0
 * @date		: 2022.11.21 ~ 2022.12.09
**/

#include "myshell.h"


int getDataNum(char charBuffer[]) {
	int i, dataNum = 0;
	int len = strlen(charBuffer);
	
	//	명령어의 개수 찾기
	for (i = 0; i < len; i++) {
		if (charBuffer[i] == ' ') {
			dataNum++;
		}
	}
	
	dataNum++; 	// (단어(명령어)의 개수) = (공백의 개수) + 1

	return dataNum;
}

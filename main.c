/*
 * Blinky.c
 *
 * Created: 2/2/2018 2:24:21 PM
 * Author : dguard
 */

#include <avr/io.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <avr/interrupt.h>
//#include <util/delay.h>
#include "uart.h"

#define TRUE 1
#define FALSE 0
#define F_CPU 16000000UL
#define BAUD 9600

int falling = TRUE;
unsigned int downTime = 0;
unsigned int upTime = 0;
unsigned int timeDiff = 0;
unsigned int timeDiff2 = 0;
unsigned int numOverflow = 0;

enum Codes getCode(int depressed, int time); // Definition of getCode() 

enum Codes
{
	DOT=0, DASH, SPACE, NA
};
typedef struct pC{
	struct pC* ifDot;
	struct pC* ifDash;
	char thisPos;
}partialCode;

partialCode NONE = {&NONE, &NONE, '%'};
//Furthest level
partialCode Four = {&NONE, &NONE, '4'};
partialCode Five = {&NONE, &NONE, '5'};
partialCode H = {&Five, &Four, 'H'};
partialCode Three = {&NONE, &NONE, '3'};
partialCode V = {&NONE, &Three, 'V'};
partialCode F = {&NONE, &NONE, 'F'};
partialCode L = {&NONE, &NONE, 'L'};
partialCode P = {&NONE, &NONE, 'P'};
partialCode One = {&NONE, &NONE, '1'};
partialCode J = {&NONE, &One, 'J'};
partialCode Six = {&NONE, &NONE, '6'};
partialCode B = {&Six, &NONE, 'B'};
partialCode X = {&NONE, &NONE, 'X'};
partialCode C = {&NONE, &NONE, 'C'};
partialCode Y = {&NONE, &NONE, 'Y'};
partialCode Seven = {&NONE, &NONE, '7'};
partialCode Z = {&Seven, &NONE, 'Z'};
partialCode Q = {&NONE, &NONE, 'Q'};
partialCode Eight = {&NONE, &NONE, '8'};
partialCode placeHolder = {&Eight, &NONE, '?'};
partialCode Nine = {&NONE, &NONE, '9'};
partialCode Zero = {&NONE, &NONE, '0'};
partialCode anotherPlaceHolder = {&Nine, &Zero, '&'};
partialCode O = {&placeHolder, &anotherPlaceHolder, 'O'};
//Second Furthest
partialCode S = {&H, &V, 'S'};
partialCode Two = {&NONE, &NONE, '2'};
partialCode U = {&F, &Two, 'U'};
partialCode R = {&L, &NONE, 'R'};
partialCode W = {&P, &J, 'W'};
partialCode D = {&B, &X, 'D'};
partialCode K = {&C, &Y, 'K'};
partialCode G = {&Z, &Q, 'G'};
//Third Furthest
partialCode M = {&G, &O, 'M'};
partialCode N = {&D, &K, 'N'};
partialCode A = {&R, &W, 'A'};
partialCode I = {&S, &U, 'I'};
//Fourth Furthest
partialCode E = {&I, &A, 'E'};
partialCode T = {&N, &M, 'T'};



char getChar(enum Codes codeArray[]){
	int length = 0;
	int containsSpaces = FALSE;
	while(TRUE){
		if(codeArray[length] == SPACE){
			containsSpaces = TRUE;
		} else {
			//printf("!");
		}
		if(codeArray[length] != NA) length++;
		else break;
	}
	if(containsSpaces == TRUE){
		//printf("Made it");
		if(length == 1) return ' ';
		return '?';
	}
	partialCode * currentNode;
	if(codeArray[0] == DOT){
		currentNode = &E;
	} else if (codeArray[0] == DASH){
		currentNode = &T;
	} else {
		//It is NA
		return '?';
	}
	int index = 1;
	while(TRUE){
		if(index == length) return currentNode->thisPos;
		if(codeArray[index] == DOT){
			currentNode = currentNode->ifDot;
		} else if(codeArray[index]==DASH){
			currentNode = currentNode->ifDash;
		} else {
			return '?';
		}
		index++;
	}
	return '?';
}

void getCharUnitTests(){
	printf("Checking for errors in getChar()\n");
	enum Codes test[] = {DOT, DOT, DOT, DOT, NA};
	if(getChar(test) == 'H'){
		printf("Passed for H\n");
	} else {
		printf("Failed for H. Thinks it is (%c).\n",getChar(test));
	}
	enum Codes testNew[] = {DOT, DOT, DOT, DOT, SPACE, NA};
	if(getChar(testNew) == '?'){
		printf("Passed for odd space\n");
		} else {
		printf("Failed for odd space. Thinks it is (%c).\n",getChar(test));
	}
	enum Codes test2[] = {DASH, DASH, DASH, DASH, DASH, NA};
	if(getChar(test2) == '0'){
		printf("Passed for 0\n");
		} else {
		printf("Failed for 0. Thinks it is (%c).\n",getChar(test2));
	}
	enum Codes test3[] = {DASH, NA};
	if(getChar(test3) == 'T'){
		printf("Passed for T\n");
		} else {
		printf("Failed for T. Thinks it is (%c).\n",getChar(test3));
	}
	enum Codes test4[] = {SPACE, NA};
	if(getChar(test4) == ' '){
		printf("Passed for []\n");
		} else {
		printf("Failed for []. Thinks it is (%c).\n",getChar(test4));
	}
}

enum Codes listOfCodes[100];
void initializeCodeList(enum Codes codes[]){
	for(int i=0; i<100;i++){
		codes[i] = NA;
	}
}

char printCodeSoFar(enum Codes toAdd){
	int length = 0;
	char result = '*';
	int endOfWord = 0;
	if(toAdd == NA) return '?';
	//printf("\n");
	for(int i=0; i<100; i++){
		length++;
		if(listOfCodes[i] == NA){
			listOfCodes[i] = toAdd;
			break;
		} /*else if(listOfCodes[i] == SPACE){
			printf("[]");
		} else if(listOfCodes[i] == DOT){
			printf(".");
		} else if(listOfCodes[i] == DASH){
			printf("_");
		}*/
	}
	//printf("\n");
	//printf("Number of Codes : %d\n",length);
	enum Codes buffer[100];
	initializeCodeList(buffer);
	int dontPrint = FALSE;
	while(result != '?'){
		for(int i = 0; i <= endOfWord; i++){
			buffer[i] = listOfCodes[i];
		}
		if(result == getChar(buffer)){
			//printf("%c", '$');
			dontPrint = TRUE;
			break;
		}
		result = getChar(buffer);
		//printf("Result thus far : %c", result);
		//if(result == ' ') printf("[]");
		//printf("\n");
		endOfWord++;
	}
	if(result == '*') return '?';
	if(endOfWord == 0) return '?';
	//printf("buffer:{");
	initializeCodeList(buffer);
	for(int i = 0; i < endOfWord - 1; i++){
		buffer[i] = listOfCodes[i];
		//printf("%i",buffer[i]);
	}
	//printf("}\n");
	result = getChar(buffer);
	if(dontPrint == FALSE) endOfWord--;
	//printf("(%d,%d,%c)\n",length, endOfWord,result);
	if(length > endOfWord){
		printf("%c",result);
		for(int i = 0; i < 100-(endOfWord+1); i++){
			listOfCodes[i] = listOfCodes[i+endOfWord];
		}
		for(int i = 100-endOfWord - 1; i < 100; i++){
			listOfCodes[i] = NA;
		}
/*		for(int i=0; i<100; i++){
		length++;
		if(listOfCodes[i] == NA){
			break;
		}else if(listOfCodes[i] == SPACE){
			printf("[]");
		} else if(listOfCodes[i] == DOT){
			printf(".");
		} else if(listOfCodes[i] == DASH){
			printf("_");
		}
	}*/
		return result;
	} else {
		return '?';
	}
}

void printCodeSoFarUnitTest(){
	printf("Checking for errors in printCodeSoFar()\n");
	for(int i = 0; i < 3; i++){
		if(printCodeSoFar(DOT) != '?'){
			//printf("Failed at %d",i);
		}
	}
	char test = printCodeSoFar(SPACE);
	if(test != 'S'){
		printf("Failed at %c",test);
		return;
	}
	for(int i = 0; i < 3; i++){
		if(printCodeSoFar(DASH) != '?'){
			//printf("Failed at %d",i);
		}
	}
	test = printCodeSoFar(SPACE);
	if(test != 'O'){
		printf("Failed at %c",test);
		return;
	}
	for(int i = 0; i < 3; i++){
		if(printCodeSoFar(DOT) != '?'){
			//printf("Failed at %d",i);
		}
	}
	test = printCodeSoFar(SPACE);
	if(test != 'S'){
		printf("Failed at %c",test);
		return;
	}
	printf("\nPassed all unit tests!\n");
}
ISR(TIMER1_OVF_vect)
{
	numOverflow++;
}

ISR(TIMER1_CAPT_vect)
{
	if (falling == TRUE)
	{
		unsigned int currTick = (TCNT1L) | (TCNT1H << 8);
		downTime = currTick;

		if (numOverflow > 1) // check if more than one overflow occured
		{
			timeDiff2 = 30000;
		}
		else if (currTick < upTime) // adjust if only one overflow occured
		{
			unsigned int adjustment = 65535 - currTick;
			adjustment = adjustment - upTime;
			adjustment = adjustment + (2 * currTick);
			timeDiff2 = adjustment;
		}
		else
		{
			timeDiff2 = currTick - upTime;
		}

		// if (timeDiff2 >= 25000)
		// {
		// 	printf(" ");
		// }

		numOverflow = 0;

		getCode(FALSE, timeDiff2);

		// Captured Falling edge
		PORTB |= (1 << PINB5); // PORTB <- 0b111011111 == set pinb5 to zero ignore all others

		// Configure for rising edge
		TCCR1B |= (1 << ICES1);

		falling = FALSE;
	}
	else
	{

		unsigned int currTick = (TCNT1L) | (TCNT1H << 8);
		upTime = currTick;

		if (numOverflow > 1)
		{
			timeDiff = 30000; // check if more than one overflow occured
		}
		else if (currTick < downTime) // adjust if only one overflow occured
		{
			unsigned int adjustment = 65535 - currTick;
			adjustment = adjustment - downTime;
			adjustment = adjustment + (2 * currTick);
			timeDiff = adjustment;
		}
		else
		{
			timeDiff = currTick - downTime;
		}

		numOverflow = 0;

		getCode(TRUE, timeDiff);

		// if ((timeDiff >= 1875) && (timeDiff <= 12500)) // these numbers represent 'tick' number of 30ms and 200ms
		// {
		// 	printf(".");

		// 	PORTB |= (1 << PINB1);
		// 	_delay_ms(100);
		// 	PORTB &= ~(1 << PINB1);
		// 	PORTB |= (1 << PINB2);
		// }
		// else if (timeDiff > 12500)
		// {
		// 	printf("-");

		// 	PORTB |= (1 << PINB2);
		// 	_delay_ms(100);
		// 	PORTB &= ~(1 << PINB2);
		// 	PORTB |= (1 << PINB1);
		// }

		// Captured Rising edge
		PORTB &= ~(1 << PINB5);

		// Configure for falling edge
		TCCR1B &= ~(1 << ICES1);

		falling = TRUE;

	}
}

enum Codes getCode(int depressed, int time)
{
	if (depressed == TRUE)
	{
		if ((time >= 1875) && (time <= 12500)) // these numbers represent 'tick' number of 30ms and 200ms
		{
			//printf(".");
			printCodeSoFar(DOT);
			PORTB |= (1 << PINB1);
			//_delay_ms(100); PUT THIS BACK IF IT ISN'T WORKING
			PORTB &= ~(1 << PINB1);
			PORTB |= (1 << PINB2);
			return DOT;
		}
		else if (timeDiff > 12500)
		{
			//printf("-");
			printCodeSoFar(DASH);
			PORTB |= (1 << PINB2);
			//_delay_ms(100); PUT THIS BACK IF IT ISN'T WORKING
			PORTB &= ~(1 << PINB2);
			PORTB |= (1 << PINB1);
			return DASH;
		}
	}
	else // never reach this right now
	{
		if (time >= 25000) // this number represents 'tick' number of 400ms
		{
			//printf(" ");
			printCodeSoFar(SPACE);
			PORTB |= (1 << PINB1) | (1 << PINB2);
			return SPACE;
		}
	}

	return NA;
}

int main(void)
{
	uart_init();
	getCharUnitTests();
	initializeCodeList(listOfCodes);
	printCodeSoFarUnitTest();
	printf("Starting Program\n");
	
	DDRB |= (1 << PINB5);
	DDRB |= (1 << PINB1);
	DDRB |= (1 << PINB2);

	PORTB = 0b110;

//Configure for falling first
	TCCR1B &= ~(1 << ICES1);

//Set clock
	TCCR1B |= (1 << CS12);	//256 PreScaler
	TIMSK1 |= (1 << ICIE1) | (1 << TOIE1);

	sei();
	while (1)
		;
}

/*
 * strings.h
 *
 *  Created on: Mar 5, 2020
 *      Author: desarrollo-1
 */

#ifndef STRINGS_H_
#define STRINGS_H_


#include "PE_Types.h"

unsigned char compareString(unsigned char *a,unsigned char *b);
void getcharFromUChar(unsigned char n,unsigned char *cc);
//unsigned char TextInsertSpace2(unsigned char indice);
void getCharsFromUINT_var(unsigned char *p,unsigned short int var);
//void convertir_Voltaje_A_Chars(double voltaje,unsigned char *N,unsigned char tarjeta);
unsigned char   voltagePositivo_a_Chars(double voltaje,unsigned char *N);
unsigned char convertir_Voltaje_A_Chars(double voltaje,unsigned char *N,unsigned char tarjeta);
unsigned char convertir_SignedShortInt_A_Char(signed short int ss,unsigned char *p);
unsigned short int convertCharstoUINT(unsigned char *p);
void getUChar2Chars(unsigned char *p,unsigned char n ,char modoAscii);
unsigned char findLastChar(unsigned char *s,unsigned char size);
void TextInsertSpace(unsigned char indice);
unsigned char TextBackSpace(unsigned char indice);
word length(unsigned char *c,unsigned char size);
word length2(unsigned char *c);
unsigned char isNumLetter(char a);
unsigned char isNum(char a);
void convertNum2Chars(unsigned char *p,unsigned char size);
unsigned char Procesamiento_con_Signo(unsigned char *p);
unsigned char Procesamiento_con_Signo2(unsigned char *p,unsigned char signo);
unsigned char isNumAscii(char a);
void convertir_Binary_to_BCD(unsigned char *p,unsigned short int var);
void suma_BCD(unsigned char dm1,unsigned char m1,unsigned char c1,unsigned char d1,unsigned char u1,unsigned char *p);
void suma_BCD_digito(unsigned char op,unsigned char n,unsigned char *p);
void getASCII_from_UChar(unsigned char n,unsigned char *p);
void procesar_ASCII(unsigned char *p,unsigned char op,unsigned char digito);
unsigned char convert_ASCII_to_char(unsigned char *p);
void operacion_ASCII_Phase2(unsigned char digito,unsigned char op,unsigned char *p);	
void getASCII_from_Fase(unsigned char *p,unsigned char fase,unsigned char frac);
void setASCII_to_Fase(unsigned char *p,unsigned char *fase,unsigned char *frac);
unsigned short int sizeof1(unsigned char *p,unsigned short int size);
void vaciar_A2B(unsigned char *a,unsigned char *b,unsigned char i,unsigned char f);
void cleanArray(unsigned char *arr,unsigned char size,unsigned char k);
void swapArrays(unsigned char *a,unsigned char *b,unsigned char size);
void cleanArrayName(unsigned char *a,unsigned char size,unsigned char k,unsigned char m);
void insertChar(unsigned char *s,unsigned char size,unsigned char index,unsigned char c);
void cleanArray_int16(int16 *arr,uint8 size,uint8 k);
void cleanArray_volatile_int16(volatile int16 *arr,volatile uint8 size,uint8 k);

#endif /* STRINGS_H_ */

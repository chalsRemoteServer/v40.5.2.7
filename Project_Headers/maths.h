/*
 * maths.h
 *
 *  Created on: Mar 5, 2020
 *      Author: desarrollo-1
 */

#ifndef MATHS_H_
#define MATHS_H_



#endif /* MATHS_H_ */



void operacionPhase(unsigned char digito,unsigned char op);
void operacionUShortInt(unsigned short int *n,unsigned char digito,unsigned char op);
signed short int getMaximun(unsigned char zoom);
float redondearFloat(float f);
double convertir_ADC_word_A_miliVolt(unsigned short int ADCval,unsigned char channel);
double absd(double m);//get absoluto from double
float absf(float m);//get absolute from float
signed short int absSSInt(signed short int a);//get the absolute from signed short int
unsigned char getCheckSUM(unsigned char cmd, unsigned char *data, unsigned char len);
unsigned char FIFOcount(unsigned char var,unsigned char op);
void getBytes_from_SSInt(unsigned char *byteMSB,unsigned char *byteLSB,signed short int ssi);
unsigned char getSigno_Convert(signed short int *n);
unsigned char is_get_number_from_pixel(unsigned char *x,unsigned char  *y);
void re_Calcular_Buffers_DDS_Modificar_Uno(unsigned char zoom1);
void re_Calcular_Buffers_DDS(unsigned char zoom_wished);
unsigned char get_Coord_Num(unsigned char xn,unsigned char bit);
unsigned char get_cuadrante(unsigned char xi,unsigned char yi);
void reconvertir_coord_reducir_uno(unsigned char xn,unsigned char yn,unsigned char cuadrante,unsigned char *xr,unsigned char *yr,unsigned char palanca);
unsigned char getCheckSUM(unsigned char cmd, unsigned char *data, unsigned char len);

/*
 * fecqController.h
 *
 *  Created on: Jul 27, 2015
 *      Author: chals
 *      
 *      Controllador de las frecuencias de configuracion 
 *      de los calculos  blabla
 */

#ifndef FECQCONTROLLER_H_
#define FECQCONTROLLER_H_

/*char  1byte
 * short 2 byte
 * long  4 byte
 * float  byte
 * double 8 to 12 byte floating point numbers
 * 
 * 
 * 
 * 
 * */
#endif /* FECQCONTROLLER_H_ */



#define MSB_HI 0  // hibyte-hiword
#define MSB_LO 1 //    0123->ORDEN DE BYTES
#define LSB_HI 2
#define LSB_LO 3

//#define HI  0
//#define LO  1

typedef union{
	unsigned long int word32;
	unsigned char byte[4];	
}word32;

typedef union{//access word: 
	unsigned  short int word16;   //   	0xaabb   
	unsigned char byte[2];        //byte[0]=aa,byte[1]=bb
}word16;




//declaacion de procedimentos funciones o metodos-----------------------------
void  init_Control_deFrecuencias(void);
void clockPulso(void);
void init_DDS(char modo);
void WriteRegisterAD9833(unsigned short int dat);
unsigned long int  getFrecuency(void); 
void vTask15_Control_de_Reloj_Analogo(void);
 
 
 
 

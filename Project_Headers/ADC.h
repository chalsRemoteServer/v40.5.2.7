/*
 * ADC.h
 *
 *  Created on: Oct 9, 2015
 *      Author: chals
 */

#ifndef ADC_H_
#define ADC_H_


#include "queue.h"

#define  NOBUSY  0x40 //indica que el pulso de busy del ADC  no ha sido recibido
#define  _BUSY_  0x41 //indica que el pulso del busy del ADC ha sido recibido
 
#define  POSXADCU8  24
#define  POSXADCU7  120
#define  POSYADC     4

#define RC_OFF()    (setReg8(CLRTC, 0xFEU))
#define RC_ON()     (setReg8(SETTC, 0x01U))
//#define CS0_ON()    (setReg8(SETTC, 0x08U)) 
//#define CS0_OFF()   (setReg8(CLRTC, 0xF7U))
#define CS1_ON()    (setReg8(SETAS, 0x08U)) 
#define CS1_OFF()   (setReg8(CLRAS, 0xF7U))

//constantes de la maquina de estados del control de adquisicion de datos de los ADC SPI
#define  ADC_1   0x31
#define  ADC_2   0x32
//#define  ADC3   0x33
//#define  ADC4   0x34

struct _ADC_{
	//volatile  unsigned char ADCstatus;//maquina de estados del control del adc adquisicion de datos
  volatile unsigned char ADCreads;//indica que se termino de leer ambos adc en el mismo ciclo de RC
};	



/********  method declarations ****************/
void displayADC_IRQ(void);//se interrumpe cada que seinterrpe el timer porque dentro de ella pero pregunta si esta activada para ejecutar su codigo
void ADCsRead(signed short int *adc8,signed short int *adc7);
void init_ADC(void);
void Busy_Interrupt_IRQ(void);
//void ReadConvert(void);
//void push(unsigned char adc,unsigned short int data);
void SerialPeriferialInterface_IRQ(void);
void push8(unsigned short int data);
void push7(unsigned short int data);
void SPIselect(unsigned char device);
void adcSample_IRQ(void);
char Busy_status(char s,char modo);// almacena el estatus del Busy del ADC para ver si ya mandamos el reloj cada milisegundo
void disable_SM1(void);
void QSPI_OnTxChar_IRQ(void);
//unsigned char Read_QSPI2(signed short int *xx8,signed short int *xx7);
void Read_QSPI2(int16 *x,int16 *y);
void QSPI_On_TMR2_IRQ(void);


#endif /* ADC_H_ */


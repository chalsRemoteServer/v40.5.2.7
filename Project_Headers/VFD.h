/*
 * VFD.h
 *
 *  Created on: Mar 10, 2017
 *      Author: chals
 */

#ifndef VFD_H_
#define VFD_H_




#endif /* VFD_H_ */
/*
 * VFD.h
 *
 *  Created on: Jul 13, 2015
 *      Author: chals
 */

#ifndef VFD_H_
#define VFD_H_



#endif /* VFD_H_ */



#define keyEN  0x22U
#define keyDN  0x23U
#define keyLF  0x24U
#define keyRT  0x25U
#define keyUP  0x26U




#define FONTSIZE1   0x01U //6x8
#define FONTSIZE2   0x02U //8x16
#define FONTSIZE3   0x03U //12x24
#define FONTSIZE4   0x04U  //16x32

typedef union{//access word: 
	unsigned  short int coord16;   //   	0xaabb   
	unsigned char byte[2];        //byte[0]=aa,byte[1]=bb
}coordn16; //coordenadas de 2 bytes 


/*  funciones procedimientos o metodos que es lo mismo ------------------------ */
void init_VFD(void);
void FontSizeVFD(unsigned char m);

void VFDcommand(unsigned char cmd);
void VFDclrscr(void);
void VFDposicion(unsigned short int x,unsigned short int y);
		

void VFDkeypad_ISR(unsigned char c);
void init_VFD_BIOS(void);
//void VFDposicionDDS(unsigned short int x,unsigned short int y);
void VFDserial_SendCharDDS(unsigned char c);
void VFDdrawLine(unsigned char pen,unsigned short int x1,unsigned short y1,unsigned short int x2,unsigned short y2);
void VFDdotDrawing(unsigned short int x,unsigned short int y);
//void disable_keyboard_IRQ(void);
//void enable_keyboard_IRQ(void);
//void Timer_enable_keyboard_IRQ(void);
void BarraDet_VFDserial_SendChar(unsigned char c);
//void disable_keyboard_IRQ2(void);
//void enable_keyboard_IRQ2(void);
//void Timer_enable_keyboard_IRQ2(void);
void VFDboxLine(unsigned char pen,unsigned char mode,unsigned short int x1,unsigned short y1,unsigned short int x2,unsigned short y2);
void VFDclrscrForce(void);
void VFDposicion_DDS(unsigned short int x,unsigned short int y);
void VFDclrscr_DDS(void);
void monitorDDS_Halt(void);
void VFDserial_SendChar_DDS(unsigned char c);
void VFDcommand_Bold_DDS(unsigned char bold);
void BarraDet_VFDposicion(unsigned short int x,unsigned short int y);
void menuKey(unsigned char key,unsigned short int Et,unsigned char c);
void teclado_Control(unsigned char c,unsigned short int EnTime,unsigned short int Keytime);

/*
 * TFTmenu.h
 *
 *  Created on: Feb 11, 2021
 *      Author: desarrollo-1
 */

#ifndef TFTMENU_H_
#define TFTMENU_H_



#endif /* TFTMENU_H_ */


#define SIZE_BUF_TFT 5

//MENUS
//#define TFT_PORTAL_INICIO 'P'
//#define TFT_DDS_MENU      'D'  //menu donde se grafica el DDS

//ESTADOS DEL MENUS--------------------------
#define COMANDO  0x49 
#define PARAM1   0x5A //parametros-1
#define FIN      0x5C


//COMMANDO turn=ON graph DDS in PC //03h,44h,5Bh,02h
#define CMD_TFT_DDS_MENU  'D'  //44h menu donde se grafica el DDS
#define CRC_DDS  0x5B//crc FIJO NO CAMBIAR

//COMMAND turn=off DDS graph    03h,50h,5ch,02h	and move tab to portal-inicio on touch-screen
#define CMD_TFT_PORTAL_INICIO 'P'
#define CRC_PORT 0x5C//crc fijo no cambiar del cambio al portal




struct TFT_MENU_{
	unsigned char contextoActual;
	void (*MenuActual)(unsigned char c);
	unsigned char estado;//estado actual de maquina de estado de comando
    unsigned char buffer[SIZE_BUF_TFT];
};


void menu_TFT_touch_ISR(unsigned char c);
void init_menuTFT(void);

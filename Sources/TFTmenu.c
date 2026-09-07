/*
 * TFTmenu.c
 *
 *  Created on: Feb 11, 2021
 *      Author: desarrollo-1
 */


#include "TFTmenu.h"
#include "TFTmenus.h"
#include "IOUPserial.h"

struct TFT_MENU_ TFTmenu;

void init_menuTFT(void){
	
	TFTmenu.contextoActual=CMD_TFT_PORTAL_INICIO;
	TFTmenu.MenuActual=TFTmenu_Portal_Inicio;
	TFTmenu.estado=COMANDO;
	IOUPserial_EnableEvent();
}

void menu_TFT_touch_ISR(unsigned char c){
	
	switch(TFTmenu.contextoActual){
		case CMD_TFT_PORTAL_INICIO:TFTmenu.MenuActual(c);break;	  
		case CMD_TFT_DDS_MENU:TFTmenu.MenuActual=TFTmenu_DDS;
		                  TFTmenu.MenuActual(c);break;
	}//fin switch
}//fin de menu de TFT touch isr--------------------




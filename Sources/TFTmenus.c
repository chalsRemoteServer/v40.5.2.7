/*
 * TFTmenus.c
 *
 *  Created on: Feb 11, 2021
 *      Author: desarrollo-1
 */



#include "TFTmenus.h"
#include "IOcomms.h"
#include "TFTmenu.h"
#include "queue.h"
#include "system.h"

extern struct _SERIAL_ serial;//_Vector_ serial;
extern struct TFT_MENU_ TFTmenu;


/*Estoy en el portal inicio.     //03h,44h,5Bh,02h
  comando para pasar ala Tab-DDS: STX,'D',5Bh,ETX*/
void TFTmenu_Portal_Inicio(unsigned char c){
	
	 switch(TFTmenu.estado){//STX,'
		 case COMANDO:if(c==STX)
			               TFTmenu.estado=PARAM1;
		 	 	       break; 
		 case PARAM1:TFTmenu.buffer[0]=c;
		             if(c==CMD_TFT_DDS_MENU)
		                   TFTmenu.estado=CRC_DDS;
		             else TFTmenu.estado=COMANDO;
		             break;		
		 case CRC_DDS:if(c==CRC_DDS){ //este comando no responde porque responde con graficacion masiva
			              serial.vars.Flags.bitF.DDS_ACTIVA=TRUE;
			              TFTmenu.contextoActual=CMD_TFT_DDS_MENU;}
		              TFTmenu.estado=COMANDO;      
		              break;
		 default:break;
	 }//fin switch 
}//menu de TFT portal inicio--------------------------------


/*Estoy en la Tab-DDS                 03h,50h,5ch,02h
 * comando para pasar ala Tab-Porta:  STX,'P',5Ch,ETX*/
void TFTmenu_DDS(unsigned char c){
     	 
	switch(TFTmenu.estado){
		case COMANDO:if(c==STX)
			            TFTmenu.estado=PARAM1;
					 else TFTmenu.estado=COMANDO;
		             break;
		case PARAM1:TFTmenu.buffer[0]=c;
		            if(c==CMD_TFT_PORTAL_INICIO)
		            	 TFTmenu.estado=CRC_PORT;
		            else TFTmenu.estado=COMANDO;
		            break;
		case CRC_PORT:if(c==CRC_PORT){
		                  serial.vars.Flags.bitF.DDS_ACTIVA=FALSE;
		                  TFTmenu.contextoActual=CMD_TFT_PORTAL_INICIO;
		                  TFTmenu.MenuActual=TFTmenu_Portal_Inicio;}
		            TFTmenu.estado=COMANDO;
		default:    break;
		            
	}//fin switch   
}//fin TFTmenu_DDS----------------------------------

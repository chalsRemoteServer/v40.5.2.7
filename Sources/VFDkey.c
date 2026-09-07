/*
 * VFDkey.c
 *
 *  Created on: Nov 8, 2019
 *      Author: desarrollo-1
 */


#include "VFDkey.h"
#include "VFDmenu.h"
#include "system.h"
#include "VFDisplay.h"
#include "VFD.h"
#include "VFDserial.h"
#include "DSP.h"
#include "Memoria.h"
#include "Reloj.h"
#include "I2C_.h"
#include "strings.h"
#include "maths.h"
#include "VFDmenuControl.h"
#include "queue.h"
#include "IOcomms.h"
#include "strings.h"

extern struct _GLOBAL_CONTEXTO g,*DDSpant;
extern struct _GLOBAL_CONTEXTO g,*AjustVibr,*AjParamProd,*ParamDet;
extern struct _GLOBAL_CONTEXTO *textProc,*NewProd,*Entest;
extern struct _Comando_DDS dds; //extern struct _DDS_ DDS1;
extern struct _Detection Deteccion;
extern struct _Menu_ menu;
extern struct _GLOBAL_PASO_OTRO_CONTEXTO gpoc;//global paso a otro contexto
extern struct _Comando_DDS dds;
extern struct _KeyPad_ keypad;

extern struct ZoomControl zoom;
extern struct EntSyst{//variables de entradas de sistema 
		unsigned char taco;
		unsigned char fotocel;
        unsigned char fotocelBloqueado;//de 1 seg a 60 segundos
		unsigned char rechazo;
		unsigned char rechazoBloqueado;
		unsigned char rechazoPruebasist_seg;
		unsigned char rechazoPruebasist_mseg;
		unsigned char producto; 
		unsigned char productoBloqueado;
		unsigned char productoPruebasist_seg;
		unsigned char productoPruebasist_mseg;
		unsigned char aux1;// xxxx=rele
		unsigned char aux1Rele;//nombre del rele seleccionado
		unsigned char aux1Label;//label seleccinado
		unsigned char aux1_seg;
		unsigned char aux1_mseg;
		unsigned char aux2;
		unsigned char aux2Rele;
		unsigned char aux2_seg;
		unsigned char aux2_mseg;
		unsigned char aux2Label;
		unsigned char byten;	
		unsigned char PulsosPorSegundo;//pulsos medidos en tiempo real por segundo
}EntradasSistemaReg;


extern volatile unsigned char contextoActual;
extern void displayControladorPantalla(unsigned char destino);
extern unsigned char cursorx,cursory; //posicion actual del cursor,
extern unsigned char arg0,arg1,arg2,arg5,arg6,arg8,arg9,arg10;//argumentos para entrar en un contexto
extern unsigned short int arg3,arg7;//argumentos para entrar en un contexto
extern unsigned char phase,phasefrac;//phase de ajustes de deteccion la variable fraccionaria es de la misma vriable
//extern unsigned short int  Sensibilidad; // variable de sensibilidad, Sensibilidad2 es la misma variable
extern unsigned char Ganancia,MarcarAltura;
extern unsigned short int  Altura; //variable de parametros de deteccion------para el DDS
extern unsigned char BorrarDDS; //variable que determina si hacemos el update DDS
extern unsigned short int contextobuffer;//variable auxiliar que permanece en contexto hasta que cambie
extern unsigned char modificado; //guarda cual menu ha sido mosificado
extern void configModificado(unsigned char m);
extern unsigned char Alta; //variable que guarda el ultimo digito de la frecuencia alta
extern unsigned char Media;//variable que guarda el ultimo digito de la frecuencia media
extern unsigned char Baja;//variable que guarda el ultimo digito de la frecuencpia baja
extern void segundero(unsigned char status);
extern unsigned char tipo_deMaquina; // garda el tipo de maquina que esta configurado
extern unsigned char semMenu2;
extern unsigned char frecuenciaSeleccion;//SELECCIONA EL NIVEL de la frecuencia actual configurada y activada
extern unsigned char semaforoDisplay;//para que  nos permita desplegar antes de activar la  barra de deteecion.

//extern void ReleRechazo(unsigned char status);
extern void ReleFalla(unsigned char status);
extern void ReleAlarma(unsigned char status);
extern void ReleBloqueo(unsigned char status);
extern void SalidaPHVF(unsigned char status);
extern void SalidaAux(unsigned char status);
extern unsigned char isMenuPruebaEntradasON;//nos dice sis esta activo el menu de pruebas de entradas 
extern unsigned char configTextoProcessorController(unsigned char contextOrigen,unsigned char contextDestino);
extern unsigned char BuscarSiguienteiProduct(unsigned char direccion);

extern unsigned char igxm5[IGXM_SIZE];//variable generica array generico
extern unsigned char indiceProducto; //Producto Set TestUP=1, que producto esta actualmete seleccionado
extern unsigned char displayADCvalores;// dice si se ejecuta desplegar los valores del adc en pruebas o no.
extern unsigned char Text[NOMBRE_PRODUCTO_SIZE];//texto que regresa  del procesador de textos
extern unsigned char name[NOMBRE_PRODUCTO_SIZE]; 

extern unsigned char passControl[PASSCNTRL_SIZE];
unsigned char cursorAnterior;



/*case PORTAL_INICIO: displayControladorPantalla(PORTAL_INICIO;//MENU-PADRE:PORTAL_INICIO----------------------
 * PORTAL_INICIO   PORTAL_INICIO   PORTAL_INICIO*/

void PortaliniciokeyUP(void){return;}//fin PortaliniciokeyUP--------PONER LAS FUNCIONES KEY DEL MENU------------------6
void PortaliniciokeyRT(void){return; }//fin PortaliniciokeyRT-------LLENAR LAS FUNCIONES DE LOS KEYS------------------8
void PortaliniciokeyLF(void){return;}//fin PortaliniciokeyLF--------------------------------------
void PortaliniciokeyDN(void){return;}//fin PortaliniciokeyLF--------------------------------------
void PortaliniciokeyEN(void){VFDcommand_Bold_DDS(FALSE);
	                         displayControladorPantalla(MENU_INSIGHT);/*a donde voy*/return;}//fin PortaliniciokeyDN--------------------------------------------------------------------------------------

/*fin case PORTAL_INICIO: displayControladorPantalla(PORTAL_INICIO;//MENU-PADRE:PORTAL_INICIO
 *fin PORTAL_INICIO   PORTAL_INICIO   PORTAL_INICIO*/

void MenuInsightkeyUP(void){if(cursory>POSY4){
                              VFDposicion(cursorx,cursory);
                              VFDserial_SendChar(' ');
                              --cursory;
                              VFDposicion(cursorx,--cursory);
                              VFDserial_SendChar('>');}
                         else 
                        	 if(cursory==POSY4){
                        		 VFDposicion(cursorx,cursory); 
			                     VFDserial_SendChar(' ');
			                     cursorx=POSXESQ235;cursory=POSY0; //xpos=POSXESQ235
			                     VFDposicion(cursorx,cursory);
			                     VFDserial_SendChar('>');
			                     VFDserial_SendChar('X');  }    
                          return;}//fin MenuInsightkeyUP--------------------------------------
void MenuInsightkeyRT(void){return;}//fin MenuInsightokeyRT--------------------------------------
void MenuInsightkeyLF(void){return;}//fin MenuInsightokeyLF--------------------------------------
void MenuInsightkeyDN(void){if(cursory==POSY0){
	                        VFDposicion(cursorx,cursory);
							VFDserial_SendChar(' ');
							VFDserial_SendChar('x');
							cursorx=POSX0;cursory=POSY4;
							VFDposicion(cursorx,cursory);
							VFDserial_SendChar('>');
							return;}
				         else{
				        	if(cursory<POSY12){
							VFDposicion(cursorx,cursory);
							VFDserial_SendChar(' ');
							++cursory;
							VFDposicion(cursorx,++cursory);
							VFDserial_SendChar('>');
							return;}}
	                     return;}//fin MenuInsightokeyDN--------------------------------------
void MenuInsightkeyEN(void){
	 switch(cursory){
		case POSY4:displayControladorPantalla(AJUSTE_DE_PRODUCTO);break;
		case POSY6:displayControladorPantalla(AJUSTE_PARAMETRICO_DE_PRODUCTO);break;
		case POSY8:displayControladorPantalla(AJUSTE_DE_SISTEMA);break;
		case POSY10:displayControladorPantalla(PANTALLA_DDS);break;
		case POSY12:displayControladorPantalla(INFORMACION_DE_USUARIO);break;
		case POSY0: displayControladorPantalla(PORTAL_INICIO);break;
		default:break;}//fin MenuInsightokeyEN-------------------------------------
}//fin MenuInsightkeyEN------------------------------------------------------------
/*  fin de MenuInsightkey+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*  fin de MenuInsightkey+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/



void AjusteProductokeyUP(void){
	                      if(cursory>POSY4){
	   			    	      VFDposicion(cursorx,cursory);
	   			    	      VFDserial_SendChar(' ');
	   			    	      --cursory;
	   			    	      VFDposicion(cursorx,--cursory);
	   			    	      VFDserial_SendChar('>');
	   			    	      return;}
	   			          else if(cursory==POSY4){VFDposicion(cursorx,cursory); 
	   			                                  VFDserial_SendChar(' ');
	   			                                  cursorx=POSXESQ235;cursory=POSY0; //xpos=POSXESQ235
	   			                                  VFDposicion(cursorx,cursory);
	   			                                  VFDserial_SendChar('>');
	   			                                  VFDserial_SendChar('X');  }
	                      return;}//fin PortaliniciokeyUP--------PONER LAS FUNCIONES KEY DEL MENU------------------6

void AjusteProductokeyRT(void){return; }//fin PortaliniciokeyRT-------LLENAR LAS FUNCIONES DE LOS KEYS------------------8
void AjusteProductokeyLF(void){return;}//fin PortaliniciokeyLF--------------------------------------

void AjusteProductokeyDN(void){
	if(cursory==POSY0){
			VFDposicion(cursorx,cursory);
			VFDserial_SendChar(' ');
			VFDserial_SendChar('x');
			cursorx=POSX0;cursory=POSY4;
			VFDposicion(cursorx,cursory);
			VFDserial_SendChar('>');return;}
	else{if(cursory<POSY14){
		   switch(passControl[_AUTHOR_]){
			   case 1:if(cursory>=POSY6){
				          cursory=POSY6;
			              return;}
			   case 2:	   
			   case 3:	   
			   case 4:
			   case 5:
				       VFDposicion(cursorx,cursory);
				   	   VFDserial_SendChar(' ');
				  	   ++cursory;
				   	   VFDposicion(cursorx,++cursory);
				       VFDserial_SendChar('>');
				   	   return;break;
			   default:return;break; }}}//fin swich
return;}//f--------------------------------------------------------------------

void AjusteProductokeyEN(void){if(cursory==POSY4){/*displayControladorPantalla(EN_DESARROLLO;*/return;}
                               if(cursory==POSY6){ displayControladorPantalla(SELECCIONAR_PRODUCTO); return;}
                               if(cursory==POSY8){ arg0=0;arg1=0;//sin argumentos para entrar aqui
                            	                   displayControladorPantalla(NUEVO_PRODUCTO);
                                                   return;}
                               if(cursory==POSY10){displayControladorPantalla(AJUSTE_PRODUCTO_SEL); return;}
                               if(cursory==POSY12){displayControladorPantalla(BORRAR_PRODUCTO); return;}//Borrar PRODUCTO
                               if(cursory==POSY14){displayControladorPantalla(COPY_PRODUCTO);return;}
                               if(cursory==POSY0){ displayControladorPantalla(PORTAL_INICIO);return;}
	                           return;}//fin PortaliniciokeyDN--------------------------------------------------------------------------------------

/* FIN  AjusteProducto  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* FIN  AjusteProducto  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * */
void AjusteParamProdkeyUP(void){
	
	if(AjParamProd->igxc0>0){//esta seleccionado cambiar por numero
	  	 operacionVariable(POSXAJUSPROD-8,cursory,AjParamProd->igxc0,SUMAR);
		 configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);
		 isEnable_Keypad(WAIT);
		 return;}
	if(cursory==POSY6){//MENU con sistema OPERATIVO
		VFDposicion_DDS(cursorx,cursory);
		VFDserial_SendChar_DDS(' ');
		cursory=POSY0;	  
		VFDposicion_DDS(POSXESQ235,cursory);
		VFDserial_SendChar_DDS('>');
		VFDserial_SendChar_DDS('X');
		VFDserial_SendChar_DDS(0x01);
		return;}
	if(cursory==POSY0){return;}
	if(cursory<=POSY14){	
		 VFDposicion_DDS(cursorx,cursory);
		 VFDserial_SendChar_DDS(' ');
		 --cursory;
		 VFDposicion_DDS(cursorx,--cursory);
		 VFDserial_SendChar_DDS('>');
		 VFDserial_SendChar_DDS(0x01);
		 return;}
}//fin AjusteParamProdkeyUP

void AjusteParamProdkeyDN(void){

	if(AjParamProd->igxc0>0){//esta seleccionado cambiar por numero
		operacionVariable(POSXAJUSPROD-8,cursory,AjParamProd->igxc0,RESTAR);
		return;}
	if(cursory==POSY0){
	   VFDposicion_DDS(POSXESQ235,POSY0); 
	   VFDserial_SendChar_DDS(' ');
	   VFDserial_SendChar_DDS('x');
	   cursory=POSY6;
	   VFDposicion_DDS(cursorx,cursory);
	   VFDserial_SendChar_DDS('>');
	   VFDserial_SendChar_DDS(0x01);
	   return;}
	if(cursory<POSY14){	 
	   VFDposicion_DDS(cursorx,cursory);
	   VFDserial_SendChar_DDS(' ');
	   cursory++;
	   VFDposicion_DDS(cursorx,++cursory);
	   VFDserial_SendChar_DDS('>');
	   VFDserial_SendChar_DDS(0x01);
	   __asm(nop);
	   return;}
}//------------------------------------------------------------



void AjusteParamProdkeyRT(void){
	if(cursory==POSY6){
		procSensxDigitoRT(POSXAJUSPROD,POSY6,&Deteccion.Sensibilidad);
	    configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);
		return;}
	if(cursory==POSY8){
		if(++phasefrac>9){
			phasefrac=0;
			if(++phase>179){
			   phase=0;
			   phasefrac=0;}}
		display_pushFIFOcOP_Phase_var(POSXAJUSPROD,POSY8);
		configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);
		return;}
}//FIN AjusteParamProdkeyRT ----------------------------------

void AjusteParamProdkeyLF(void){
	if(cursory==POSY6){
	   procSensxDigitoLF(POSXAJUSPROD,POSY6,&Deteccion.Sensibilidad);
	   configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);}
   if(cursory==POSY8){
		   if(phasefrac!=0)
			 phasefrac--;
		   else{phasefrac=9;
				if(phase==0)
				  phase=179;
				  else phase--;}
		   display_pushFIFOcOP_Phase_var(POSXAJUSPROD,POSY8);//fin contextobuffer 0
		   configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);
									   return;}
}//FIN  AjusteParamProdkeyLF-------------------------------------


/*  resta o suma a un digto a la variable de sensibilidad o phase*/
/* Metodo exclusivo de Ajustes PArametricos de Producto por la vriable global */
void operacionVariable(unsigned char posx,unsigned short int y,unsigned char igxm0,unsigned char op){
byte a[5],*p,i,b[6];
typedef unsigned short int usint;
	     p=&a[0];
		 if((y==POSY6)||(y==POSY2)){
			  operacionUShortInt(&Deteccion.Sensibilidad,igxm0,op);//opera en el digito indicado
			  cursorx=posx;//+(8*(5-igxm0));//  igxm
			  VFDposicion_DDS(cursorx,cursory);
			  getCharsFromUINT_var(p,Deteccion.Sensibilidad);
			  switch(igxm0){ //VFDserial_SendChar_DDS((*(p+5-igxm0))+'0');
				  case 5:b[0]='>';
						 b[1]=a[0]+0x30;
						 b[2]=a[1]+0x30;
						 b[3]=a[2]+0x30;
						 b[4]=a[3]+0x30;
						 b[5]=a[4]+0x30;
				         break;
				  case 4:b[0]=a[0]+0x30;
				         b[1]='>';
				         b[2]=a[1]+0x30;
				         b[3]=a[2]+0x30;
				         b[4]=a[3]+0x30;
				         b[5]=a[4]+0x30;
				         break;
				  case 3:b[0]=a[0]+0x30;
						 b[1]=a[1]+0x30;
						 b[2]='>';
						 b[3]=a[2]+0x30;
						 b[4]=a[3]+0x30;
						 b[5]=a[4]+0x30;
						 break;
				  case 2:b[0]=a[0]+0x30;
						 b[1]=a[1]+0x30;
						 b[2]=a[2]+0x30;
						 b[3]='>';
						 b[4]=a[3]+0x30;
						 b[5]=a[4]+0x30;
						 break;
				  case 1:b[0]=a[0]+0x30;
						 b[1]=a[1]+0x30;
						 b[2]=a[2]+0x30;
						 b[3]=a[3]+0x30;
						 b[4]='>';
						 b[5]=a[4]+0x30;
						 break; 
				  default:break;}
			for(i=0;i<6;i++)
				VFDserial_SendChar_DDS(b[i]);
				  return;}
		 if((y==POSY8)||(y==POSY4)){ 
			  if(phasefrac>9)
				  phasefrac=0;
			  getASCII_from_Fase(&a[0],phase,phasefrac);
			  operacion_ASCII_Phase2(igxm0,op,&a[0]);
			  //configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);
			  display_Phase_DDS(posx+8,POSY8,&a[0]);
			  setASCII_to_Fase(&a[0],&phase,&phasefrac);
			  return;}
}//fin operacionVariable


void AjusteParamProdkeyEN(void){
//unsigned char a[5],*p;
 
	    switch(cursory){
	    	case POSY2: /* INIZIAIZAR MEMORIA*/ break;
	    	case POSY4:                         break;
	    	case POSY6: procSensxDigitoEN(&(AjParamProd->igxc0),POSXAJUSPROD-8,0); 
	    	            break;//del case posy6
	    	case POSY8: procFasexDigitoEN(&(AjParamProd->igxc0),POSXAJUSPROD,POSY10);
						break;
			case POSY10: //  _display   pushFIFOcOP  BorrarContadores_var();
				        configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);
						displayControladorPantalla(configModificacionController(AJUSTE_PARAMETRICO_DE_PRODUCTO,AJUSTE_PARAMETRICO_DE_PRODUCTO));
						arg2=RESET;//se borraran los contadores
						break;
			case POSY12:  displayControladorPantalla(PANTALLA_DDS); break;
			case POSY14: //Para Pasar a Ajustes Avanzados
						menu.bytes.semMenuPendiente=TRUE;//HABILITAMOS  funcion en S.O de escape
						displayBoxDestructor(ACTIVADO);//destruir y parar la barra de deteccion.
						//displayControladorPantalla(configModificacionController(AJUSTES_AVANZADOS,AJUSTE_PARAMETRICO_DE_PRODUCTO));
						menu.contexto.Anterior=AJUSTE_PARAMETRICO_DE_PRODUCTO;
						displayControladorPantalla(AJUSTES_AVANZADOS);
						break;
			case POSY0:
			        	displayControladorPantalla(configModificacionController(PORTAL_INICIO,AJUSTE_PARAMETRICO_DE_PRODUCTO));
			        	break;
			default: __asm(Halt);}//debug error de diseño se softwre
return;}
/* FIN  AjusteParamProdto  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* FIN  AjusteParamProd ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * */
    
void AjustedeSistemakeyUP(void){
	    if(cursory>POSY4){
           VFDposicion(cursorx,cursory);
           VFDserial_SendChar(' ');
           --cursory;
           VFDposicion(cursorx,--cursory);
           VFDserial_SendChar('>');
           return;}
        else 
        	if(cursory==POSY4){
        		VFDposicion(cursorx,cursory); 
                VFDserial_SendChar(' ');
                cursorx=POSXESQ235;cursory=POSY0; //xpos=POSXESQ235
                VFDposicion(cursorx,cursory);
                VFDserial_SendChar('>');
                VFDserial_SendChar('X');  }
return;}

void AjustedeSistemakeyRT(void){
return;}	 


void AjustedeSistemakeyLF(void){
	return;}

void AjustedeSistemakeyDN(void){
	 	 if(cursory==POSY0){
	 		 VFDposicion(cursorx,cursory);
	 		 VFDserial_SendChar(' ');
	 		 VFDserial_SendChar('x');
	 		 cursorx=POSX0;cursory=POSY4;
	 		 VFDposicion(cursorx,cursory);
	 		 VFDserial_SendChar('>');return;}
	     else{
	 	 		 if(cursory<POSY14){
	 	 			 __asm(nop);
	 	 			 switch(passControl[_AUTHOR_]){ //Autor=3
	 	 				 case 1: 
	 	 				 case 2:return;break;
	 	 				 case 3:
	 	 				 case 4:if(cursory==POSY12){
	 	 					         return;break;}
	 	 				 case 5:VFDposicion(cursorx,cursory);
		 	 			 	 	VFDserial_SendChar(' ');
		 	 			        ++cursory;
		 	 			        VFDposicion(cursorx,++cursory);
		 	 			        VFDserial_SendChar('>');
	 	 				 default: return;break;
	 	 			 }}}//fin switch, if, else
return;}

void AjustedeSistemakeyEN(void){
	if(cursory==POSY2){/*);ret;*/ return;}
	if(cursory==POSY4){ displayControladorPantalla( MENU_ADMINISTRATIVO); return;}//-------------M8:poner el contexto al que se quiere entrar
	if(cursory==POSY6){ displayControladorPantalla(AJUSTE_DE_VIBRACION);  return;}
	if(cursory==POSY8){ displayControladorPantalla(CONFIGURAR_ENTRADAS_DE_SISTEMA); cursorAnterior=POSY0;return;}
	if(cursory==POSY10){displayControladorPantalla(CONTROL_DE_FRECUENCIA);return;}
	if(cursory==POSY12){displayControladorPantalla(ID_COMUNICACIONES);    return;}
	if(cursory==POSY14){displayControladorPantalla(INGENIERIA_GRACIDA);   return;}
	if(cursory==POSY0){ displayControladorPantalla(PORTAL_INICIO);	      return;}	        
return;  }//fin AjustedeSistemakeyEN
/* fin AjustedeSistemakeyDN+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * AjustedeSistemakeyDN+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

//------------------------------------------------------------+++++++++++++++++++++++++++++++++++++++++++++PASO-4
//-------------------------------------------------------------------------------------TRAER LOS CODIGOS DEL
//+---------------------------------------------------------------------------VFDmenu.c----------------------
//--------------------------------------------------------------------------------PASO IR A SISTEMA OP ESCAPE-----------
//posiiones donde uno se mueve	
	
    	  

void PANTALLA_DDSkeyUP(void){
unsigned char key[] ={POSY0,POSY2,POSY4,POSY6,POSY8,POSY10,POSY12,POSY14,0};
unsigned char key2[]={POSY0,POSY10,POSY12,POSY14,0};
unsigned char *p;
unsigned char z[3];
	   	
	   	
	   	if(passControl[_AUTHOR_]==0)
	   		p=&key2[0];
	   	else p=&key[0];
	   	if((DDSpant->igxc5==0)&&(DDSpant->igxc1==0)){//no estamos manipulando digito x digito
	     	   displayControlCursorSubMenusKey(PARRIBA,&cursorx,&cursory,POSXESQ235,p,0);
	           return;}
	   	if( DDSpant->igxc5>0){//esta seleccionado cambiar por numero
	   			operacionVariable(POSXDDSNUMS-8,cursory,DDSpant->igxc5,SUMAR);
	   		    configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);
	   		    dds.Bnderas.bit.DDS1_BORRAR=FALSE;
				dds.Bnderas.bit.Apagando=TRUE; //se Desactiva el Despligue de los Puntitos en pantalla del DD
				displayControladorPantalla(PANTALLA_DDS);
				dds.Bnderas.bit.clean_Buffers=FALSE;//borrar los buffer donde se guardan los puntos que se han pintado y detectado
				switch(cursory){
					case POSY2:DDSpant->igxc2='S';break; //cursor en sensibilida
					case POSY4:DDSpant->igxc2='P';break;
					default:return;}
	   		    isEnable_Keypad(WAIT);
	   			return;}
	   	switch(cursory){
	   		case POSY2: VFDposicion_DDS(cursorx,cursory);
						VFDserial_SendChar_DDS(' ');
						cursory=POSY0;	  
						VFDposicion_DDS(POSXESQ235,cursory);
						VFDserial_SendChar_DDS('>');
						VFDserial_SendChar_DDS('X');
						VFDserial_SendChar_DDS(0x01);
	   			        break;
	   		case POSY10:switch(DDSpant->igxc1){
					       case 2:VFDposicion_DDS(POSXDDSNUMS,POSY10);
	   						      getASCII_from_UChar(zoom.Zoom,&z[0]);
	   						      procesar_ASCII(&z[0],SUMAR,CENTRAL);//sumar uno al digito decena
	   						      zoom.Zoom=convert_ASCII_to_char(&z[0]);
	    				   	      VFDserial_SendChar_DDS(z[1]);
	    				   	      VFDserial_SendChar_DDS(0x01);
					              isEnable_Keypad(WAIT);
                                  break;
	   					   case 1:VFDposicion_DDS(POSXDDSNUMS,POSY10);
	   					   	   	  getASCII_from_UChar(zoom.Zoom,&z[0]);
	   					   	   	  procesar_ASCII(&z[0],SUMAR,RIGHT);	
	   						      zoom.Zoom=convert_ASCII_to_char(&z[0]);
	   					   	      VFDserial_SendChar_DDS(z[1]);
	   					   	      VFDserial_SendChar_DDS('>');
	   					   	      VFDserial_SendChar_DDS(z[2]);
	   					   	      VFDserial_SendChar_DDS(0x01);
								  isEnable_Keypad(WAIT);
								  break;
	   					   default:DDSpant->igxc1=0;break;  }
	   			        break;//case posy10
	   		default:break;}
	//empieza a contar el timer para activarlo, para que alcance a mandar el buffer rx serial
}//fin PANTALLA_DDSkeyUP--------------------



void PANTALLA_DDSkeyDN(void){
unsigned char key[] ={POSY0,POSY2,POSY4,POSY6,POSY8,POSY10,POSY12,POSY14,0};
unsigned char key2[]={POSY0,POSY10,POSY12,POSY14,0};
unsigned char *p;	
unsigned cc=24,y=9,b;
unsigned char z[3];

	   	b=y;
	    	
	   	if(passControl[_AUTHOR_]==0){
	   		p=&key2[0];
	 	    b=cc; }
	    else p=&key[0];
	   	if((!DDSpant->igxc5)&&(!DDSpant->igxc1)){//no estamos manipulando digito x digito
	      	displayControlCursorSubMenusKey(PABAJO,&cursorx,&cursory,POSXESQ235,p,DDS_MENU_X);
	   	    return;}
	   	if(DDSpant->igxc5>0){//esta seleccionado cambiar por numero
	   			operacionVariable(POSXDDSNUMS-8,cursory,DDSpant->igxc5,RESTAR);
	   			configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);
				dds.Bnderas.bit.DDS1_BORRAR=FALSE;
				dds.Bnderas.bit.Apagando=TRUE; //se Desactiva el Despligue de los Puntitos en pantalla del DD
				displayControladorPantalla(PANTALLA_DDS);
				dds.Bnderas.bit.clean_Buffers=FALSE;//borrar los buffer donde se guardan los puntos que se han pintado y detectado
			    switch(cursory){
			    	case POSY2:DDSpant->igxc2='S';break; //cursor en sensibilida
			    	case POSY4:DDSpant->igxc2='P';break;
			    	default:return;}
				isEnable_Keypad(WAIT);
	   			return;}
		switch(cursory){
			case POSY10:switch(DDSpant->igxc1){
						   case 2:VFDposicion_DDS(POSXDDSNUMS,POSY10);
								  getASCII_from_UChar(zoom.Zoom,&z[0]);
								  procesar_ASCII(&z[0],RESTAR,CENTRAL);//sumar uno al digito decena
								  zoom.Zoom=convert_ASCII_to_char(&z[0]);
								  VFDserial_SendChar_DDS(z[1]);
								  VFDserial_SendChar_DDS(0x01);
								  isEnable_Keypad(WAIT);
								  break;
						   case 1:VFDposicion_DDS(POSXDDSNUMS,POSY10);
								  getASCII_from_UChar(zoom.Zoom,&z[0]);
								  procesar_ASCII(&z[0],RESTAR,RIGHT);	
								  zoom.Zoom=convert_ASCII_to_char(&z[0]);
								  VFDserial_SendChar_DDS(z[1]);
								  VFDserial_SendChar_DDS('>');
								  VFDserial_SendChar_DDS(z[2]);
								  VFDserial_SendChar_DDS(0x01);
								  isEnable_Keypad(WAIT);
								  break;
						   default:DDSpant->igxc1=0;break;  }
						break;//case posy10
			default:break;}
//empieza a contar el timer para activarlo, para que alcance a mandar el buffer rx seria	   	
}//PANTALLA_DDSkeyDN------------------------------------------------------------------------


void PANTALLA_DDSkeyRT(void){
   switch(cursory){ 
	   
	   case POSY2:   cursorx=POSXDDSNUMS;
		            if(!procSensxDigitoRT(cursorx,POSY2,&Deteccion.Sensibilidad))
		                  return;
					configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);
					dds.Bnderas.bit.DDS1_BORRAR=FALSE;
					dds.Bnderas.bit.Apagando=TRUE; //se Desactiva el Despligue de los Puntitos en pantalla del DD
				    displayControladorPantalla(PANTALLA_DDS);
				    dds.Bnderas.bit.clean_Buffers=FALSE;//borrar los buffer donde se guardan los puntos que se han pintado y detectado
				    DDSpant->igxc2='S'; //cursor en sensibilida
					break;
	   case POSY10: if(zoom.Zoom==99)
		                     break;
	                re_Calcular_Buffers_DDS(zoom.Zoom+1);//zoom.Zoom++;
	                displayZoom(POSXDDSNUMS,POSY10);
	                isEnable_Keypad(WAIT);
					dds.Bnderas.bit.DDS1_BORRAR=TRUE;//borrar la pantlla del DDS quitar todos los puntos
					dds.Bnderas.bit.Apagando=TRUE; //se Desactiva el Despligue de los Puntitos en pantalla del DDS
					displayControladorPantalla(PANTALLA_DDS);
					dds.Bnderas.bit.DDS_Reload=TRUE;//repintar el DDS con los puntos guardados en los buffers
					dds.Bnderas.bit.clean_Buffers=FALSE;//NO BORRAR BUFERS
	                break;
   default:break;}
}	//fin  PANTALLA_DDSkeyRT-----------------

void PANTALLA_DDSkeyLF(void){
	
	switch(cursory){ 
		
      case POSY2:cursorx=POSXDDSNUMS;
    	         if(!procSensxDigitoLF(cursorx,POSY2,&Deteccion.Sensibilidad))
    	              return;//llego al uno
				 configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);
				 dds.Bnderas.bit.DDS1_BORRAR=FALSE;
				 dds.Bnderas.bit.Apagando=TRUE; //se Desactiva el Despligue de los Puntitos en pantalla del DD
				 displayControladorPantalla(PANTALLA_DDS);
				 dds.Bnderas.bit.clean_Buffers=FALSE;//borrar los buffer donde se guardan los puntos que se han pintado y detectado
				 DDSpant->igxc2='S'; //cursor en sensibilidad
				 break;
	  case POSY10:  if(zoom.Zoom==1)
			                     break;
					re_Calcular_Buffers_DDS(zoom.Zoom-1);
					displayZoom(POSXDDSNUMS,POSY10);//PINTAR el numero de la variable del zoom
					isEnable_Keypad(WAIT);///deactivado el teclado
					dds.Bnderas.bit.DDS1_BORRAR=TRUE;//borrar la pantlla del DDS quitar todos los puntos
					dds.Bnderas.bit.Apagando=TRUE; //se Desactiva el Despligue de los Puntitos en pantalla del DDS
					displayControladorPantalla(PANTALLA_DDS);
					dds.Bnderas.bit.DDS_Reload=TRUE;//repintar el DDS con los puntos guardados en los buffers
					dds.Bnderas.bit.clean_Buffers=FALSE;//NO BORRAR BUFERS
					break;
	   default:break;}
	
}//PANTALLA_DDSkeyLF-----------------------


void PANTALLA_DDSkeyEN(void){
unsigned char z[3];
		   	
	    switch(cursory){
	    	case POSY0:if(modificado==YES){
				  	  	  MemoEEPROM(SAVE,PANTALLA_DDS);
						  modificado=NOP; }
			  	  	   displayControladorPantalla(PORTAL_INICIO);
			  	  	   BorrarDDS=NONE; //ImportanteÂ¡Â¡ CUANDO regresemos al DDS el cursos estara en la esquina
			  	  	   dds.Bnderas.bit.Apagando=TRUE; //se Desactiva el Despligue de los Puntitos en pantalla del DDS
			  	  	   VFDcommand_Bold_DDS(FALSE);
			  	  	   break;
	    	case POSY2:procSensxDigitoEN(&(DDSpant->igxc5),POSXDDS+8,POSXDDS-8); 
	    			   break;//del case posy6
	    	case POSY4:procFasexDigitoEN(&(DDSpant->igxc5),POSXDDS+8,POSY6);
	    	           break;
	    	case POSY10:switch(DDSpant->igxc1){
	    				   case 0:DDSpant->igxc1=2;
	    				          VFDposicion_DDS(POSXDDSCURSORMENU,POSY10);
	    				   	   	  VFDserial_SendChar_DDS(' ');
	    				   	      VFDserial_SendChar_DDS('Z');
	    				          VFDserial_SendChar_DDS('>');
 				                  getASCII_from_UChar(zoom.Zoom,&z[0]);
								  VFDserial_SendChar_DDS(z[1]);
								  VFDserial_SendChar_DDS(z[2]);
								  VFDserial_SendChar_DDS(' ');
					              isEnable_Keypad(WAIT);
	    				          break;
	    				   case 2:DDSpant->igxc1=1;
	    				          getASCII_from_UChar(zoom.Zoom,&z[0]);
	    				          VFDposicion_DDS(POSXDDSNUMS-8,POSY10);
								  VFDserial_SendChar_DDS(' ');
								  VFDserial_SendChar_DDS(z[1]);
								  VFDserial_SendChar_DDS('>');
								  VFDserial_SendChar_DDS(z[2]);
								  VFDserial_SendChar_DDS(' ');
					              isEnable_Keypad(WAIT);
								  break;
	    				   case 1:DDSpant->igxc1=0;
	    				          if(zoom.Zoom==0)
	    				        	  zoom.Zoom=99;
 				                  getASCII_from_UChar(zoom.Zoom,&z[0]);
 				                  VFDposicion_DDS(POSXDDSCURSORMENU,POSY10);
							      VFDserial_SendChar_DDS('>');
							      VFDserial_SendChar_DDS('Z');
							      VFDserial_SendChar_DDS(' ');
							      VFDserial_SendChar_DDS(z[1]);
							      VFDserial_SendChar_DDS(z[2]);
							      VFDserial_SendChar_DDS(' ');
							      VFDserial_SendChar_DDS(' ');
							      re_Calcular_Buffers_DDS(zoom.Zoom);
							      displayZoom(POSXDDSNUMS,POSY10);//PINTAR el numero de la variable del zoom
							      isEnable_Keypad(WAIT);///deactivado el teclado
								  dds.Bnderas.bit.DDS1_BORRAR=TRUE;//borrar la pantlla del DDS quitar todos los puntos
								  dds.Bnderas.bit.Apagando=TRUE; //se Desactiva el Despligue de los Puntitos en pantalla del DDS
							      displayControladorPantalla(PANTALLA_DDS);
								  dds.Bnderas.bit.DDS_Reload=TRUE;//repintar el DDS con los puntos guardados en los buffers
								  dds.Bnderas.bit.clean_Buffers=FALSE;//NO BORRAR BUFERS
								  isEnable_Keypad(WAIT);
							      break;
	    				   default:break; }
	    		        break;//fin de POSY10
			case POSY12:if(DDSpant->igxc4==0){
				                  DDSpant->igxc4=1;
	    				          VFDposicion_DDS(POSXDDSNUMS,POSY12);
							      VFDserial_SendChar_DDS('1');
							      VFDserial_SendChar_DDS(0x01);
								  isEnable_Keypad(WAIT);
			                      return;}
                        if(DDSpant->igxc4==1){
                        	      DDSpant->igxc4=0;
	    				          VFDposicion_DDS(POSXDDSNUMS,POSY12);
							      VFDserial_SendChar_DDS('0');
							      VFDserial_SendChar_DDS(0x01);
								  isEnable_Keypad(WAIT);
								  return;}
				   	    break;//fin case POSY12
	    	case POSY14:dds.Bnderas.bit.DDS1_BORRAR=TRUE;
	    	            dds.Bnderas.bit.Apagando=TRUE; //se Desactiva el Despligue de los Puntitos en pantalla del DDS
	    	            //BorrarDDS=YES; //deprecated ImportanteÂ¡Â¡ CUANDO regresemos al DDS el cursos estara en la esquina
			  	  	  	displayControladorPantalla(PANTALLA_DDS);
			  	  	  	dds.Bnderas.bit.clean_Buffers=TRUE;//borrar los buffer donde se guardan los puntos que se han pintado y detectado
	    	           // if(modificado==YES){debug porner esto lo quite porque marca error pero si debe ir
						//  MemoEEPROM(SAVE,PANTALLA_DDS);
						//  modificado=NOP; }      
	    	            break;
	    	default:break;}//fin switch 	            
	     
    	
}//fin PANTALLA_DDSkeyEN------------------

/* FIN  PANTALLA_DDSkey+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void MenuInformacionUsuariokeyUP(void){
	if(cursory==POSY2){//PRIMERA POSICION DESPUES DE LA X    
		   VFDposicion(POSX0,POSY2);
		   VFDserial_SendChar(' ');cursory=POSY0;
		   VFDposicion(POSXESQUINA,cursory);
		   VFDserial_SendChar('>');VFDserial_SendChar('X');
		   return;}//posicion primera
		 if(cursory==POSY0) return;
		 if(cursory<(POSY14+1)){
		   VFDposicion(POSX0,cursory); 
		   VFDserial_SendChar(' ');--cursory;
		   VFDposicion(POSX0,--cursory);
		   VFDserial_SendChar('>'); return;}
}//fin MenuInformacionUsuariokeyUP -------------------

void MenuInformacionUsuariokeyRT(void){return;} 
void MenuInformacionUsuariokeyLF(void){return;}

void MenuInformacionUsuariokeyDN(void){
	if(cursory==POSY0){//PRIMERA POSICION DESPUES DE LA X
		   VFDposicion(POSXESQUINA,POSY0);
		   VFDserial_SendChar(' ');VFDserial_SendChar('x');
		   cursorx=POSX0;cursory=POSY2;
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar('>'); return;}
		 if(cursory==POSY14){//PRIMERA POSICION DESPUES DE LA X
		   return;}
		 if(cursory<POSY14){
			VFDposicion(POSX0,cursory);
			VFDserial_SendChar(' ');
			cursorx=POSX0;++cursory;
			VFDposicion(cursorx,++cursory);
			VFDserial_SendChar('>'); return;}
}//fin MenuInformacionUsuariokeyDN----------------------------------------

void MenuInformacionUsuariokeyEN(void){
	if(cursory==POSY2) {/*displayControladorPantalla(HACER_PRUEBA_DE_RUIDO;*/  return;}
	if(cursory==POSY4) {gpoc.igpoc0=0;//resetea las var de despliegue de numeros
		                displayControladorPantalla(MEDIDAS_DE_VOLTAJE);  return;}
	if(cursory==POSY6) {/*displayControladorPantalla(AJUSTE_DE_PRODUCTOINFO); */ return;}
	if(cursory==POSY8) {displayControladorPantalla(TIEMPO_DE_RECHAZOINFO);  return;}
	if(cursory==POSY10){displayControladorPantalla(AJUSTE_SISTEMA_ENTRADAS);  return;}
	if(cursory==POSY12){displayControladorPantalla(ID_MAQUINA);  return;}
	if(cursory==POSY14){DisplayCuentaProducto();  return;}
	if(cursory==POSY0) {if(menu.contexto.Anterior==AJUSTE_DE_SISTEMA){
		                 displayControladorPantalla(AJUSTE_DE_SISTEMA);return;}//SI ACTIVAMOS AQUI EL menu control se ejecuta dos veces
						if(menu.contexto.Anterior==MENU_INSIGHT){
			                 displayControladorPantalla(MENU_INSIGHT);return;}//SI ACTIVAMOS AQUI EL menu control se ejecuta dos veces
						else displayControladorPantalla(PORTAL_INICIO);}
}//fin MenuInformacionUsuariokeyEN---------------------------------

/* FIN  MenuInformacionUsuariokeyEN++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * */

void MenuAdministrativokeyUP(void){
		if(cursory>POSY2){//PRIMERA POSICION DESPUES DE LA X
			  VFDposicion(cursorx,cursory);
			  VFDserial_SendChar(' ');
			  --cursory;
			  VFDposicion(cursorx,--cursory);
			  VFDserial_SendChar('>');
			  return;}
		else if(cursory==POSY2){VFDposicion(cursorx,cursory); 
			 VFDserial_SendChar(' ');
			 cursorx=POSXESQ235;cursory=POSY0; 
			 VFDposicion(cursorx,cursory);
		     VFDserial_SendChar('>');
			 VFDserial_SendChar('X');  }
}//fin  MenuAdministrativokeyUP  --------------

void MenuAdministrativokeyRT(void){return;}	 
void MenuAdministrativokeyLF(void){return;}

void MenuAdministrativokeyDN(void){
	if(cursory==POSY0){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		VFDserial_SendChar('x');
		cursorx=POSX0;cursory=POSY2;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');return;}
	else{
		if(cursory<POSY14){//POSICION FINAL
			VFDposicion(cursorx,cursory);
		    VFDserial_SendChar(' ');
		    ++cursory;
		    VFDposicion(cursorx,++cursory);
		    VFDserial_SendChar('>');return;}}
}//finn  MenuAdministrativokeyDN----------------------

void MenuAdministrativokeyEN(void){
if(cursory==POSY2) {displayControladorPantalla(configModificacionController(CONTROL_PASSWORD,MENU_ADMINISTRATIVO));}
if(cursory==POSY4) {displayControladorPantalla(configModificacionController(RELOJ_DE_SISTEMA,MENU_ADMINISTRATIVO));}
if(cursory==POSY6) { return;}
if(cursory==POSY8) {displayControladorPantalla(configModificacionController(ESTABLECER_IDIOMA,MENU_ADMINISTRATIVO));} 
if(cursory==POSY10){displayControladorPantalla(configModificacionController(INIT_MEMO_Y_REST,MENU_ADMINISTRATIVO));}
if(cursory==POSY12){rotateAjusteGuardiavar();
					displayAjusteGuardiaControl();
					configModificado(MENU_ADMINISTRATIVO);
					return;}
if(cursory==POSY14){rotateControldeUsuariovar();
					displayControldeUsuariovar(); 
					configModificado(MENU_ADMINISTRATIVO);
					return;}
if(cursory==POSY0){ displayControladorPantalla(configModificacionController(AJUSTE_DE_SISTEMA,MENU_ADMINISTRATIVO));
					MenuControl();return;}
				
}//FIN MenuAdministrativokeyEN-------------------------------
/* FIN   MenuAdministrativokey+++++++++    +++ MenuAdministrativokey  ++++++++++++*/
					

void ConfigIOsistemkeyUP(void){
	if(cursory>POSY4){//PRIMERA POSICION DESPUES DE LA X
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		--cursory;
		VFDposicion(cursorx,--cursory);
		VFDserial_SendChar('>');
        return;}//posicion primera
    else 
    	if(cursory==POSY4){
    		VFDposicion(cursorx,cursory); 
            VFDserial_SendChar(' ');
            cursorx=POSXESQ239;cursory=POSY0; //XPOS2=POSXESQ239
            VFDposicion(cursorx,cursory);
            VFDserial_SendChar('>');
            VFDserial_SendChar('X'); }
}//void ConfigIOsistemkeyUP(void){----------------------------

void ConfigIOsistemkeyRT(void){return;}	 
void ConfigIOsistemkeyLF(void){return;}

void ConfigIOsistemkeyDN(void){
	if(cursory==POSY0){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		VFDserial_SendChar('x');
		cursorx=POSX0;cursory=POSY4;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');return;}
	else{
		if(cursory<POSY14){//POSICION FINAL
		  VFDposicion(cursorx,cursory);
		  VFDserial_SendChar(' ');
		  ++cursory;
		  VFDposicion(cursorx,++cursory);
		  VFDserial_SendChar('>');return;}}
}//fin ConfigIOsistemkeyDN-----------------------------------


void ConfigIOsistemkeyEN(void){
	
	if(cursory==POSY4){
					  //rotateEntradasSistemasVars(_TACOMETRO_);
					  //displayEntradadeSistemas(_TACOMETRO_);
					  //configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);//indica que ha sido modificado la configuracnio de entradas de sistema 
					  displayControladorPantalla(ENTRADA_TACO);
					  return;}//enter en tacho
   if(cursory==POSY6){//rotateEntradasSistemasVars(_FOTOCELDA_);
					  //displayEntradadeSistemas(_FOTOCELDA_);
					  //configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);//indica que ha sido modificado la configuracnio de entradas de sistema
					  displayControladorPantalla(ENTRADA_FOTOCEL);
					  return;}
   if(cursory==POSY8){//rotateEntradasSistemasVars(_RECHAZO_);
					  //displayEntradadeSistemas(_RECHAZO_);
					  //configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);//indica que ha sido modificado la configuracnio de entradas de sistema 
	                   displayControladorPantalla(ENTRADA_TEST_RECHAZO);
	                   return;}
   if(cursory==POSY10){//rotateEntradasSistemasVars(_PRODUCTO_);
					   //displayEntradadeSistemas(_PRODUCTO_);
					   //configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);//indica que ha sido modificado la configuracnio de entradas de sistema
	   	   	   	   	   displayControladorPantalla(ENTRADA_TEST_PRODUCTO);	
	                   return;}
   if(cursory==POSY12){//rotateEntradasSistemasVars(_AUX1_);
					   //displayEntradadeSistemas(_AUX1_);
					   //configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);//indica que ha sido modificado la configuracnio de entradas de sistema
					   displayControladorPantalla(ENTRADA_AUX1);
	                   return;}
   if(cursory==POSY14) {//rotateEntradasSistemasVars(_AUX2_);
					    //displayEntradadeSistemas(_AUX2_);
					    //configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);//indica que ha sido modificado la configuracnio de entradas de sistema
					   displayControladorPantalla(ENTRADA_AUX2);
	                    return;}
   if(cursory==POSY0){displayControladorPantalla(configModificacionController(AJUSTE_DE_SISTEMA,CONFIGURAR_ENTRADAS_DE_SISTEMA));
					  return;}//SI ACTIVAMOS AQUI EL menu control se ejecuta dos veces
					 
}//FIN ConfigIOsistemkeyEN--------------------------------------------------------------------------- 
/* FIN   ConfigIOsistemkey++++++++++++++++++++++++   ConfigIOsistemkey+++++++++++++++++++++++++++++++++++++
 * */
		

void IDcomunicacioneskeyUP(void){
	if(cursory==POSY4){//PRIMERA POSICION DESPUES DE LA X
				cursorx -=8;
				VFDposicion(cursorx,cursory);
				VFDserial_SendChar(' ');
				cursorx=POSXESQUINA+8;cursory=POSY0;
				VFDposicion(cursorx,cursory);
				VFDserial_SendChar('>');VFDserial_SendChar('X');
				return;}//posicion primera
				else 
					if(cursory==POSY0){
						VFDposicion(cursorx,cursory); 
						VFDserial_SendChar(' ');VFDserial_SendChar('x');
						cursorx=POSXNUM-10;cursory=POSY4; 
						VFDposicion(cursorx,cursory);
						VFDserial_SendChar('>'); }
}//FIN IDcomunicacioneskeyUP

void IDcomunicacioneskeyRT(void){return;}	 
void IDcomunicacioneskeyLF(void){return;}

void IDcomunicacioneskeyDN(void){
	if(cursory==POSY4){//PRIMERA POSICION DESPUES DE LA X
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		cursorx=POSXESQUINA+8;cursory=POSY0;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');VFDserial_SendChar('X');
		return;}//posicion primera
    else
    	if(cursory==POSY0){
    	    VFDposicion(cursorx,cursory); 
            VFDserial_SendChar(' ');VFDserial_SendChar('x');
            cursorx=POSXNUM-10;cursory=POSY4; 
            VFDposicion(cursorx,cursory);
            VFDserial_SendChar('>'); }
}//FIN IDcomunicacioneskeyDN +++++++++++++++++++++++++++++++++++++++++

void IDcomunicacioneskeyEN(void){
	if(cursory==POSY4){displayIDcommNUM();return;}//enter en tacho
	if(cursory==POSY6){/*esto debe estar vacio    */return;}
	if(cursory==POSY8){/*esto debe estar vacio    */return;}
	if(cursory==POSY10){/*esto debe estar vacio    */return;}
	if(cursory==POSY12){/*esto debe estar vacio    */return;}
	if(cursory==POSY14) {/*esto debe estar vacio    */return;}
	if(cursory==POSY0){
		displayControladorPantalla(configModificacionController(AJUSTE_DE_SISTEMA,ID_COMUNICACIONES));
		return;}//SI ACTIVAMOS AQUI EL menu control se ejecuta dos veces
				
}// FIN  IDcomunicacioneskeyEN++++++++++++++++++++++++++++++++++++++++++++++++++++++
//FIN DE IDcomunicacioneskeyUP++++++++++++++++++++++++++++++++++++++++++++
					

void ControlFrecuenciaskeyUP(void){
	if(cursory==POSY4){//PRIMERA POSICION DESPUES DE LA X    
		VFDposicion(POSX0,cursory);
		VFDserial_SendChar(' ');cursory=POSY0;
		VFDposicion(POSXESQUINA,cursory);
		VFDserial_SendChar('>');VFDserial_SendChar('X');
		return;}//posicion primera
	 if(cursory==POSY8){
			VFDposicion(POSX0,POSY8); 
			VFDserial_SendChar(' ');cursory=POSY4;
			VFDposicion(POSX0,cursory);
			VFDserial_SendChar('>');return;}
	 if(cursory==POSY12){
			VFDposicion(POSX0,POSY12); 
			VFDserial_SendChar(' ');cursory=POSY8;
			VFDposicion(POSX0,cursory);
			VFDserial_SendChar('>'); return;}
	 if(cursory==POSY14){
		 VFDposicion(POSX0,POSY14); 
		 VFDserial_SendChar(' ');cursory=POSY12;
		 VFDposicion(POSX0,cursory);
		 VFDserial_SendChar('>');return;}
}// FIN ControlFrecuenciaskeyUP---------------------------

void ControlFrecuenciaskeyRT(void){
	if(cursory==POSY4){
		VFDposicion(POSXCFNUM,POSY4);
		if(++Alta>9) Alta=0;
		VFDserial_SendChar(Alta+0x30U);
		return;}
	if(cursory==POSY8){
		VFDposicion(POSXCFNUM,POSY8);
		if(++Media>9) Media=0;
		VFDserial_SendChar(Media+0x30U);
		return;}
	if(cursory==POSY12){
		VFDposicion(POSXCFNUM,POSY12);
		if(++Baja>9) Baja=0;
		VFDserial_SendChar(Baja+0x30U);
		return;}
}//fin  ControlFrecuenciaskeyRT----------------------------

void ControlFrecuenciaskeyLF(void){
	if(cursory==POSY4){
		VFDposicion(POSXCFNUM,POSY4);
		if(Alta==0) Alta=9; else Alta--;
		VFDserial_SendChar(Alta+0x30U);
		return;}
    if(cursory==POSY8){
    	VFDposicion(POSXCFNUM,POSY8);
    	if(Media==0) Media=9; else Media--;
		VFDserial_SendChar(Media+0x30U);
		return;}
    if(cursory==POSY12){
    	VFDposicion(POSXCFNUM,POSY12);
    	if(Baja==0) Baja=9; else Baja--;
    	VFDserial_SendChar(Baja+0x30U);
    	return;} 
}//fin ControlFrecuenciaskeyLF-------------------------------

void ControlFrecuenciaskeyDN(void){
	if(cursory==POSY0){//PRIMERA POSICION DESPUES DE LA X
		VFDposicion(POSXESQUINA,POSY0);
		VFDserial_SendChar(' ');VFDserial_SendChar('x');
		cursorx=POSX0;cursory=POSY4;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>'); return;}
	if(cursory==POSY4){//PRIMERA POSICION DESPUES DE LA X
		VFDposicion(POSX0,POSY4);
		VFDserial_SendChar(' ');
		cursorx=POSX0;cursory=POSY8;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>'); return;}
	if(cursory==POSY8){
		VFDposicion(POSX0,POSY8);
		VFDserial_SendChar(' ');
		cursorx=POSX0;cursory=POSY12;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>'); return;}
	if(cursory==POSY12){
		VFDposicion(POSX0,POSY12);
		VFDserial_SendChar(' ');
		cursorx=POSX0;cursory=POSY14;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>'); return;}
}//fin ControlFrecuenciaskeyDN--------------------------------------------------

void ControlFrecuenciaskeyEN(void){
	if(cursory==POSY4) {/*esto debe estar vacio   */return;}
	if(cursory==POSY6) {/*esto debe estar vacio   */return;}
	if(cursory==POSY8) {/*esto debe estar vacio   */return;}
	if(cursory==POSY10){/*esto debe estar vacio   */return;}
	if(cursory==POSY12){/*esto debe estar vacio   */return;}
	if(cursory==POSY14){MemoEEPROM_DDS(SAVE,CONTROL_DE_FRECUENCIA,RESET_DDS_PWG);
						   /*init_DDSpantalla()*/;/*esto debe estar vacio   */return;}
	if(cursory==POSY0) {MemoEEPROM_DDS(SAVE,CONTROL_DE_FRECUENCIA,RESET_DDS_PWG);
						displayControladorPantalla(AJUSTE_DE_SISTEMA);
						return;}//SI ACTIVAMOS AQUI EL menu control se ejecuta dos veces
return;}// fin ControlFrecuenciaskeyEN---------------------------------------------------
/* FIN ControlFrecuenciaskeyUP+++++++++++++++++++++++++++++++++++++++++++++++++
 * */


void MedidasVoltajekeyUP(void){return;}
void MedidasVoltajekeyRT(void){return;}	 
void MedidasVoltajekeyLF(void){return;}
void MedidasVoltajekeyDN(void){return;}
void MedidasVoltajekeyEN(void){
	if(cursory==POSY2) {displayControladorPantalla(HACER_PRUEBA_DE_RUIDO);  return;}
	if(cursory==POSY4) {displayControladorPantalla(MEDIDAS_DE_VOLTAJE);       return;}
	if(cursory==POSY6) {displayControladorPantalla(AJUSTE_DE_PRODUCTOINFO);   return;}
	if(cursory==POSY8) {displayControladorPantalla(TIEMPO_DE_RECHAZOINFO);    return;}
	if(cursory==POSY10){displayControladorPantalla(AJUSTE_SISTEMA_ENTRADAS);  return;}
	if(cursory==POSY12){displayControladorPantalla(ID_MAQUINA);        return;}
	if(cursory==POSY14){displayControladorPantalla(CUENTA_PRODUCTO);   return;}
	if(cursory==POSY0) {gpoc.igpoc0=0;//resetar valores de voltaje a desplegar
		                displayControladorPantalla(INFORMACION_DE_USUARIO); return;}//SI ACTIVAMOS AQUI EL menu control se ejecuta dos veces                          	        
}//fin MedidasVoltajekeyEN--------------------------------------------------

void MedidasVoltajeskeyUP(void){return;}
void MedidasVoltajeskeyRT(void){return;}	 
void MedidasVoltajeskeyLF(void){return;}
void MedidasVoltajeskeyDN(void){return;}
void MedidasVoltajeskeyEN(void){
	if(cursory==POSY2) {  return;}
	if(cursory==POSY4) {  return;}
	if(cursory==POSY6) {  return;}
	if(cursory==POSY8) {  return;}
	if(cursory==POSY10){  return;}
	if(cursory==POSY12){  return;}
	if(cursory==POSY14){  return;}
	if(cursory==POSY0) {gpoc.igpoc0=0;//resetar valores de voltaje a desplegar
		                displayControladorPantalla(PRUEBA); return;}//SI ACTIVAMOS AQUI EL menu control se ejecuta dos veces                          	        
}//fin MedidasVoltajekeyEN--------------------------------------------------




void AjusteProductoInfokeyUP(void){return;}
void AjusteProductoInfokeyRT(void){return;}	 
void AjusteProductoInfokeyLF(void){return;}
void AjusteProductoInfokeyDN(void){return;}
void AjusteProductoInfokeyEN(void){return;}

void TiempoRechazoInfokeyUP(void){return;}
void TiempoRechazoInfokeyRT(void){return;}	 
void TiempoRechazoInfokeyLF(void){return;}
void TiempoRechazoInfokeyDN(void){return;}
void TiempoRechazoInfokeyEN(void){displayControladorPantalla(INFORMACION_DE_USUARIO);}

void AjusteSistemaEntradaskeyUP(void){return;}
void AjusteSistemaEntradaskeyRT(void){return;}	 
void AjusteSistemaEntradaskeyLF(void){return;}
void AjusteSistemaEntradaskeyDN(void){return;	}
void AjusteSistemaEntradaskeyEN(void){displayControladorPantalla(INFORMACION_DE_USUARIO);}

void IDmaquinakeyUP(void){return;}
void IDmaquinakeyRT(void){return;}	 
void IDmaquinakeyLF(void){return;}
void IDmaquinakeyDN(void){return;}
void IDmaquinakeyEN(void){displayControladorPantalla(INFORMACION_DE_USUARIO);}

void CuentaProductokeyUP(void){return;}
void CuentaProductokeyRT(void){return;}	 
void CuentaProductokeyLF(void){return;}
void CuentaProductokeyDN(void){return;}
void CuentaProductokeyEN(void){return;}

void EstablecerIdiomakeyUP(void){}
void EstablecerIdiomakeyRT(void){}	 
void EstablecerIdiomakeyLF(void){}
void EstablecerIdiomakeyDN(void){}
void EstablecerIdiomakeyEN(void){displayControladorPantalla(MENU_ADMINISTRATIVO);}

void ControlpasswordkeyUP(void){
	if(cursory==POSY6){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		cursorx=POSXESQ235;cursory=POSY0;	  
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');VFDserial_SendChar('X');
		return;}
	if(cursory==POSY0){return;}
	if(cursory<=POSY12){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');--cursory;
		VFDposicion(cursorx,--cursory);
		VFDserial_SendChar('>');
		return;}
}//fIN  ControlpasswordkeyUP+++++++++++++++++++++++++++++++++++++++

void ControlpasswordkeyRT(void){
	 
	  if(cursorx==POSX0CNTRLPASSW){
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar(' ');
		   cursorx=POSX9CNTRLPASSW;
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar('>');
	       return;}
	  if(cursorx==POSX9CNTRLPASSW){
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar(' ');
		   cursorx=POSX0CNTRLPASSW;
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar('>');
		   return;}
return;}//fin ControlpasswordkeyRT----------------------------------------
		  

void ControlpasswordkeyLF(void){
	ControlpasswordkeyRT();
return;}//--ControlpasswordkeyLF------------------------------------------

void ControlpasswordkeyDN(void){
	if(cursory==POSY0){
		VFDposicion(POSXESQ235,POSY0); 
		VFDserial_SendChar(' ');VFDserial_SendChar('x');
		cursory=POSY6;
		cursorx=POSX0CNTRLPASSW;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');return;}
  if(cursory==POSY12)
	    return;
  if(cursory>=POSY6){
    VFDposicion(cursorx,cursory);
    VFDserial_SendChar(' ');++cursory;
    VFDposicion(cursorx,++cursory);
    VFDserial_SendChar('>');
    return; }
return;}//fin ControlpasswordkeyDN++++++++++++++++++++++++++++++++++++++++++

void ControlpasswordkeyEN(void){
  if(cursory==POSY0){
	  menu.contexto.Actual=CONTROL_PASSWORD;
	  displayControladorPantalla(MENU_ADMINISTRATIVO);
    return;}
  if(cursorx==POSX9CNTRLPASSW){
	   switch(cursory){
		   case POSY6: rotateContrasenaVars(POSY6);
					   displayControlContrasenaVal(1);
				       configModificado(CONTROL_PASSWORD);
				       return;break;
		   case POSY8: rotateContrasenaVars(POSY8); 
				       displayControlContrasenaVal(2);  
					   configModificado(CONTROL_PASSWORD);
					   return;break;
           case POSY10:rotateContrasenaVars(POSY10);
				       displayControlContrasenaVal(3);
					   configModificado(CONTROL_PASSWORD);
					   return;break;
		   case POSY12:rotateContrasenaVars(POSY12);
					   displayControlContrasenaVal(4);
				 	   configModificado(CONTROL_PASSWORD);
				 	   return;break;
		   case POSY0: displayControladorPantalla(configModificacionController(MENU_ADMINISTRATIVO,CONTROL_PASSWORD));	                
					   return;break;
		   default:return;break;}
        return;}//EN IF POSX0CNTRLPASSW
  if(cursorx==POSX0CNTRLPASSW){//abajo:convertir posy6,8,10,12 en nivel 1,2,3,4
	   arg0=(cursory/2)-2;//que nivel de password se quiere modificar
	   arg1=0;//monitor de error de password invalida
	   arg2=0;//despligue de texto=Confirmar Password
//	   menu.contexto.Anterior=CONTROL_PASSWORD;
	   configModificado(CONTROL_PASSWORD);
	   displayControladorPantalla(TEXT_PROCESSOR);
	   return;}//fin if POSX0CNTRLPASSW  
}//FIN ControlpasswordkeyEN

void RelojSistemakeyUP(void){
	if(cursory==POSY2){
		VFDposicion(POSX0,cursory);
		VFDserial_SendChar(' ');cursory=POSY0;	  
		VFDposicion(POSXESQUINA,cursory);
		VFDserial_SendChar('>');VFDserial_SendChar('X');
		cursory=POSY0; return;}
  if(cursory==POSY0){return;}
  if(cursory<=POSY12){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');--cursory;
		VFDposicion(cursorx,--cursory);
		VFDserial_SendChar('>');return;}
}//FIN RelojSistemakeyUP--------------------------------------------

void RelojSistemakeyRT(void){
	configReloj(KEY_RT);
    configModificado(RELOJ_DE_SISTEMA);//en cual contexto se modificaron variables
}//fin   RelojSistemakeyRT++++++++++++++++++++++++++++++++++++++++++++++++++

void RelojSistemakeyLF(void){
	configReloj(KEY_LF);
	configModificado(RELOJ_DE_SISTEMA);//en cual contexto se modificaron variables
}//----------------------------------------------------------------------

void RelojSistemakeyDN(void){
	if(cursory==POSY0){
		VFDposicion(POSXESQUINA,POSY0); 
		VFDserial_SendChar(' ');VFDserial_SendChar('x');
		cursory=POSY2;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');return;}
	if(cursory==POSY12) return;
	if((cursory+1)>POSY2){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');++cursory;
		VFDposicion(cursorx,++cursory);
		VFDserial_SendChar('>');return; }
}//FIN RelojSistemakeyDN---------------------------------------------------

void RelojSistemakeyEN(void){
	if(cursory==POSY2) { return;  }
	if(cursory==POSY4) { return;  }
	if(cursory==POSY6) { return;  }
	if(cursory==POSY8) { return;  }
	if(cursory==POSY10){ return;  }
	if(cursory==POSY0){
		segundero(DESACTIVADO);
		displayControladorPantalla(configModificacionController(MENU_ADMINISTRATIVO,RELOJ_DE_SISTEMA));}
		
}//FIN RelojSistemakeyEN----------------------------------------------------


void IngenieriaGracidakeyUP(void){
	if((cursory==POSY0)&&(cursorx==POSX0)){//PRIMERA POSICION DESPUES DE LA X
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		cursorx=POSXESQ235;cursory=POSY0;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');
		VFDserial_SendChar('X');
		 return;}
	if(cursory>POSY0){
		VFDposicion(cursorx,cursory); 
		VFDserial_SendChar(' ');
		cursory--;
		VFDposicion(cursorx,--cursory);
		VFDserial_SendChar('>'); return; }
}//FIN IngenieriaGracidakeyUP-------------------------------------

void IngenieriaGracidakeyRT(void){return;}	 
void IngenieriaGracidakeyLF(void){return;}

void IngenieriaGracidakeyDN(void){
	  if((cursory==POSY0)&&(cursorx==POSXESQ235)){//PRIMERA POSICION DESPUES DE LA X
			VFDposicion(cursorx,cursory);
			VFDserial_SendChar(' ');VFDserial_SendChar('x');
			cursorx=POSX0;cursory=POSY0;
			VFDposicion(cursorx,cursory);
			VFDserial_SendChar('>');return;}
	  if(cursory<POSY14){
			VFDposicion(cursorx,cursory); 
			VFDserial_SendChar(' ');
			cursory++;
			VFDposicion(cursorx,++cursory);
			VFDserial_SendChar('>'); return; }
}//FIN IngenieriaGracidakeyDN----------------------------------------

void IngenieriaGracidakeyEN(void){
	if(cursory==POSY2){displayControladorPantalla(GANANCIA_);  return;}
    if(cursory==POSY4){displayControladorPantalla(FILTRO_);return;}
	if(cursory==POSY6){  return;}
	if(cursory==POSY8){  return;}
	if(cursory==POSY10){displayControladorPantalla(PARTE_11); return;}
	if(cursory==POSY12){return;}
	if(cursory==POSY14){displayControladorPantalla(INGENIERIA_GRACIDA2);return;}
	if((cursory==POSY0)&&(cursorx==POSX0)){
						displayControladorPantalla(TIPO_DE_MAQUINA);return;   }
	if((cursory==POSY0)&&(cursorx==POSXESQ235)){
						displayControladorPantalla(AJUSTE_DE_SISTEMA);return;}
				         	        
}//fin IngenieriaGracidakeyEN-------------------------------------

void InitMemoria_y_RestaurarkeyUP(void){return;}
void InitMemoria_y_RestaurarkeyRT(void){
		VFDposicion(8,POSY12);
		VFDserial_SendChar('n');
		VFDserial_SendChar('o');
		cursorx=POSX125;
		VFDposicion(cursorx,POSY12);
		VFDserial_SendChar('S');
		VFDserial_SendChar('I');
}//fin 	 InitMemoria_y_RestaurarkeyRT---

void InitMemoria_y_RestaurarkeyLF(void){
		cursorx=8;
		VFDposicion(cursorx,POSY12);
		VFDserial_SendChar('N');
		VFDserial_SendChar('O');
		VFDposicion(POSX125,POSY12);
		VFDserial_SendChar('s');
		VFDserial_SendChar('i');
}//FIN InitMemoria_y_RestaurarkeyLF---------

void InitMemoria_y_RestaurarkeyDN(void){return;}
void InitMemoria_y_RestaurarkeyEN(void){
		if(cursorx==POSX125)
			initMemoria_y_Restaurar();
		displayControladorPantalla(PORTAL_INICIO);
}//FIN InitMemoria_y_RestaurarkeyEN

void TipodeMaquinakeyUP(void){
	if(cursory==POSY2){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');cursory=POSY0;
		VFDposicion(POSXESQ235,cursory);
		VFDserial_SendChar('>');VFDserial_SendChar('X');return;}
	if(cursory==POSY0) return;
	if(cursory<=POSY14){
		 VFDposicion(cursorx,cursory);
		 VFDserial_SendChar(' ');--cursory;
		 VFDposicion(cursorx,--cursory);
		 VFDserial_SendChar('>');return;}
}//FIN TipodeMaquinakeyUP-----------------------------------

void TipodeMaquinakeyRT(void){return;}	 
void TipodeMaquinakeyLF(void){return;}
void TipodeMaquinakeyDN(void){
	if(cursory==POSY0){
		VFDposicion(POSXESQ235,cursory);
		VFDserial_SendChar(' ');VFDserial_SendChar('x');
		++cursory;
		VFDposicion(cursorx,++cursory);
		VFDserial_SendChar('>');return;}
	if(cursory<POSY14){
		 VFDposicion(cursorx,cursory);
		 VFDserial_SendChar(' ');cursory++;
		 VFDposicion(cursorx,++cursory);
		 VFDserial_SendChar('>');return;}
}//FIN  TipodeMaquinakeyDN----------------------------------

void TipodeMaquinakeyEN(void){
	if(cursory==POSY0){ displayControladorPantalla(INGENIERIA_GRACIDA);}
	switch(cursory){
		case POSY2:tipo_deMaquina=MULTI_100_286_875;return;
		case POSY4:tipo_deMaquina=MULTI_30_100_26;return;
		case POSY6:tipo_deMaquina=SINGLE_875_286;return;
		case POSY8:tipo_deMaquina=SINGLE_100_30;return;
		case POSY10:tipo_deMaquina=VF_875_286;return;
		case POSY12:tipo_deMaquina=VF_100_30;return;
		case POSY14:tipo_deMaquina=PHARMACEUTICAL;return;
		default: tipo_deMaquina=PHARMACEUTICAL;return;}
    configModificado(TIPO_DE_MAQUINA);//en cual contexto se modificaron variables
    displayControladorPantalla(configModificacionController(INGENIERIA_GRACIDA,TIPO_DE_MAQUINA));//si se modificaron variables en un contexto mandamos el contexto de estas seguro?
    return;//return del keyEN 
}//FIN TipodeMaquinakeyEN---------------------------------

void MenuGananciakeyUP(void){
	if(cursory==POSY2){
    VFDposicion(POSX0,cursory);
    VFDserial_SendChar(' ');cursory=POSY0;	  
    VFDposicion(POSXESQ235,cursory);
    VFDserial_SendChar('>');VFDserial_SendChar('X'); return;}
  if(cursory==POSY0){return;}
  if(cursory<=POSY4){
    VFDposicion(cursorx,cursory);
    VFDserial_SendChar(' ');--cursory;
    VFDposicion(cursorx,--cursory);
    VFDserial_SendChar('>');return;}}
void MenuGananciakeyRT(void){}	 
void MenuGananciakeyLF(void){}
void MenuGananciakeyDN(void){if(cursory==POSY0){
    VFDposicion(POSXESQ235,POSY0); 
    VFDserial_SendChar(' ');VFDserial_SendChar('x');
    cursory=POSY2;
    VFDposicion(cursorx,cursory);
    VFDserial_SendChar('>');return;}
  if(cursory<POSY4){
    VFDposicion(cursorx,cursory);
    VFDserial_SendChar(' ');cursory++;
    VFDposicion(cursorx,++cursory);
    VFDserial_SendChar('>');return;}}

void MenuGananciakeyEN(void){
	if(cursory==POSY2){
		gananciaValuesRotary(DRIVER);
		displayGananciaValues(DRIVER);
		configModificado(GANANCIA_);//YA SE mdifico el valor
		return;}
	if(cursory==POSY4){ 
		gananciaValuesRotary(ANALOGA);
		displayGananciaValues(ANALOGA);
		configModificado(GANANCIA_);//YA SE mdifico el valor
		return;}
	if(cursory==POSY0){
		displayControladorPantalla(configModificacionController(INGENIERIA_GRACIDA,GANANCIA_));
		return;}                              
}//FIN MenuGananciakeyEN------------------------------------------

void MenuFiltrokeyUP(void){if(cursory==POSY2){
    VFDposicion(POSX0,cursory);
    VFDserial_SendChar(' ');cursory=POSY0;	  
    VFDposicion(POSXESQ235,cursory);
    VFDserial_SendChar('>');VFDserial_SendChar('X'); return;}
 if(cursory==POSY0){return;}}
void MenuFiltrokeyRT(void){}	 
void MenuFiltrokeyLF(void){}
void MenuFiltrokeyDN(void){if(cursory==POSY0){
    VFDposicion(POSXESQ235,POSY0); 
    VFDserial_SendChar(' ');VFDserial_SendChar('x');
    cursory=POSY2;
    VFDposicion(cursorx,cursory);
    VFDserial_SendChar('>');return;}
  else return;}
void MenuFiltrokeyEN(void){if(cursory==POSY2){
    filtroVarRotate();
    displayfiltroVar();
    configModificado(FILTRO_);
    return;}
if(cursory==POSY0){
	displayControladorPantalla(configModificacionController(INGENIERIA_GRACIDA,FILTRO_));
	return;}
return;//return del keyEN 
}

void IngenieriaGracida2keyUP(void){if(cursory==POSY2){
    VFDposicion(cursorx,cursory);
    VFDserial_SendChar(' ');cursory=POSY0;	  
    VFDposicion(POSXESQ235,cursory);
    VFDserial_SendChar('>');VFDserial_SendChar('X'); return;}
  if(cursory==POSY0){return;}
  if(cursory<=POSY14){
 	 VFDposicion(cursorx,cursory);
 	 VFDserial_SendChar(' ');--cursory;
 	 VFDposicion(cursorx,--cursory);
 	 VFDserial_SendChar('>');return;}
}//fin IngenieriaGracida2keyUP-------------------------------

void IngenieriaGracida2keyRT(void){return;}	 
void IngenieriaGracida2keyLF(void){return;}

void IngenieriaGracida2keyDN(void){if(cursory==POSY0){
    VFDposicion(POSXESQ235,POSY0); 
    VFDserial_SendChar(' ');VFDserial_SendChar('x');
    cursory=POSY2;
    VFDposicion(cursorx,cursory);
    VFDserial_SendChar('>');return;}
  if(cursory<POSY14){
    VFDposicion(cursorx,cursory);
    VFDserial_SendChar(' ');cursory++;
    VFDposicion(cursorx,++cursory);
    VFDserial_SendChar('>');return;}
}//FIN  IngenieriaGracida2keyDN-----------------------

void IngenieriaGracida2keyEN(void){
	switch(cursory){
		case POSY2:/* INIZIAIZAR MEMORIA*/ break;
		case POSY4: PSU_IO_var_rotate();displayPSU_IO_var(); 
					configModificado(INGENIERIA_GRACIDA2);break;
		case POSY6: ErrorAnalogoVarRotate();
				    displayErrorAnalogoVar();
					configModificado(INGENIERIA_GRACIDA2); break;
		case POSY8:/* ajustar tolerancia*/break;
		case POSY10:ReporteProductoVarRotate();
					displayReporteProductoVar();
					configModificado(INGENIERIA_GRACIDA2);break;
		case POSY12:displayControladorPantalla(configModificacionController(INGENIERIA_GRACIDA2,PRUEBA));
				    break;/* Prueba */
		case POSY14: /*  Auto set-up sequence */ break;
		case POSY0: //INGRIA-GRACD2=16h  INGERIA-GRACIDA:A9
		           displayControladorPantalla(configModificacionController(INGENIERIA_GRACIDA,INGENIERIA_GRACIDA2));
	 	           break;
		default:break;}
}// FIN IngenieriaGracida2keyEN----------------------------------------------------------------



void AjustesAvanzadoskeyUP(void){
	if(cursory>POSY8){//PRIMERA POSICION DESPUES DE LA X
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		--cursory;
		VFDposicion(cursorx,--cursory);
		VFDserial_SendChar('>');
		return;}
    else 
    	if(cursory==POSY8){VFDposicion(cursorx,cursory); 
            VFDserial_SendChar(' ');
            cursorx=POSXESQ235;cursory=POSY0; 
            VFDposicion(cursorx,cursory);
            VFDserial_SendChar('>');
            VFDserial_SendChar('X');  }
}// FIN AjustesAvanzadoskeyUP------------------------------

void AjustesAvanzadoskeyRT(void){return;}	 
void AjustesAvanzadoskeyLF(void){return;}

void AjustesAvanzadoskeyDN(void){
	if(cursory==POSY0){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		VFDserial_SendChar('x');
		cursorx=POSX0;cursory=POSY8;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');return;}
	else{
#if (_TIPO_DE_MAQUINA_==PHARMACEUTICO)
		if(cursory<POSY12)
#else
#if (_TIPO_DE_MAQUINA_==TRIPLE_FRECUENCIA)
		if(cursory<POSY14)//POSICION FINAL
#endif
#endif		
			{VFDposicion(cursorx,cursory);
			VFDserial_SendChar(' ');
			++cursory;
			VFDposicion(cursorx,++cursory);
			VFDserial_SendChar('>');return;}}
}//FIN AjustesAvanzadoskeyDN-----------------------

void AjustesAvanzadoskeyEN(void){
switch(cursory){
	case POSY2: break;
	case POSY4: break;
	case POSY6: break;
	case POSY8: displayControladorPantalla(NOMBRE_PRODUCTO);break;
	case POSY10: displayControladorPantalla(PARAMETROS_DE_DETECCION);break;
	case POSY12:displayControladorPantalla(configModificacionController(AJUSTES_AVANZADOS,AJUSTE_TIEMPO_RECHAZO));
	            break;
	case POSY14: displayControladorPantalla(FRECUENCIA_SELECT);break; 
	case POSY0:if((arg5==CAMBIAR)||(arg6==CAMBIAR))
						 configModificado(AJUSTES_AVANZADOS);
			   if(arg3==Deteccion.Sensibilidad)
						 configModificado(AJUSTES_AVANZADOS);
			   if(arg0==phasefrac)
						 configModificado(AJUSTES_AVANZADOS);
			   if(arg1==phase)
					configModificado(AJUSTES_AVANZADOS);
					displayControladorPantalla(configModificacionController(AJUSTE_PARAMETRICO_DE_PRODUCTO,AJUSTES_AVANZADOS));
			   break;
	default:break;}
}//FIN AjustesAvanzadoskeyEN---------------------------------------------

void FrecuenciaSelectkeyUP(void){
	if(cursory==POSY2){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');cursory=POSY0;	  
		VFDposicion(POSXESQ235,cursory);
		VFDserial_SendChar('>');
		VFDserial_SendChar('X'); return;}
	if(cursory==POSY0){return;}
	if(cursory<=POSY6){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');--cursory;
		VFDposicion(cursorx,--cursory);
		VFDserial_SendChar('>');return;}
}//FIN FrecuenciaSelectkeyUP-----------------------------

void FrecuenciaSelectkeyRT(void){return;}	 
void FrecuenciaSelectkeyLF(void){return;}

void FrecuenciaSelectkeyDN(void){
	if(cursory==POSY0){
		VFDposicion(POSXESQ235,POSY0); 
		VFDserial_SendChar(' ');VFDserial_SendChar('x');
		cursory=POSY2;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');return;}
	if(cursory<POSY6){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');cursory++;
		VFDposicion(cursorx,++cursory);
		VFDserial_SendChar('>');return;}
}// FIN FrecuenciaSelectkeyDN----------------------------

void FrecuenciaSelectkeyEN(void){
	if(cursory==POSY2){frecuenciaSeleccion=ALTA; configModificado(FRECUENCIA_SELECT);
					   displayFrecuenciaSelect_active_var();return;}
	if(cursory==POSY4){frecuenciaSeleccion=MEDIA; configModificado(FRECUENCIA_SELECT);
						displayFrecuenciaSelect_active_var();return;}
	if(cursory==POSY6){frecuenciaSeleccion=BAJA;configModificado(FRECUENCIA_SELECT);
						displayFrecuenciaSelect_active_var();return;}
	if(cursory==POSY8){/*  Nombres de producto       */return;}
	if(cursory==POSY10){/* Parametros de deteccion   */return;}
	if(cursory==POSY12){/* Ajustar tiempo de rechazo */return;}
	if(cursory==POSY14){ return;}
	if(cursory==POSY0) {
		displayControladorPantalla(configModificacionController(AJUSTES_AVANZADOS,FRECUENCIA_SELECT));
	  	return;}
}//FIN FrecuenciaSelectkeyEN---------------------------------------

void PruebaMenukeyUP(void){
	if(cursory>POSY2){//PRIMERA POSICION DESPUES DE LA X
		  VFDposicion(cursorx,cursory);
		  VFDserial_SendChar(' ');
		  --cursory;
		  VFDposicion(cursorx,--cursory);
		  VFDserial_SendChar('>');
		  return;}
	else if(cursory==POSY2){VFDposicion(cursorx,cursory); 
         VFDserial_SendChar(' ');
         cursorx=POSXESQ235;cursory=POSY0; 
         VFDposicion(cursorx,cursory);
		 VFDserial_SendChar('>');
		 VFDserial_SendChar('X');  }
}// fin PruebaMenukeyUP-------------------------------------

void PruebaMenukeyRT(void){return;}	 
void PruebaMenukeyLF(void){return;}

void PruebaMenukeyDN(void){
	if(cursory==POSY0){VFDposicion(cursorx,cursory);
	     VFDserial_SendChar(' ');
		 VFDserial_SendChar('x');
		 cursorx=POSX0;cursory=POSY2;
		 VFDposicion(cursorx,cursory);
	     VFDserial_SendChar('>');return;}
	else{
		if(cursory<POSY14){//POSICION FINAL
			VFDposicion(cursorx,cursory);
		    VFDserial_SendChar(' ');
		    ++cursory;
		    VFDposicion(cursorx,++cursory);
		    VFDserial_SendChar('>');return;}}
}//FIN PruebaMenukeyDN---------------------------------------------

void PruebaMenukeyEN(void){
	if(cursory==POSY2) {displayControladorPantalla(I2CMENU);return;}
	if(cursory==POSY4) {displayControladorPantalla(I2CEEPROMREAD);return;}
	if(cursory==POSY6) {displayControladorPantalla(I2CNVRAMDATA);return;}
	if(cursory==POSY8) {displayControladorPantalla(SPI_MENU);return;}
	if(cursory==POSY10){displayControladorPantalla(PRUEBAS_SALIDAS);return;}
	if(cursory==POSY12){displayControladorPantalla(PRUEBAS_ENTRADAS);return;}
	if(cursory==POSY14){displayControladorPantalla(MEDIDAS_DE_VOLTAJES); return;}
	if(cursory==POSY0) {displayControladorPantalla(INGENIERIA_GRACIDA2);return;}
}//FIN PruebaMenukeyEN---------------------------------------------

void I2CMenukeyUP(void){
	if(cursory>POSY2){//PRIMERA POSICION DESPUES DE LA X
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		--cursory;
		VFDposicion(cursorx,--cursory);
		VFDserial_SendChar('>');
		return;}
	else 
		if(cursory==POSY2){VFDposicion(cursorx,cursory); 
           VFDserial_SendChar(' ');
           cursorx=POSXESQ235;cursory=POSY0; 
           VFDposicion(cursorx,cursory);
           VFDserial_SendChar('>');
           VFDserial_SendChar('X');  }
}//FIN I2CMenukeyUP -----------------

void I2CMenukeyRT(void){return;}	 
void I2CMenukeyLF(void){return;}

void I2CMenukeyDN(void){
	if(cursory==POSY0){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		VFDserial_SendChar('x');
		cursorx=POSX0;cursory=POSY2;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');return;}
	else{
		if(cursory<POSY14){//POSICION FINAL
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		++cursory;
		VFDposicion(cursorx,++cursory);
		VFDserial_SendChar('>');return;}}
}//FIN I2CMenukeyDN --------------------------------------

void I2CMenukeyEN(void){
	if(cursory==POSY2) {pruebaI2C();/*MenuControl();*/return;}
	if(cursory==POSY4) {prueba2I2C();/*displayControladorPantalla(*//*MenuControl();*/return;}
	if(cursory==POSY6) {pruebatxe2prom();/*MenuControl();*/ return;}
	if(cursory==POSY8) {pruebarxe2prom();/*MenuControl();*/return;}
	if(cursory==POSY10){/*MenuControl();*/return;}
	if(cursory==POSY12){/*MenuControl();*/return;}
	if(cursory==POSY14){/*MenuControl();*/ return;}
	if(cursory==POSY0){displayControladorPantalla(PRUEBA);
                       return;}}

void I2CeepromDatakeyUP(void){return;}
void I2CeepromDatakeyRT(void){return;}	 
void I2CeepromDatakeyLF(void){return;}
void I2CeepromDatakeyDN(void){return;}

void I2CeepromDatakeyEN(void){
	if(cursory==POSY2) {/*MenuControl();*/return;}
	if(cursory==POSY4) {/*displayControladorPantalla(*//*MenuControl();*/return;}
	if(cursory==POSY6) {/*MenuControl();*/ return;}
	if(cursory==POSY8) {/*MenuControl();*/return;}
	if(cursory==POSY10){/*MenuControl();*/return;}
	if(cursory==POSY12){/*MenuControl();*/return;}
	if(cursory==POSY14){/*MenuControl();*/ return;}
	if(cursory==POSY0){displayControladorPantalla(PRUEBA);
					   VFDclrscr();
					   return;}
}// FIN I2CeepromDatakeyEN

void I2CNVRAMdatakeyUP(void){return;}
void I2CNVRAMdatakeyRT(void){return;}	 
void I2CNVRAMdatakeyLF(void){return;}
void I2CNVRAMdatakeyDN(void){return;}

void I2CNVRAMdatakeyEN(void){
	if(cursory==POSY2) {/*MenuControl();*/return;}
	if(cursory==POSY4) {/*displayControladorPantalla(*//*MenuControl();*/return;}
	if(cursory==POSY6) {/*MenuControl();*/ return;}
	if(cursory==POSY8) {/*MenuControl();*/return;}
	if(cursory==POSY10){/*MenuControl();*/return;}
	if(cursory==POSY12){/*MenuControl();*/return;}
	if(cursory==POSY14){/*MenuControl();*/ return;}
	if(cursory==POSY0){displayControladorPantalla(PRUEBA);
					   VFDclrscr();
					   return;}
}//fin I2CNVRAMdatakeyEN ---------------------------------------

void SPIMenukeyUP(void){
	if(cursory>POSY2){//PRIMERA POSICION DESPUES DE LA X
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		--cursory;
		VFDposicion(cursorx,--cursory);
		VFDserial_SendChar('>');
		return;}
	else
		if(cursory==POSY2){
				VFDposicion(cursorx,cursory); 
			 VFDserial_SendChar(' ');
			 cursorx=POSXESQ235;cursory=POSY0; 
			 VFDposicion(cursorx,cursory);
			 VFDserial_SendChar('>');
			 VFDserial_SendChar('X');  }
}//FIN SPIMenukeyUP -------------------------------------------
void SPIMenukeyRT(void){return;}	 
void SPIMenukeyLF(void){return;}

void SPIMenukeyDN(void){
	if(cursory==POSY0){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		VFDserial_SendChar('x');
		cursorx=POSX0;cursory=POSY2;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');return;}
	else{
		if(cursory<POSY14){//POSICION FINAL
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar(' ');
		   ++cursory;
			VFDposicion(cursorx,++cursory);
			VFDserial_SendChar('>');return;}}
}//FIN SPIMenukeyDN---------------------------

void SPIMenukeyEN(void){
	if(cursory==POSY2) {sendSPI_DSScomm(); return;}
	if(cursory==POSY4) {displayControladorPantalla(DISPLAY_ADC);return;}
	if(cursory==POSY6) {/*displayControladorPantalla(;*/return;}
	if(cursory==POSY8) {/*ontextoActual=;*/return;}
	if(cursory==POSY10){/*MenuControl();*/return;}
	if(cursory==POSY12){/*MenuControl();*/return;}
	if(cursory==POSY14){/*MenuControl();*/ return;}
	if(cursory==POSY0){displayControladorPantalla(PRUEBA);
					   return;}
}// FIN SPIMenukeyEN--------------------------------------------

void PruebasSalidasMenukeyUP(void){
	if(cursory>POSY2){//PRIMERA POSICION DESPUES DE LA X
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		--cursory;
		VFDposicion(cursorx,--cursory);
		VFDserial_SendChar('>');
		return;}
	else 
		if(cursory==POSY2){VFDposicion(cursorx,cursory); 
			 VFDserial_SendChar(' ');
			 cursorx=POSXESQ235;cursory=POSY0; 
			 VFDposicion(cursorx,cursory);
			VFDserial_SendChar('>');
			 VFDserial_SendChar('X');  }}
void PruebasSalidasMenukeyRT(void){}	 
void PruebasSalidasMenukeyLF(void){}
void PruebasSalidasMenukeyDN(void){
	if(cursory==POSY0){VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		VFDserial_SendChar('x');
		cursorx=POSX0;cursory=POSY2;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');return;}
	else{
		if(cursory<POSY10){//POSICION FINAL
			VFDposicion(cursorx,cursory);
			VFDserial_SendChar(' ');
			++cursory;
			VFDposicion(cursorx,++cursory);
			VFDserial_SendChar('>');return;}}}

void PruebasSalidasMenukeyEN(void){
	if(cursory==POSY2) {prueba_SalidasON(cursory,&ReleRechazo); return;}
	if(cursory==POSY4) {prueba_SalidasON(cursory,&ReleFalla);   return;}
	if(cursory==POSY6) {prueba_SalidasON(cursory,&ReleAlarma);  return;}
	if(cursory==POSY8) {prueba_SalidasON(cursory,&ReleBloqueo); return;}
	if(cursory==POSY10){prueba_SalidasON(cursory,&SalidaAux);   return;}
	if(cursory==POSY12){  return;}
	if(cursory==POSY14){/*MenuControl();*/ return;}
	if(cursory==POSY0){displayControladorPantalla(PRUEBA);
    /*MenuControl();*/return;
}}

void PruebasEntradasMenukeyUP(void){}
void PruebasEntradasMenukeyRT(void){}	 
void PruebasEntradasMenukeyLF(void){}
void PruebasEntradasMenukeyDN(void){}

void PruebasEntradasMenukeyEN(void){
	displayControladorPantalla(PORTAL_INICIO);
	isMenuPruebaEntradasON=FALSE;
	MenuControl();return;
	 return;//return del keyEN    
 }

void AjustesAvanzadosAPPkeyUP(void){}
void AjustesAvanzadosAPPkeyRT(void){}	 
void AjustesAvanzadosAPPkeyLF(void){}
void AjustesAvanzadosAPPkeyDN(void){}
void AjustesAvanzadosAPPkeyEN(void){}




void ParametrosdeDeteccionkeyUP(void){

	if(ParamDet->igxc0>0){//et sado enter para seleccionar el char que se desea cambiar
		if(cursory==POSY4){display_pushFIFO_UShortInt(&Deteccion.Sensibilidad,ParamDet->igxc0,SUMAR,POSY4,PARAMETROS_DE_DETECCION); return;}
		if(cursory==POSY8){display_pushFIFO_UShortInt(&Altura,ParamDet->igxc0,SUMAR,POSY8,PARAMETROS_DE_DETECCION); return; }
	    if(cursory==POSY6){ 
		  operacionPhase(ParamDet->igxc0,SUMAR);
		  configModificado(AJUSTE_PARAMETRICO_DE_PRODUCTO);
		  display_pushFIFOcOP_Phase_var(POSXAJUSPROD,POSY8);
		  return;} }//fin 
	if(cursory>POSY4){//PRIMERA POSICION DESPUES DE LA X
		pushFIFOcOP_inicioCMD();
		pushFIFOcOP_VFDposicion(cursorx,cursory);
		pushFIFOcOP_VFDserial_SendChar(' ');
		--cursory;
		pushFIFOcOP_VFDposicion(cursorx,--cursory);
		pushFIFOcOP_VFDserial_SendChar('>');
		pushFIFOcOP_finCMD();
		return;}
    else
    	if(cursory==POSY4){
			pushFIFOcOP_inicioCMD();
			pushFIFOcOP_VFDposicion(cursorx,cursory); 
			pushFIFOcOP_VFDserial_SendChar(' ');
			 cursorx=POSXESQ235;cursory=POSY0; 
			 pushFIFOcOP_VFDposicion(cursorx,cursory);
			 pushFIFOcOP_VFDserial_SendChar('>');
			 pushFIFOcOP_VFDserial_SendChar('X');  
			 pushFIFOcOP_finCMD(); }//fin if    	if(cursory==POSY4){
}//fin ParametrosdeDeteccionkeyUP-----------------

void ParametrosdeDeteccionkeyRT(void){
	if(cursory==POSY2){return; }
	if(cursory==POSY4){if(ParamDet->igxc0>0)//para que no empalmen los algoritmos
		                    return;
		               if(++Deteccion.Sensibilidad>32000)
 	                        Deteccion.Sensibilidad=0;
                       pushFIFOcOP_displayUINT_var(POSXPARAMDETECT,POSY4,&Deteccion.Sensibilidad,NONE);
                       configModificado(PARAMETROS_DE_DETECCION);
                       return;} 
	if(cursory==POSY6){//aqui falta codigo de variable de la phase
			   if(++phasefrac>9){
				   phasefrac=0;
				   if(++phase>179){
						   phase=0;
					      phasefrac=0;}}
	           display_pushFIFOcOP_Phase_var(POSXPARAMDETECT,POSY6);
			   configModificado(PARAMETROS_DE_DETECCION);
			   return;}
	if(cursory==POSY8){	
		if(++Altura>32000)
			Altura=0;
	    pushFIFOcOP_displayUINT_var(POSXPARAMDETECT,POSY8,&Altura,NONE);
	    configModificado(PARAMETROS_DE_DETECCION);
		return;}//aqui falta el codigo para manipular la variable MarcarAltura
	if(cursory==POSY10){ Display_rotate_MarcarAltura(POSXPARAMDETECT,POSY10);return;} //aqui falta el codigo para manipular la GANANCIA
	if(cursory==POSY12){Display_rotate_Ganancia_var(POSXPARAMDETECT,POSY12); return;}	
	if(cursory==POSY14){rotate_frecuencySelect_var();
						displayfrecuencySelect_var();
						configModificado(PARAMETROS_DE_DETECCION);}
}//FIN     ParametrosdeDeteccionkeyRT------------------------------------------                
	 
void ParametrosdeDeteccionkeyLF(void){
	if(cursory==POSY2){return;}
	if(cursory==POSY4){
		if(ParamDet->igxc0>0)
			return;
		if(--Deteccion.Sensibilidad==0)
			 Deteccion.Sensibilidad=32000;
			 pushFIFOcOP_displayUINT_var(POSXPARAMDETECT,POSY4,&Deteccion.Sensibilidad,NONE);
			 configModificado(PARAMETROS_DE_DETECCION);
			 return;} 
	if(cursory==POSY6){// variable de la phase
		if(phasefrac!=0)
			phasefrac--;
		else{phasefrac=9;
			if(phase==0)
				phase=179;
			else phase--;}
		display_pushFIFOcOP_Phase_var(POSXCFNUM,POSY8);//fin contextobuffer 0
		configModificado(PARAMETROS_DE_DETECCION);
		return;
		return;}//aqui falta el codigo para manipular la variable  Altura
	if(cursory==POSY8){ 
		   if(--Altura==0)
			   Altura=32000;
		       pushFIFOcOP_displayUINT_var(POSXPARAMDETECT,POSY8,&Altura,NONE);
			   configModificado(PARAMETROS_DE_DETECCION);
		       return;}//aqui falta el codigo para manipular la variable MarcarAltura
	if(cursory==POSY10){
		   Display_rotate_MarcarAltura(POSXPARAMDETECT,POSY10);
		   return;} //aqui falta el codigo para manipular la GANANCIA
	if(cursory==POSY12){Display_rotate_Ganancia_var(POSXPARAMDETECT,POSY12); return;}
	if(cursory==POSY14){rotate_frecuencySelect_var();
						displayfrecuencySelect_var();
						configModificado(PARAMETROS_DE_DETECCION);} 
}//fin ParametrosdeDeteccionkeyLF-----------------------------------------------Description	Resource	Path	Location	Type
//Undefined : "displayGanancia_var" Referenced from "ParametrosdeDeteccionkeyEN" in	39_15_08_IN1UP		 	C/C++ Problem




void ParametrosdeDeteccionkeyDN(void){
unsigned char a[5]={" "},*p=0;	
    p=&a[0];
	if(ParamDet->igxc0>0){//et sado enter para seleccionar el char que se desea cambiar
		if(cursory==POSY4){display_pushFIFO_UShortInt(&Deteccion.Sensibilidad,ParamDet->igxc0,RESTAR,POSY4,PARAMETROS_DE_DETECCION);return;}//fin igxm
		if(cursory==POSY8){display_pushFIFO_UShortInt(&Altura,ParamDet->igxc0,RESTAR,POSY8,PARAMETROS_DE_DETECCION);return; }
	    return;}
	if(cursory==POSY0){
			pushFIFOcOP_inicioCMD();
			pushFIFOcOP_VFDposicion(cursorx,cursory);
			pushFIFOcOP_VFDserial_SendChar(' ');
			pushFIFOcOP_VFDserial_SendChar('x');
			cursorx=POSX0;cursory=POSY4;
			pushFIFOcOP_VFDposicion(cursorx,cursory);
			pushFIFOcOP_VFDserial_SendChar('>');
			pushFIFOcOP_finCMD();
			return;}
	else{
		if(cursory<POSY12){//POSICION FINAL
			pushFIFOcOP_inicioCMD();
			pushFIFOcOP_VFDposicion(cursorx,cursory);
			pushFIFOcOP_VFDserial_SendChar(' ');
			  ++cursory;
			  pushFIFOcOP_VFDposicion(cursorx,++cursory);
			  pushFIFOcOP_VFDserial_SendChar('>');
			  pushFIFOcOP_finCMD();
			  return;}}
}//FIN ParametrosdeDeteccionkeyDN-------------------------



 
void ParametrosdeDeteccionkeyEN(void){
unsigned char a[6]={" "},*p=0;	
unsigned char b[6]={" "};  
 switch(cursory){
	case POSY2:break;
	case POSY4:display_UInt_CharsSelected(Deteccion.Sensibilidad,&(ParamDet->igxc0),POSY4,POSXPARAMDETECT);break;
	case POSY6: return;
	case POSY8: return;
	case POSY10:Display_rotate_MarcarAltura(POSXPARAMDETECT,POSY10);return;
	case POSY12:Display_rotate_Ganancia_var(POSXPARAMDETECT,POSY12);return;
	case POSY14:rotate_frecuencySelect_var();
				displayfrecuencySelect_var();
				configModificado(PARAMETROS_DE_DETECCION);
				return;break;
	case POSY0:
		displayControladorPantalla(configModificacionController(AJUSTES_AVANZADOS,PARAMETROS_DE_DETECCION));
		menu.bytes.semMenuPendiente=TRUE; //habilitamos escape deL Menu por sistema Operativo
		displayBoxDestructor(ACTIVADO);//destruir y parar la barra de deteccion.
		//MenuControl();toma el control el SO
		return;break;
	default:break;}
}// FIN ParametrosdeDeteccionkeyEN----------------------
	

void ADCsValueskeyUP(void){return;}
void ADCsValueskeyRT(void){return;}	 
void ADCsValueskeyLF(void){return;}
void ADCsValueskeyDN(void){return;}
void ADCsValueskeyEN(void){
	if(cursory==POSY2) {/*displayControladorPantalla(*/return;}
	if(cursory==POSY4) {/*ontextoActual=;*/return;}
	if(cursory==POSY6) {/*displayControladorPantalla(;*/return;}
	if(cursory==POSY8) {/*displayControladorPantalla(;*/return;}
	if(cursory==POSY10){/*displayControladorPantalla(;*/return;}
	if(cursory==POSY12){/*displayControladorPantalla(*/return;}
	if(cursory==POSY14){/*displayControladorPantalla(;*/return;}
	if(cursory==POSY0){	displayADCvalores=DESACTIVADO;
						displayControladorPantalla(SPI_MENU);
						/*MenuControl();*/return;}
}// FIN ADCsValueskeyEN----------------------------------------

void SeleccionarProductokeyUP(void){		
	BorrarProductokey_UP(&displaySeleccionarProductoTitle);
}//fin SeleccionarProductokeyUP --------------------------------

void SeleccionarProductokeyRT(void){return;}	 
void SeleccionarProductokeyLF(void){return;}

void SeleccionarProductokeyDN(void){
	BorrarProductokeyDown(&displaySeleccionarProductoTitle);
}//FIN SeleccionarProductokeyDN -------------------------

void SeleccionarProductokeyEN(void){
	if(cursory==POSY0){//nos salimos no seleccionamos nada
		displayControladorPantalla(AJUSTE_DE_PRODUCTO);//ir al Padre
		 return;} 
    arg0=getiProducSelected(cursory);//get the product index selected
	if(arg0==indiceProducto)//es diferente al actual el seleccionado
		return;
	if(arg0>MAXIMO_NUMERO_PROD)
		__asm(Halt);//debug error de  diseño de software
	if(validarName(arg0)){//el seleccionado es vacio? es valido
//		displayControladorPantalla(PORTAL_INICIO);
		menu.contexto.Anterior=SELECCIONAR_PRODUCTO;
		configModificado(SELECCIONAR_PRODUCTO);//se modifico en el menu de BORRAR PRODUCTo
		displayControladorPantalla(configModificacionController(AJUSTE_DE_PRODUCTO,SELECCIONAR_PRODUCTO));}
	else 
		__asm(Halt);//debug error de diseño de spftware
}//fin SeleccionarProductokeyEN--------------------------------------------------

void monitorInvalidPassword(void){//arg0 esta ocupado en CONTROL PASSWORD
if((arg1==ERROR_PASS)&&(menu.contexto.Anterior==CONTROL_PASSWORD)){//hubo password invalida????
      arg1=0;	 
	  displayTextoProcessor();}
}//fin monitorInvalidPassword-------------------------------------------------------------

void TextoProcessorkeyUP(void){
	monitorInvalidPassword();
	if(cursory==POSY0)
		return;//EN ESTE CONTEXTO NO USAMOS arg0 PORQUE guarda el DATOS
	if(cursory==POSY6){//DEL menu anterior que se va usar en el menu que viene
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		cursorx=POSX_TEXT_PROCS_OK;cursory=POSY0;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');
		return;}
	if(cursory>POSY6)
	if(cursory<(POSY14+1)){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		cursory-=2;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');
		return;}
}//FIN TextoProcessorkeyUP----------------------------

void TextoProcessorkeyRT(void){
	monitorInvalidPassword();
   if(cursory==POSY0){
	if(cursorx==POSX_TEXT_PROCS_OK){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		cursorx=POSX_TEXT_PROCS_X;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');
		VFDserial_SendChar('X');
		return;}
	if(cursorx==POSX_TEXT_PROCS_X){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		VFDserial_SendChar('x');
		cursorx=POSX_TEXT_PROCS_OK;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');
		return;}}
	if(cursory==POSY14)
		if(cursorx==POSX_COL3){
			VFDposicion(cursorx,cursory);
			VFDserial_SendChar(' ');
			cursorx=POSX_COL7;
			VFDposicion(cursorx,cursory);
			VFDserial_SendChar('>');
			return;}
	 if(cursory>POSY4){
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar(' ');
		   cursorx=getColTextProc(RIGHT);
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar('>');
		   return;}        
}//FIN TextoProcessorkeyRT----------------------------

void TextoProcessorkeyLF(void){
	monitorInvalidPassword();	
  if(cursory==POSY0){
	 if(cursorx==POSX_TEXT_PROCS_OK){
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar(' ');
 		   cursorx=POSX_TEXT_PROCS_X;
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar('>');
		   VFDserial_SendChar('X');
			 return;}
	  if(cursorx==POSX_TEXT_PROCS_X){
	    	VFDposicion(cursorx,cursory);
		    VFDserial_SendChar(' ');
		    VFDserial_SendChar('x');
		    cursorx=POSX_TEXT_PROCS_OK;
			 VFDposicion(cursorx,cursory);
			 VFDserial_SendChar('>');
			 return;}}
	   if(cursory==POSY14)
	   if(cursorx==POSX_COL7){
			VFDposicion(cursorx,cursory);
			VFDserial_SendChar(' ');
		    cursorx=POSX_COL3;
			VFDposicion(cursorx,cursory);
			VFDserial_SendChar('>');
		   return;}
		if(cursory>POSY4){
			VFDposicion(cursorx,cursory);
	   	    VFDserial_SendChar(' ');
		    cursorx=getColTextProc(LEFT);
		    VFDposicion(cursorx,cursory);
		    VFDserial_SendChar('>');}
}//FIN TextoProcessorkeyLF -----------------------------------------

void TextoProcessorkeyDN(void){
	monitorInvalidPassword();
	if(cursory==POSY0){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
    if(cursorx==POSX_TEXT_PROCS_X)
        VFDserial_SendChar('x');
    	cursorx=POSX_COL1;cursory=POSY6;
    	VFDposicion(cursorx,cursory);
	    VFDserial_SendChar('>');
	    return;}
	if(cursory==POSY6){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		cursory+=2;
		VFDposicion(cursorx,cursory);
	    VFDserial_SendChar('>');
	    return;}
	if(cursory==POSY12){
		if(cursorx==POSX_COL4)
		   return;
		if(cursorx==POSX_COL5)
			return;
		if(cursorx==POSX_COL6)
			return;}
		if(cursory>POSY6){
			if(cursory==POSY14)
				return;
			VFDposicion(cursorx,cursory);
			VFDserial_SendChar(' ');
			cursory+=2;
			VFDposicion(cursorx,cursory);
			VFDserial_SendChar('>');
			return;}
}//fin  TextoProcessorkeyDN--------------------------


void TextoProcessorkeyEN(void){
unsigned char a[]= "Contrasena invalida",*p=0,c;
word n;
unsigned char k;
	
    isEnable_Keypad2(WAIT,144000); 
	if(cursory==POSY0){//displayControladorPantalla(NuevoProducto.menuPadre;
		if(cursorx==POSX_TEXT_PROCS_OK){//aceptamos el nombre
			switch(menu.contexto.Anterior){
				case NUEVO_PRODUCTO:
					    textProc->igxc0=NOMBRE_PRODUCTO_SIZE-1;
						for(k=textProc->igxc0;k>0;k--)//insertamos el NAME_INIT
							Text[k]=Text[k-1];
						Text[0]=NAME_INIT;//insertar la clave de correcto
						configModificado(NUEVO_PRODUCTO);//modificamos el contenido de menu NUEVO_PRODUCTO
						configModificacionController(NUEVO_PRODUCTO,NUEVO_PRODUCTO);//resguarda el cntexto modificado
						menu.contexto.Actual=NUEVO_PRODUCTO;
						menu.contexto.padre=AJUSTE_DE_PRODUCTO;
                        vaciar_A2B(&Text[0],&name[0],0,sizeof(Text)-1);
						displayControladorPantalla(AJUSTE_DE_PRODUCTO_MODE); //pasamos al siguiente screen AJUSTE DE PRODUCTO MODE
						return;
						break;//fin nuevo producto
				case NOMBRE_PRODUCTO:
					 switch(menu.contexto.padre){
					   case AJUSTE_DE_PRODUCTO:if(!menu.contexto.Modificado==NOMBRE_PRODUCTO)
						                              break;
					   case AJUSTE_PARAMETRICO_DE_PRODUCTO://venimos de ajuste parametrico de producto y de ajustes avanzados
											   swapArrays(&name[0],&Text[0],NOMBRE_PRODUCTO_SIZE);//el nuevo nombre a name y el viejo nombre a text para guardarlo en caso de error o cambio de parecer
											   cleanArrayName(&name[0],sizeof(name),0,0);
											   cleanArrayName(&Text[0],sizeof(Text),0,0);
											   displayControladorPantalla(AJUSTES_AVANZADOS);
											   break;
					   default:break; }
			           break;//FIN  nombre producto
				case CONTROL_PASSWORD:
					    if(arg2==CONFIRMAR){//es confirmacion de contraseña nueva
					    	arg2=0;//se limpia confirmacion
					    	if(compareString(&Text[0],&igxm5[0])){//se confirmo bien
					    		setPasswords(arg0,&Text[0]);
					    		menu.contexto.Actual=AJUSTE_DE_PRODUCTO;//sera el padre
					    		displayControladorPantalla(CONTROL_PASSWORD);}
					    	else{//en la confirmacion se puso mal la contraseña
					    		textProc->igxc0=0;//iniciamos desplegando mayuculas
								textProc->igxc1=0;//indice del arreglo del Texto. 
								textProc->igxc4=0;//no ha habido escritura de ascii
								//arg0=0;//gurada el level 1 a 4 que quiere modificarse
								arg1=0;//error de  valida password
								arg2=0;//para saber si estamo en confirmcion
								displayTextoProcessor(); //se reinicia el text-procesor
					    	     }}
					    else{//vamos a poner una nueva password
							if(validatePasswords(p,&Text[0])){//validamos la password
							      saveTextBuffer(&Text[0]);//save Text buffer
								  arg2=CONFIRMAR; //configurar arg2 para poner confirmar pass
								  displayTextoProcessor();} //se reinicia el text-procesor
							else {//anuncio de que es l password invalida
	        	    		      VFDposicion(0,POSY2);
	        	    		      VFDserial_SendBlock(&a[0],sizeof(a),&n); 
 		                          arg1=ERROR_PASS;}//arg0 esta ocupado
					    break;//fin CONTROL_PASSWORD
				case SUPERVISOR:k=textProc->igxc0;
					            p=&k;//vamos a guardar a que nivel se accedio
					            if(validatePassword(p,&Text[0])){//es valida la password y regresa cual nivel es la pass
					        	     passControl[_AUTHOR_]=*p;//textProc->igxc0;//el nivel de autorizacion lo guarda
					        	     menu.contexto.Anterior=SUPERVISOR;
					        	     displayControladorPantalla(passControl[_DESTIN_]);
				                     return;}
					            else{ displayControladorPantalla(PORTAL_INICIO);
					                  break;}
					            break;//fin case supervisor
					    
				default:break;}}
            return;}//fin OK cursorx==POSX_TEXT_PROCS_OK---------
       if(cursorx==POSX_TEXT_PROCS_X){//NO ACEPTAMOS EL NOMBRE
		   switch(menu.contexto.Anterior){
			    case NUEVO_PRODUCTO:   displayControladorPantalla(MENU_INSIGHT);
			                           return;break;
			    case NOMBRE_PRODUCTO:
			    	      switch(menu.contexto.padre){
							  case AJUSTE_PARAMETRICO_DE_PRODUCTO:
										 arg5=0;//NO se ha modificado el nombre del producto
										 displayControladorPantalla(AJUSTES_AVANZADOS);                                                     
										 break;
							  default:break;}					 
						  break;
			    case CONTROL_PASSWORD:textProc->igxc0=0;//iniciamos desplegando mayuculas
			                          textProc->igxc1=0;//indice del arreglo del Texto. 
			                          textProc->igxc4=0;//no ha habido escritura de ascii
 	  	  		                      arg0=0;//se usara en la validacion de password mismo contexto
 	  	  		                      arg1=0;//error de  valida password
 	  	  		                      arg2=0;//importante
			    	                  displayControladorPantalla(CONTROL_PASSWORD);
			                          return;break;
			    case SUPERVISOR:menu.contexto.Actual=passControl[_DESTIN_];
			    	            displayControladorPantalla(passControl[_ORIGEN_]);
			    	            break;
			   default:return; break;}
   		      return;}//fin cursorx==POSX_TEXT_PROCS_X
	   	   return;}// fin ==POSY0--------------------------------------------------------------------------
 if(cursory>POSY0){
	 monitorInvalidPassword();  
     if(menu.contexto.Anterior==NOMBRE_PRODUCTO){
    	    TextProcessorSpecial();
    	    return;}
	 if(cursory==POSY14){
		if(cursorx==POSX_COL1){
			   if(textProc->igxc0==MAYUSCULAS)
				   return;
			   else{//esta activa alguna otra pantalla que no es mayusculas
					  VFDposicion(cursorx,cursory);
					  VFDserial_SendChar(' ');//limpiar el cursor actual
					  displayTextoProcessorMayusculas();
					  cursorx=POSX_COL1;cursory=POSY14;
					  VFDposicion(cursorx,cursory);
					  VFDserial_SendChar('>');
					  textProc->igxc0=MAYUSCULAS;
					  return;}return;}//fin col1
		if(cursorx==POSX_COL2){
			  if(textProc->igxc0==MINUSCULAS)
				  return;
			  else{
				  VFDposicion(cursorx,cursory);
				  VFDserial_SendChar(' ');
				  displayTextoProcessorMinusculas();
			  cursorx=POSX_COL2;cursory=POSY14;
			  VFDposicion(cursorx,cursory);
			  VFDserial_SendChar('>');
			  textProc->igxc0=MINUSCULAS;
				  return;} return;}
		if(cursorx==POSX_COL3){
			  if(textProc->igxc0==SYMBOL)
				  return;
			  else{
				  VFDposicion(cursorx,cursory);
				  VFDserial_SendChar(' ');
				  displayTextoProcessorSymbol();
				  cursorx=POSX_COL3;cursory=POSY14;
				  VFDposicion(cursorx,cursory);
					  VFDserial_SendChar('>');
					  textProc->igxc0=SYMBOL;
				 return;}return;}
		if(cursorx==POSX_COL7){
			  if(textProc->igxc1!=0){
				if(textProc->igxc1==1)
				   return;
				if(textProc->igxc4==DONE)
				textProc->igxc1-=1;
				VFDposicion((unsigned short)((textProc->igxc1)*8),POSY4);//mover el cursor uno a la izquierda
				VFDserial_SendChar(' ');//borramos el   ^
				VFDposicion((unsigned short)((--textProc->igxc1)*8),POSY4);//mover el cursor uno a la izquierda
				VFDserial_SendChar(CX);//escribimos el ^ uno antes de donde estaba
				textProc->igxc4=VERDE;//indica que hubo atraso del cursor
				return;}
	         return;}//fin POSX_COL7
		if(cursorx==POSX_COL8){//mueve cursor a la derecha
			if(textProc->igxc1<(NOMBRE_PRODUCTO_SIZE-1)){//indice del array
			   if(textProc->igxc4==DONE)
					textProc->igxc1-=1;
			   VFDposicion((unsigned short)((textProc->igxc1)*8),POSY4);
			   VFDserial_SendChar(' ');//borramos el   ^
			   VFDposicion((unsigned short)((++textProc->igxc1)*8),POSY4);//mover el cursor uno a la derecha
			   VFDserial_SendChar(CX);//escribimos el ^ uno antes de donde estaba
			   if(isNumLetter(Text[textProc->igxc1])==FALSE)
					 Text[textProc->igxc1]=' ';
			   textProc->igxc4=VERDE;//indica que hubo adelanto de cursor
		   return;}return;}//FIN POSX_COL8
		if(cursorx==POSX_COL9){//agrega espacio
			if(textProc->igxc4==DONE)
				 textProc->igxc1-=1;
			if(textProc->igxc1<=(NOMBRE_PRODUCTO_SIZE-1)){   //indice de l array
			  TextInsertSpace(textProc->igxc1);
			  DisplayNewTextProc();
			 textProc->igxc4=VERDE;//india que hubo un insert Space
		   return;}return;}
		if(cursorx==POSX_COL10){
			 if(textProc->igxc1==0)
				 return;
			 if((textProc->igxc4==DONE)&&(textProc->igxc1==1))//fix a bug
				 return;
			 if(textProc->igxc4==DONE)
				 textProc->igxc1-=1;
			 arg5=CAMBIAR;//se ha modificado el nombre del producto
			 c=TextBackSpace(textProc->igxc1);//revisr esta funcion
			 DisplayNewTextProc();
			 if((c>1)&&(c!=textProc->igxc1)){
				VFDposicion((unsigned short)((textProc->igxc1)*8),POSY4);
				VFDserial_SendChar(' ');
				textProc->igxc1=c;
				VFDposicion((unsigned short)((textProc->igxc1)*8),POSY4);
				VFDserial_SendChar(CX);//escribimos el ^ uno antes de donde estaba
				textProc->igxc4=VERDE;//india que hubo un back Space    
				return;}
			 else{return;}}
	     return;} //fin POSY14---------------------------------------------   
	          if(textProc->igxc1<NOMBRE_PRODUCTO_SIZE){//Escribir algun ascii en pantalla
	        	  arg5=CAMBIAR;//se ha modificado el nombre del producto
				  Text[textProc->igxc1]=getAscii(cursorx,cursory,textProc->igxc0);//guardamos el ascii seleccionado
				  //Text[textProc->igxc1+1]=0;//siguiente es cero como limite
		          if(menu.contexto.Anterior==NOMBRE_PRODUCTO){
		        	  VFDposicion((unsigned short)((textProc->igxc1-1)*8),POSY2);//posicion del cursor del nombre
		        	  VFDserial_SendChar(Text[textProc->igxc1]);
		        	  if(textProc->igxc1!=(NOMBRE_PRODUCTO_SIZE-1)){
							  if(textProc->igxc1!=0){//ess el indice del Text[]
								  VFDposicion((unsigned short)((textProc->igxc1-1)*8),POSY4);
								  VFDserial_SendChar(' ');}
							  VFDposicion((unsigned short)((textProc->igxc1)*8),POSY4);
							  VFDserial_SendChar(CX);}}
		          else{VFDposicion((unsigned short)((textProc->igxc1)*8),POSY2);//posicion del cursor del nombre
					  if((menu.contexto.Anterior==CONTROL_PASSWORD)||(menu.contexto.Anterior==SUPERVISOR))
						  VFDserial_SendChar('*');
					  else	  
						   VFDserial_SendChar(Text[textProc->igxc1]);
					  if(textProc->igxc1!=0){//ess el indice del Text[]
						  VFDposicion((unsigned short)((textProc->igxc1-1)*8),POSY4);
						  VFDserial_SendChar(' ');}
					  VFDposicion((unsigned short)((textProc->igxc1)*8),POSY4);
					  VFDserial_SendChar(CX);}
		          textProc->igxc1++;//lo desplegamos y apuntamos al que sigue indice de Text
		          textProc->igxc4=DONE;}//ha habido una escritua de ascii en pantalla
             return;}//fin Mas alla de POSY0 
          
 }//FIN DE TextoProcessorkeyEN-----------------------------------------------------------------------

//text processor for name modify	
void TextProcessorSpecial(void){
	  if(cursory==POSY14){
		  specialFunctions();
		  return;}
      if(textProc->igxc1>=NOMBRE_PRODUCTO_SIZE)//Escribir algun ascii en pantalla
          return;
	  arg5=CAMBIAR;//se ha modificado el nombre del producto
	  if(textProc->igxc1>(NOMBRE_PRODUCTO_SIZE-1))//si escribimos mucho llegara hasta 20 el minimo es 19
		  return;
	  Text[textProc->igxc1]=getAscii(cursorx,cursory,textProc->igxc0);//guardamos el ascii seleccionado
	  DisplayCharTextProc(textProc->igxc1,Text[textProc->igxc1]);
	  DisplayCursorTextProc(textProc->igxc1+1,textProc->igxc1);
	  textProc->igxc1++;
return;	
}//fin TextProcessorSpecial--------------------------------------


//metodo del procesador de textos
void specialFunctions(void){
unsigned char c;	
	 if(textProc->igxc1==0)
		  return;
	  switch(cursorx){
		  case POSX_COL1:break;
		  case POSX_COL2:break;
		  case POSX_COL3:break;
		  case POSX_COL4:break;
		  case POSX_COL5:break;
		  case POSX_COL6:break;
		  case POSX_COL7:textProc->igxc1=DisplayRecorrerIndiceIzqTextProc(textProc->igxc1,LEFT);break;
		  case POSX_COL8:textProc->igxc1=DisplayRecorrerIndiceIzqTextProc(textProc->igxc1,RIGHT);break; 
		  case POSX_COL9:textProc->igxc1=DisplayInsertSpaceTextProc(textProc->igxc1);break;
		  case POSX_COL10:c=TextBackSpace(textProc->igxc1);//Revisar esta funcionc: lugar actual, igxm1 lugar anterior
		                  DisplayNewTextProc();//despleiga el texto
		                  DisplayCursorTextProc(c,textProc->igxc1);//despliega el cursos en posicion actual.
		                  textProc->igxc1=c;
			             break;
		  default:break;  
	  }//fin switch
	
}//fin specialFunctions-------------------------------------------------





void AjusteProductoHijokeyUP(void){
	if(cursory==POSY2)
			return;
  if(cursory==POSY4){
      VFDposicion(cursorx,cursory);
      VFDserial_SendChar(' ');
      cursory=POSY2;
      VFDposicion(cursorx,cursory);
      VFDserial_SendChar('>');
      return;}
}//FIN  AjusteProductoHijokeyUP

void AjusteProductoHijokeyRT(void){return;}	 
void AjusteProductoHijokeyLF(void){return;}
void AjusteProductoHijokeyDN(void){
	if(cursory==POSY2){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		cursory=POSY4;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');
		return;}
    if(cursory==POSY4)
        	 return;
}// FIN AjusteProductoHijokeyDN

void AjusteProductoHijokeyEN(void){
	if(cursory==POSY2){//en desarrollo
        return;}
    if(cursory==POSY4){
		 displayControladorPantalla(AJUSTES_AVANZADOS);
		 MenuControl();
		 return;}    
}//fin AjusteProductoHijokeyEN ---------------------------------------------

void Parte11MenukeyUP(void){
	if(cursory>POSY4){
       VFDposicion(cursorx,cursory);
       VFDserial_SendChar(' ');
       --cursory;
       VFDposicion(cursorx,--cursory);
       VFDserial_SendChar('>');
       return;}
    else 
    	if(cursory==POSY4){VFDposicion(cursorx,cursory); 
	                       VFDserial_SendChar(' ');
	                       cursorx=POSXESQ235;cursory=POSY0; 
	                       VFDposicion(cursorx,cursory);
	                       VFDserial_SendChar('>');
	                       VFDserial_SendChar('X');  }
}//FIN Parte11MenukeyUP------------------------------------------

void Parte11MenukeyRT(void){return;}	 
void Parte11MenukeyLF(void){return;}

void Parte11MenukeyDN(void){
	if(cursory==POSY0){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		VFDserial_SendChar('x');
		cursorx=POSX0;cursory=POSY4;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');return;}
	else{if(cursory<POSY14){
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		++cursory;
		VFDposicion(cursorx,++cursory);
		VFDserial_SendChar('>');return;}}
}//Parte11MenukeyDN---------------------------

void Parte11MenukeyEN(void){
	if(cursory==POSY2){/*displayControladorPantalla(;*/ return;}
	if(cursory==POSY4){rotateVarParte11();  
				       displayvarParte11();
                       configModificado(PARTE_11);//indica que ha sido modificado la configuracnio de entradas de sistema 
                       return;}//-------------M8:poner el contexto al que se quiere entrar
	if(cursory==POSY6){/*displayControladorPantalla(*/return;}
	if(cursory==POSY8){/*displayControladorPantalla(*/return;}
	if(cursory==POSY10){/*displayControladorPantalla(*/return;}
	if(cursory==POSY12){/*displayControladorPantalla(*/return;}
	if(cursory==POSY14){/*displayControladorPantalla(*/return;}
	if(cursory==POSY0){
//  	                 displayControladorPantalla(configModificacionController(Parte11.menuPadre,Parte11.menuNodo);
  	               MenuControl();return;}
}//FIN Parte11MenukeyEN-------------------------------------------------

void AjusteVibracionMenukeyUP(void){return;}
void AjusteVibracionMenukeyRT(void){return;}	 
void AjusteVibracionMenukeyLF(void){return;}
void AjusteVibracionMenukeyDN(void){return;}

void AjusteVibracionMenukeyEN(void){
	if(AjustVibr->igxc4==DONE){ //==DONE){
         // pushMenuMonitor(AJUSTE_DE_VIBRACION);//push->OS menu we are in.
		 menu.bytes.semMenuPendiente=TRUE;//semaforo de Menu escape
         displayBoxDestructor(ACTIVADO);//destruir y parar la barra de deteccion.
         configModificado(AJUSTE_DE_VIBRACION);//debug se modifico este contexto, Aqui se ejecuta solo si cambio el angulo de vibracion 
         displayControladorPantalla(PORTAL_INICIO);//configModificacionController(AjusteVibracion.menuPadre,AjusteVibracion.menuNodo); 
//       MenuControl(); Ahora es controlado por Control_Operativo_de_Escape_de_Menus
    }//solo se activa el enter si ya termino de config. Ang. de Vib.
}//fin AjusteVibracionMenukeyEN------------------------

void AjusteProductoModekeyUP(void){
	if(cursory==POSY0)
	   return;
    if(cursory==POSY2){
		 VFDposicion(cursorx,cursory);
		 VFDserial_SendChar(' ');
		 cursorx=POSX_ESQ239;cursory=POSY0;
		 VFDposicion(cursorx,cursory);
		 VFDserial_SendChar('>');
		 VFDserial_SendChar('X');
		 return;}
	if(cursory==POSY4){
		 VFDposicion(cursorx,cursory);
		 VFDserial_SendChar(' ');
		 cursorx=0;cursory=POSY2;
		 VFDposicion(cursorx,cursory);
		 VFDserial_SendChar('>');
		 return;}
}//FIN AjusteProductoModekeyUP----------------------------

void AjusteProductoModekeyRT(void){return;}	 
void AjusteProductoModekeyLF(void){return;}

void AjusteProductoModekeyDN(void){
	if(cursory==POSY0){
		cursorx=POSX_ESQ239;cursory=POSY0;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');
		VFDserial_SendChar('x');
		cursorx=0;cursory=POSY2;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');
		return;}
	if(cursory==POSY2){
	   cursorx=0;cursory=POSY2;
	   VFDposicion(cursorx,cursory);
	   VFDserial_SendChar(' ');
	   cursorx=0;cursory=POSY4;
	   VFDposicion(cursorx,cursory);
	   VFDserial_SendChar('>');
	   return;}
	if(cursory>=POSY4)
		return;          
}//FIN AjusteProductoModekeyDN-------------------------------

void AjusteProductoModekeyEN(void){
  switch(cursory){
	case POSY0:switch(menu.contexto.Modificado){
			      case NUEVO_PRODUCTO:displayControladorPantalla(ESTAS_SEGURO2);break;
			      case AJUSTE_PRODUCTO_SEL:
			      default:break;}
               break;
    case POSY2://debug Queda pendievnte la AUTOMATICO
	           break;
    case POSY4://Ajuste Manual
    	switch(menu.contexto.Anterior){
    		case NUEVO_PRODUCTO:saveNewProduct(arg0);//guardamos el producto nombre y sus variables estandar con el iProduct=arg0
    							displayControladorPantalla(AJUSTES_AVANZADOS);//trae el iProduct desde Select Espacio Nuevo Product
                                break;
    		case AJUSTE_PRODUCTO_SEL: //displayControladorPantalla(AJUSTES_AVANZADOS
			                         //fin de ajuste producto seleccion menu
    			
    			                     
			                         break;
			default:break;}	                         
    default:break;}		  
}//FIN  AjusteProductoModekeyEN---------------------------------

void BorrarProductokeyUP(void){
	BorrarProductokey_UP(&displayBorrarProductoTitle);
}//FIN BorrarProductokeyUP--------------------------------

void BorrarProductokey_UP(void (*func)(void)){
	unsigned char i,j=0;	

			if(cursorx==POSXESQUINA)
				return;
			if(is_iProduct_1()){//esta la seleccion en iProduct TestSetUp
				cursory=POSY2;cursorx=POSXSELPRODCURSORX;
				VFDposicion(cursorx,cursory);
				VFDserial_SendChar(' ');
				cursory=POSY0;cursorx=POSXESQUINA; 
				VFDposicion(cursorx,cursory);
				VFDserial_SendChar('>');
				VFDserial_SendChar('X');
			    return;}
			i=BuscarSiguienteiProduct(PARRIBA);
			if(i==0)
				__asm(Halt);
			if(isSameScreeniProduct(PARRIBA,i)){//esta el iProduct encontrado en la misma pantalla
				VFDposicion(POSXSELPRODCURSORX,cursory);
				VFDserial_SendChar(' ');
				cursory=convertiProduct2POSY(i);//Convertir el iproduct de la pantalla actual to posY
				cursorx=POSXSELPRODCURSORX;
				VFDposicion(cursorx,cursory);
				VFDserial_SendChar('>');
				return;}
			else{keypad.Status.bits.enable=FALSE; 
			     if(i<7)
					 LimitesMenuPROD(1,SET);//nueva pantalla ponemos limite superior como i-3, proximo producto encontrado
			     else
			    	 LimitesMenuPROD(i,SET);//nueva pantalla ponemos limite superior como i-3, proximo producto encontrado
				 fillScreenNames();//rellenamos la nueva pantalla con productos y sus indices
				 func();//displayBorrarProductoTitle();//desplegamos titulo
				 VFDposicion(cursorx,cursory);
				 VFDserial_SendChar(' ');
				 cursorx=POSXSELPRODCURSORX;
				 cursory=convertiProduct2POSY(i);//ponemos curses en el iProduct encontrado
				 VFDposicion(cursorx,cursory);
				 VFDserial_SendChar('>');
				 //Timer_enable_keyboard_IRQ2();
				 return;}	
}//fin BorrarProductokey_UP------------------------------------------------------


void BorrarProductokeyRT(void){return;}	 
void BorrarProductokeyLF(void){return;}

void BorrarProductokeyDN(void){
	BorrarProductokeyDown(&displayBorrarProductoTitle);
}//fin BorrarProductokeyDN-------------------------------------


void BorrarProductokeyDown(void (*func)(void)){
unsigned char i,j=0;	
			if(iProductEmpty())//¿est vacio de productos? no hay nada que borrar
				return;
			if(LimitesMenuPROD(0,LEER)==CANTIDAD_PRODUCTOS)
						return;
			if(cursorx==POSXESQUINA){
				VFDposicion(POSXESQUINA,POSY0);
				VFDserial_SendChar(' ');  
				VFDserial_SendChar('x');
			    cursorx=POSXSELPRODCURSORX;cursory=POSY2;
			    VFDposicion(cursorx,cursory);
			    VFDserial_SendChar('>');
			    return;}
			i=BuscarSiguienteiProduct(PABAJO);
			if(i==0)//ya no hay nada mas para abajo
				return;
			if(isSameScreeniProduct(PABAJO,i)){//esta el iProduct encontrado en la misma pantalla
				VFDposicion(POSXSELPRODCURSORX,cursory);
				VFDserial_SendChar(' ');
				cursory=convertiProduct2POSY(i);//Convertir el iproduct de la pantalla actual to posY
				cursorx=POSXSELPRODCURSORX;
				VFDposicion(cursorx,cursory);
				VFDserial_SendChar('>');
			    return;}
			else{keypad.Status.bits.enable=FALSE; 
			     LimitesMenuPROD(i,SET);//nueva pantalla ponemos limite superior como i-3, proximo producto encontrado
			     fillScreenNames();//rellenamos la nueva pantalla con productos y sus indices
				 func();//displayBorrarProductoTitle();//desplegamos titulo
				 VFDposicion(cursorx,cursory);
				 VFDserial_SendChar(' ');
				 cursorx=POSXSELPRODCURSORX;
				 cursory=convertiProduct2POSY(i);//ponemos curses en el iProduct encontrado
				 VFDposicion(cursorx,cursory);
				 VFDserial_SendChar('>');
				 //Timer_enable_keyboard_IRQ2();
	             return;}	
}//fin BorrarProductokeyDown  ----------------------------------------


void BorrarProductokeyEN(void){
//unsigned char i;
	if(cursory==POSY0){//nos salimos no seleccionamos nada
	    displayControladorPantalla(AJUSTE_DE_PRODUCTO);
	    return;} 
		arg0=getiProducSelected(cursory);//get the product index selected
		if(arg0==1)//quieres borrar Test-Setup?
			return;//no hacer nada, no se puede
		if(arg0==indiceProducto){//quieres borrar el seleccionado actual?
			arg1=BUSY;//este cuate quiere borrar el producto Seeccionado Actual
			arg2=searchNextiProduct(PARRIBA,arg0);//buscamos siguiente producto para arriba
			indiceProducto=arg2;}
		else arg1=0;
		menu.contexto.Anterior=BORRAR_PRODUCTO;
		configModificado(BORRAR_PRODUCTO);//se modifico en el menu de BORRAR PRODUCTo
		displayControladorPantalla(configModificacionController(AJUSTE_DE_PRODUCTO,BORRAR_PRODUCTO));
}//fin BorrarProductokeyEN----------------------------------------------

void CopyProductokeyUP(void){
	BorrarProductokey_UP(&displayCopyProductoTitle);
}//FIN CopyProductokeyUP--------------------------------------------

void CopyProductokeyRT(void){return;}	 
void CopyProductokeyLF(void){return;}

void CopyProductokeyDN(void){
	BorrarProductokeyDown(&displayCopyProductoTitle);
}//FIN CopyProductokeyDN-------------------------------------------------

void CopyProductokeyEN(void){
	if(cursory==POSY0){//nos salimos no seleccionamos nada
		displayControladorPantalla(AJUSTE_DE_PRODUCTO);    
	    return;} 
    arg0=getiProducSelected(cursory);
    displayControladorPantalla(PASTE_PRODUCTO);
    
			
		
}//fin CopyProductokeyEN------------------------------------------

void AjusteProductoSelectkeyUP(void){
	BorrarProductokey_UP(&displayAjusteProductoSelectTitle);          
}//fin AjusteProductoSelectkeyUP------------------------------------

void AjusteProductoSelectkeyRT(void){return;}	 
void AjusteProductoSelectkeyLF(void){return;}

void AjusteProductoSelectkeyDN(void){
	BorrarProductokeyDown(&displayAjusteProductoSelectTitle);
}//FIN AjusteProductoSelectkeyDN------------------------------------



void AjusteProductoSelectkeyEN(void){
	if(cursory==POSY0){//nos salimos no seleccionamos nada
			displayControladorPantalla(AJUSTE_DE_PRODUCTO);//ir al Padre
			 return;} 
	    arg0=getiProducSelected(cursory);//get the product index selected
		//if(arg0==indiceProducto)//es diferente al actual el seleccionado
			//return;
		if(arg0>MAXIMO_NUMERO_PROD)
			__asm(Halt);//debug error de  diseño de software
		if(validarName(arg0)){//el seleccionado es vacio? es valido
			MemoEEPROM(CAMBIAR,menu.contexto.Modificado);//ejecuta cambiar de producto por arg0
			menu.contexto.Anterior=AJUSTE_DE_PRODUCTO;
			displayControladorPantalla(AJUSTE_DE_PRODUCTO_MODE);}
		else 
			__asm(Halt);//debug error de diseño de spftware
}//FIN AjusteProductoSelectkeyEN------------------------------


void NuevoProductokey_UP(void (*func)(void)){
	unsigned char aux1;	
		if(cursory==POSY0)   
		   if(cursorx==POSXESQUINA)
			    return;
		if(cursory==POSY4)
			if(LimitesMenuPROD(0,READ_LIM_SUP)==1){
		    	VFDposicion(POSXSELPRODCURSORX,cursory);
			    VFDserial_SendChar(' ');
			    cursorx=POSXESQUINA;
			    cursory=POSY0;
		    	VFDposicion(cursorx,cursory);
		    	VFDserial_SendChar('>');
		    	VFDserial_SendChar('X');
		    	return;}
			NewProd->igxc0=SearchSpace(UP_SCREEN);//busca lugar apartir del lugar iProduct=2 regresa el lugar disponible
			if(NewProd->igxc0==0){//No hay espacio disponible?
				displayNuevoProductoXY(POSY0,func);
				return;}
			else{//si hay espacio hacia arriba      
		     if(NewProd->igxc0<8){//esta el espacio en la primera pantalla?
		  	  if(LimitesMenuPROD(0,READ_LIM_SUP)==1){//estamos en la primera pantalla
			    	VFDposicion(POSXSELPRODCURSORX,cursory);
				    VFDserial_SendChar(' ');
				    cursorx=POSXSELPRODCURSORX;
				    cursory=getPosYNewProdDN(1,NewProd->igxc0);//obtenemos el cursor y del lugar encontrado nuevo disponible para poner el nuevo nombre
					VFDposicion(POSXSELPRODCURSORX,cursory);
				    VFDserial_SendChar('>');
					 return;}
		  	   else{ LimitesMenuPROD(1,ESCRIBIR);
	              	 cursory=getPosYNewProdDN(1,NewProd->igxc0);//superior,iProduct-selected
		  	         displayNuevoProductoXY(cursory,func); //vamos ala primer pantallla
		  	         return;}
		       	   	 return;}
			  else{//estamos en otra pantalla aparte de la primera
				  aux1=getiProducSelected(cursory);//en cual iProd estamos
				  if(LimitesMenuPROD(0,READ_LIM_SUP)>NewProd->igxc0){//el espacio esta en una pantalla arriba?
					   if(NewProd->igxc0<8){//vemos si tenemos que ir ala primera pantalla
						   LimitesMenuPROD(1,ESCRIBIR);
						   cursory=getPosYNewProdDN(1,NewProd->igxc0);
						   displayNuevoProductoXY(cursory,func); //vamos ala primer pantallla
					        return;}//ponemos el limite superior como 1
					   else{//Aque pantalla vamos si no es a la primera?
						   LimitesMenuPROD(NewProd->igxc0,ESCRIBIR);
						   cursory=getPosYNewProdDN(NewProd->igxc0,NewProd->igxc0);
						   displayNuevoProductoXY(cursory,func); //vamos ala primer pantallla
	                    return;}
					   return;}
					else{//es en la misma pantalla donde encontramos el espacio
	               VFDposicion(cursorx,cursory);//limpiaremos el cursor
	               VFDserial_SendChar(' ');//Limpiar cursor
	               cursory=getPosYNewProdDN(LimitesMenuPROD(0,READ_LIM_SUP),NewProd->igxc0);
	               VFDposicion(cursorx,cursory);
	               VFDserial_SendChar('>');// cursor
	               return;} 
					return;}     
			return;}	
	
}//fin NuevoProductokey_UP------------------------------

void NuevoProductokeyUP(void){
	NuevoProductokey_UP(&displayNuevoProductoTitle);
}//FIN NuevoProductokeyUP----------------------------------

void NuevoProductokeyRT(void){return;} 
void NuevoProductokeyLF(void){return;}

void NuevoProductokeyDN(void){
	NuevoProductokeyDown(&displayNuevoProductoTitle);
}//FIN  NuevoProductokeyDN---------------------------------------

//para crear un nuevo producto control de teclado
void NuevoProductokeyDown(void (*func)(void)){
unsigned char aux1;	
	if(cursory==POSY0){
	if(cursorx==POSXESQUINA){
	  textProc->igxc0=SearchSpace(INIT_NEW_PROD_MENU);//busca lugar apartir del lugar iProduct=2 regresa el lugar disponible
	  if(textProc->igxc0<CANTIDAD_PRODUCTOS){//hay espacio disponible
		VFDposicion(POSXESQUINA,cursory);
		VFDserial_SendChar(' ');
		VFDserial_SendChar('x');	 
		 if(textProc->igxc0<8){//hay espacio en los primeros siete, se ven en pantalla los prod del 1 al 7      
			 cursory=NewProd->igxc0*2;
			 VFDposicion(POSXSELPRODCURSORX,cursory);
			 VFDserial_SendChar('>');}
		 else{LimitesMenuPROD(textProc->igxc0,ESCRIBIR);//ponemos el limite superior el numero de producto donde vamos a escribir el nuevo
				 fillScreenNames();
				 func();//displayNuevoProductoTitle();
				 cursorx=POSXSELPRODCURSORX;cursory=POSY0;
				 VFDposicion(POSXSELPRODCURSORX,cursory);
				 VFDserial_SendChar('>');}}
			  return;}}  
	 aux1=getiProducSelected(cursory);//en cual iProd estamos
	 textProc->igxc0=SearchSpace(DOWN_SCREEN);//busca espacio del siguiente para abajo  	
	 if(textProc->igxc0<CANTIDAD_PRODUCTOS){//si hay espacio
		if(textProc->igxc0>LimitesMenuPROD(0,READ_LIM_INF)){//esta en otra pantalla
		   LimitesMenuPROD(textProc->igxc0,ESCRIBIR);//ponemos el sup= iProduct encontrado como limite sup
		   fillScreenNames();
		   func();//displayNuevoProductoTitle();
		   VFDposicion(POSXSELPRODCURSORX,cursory);
		   VFDserial_SendChar(' ');
		   cursory=POSY2;//INICIA el primer prod de esta pantalla
		   cursorx=POSXSELPRODCURSORX;
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar('>');}
			else{//estamos en la misma pantalla
			 VFDposicion(POSXSELPRODCURSORX,cursory);
		   VFDserial_SendChar(' ');
		   cursorx=POSXSELPRODCURSORX;
		   cursory=getPosYNewProdDN(LimitesMenuPROD(0,READ_LIM_SUP),NewProd->igxc0);//obtenemos el cursor y del lugar encontrado nuevo disponible para poner el nuevo nombre
		   VFDposicion(POSXSELPRODCURSORX,cursory);
		   VFDserial_SendChar('>');
			  return;} }
}//fin NuevoProductokeyDowN para crear un nuevo producto control de teclado---------------------



void NuevoProductokeyEN(void){
	if(cursory==POSY0){
		displayControladorPantalla(AJUSTE_DE_PRODUCTO);
	    return;}
   if(LimitesMenuPROD(cursory,READ_BUFF_PROD)!=NAME_INIT)
	   {menu.contexto.Anterior=NUEVO_PRODUCTO;//argumentos al nuevo hilo
      // configTextoProcessorController(NuevoProducto.menuNodo,NuevoProducto.menuNodo);	    
	   arg0=getiProducSelected(cursory);//en cual iProd estamos//numero de item de iProduct
	   displayControladorPantalla(TEXT_PROCESSOR);
	   return;}    
}//fin NuevoProductokeyEN---------------------------------------------------------------------


void PasteProductkeyUP(void){
	NuevoProductokey_UP(&displayPasteProductoTitle);
}//fin PasteProductkeyUP---------------------------

void PasteProductkeyRT(void){return;}	 
void PasteProductkeyLF(void){return;}

void PasteProductkeyDN(void){
	NuevoProductokeyDown(&displayPasteProductoTitle);
}//fin PasteProductkeyDN------------------------------

void PasteProductkeyEN(void){	
	 if((cursory==POSY0)||(cursorx==POSXESQUINA)){
		 arg0=0;//limpiamos que viene de copy, que es lo que queremos copiar
		 displayControladorPantalla(AJUSTE_DE_PRODUCTO);}
	 arg1=getiProducSelected(cursory);//en donde lo querem	os pegar     
	 modificado=PASTE_PRODUCTO;
	 displayControladorPantalla(configModificacionController(COPY_PRODUCTO,PASTE_PRODUCTO));
}//fin PasteProductkeyEN------------------------------- 
	



void AjusteTiempoRechazokeyRT(void){
	
}//fin AjusteTiempoRechazokeyRT----------------------------

void AjusteTiempoRechazokeyLF(void){
	
}//fin AjusteTiempoRechazokeyLF----------------------------

void AjusteTiempoRechazokeyUP(void){
const POSICION_X=POSXESQ235;
	switch(cursory){
		  case POSY0: break;
		  case POSY12:cursorx=0;
			          VFDposicion_DDS(cursorx,POSY12);
			          VFDserial_SendChar_DDS(' ');
			          VFDserial_SendChar_DDS(0x01);
			          cursory=POSY0;cursorx=POSXESQ235;
			          VFDposicion_DDS(cursorx,cursory);
			          VFDserial_SendChar_DDS('>');
			          VFDserial_SendChar_DDS('X');
			          VFDserial_SendChar_DDS(0x01);
					  VFDserial_SendChar_DDS(0x02);
			          break;
		  case POSY14:cursory=POSY14;cursorx=0;
			          VFDposicion_DDS(cursorx,cursory);
			          VFDserial_SendChar_DDS(' ');
			          VFDserial_SendChar_DDS(0x01);
			          cursory=POSY12;
			          VFDposicion_DDS(cursorx,cursory);
					  VFDserial_SendChar_DDS('>');
					  VFDserial_SendChar_DDS(0x01);
					  VFDserial_SendChar_DDS(0x02);
			          break;  
		  default:break;}  //fin switch--------------------------	
}//FIN AjusteTiempoRechazokeyUP------------------------------

void AjusteTiempoRechazokeyDN(void){
const POSICION_X=POSXESQ235;	
  switch(cursory){
	  case POSY0: VFDposicion_DDS(POSICION_X,POSY0);
	  	  	  	  VFDserial_SendChar_DDS(' ');
	  	  	  	  VFDserial_SendChar_DDS('x');
	  	  	      VFDserial_SendChar_DDS(0x01);
	  	  	      VFDserial_SendChar_DDS(0x02);
	  	  	  	  cursory=POSY12;cursorx=0;
	  	  	      VFDposicion_DDS(cursorx,cursory);
	  	  	      VFDserial_SendChar_DDS('>');
	  	  	      VFDserial_SendChar_DDS(0x01);
	  	  	  	  VFDserial_SendChar_DDS(0x02);
	              break;
	  case POSY12:cursorx=0;
		          VFDposicion_DDS(cursorx,POSY12);
		          VFDserial_SendChar_DDS(' ');
		          cursory=POSY14;
		          VFDposicion_DDS(cursorx,cursory);
		          VFDserial_SendChar_DDS('>');
		          VFDserial_SendChar_DDS(0x01);
		          VFDserial_SendChar_DDS(0x02);
		          break;
	  case POSY14:break;  
	  default:break;}  //fin switch--------------------------	
}//fin AjusteTiempoRechazokeyDN------------------------------

void AjusteTiempoRechazokeyEN(void){
	
   switch(cursory){
	   case POSY0:displayControladorPantalla(configModificacionController(AJUSTE_TIEMPO_RECHAZO,AJUSTES_AVANZADOS));
	              Deteccion.CuadroMadreReady=FALSE;//SE desactiva el depleigue por sistema opertivo. 
	              break;
	   case POSY12:  
		           break;
	   case POSY14:break;
	   default:break;}
	
}//fin AjusteTiempoRechazokeyEN------------------------------

void NombreProductoEditkeyUP(void){}
void NombreProductoEditkeyRT(void){}	 
void NombreProductoEditkeyLF(void){}
void NombreProductoEditkeyDN(void){}
void NombreProductoEditkeyEN(void){}							 


void EntradaTacokeyUP(void){
unsigned char key[]={POSY0,POSY2,POSY8,0};//posiiones donde uno se mueve	
	displayControlCursorSubMenusKey(PARRIBA,&cursorx,&cursory,POSXESQ235,&key[0],0);
}//fin EntradaTacokeyUP------------------------------------------

void EntradaTacokeyRT(void){}
void EntradaTacokeyLF(void){}

void EntradaTacokeyDN(void){
unsigned char key[]={POSY0,POSY2,POSY8,0};//posiiones donde uno se mueve	
	  displayControlCursorSubMenusKey(PABAJO,&cursorx,&cursory,POSXESQ235,&key[0],0);
}//fin EntradaTacokeyDN----------------------------------------

void EntradaTacokeyEN(void){
	switch(cursory){
	  case POSY0:displayControladorPantalla(CONFIGURAR_ENTRADAS_DE_SISTEMA);
	  	  	  	  cursorAnterior=POSY4;  break;
	  case POSY2:EntradasSistemaReg.taco=rotateSensor(EntradasSistemaReg.taco);
	             displaySensorActivation(EntradasSistemaReg.taco,POSY2);
	             configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);
	             break;
	  case POSY4:
	  case POSY6:	  
	  case POSY8:EntradasSistemaReg.taco=rotateSensorHighNibble(EntradasSistemaReg.taco);
	  	  	  	 VFDposicion(POSX_MENU_TACO,cursory);
				 VFDserial_SendChar(getHighNibbleSensor(EntradasSistemaReg.taco));
				 configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);
				 break;
	  case POSY10:break;
	  case POSY12:break;
	  default: break;
  }	//fin switch--------------------	
}//fin EntradaTacokeyEN-------------------------------------------

void EntradaTest__KeyUP(unsigned char *bloqueado,unsigned char *seg,unsigned char *mseg){
    EntradaTest__Key(bloqueado,seg,mseg,SUMAR,PARRIBA);
}//fin  EntradaTest__KeyUP-------------------------------------------------------------------


void EntradaTest__KeyDN(unsigned char *bloqueado,unsigned char *seg,unsigned char *mseg){
      EntradaTest__Key(bloqueado,seg,mseg,RESTAR,PABAJO);
}//fin EntradaTest__KeyDN---------------------------------------------------------- 


//void EntradaTest__Key(unsigned char *bloqueado,unsigned char *seg,unsigned char *mseg,unsigned char op,unsigned char dir){
unsigned char key[]={POSY0,POSY2,POSY4,POSY6,POSY8,0};	
//unsigned char pseg,pmseg;
//	  if(displayBloqueoDigitoControl(op,bloqueado,&igxm,POSX_MENU_TACO,POSY6))
//		   return;Q
//	  if(displayPruebaSistema_float(dir,seg,mseg,&igxm1,POSY8))
//		    return;
//	  displayControlCursorSubMenusKey(dir,&cursorx,&cursory,POSXESQ235,&key[0],0);

/*
 * Description	Resource	Path	Location	Type
Undefined : "contextoActual" Referenced from "EstaSeguro2keyEN" in	39_16_50_IN1UP		 	C/C++ Problem
 * 
 * 
 * */


void EntradaTestRechazokeyUP(void){
EntradaTest__KeyUP(&EntradasSistemaReg.rechazoBloqueado,&EntradasSistemaReg.rechazoPruebasist_seg,&EntradasSistemaReg.rechazoPruebasist_mseg);
}//FIN EntradaTestRechazokeyUP-------------------------------------------

void EntradaTestProductokeyUP(void){
EntradaTest__KeyUP(&EntradasSistemaReg.productoBloqueado,&EntradasSistemaReg.productoPruebasist_seg,&EntradasSistemaReg.productoPruebasist_mseg);
}//finEntradaTestProductokeyUP ----------------------------------------------------------------------------------------------------------


void EntradaTestRechazokeyRT(void){}//FIN EntradaTestRechazokeyRT--------------------------------------------
void EntradaTestRechazokeyLF(void){}
void EntradaTestProductokeyRT(void){}
void EntradaTestProductokeyLF(void){}


void EntradaTestRechazokeyDN(void){
EntradaTest__KeyDN(&EntradasSistemaReg.rechazoBloqueado,&EntradasSistemaReg.rechazoPruebasist_seg,&EntradasSistemaReg.rechazoPruebasist_mseg);
}//fin EntradaTestRechazokeyDN-------------------------------------------

void EntradaTestProductokeyDN(void){
//EntradaTest__KeyDN(&EntradasSistemaReg.productoBloqueado,&EntradasSistemaReg.productoPruebasist_seg,&EntradasSistemaReg.productoPruebasist_mseg);
}//fin EntradaTestProductokeyDN-----------------------------------------

void EntradaTestRechazokeyEN(void){
//EntradaTest_keyEN(&EntradasSistemaReg.rechazo,EntradasSistemaReg.rechazoBloqueado,EntradasSistemaReg.rechazoPruebasist_seg,EntradasSistemaReg.rechazoPruebasist_mseg);    	
cursorAnterior=POSY8;
}//fin EntradaTestRechazokeyEN------------------------------------------------------------------------------

void EntradaTestProductokeyEN(void){
EntradaTest_keyEN(&EntradasSistemaReg.producto,EntradasSistemaReg.productoBloqueado,EntradasSistemaReg.productoPruebasist_seg,EntradasSistemaReg.productoPruebasist_mseg);	
cursorAnterior=POSY10;
}//fin EntradaTestProductokeyEN-----------------------------------------------------------------------------

void EntradaTest_keyEN(unsigned char *sensor,unsigned char bloqueado,unsigned char seg,unsigned char mseg){
unsigned char sensor_valor,m;
  m=mseg=bloqueado=seg;
  sensor_valor=*sensor;
  switch(cursory){
			  case POSY0:displayControladorPantalla(CONFIGURAR_ENTRADAS_DE_SISTEMA);break;
			  case POSY2:sensor_valor=*sensor;
				         *sensor=rotateSensor(sensor_valor);
			  	  	  	 sensor_valor=*sensor;
			             displaySensorActivation(sensor_valor,POSY2);
			             configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);
			             break;
			  case POSY4:*sensor=rotateSensorHighNibble(sensor_valor);
			  	  	  	 sensor_valor=*sensor;
			  	  	     displayAltoSiBajoSi_Sensor(sensor_valor,cursory);
			  	  	     configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);
			  	  	     break;
//			  case POSY6:*sensor=rotateCursorBloquedo(sensor_valor,&igxm);
//			             displayrotateCursorBloquedo(bloqueado,POSX_MENU_TACO,POSY6,&igxm);
//				         break;
//			  case POSY8:if(++igxm1>3)//se activo el control de los digitos de prueba de sistemas
//				             igxm1=0;  //digito seleccion
//	                  	 display_Prueba_de_Sistema(seg,mseg,&igxm1,POSX_MENU_TACO,POSY8);
//						 break;
			  case POSY10:break;
			  case POSY12:break;
			  default: break;
		  }	//fin switch--------------------
}//fin EntradaTest_keyEN-----------------------------------------------------------



void EntradaFotoCelkeyUP(void){
//unsigned char key[]={POSY0,POSY2,POSY4,POSY6,0};//posiiones donde uno se mueve	
//	if(displayBloqueoDigitoControl(SUMAR,&EntradasSistemaReg.fotocelBloqueado,&igxm,POSX_MENU_TACO,POSY6))
//		return;
//    displayControlCursorSubMenusKey(PARRIBA,&cursorx,&cursory,POSXESQ235,&key[0],0);
}//FIN EntradaFotoCelkeyUP-----------------------------------------

void EntradaFotoCelkeyRT(void){}
void EntradaFotoCelkeyLF(void){}

void EntradaFotoCelkeyDN(void){
//unsigned char key[]={POSY0,POSY2,POSY4,POSY6,0};//posiiones donde uno se mueve	
//	if(displayBloqueoDigitoControl(RESTAR,&EntradasSistemaReg.fotocelBloqueado,&igxm,POSX_MENU_TACO,POSY6))//control de cmbio de digito de un item
//			  return;
//    displayControlCursorSubMenusKey(PABAJO,&cursorx,&cursory,POSXESQ235,&key[0],0);//control del cursor
}//FIN EntradaFotoCelkeyDN---------------------------------------


void EntradaFotoCelkeyEN(void){
	switch(cursory){
		  case POSY0:displayControladorPantalla(CONFIGURAR_ENTRADAS_DE_SISTEMA);
		  	  	  	 cursorAnterior=POSY6;break;
		  case POSY2:EntradasSistemaReg.fotocel=rotateSensor(EntradasSistemaReg.fotocel);
		             displaySensorActivation(EntradasSistemaReg.fotocel,POSY2);
		             configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);
		             break;
		  case POSY4:EntradasSistemaReg.fotocel=rotateSensorHighNibble(EntradasSistemaReg.fotocel);
		  	  	     displayAltoSiBajoSi_Sensor(EntradasSistemaReg.fotocel,cursory);
		  	  	     configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);
		  	  	     break;
//		  case POSY6:EntradasSistemaReg.fotocelBloqueado=rotateCursorBloquedo(EntradasSistemaReg.fotocelBloqueado,&igxm);
//		             displayrotateCursorBloquedo(EntradasSistemaReg.fotocelBloqueado,POSX_MENU_TACO,POSY6,&igxm);
//			         break;
		  case POSY8: 
					  break;
		  case POSY10:break;
		  case POSY12:break;
		  default: break;
	  }	//fin switch--------------------
}//fin EntradaFotoCelkeyEN----------------------------------------------------------


void EntradaAuxiliarkey(unsigned char dir,unsigned char *seg,unsigned char *mseg,unsigned char *digito,unsigned char posy){
	unsigned char key[]={POSY0,POSY2,POSY4,POSY6,POSY8,POSY10,POSY12,0};//posiiones donde uno se mueve	
		if(displayPruebaSistema_float(dir,seg,mseg,digito,posy))
			    return;
	    displayControlCursorSubMenusKey(dir,&cursorx,&cursory,POSXESQ235,&key[0],0);				
}//fin EntradaAuxiliarkey-------------------------------------------------------------------------------------------------------

void EntradaAuxiliarkeyUP(unsigned char *seg,unsigned char *mseg,unsigned char *digito,unsigned char posy){
	EntradaAuxiliarkey(PARRIBA,seg,mseg,digito,posy);
}//FIN EntradaAuxiliar1keyUP-------------------------------------------------------

void EntradaAuxiliar1keyUP(void){
//	EntradaAuxiliarkeyUP(&EntradasSistemaReg.aux1_seg,&EntradasSistemaReg.aux1_mseg,1,POSY10);
}//FIN EntradaAuxiliar1keyUP-------------------------------------------------------

void EntradaAuxiliar2keyUP(void){
//	EntradaAuxiliarkeyUP(&EntradasSistemaReg.aux2_seg,&EntradasSistemaReg.aux2_mseg,1,POSY10);
}//EntradaAuxiliar2keyUP------------------------------------------------------------

void EntradaAuxiliar1keyRT(void){}
void EntradaAuxiliar1keyLF(void){}
void EntradaAuxiliar2keyRT(void){}
void EntradaAuxiliar2keyLF(void){}


void EntradaAuxiliar1keyDN(void){
//    EntradaAuxiliarkeyDN(&EntradasSistemaReg.aux1_seg,&EntradasSistemaReg.aux1_mseg,&igxm1,POSY10);
}//fin EntradaAuxiliar1keyDN-------------------------------------------------------

/*
 * Description	Resource	Path	Location	Type
Undefined : "menu" Referenced from "run_Menu" in	39_16_50_IN1UP		 	C/C++ Problem
 * */

void EntradaAuxiliar2keyDN(void){
//    EntradaAuxiliarkeyDN(&EntradasSistemaReg.aux2_seg,&EntradasSistemaReg.aux2_mseg,&igxm1,POSY10);
}//fin EntradaAuxiliar2keyDN-------------------------------------------------------------------------


void EntradaAuxiliarkeyDN(unsigned char *seg,unsigned char *mseg,unsigned char *digito,unsigned char posy){
unsigned char key[]={POSY0,POSY2,POSY4,POSY6,POSY8,POSY10,POSY12,0};//posiiones donde uno se mueve	
	if(displayPruebaSistema_float(PABAJO,seg,mseg,digito,posy))
		    return;
    displayControlCursorSubMenusKey(PABAJO,&cursorx,&cursory,POSXESQ235,&key[0],0);	       		
}//fin EntradaAuxiliar1keyDN-------------------------------------------------------

void EntradaAuxiliar2keyEN(void){
  EntradaAuxiliarkeyEN(&EntradasSistemaReg.aux2,&EntradasSistemaReg.aux2Label,EntradasSistemaReg.aux2_seg,EntradasSistemaReg.aux2_mseg,&EntradasSistemaReg.aux2Rele);
}//fin EntradaAuxiliar2keyEN-------------------------------------------------------------------------------------------------------------


void EntradaAuxiliar1keyEN(void){
	EntradaAuxiliarkeyEN(&EntradasSistemaReg.aux1,&EntradasSistemaReg.aux1Label,EntradasSistemaReg.aux1_seg,EntradasSistemaReg.aux1_mseg,&EntradasSistemaReg.aux1Rele);
}// FIN EntradaAuxiliar1keyEN-----------------------------------------------------------------


void EntradaAuxiliarkeyEN(unsigned char *sensor,unsigned char *label,unsigned char seg, unsigned char mseg,unsigned char *rele){
unsigned char valor,m;	
    valor=*sensor;
    m=mseg=seg;
	switch(cursory){
			  case POSY0:displayControladorPantalla(CONFIGURAR_ENTRADAS_DE_SISTEMA);
			  	  	  	 cursorAnterior=POSY12;break;
			  case POSY2:*sensor=rotateSensor(valor);
						 valor=*sensor;
						 displaySensorActivation(valor,POSY2);
						 configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);
						 break;
			  case POSY4:*sensor=rotateSensorHighNibble(valor);
						 valor=*sensor;
						 displayAltoSiBajoSi_Sensor(valor,cursory);
						 configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);
						 break;
			  case POSY6:valor=*label;
				         *label=rotateLabelAux(valor);
				         valor=*label;
			             display_LabelAux(1,valor,POSY6);
				         break;
			  case POSY8:valor=*rele;
				         *rele=rotate_auxRele(valor);
				         valor=*rele;
			  	  	  	 display_auxRele(valor,POSY8);
						  break;
//			  case POSY10:if(++igxm1>3)//se activo el control de los digitos de prueba de sistemas
//		             	 	 igxm1=0;  //digito seleccion
//			  	  	  	  display_Prueba_de_Sistema(seg,mseg,&igxm1,POSX_MENU_TACO,POSY10);
//			  	  	  	  break;
//			  case POSY12:break;
			  default: break;
		  }	//fin switch--------------------
}//fin EntradaAuxiliarkeyEN---------------------------------------------------------







void EstaSeguro2keyUP(void){return;}

void EstaSeguro2keyRT(void){
	if(cursorx==POSX0){//PRIMERA POSICION DESPUES DE LA X
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');VFDserial_SendChar('n');VFDserial_SendChar('o');
		cursorx=POSXSI;cursory=POSY10;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');VFDserial_SendChar('S');VFDserial_SendChar('I');
		return;}//posicion primera
	else{cursorx=POSXSI;cursory=POSY10;
		 VFDposicion(cursorx,cursory);
		 VFDserial_SendChar(' ');VFDserial_SendChar('s');VFDserial_SendChar('i');
		cursorx=POSX0;cursory=POSY10; 
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');VFDserial_SendChar('N');VFDserial_SendChar('O');  }
}//fin EstaSeguro2keyRT---------------------------------------

void EstaSeguro2keyLF(void){
	if(cursorx==POSX0){//PRIMERA POSICION DESPUES DE LA X
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar(' ');VFDserial_SendChar('n');VFDserial_SendChar('o');
		cursorx=POSXSI;cursory=POSY10;
		VFDposicion(cursorx,cursory);
		VFDserial_SendChar('>');VFDserial_SendChar('S');VFDserial_SendChar('I');
		return;}//posicion primera
	else{cursorx=POSXSI;cursory=POSY10;
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar(' ');VFDserial_SendChar('s');VFDserial_SendChar('i');
		   cursorx=POSX0;cursory=POSY10; 
		   VFDposicion(cursorx,cursory);
		   VFDserial_SendChar('>');VFDserial_SendChar('N');VFDserial_SendChar('O');  }
 }//fin EstaSeguro2keyLF-------------------------------------

void EstaSeguro2keyDN(void){return;}

void EstaSeguro2keyEN(void){
unsigned char i;	
 switch(menu.contexto.Modificado){
    case NUEVO_PRODUCTO:
			if(cursorx==POSX0){//No queremos guardar nada
				MemoEEPROM(RESTAURAR,menu.contexto.Modificado);}//enter en NO
			if(cursorx==POSXSI){
				MemoEEPROM(SAVE,menu.contexto.Modificado);
				indiceProducto=arg0;
				menu.contexto.Actual=NUEVO_PRODUCTO;
				displayControladorPantalla(PORTAL_INICIO);}
			break;//  NUEVO_PRODUCTO  
    case BORRAR_PRODUCTO:
    	   if(cursorx==POSX0){
    		   displayControladorPantalla(AJUSTE_DE_PRODUCTO);
    	       break;}
    	   if(cursorx==POSXSI){
    		   writeEEPROMbyte2(PROD_VAR0_ADD+(PROD_VAR_SIZE*(arg0-1)),0);//borrar INIT PROD del producto seleccionado con var arg0
               if(arg1==BUSY){//se borro el seleccionado?
                    cargarNuevoProducto(arg2,BORRAR);
                    displayControladorPantalla(PORTAL_INICIO);}
               else displayControladorPantalla(AJUSTE_DE_PRODUCTO);
               break;}
    case PASTE_PRODUCTO:
    		if(cursorx==POSX0){
    			arg0=0;arg1=0;arg2=0;
    			displayControladorPantalla(AJUSTE_DE_PRODUCTO);
    		    break;}
    		if(cursorx==POSXSI){
    			Guardar_Var_Prod_a_Cache(&name[0]);//guardar variables de producto en cache
    			Copy_Cache_to_SpaceProduct(indiceProducto);//pasar el cache  a su lugar de memoria
    			Cargar_SpaceiProduct_to_Cache(arg0);//cargar el nuevo producto Seleccionado Copiado al cache
    			Descargar_Cache_to_Var_Product();//pasar el cache a las variables de producto
    			Copy_Cache_to_SpaceProduct(arg1);//pasar el cache al espacio seleccionado de PASTE - producto
    			indiceProducto=arg1;//cambiar el index-product al producto paste seleccionado
    			displayControladorPantalla(AJUSTE_DE_PRODUCTO);//cambiar el contexto al ajuste de producto
    			break;}
    		break;
    case SELECCIONAR_PRODUCTO:
    		if(cursorx==POSX0){
				arg0=0;arg1=0;arg2=0;
				displayControladorPantalla(AJUSTE_DE_PRODUCTO);
				break;}
    		if(cursorx==POSXSI){//aceptamos
    			 MemoEEPROM(CAMBIAR,menu.contexto.Modificado);//ejecuta cambiar de producto por arg0
    			 indiceProducto=arg0;
				 displayControladorPantalla(PORTAL_INICIO);
    			 break;}
    		break;
    case AJUSTE_PARAMETRICO_DE_PRODUCTO:
    	    if(cursorx==POSX0){
    	    	Deteccion.Sensibilidad=arg3;//guardamos valores por si se modifican
    	        phasefrac=arg0;//guardamos valores por si se modifican
    	        phase=arg1;	
    	        if(arg2==RESET)
    	        	displayControladorPantalla(AJUSTE_PARAMETRICO_DE_PRODUCTO);
    	        else displayControladorPantalla(PORTAL_INICIO);
    	        break;}
    		if(cursorx==POSXSI){//aceptamos
   			    MemoEEPROM(SAVE,menu.contexto.Modificado);//ejecuta cambiar de producto por arg0
   			    if(arg2==RESET)
   			    	 displayControladorPantalla(AJUSTE_PARAMETRICO_DE_PRODUCTO);
   			    else displayControladorPantalla(PORTAL_INICIO);
    		    break;}    	    
    	    break;
    case AJUSTES_AVANZADOS:
     	   if(cursorx==POSX0){
      	        phasefrac=arg0;//guardamos valores por si se modifican
				phase=arg1;
				Deteccion.Sensibilidad=arg3;//guardamos valores por si se modifican
				if(arg2!=0)//se usa para saber si se quieren borrar los contadores
					 __asm(Halt);//error porque debe estr en cero
				Altura=arg7;// para guardar altura
				MarcarAltura=arg8;//para guardar Marcar altura
			    Ganancia=arg9; //para guardar ganancia
				frecuenciaSeleccion=arg10;// para guardar frecq.
				for(i=0;i<NOMBRE_PRODUCTO_SIZE;i++) 
					name[i]=Text[i];
				displayControladorPantalla(AJUSTE_PARAMETRICO_DE_PRODUCTO);
     	       break;}
     	   if(cursorx==POSXSI){
     		   MemoEEPROM(SAVE,menu.contexto.Modificado);
     		   displayControladorPantalla(AJUSTE_PARAMETRICO_DE_PRODUCTO);
     	       break;}
     	   
    	   break;
    		
    default:break;  	   
	     }//switch contextoModificado 
}//fin EstaSeguro2keyEN--------------------------------------------
	    		

/*
 * 
 * Description	Resource	Path	Location	Type
Undefined : "menu" Referenced from "AjusteParamProdkeyEN" in	39_16_50_IN1UP		 	C/C++ Problem
 * 
 * 
 * 
 * */

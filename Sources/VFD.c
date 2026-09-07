/*
 * VFD.c
 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */


/*
 * VFD.c
 *
 *  Created on: Jul 13, 2015
 *      Author: chals
 */



#include "VFD.h"
#include "VFDserial.h"
//#include "TI1_delays.h" //timer for delays
#include "VFDmenu.h"
#include "delay.h"
#include "system.h"
#include "queue.h"
#include "DSP.h"


typedef union{//access word: 
	unsigned  short int wordx;   //   	0xaabb   
	unsigned char byteH;      //         aa
	unsigned char byte[2];        //byte[0]=aa,byte[1]=bb
}uword1;

extern struct _GLOBAL_CONTEXTO g,*AjustVibr,*DDSpant,*AjParamProd,*ParamDet;
extern struct _KeyPad_ keypad;

extern unsigned int delaycount;
unsigned char keyboard_IRQ,keyboard_IRQ2;
unsigned char timerEnableKeypad;//PARA activar el teclado despues de desactivarlo y esperar que se acabe de llegar todos los caracteres al display y esperar, porque tarda en desplegar y le estan apriete y aprite al teclado
extern void monitorActividad_IRQ(void);
extern struct _Menu_  menu;//contexto Actual que se despliega


void init_VFD(void){
	   // delay_ms(200);//<--prueba para quitar error de Fnt size
	   // TI1_delays_Disable();
	    //TI1_delays_DisableEvent();
	    
	    VFDcommand(0x1BU); // init display  ESC@= 1BH,40H
        VFDcommand(0x40U); // Setting return to default values
	//*************************************************************
		VFDcommand(0x1FU);//CURSOR display, default=off
		VFDcommand(0x43U);  // comando  1Fh,43h,n,  n=1->display cursor ON
		VFDcommand(0x01U);// n=01h,-> O

		VFDcommand(0x1FU);//setup IO port0 as input, pin1:bit7, pin7:bit1, pin8:bit0
		VFDcommand(0x28U);//comando 1fh,28h,70h,01h,n(00h:port0|01h:Port1),a(bit:0->INPUT|1:->OUTPUT)
		VFDcommand(0x70U);//
		VFDcommand(0x01U);
		VFDcommand(0x00U);//n=port0 -> 00h
		VFDcommand(0x00U);// port0 as input so: 0000 0000
	    
		VFDcommand(0x1FU);//setup IO port1 as OUTPUT, pin1:bit7, pin7:bit1, pin8:bit0
		VFDcommand(0x28U);//comando 1fh,28h,70h,01h,n(00h:port0|01h:Port1),a(bit:0->INPUT|1:->OUTPUT)
		VFDcommand(0x70U);//
		VFDcommand(0x01U);//
		VFDcommand(0x01U);//n=port1 -> 01h, sera puerto de salida
		VFDcommand(0xFFU);// port1 as OUTPUT so: 1111 1111

		VFDcommand(0x1FU);//Datos a SACAR por el PUERTO, commando  1Fh,28h,70h,n,a
		VFDcommand(0x28U);//n:IO port, 00h para port0
		VFDcommand(0x70U);//n:IO port, 01h para port1
		VFDcommand(0x10U);//
		VFDcommand(0x01U);//n=puerto1, PUERTO DE SALIDA, port1
		VFDcommand(0x03U);//salidas en puerto1->seran: 0011

		FontSizeVFD(FONTSIZE2);//TAMAÑO DE fuente 2->8x16

		VFDcommand(0x1FU); //comando de FONT BOLD
	    VFDcommand(0x28U); // COMANDO 1fh,28h,67h,41,m
		VFDcommand(0x67U);//m=00h, fixed width,, m=02h-> proportional1
	    VFDcommand(0x41U);//m=03->proportional-2  m=03h->proportional-3
		VFDcommand(0x00U);//m bold=01H

		
		/*VFDcommand(0x1FU);// font size select of a character command
		VFDcommand(0x28U);//  comando 1fh,28h,67h,01h,m
		VFDcommand(0x67U);//m= 01h  6x8 font,   m=02h->8x16 font
		VFDcommand(0x01U);//m=03h-> 12x24   m=04h->16x32 font
		VFDcommand(0x02U);*/

		/*VFDcommand(0x1FU); //comando de FONT WIDTH
		VFDcommand(0x28U); // COMANDO 1fh,28h,67h,04h,m
		VFDcommand(0x67U);//m=00h, fixed width,, m=02h-> proportional1
		VFDcommand(0x04U);//m=03->proportional-2  m=03h->proportional-3
		VFDcommand(0x00U);*/

	
		keyboard_IRQ=TRUE;
}//fin init VFD -------------------------------------------------------------------

void init_VFD_BIOS(void){
	   // delay_ms(200);//<--prueba para quitar error de Fnt size
	   // TI1_delays_Disable();
	    //TI1_delays_DisableEvent();
	    VFDcommand(0x1BU); // init display  ESC@= 1BH,40H (Initialize Display)
        VFDcommand(0x40U); // Setting return to default values(Initialize Display)
	//******************************CUrsosr display *****************************
		VFDcommand(0x1FU);//CURSOR display, default=off
		VFDcommand(0x43U);  // comando  1Fh,43h,n,  n=1->display cursor ON
		VFDcommand(0x01U);// n=01h,-> O
    

		FontSizeVFD(FONTSIZE1);//TAMAÑO DE fuente 2->8x16
		/*VFDcommand(0x1FU);// font size select of a character command
		VFDcommand(0x28U);//  comando 1fh,28h,67h,01h,m
		VFDcommand(0x67U);//m= 01h  6x8 font,   m=02h->8x16 font
		VFDcommand(0x01U);//m=03h-> 12x24   m=04h->16x32 font
		VFDcommand(0x02U);*/

		/*VFDcommand(0x1FU); //comando de FONT WIDTH
		VFDcommand(0x28U); // COMANDO 1fh,28h,67h,04h,m
		VFDcommand(0x67U);//m=00h, fixed width,, m=02h-> proportional1
		VFDcommand(0x04U);//m=03->proportional-2  m=03h->proportional-3
		VFDcommand(0x00U);*/

		/*VFDcommand(0x1FU); //comando de FONT BOLD
	    VFDcommand(0x28U); // COMANDO 1fh,28h,67h,41,m
		VFDcommand(0x67U);//m=00h, fixed width,, m=02h-> proportional1
	    VFDcommand(0x41U);//m=03->proportional-2  m=03h->proportional-3
		VFDcommand(0x00U);//m bold=01H*/
	
}//fin init VFD -------------------------------------------------------------------





void FontSizeVFD(unsigned char m){
// pusimos estos delay y el tamaño de la Font no obedecia
	//y se ponia chica
//con estos delay ya obedece pero no sabemos si todos o cual
	VFDcommand(0x1FU);// font size select of a character command
	 delay_us(10);//<--pusimos estos delay
	VFDcommand(0x28U);//  comando 1fh,28h,67h,01h,m
	 delay_us(10);//<--pusimos estos delay
	VFDcommand(0x67U);//m= 01h  6x8 font,   m=02h->8x16 font
	 delay_us(10);//<--pusimos estos delay
	VFDcommand(0x01U);//m=03h-> 12x24   m=04h->16x32 font
	 delay_us(10);//<--pusimos estos delay
	VFDcommand(m);

}// fin font size for the VFD----------------------------------------------------


void VFDcommand(unsigned char cmd){
	
	//TI1_delays_Init();
	//delay1us();
	//con 1 ms se le quito el error de FontSize
	//con 100us tiene error de FontSize
	//con 500us se le quito el error de FontSize
	//con 250us se le quito el error de FontSize
	//Se le quito el error con 125us lo dejamos en 125us
	
	delay_us(225);//<--prueba para quitar error de Fnt size
	VFDserial_SendChar(cmd);
	delay_us(225);
	//TI1_delays_DisableEvent();
	//TI1_delays_Disable();
	
}//fin vfd command---------------------------------------------------------------


void VFDclrscr(void){
	monitorDDS_Halt();//debug, quitar un dia que la version este super probada
	VFDserial_SendChar(0x0CU);//(Display Clear)   Display screen is cleared and cursor moves to home position.
    delay_us(100);//con 50 sigue sin borrar el anterior
    delay_us(100);//con 50 sigue sin borrar el anterior
    delay_us(100);//con 50 sigue sin borrar el anterior
    delay_us(100);//con 50 sigue sin borrar el anterior
    delay_us(100);//con 50 sigue sin borrar el anterior
    delay_us(100);//con 50 sigue sin borrar el anterior
	delay_us(100);//con 50 sigue sin borrar el anterior
	delay_us(100);//con 50 sigue sin borrar el anterior
	delay_us(100);//con 50 sigue sin borrar el anterior
	delay_us(100);//con 50 sigue sin borrar el anterior
	delay_us(100);//con 50 sigue sin borrar el anterior
}//fin clear screen VFD-------------------------------------------------------------

void VFDclrscr_DDS(void){
//	FIFO_DDS_push(0x0CU,10);  // VFDserial_SendChar(0x0CU);//(Display Clear)   Display screen is cleared and cursor moves to home position.
}//fin clear screen VFD-------------------------------------------------------------


//No se ejecuta bien el comando de limpiar cuando hay barra de deteccion. Description	Resource	Path	Location	Type

void VFDclrscrForce(void){
unsigned short int i;	
   
	VFDposicion(0,0);
	for(i=0;i<512;i++)
		VFDserial_SendChar(' ');
}//limpiar pantalla a la fuerza-------------------------


//para evitar Usar estas funciones cuando estamos en contexto de DDS
//DDSpant->igxc0 esta variable desabilita este monitor, solo para usarse en el display maestro inicial
void monitorDDS_Halt(void){
//unsigned char c;
     //c=AjustVibr->igxc0;//DDSpant->igxc0;
//	 if(menu.contexto.Actual==PANTALLA_DDS)
//		 if(c==0xAA)
//			  return;
//		 else
//	       __asm(Halt);//Debug error de diseño de software, no se debe usar esta funcion
}//fin monitorDDS-------------------------------------------------------------



/*   posicion de x es de 0000h a 1ffh  ó  a 511 
  *    posicion de y es de 0000h a 0Fh  * */
void VFDposicion(unsigned short int x,unsigned short int y){ //MANDA DEL COMANDO DE POSICION AL vfd
uword1 v; 
	// unsigned char  byteL=(unsigned char)x;//toma el lowbyte
	 //unsigned char  byteH=(unsigned char)(x<<)
     monitorDDS_Halt();//debug, quitar un dia que la version este super probada
	 v.wordx=x;
	 VFDserial_SendChar(0x1FU);//comando de posicion 
	 VFDserial_SendChar(0x24U); //comando 1fh,24h,xL,xH,yL,yH
	 VFDserial_SendChar(v.byte[LO]);//v.byte[LO]);
	 VFDserial_SendChar(v.byte[HI]);//v.byteH);
	 v.wordx=y;
	 VFDserial_SendChar(v.byte[LO]);
	 VFDserial_SendChar(v.byte[HI]);
     delay_us(100);//con 50 sigue sin borrar el anterior
     delay_us(100);//con 50 sigue sin borrar el anterior
     delay_us(100);//con 50 sigue sin borrar el anterior
     delay_us(100);//con 50 sigue sin borrar el anterior
     delay_us(100);//con 50 sigue sin borrar el anterior
     delay_us(100);//con 50 sigue sin borrar el anterior
     delay_us(100);//con 50 sigue sin borrar el anterior
     delay_us(100);//con 50 sigue sin borrar el anterior
 }// fin posicionVFD-------------------------------------------------------------

void BarraDet_VFDposicion(unsigned short int x,unsigned short int y){
uword1 v;	
	 v.wordx=x; 
	 FIFO_barraNum_push(COMANDO_POS);
	 FIFO_barraNum_push(v.byte[LO]);
	 v.wordx=y;
	 FIFO_barraNum_push(v.byte[LO]);
}//FIN ------------------------------------------------------------------------


void BarraDet_VFDserial_SendChar(unsigned char c){
	 FIFO_barraNum_push(COMANDO_CHAR);
	 FIFO_barraNum_push(c);
}//fin BarraDet_VFDserial_SendChar--------------------------------------------------

void VFDserial_SendChar_DDS(unsigned char c){
	FIFO_Display_DDS_Char_push(c,0xFE);//0xFE means that is just a char display          
}//fin VFDserial_SendChar_DDS---------------------------------


void VFDposicion_DDS(unsigned short int x,unsigned short int y){ //MANDA DEL COMANDO DE POSICION AL vfd
	FIFO_Display_DDS_Char_push((unsigned char)x,(unsigned char)y);          
}// fin posicionVFD-------------------------------------------------------------


/* comando para mandar en el dds MENU si queremos las letras en bold  o no
 * bold: TRUE|FALSE*/
void VFDcommand_Bold_DDS(unsigned char bold){
	FIFO_Display_DDS_Char_push(bold,0xFD);//FDh means that we'll send a command of kind bold
}//fin VFDcomando DDS-------------------------------------


void VFDkeypad_ISR(unsigned char c){
	monitorActividad_IRQ();   
	if(keyboard_IRQ){ 	  //0.0005sec*300=0.15secs
	  switch(menu.contexto.Actual){//2000 means 1000miliseconds
	    case PORTAL_INICIO:teclado_Control(c,20,500); break;//tiempos de enter y no enter
	    case MENU_INSIGHT:teclado_Control(c,100,50);break;	    
	    default:teclado_Control(c,100,100);break;   
		}}// fin switch
   
}// fin VFD keypad Interrupt Service Routine++++++++++++++++++++++++++++++++++++++++

/*c: es el char que ha sido preionado en el teclado
 * Entime: es el timepo para darle delay al  tecla enter
 * Keytime: es el delay y o constantes para manejar el tiempo de otras keys*/
void teclado_Control(unsigned char c,unsigned short int EnTime,unsigned short int Keytime){
  switch(c){	
	case 'A':menuKey(keyUP,Keytime,c);break;//up-key -pressed
    case 'O':menuKey(keyDN,Keytime,c);break;//down key-pressed
    case 'L':menuKey(keyLF,Keytime,c);break;//left key-pressed
    case 'R':menuKey(keyRT,Keytime,c);break;//right-keypressed
    case 'E':menuKey(keyEN,EnTime ,c);break;
	case 'S': //suprimir
	case 'e': //escape
	case 'a':keypad.var.c0=c;
		     break;
	default:break;}
  
}//fin telacado_controller--------------------------------------


/*f: apunta a funcion menu
 * Entime: es el timepo para darle delay al  tecla enter
 * Keytime: es el day o constantes para manejar el */
void menuKey(unsigned char key,unsigned short int Et,unsigned char c){
//static unsigned char n;
	if(keypad.var.timerkeypad++>Et){
		keypad.var.c0=0;
		keypad.var.timerkeypad=0;
		keypad.Status.bits.enable=1;}
	if(keypad.var.c0!=c){
       //if(keypad.Status.bits.enable){
    	 //  keypad.Status.bits.enable=0;
           menus(key);}
  keypad.var.c0=c;
}//fin de menukey--------------------------------------------------


//void disable_keyboard_IRQ(void){ keyboard_IRQ=FALSE; }//fin keyboardIRQ
//void enable_keyboard_IRQ(void){keyboard_IRQ=TRUE;}
//void Timer_enable_keyboard_IRQ(void){ timerEnableKeypad=0xAA;}

//void disable_keyboard_IRQ2(void){ keyboard_IRQ=FALSE; }//fin keyboardIRQ
//void enable_keyboard_IRQ2(void){keyboard_IRQ=TRUE;}
//void Timer_enable_keyboard_IRQ2(void){ timerEnableKeypad=0xAA;}




void VFDdrawLine(unsigned char pen,unsigned short int x1,unsigned short y1,unsigned short int x2,unsigned short y2){
coordn16 coordenadas;
unsigned short int delayhere=10;     
	    
        /*if(y1>255) y1=255;  //seguro por si hay un bug de rango
          if(y2>255) y2=255; //seguro por si hay un bug de rango codigo mete error no dibuja la linea
          if(x1>127) x1=127; //seguro por si hay un bug de rango segun coordenadas 
          if(x2>127) x2=127; //seguro por si hay un bug de rango*/
        VFDserial_SendChar(0x1FU);delay_us(delayhere);
        VFDserial_SendChar(0x28U);delay_us(delayhere);
        VFDserial_SendChar(0x64U);delay_us(delayhere);
        VFDserial_SendChar(0x11U);delay_us(delayhere);
        VFDserial_SendChar(0x00U);delay_us(delayhere);//comando para dibujar una linea
        VFDserial_SendChar(pen);delay_us(delayhere);// comando pen
        coordenadas.coord16=x1;
        VFDserial_SendChar(coordenadas.byte[LO]);delay_us(delayhere);
        VFDserial_SendChar(coordenadas.byte[HI]);delay_us(delayhere);
        coordenadas.coord16=y1;
        VFDserial_SendChar(coordenadas.byte[LO]);delay_us(delayhere);
        VFDserial_SendChar(coordenadas.byte[HI]);delay_us(delayhere);
        coordenadas.coord16=x2;
        VFDserial_SendChar(coordenadas.byte[LO]);delay_us(delayhere);
        VFDserial_SendChar(coordenadas.byte[HI]);delay_us(delayhere);
        coordenadas.coord16=y2;
        VFDserial_SendChar(coordenadas.byte[LO]);delay_us(delayhere);
        VFDserial_SendChar(coordenadas.byte[HI]);delay_us(delayhere);
        
}// fin draw line -----------------------------------------------------------------------------------------
     

void VFDdotDrawing(unsigned short int x,unsigned short int y){
coordn16 coordenadas;
unsigned short int n=40;  //<<--NO DISMINUIR Menos de 40 Nos da basura en Pantalla
unsigned short int b=30;  //<<--NO DISMINUR PORQUE DESPLIEGA BASURA

        //320usegundos de delays
        VFDserial_SendChar(0x1FU);
        delay_us(n);//<<----NO MOVER O DESPLIEGA BASURA EN PANTALLA
        VFDserial_SendChar(0x28U);
        delay_us(n);//<<----NO MOVER O DESPLIEGA BASURA EN PANTALLA
        VFDserial_SendChar(0x64U);
        delay_us(n);//<<----NO MOVER O DESPLIEGA BASURA EN PANTALLA
        VFDserial_SendChar(0x10U);
        delay_us(n);//<<----NO MOVER O DESPLIEGA BASURA EN PANTALLA
        VFDserial_SendChar(0x01U);
        delay_us(n);//<<----NO MOVER O DESPLIEGA BASURA EN PANTALLA
        coordenadas.coord16=x;
        VFDserial_SendChar(coordenadas.byte[LO]);
        delay_us(b);//<<----NO MOVER O DESPLIEGA BASURA EN PANTALLA
        VFDserial_SendChar(coordenadas.byte[HI]);
        delay_us(b);//<<----NO MOVER O DESPLIEGA BASURA EN PANTALLA
        coordenadas.coord16=y;
        VFDserial_SendChar(coordenadas.byte[LO]);
        delay_us(b);//<<----NO MOVER O DESPLIEGA BASURA EN PANTALLA
        VFDserial_SendChar(coordenadas.byte[HI]);
        delay_us(30);//<<----NO DISMINUR O DESPLIEGA BASURA EN PANTALLA
        
}// fin de VFD dot drawing--------------------------------------------------------------------------------


void VFDboxLine(unsigned char pen,unsigned char mode,unsigned short int x1,unsigned short y1,unsigned short int x2,unsigned short y2){
coordn16 coordenadas;
unsigned short int delayhere=10;     
	    
        VFDserial_SendChar(0x1FU);delay_us(delayhere);
        VFDserial_SendChar(0x28U);delay_us(delayhere);
        VFDserial_SendChar(0x64U);delay_us(delayhere);
        VFDserial_SendChar(0x11U);delay_us(delayhere);
        if(mode!=BOX_VACIA){
        	if(mode!=BOX_LLENA){
        	      mode=BOX_VACIA;}}
        VFDserial_SendChar(mode);delay_us(delayhere);//comando para dibujar una linea
        if(pen!=0x01){
        	if(pen!=0x00){
        		pen=0x01;}}
        VFDserial_SendChar(pen);delay_us(delayhere);// comando pen
        coordenadas.coord16=x1;
        VFDserial_SendChar(coordenadas.byte[LO]);delay_us(delayhere);
        VFDserial_SendChar(coordenadas.byte[HI]);delay_us(delayhere);
        coordenadas.coord16=y1;
        VFDserial_SendChar(coordenadas.byte[LO]);delay_us(delayhere);
        VFDserial_SendChar(coordenadas.byte[HI]);delay_us(delayhere);
        coordenadas.coord16=x2;
        VFDserial_SendChar(coordenadas.byte[LO]);delay_us(delayhere);
        VFDserial_SendChar(coordenadas.byte[HI]);delay_us(delayhere);
        coordenadas.coord16=y2;
        VFDserial_SendChar(coordenadas.byte[LO]);delay_us(delayhere);
        VFDserial_SendChar(coordenadas.byte[HI]);delay_us(delayhere);
        
}// fin draw line -----------------------------------------------------------------------------------------









/*
 * queue.c
 *
 *  Created on: Oct 3, 2017
 *      Author: chals
 */


#include "queue.h"
#include "system.h"
#include "DSP.h"
#include "IOcomms.h"
//#include "PE_Types.h" ya esta en h, revisar y borrar it
#include "system.h"
#include "strings.h"
#include "VFD.h"
#include "maths.h"
#include "Memoria.h"
#include "errorController.h"
#include "VFDmenu.h"
#include "I2C_.h"





typedef union{//access word: 
	unsigned  short int wordx;   //   	0xaabb   
	unsigned char byteH;      //         aa
	unsigned char byte[2];        //byte[0]=aa,byte[1]=bb
}uword3;

extern struct _Menu_ menu;
const unsigned char SIZE_FIFO_DDS_pixel=35;//tamaño del FIFO DDS pixels a ser pintados en DDS menu
//extern unsigned char EventDDS;
//struct NodoBox FIFObox[SIZE_FIFO_BOX],
  //extern struct _DDS_ DDS1;	
//extern struct COMMUNICACION IO_TX;
struct  _SERIAL_ serial;  //struct _Vector_ serial;
struct _Comando_DDS dds;
struct _FIFO_1byte_ serialKeyPad;
struct _IIC_ i2c;
//const unsigned char SIZE_FIFO_OFFSET=32;//numero de taps del filtro digital del offset
signed short int mem1[SIZE_MEM];//variable por contexto
//extern signed short int memy[SIZE_MEM];//variable por contexto
signed short int nmem;//size mem llenado nmero

extern struct _Error323_ sysMon;

extern struct _Comando_DDS dds;
extern  struct _I2C_CONTROL IIC;
struct _COMMs_ com; //comunicaciones
struct _Recursos recurso;
struct _Signale_ Signale;


static void init_FIFO_volatile_int16_v1(volatile struct _FIFO_volatile_int16_v1 *s,volatile int16 *buff,uint8 size);
static void init_FIFO_General_int16_v1(struct _FIFO_int16_v1 *s,int16 *buff,uint8 size);

static void reset_FIFO_general_int16_v1(struct _FIFO_int16_v1 *s,uint8 size);
static uint8 busqueda3(int16 *arr,uint8 size,int16 dato);
static int16 *shift_left_int16(int16 *arr,uint8 size);
static int16 FIFO_general_int16_peek_v1(struct _FIFO_int16_v1 *s);
static uint8 FIFO_general_int16_push_v1(int16_t dato,struct _FIFO_int16_v1 *s);
static uint8 FIFO_general_int16_pop_v1(int16_t *dato,struct _FIFO_int16_v1 *s);
static uint8 FIFO_general_int16_search_v1(int16_t dato,struct _FIFO_int16_v1 *s);

static int16 FIFO_volatile_int16_peek_v1(volatile struct _FIFO_volatile_int16_v1 *s);
static uint8 FIFO_volatile_int16_push_v1(int16 dato,volatile struct _FIFO_volatile_int16_v1 *s);
static uint8 FIFO_volatile_int16_pop_v1(int16 *dato,volatile struct _FIFO_volatile_int16_v1 *s);
static uint8 FIFO_volatile_int16_search_v1(int16 dato,volatile struct _FIFO_volatile_int16_v1 *s);
static void reset_FIFO_volatile_int16_v1(volatile struct _FIFO_volatile_int16_v1 *s,uint8 size);
static uint8 busqueda_v_v4(volatile int16 *arr,volatile uint8 size,int16 dato);
static volatile int16 *shift_left_volatile_int16(volatile int16 *arr,uint8 size);



unsigned char  buffer1[SIZE_BUFFER1];//buffer FIFO de tx IOUP
unsigned char  buffer2[SIZE_BUFFER1];//buffer FIFO de rx IOUP
unsigned char  buffer3[SIZE_BUFFER2];//fifo de i2c TX 
unsigned char  buffer4[SIZE_BUFFER2];//FIFO DE I2C  RX
unsigned char  buffer5[SIZE_BUFFER5];//fifo  DE tx a MCU-TFT-IO
unsigned char  buffer6[SIZE_BUFFER6];//fifo  DE rx from mcu-io-tft 

unsigned char keypadIn[SIZE_KEYPAD_IN];
unsigned char FIFO_SER_KEYPAD[SIZE_SER_KEYPAD];
unsigned char FIFO_DDS_DISPLAY_PIXEL_X[SIZE_FIFO_DDS_pixel];
unsigned char FIFO_DDS_DISPLAY_PIXEL_Y[SIZE_FIFO_DDS_pixel];
unsigned char FIFO_DDS_DISPLAY_PIXEL_P[SIZE_FIFO_DDS_pixel]; //PEN
unsigned char FIFO_DDS_DISPLAY_CHAR_X[SIZE_DDS_PIXEL];//display posicion y char si y=0xFF es character sino es posicion
unsigned char FIFO_DDS_DISPLAY_CHAR_Y[SIZE_DDS_PIXEL];
unsigned char FIFObox[SIZE_FIFO_BOX];
unsigned char FIFOboxCharx[SIZE_FIFO_BOX];
unsigned char FIFOboxChary[SIZE_FIFO_BOX];
signed short int FIFO_OFFSET_X[SIZE_FIFO_OFFSET];
signed short int FIFO_OFFSET_Y[SIZE_FIFO_OFFSET];
unsigned char IO_TX_BUFFER[SIZE_IO_TX_BUFFER];//array donde se guarda la direccion del paquete que se va enviar
unsigned char B[SIZE_B];//ARRAY donde se guardan los paquetes de datos a enviar ala tarjeta IO o CPU
unsigned char FIFO_BARRA_NUM[SIZE_BARR_NUM];
signed short int FIFO_CMD_DDSx[SIZE_FIFO_CMD_DDS];//FIFO DE CMD DDS TOUCHSCREEN
signed short int FIFO_CMD_DDSy[SIZE_FIFO_CMD_DDS];
//unsigned char *head5p,*head5x,*head5y,*tail5,*tail5p,*tail5y,*tail5x,*last5,*last5x,*last5y,*last5p;
unsigned char *head6x,*tail6x,*last6x;
unsigned char *head7y,*tail7y,*last7y;
unsigned char *head8p,*tail8p,*last8p;
unsigned char *headx,*heady,*taily,*tailx;//*head4x,*tail4x,*last4x;
unsigned char *popx, *popy, *pushx, *pushy, cuentaDDS;//*head5y,*tail5y,*last5y;
unsigned char *headbox,*lastbox,*tailbox;
unsigned char *headboxCharx,*headboxChary,*lastboxCharx,*lastboxChary,*tailboxCharx,*tailboxChary;
signed short int *headox,*headoy,*headzerox;//pointer del fifo offset
unsigned char *headTX,*lastTX,*popTX;//head y last de los array A y B
unsigned char **phh,*ph;//apuntadores para el pop como header en arra A y B
unsigned char *headBN,*lastBN,*pushBN,*popBN,cuentaBN;
signed short int *first1x,*last1x,*head1x,*tail1x;
signed short int *first1y,*last1y,*head1y,*tail1y;


void init_queues(void){
	//unsigned char c;
	  /*init_FIFO_DDS_Display_Pixel2();
	  init_FIFO_DDS_Display_Pixel2_v2();
	  init_FIFO_DDS_Display_Char2();
	  malloc_display_Box_Chars();
	  init_FIFO_OFFSET();
	  init_FIFOs_TX_IOUP();
	  init_FIFO_BARRA_NUMERO();
	  init_FIFO_TX_serial_General();
	  init_FIFO_CMD_DDS();
	  init_FIFO_SaveTemp_pixel_DDS();
	  init_FIFO_General_1byte(&serialKeyPad,&FIFO_SER_KEYPAD[0],&FIFO_SER_KEYPAD[SIZE_SER_KEYPAD-1],SIZE_SER_KEYPAD);
	  init_FIFO_General_1byte(&serial.rx,&buffer2[0],&buffer2[SIZE_BUFFER1-1],SIZE_BUFFER1);
	  init_FIFO_General_1byte(&serial.tx,&buffer1[0],&buffer1[SIZE_BUFFER1-1],SIZE_BUFFER1);
	  init_FIFO_General_1byte(&i2c.tx,&buffer3[0],&buffer3[SIZE_BUFFER2-1],SIZE_BUFFER2);
	  init_FIFO_General_1byte(&i2c.rx,&buffer4[0],&buffer4[SIZE_BUFFER2-1],SIZE_BUFFER2);
	 */
	  init_FIFO_General_1byte_v2(&com.TxDisp,&buffer5[0],SIZE_BUFFER5);     	  
	  init_FIFO_General_1byte_v2(&com.RxDisp,&buffer6[0],SIZE_BUFFER6);     	  
	  init_FIFO_volatile_int16_v1(&Signale.Raw.X,&Signale.Raw_X[0],SIZE_DSP_FIFOs);
	  init_FIFO_volatile_int16_v1(&Signale.Raw.Y,&Signale.Raw_Y[0],SIZE_DSP_FIFOs);
	  init_FIFO_General_int16_v1(&Signale.CentOffset.X,&Signale.Cent_X[0],SIZE_DSP_FIFOs);
	  init_FIFO_General_int16_v1(&Signale.CentOffset.Y,&Signale.Cent_Y[0],SIZE_DSP_FIFOs);
	  
      	  
     	  //init_FIFO_IIC();//se encuentra en-->> init_I2C
}//fin de initizalizr las FIFOS------------------------------



void init_FIFO_TX_serial_General(void){
		  serial.tx.head=&buffer1[0];
		  serial.tx.tail=&buffer1[SIZE_BUFFER_TX-1];
		  serial.tx.pop=serial.tx.head;
		  serial.tx.push=serial.tx.head;
		  serial.tx.ncount=0;
	      serial.vars.Flags.bitF.DDS_ACTIVA=FALSE;//reset registro de repetdos para graficar el DDS
}//Fin init_FIFO_TX_serial_General---------------------------


/*parametro 
 * 1: La fifo a inizializar
 * 2: pointer to first element array of fifo
 * 3:pointer to last element array of fifo
 * 4: size of fifo 
 * version 300322-1205
 * version 310322-1637 add reset as general */
void init_FIFO_General_1byte(struct _FIFO_1byte_ *s,unsigned char *h,unsigned char *t,unsigned char size){//FIFO_SER_KEYPAD[SIZE_SER_KEYPAD];
	s->head=h;
	s->tail=t;
	s->pop=s->head;
	s->push=s->head;
	s->ncount=0;
	s->popf=FIFO_general_1byte_pop;
	s->appendByte=FIFO_general_1byte_push;
	s->size=size;
	s->resetFIFO=reset_FIFO_general_UChar;
}//fin init_FIFO_RX_serial_Keypad-----------------------------------


//se resetea toda la fifo y todo queda cmo de inicio
//  serial.resetFIFO=reset_FIFO_serial_TX;
// version 310322-1626 
void reset_FIFO_general_UChar(struct _FIFO_1byte_ *s){
	   s->push=s->head;
	   s->ncount=0;
	   for(s->pop=s->head;s->pop<=s->tail;s->pop++)
		   *(s->pop)=0;
	   s->pop=s->head;
}//fin reset_FIFO_serial_TX---fin se resetea toda la fifo



//fifo de la memoria para salvar los pixelles pintados o detectados
void init_FIFO_SaveTemp_pixel_DDS(void){
unsigned short int i;	
  dds.SaveTempPix.p[HEADx]=&dds.SaveTempPix.xPixel[0];
  dds.SaveTempPix.p[TAILx]=&dds.SaveTempPix.xPixel[SIZE_TEMP_PIXEL-1];
  dds.SaveTempPix.p[POPx] = dds.SaveTempPix.p[HEADx];
  dds.SaveTempPix.p[PUSHx]= dds.SaveTempPix.p[HEADx];
  dds.SaveTempPix.p[HEADy]=&dds.SaveTempPix.yPixel[0];
  dds.SaveTempPix.p[TAILy]=&dds.SaveTempPix.yPixel[SIZE_TEMP_PIXEL-1];
  dds.SaveTempPix.p[POPy] = dds.SaveTempPix.p[HEADy];
  dds.SaveTempPix.p[PUSHy]= dds.SaveTempPix.p[HEADy];
  for(i=0;i<SIZE_TEMP_PIXEL;i++){
			  dds.SaveTempPix.xPixel[i]=0;
			  dds.SaveTempPix.yPixel[i]=0;}
		          			 		 
}//fin init FIFO pixel dds-------------------------------

//iniiaizar fifo para el touch
void init_FIFO_CMD_DDS(void){
	      dds.touch.headx=&dds.touch.bufferX[0];
	      dds.touch.heady=&dds.touch.bufferY[0];
	      dds.touch.tailx=&dds.touch.bufferX[SIZE_BUFFER_DDS_FIFO-1];
	      dds.touch.taily=&dds.touch.bufferY[SIZE_BUFFER_DDS_FIFO-1];
	      dds.touch.popx=dds.touch.pushx=dds.touch.headx;
	      dds.touch.popy=dds.touch.pushy=dds.touch.heady;
	      dds.touch.ncount=0;
}//fin init_FIFO_CMD_DDS-------------------------------------

void init_FIFO_BARRA_NUMERO(void){
byte i;	
	 headBN=&FIFO_BARRA_NUM[0];
	 lastBN=&FIFO_BARRA_NUM[SIZE_BARR_NUM-1];
	 pushBN=popBN=headBN;                       
     for(i=0;i<SIZE_BARR_NUM;i++)
    	 FIFO_BARRA_NUM[i]=0;
}//----------------------------------------------------------------------


void init_FIFOs_TX_IOUP(void){
unsigned char i;

 
    for(i=0;i<SIZE_IO_TX_BUFFER;i++)
    	IO_TX_BUFFER[i]=0;
    headTX=&IO_TX_BUFFER[0];
    popTX=headTX;
    lastTX=&IO_TX_BUFFER[SIZE_IO_TX_BUFFER];
    
}//init_FIFOs_TX_IOUP----------------------------------------------------


void init_FIFO_OFFSET(void){
	headox=&FIFO_OFFSET_X[0];
    headoy=&FIFO_OFFSET_Y[0];
    headzerox=headox;   
	
}//fin initizalizatione de FIFO offset-------------------------


/*  init la FIFO es la cola para enlistar los datos
 * que se van a mandar por serial cuando estemos en PANTALLA_DDS   */
void init_FIFO_DDS_Display_Pixel2_v2(void){
unsigned short int i;

     dds.pixel.headx=&dds.pixel.buffX[0];
     dds.pixel.heady=&dds.pixel.buffX[0];
     dds.pixel.head=&dds.pixel.pen[0];
     dds.pixel.tail =&dds.pixel.pen[SIZE_DDS_PIXEL-1];
     dds.pixel.tailx=&dds.pixel.buffX[SIZE_DDS_PIXEL-1];
     dds.pixel.taily=&dds.pixel.buffY[SIZE_DDS_PIXEL-1];
     dds.pixel.popx=dds.pixel.headx;
     dds.pixel.popy=dds.pixel.heady;
     dds.pixel.pop=dds.pixel.head;
     dds.pixel.pushx=dds.pixel.popx;
     dds.pixel.pushy=dds.pixel.popy;
     dds.pixel.push=dds.pixel.pop;
     for(i=0;i<SIZE_DDS_PIXEL;i++){
    	   dds.pixel.buffX[i]=0;
    	   dds.pixel.buffY[i]=0;
    	   dds.pixel.pen[i]=0xFF;} 
}//fin init_FIFO_DDS------------------------------------------------------



/*  init la FIFO es la cola para enlistar los datos
 * que se van a mandar por serial cuando estemos en PANTALLA_DDS   */
void init_FIFO_DDS_Display_Pixel2(void){
unsigned char *px,*py,*p;    
	 head6x=&FIFO_DDS_DISPLAY_PIXEL_X[0];
	 head7y=&FIFO_DDS_DISPLAY_PIXEL_Y[0];
	 head8p=&FIFO_DDS_DISPLAY_PIXEL_P[0];
	 tail6x=&FIFO_DDS_DISPLAY_PIXEL_X[SIZE_FIFO_DDS_pixel-1];
	 tail7y=&FIFO_DDS_DISPLAY_PIXEL_Y[SIZE_FIFO_DDS_pixel-1];
	 tail8p=&FIFO_DDS_DISPLAY_PIXEL_P[SIZE_FIFO_DDS_pixel-1];
	 last6x=head6x;
	 last7y=head7y;
	 last8p=head8p;
	 *last6x=0;
	 *last7y=0;
	 *last8p=0;
	 for(px=head6x,py=head7y,p=head8p;px<=tail6x;px++,py++,p++){
		    *p=0xFF;
	        *px=0;
	        *py=0;}
}//fin init_FIFO_DDS------------------------------------------------------




/*  init la FIFO es la cola para enlistar los datos
 * que se van a mandar por serial cuando estemos en PANTALLA_DDS   */
void init_FIFO_DDS_Display_Char2(void){
unsigned char *px,*py;    
	 headx=&FIFO_DDS_DISPLAY_CHAR_X[0];
	 heady=&FIFO_DDS_DISPLAY_CHAR_Y[0];
	 tailx=&FIFO_DDS_DISPLAY_CHAR_X[SIZE_DDS_PIXEL-1];
	 taily=&FIFO_DDS_DISPLAY_CHAR_Y[SIZE_DDS_PIXEL-1];
	 popx=pushx=headx;
	 popy=pushy=heady;
	 for(px=headx;px<=tailx;px++)
	        *px=0;
	 for(py=heady;py<=taily;py++)
	        *py=0xFF;//es solo char, no es posicion.
	 cuentaDDS=0;//cuenta el numero de elementos guardados
}//fin init_FIFO_DDS------------------------------------------------------


void FIFO_Display_DDS_Char_clean(void){
	init_FIFO_DDS_Display_Char2();}

void FIFO_Display_DDS_Pixel_clean(void){
	init_FIFO_DDS_Display_Pixel2();}

//return FALSE if is empty
/* version 300322-1156*/
unsigned char FIFO_general_1byte_pop(unsigned char *dato,struct _FIFO_1byte_ *s){	
	if(s->ncount==0)
		return FALSE;
	*dato=*(s->pop);//solo hay un dato en la FIFO
	*(s->pop)=0;//vaciamos nodo
	switch(get_case_FIFO_general(s)){
		case 33:
		case 1:s->ncount=0;
		       break;
		case 6:       
		case 2:s->ncount--;
		       s->pop=s->head;
		       break;
		case 3:
		case 4:
		case 5:
		case 7:
		case 8:s->ncount--;
		       s->pop++;//un lugar hacia abajo
		       break;	
		default: __asm(Halt); break;}//error de programacion}//fin switch
return TRUE;
}//FIFO_general_1byte_push------------------------------------------


unsigned char FIFO_general_1byte_push(unsigned char dato,struct _FIFO_1byte_ *s){
	  if(s->ncount<s->size)
		 goto  lqwe2;
	  else return FALSE;//FIFO llena
lqwe2:
	switch(get_case_FIFO_general(s)){
		case 5:
		case 33:*(s->push)=dato;
	            s->ncount++;
	            s->push=s->head;
			    break;
		case 1:*(s->push)=dato;
			   s->ncount++;
			   if(s->push==s->tail)
				   s->push=s->head;
			   else s->push++;
		       break;
		case 7:       
		case 2:*(s->push)=dato;
		       s->ncount++;
		       s->push++;
		       break;
		case 3:*(s->push)=dato;//nomas cabe un dato, este dato y ya
	           s->ncount++;
	           break;
		case 8:       
		case 6:       
		case 4:*(s->push)=dato;//nomas cabe un dato, este dato y ya
               s->ncount++;
               if(((s->push)+1)!=s->pop)//el que sigue es pop, 'tons'ta llena
            	    s->push++;
               break;	
		default: __asm(Halt); break;}//error de programacion}//fin switch
return TRUE;
	
	
}//FIFO_general_1byte_push------------------------------------------


/* version 300322-1212*/
unsigned char get_case_FIFO_general(struct _FIFO_1byte_ *s){
	 if(s->pop==s->push)
		    return 1;//
	 if((s->pop==s->tail)&&(s->push==s->head))
		    return 2;
	 if((s->pop==s->head)&&(s->push==s->tail))
		     return 3;
	 if((s->pop==s->tail)&&(s->push==s->tail))
	 	     return 33;
	 if((s->pop!=s->tail)&&(s->push==s->head))
	 		 return 4;
	 if((s->pop!=s->head)&&(s->push==s->tail))
	 		 return 5;
	 if((s->pop==s->tail)&&(s->push>s->head))
	 	     return 6;
	 if(s->pop<s->push)//estan en medio y pop esta arriba de push
	 	 	 return 7;
	 if(s->pop>s->push)
	 	 	 return 8;
	 else return 0xAA; 	 	 	       	 
}//finget_case_FIFO_general--------------------------------------



/*Mete en una FIFO los datos que se van a mandar al DDS
 * */
unsigned char FIFO_Display_DDS_Pixel_push_v2(unsigned char datox,unsigned char datoy,unsigned char pen){
	if(dds.Bnderas.bit.Apagando==TRUE)
		return;
	if(isFIFO_DDS_Display_Pixel_llena2_v2()){//tal vez necesitamos regresar un true o false
		 return FALSE;}//asm(Halt); //Debug, aumentar el tamaño error de diseño de software
   *dds.pixel.pushx=datox;
   *dds.pixel.pushy=datoy;
   *dds.pixel.push=pen;
   if(dds.pixel.pushx==dds.pixel.tailx){
	   dds.pixel.pushx=dds.pixel.headx;
	   dds.pixel.pushy=dds.pixel.heady;
	   dds.pixel.push=dds.pixel.head;}
   else{ dds.pixel.pushx++;
         dds.pixel.pushy++;
         dds.pixel.push++;}
   dds.pixel.ncount++;
	     
return TRUE;
}//fin FIFO_DDS_push---------------------------------------------



/*Mete en una FIFO los datos que se van a mandar al DDS
 * */
unsigned char FIFO_Display_DDS_Pixel_push(unsigned char datox,unsigned char datoy,unsigned char pen){
static unsigned long int debug2;
//static unsigned char x1[100],y1[100];
unsigned short int i;
  
    if(dds.Bnderas.bit.Apagando==TRUE)
		return;
	if(datox==0)
		if(datoy==0)
			return;
    if(isFIFO_DDS_Display_Pixel_llena2()){//tal vez necesitamos regresar un true o false
		 return FALSE;}//asm(Halt); //Debug, aumentar el tamaño error de diseño de software
	debug2++;
	for(i=0;i<100;i++){
			if(dds.rePaint.x1[i]==datox){
			   if(dds.rePaint.y1[i]==datoy)
				   goto l234;}}
	for(i=0;i<100;i++){
		if((dds.rePaint.x1[i]==0)&(dds.rePaint.y1[i]==0)){
			dds.rePaint.x1[i]=datox;
			dds.rePaint.y1[i]=datoy;
		    break;}}
   	
	
l234:
    if(datox==115)
    	 __asm(nop);
    if(datox==116)	
    	 __asm(nop);
	*head6x=datox;
	*head7y=datoy;
	*head8p=pen;
	if(head8p==tail8p){//estamos all final
		if(last8p==&FIFO_DDS_DISPLAY_PIXEL_P[0])
              return TRUE;   //ya esta llena pero si cupo
		else {head8p=&FIFO_DDS_DISPLAY_PIXEL_P[0];
			  head6x=&FIFO_DDS_DISPLAY_PIXEL_X[0];
			  head7y=&FIFO_DDS_DISPLAY_PIXEL_Y[0];}}
	else{if((head8p+1)==last8p){//esta lleno pero cup el ultimo
		      if(*last8p==0xFF){//esta vacio el nodo
		    	  head8p=last8p;
		    	  head6x=last6x;
		    	  head7y=last7y;
		    	  return TRUE;}//el last eta vacio
		      else{return TRUE;}}//Esta lleno la fifo pero si cupo
	     else {head8p++;
	           head6x++;
	           head7y++;}}
return TRUE;
}//fin FIFO_DDS_push---------------------------------------------





/*Mete en una FIFO los datos que se van a mandar al DDS
 * */
unsigned char FIFO_Display_DDS_Char_push(unsigned char datox,unsigned char datoy){
	if(dds.Bnderas.bit.Apagando==TRUE)
			return;	
	if(cuentaDDS==(SIZE_DDS_PIXEL-1))
		 __asm(Halt);//debug error de software
	if(*pushy==0xFF){//esta libre el nodo
		*pushx=datox;
	    *pushy=datoy;}
	else {
		if(menu.contexto.Actual==AJUSTE_DE_VIBRACION)
			         return TRUE;
		else{__asm(nop);
             __asm(Halt);}}//debug error de ingenieria no hay nodo libre
	if(pushx==(next(headx,tailx,pushx,popx))){
		if(menu.contexto.Actual==AJUSTE_DE_VIBRACION)
	         return TRUE; 
		else{asm(Halt);//debug,fifo llena, pasa avece en ajuste de vibracion
		     return FALSE;}}
	else {pushx=next(headx,tailx,pushx,popx);
	      pushy=next(heady,taily,pushy,popy);
	      cuentaDDS=FIFOcount(cuentaDDS,SUMAR);}
return TRUE;
}//fin FIFO_DDS_push---------------------------------------------



//evalua como esta la  FIFO circular  del DDS
//TRUE si esta llena
unsigned char isFIFO_DDS_Display_Pixel_llena2_v2(void){
	
	
	if(dds.pixel.pushx==dds.pixel.popx)
		return FALSE;
	else{if(dds.pixel.pushx==dds.pixel.tailx){
	             if(dds.pixel.popx==dds.pixel.headx)
	                     return TRUE; //fifo llena
	             else return FALSE;}
	    else{if((dds.pixel.pushx+1)==dds.pixel.popx)
	                   return TRUE;//fifo llena
	    	 else{return FALSE; }}
	}
				          
			  
}//FIN isFIFO_DDS_llena-------------------------------------------


//evalua como esta la  FIFO circular  del DDS
unsigned char isFIFO_DDS_Display_Pixel_llena2(void){
	  if(FIFO_DDS_next_X(head6x)==last6x)//el que sigue es el lasts?
		  return TRUE; // si esta llena
	  if(head6x==last6x){
		  if(*head8p==0xFF)
			   return FALSE;//no esta llena la fifo circular		 
		  else return FALSE;}//NO fifo circular llena}
	  else return FALSE; // no esta llena la fifo circular	
}//FIN isFIFO_DDS_llena-------------------------------------------





unsigned char isFIFO_DDS_Display_Pixel_empty2_v2(void){
	 if(dds.pixel.ncount==0)
		 return TRUE;
return FALSE;	 
}//fin isFIFO_DDS_empty-------------------------------------------

unsigned char *gotonext(unsigned char *last,unsigned char *first,unsigned char *tail){  
	 if(last==tail)
		 last=first;
	 else return(last+1);
}//fin goto next----------------------------------------------------------------------


unsigned char isFIFO_DDS_Display_Char_empty2(void){
	   if(cuentaDDS==0)
		    return TRUE;//Esta vacia la FIFO
	   else return FALSE;//no esta vacia la FIFO
	   
}//fin isFIFO_DDS_empty-------------------------------------------


/* Esta funcion te regresa el siguiente * 
 * *p es la direcion del array que vamos a  regresar la direccion siguiente hacia abajo
 *  circular. */
unsigned char *FIFO_DDS_next_X(unsigned char *p){
	  if(p==tail6x) //estamos al final del array? tons regresa la direccin,0,
		   return &FIFO_DDS_DISPLAY_PIXEL_X[0];
	  return (p+1);
}//fin  del next del FIFO indicado en el indice *p


unsigned char isFIFO_DDS_Display_Pixel_empty2(void){
	   if(last6x==tail6x){
		   if(head6x==&FIFO_DDS_DISPLAY_PIXEL_X[0]) 
			   return TRUE;//ESTA vacia la FIFO o hay spacio
		   else  return FALSE;}//no esta vacia la FIFO
	   else {if((last8p+1)==head8p){
		          if(*head8p==0xFF)
		        	   return TRUE;//hay espacio
		          else 
		               return FALSE;}//no esta vacia la FIFO
	        else {if(last6x==head6x){
	        	       if(*last8p==0xFF){//nos realineamos a la direcion 0 si esta vacio
	        	    	   last6x=head6x=&FIFO_DDS_DISPLAY_PIXEL_X[0];
	        	    	   last7y=head7y=&FIFO_DDS_DISPLAY_PIXEL_Y[0];
	        	    	   FIFO_DDS_DISPLAY_PIXEL_P[0]=0xFF;
	        	    	   last8p=head8p=&FIFO_DDS_DISPLAY_PIXEL_P[0];
	        	    	   return TRUE;}//ESTA vaciaV la FIFO
	        	       else return FALSE;}//no esta vacia la FIFO
	              else return FALSE;}}//no esta vacia la FIFO
}//fin isFIFO_DDS_empty-------------------------------------------



/*regresa el valor de la fifo el dato y el delay si regresa 0, es que la FIFO es
 * esta vacia
 * */
unsigned char FIFO_Display_DDS_Pixel_pop2_v2(unsigned char *x,unsigned char *y,unsigned char *p){
//unsigned char ret;
     *p=*dds.pixel.pop;
     *x=*dds.pixel.popx;
     *y=*dds.pixel.popy;
     *dds.pixel.popx=0;//vaciamos nodo
     *dds.pixel.popy=0;
     *dds.pixel.pop=0xFF;//nodo vacio
     dds.pixel.ncount--;
     if(dds.pixel.popx==dds.pixel.tailx){//estoy al final del array
    	   dds.pixel.popx=dds.pixel.headx;
    	   dds.pixel.popy=dds.pixel.heady;
    	   dds.pixel.pop=dds.pixel.head;
           return TRUE;}
     else{dds.pixel.popx++;
          dds.pixel.popy++;
          dds.pixel.pop++;
          return TRUE;}
   
return TRUE;
}// fin FIFO_DDS_pop


/*regresa el valor de la fifo el dato y el delay si regresa 0, es que la FIFO es
 * esta vacia ANTES DE EJECUTAR ESTA FUNCION SE VERIFICO  QUE NO ESTA VACIA LA FIFO
 * */
unsigned char FIFO_Display_DDS_Char_pop2(unsigned char *x,unsigned char *y){
//unsigned char ret;
     *x=*popx;
     *y=*popy;

     *popx=0;//vaciamos nodo
     *popy=0xFF;//nodo vacio
     cuentaDDS=FIFOcount(cuentaDDS,RESTAR);
     popx=next(headx,tailx,popx,pushx);
     popy=next(heady,taily,popy,pushy);
          
return TRUE;
}// fin FIFO_DDS_pop


/* Construccion de la lista FIFO
 * Memory Allocation of Box lists data structure double linked list*/
void malloc_display_Box(void){//struct NodoBox *N){
unsigned char *p;
        headbox=&FIFObox[0]; 
        lastbox=headbox;
        tailbox=&FIFObox[SIZE_FIFO-1]; //oldest
        *tailbox=0xFF;
        *headbox=0xFF;
        for(p=headbox+1;p<tailbox;p++)//verificar que es menos 2
                *p=0xFF;
}// fin malloc----------------------------------------------------------------------------------------------------------

/* construccion de la lista FIFO para desplegar chars en los menus que 
 * usan barra de ddeteccion*/
void malloc_display_Box_Chars(void){//struct NodoBox *N){
unsigned char *p;
        headboxChary=&FIFOboxChary[0];
        headboxCharx=&FIFOboxCharx[0];                
        lastboxChary=headboxChary;
        lastboxCharx=headboxCharx;
        tailboxChary=&FIFOboxChary[SIZE_FIFO-1];
        tailboxCharx=&FIFOboxCharx[SIZE_FIFO-1]; //oldest
        *tailboxChary=0xFF;
        *headboxChary=0xFF;//0xFF nos indica nodo vacio.
        for(p=headboxChary+1;p<tailboxChary;p++)//verificar que es menos 2
                *p=0xFF;
}// fin malloc----------------------------------------------------------------------------------------------------------


unsigned char FIFO_displayBox_push(unsigned char box){
//struct NodoBox *p;
      if(*headbox==0xFF){//esta vacio?
        	 *headbox=box;
         if(headbox==tailbox){//estoy en tail?
             if(FIFObox[0]==0xFF)//el que sigue esta vacio
                  headbox=&FIFObox[0];
             else return TRUE;}//Si hubo lugar, pero nos quedamos aqui, esta lleno toda la lista
         else {headbox++;
               return TRUE;}}
      else return FALSE;//no hay lugar
}//fin insert in FIFO the box number-------------------------------------------------------------


//mete en FIFO Los chars y posiciones  para cuando estamos en un menu con barra de deteccion
unsigned char FIFO_displayBoxChar_push(unsigned char x,unsigned char y){
//struct NodoBox *p;
      if(*headboxChary==0xFF){//esta vacio?
         *headboxChary=y;
         *headboxCharx=x;
         if(headboxChary==tailboxChary){//estoy en tail?
             if(FIFOboxChary[0]==0xFF){//el que sigue esta vacio
                  headboxChary=&FIFOboxChary[0];
                  headboxCharx=&FIFOboxCharx[0];}
             else return TRUE;}//Si hubo lugar, pero nos quedamos aqui, esta lleno toda la lista
         else {headboxChary++;
               headboxCharx++;
               return TRUE;}}
      else return FALSE;//no hay lugar
}//fin insert in FIFO the box number-------------------------------------------------------------


unsigned char FIFO_displayBox_pop(void){	  
unsigned char b;
       if(*lastbox==0xFF)
    	   return 0xFF;//esta vacia
       b=*lastbox;
       *lastbox=0xFF;//borrarmos el dato que sacamos
       FIFObox_nextLast();
       return b;//si hubo dato pero era el ultimo
}//fin pop from FIFO ---------------------------------------------------------------------------------------


unsigned char FIFO_displayBoxChars_pop(unsigned char *x,unsigned char *y){	  
       if(*lastboxChary==0xFF)
    	   return 0;//esta vacia
       *x=*lastboxCharx;
       *y=*lastboxChary;
       *lastboxChary=0xFF;//borrarmos el dato que sacamos
       *lastboxCharx=0;
       FIFOboxChars_nextLast();
       return 0xFF;//si hubo dato pero era el ultimo
}//fin pop from FIFO ---------------------------------------------------------------------------------------

unsigned char FIFObox_nextLast(void){
	   *lastbox=0xFF;
FnL1:  if(lastbox==tailbox)
		   lastbox=&FIFObox[0];
	   else lastbox++;
	   if(*lastbox==0xFF){
	        if(lastbox==headbox) 
	            return 0;
	        else goto FnL1;}
	   else return 0;  
}//fin FIFObox_nextLast-------------------------------------

unsigned char FIFOboxChars_nextLast(void){
	   *lastboxChary=0xFF;
FnL2:  if(lastboxCharx==tailboxCharx){
		   lastboxCharx=&FIFOboxCharx[0];
		   lastboxChary=&FIFOboxChary[0];}
	   else {lastboxCharx++;
	         lastboxChary++;}
	   if(*lastboxChary==0xFF){
	        if(lastboxChary==headboxChary) 
	            return 0;
	        else goto FnL2;}
	   else return 0;  
}//fin FIFObox_nextLast-------------------------------------





unsigned char FIFO_displayBox_isEmpty(void){
	   if(lastbox==headbox){
		   if(*lastbox==0xFF){
		       return TRUE;}//esta vacio
		   else {return FALSE;}}//hay un dato
	   else return FALSE;
}///fin de esta vacia la fifo de displayBox -------------------------------------------


//sirve para sacar y meter un dato mas viejo y meter el dato mas nuevo para un filtro digital de offset
signed short int popPushFIFO_OFFSET(signed short int *ry,signed short int xn,signed short int yn){
signed short int rx;	
	   rx=*headox;
	   *ry=*headoy;
	   *headox=xn;
	   *headoy=yn;
	   headzerox=&FIFO_OFFSET_X[0];
	   if(headox==headzerox){
		   headox=&FIFO_OFFSET_X[SIZE_FIFO_OFFSET-1];
		   headoy=&FIFO_OFFSET_Y[SIZE_FIFO_OFFSET-1]; }
	   else{headox--;
	        headoy--;}
return rx;
}// fin popPushFIFO-----------------------------------------------------


//unsigned char pushFIFO_IO_TX(unsigned char *data,unsigned char size){
//unsigned char i;	
//	if(size>IO_TX.Libres)//no hay lugar en la FIFO.
//		return FALSE;
//	for(i=0;i<size;i++,data++){
//	      *headTX=*data;
//	      IO_TX.Libres--;
//	      headTX=next(&IO_TX_BUFFER[0],&IO_TX_BUFFER[SIZE_IO_TX_BUFFER],headTX,popTX);}
//}//---------------------------------------------------------------------

//le damos el siguiente nodo a una FIFO y si se empalma con el pop no avanza
unsigned char *next(unsigned char *inicio,unsigned char *final,unsigned char *pos,unsigned char *pop){ 
   if(pos==final){
	   if(inicio==pop)
		   return pos;//esta llena la fifo y nos quedamos aqui;
	   else return inicio;}
   else {if((pos+1)==pop) //el que sigue es pop
	           return pos; //esta llena la FIFOY y no avanzamos
   	   	 else return pos+1;}
}//fin de next regresa la dreccion del siguiente nodo de una lista FIFO

//regresamos el siguiente nodo en una FIFO circular.         
signed short int **next2(signed short int *inicio,signed short int *final,signed short int **pos){
signed short int *p;
	   if(*pos==final)
		    *pos=inicio;
	   else  {p=*pos; 
		      //pos=*(*(pos+0)+1);//
	          p++;
	          *pos=p;
	          }
return pos;
}//fin de next nodo en la fifo ------------------------------------------


//carga el siguente nodo de la lista FIFO de datos a transmitir ala IO
unsigned char isLoad_next(unsigned char **p){
//unsigned char *p2;	  
	    if(*p==headTX){//llegamos al primer elemento de push que ya se mando a transmitir
	        **p=0;//se limpia el head
	        return TRUE;}//regresamos que ya no hay mas por transmitir, final de la FIFO
	    if(*p==&IO_TX_BUFFER[SIZE_IO_TX_BUFFER-1]){
	    	**p=0;//limipamos valor en el ultimo 
	    	*p=&IO_TX_BUFFER[0];
	        return FALSE;}//todavia no llemos al final de la FIFO
	    **p=0;//limpiamos nodo dela fifo que ya se envio
	    (*p)++;//incrementa la direccion a la cual estamos apuntando 
	    return FALSE;//todava no se termina la fifo
}//fin de cargar el siguiente nodo de comunicaciones-------------------------------------------


//aqui vamos a sacar el dato  mas antiguio o First Input amenos que no este llena la FIFO  sino esta
//llena no sacamos nada 
signed short int popFIFO_average_Offset(signed short int **pos,signed short int **pop,signed short int *tail,signed short int *ini){
signed short int ssi;	
	
       if(*pos==*pop){//se lleno la FIFO ó No ha entrado nada todavia?
    	  if(**pos==0) //no a entrado ningun dato  
		       return 0;//regresamos 0
    	  else {ssi=**pop;//esta llena la FIFO, y sacamos el elemento primero que llego
    	        **pop=0;//limpiamos el lugar que sacamos
    		    pop=next2(ini,tail,pop);
    	        return ssi;}}
        
}//fin popFIFO_average_Offset------------------------------------------------------------------
       
       
void pushFIFO_average_Offset(signed short int **pos,signed short int **pop,signed short int *tail,signed short int *ini,signed short int dato){
	
	 if(pos==pop){///se lleno la FIFO?
		 if(*pos==0) {// esta vacia la fifo
a527:         **pos=dato;//metemos el dato en la posision push
		       pos=next2(ini,tail,pos);}
		 else{goto a527;}}
	 else {**pos=dato;
	       pos=next2(ini,tail,pos);}
return;
}//fin pushFIFO_average_Offset--------------------------------------------------------
       
       




//cada vez que se transmite todo el buffer la lista que almacena los paquetes
// se resetean los punteros para volverla a llenarla como la primera vez
void reset_pointer(void){
	
	    headTX=&IO_TX_BUFFER[0];
	    popTX=headTX;
	    lastTX=&IO_TX_BUFFER[SIZE_IO_TX_BUFFER];
}//fn reset pointers-------------------------------------------------------------------

/* obtenemos el comando del paquete a ser enviado de un apuntador que apunta a la tabla(array A)
 * donde se guarda el apuntador que apunta a la tabla(aarray b) donde se guarda el comando */
unsigned char getComand_FIFO_B(unsigned char n){
	  
	return n;
}//fin obtener el byte dato comando de un apuntador que apunta a una tabla de apuntadores que apunta a otra tabla donde esta el comando guardado

//Desplega datos usando el sistema operativo cuando hay una barra 
void BarraDet_displayUINT_var(unsigned char posx,unsigned char posy,unsigned short int *usint){
unsigned char a[5],*p,n=0;
//uword3 v;	

		p=&a[0];
		getCharsFromUINT_var(p,*usint);
		VFDposicion_DDS(posx,posy);		
		VFDserial_SendChar_DDS(*(p+0)+0x30); //decmil+0x30);
		VFDserial_SendChar_DDS(*(p+1)+0x30);//mil+0x30);
		VFDserial_SendChar_DDS(*(p+2)+0x30);//cent+0x30);
		VFDserial_SendChar_DDS(*(p+3)+0x30);//dec+0x30);
		VFDserial_SendChar_DDS(*(p+4)+0x30);//unidad+0x30);	  
	   // if(n>0)
	     //  __asm(HALT);//debug alto, error de ingenieria de software
}//fin BarraDet_displayUINT_var-------------------------------------------------------------





//esta funcion mete datos ala FIFO DE BARRA con NUMEROS
//SI MANDA true es porque esta llena la FIFO
unsigned char FIFO_barraNum_push(unsigned char c){
unsigned char *p;	
	 
	  if(pushBN==popBN){
		   if(*pushBN==0){//esta vacia la fifo
		          *pushBN=c;
		           pushBN++;
		           cuentaBN++;
		           return FALSE;}
		   else{return TRUE;}}  // la fifo esta llena no escribimos nada
	  else{*pushBN=c;
	        p=pushBN;
	        pushBN=next(headBN,lastBN,pushBN,popBN);
	        if(p==pushBN)
	             return  TRUE;//fifo llena
	        else {cuentaBN++;
	        	  return FALSE;}} 
}//fin FIFO_barraNum_push---------------------------------------------------


void FIFO_barraNum_VFDposicion(unsigned char posx,unsigned char posy){
byte n=0;	
uword3 v;	

	    v.wordx=posx;
		n+=FIFO_barraNum_push(COMANDO_POS);//empieza el comando para desplegar 5 chars en una posicion
		n+=FIFO_barraNum_push(v.byte[LO]);
		v.wordx=posy;
		n+=FIFO_barraNum_push(v.byte[LO]);	
		if(n>0)
	          __asm(HALT);//debug alto, error de ingenieria de software
}//fin FIFO_barraNum_VFDposicion---------------------------------------------


void FIFO_barraNum_VFDserial_SendChar(unsigned char c){
byte n=0;
		n+=FIFO_barraNum_push(COMANDO_CHAR);//empieza el comando para desplegar 5 chars en una posicion
		n+=FIFO_barraNum_push(c); 
		if(n>0)
		  __asm(HALT);//debug alto, error de ingenieria de software
}//fin FIFO_barraNum_VFDserial_SendChar------------------------------------------


unsigned char  FIFO_barraNum_pop(void){
unsigned char c;	
	  if(popBN==pushBN){
		  if(popBN==0)
			   return 0xFF;//vacia la FIFO
		  else {c=*popBN;
		        *popBN=0;//limpiamos el nodo
		        cuentaBN--;
			    return c;}}
	  else{if(popBN==lastBN){
		           if(*popBN==0){
		        	   popBN=headBN;
		        	   return 0xFF;}
		           else{c=*popBN;
		                *popBN=0;
		                cuentaBN--;
		                popBN=headBN;
		                return c;}}
	  	   else{c=*popBN;
	  	        *popBN=0;
	  	         popBN++;
	  	         cuentaBN--;
	  	         return c;}}
		
}//fin FIFO_barraNum_pop---------------------------------------------------------


//regresa TRUE si no esta vacia la FIFO  de almacenar numero y posiciones en BArra detectora menus
unsigned char isFIFO_num_Not_Empty(void){//return TRUE if there is something in the FIFO
unsigned  char *i;

    if(*popBN>0)
    	  return TRUE;
    for(i=headBN;i<=lastBN;i++)
    	 if(*i>0)
    		 return TRUE;  
    return FALSE; 
}//fin isFIFO_num_Not_Empty----------------------------------------------------


unsigned char search_NO_Repetido(signed short int x,signed short int y){
unsigned short int i;	
unsigned char a=0,b=0;	 
static unsigned short int k;
       if(serial.vars.Flags.bitF.resetDDS){
    	     for(i=0;i<SIZE_MEM;i++)
    	    	if(mem1[i]==(x-y))
    	    		return FALSE;//si esta repetido
    	     //serial.Flags.bitFlag.resetDDS=FALSE;
    	     if(++nmem>(SIZE_MEM-1))   
                 nmem=0;
    	     mem1[nmem]=x-y;
             return TRUE;}//NO ESTA repetido
return TRUE; //no esta repetido.
}//fin search_NO_Repetido-----------------------------------------



/* funcion para meter en la FIFO de comando de DDS para 
 * transmitir el DDS al touchscreen   */
unsigned char push_FIFO_CMD_DDS(signed short int x,signed short int y){
const unsigned char FIFO_NOMOV =0x45;//NO MOVER pointer, ok
const unsigned char FIFO_VUELTA=0x46;//POinter esta al final va a head
const unsigned char FIFO_NORMAL=0x47;//POINTER se incrementa al siguiente
const unsigned char FIFO_1LUGAR=0x48;//HAY  un lugar y la dejamos llena
const unsigned char FIFO_VUELTA2=0x49;//POinter esta al final va a head
unsigned char m;	 
    if((x==0)&&(y==0))
    	 return;
    if(search_NO_Repetido(x,y)){
		if((*dds.touch.popx==0)&&(*dds.touch.popy==0)){
			if(dds.touch.popx==dds.touch.tailx)
				 m=FIFO_VUELTA;
			else m=FIFO_1LUGAR;}
		else{if(dds.touch.popx==dds.touch.tailx)
				 m=FIFO_VUELTA2; 
			 else{m=FIFO_NORMAL;}} 	 
		switch(m){
		  case FIFO_NOMOV:*dds.touch.popx=x;*dds.touch.popy=y;//meter valor y no mueve pointers
						   dds.touch.ncount++;
						   return TRUE;
		  case FIFO_VUELTA:*dds.touch.popx=x;*dds.touch.popy=y;//meter valor y mover pointer al head porque esta en tail
						   dds.touch.popx=dds.touch.headx;dds.touch.popy=dds.touch.heady;
						   dds.touch.ncount++;
						   return TRUE;
		  case FIFO_NORMAL:dds.touch.popx++;dds.touch.popy++;
						   *dds.touch.popx=x;*dds.touch.popy=y;
						   dds.touch.ncount++;
						   return TRUE;
		  case FIFO_1LUGAR:*dds.touch.popx=x;*dds.touch.popy=y;
						   dds.touch.ncount++;
						   return TRUE;//solo habia un lugar aqui nos quedamos
		  case FIFO_VUELTA2:dds.touch.popx=dds.touch.headx;dds.touch.popy=dds.touch.heady;
							*dds.touch.popx=x;*dds.touch.popy=y;
							dds.touch.ncount++;
							return TRUE;
		  default:return FALSE;break;}//fin switch
    }//nosearch 
}//fin push FIFO CMD DDS----------------------------------------



/* saca  de la FIFO que guarda las coordenadas x & y de deteccion
 *  de los canales de deteccion para ser graficados, ya sn offset
 *  version-1  */
unsigned char pop_FIFO_CMD_DDS(signed short int *x,signed short int *y){
const unsigned char FIFO_VACIA=0xE3;
const unsigned char FIFO_VALOR_NOM=0xE4;
const unsigned char FIFO_VUELTA= 0xE5;
const unsigned char FIFO_NORMAL=0xE6;
unsigned char m;
	 if(dds.touch.popx==dds.touch.pushx){
		  if((*dds.touch.popx==0)&&(*dds.touch.popy==0))
			   m=FIFO_VACIA;
		  else m=FIFO_VALOR_NOM;}
	 else{if(dds.touch.popx==dds.touch.tailx)
	            m=FIFO_VUELTA; 
	 	  else m=FIFO_NORMAL; }
	 switch(m){
      case FIFO_VACIA:dds.touch.ncount=0;return FALSE;//fifo vacia regresa falso
      case FIFO_VALOR_NOM:*x=*dds.touch.popx;//no se mueve apuntador NO MOVER 
	                    *y=*dds.touch.popy;
	                    *dds.touch.popx=0;*dds.touch.popy=0;//vaciamos nodo
	                    dds.touch.ncount--;
	                    return TRUE;
      case FIFO_VUELTA:*x=*dds.touch.popx;*y=*dds.touch.popy;//fin dela cola apuntara a la inicio
		             *dds.touch.popx=0;*dds.touch.popy=0;
		             dds.touch.popx=dds.touch.headx;dds.touch.popy=dds.touch.heady;
		             dds.touch.ncount--;
		             return TRUE;
      case FIFO_NORMAL:*x=*dds.touch.popx;*y=*dds.touch.popy;
                     *dds.touch.popx=0;*dds.touch.popy=0;
                     dds.touch.popx++;dds.touch.popy++;dds.touch.ncount--;
                     return TRUE;
		 default:return FALSE;}	  
}//fin de pop FIFO comando DDS transmision al touch-----------------





//se resetea toda la fifo y todo queda cmo de inicio
//  serial.resetFIFO=reset_FIFO_serial_TX;
//VERSION-1
void reset_FIFO_serial_TX(void){
	   
	   serial.tx.push=serial.tx.head;
	   serial.tx.ncount=0;
	   for(serial.tx.pop=serial.tx.head;serial.tx.pop<=serial.tx.tail;serial.tx.pop++)
           *serial.tx.pop=0;
	   serial.tx.pop=serial.tx.head;
	   
}//fin reset_FIFO_serial_TX---fin se resetea toda la fifo

/*el pointer last apunta al utimo elemento dela fifo siempre
 * no apunta al ultimo lugar libre sino al ultimo elemento
 *     APODO ES  serial.appendByte  
 *     version-1 */
void push_FIFO_TRANSMISION_serial_IO(unsigned char byte){
const unsigned char VACIO  =0xE4;//Esta vacio, push igual que pop
const unsigned char VUELTA =0xE5;//lleno nodo, push y pop en tail
//const unsigned char NORMAL1=0xE6;//nodo lleno, push pop en medio
const unsigned char LLENA  =0xE7;//nodo lleno,push en tail, pop en head
//const unsigned char VUELTA2=0xE8;//nodo lleno,push en tail, pop no en head
const unsigned char LLENA1 =0xE9;//nodo lleno, push en tail, pop en head
unsigned char m;
	
	 if(serial.tx.push==serial.tx.pop){
		  if(*serial.tx.push==0)
			   m=VACIO;
		  else{if(serial.tx.push==serial.tx.tail)
						 m=VUELTA;
			   else m=NORMAL;}}
	 else{if(serial.tx.push==serial.tx.tail){
				 if(serial.tx.pop==serial.tx.head)
						 m=LLENA;	  
				 else m=VUELTA;}
		  else m=NORMAL;}
	 switch(m){//estado l
		 case VACIO: *serial.tx.push=byte;
		             serial.tx.ncount++;
		  	  	  	 break;
		 case VUELTA:serial.tx.push=serial.tx.head;
		             *serial.tx.push=byte;
		             serial.tx.ncount++;
		             break;
		 case NORMAL:serial.tx.push++;
		             *serial.tx.push=byte;
		             serial.tx.ncount++;
		             break;
		 case LLENA: 
		 default:   //__asm(Halt);
		 	 	 	__asm(nop);__asm(nop);__asm(nop);__asm(nop);
		            break;}
return;		             		 
}//fin de meter en la fifo de transmision serial a a IO-------------


/* Regresa el dato que sigue de la FIFO del buffer para enviar
 *  el serial a la IO si regresa 0xAA y el estatus es 0101 0101==6 
 *  status XOR return =FF significa que no hay datos, vacia,
 *  VERSION-1 */
unsigned char pop_FIFO_TRANSMISION_serial_IO(unsigned char *status){
unsigned char n; 
     *status=0;
	 if(serial.tx.pop==serial.tx.push){//caso-.1 iguales
		 if(*serial.tx.pop==0){//caso 1 y 2 esta vacio head o tail
			   *status=0x55;//esta vacio y no nos movemos
		       n=0xAA;}	
		 else{ n=*serial.tx.pop;  //caso 3 y 4 no estan vacios en head o tail
			   *serial.tx.pop=0;
		       serial.tx.ncount--;}}
	 else{n=*serial.tx.pop;
          *serial.tx.pop=0;
          serial.tx.ncount--;
		  if(serial.tx.pop==serial.tx.tail)
		       serial.tx.pop=serial.tx.head;
	 	  else serial.tx.pop--;} 
return n;	 
}//fin pop FIFO de transmision serial IO


//vaciar el segundo buffer en el primer buffer-------------
void vaciarBuffer(unsigned char *p,unsigned char *p2,unsigned char size){
unsigned char i;
      for(i=0;i<size;i++){
    	  *(p+i)=0;
    	  *(p+i)=*(p2+i);
    	  *(p2+i)=0;}
}//fin vaciarBuffer----------------------------------------------------

/* fifo para meter coordenadas x&y que se pintan en el display
 * aunque se salgan del cuadro de los limites, 
 * regresa TRUE, si se guardo bien, falso si esta llena la FIFO */
unsigned char push_FIFO__save_Temp_pixel_DDS(signed short int x,signed short int y){

	if(dds.SaveTempPix.p[POPx]==dds.SaveTempPix.p[PUSHx]){
		if((*dds.SaveTempPix.p[PUSHx]==0)&(*dds.SaveTempPix.p[PUSHy]==0)){
			 *dds.SaveTempPix.p[PUSHx]=x;
			 *dds.SaveTempPix.p[PUSHy]=y;
			 dds.SaveTempPix.ncount++;
		     return TRUE;}
		else{if(dds.SaveTempPix.p[PUSHx]==dds.SaveTempPix.p[TAILx]){
			      dds.SaveTempPix.p[PUSHx]=dds.SaveTempPix.p[HEADx];
			      dds.SaveTempPix.p[PUSHy]=dds.SaveTempPix.p[HEADy];
			      *dds.SaveTempPix.p[PUSHx]=x;
			      *dds.SaveTempPix.p[PUSHy]=y;
			      dds.SaveTempPix.ncount++;
			      return TRUE;}
		     else{dds.SaveTempPix.p[PUSHx]++;
				  dds.SaveTempPix.p[PUSHy]++;
				  *dds.SaveTempPix.p[PUSHx]=x;
				  *dds.SaveTempPix.p[PUSHy]=y;
				  dds.SaveTempPix.ncount++;
		          return TRUE;}}}
	else{if((dds.SaveTempPix.p[PUSHx]+1)==dds.SaveTempPix.p[POPx]){
		       if((*dds.SaveTempPix.p[PUSHx]==0)&&(*dds.SaveTempPix.p[PUSHy]==0)){
		    	      *dds.SaveTempPix.p[PUSHx]=x;
		    	      *dds.SaveTempPix.p[PUSHy]=y;
		    	      dds.SaveTempPix.ncount++;
		    	      return TRUE;}
		       else{ return FALSE;}}//fifo llena
	     else{if(dds.SaveTempPix.p[PUSHx]==dds.SaveTempPix.p[TAILx]){
	    	        if(dds.SaveTempPix.p[POPx]==dds.SaveTempPix.p[HEADx]){
	    	        	   return FALSE;}//FIFO llena
	    	        else{dds.SaveTempPix.p[PUSHx]=dds.SaveTempPix.p[HEADx];
	    	             dds.SaveTempPix.p[PUSHy]=dds.SaveTempPix.p[HEADy];
	    	             *dds.SaveTempPix.p[PUSHx]=x;
	    	             *dds.SaveTempPix.p[HEADy]=y;
	    	             dds.SaveTempPix.ncount++;
	    	             return TRUE;}}
	          else{dds.SaveTempPix.p[PUSHx]++;
	               dds.SaveTempPix.p[PUSHy]++;
	               *dds.SaveTempPix.p[PUSHx]=x;
	               *dds.SaveTempPix.p[PUSHy]=y;
	               dds.SaveTempPix.ncount++;
	    	       return TRUE; }}}	
	
}//fin de FIFO pixel DDS.......................................


/* saca de la fifo las coordenadas de deteccion x&y ppara
 * repintar o cambiar el zoom en el DDS
 * return false cuando esta vacio*/
unsigned char pop_FIFO_save_Temp_pixel_DDS(signed short int *x,signed short int *y){
//signed short int *head;
	if(dds.SaveTempPix.p[POPx]==dds.SaveTempPix.p[PUSHx]){
		if((*dds.SaveTempPix.p[POPx]==0)&&(*dds.SaveTempPix.p[POPy]==0)){
			   return FALSE;}
		else{
			 *x=*dds.SaveTempPix.p[POPx];
		     *y=*dds.SaveTempPix.p[POPy];
		     *dds.SaveTempPix.p[POPx]=0;
		     *dds.SaveTempPix.p[POPy]=0;//debug
		     dds.SaveTempPix.ncount--;
		     return TRUE;}}
	else{*x=*dds.SaveTempPix.p[POPx];
         *y=*dds.SaveTempPix.p[POPy];
         *dds.SaveTempPix.p[POPx]=0;
         *dds.SaveTempPix.p[POPy]=0;
	     if(dds.SaveTempPix.p[POPx]==dds.SaveTempPix.p[TAILx]){
	    	  if(dds.SaveTempPix.p[PUSHx]==dds.SaveTempPix.p[HEADx]){
	    		      dds.SaveTempPix.p[POPx]=dds.SaveTempPix.p[HEADx];
	    		      dds.SaveTempPix.p[POPy]=dds.SaveTempPix.p[HEADy];
	    		      dds.SaveTempPix.ncount--; 
	    		      return TRUE;}
	    	  else{dds.SaveTempPix.p[POPx]=dds.SaveTempPix.p[HEADx];
	    	       dds.SaveTempPix.p[POPy]=dds.SaveTempPix.p[HEADy];
	    	       dds.SaveTempPix.ncount--;
	    	       return TRUE;}}
	     else{dds.SaveTempPix.p[POPx]++;
	          dds.SaveTempPix.p[POPy]++;
	          dds.SaveTempPix.ncount--;
	          if(dds.SaveTempPix.p[POPy]>dds.SaveTempPix.p[TAILy]){
	        	    __asm(nop);
	                __asm(Halt);}
	          return TRUE;}}
	    	        
}//fin pop fifo pixel DDS

unsigned char is_FULL_FIFO_save_Temp_pixel_DDS(void){
	if(dds.SaveTempPix.ncount==(SIZE_TEMP_PIXEL-1))
		return TRUE;//si esta lleno
return FALSE;
}//fin is_FIFO_save_Temp_pixel_DDS-------------------------

unsigned char is_Empty_FIFO_save_Temp_pixel_DDS(void){
	if(dds.SaveTempPix.ncount==0)
		return TRUE;
	else return FALSE;
}//fin de is_Empty_FIFO_save_Temp_pixel_DDS------------


/*regresa el valor de la fifo el dato y el delay si regresa 0, es que la FIFO es
 * esta vacia
 * */
unsigned char FIFO_Display_DDS_Pixel_pop2(unsigned char *x,unsigned char *y,unsigned char *p){
//unsigned char ret;
     *p=*last8p;
     *x=*last6x;
     *y=*last7y;
     *last6x=0;//vaciamos nodo
     *last7y=0;
     *last8p=0xFF;//nodo vacio
     if(last6x==tail6x){//estoy al final del array
    	   if(head6x==&FIFO_DDS_DISPLAY_PIXEL_X[0]){  //next esta fifo vacia
    		   last6x=head6x;
    		   last7y=head7y;
    		   last8p=head8p;
    		   return TRUE;}
    	   else {last6x=&FIFO_DDS_DISPLAY_PIXEL_X[0];
    	         last7y=&FIFO_DDS_DISPLAY_PIXEL_Y[0];
    	         last8p=&FIFO_DDS_DISPLAY_PIXEL_P[0];}}
     else {last6x++;
           last7y++;
           last8p++;}//se incrementa al segundo de la fifo
return TRUE;
}// fin FIFO_DDS_pop


/* limpiar los buffer que guardan los puntos de repintad cuando se
 *  cambia de zoom  */
void clean_repaint_Pixels_DDS(void){
#if (SIZE_DDS_ZOOM<253)
	unsigned char i,j;
#else
	unsigned short int i,j;
#endif
    for(i=0;i<SIZE_Y;i++)
     for(j=0;j<SIZE_X;j++)	
    	 dds.rePaint.xy[i][j]=0;
//    for(i=0;i<SIZE_DDS_ZOOM;i++){ 
//         dds.rePaint.xx[i]=0;
//         dds.rePaint.yy[i]=0;}
    dds.rePaint.ii=0;
   
	
}//fin  clean repaint pixels DDS 


void init_FIFO_IIC(void){
	 IIC.headRx=&(IIC.buffRX[0]);
	 IIC.headTx=&(IIC.buffTX[0]);
	 IIC.lasRx=&(IIC.buffRX[BUFFER_RX_IIC-1]);
	 IIC.lasTx=&(IIC.buffTX[BUFFER_TX_IIC-1]);
	 IIC.popRx=IIC.pushRx=IIC.headRx;
	 IIC.popTx=IIC.pushTx=IIC.headTx;
	 IIC.ncountRX=0;
	 IIC.ncountTX=0;
}//fin init_FIFO_IIC------------------------------



unsigned char IIC_FIFO_push_RX(unsigned char n){
	    n=1;
		__asm(nop);
		__asm(Halt);
}//FIN IIC_FIFO_push_RX-----------------------------


unsigned char IIC_FIFO_push_TX(unsigned char n){
   	    if(IIC.ncountTX<(BUFFER_TX_IIC-0)){
   	    	*IIC.pushTx=n;
   	    	 if(IIC.pushTx==IIC.headTx)
   	    		 IIC.pushTx=IIC.lasTx;
   	    	 else IIC.pushTx--;
   	        IIC.ncountTX++;
   	        return TRUE;}//everything ins ok
return FALSE;//fifo llena
}//fin IIC_FIFO_push_TX-----------------------------


unsigned char IIC_FIFO_pop_RX(void){
	__asm(nop);
	__asm(Halt);
     
}//fin  IIC_FIFO_pop_RX-----------------------------

unsigned char IIC_FIFO_pop_TX(void){
unsigned char n;	
	    if(IIC.ncountTX>0){ 
	       n=*(IIC.popTx);
	       *(IIC.popTx)=0;
	       if(IIC.popTx==IIC.headTx)
	    	   IIC.popTx=IIC.lasTx;
	       else IIC.popTx--;	
	       IIC.ncountTX--;	
	       return n;} 
	    return 0;
}// fin IIC_FIFO_pop_TX-----------------------------


/* Protocolo de asignacion de carga de paquete
 *  STX,DEVICE,LEN,APP,CMD,D0,D1,D2,D3,ETX  
 *   len=cuenta apartir despues de len e.g. arriba len=8 
 * */
/* sacamoS el paquete de tamaño 10 de la fifo f de II2C de 
 * y lo vaciamos en la direccion p 
 *   return
 *   Los parameros *p pointer to array package
 *   *pop  pointer to function pop or tx or rx 
 *    se regresa a paquete sin el STX*/
unsigned char get_Paquete(unsigned char *p,unsigned char (*pop)(void)){
unsigned char n;	
    if((pop())!=STX){
    	 return FALSE;}
    else{ if(is_device(pop,&n)){
    	       *p=n;//DEvice
    	       if(is_Lenght(pop,&n)){
    	    	  *(p+1)=n;//lenght
    	    	  if(is_App(pop,&n)){
    	    		 *(p+2)=n;//APP byte
    	    		 if(is_CMD(pop,&n,*(p+2))){
    	    			 *(p+3)=n;
    	    			 get_parametros(pop,p);
    	    		     return TRUE;}}}}} 	    				  
return FALSE;//error de protocolo    	    			 
}//FIN get_Paquete------------------------------------------------


/*  obtenemos TRue si el parametro tiene la direccion 
 * de algun dispositivo  registrado, parametro es un apuntador a funcion
 *  pop tx o rx  e pop de la fifo IIC
 *  param n returns the address 
 * */
unsigned char is_device(unsigned char (*pop)(void),unsigned char *n){
unsigned char nn;	
	nn=(*pop)();*n=0;
	switch(nn){
	  case EEPROM:
	  case ACELER:
	  case RTC_IC:
	  case ARDUINO:*n=nn;return TRUE;break;
	  default: return FALSE;;break;}	  
}//fin is_device--------------------------------------------------


//el byte que se va sacar dela pop tiene longitud mas de cero es valido sino no
unsigned char is_Lenght(unsigned char(*pop)(void),unsigned char *n){
//unsigned char m;
   *n=(*pop)();
   if((*n)>0)
	   return TRUE;
   else return FALSE;
}//fin is_Lenght----------------------------------------------------


//el byte que se saca de la pop de IIC  TX|RX  es para una aplicacion valida?
unsigned char is_App(unsigned char(*pop)(void),unsigned char *n){
unsigned char m;	
   	m=(*pop)(); *n=m;
   	switch(m){
   		case CAMBIO_FRECQ_APP:return TRUE;break;
   		default:return FALSE;break;		
   	}//fin de switch
}//fin is_App

//es un comando valido para la applicacion  seleccionada
unsigned char is_CMD(unsigned char(*pop)(void),unsigned char *n,unsigned char app){
unsigned char m;
   m=(*pop)();*n=m;
   switch(app){
	   case  CAMBIO_FRECQ_APP:if(m==NUEVA_FRECQ_CMD)
		                           return TRUE;
	                          break;
	   default:return FALSE;
return FALSE;	   
   }//fin switch
}//fin is_CMD


/* obtenemo los parametros para el paquete actual
 *  el utlimo byte debe ser el ETX*/
void get_parametros(unsigned char (*pop)(void),unsigned char *p){
unsigned char len,i;	
	len=*(p+1);
    for(i=0;i<len;i++)
    	  *(p+4+i)=(*pop)();
}//fin get parameters--------------------------------------



// Versión v2: funciones para _FIFO_1byte_v2_ (appendByte con un solo parámetro)
void init_FIFO_General_1byte_v2(struct _FIFO_1byte_v2 *s, unsigned char *buff,unsigned char size){
    s->head = buff;
    s->tail = buff+(size-1);//ultimo elemento
    s->pop = s->push= buff;
    s->ncount = 0;
    s->dequeue = FIFO_general_1byte_pop_v2;
    s->append = FIFO_general_1byte_push_v2;
    s->peek =FIFO_general_1byte_peek_v2;
    s->search=FIFO_general_1byte_search_v2;
    s->size = size;
    s->resetFIFO = reset_FIFO_general_UChar_v2;
}//fini init general-------------------------------------------



// Versión v2: funciones para _FIFO_1byte_v2_ (appendByte con un solo parámetro)
static void init_FIFO_General_int16_v1(
		                     struct _FIFO_int16_v1 *s, 
		                     int16 *buff,uint8 size){

    s->head = buff;
    s->tail = buff+(size-1);//ultimo elemento
    s->pop = s->push= buff;
    s->ncount = 0;
    s->dequeue = FIFO_general_int16_pop_v1;
    s->append = FIFO_general_int16_push_v1;
    s->peek =FIFO_general_int16_peek_v1;
    s->search=FIFO_general_int16_search_v1;
    s->size = size;
    s->resetFIFO = reset_FIFO_general_int16_v1;
}//fini init general-------------------------------------------



// Versión v2: funciones para _FIFO_1byte_v2_ (appendByte con un solo parámetro)
static void init_FIFO_volatile_int16_v1(
		                    volatile struct _FIFO_volatile_int16_v1 *s, 
		                    volatile  int16 *buff,
		                               uint8 size){

    s->head = buff;
    s->tail = buff+(size-1);//ultimo elemento
    s->pop = s->push= buff;
    s->ncount = 0;
    s->dequeue = FIFO_volatile_int16_pop_v1;
    s->append = FIFO_volatile_int16_push_v1;
    s->peek =FIFO_volatile_int16_peek_v1;
    s->search=FIFO_volatile_int16_search_v1;
    s->size = size;
    s->resetFIFO = reset_FIFO_volatile_int16_v1;
}//fini init general-------------------------------------------








void reset_FIFO_general_UChar_v2(struct _FIFO_1byte_v2 *s, unsigned char size){
    s->pop = s->push =s->head;
    s->ncount = 0;
    cleanArray(s->head, size, 0);
}//-----------------------------------------------


static void reset_FIFO_general_int16_v1(struct _FIFO_int16_v1 *s,uint8 size){
    s->pop = s->push =s->head;
    s->ncount = 0;
    cleanArray_int16(s->head, size, 0);
}//-----------------------------------------------

static void reset_FIFO_volatile_int16_v1(volatile struct _FIFO_volatile_int16_v1 *s,uint8 size){
    s->pop = s->push =s->head;
    s->ncount = 0;
    cleanArray_volatile_int16(s->head, size, 0);
}//-----------------------------------------------



//sacar un dato de la queue fifo
unsigned char FIFO_general_1byte_pop_v2(unsigned char *dato, struct _FIFO_1byte_v2 *s){
    if(s->ncount == 0)
        return FALSE;
    if(s->mode==0){//normal
		if(s->ncount == 1){
			*dato = *(s->pop);
			*(s->pop) = 0;
			s->pop = s->push = s->tail;
			s->ncount = 0;}
		else{
			*dato = *(s->pop);
			*(s->pop) = 0;
			if(s->ncount > 0) s->ncount--;
			if(s->pop == s->tail)
				s->pop = s->head;
			else
				s->pop++;}
		return TRUE;}
    else{//modo debug
    	*dato=*(s->head);
    	s->push=shift_left(s->head,s->ncount);
        s->ncount--;
    	return TRUE;}
return FALSE;    
}//fin fifo gen -------------------------------------------------



//sacar un dato de la queue fifo
static uint8  FIFO_general_int16_pop_v1(int16_t *dato,struct _FIFO_int16_v1 *s){
    if(s->ncount == 0)
        return FALSE;
    if(s->mode==0){//normal
		if(s->ncount == 1){
			*dato = *(s->pop);
			*(s->pop) = 0;
			s->pop = s->push = s->tail;
			s->ncount = 0;}
		else{
			*dato = *(s->pop);
			*(s->pop) = 0;
			if(s->ncount > 0) s->ncount--;
			if(s->pop == s->tail)
				s->pop = s->head;
			else
				s->pop++;}
		return TRUE;}
    else{//modo debug
    	*dato=*(s->head);
    	s->push=shift_left_int16(s->head,s->ncount);
        s->ncount--;
    	return TRUE;}
return FALSE;    
}//fin fifo gen -------------------------------------------------


//sacar un dato de la queue fifo
static uint8  FIFO_volatile_int16_pop_v1(int16_t *dato,
		volatile  struct _FIFO_volatile_int16_v1 *s){
    if(s->ncount == 0)
        return FALSE;
    if(s->mode==0){//normal
		if(s->ncount == 1){
			*dato = *(s->pop);
			*(s->pop) = 0;
			s->pop = s->push = s->tail;
			s->ncount = 0;}
		else{
			*dato = *(s->pop);
			*(s->pop) = 0;
			if(s->ncount > 0) s->ncount--;
			if(s->pop == s->tail)
				s->pop = s->head;
			else
				s->pop++;}
		return TRUE;}
    else{//modo debug
    	*dato=*(s->head);
    	s->push=shift_left_volatile_int16(s->head,s->ncount);
        s->ncount--;
    	return TRUE;}
return FALSE;    
}//fin fifo gen -------------------------------------------------









/* mueve todos elementos ala izq 1,2,3 ->queda-> 2,3 y regresa puntero al 3
 * ncount es la cantidad de elementos que hay en la fifo incluyendo el que se va a sacar */
static unsigned char *shift_left(unsigned char *arr,unsigned char size){
unsigned char last_value = arr[size - 1],i;
      switch(size){
    	  case 0:return &arr[0];
      	  case 1:arr[0]=0;return &arr[0];
      	  case 2:arr[0]=arr[1];arr[1]=0;return &arr[0];
      	  default:
      	  	  for(i=0;i< size-1;i++){
	               arr[i] = arr[i + 1];}
	          arr[size - 1] = 0;
	          return &arr[size-2];}
}//fin shift left------------------------------------------------



/* mueve todos elementos ala izq 1,2,3 ->queda-> 2,3 y regresa puntero al 3
 * ncount es la cantidad de elementos que hay en la fifo incluyendo el que se va a sacar */
static int16 *shift_left_int16(int16 *arr,uint8 size){
uint8 i;
int16 value = arr[size - 1];

      switch(size){
    	  case 0:return &arr[0];
      	  case 1:arr[0]=0;return &arr[0];
      	  case 2:arr[0]=arr[1];arr[1]=0;return &arr[0];
      	  default:
      	  	  for(i=0;i< size-1;i++){
	               arr[i] = arr[i + 1];}
	          arr[size - 1] = 0;
	          return &arr[size-2];}
}//fin shift left------------------------------------------------


/* mueve todos elementos ala izq 1,2,3 ->queda-> 2,3 y regresa puntero al 3
 * ncount es la cantidad de elementos que hay en la fifo incluyendo el que se va a sacar */
static volatile int16 *shift_left_volatile_int16(volatile int16 *arr,uint8 size){
uint8 i;
volatile int16 value = arr[size - 1];

      switch(size){
    	  case 0:return &arr[0];
      	  case 1:arr[0]=0;return &arr[0];
      	  case 2:arr[0]=arr[1];arr[1]=0;return &arr[0];
      	  default:
      	  	  for(i=0;i< size-1;i++){
	               arr[i] = arr[i + 1];}
	          arr[size - 1] = 0;
	          return &arr[size-2];}
}//fin shift left------------------------------------------------









//regresa FALSE si no se pudo insertar en la cola
unsigned char FIFO_general_1byte_push_v2(unsigned char dato, struct _FIFO_1byte_v2 *s){
unsigned char ret=FALSE;
  if(s->ncount < s->size){ 
    if(s->mode==0){//normal
			*(s->push) = dato;
			if(s->push==s->tail) s->push=s->head;
			else{s->push++;}
			s->ncount++;
			ret=TRUE;}
    else{//modo debug, en esta version no es circular
    	*(s->push) = dato;
    	s->push++;
    	s->ncount++;}}
  if(s->push>s->tail){__asm(nop);//error de software o pila
  	  	  	  	  	  __asm(Halt);}
return ret;
}//fin de push en fifo------------------------------------------------


//regresa F si no se pudo insertar en la cola
static uint8 FIFO_general_int16_push_v1(int16_t dato,struct _FIFO_int16_v1 *s){
unsigned char ret=FALSE;//no hay lugar, fifo llena
  if(s->ncount < s->size){ 
    if(s->mode==0){//normal
			*(s->push) = dato;
			if(s->push==s->tail) s->push=s->head;
			else{s->push++;}
			s->ncount++;
			ret=TRUE;}
    else{//modo debug, en esta version no es circular
    	*(s->push) = dato;
    	s->push++;
    	s->ncount++;ret=TRUE;}}
  if(s->push>s->tail){__asm(nop);//error de software o pila
  	  	  	  	  	  __asm(Halt);}
return ret;
}//fin de push en fifo------------------------------------------------------------



//regresa F si no se pudo insertar en la cola
static uint8 FIFO_volatile_int16_push_v1(int16_t dato,volatile struct _FIFO_volatile_int16_v1 *s){
unsigned char ret=FALSE;//no hay lugar, fifo llena
  if(s->ncount < s->size){ 
    if(s->mode==0){//normal
			*(s->push) = dato;
			if(s->push==s->tail) s->push=s->head;
			else{s->push++;}
			s->ncount++;
			ret=TRUE;}
    else{//modo debug, en esta version no es circular
    	*(s->push) = dato;
    	s->push++;
    	s->ncount++;ret=TRUE;}}
  if(s->push>s->tail){__asm(nop);//error de software o pila
  	  	  	  	  	  __asm(Halt);}
return ret;
}//fin de push en fifo------------------------------------------------------------








/* muestra el primer elemento que se va  a sacar en fifo */
unsigned char FIFO_general_1byte_peek_v2(struct _FIFO_1byte_v2 *s){
	if(s->mode==0){//modo normal
		return *(s->pop);}
	else{ return *(s->head);}
}//fin de peek the first data pointed to be fifo poped-------------




/* muestra el primer elemento que se va  a sacar en fifo */
static int16 FIFO_general_int16_peek_v1(struct _FIFO_int16_v1 *s){
	if(s->mode==0){//modo normal
		return *(s->pop);}
	else{ return *(s->head);}
}//fin de peek the first data pointed to be fifo poped-------------

/* muestra el primer elemento que se va  a sacar en fifo */
static int16 FIFO_volatile_int16_peek_v1(volatile struct _FIFO_volatile_int16_v1 *s){
	if(s->mode==0){//modo normal
		return *(s->pop);}
	else{ return *(s->head);}
}//fin de peek the first data pointed to be fifo poped-------------




//regresa 1 si lo encuentra
unsigned char FIFO_general_1byte_search_v2(unsigned char dato,struct _FIFO_1byte_v2 *s){
unsigned char *ptr;
	if(s->mode==0){//modo normal
	  ptr=s->pop;
	  switch(s->ncount){
		  case 0:return FALSE;
		  case 1:return(dato==*ptr)?TRUE:FALSE;
		  default:do{if(*ptr==dato)return TRUE;
                	 ptr++;
                	 if(ptr>=s->tail)ptr=s->head;}
		   	         while(ptr!=s->push);
		  	  	  	 return FALSE;}}
	else{return(busqueda2(s->head,s->ncount,dato))?TRUE:FALSE;}
return FALSE;
}//fin de search v2------------------------------------------------------------



//regresa 1 si lo encuentra
uint8 FIFO_general_int16_search_v1(int16 dato,struct _FIFO_int16_v1 *s){
int16  *ptr;
	if(s->mode==0){//modo normal
	  ptr=s->pop;
	  switch(s->ncount){
		  case 0:return FALSE;
		  case 1:return(dato==*ptr)?TRUE:FALSE;
		  default:do{if(*ptr==dato)return TRUE;
                	 ptr++;
                	 if(ptr>=s->tail)ptr=s->head;}
		   	         while(ptr!=s->push);
		  	  	  	 return FALSE;}}
	else{return(busqueda3(s->head,s->ncount,dato))?TRUE:FALSE;}
return FALSE;
}//fin de search v2------------------------------------------------------------

//regresa 1 si lo encuentra
uint8 FIFO_volatile_int16_search_v1(int16 dato,volatile struct _FIFO_volatile_int16_v1 *s){
volatile int16  *ptr;
	if(s->mode==0){//modo normal
	  ptr=s->pop;
	  switch(s->ncount){
		  case 0:return FALSE;
		  case 1:return(dato==*ptr)?TRUE:FALSE;
		  default:do{if(*ptr==dato)return TRUE;
                	 ptr++;
                	 if(ptr>=s->tail)ptr=s->head;}
		   	         while(ptr!=s->push);
		  	  	  	 return FALSE;}}
	else{return(busqueda_v_v4(s->head,s->ncount,dato))?TRUE:FALSE;}
return FALSE;
}//fin de search v2------------------------------------------------------------






unsigned char busqueda2(unsigned char *arr,unsigned char size,unsigned char dato){
	if(size==0)return 0;
	if(*arr==dato)return 1;
	return busqueda2(arr+1,size-1,dato);
}//fin algoritmo de busqueda----------------------------------------

static uint8 busqueda3(int16 *arr,uint8 size,int16 dato){
	if(size==0)return 0;
	if(*arr==dato)return 1;
	return busqueda3(arr+1,size-1,dato);
}//fin algoritmo de busqueda----------------------------------------


static uint8 busqueda_v_v4(volatile int16 *arr,volatile uint8 size,int16 dato){
	if(size==0)return 0;
	if(*arr==dato)return 1;
	return busqueda_v_v4(arr+1,size-1,dato);
}//fin algoritmo de busqueda----------------------------------------



// Función para solicitar RECURSO: transmision serial  hacia tarjeta display IO
// TRUE solamente si se encola el id que no esta en cola y esta libre el rec
/*  1.- si esta ocupado return FALSE
 *        1.1.-se revisa si ya esta en el turno sino se mete al turno
 *  2.- Si esta libre return TRUE y se asigna
 *  3.- si
 * */
unsigned char solicitarRecurso(unsigned char id) {	
const unsigned char MAX_TASK=2;//numero de tareas a manejar
static unsigned char Recurso;
unsigned char *r,dato;

    r=&Recurso;    //isREC   0x07 
    if(id==ID_vTask_isREC){return Recurso;}//comando para leer recurso
    if(id==ID_vTask_LIBERAR){Recurso=0;return 0;}
    switch(id){//verificacion de seguiridad
    	case ID_vTask_2:
    	case ID_vTask_3://Agregar abajo otra Tarea que necesita Recursos
    	case ID_vTask_10:break;
    	default:return FALSE;}
    if(Recurso>0){//recurso ocupado
    	if(recurso.COMM_IO.ncount<recurso.COMM_IO.size){
    		if(recurso.COMM_IO.ncount==0){
    			 recurso.COMM_IO.append(id,&recurso.COMM_IO);return FALSE;}
    	    if(recurso.COMM_IO.search(id,&recurso.COMM_IO)){
    	    	 return FALSE;}//ya esta en la cola
    	    else{recurso.COMM_IO.append(id,&recurso.COMM_IO);return FALSE;}}
    	else{return FALSE;}}
    else{if(recurso.COMM_IO.ncount==0){ //recurso=0, libre,, fifo vacia
     	 	  Recurso=id;return TRUE;}
    	 else{if(recurso.COMM_IO.dequeue(&dato,&recurso.COMM_IO)){
    		       Recurso=dato;return TRUE;}
    	 	  else{return FALSE;}}}//no hay fifo,
return FALSE;
}//--------------------------------------------------------------------------------------



// Función para liberar recurso
/* verifica si el recurso esta vacio  si lo esta sale return true recurso=0
 * si no esta vacio, verifica, si esta en fifo si lo esta lo quita sino no
 * busca si esta el recurso en fifo si lo esta lo quita.  */
unsigned char liberarRecurso(unsigned char id){
	if(solicitarRecurso(ID_vTask_isREC)==id){
	    solicitarRecurso(ID_vTask_LIBERAR);
		return TRUE;}
return FALSE;  // Recurso liberado, sin esperas
}//----------------------------------------------------------------




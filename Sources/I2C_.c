/*
 * I2C_.c
 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */
/*
 * I2C.c
 *
 *  Created on: Sep 3, 2015
 *      Author: chals Lara
 * para: GRACIDA SISTEMAS INDUSTRIALES 
 * proyecto: Tarjeta μProcesadora INSIGHT, para tarjeta version 2.     
 *      
 *  La comunicacion I2C se establece para 3 dispositivos que son:
 *  una memoria EEPROM
 *  un RTC y memoria NVRAM 
 *  y un acelerometro de freescale.
 *  
 *  EEPROM: Establecemos que, la memoria EEPROM sea para guardar datos de configuracion que se 
 *  mdifican cada que se modifica la configuracion. Las variables seran guardadas en EEPROM si cumplen con:
 *     1.-Necesitan guardar su valor aun con el equipo apagado
 *     2.-Solo el usuario puede cambiar estos valores.
 *  
 *  en la memoria NVRAM guardamos datos del sistema que se necesitan aunque se apague el
 *  sistemay pueden ser modificados en cualquier instante por lo que  se establece que las
 *  variables que tengan que ir en la NVRAM tendran que cumplir con:
 *      1.- Necesita guardar valor aunque se apague la tarjeta
 *      2.- El valor puede ser cambiado por el sistema en cualquier momento.
 *            
 * Por lo que las MEMORIAS  deberan ser gestionadas en tiempo real sin interrupcion de pormedio
 * desactivando la interrupcion I2C, 
 * EL acelerometro debe ser gestionado con interrupciones ya que es de menor prioridad y trabaja
 *  constantemente.
 *       
 *  Copyright ©2015 Gracida Sistemas Industriales    
 */


#include "I2C_.h"
//#include "PE_Types.h"
//#include "PE_Error.h"
//#include "PE_Const.h"
#include "IO_Map.h"
//#include "Cpu.h"
#include "I2C.h"
#include "delay.h"
#include "errorController.h"
#include "system.h"
#include "queue.h"
#include "Memoria.h"


#define PACIENCIA        5 //numero de veces que mandamos la transmision esperanzados que contesten el ACK
//#define NULL            0
#define DELAYI2CRX       1000//delay de i2c recepcion-> no MOVER ESTE VALOR
//#define DMA_SIZE_X         50//Memoria LEturas, Transferencias
#define DMA_SIZE         10   //filas
#define I2CFIFOSIZE        15//numero de items en la lista

#define CALENDARSIZE     8 //bytes para leer el calendario y el RTC
/* I2CR: IEN=0,IIEN=0,MSTA=0,MTX=1,TXAK=0,RSTA=0,??=0,??=0  00101 1000  */

#if(I2CBUFSIZE<10)
 #error "error de constante I2CBUFSIZE"
#endif

//unsigned char DMA[DMA_SIZE_X][DMA_SIZE_Y];



typedef union{//access word: 
	unsigned  short int wordx;   //   	0xaabb   
	unsigned char byteH;      //         aa
	unsigned char byte[2];        //byte[0]=aa,byte[1]=bb
}word16;



typedef union{//access word: 
	unsigned int  word4bytes;   //   	0xaabbccdd   
	unsigned char byte[4]; //byte[0]=aa,byte[1]=bb,byte[2]=c,byte[3]=d
}word32;





//Estructura para usar la memoria y del device y poder COMUNICARSE POR I2C
struct I2Ccom{
	unsigned char device;//direccion del dispositivo
	unsigned char ndata; //numero de bytes
	unsigned char operacion; //que tipo de operacion se va a realizar
    unsigned char buffer[I2CBUFSIZE];
   // unsigned char buffer2[I2CBUFSIZE];
    unsigned char count;
    unsigned char addH; //address high
    unsigned char addL;//address Low
    unsigned char head;//es el byte head el primer byte
};



struct I2Cst{//estructura de apuntadores a funciones ó digamos... una clase
	 void (*stop)(void); 
	 void (*start)(void);
	 void (*restart)(void);
	 void (*send)(unsigned char d);
	 void (*setMode)(unsigned char m);
	 unsigned char (*getMode)(void); 
	 void (*txACK)(unsigned char a);
	 void (*interrupts)(char e);
	 unsigned char sem;
}I2C;


struct I2C_FIFO{
	 unsigned char operacion;// WRITE || READ
	 unsigned char buffer[I2CBUFSIZE]; //BUFFER DE LECTURA
	 unsigned  long int Address;//direccion de comienzo
	 struct I2C_FIFO *next;
	 struct I2C_FIFO *prev;
	 unsigned char status;//bandera de estatus
	 struct I2Ccom device;
};

typedef union{
	unsigned long int word32;
	unsigned char byte[4];	
}i2cword32;


typedef struct I2Ccom I2Cs;
struct I2Ccom *device,ds1338,cat2401,mma8453;
unsigned char I2CdatosMode;
unsigned char datain[8];
unsigned char d1,d2,d3,d4,d5;
unsigned long int eaddg;


struct I2C_NODE   *i2chead,i2cFIFO[I2CFIFOSIZE],*i2ctail;
struct _I2C_CONTROL IIC;

void init_I2C(void){
//unsigned char i;	
  initI2C();//init  de registros del nucleo
  I2C.stop=I2Cstop;
  I2C.restart=I2Crepeatedstart;
  I2C.start=I2Cstart;
  I2C.send=I2Csend;
  I2C.setMode=setI2CMode;
  I2C.getMode=getI2CMode;
  I2C.txACK=txACK;
  I2C.sem=VERDE;
  I2C.interrupts=I2Cinterrupts;
  ds1338.device=DS1338;
  cat2401.device=CAT2401;
  init_I2C_FIFO();
  //for(i=0;i<I2CBUFSIZE;i++)
	//   I2Cidbuffer[i]=0;
  IIC.PushRX=IIC_FIFO_push_RX;
  IIC.PushTX=IIC_FIFO_push_TX;
  IIC.headRx=&(IIC.buffRX[0]);
  IIC.headTx=&(IIC.buffTX[0]);
  IIC.lasRx =&(IIC.buffRX[BUFFER_RX_IIC-1]);
  IIC.lasTx =&(IIC.buffTX[BUFFER_RX_IIC-1]);
  IIC.ncountRX=0;
  IIC.ncountTX=0;
  IIC.popRX=IIC_FIFO_pop_RX;
  IIC.popRx=IIC.pushRx=&(IIC.buffRX[0]);
  IIC.popTX=&IIC_FIFO_pop_TX;
  IIC.popTx=IIC.pushTx=&(IIC.buffTX[0]);
  IIC.device=0;//none device selected
  IIC.semaforo=VERDE;//se puede tranmitir
  IIC.estado=0;
  IIC.start=I2Cstart;
  IIC.Ack=ACK_IIC;
  IIC.stop=IIC_stop;
//  IIC.sendPack=II2_enviar_Paquete;
  IIC.delayCnt=0;
  init_FIFO_IIC();
}//fin init  IIC communications---------------------------------------------------------

void I2Cinterrupts(char e){//habilita o desabilita las interrupcion I2C
	 if(e==ENABLE) setReg8(I2CR,getReg8(I2CR)| 0x40U);
	 else setReg8(I2CR,getReg8(I2CR)& 0xBF);
}// FIN I2C interrupts enable disable


void setI2CMode(unsigned char m){
	 if(m==TRANSMITIR)//XXXN XXXX    MTX
		 setReg8(I2CR,getReg8(I2CR) | 0x10); //TRANSMITIR=0x10 
	 else {setReg8(I2CR,getReg8(I2CR) & 0xEF);} //RECIVIR=0xEF
	                      
}//fin I2C MODO DE TRANSMITIR RECIBIR---------------------------------

unsigned char getI2CMode(void){//
	if((getReg8(I2CR)& MTX)==MTX)
		return TRANSMITIR;
	else return RECIBIR;
}//fin i2c get mode ---------------------------------------------------



void clearflagIIF(void){
 //unsigned char i;	
	//i=getReg8Bits(I2SR) & 0xFD;
	setReg8(I2SR,getReg8(I2SR) & 0xFD);
	
}//fin clear flag IIF----------------------------------------------------


inline unsigned char isRXAK(void){//se recibio el RXAK, RXAK=0 an acknowledge signal was received after completion of 8-bit data transmition on the bus 
	//RXAK= no acknowledge was detected at the ninth clock
   	return(getReg8(I2SR) & 0x01);
}




void init_I2C_FIFO(void){  
unsigned char i;
      for(i=0;i<I2CFIFOSIZE;i++){
    	  if(i==0){
    	      i2chead=&i2cFIFO[0];
    	      i2ctail=i2chead;
    	      i2cFIFO[0].prev=&i2cFIFO[I2CFIFOSIZE-1];
    	      i2cFIFO[0].next=&i2cFIFO[1];
    	      i2cFIFO[0].id=NODO_FREE;}
    	  else{
    	  if(i==(I2CFIFOSIZE-1)){
    		  i2cFIFO[I2CFIFOSIZE-1].prev=&i2cFIFO[I2CFIFOSIZE-2];
    		  i2cFIFO[I2CFIFOSIZE-1].next=&i2cFIFO[0];
    		  i2cFIFO[I2CFIFOSIZE-1].id=0;}
    	  else{i2cFIFO[i].next=&i2cFIFO[i+1];
    	       i2cFIFO[i].prev=&i2cFIFO[i-1];
    	       i2cFIFO[i].id=0;}}}
}//FIN INIT fifo-----------------------------------

/*iNSertamos el request de operacion a alguna memoria
 * se mueve el head uno antes */
char i2cFIFOpush(unsigned long int eadd, unsigned char op,unsigned char id,unsigned char *data, unsigned char nd){
 unsigned char i,j;	
     if(i2chead->id==NODO_FREE)    
	      goto FIFOpush1;
     else{if(i2chead->prev->id==NODO_FREE){//subir head uno
               i2chead=i2chead->prev;
               goto FIFOpush1;}
          else return FALSE;}//la lista esta llena
          
FIFOpush1:     i2chead->address=eadd;
    	       i2chead->operacion=op;
    	       i2chead->ndata=nd;
    	       i2chead->id=id;
    	       if(nd<I2CBUFSIZE)
    	        	j=nd;
    	       else j=I2CBUFSIZE;
    	        for(i=0;i<j;i++)
    	          i2chead->data[i]=*(data+i);
    	       return TRUE;
} //fin i2c fifo push------------------------------------------------------------


/* saca LOS DATOS  que llegaron y tiene que tener la informacion 
 * del socket osea que aplicacion es esa informacion y debe ejecutar el programa
 * y procesar la informacion */
void i2cFIFOpop(struct I2C_NODE *i2cNodo){
 unsigned char i;
	 
          i2cNodo->address=i2ctail->address;
          for(i=0;i<I2CBUFSIZE;i++)
            i2cNodo->data[i]=i2ctail->data[i];
          i2cNodo->device=i2ctail->device;
          i2cNodo->id=i2ctail->id;
          i2cNodo->ndata=i2ctail->ndata;
          i2cNodo->operacion=i2ctail->operacion;
          i2ctail->id=NODO_FREE;
          if(i2chead==i2ctail)
        	   return;// i2cNodo;
          else {i2ctail=i2ctail->prev;
                return;}// i2cNodo;}
     
}//fin de hacer pop del i2c FIFO--------------------------------------


      


unsigned char I2Cread(void){
            while((getReg8(I2SR)& ICF)!=ICF){ //1:complete Transmission completa? 	 
    	          LedLoop();}
            return(getReg8(I2DR));
}//FIN read from I2c  bus----------------------------------------------------------------         



unsigned char *getCalendarRTC(void){//GET CALENDAR-NVRAM from RTC-- return pointer to array

	
}//fin get calendar and RTC from NVRAM-RTC----------------------------------------------------------




void saveClockNVRAM(unsigned char reg[]){

 reg[0]=0;

}//fin save clock NVRAM data-------------------------------------------------

unsigned char I2Ccomm(unsigned char device1){
	
	 if(I2C.sem==VERDE){     
		      I2C.sem++;// semaforo indica que el modulo I2C esta siendo usado
		      if(device1==DS1338)
		    	   device=&ds1338;
		      else device=&cat2401;
		      switch(device->operacion){
		       case SEQREAD:
		       case PAGEWRITE:                                //24M01
		       case READP: 
		       case WRITE: 	   I2C.start();
		                       I2C.interrupts(ENABLE);
		                       I2Csend(device->device);	                    	           
		                   break;//break ds1338 fin 2nd switch
		       
		          }//fin switch  
		 
	    return 0;}
	 else return WAIT;
}//fin transmitir




void txACK(unsigned char a){
  	
	if(a)
		setReg8(I2CR,getReg8(I2CR)| TXACK);
	else
		setReg8(I2CR,getReg8(I2CR)& ~TXACK); 
}



void I2Csend(unsigned char d){
	setReg8(I2DR,d);
}//FIN i2c send data---------------------------------

unsigned char I2Cget(void){
	
	 return getReg8(I2DR);
}//obtener el dato del buffer I2C-------------------


//unsigned char noACK(unsigned short int t){//cero para transmitir, 1ms para recibir en2401
	//if((getReg8(I2SR)& RXAK)==0){ //0:there is ack, vemos si hay reconocimiento sino salimos
	//	if(t>0)//muy importante  no quitar
//			delay_us(t);//este delay es super-importante costo 12 DIAS DE investigacion
//		return FALSE;}
//	else { LedErrorCommunicaciones(ERROR02_I2CCOMMFAILS);
//	       I2Cstop();//genera el STOP
//	       I2C.sem=VERDE;
//	       return TRUE;}
	          
//}//es el pulso acknowledge present-------------------------------------------------





void I2Cstart(void){
char i;	
	setReg8(I2CR,0x00);/* INIZIALIZACION SUPER IMPORTANTE*/
    setReg8(I2CR,0xA0);   /*****no*modificar******/
	i=getReg8(I2DR);//dummy read  *****no*modificar******
	setReg8(I2SR,0x00);   /*****no*modificar******/
	setReg8(I2CR,0x00);    /*****no*modificar******/
	setReg8(I2CR,0x80);/*TERMINA INIZIALIZAION SUPER IMPORTANTE*/
		             
		
	if((getReg8(I2SR)& 0x20)==0x20){//IBB xxNx xxxx ask if (IBB set)bus is busy 
		 setReg8(I2CR,getReg8(I2CR)| 0x20);  //if so, clears it signaling a STOP with  MSTA 1->0
	 	 setReg8(I2CR,getReg8(I2CR)& 0xDF);}//set to 1 then to 0 to generate STOP signal
	 setReg8(I2CR,getReg8(I2CR) | 0x10);//set transmit mode I2CR@MTX
	 setReg8(I2CR,getReg8(I2CR) | 0x20);//set master mode I2CR@MSTA this  generates a START condition
	
}// fin start i2c----------------------------------------------------------------

void I2Crepeatedstart(void){
	setReg8(I2CR,getReg8(I2CR)| RSTA);//Generate a repeated START by setting I2CR[RSTA].
}//fin repeated start----------------------------------------------------------------------


unsigned char pruebaI2C(void){
    const SIZE_ARR=9;	
    const ADD_GEN_PURPUSE=0x0A;
	unsigned char reg[SIZE_ARR]={1,2,3,4,5,6,7,8,9};//control=10010000
    unsigned char i;
	    
	      ds1338.ndata=SIZE_ARR;
	      ds1338.operacion=WRITE;  
	      for(i=1;i<SIZE_ARR;i++)
	             ds1338.buffer[i]=reg[i]; //datos a escribir apartir de la memoria pointer
	      ds1338.buffer[0]=ADD_GEN_PURPUSE;//direccion de memoriaa 
	      ds1338.count=0;
	      if(I2Ccomm(DS1338)==0)
	    		  return 1;
	      else 
	    	  return 0;
	      
	     	
}//fin prueba -------------------------------------



void pruebarxe2prom(void){
//unsigned char i;	
	   /*  cat2401.count=0;
	     cat2401.ndata=10;
	     cat2401.operacion=SEQREAD;
	     for(i=0;i<sizeof(cat2401.buffer2);i++)
	              cat2401.buffer2[i]=0;
	     I2Ccomm(CAT2401);
	  */
}//fin  prueba rx e2prom-----------------------------------------

unsigned char prueba2I2C(void){
	const SIZE_ARR=9;	
         ds1338.device=DS1338;
         ds1338.count=1;
         ds1338.ndata=SIZE_ARR;		  
         ds1338.operacion=READP;
         if(I2Ccomm(DS1338)==0)
        	 return TRUE;
         else
        	 return FALSE;
}// fin prueba I2C --------------------------------------------




/* lee la eeprom   nbytes: numero de bytes a escribir  add1:direccion de inicio de escritura*/
//unsigned char *readEEPROM(unsigned long int add1,unsigned char nbytes){
//Dword add;   funcion deprecated
      //if(I2C.sem!=VERDE)
        //  delay_ms(100);  //esperamos que acabe de recibir para devolver el dato
      //if(I2C.sem==VERDE){ 
       //  add.Dwordx=add1;//poder separar el int32 en 4 bytes
	    // cat2401.count=0;
	    // cat2401.ndata=nbytes;
	    // cat2401.buffer[0]=add.byte[2];// LSB
	    // cat2401.buffer[1]=add.byte[3];//MSB
	    // if((add.byte[1]&0x01)==0x01)
	   // 	 cat2401.device=CAT2401|0x20;//bit a16, ESTE CHIP TIENE address externa fijada a 00
	   //  else cat2401.device=CAT2401;
	   //  cat2401.operacion=SEQREAD;
	   //  while(I2Ccomm(CAT2401)==WAIT) 
	  //  	      __asm{ nop };// Se manda a procesar la recepcion sino esperamos que se libere el recurso
                  
	  //   I2C.sem++; //VOLVEMOS A apartar EL MODULO PARA transferir los datos,s
	  //   delay_ms(300);
	  //   return(&cat2401.buffer2[0]); //regresamos el apuntador de recpcion, el primer dato esta en 1, es por un bug no resuelt
      //   }
//}// fin read memory eeprom for variables and reports.-------------------------------------------



void I2CliberarModulo(void){ // IMPORTANTE !!! va unas INSTRUCCIONES DESPUES DE readEEPROM y creo tambien de la otra memoria
	I2C.sem=VERDE;
}//fin de liberar modulo de I2C desde otra unidad de compilacion

unsigned char *getPointerI2CBuffTx(unsigned char device1){
	    switch(device1){
	      case CAT2401:return(&cat2401.buffer[2]);break;
	      case DS1338: return(&ds1338.buffer[2]);break;
	      case MMA8453:return(&mma8453.buffer[2]);break;
	      default:break; 	  
	       }//fin switch
}//fin get pointer buffer transmisor de I2C

unsigned char *getPointerI2CBuff(unsigned char device1){
	    switch(device1){
	      case CAT2401:return(&cat2401.buffer[0]);break;
	      case DS1338: return(&ds1338.buffer[0]);break;
	      case MMA8453:return(&mma8453.buffer[0]);break;
	      default:break; 	  
	       }//fin switch
}//fin get pointer buffer transmisor de I2C


/* lee la NVRAM  nbytes: numero de bytes a escribir  add1:direccion de inicio de escritura*/
unsigned char *readNVRAM(unsigned char add1,unsigned char nbytes,unsigned char id){
unsigned char *p=0;
         
	     /*  NVRAM have  00 to 07h  and RAm spacce to 08h to 3Fh,  */
	     i2cFIFOpush(add1,DS1338_NVRAM_READ_SLAVE_RX_TX,id,p,nbytes);
}// fin read memory NVRAM for variables --------------------------------------------------------------


void writeNVRAM(unsigned char *p,unsigned char address1,unsigned char nbytes){	
//Dword add;
unsigned char i,j;
          //add.Dwordx=address1;
          ds1338.count=0;
          ds1338.ndata=nbytes+1;//mas 1 por el 1 byte de direccion
          ds1338.buffer[0]=address1;
          //cat2401.buffer[1]=add.byte[3];
          for(i=1,j=0;i<ds1338.ndata;i++,j++)
        	  ds1338.buffer[i]=*(p+j);//
          ds1338.device=DS1338;
          ds1338.operacion=WRITE;
          while(I2Ccomm(DS1338)==WAIT) //se manda procesesar el dato para mandarlo, esperamos que termina de emitir la cola la interrupcion
        	            __asm{ nop };
          while(I2C.sem>VERDE) __asm{ nop};//espera que se manden los da
          
}// fin write eeprom---at address, and write from pointer *p----------------------------------

void pruebatxe2prom(void){
unsigned int i;
unsigned char j;
	/*unsigned char reg[]={0,0,0x11,0x14,0x15};
	   cat2401.ndata=sizeof(reg);
	   cat2401.operacion=PAGEWRITE;
	   for(i=0;i<sizeof(reg);i++){
	   	             cat2401.buffer[i]=reg[i];
	   	             cat2401.buffer2[i]=0;}	   
	   I2Ccomm(CAT2401);*/
       initI2C();
       for(i=0,j=0;i<300;i++,j++){
        writeEEPROMbyte(i,j);
        delay_ms(1200);}
    
}//fin pruebatx e2prom-----------------------------



void ControladorMaestroEEPROM(void){//debug  ya no necesito esto y tu?
//static unsigned long int i,j;
  //  if(i++>20000){i=0;
    //  if(j<10) 
    	//  j=10;
	      //readEEPROMbyte(j++);}
}//fin de controlador maestro de la memeoria EEPROM 



void initI2C(void){
	  setReg8(I2CR, 0x60U);   //primero se configura este              
	  /* I2FDR: ??=0,??=0,IC=0x12 */
	  setReg8(I2FDR, 0x1FU);//para 20Mhz  0F=240 div 1FU:104khz,   12U:10KHZ                
	  /* I2ADR: ADR=0x1C,??=0 */
	  setReg8(I2ADR, 0x38U);                
	                
	  /* I2CR: IEN=1 */
	  setReg8Bits(I2CR, 0x80U);//encendemos modulo
	  delay_ms(100);//sin estos delay no funciona
	  setReg8(I2CR, 0xD0U);   //1101 0000 hacemos un STOP-> IBB=0
	  /* I2SR: ICF=1,IAAS=0,IBB=0,IAL=0,??=0,SRW=0,IIF=0,RXAK=1 */
	  delay_ms(100);//sin estos delay no funciona
	  setReg8(I2SR, 0x81U);
	  delay_ms(100);//sin estos delay no funciona
}

void writeEEPROMbyte(unsigned long int eadd, unsigned char data){
i2cword32 w32;
   if(I2C.sem==VERDE){
	 setReg8(I2CR,0xD8);//1101 1000   
     w32.word32=eadd;
     cat2401.count=0;
     cat2401.device=CAT2401;
     cat2401.ndata=1;
     cat2401.operacion=BYTEWRITE;
     cat2401.buffer[0]=data;
     cat2401.addH=w32.byte[2];
     cat2401.addL=w32.byte[3];
     device=&cat2401;
     cat2401.head=getI2CdevAddress(CAT2401,eadd);
     setReg8(I2SR, 0x81U);
     beginTransmissions(cat2401.head);
     delay_ms(1); //tiempo que tarda la transaccion
}}

/*first byte    A1 A2 A3 A4  D2  D1 a16  R/W =A1..A4 direccion del dispositivo
          D2  D1  direccion fisica del dispositivo
          a16    BIT 16 de direccion logica de memoria de 1MB
          R/W  bit de escritura=0   lectura=1*/
unsigned char getI2CdevAddress(unsigned char Device, unsigned long int e2promAdd){
i2cword32  w32;
        
    switch(Device){//obtenemos el primer byte, el byte head en modo escritura
      case CAT2401:w32.word32=e2promAdd;
                   if((w32.byte[1]&0x01)==0x01)//bit a16
        	             return(0xA2);
        	       else  return(0xA0);
   	               break;
      default:break;}              
		
}//fin get i2c device address te first byte get several information

void beginTransmissions(unsigned char add){
	  I2C.sem=VERDE+1;
	  start();
	  I2C.send(add);
     // I2Cwrite(add);
}

void I2Cwrite(unsigned char data){
	I2C.send(data);
	while(noACK())
		__asm(nop);
}



void start(void){	
unsigned char i;
	  i=getReg8(I2SR);
	  if((i&0x20)==0x0)//bus is  idle 
		  setReg8(I2CR,getReg8(I2CR) | 0x20);//set master mode I2CR@MSTA this  generates a START condition
}




unsigned char noACK(void){//cero para transmitir, 1ms para recibir en2401
	
	if((getReg8(I2SR)& RXAK)==0) 
		return FALSE;
	else {return  TRUE;
          I2Cstop();
	       }
}//es el pulso acknowledge present-------------------------------------------------

//funcion que trabaja con sistea operativo
//ackonolegde despues de mndar una byte 
unsigned char ACK_IIC(void){
	if((getReg8(I2SR)&RXAK)==0)
		return TRUE;//SI HAY ACK
	else return FALSE;//NO HY acknowledge
}



void I2Cstop(void){
	 setReg8(I2CR,getReg8(I2CR) & 0xDF);//genera el STOP
	 //I2C.interrupts(DISABLE);
//	 I2C.sem=VERDE;
//	 device->operacion=0;
//	 delay_ms(5);
}//fin STOP signal I2C 


//genera el stop de I2C PERO ESSTE es para sistema operativo
void IIC_stop(void){
	 setReg8(I2CR,getReg8(I2CR) & 0xDF);//genera el STOP
}


PE_ISR(iif)//ESTE PROCESADOR SOLO ESTA EN MASTER MODE
{
	clearflagIIF();
 if(I2C.sem!=VERDE){
    switch(device->operacion){//switch1
    	  case BYTEWRITE: if(noACK()) return;
    	                  I2C.send(device->addH);
    	                  device->operacion=BYTEWRITE1;
    	                  break;
    	  case BYTEWRITE1:if(noACK()) return;   
    	                  I2C.send(device->addL);
    	                  device->operacion=BYTEWRITE2;
    	                  break;
    	  case BYTEWRITE2:if(noACK()) return;
    	                  I2C.send(device->buffer[0]);
    	                  device->operacion=BYTEWRITE3;
    	                  break;
    	  case BYTEWRITE3:if(noACK()) return;
    	                  I2Cstop();
    	                  break;//fin byte read
    	  case BYTEREAD:  if(noACK()) return;
    	                  I2C.send(device->addH);
    	                  device->operacion=BYTEREAD1;
    	                  break;
    	  case BYTEREAD1: if(noACK()) return;
    	                  I2C.send(device->addL);//segundo byte de direccion
    	                  device->operacion=BYTEREAD2;
    	                  break;
    	  case BYTEREAD2: if(noACK()) return;
    	                  setReg8(I2CR,0xFC);//1111 1100  repeated start
    	                  I2C.send(device->head|0x01);//byte head | R/W=1
    	                  device->operacion=BYTEREAD3;
    	                  break;//
    	  case BYTEREAD3: if(noACK()) return;
    	                  d1=0xAA;d2=0xBB;d3=0xCC;
    	                  setReg8(I2CR,0xE0);//1110 0000   //I2C.setMode(RECIBIR);
    	                  delay_ms(1);
    	                  ///d1=getReg8(I2DR); //__asm(nop);/// importane este nop, PARA QUE descargue el valor en el registro, el valor que recibio
    	                  delay_us(300);
    	                  d1=getReg8(I2DR);
    	                  delay_us(300);
    	                  d2=getReg8(I2DR);
    	                  delay_us(300);
    	                  d3=getReg8(I2DR);
    	                  delay_us(300);
    	                  do{device->buffer[device->count++]=getReg8(I2DR);//registro de Direccion 0'
    	                     delay_us(300);}
    	                  while(device->count<device->ndata);  
    	                  device->operacion=BYTEREAD4;
    	                  break;
    	  case BYTEREAD4: I2C.sem=VERDE;            
    	                  I2Cstop();// FIN READ BYTE
    	                  break;
    	                  
    	  default: I2Cstop();
    	           break;
    	                  
    	                  
       	
       }}//fin switch

}// ENDS pe_isr(iif)-----------------------------------------------------------------


void I2C_polling_ISR(void)//ESTE PROCESADOR SOLO ESTA EN MASTER MODE
{ if((getReg8(I2SR)& 0x02)==0x02){ //hay bandera?
	clearflagIIF();
    switch(device->operacion){//switch1
    	  case BYTEWRITE: if(noACK()) return;
    	                  I2C.send(device->addH);
    	                  device->operacion=BYTEWRITE1;
    	                  break;
    	  case BYTEWRITE1:if(noACK()) return;   
    	                  I2C.send(device->addL);
    	                  device->operacion=BYTEWRITE2;
    	                  break;
    	  case BYTEWRITE2:if(noACK()) return;
    	                  I2C.send(device->buffer[0]);
    	                  device->operacion=BYTEWRITE3;
    	                  break;
    	  case BYTEWRITE3:if(noACK()) return;
    	                  I2Cstop();
    	                  break;
    	                  
       	
       	}}//fin switch

}// ENDS pe_isr(iif)-------------------------------------------------------------------------------

/* COntrol de los sockets de I2C
// * */
//unsigned char I2Csockets(unsigned char id,unsigned char contexto,unsigned char op){
//const unsigned char I2C_SOCKETS_SIZE=10;//10 sockets en pila	
//const unsigned char I2C_Y=2,ID_COL=0,CONTEXT_COL=1;
//static unsigned char sockets[I2C_SOCKETS_SIZE][I2C_Y];//debug este numero deberia ser como 15
//unsigned char i;
//	    
//	switch(op){
//     case GET:for(i=1;i<I2CBUFSIZE;i++){//obtenemos un id libre
//		         if(sockets[i][ID_COL]==0){
//		        	 sockets[i][ID_COL]=BUSY;
//			         sockets[i][CONTEXT_COL]=contexto;
//			     return i;}}//RETURN el id seleccinado
//	          return 0;//esta lleno el array
//    	      break;
//     case FREE: sockets[id][ID_COL]=0; //liberamos un id
//                sockets[id][CONTEXT_COL]=0;
//                return 0;
//                break;
//     case WRITE:
//    	        break;
//    	 
//     default: break;
//	}//fin switch
//		
//	
//}//fin del controlador maestro de los sockets de I2C----------------------------------




//void ControladorOperativoComunicacionI2C(void){
//static unsigned char Autoinit; /// 12c communications Operative systems 	
//static struct I2C_NODE i2cOSnode,*np;
//static unsigned char state;
//unsigned char i;
//      if(Autoinit==0xAA)
//      	 goto I2Cn1;
//      else  {Autoinit=0xAA;
//             state=FREE;}
//I2Cn1:  if(state==FREE){//Revisar si hay una operacion en Proceso
//       if(i2ctail->id==NODO_FREE)  //Revisar si hay algo en la fifo
//   	          return;			
//       else {i2cFIFOpop(&i2cOSnode);// sacar  un nodo
//             goto I2Cinit;}}
//       else {   goto  I2CstateMachine;  }//ya hay un proceso en curso
	    	 
         
//I2Cinit:      //  I2CclearLEDerror();
//              switch(i2cOSnode.operacion){//switch1  OPERATIONS CHOICE SELECTION STATE-MACHINE
//              case DS1338_NVRAM_READ_SLAVE_RX_TX://lets READ NVRAM DS1338  write pointer then read, Slave Receive TRansmit
//            	     state=I2C_OS1;//estado uno
//            	     return;
//            	     break;//fin DS1338_NVRAM_READ_SLAVE_RX_TX
//               default:goto I2Cerror;break;}//fin switch1      

//I2Cerror:       if(state!=FREE){
//              	I2Csockets(i2cOSnode.id,0,FREE);//liberamos el socket
//              	state=FREE;}//liberamos la maquina de estados
                //I2CsetLEDerror(); 
//              return;
                
                
                
                //I2CstateMachine:switch(state){//STATE-MACHINE          
	// case I2C_OS1:     //DS1338 NVRAM, write pointer then read NVRAM
	    	//  if((i&IBB)==IBB)//bus is busy?
	    	        	 // return;//waits until it is clear
	    	         // i=MTX;//set as transmit mode by settint I2CR[MTX]
	    	         
	    	         //i=getReg8(I2CR); //Revision de estado
                     // if((i&IEN)!=IEN)//habilitado modulo?
                    	 // goto I2Cerror;
                     //   if((i&IIEN)!=IIEN)//habilitado las interrupcinoes      
                    	 //  goto I2Cerror;
                     //  if((i&MTX)!=MTX)
                    	 // goto I12Cerror;
                     //i=getReg8(I2SR);
                     //if((i&ICF)!=ICF)//transfer in progress?
                    	 // return; //esperemos que termine la transferencia
                     //if((i&IBB)==IBB)//bus is busy
                     // goto I2Cerror;
                     //if((i&IAL)==IAL)//ARbitration LOSS
                    	 //  goto I2Cerror;
                     //state=I2C_OS2;
                     //return;break;
                     //	    //case I2C_OS2: break;
                     
//	    default:break;          
     //     }//fin STATE-MACHINE
	    	 

//}//fin CONTROLADOR PRINCIPAL OPERATIVO DE COMUNICACIONES i2c---------------------------------------


/* DRIVER CONTROLADOR OPERATIVO PRINCIPAL DE LA GESTION MAESTRA CENTRAL 
 * DE LAS COMUNICACIONES I2C    * */
//Controlador Maestro en el Nucleo Central del  
//del sistema Operativo de la comunicacion
//Inter-Integrated Circuit Communication
void Controlador_Principal_Central_Communicacion_IIC(void){
	
	if(IIC.semaforo==ROJO){
	   if(IIC.ncountRX)
	       Controlador_Operativo_de_Recepcion_IIC();
	   else control_Transito_IIC();}
	else Controlador_Operativo_de_Transmision_IIC();
	
}//fin Controlador_en_Nucleo_Central_Operativo_comm_IIC-----
//----------------------------------------------------------

/* hace control de transito del bus IIC*/
void control_Transito_IIC(void){
	 IIC.ncountRX=0;
	 IIC.semaforo=VERDE;
	 if(IIC.popRx==IIC.pushRx)
		 return;
	 else {IIC.popRx=IIC.pushRx=IIC.headRx;
	       __asm(Halt);}//verificar algoritmo.
	
}//control_Transito_IIC-------------------------


/*controlador operativo maestro principal
 * del manejador central del nucleo del sistema General
 *  de la comunicacion de REcepcion IIC */
void Controlador_Operativo_de_Recepcion_IIC(void){
	
}// Controlador_Operativo_de_Recepcion_IIC


/*controlador operativo maestro principal
 * del manejador central del nucleo del sistema General
 *  de la comunicacion de Transmision IIC 
 *  protocolo
 *  STX,DEVICE,LEN,APP,CMD,PARAM0,..,PARAMn,ETX*/
void Controlador_Operativo_de_Transmision_IIC(void){
static unsigned char nbytes;
 int i;
 unsigned char j;
  switch(IIC.estado){//get paquete regresa sin STX
	  case e0: if(IIC.ncountTX){  
				 if(get_Paquete(&(IIC.paquete[0]),IIC.popTX)) //sacamos el paquete a transmitir 
					 IIC.estado=e1;
			     else  IIC.estado=e0;} //error de protocolo
               break;
	  case e1:nbytes=procesador_Paquete_IIC(&(IIC.paquete[0]));
	          IIC.estado=e2;break;
	  case e2://IIC.start();
	          IIC.estado	=e3;break;
	  case e3:for(i=0,j=0; ;i++,j++){
		         IIC.start();
		         delay_us(250);
		         I2C.send(0x17);//0001 0111
		         delay_us(250);
		         I2C.send(0x55);//0101 0101
		         delay_us(250);
		         I2C.send(j);
		         delay_us(250);
		         I2C.stop();
		         delay_ms(1);
	               }  
		      I2C.send(IIC.paquete[0]);
	          IIC.nbytes=1;
	          IIC.estado=e4;
	          IIC.delayCnt=0;
	          break;
	  case e4:if(IIC.delayCnt>15){
		            //Memo_Reporte(D_ERROR,SUB_EV_UP,IIC_NO_ACK);
		            IIC.estado=e0;
		            I2C.stop();
		            break;}
		      if(IIC.Ack()) 
		          IIC.estado=e5;
	          break;
	  case e5:if(IIC.nbytes>nbytes){
		          IIC.stop();
	              IIC.estado=e0;
	              I2C.stop();
	              //if(interupccionRX)
	            	///   IIC.semaforo=rojo;
	             // else
	              IIC.semaforo=VERDE;
	              break;}
	          else
		         I2C.send(IIC.paquete[IIC.nbytes++]);
	          IIC.estado=e4;
	          break;
	  default:break;        
   }//FIN MAQUINA de estados de IIC tx
}// Controlador_Operativo_de_Transmision_IIC


/* el array donde guarda el paquete con protocolo
 * lo modifica para que tenga en index=0 la direccion 
 * y en lo demas los bytes exactos a transmitir a ese device
 * protocolo, aqui y no trae el STX 
 * DEVICE,LEN,APP,CMD,PARM0,..,PARAMn,ETX
 * regresa los bytes a enviar
 * */
unsigned char  procesador_Paquete_IIC(unsigned char *p){
const unsigned char DEVICE=0,LEN=1,APP=2,CMD=3;	
unsigned char a[PACK_SIZE],i;
	 
	for(i=0;i<4;i++)
	   a[i]=*(p+i);
	switch(a[APP]){
		case CAMBIO_FRECQ_APP:
			    if(a[CMD]==NUEVA_FRECQ_CMD){
			        *(p+0)=a[DEVICE];
			        *(p+1)=*(p+4);
			        *(p+2)=*(p+5);
			        *(p+3)=*(p+6);
			        return 3;}//tres bytes  a enviar
		      break;	        
		              
			
		default: break;}		
			
return 0;
}//fin procesador_Paquete_IIC----------------------------


/* cambio de  frecuencia usando I2C se le mete un protocolo
 * al l fifo y luego el sistema operativo lo lee y envia mensaje
 * procesado*/
void Cambio_de_Frecuencia_por_IIC(unsigned short int f){
unsigned char i;
const unsigned char len1=6;
const unsigned char Prog=CAMBIO_FRECQ_APP;
const unsigned char cmd=NUEVA_FRECQ_CMD;
//STX,DEVICE,LEN,PROGRAM,CMD,PARAM0
unsigned char a[9]={STX,ARDUINO,len1,Prog,cmd};

word32 longword;
       
       longword.word4bytes=1000*(unsigned int)(f);
       a[5]=longword.byte[1];
       a[6]=longword.byte[2];
       a[7]=longword.byte[3];
       a[8]=ETX;     
       for(i=0;i<(len1+3);i++)
    	     IIC.PushTX(a[i]);
         
	
}//fin Cambio_de_Frecuencia_por_IIC----------------------------


void I2C_Monitor (void){
int i=0;	
	IIC.start();
	for(i=0;i<5000;i++)
	  I2C.send(0x00);
	I2C.stop();
}//fin i2c monitor-----------------------




//void II2_enviar_Paquete(unsigned char *p,unsigned char len){
//unsigned char i;	
	
//    for(i=0;i<len;i++)
//	    IIC.PushTX(*(p+i));
//	    
	

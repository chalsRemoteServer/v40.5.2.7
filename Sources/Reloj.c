/*
 * Reloj.c
 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */


/*
 * Reloj.c
 *
 *  Created on: Jul 20, 2015
 *      Author: chals
 */


#include "Reloj.h"
#include "I2C_.h"
#include "Memoria.h"
#include "RTC1.h"
#include "system.h"
//#include "VFDmenu.h"
#include "VFDisplay.h"

#define NVRAM_READ_SIZE    56 //numero de byte que extraemos de la NVRAM  cuando la leemos

typedef union{//access word: 
	unsigned  long int Dwordx;   //   	0xffxxaabb    capacidad: 131071 bytes=1FFFFH=
	//unsigned char byteH;      //         ff
	unsigned char byte[4];        //byte[0]=ff,byte[1]=xx,byte[2]=aa,byte[3]=bb;
}Dword;


struct Relojst{
	void (*setYear1)(unsigned char n);
	void (*setMonth1)(unsigned char n);
	void (*setDate1)(unsigned char n);
	void (*sethora1)(unsigned char n);
	void (*setminutos1)(unsigned char n);
	void (*setsegundos1)(unsigned char n);
	unsigned char (*gethora1)(void);
	unsigned char (*getYear1)(void);
	unsigned char (*getMonth1)(void);
	unsigned char (*getDate1)(void);
	unsigned char (*getminutos1)(void);
	unsigned char (*getsegundos1)(void);//tinen que tener un nombre diferente no identico
	
	
}Reloj; //no funciona marca error de cpu


/* VARIABLES DEL TIEMPO */
unsigned char year,mes,dia,fecha,hora,minutos,segundos,control;
//unsigned char rtc[7];//el orden es:año,mes,dia,hora,minutos,dia-semana,segundos
unsigned char segunderoReloj;



void init_Reloj(void){
	RTC1_Enable();//habilitar RTC
	
	Reloj.setDate1=setDate;
	Reloj.setMonth1=setMonth;
	Reloj.setYear1=setYear;
	Reloj.sethora1=sethora;
	Reloj.setminutos1=setminutos;
	Reloj.setsegundos1=setsegundos;
	Reloj.getDate1=getDate;
	Reloj.getMonth1=getMonth;
	Reloj.getYear1=getYear;
	Reloj.gethora1=gethora;
	Reloj.getminutos1=getminutos;
	Reloj.getsegundos1=getsegundos;
}//fin init reloj--------------------------------------------------------------


void readclock(void){//traemos de la NVRAM  del relog las variables del tiempo
unsigned char *p,control;	 
	  p=getCalendarRTC();
      segundos=*p;
      minutos=*(p+1);
      hora=*(p+2);
      dia=*(p+3);
      fecha=*(p+4);
      mes=*(p+5);
      year=*(p+6);
      control=*(p+7);
 //     p=readNVRAM(0,NVRAM_READ_SIZE);
      
}//lee los valores de las variables del tiempo del reloj del sistema

unsigned char  gethora(void){ 
Dword Time;	
	    Time.Dwordx=getReg32(HOURMIN);
		return(Time.byte[2]); //pag 139. 2byte saves the hour
	}//fin get hora---------------------------------------
unsigned char getYear(void){return(year); }
unsigned char getMonth(void){return(mes);}
unsigned char getDate(void){return(fecha);}
unsigned char getminutos(void){
	Dword Time;	
		    Time.Dwordx=getReg32(HOURMIN);
			return(Time.byte[3]); //pag 139. 2byte saves the hour
}//fin get minutos------------------------------------------
unsigned char getDay(void){return(dia);}
void setDay(unsigned char d){dia=d;}

unsigned char getControl(void){return(control);}
void setControl(unsigned char c){control=c;}

void setYear(unsigned char n){
	           year=n;}
void setMonth(unsigned char n){
	           mes=n;}
void setDate(unsigned char n){
	           fecha=n;}//funcion encapsulada que suma 1
void sethora(unsigned char n){
	Dword Time;
	     Time.Dwordx=getReg32(HOURMIN);
	     Time.byte[2]=n;
	     setReg32(HOURMIN,Time.Dwordx);
}//fin set hora

void setminutos(unsigned char n){
Dword Time;
	     Time.Dwordx=getReg32(HOURMIN);
	     Time.byte[3]=n;
	     setReg32(HOURMIN, Time.Dwordx);
}//fin set minutos

void saveClockMoficado(void){//guarda los datos del relojNVRAM porque se modificaron
unsigned char clock[NVRAMREGCONTRLSIZE];	 
     clock[0]=0x00;//segundos
     clock[1]=convertDate(minutos,_MIN_);
     clock[2]=convertDate(hora,_HORA_);
     clock[3]=convertDate(0,_DIAS_);//dias de la semana domingo a lunes
     clock[4]=convertDate(fecha,_DIA_);
     clock[5]=convertDate(mes,_MES_);
     clock[6]=convertDate(year,_YEAR_);//ya estan listas para mandarlas a la NVRAM
     clock[7]=0x90; //10010000 Registro de Control de la NVRAM
	 saveClockNVRAM(clock);
	
}// fin save clock modificado----------------------------------



unsigned char convertDate(unsigned char n,unsigned char tipo){//convert to format NVRAM1338 registers BCD
unsigned char cent=0,dec=0,uni=1;
	switch(tipo){
	  case _MIN_:while(--n>0){if(++uni>9){uni=0;
                               if(++dec>5){dec=0;}}}
	             dec |=0x40; //activacion principal del oscilador antes del shift para que quede 0x80
	             break;
	  case _HORA_:while(--n>0){if(++uni>9){uni=0;
                                if(++dec>12){dec=0;}}}	 
	              dec &=0x03; //desactivacion de 12 horas y activacion de 24 horas
	              break;
	  case _DIAS_: dec=0;uni=1;break;
	  case _DIA_:while(--n>0){if(++uni>9){uni=0;
                              if(++dec>31){dec=0;}}}
	             break;
	  case _MES_:while(--n>0){if(++uni>9){uni=0;
                              if(++dec>12){dec=0;}}}  
	             break;
	  case _YEAR_:while(--n>0){if(++uni>9) {uni=0;
                              if(++dec>9){dec=0;
                               if(++cent>2){cent=0;}}}}
	             break;
	  default:break;}//fin switch
	  
	  dec<<=4;
return(dec|uni);
}//fin convert datte to format register ds1338------------------

void displaySegundero_IRQ(void){//interrupts every 200ms------------------------------------------
char count;
   if(segunderoReloj==ACTIVADO){
	       count=displayTime(_SEG_);
	       if(count==0){
	    	      count=displayTime(_MIN_);
	    	      if(count==0)
	    	    	  displayTime(_HORA_);}
          }//fincount...........................................................
}//fin display segundero en cuando estamos en el conteto de config del reloj

void segundero(unsigned char status){
     segunderoReloj=status;
	 if((segunderoReloj!=ACTIVADO)&&(segunderoReloj!=DESACTIVADO)){//Proteccion contra-errores
		 segunderoReloj=DESACTIVADO;}
}//fin activar o no el segundero--------------------------------

unsigned char getsegundos(void){
	Dword Time;	
		    Time.Dwordx=getReg32(SECONDS);
			return(Time.byte[3]); //pag 139. 
}//fin get segundos--------------------------------------------

void setsegundos(unsigned char s){
	 Dword Time;
	       Time.byte[3]=s;
	       setReg32(SECONDS,Time.Dwordx);
}//fin set segundos -----------------------------------------

void getTimeNVRAM(void){//sacamos los 
unsigned char *p=0;
       //p=readNVRAM(0,8);//reads just the time variables	  
	   setsegundos(convertBCD2Binary(*p));
	   setminutos(convertBCD2Binary(*(p+1)));
	   sethora(convertBCD2Binary(*(p+2)));
	   setDay(convertBCD2Binary(*(p+3)));
	   setDate(convertBCD2Binary(*(p+4)));
	   setMonth(convertBCD2Binary(*(p+5)));
	   setYear(convertBCD2Binary(*(p+6)));
	   setControl(*(p+7));
       I2CliberarModulo();
}//FIN  get time from NVRAM and Pours into embedded TIME vars--------------------------------------

void saveTimeNVRAM(void){
unsigned char t[8],*p;
     t[0]=convertBinary2BCD(getsegundos()) & 0x7F; //desactivamos el bit 8=halt clock
     t[1]=convertBinary2BCD(getminutos());
     t[2]=convertBinary2BCD(gethora());
     t[3]=1;
     t[4]=convertBinary2BCD(getDate());
     t[5]=convertBinary2BCD(getMonth());
     t[6]=convertBinary2BCD(getYear());
     t[7]=0x10;    //0001 0000
     p=&t[0];
	 writeNVRAM(p,0,8);//escribir a la NVRAM apartir de la dir0  8 bytes
}//fin save time NVRAM----------------------------------------------------------------------------

unsigned char convertBinary2BCD(unsigned char n){
unsigned char uni=0,dec=0;
         if(n==0) return 0;
         while(n!=0){
        	  if(++uni==10){
        		  uni=0;
        	      ++dec;}
        	  --n;}
	     dec<<=4;
	     return(dec|uni);
	return 0;//<<--quitar
}//fin convert binary to BCD-----------------------------------------------


unsigned char  convertBCD2Binary(unsigned char n){//convertir dos BDC's en un byte en un byte binario
unsigned char i;
            i=n&0x0f;
            n&=0xf0;
            n=(n>>4)*10+i;
            return(n);
}//fin convert BCD to Binary-----------------------------------------------------------------

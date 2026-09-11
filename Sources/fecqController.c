/*
 * fecqController.c
 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */


/*
 * fecqController.c
 *
 *  Created on: Jul 27, 2015
 *      Author: chals
 *      
 *  Los parametros para configurar el puerto QSPI del procesador usando los
 *  chips multiplexores y los inversores del circuto IN1UP tal cual para multiplear la 
 *  señal de reloj hasta el DDS@AD9833 son los siguientes
 *  MSB-FIRST:YES++++++++++++++++++++++++++++++++
 *  CLOCK EDGE:FALLING EDGE+++++++++++++++++++++
  * SHIFT CLOCK IDLE POLARITY= LOW++++++++++++++++
  * SHIFT-CLOCK=12.7useg++++++++++++++++++++++++
  * QSPI_DOUT-17++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  * QSPI_SCK_21+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  * 
 */

#include "frecqController.h"
#include "Memoria.h"
#include "errorController.h"
#include "FSYNC.h"
#include "SDATA.h"
//#include "SCLK.h"
#include "VFD.h"
#include "SM1.h"
#include "delay.h"
#include "CS0.h"
#include "CS1.h"
#include "ADC.h"
#include "system.h"
#include "ADC_BAL.h"
#include "CLK_AN_COUNT.h"


#ifndef  DDS_GENERADOR
  #define  ARDUINO_DDS 0x1A
  #define  AD9834_DDS  0x2A
  #define  DDS_GENERADOR AD9834_DDS  //ARDUINO_DDS
#endif


#define CUADRADA       12
#define SINUSOIDAL     33
#define XTAL_ANALOGA   24576000//FRECUENCIA DEL CRISTAL DE LA ANALOGA INSGHT
#define FRECUENCIA_ALTA  870000  //frecuencias base para sumarle la variable
#define FRECUENCIA_MEDIA 280000
#define FRECUENCIA_BAJA  100000 
#define FORMA_DE_ONDA    SINUSOIDAL//CUADRADA O SINUSOIDAL



extern unsigned char frecuenciaSeleccion;//SELECCIONA EL NIVEL de la frecuencia actual configurada y activada
extern unsigned char Alta; //variable que guarda el ultimo digito de la frecuencia alta
extern unsigned char Media;//variable que guarda el ultimo digito de la frecuencia media
extern unsigned char Baja;//variable que guarda el ultimo digito de la frecuencpia baja
//extern  volatile unsigned char ADCstatus;//maquina de estados del control del adc adquisicion de datos
extern struct _I2C_CONTROL IIC;
extern volatile ErrorControl_t FailsCtl;
volatile uint8 delay_RelojAnalogo_IRQ;//cuenta 100mseg y alli se queda si llega alos 100mseg.,
unsigned long int frecuency0;//store the actual frecq to compare in case of any change of it


static void vTask19_encender_DDS_y_Driver(uint8 *mem8);
static void detector_DDS(void);
static void detector_BAL_DRV(void);
static void Programar_DDS(void);


//extern unsigned long int frecuency; //guarda el valor actual de la frecuencia activada

//void init_Control_deFrecuencias(void ){	
/*#if DDS_GENERADOR == AD9834_DDS	 
	 init_DDS(_BOOT_);
#elif DDS_GENERADOR == ARDUINO_DDS
     Cambio_de_Frecuencia_por_IIC_1(1000);//frecuencia en khz
#endif*/
//}// fin de la inicializacion de control de frecuencias
 
 

unsigned long int  getFrecuency(void){
#if FORMA_DE_ONDA == SINUSOIDAL	   
	      switch(frecuenciaSeleccion){
	       case ALTA:  return(FRECUENCIA_ALTA+Alta*(unsigned long int)1000);break;
	       case MEDIA: return(FRECUENCIA_MEDIA+Media*(unsigned long int)1000);break;
	       case BAJA:  return(FRECUENCIA_BAJA+Baja*(unsigned long int)1000);break;	   
	       default:frecuenciaSeleccion=MEDIA;
	               return(getFrecuency());break;
	      }//fin switch
#elif FORMA_DE_ONDA == CUADRADA
	      switch(frecuenciaSeleccion){
	      	       case ALTA:  return((FRECUENCIA_ALTA+Alta*(unsigned long int)1000)*2);break; 
	      	       case MEDIA: return((FRECUENCIA_MEDIA+Media*(unsigned long int)1000)*2);break;
	      	       case BAJA:  return((FRECUENCIA_BAJA+Baja*(unsigned long int)1000)*2);break;	   
	      	       default:frecuenciaSeleccion=MEDIA;
	      	               return(getFrecuency());break;
	      	      }//fin switch
#endif	      
 }// fin de get frecuency---------------------------------------------------------------
 

// void SM1_Init_v2(void){
// 	
// 	  setReg16(QDLYR,0x0007U);//para 58.98Mhz,7=3.8us 4=2.17useg 
// 	  setReg16(  QMR,0x82FAU);
// 	  // MSTR  = 1
// 	  // BITSE = 1  -> 16 bits
// 	  // CPOL  = 1
// 	  // CPHA  = 0
// 	  // MSB first
// 	  // BAUD  = 0xFA
// 	  setReg16(QWR, 0x0050U);//new=0, end=5->6words
// 	  setReg16(QIR, 0x110DU);//limpiar flag QSPI
// 	  setReg16(QAR, 0x0020U);//comando RAM[0], inicio:20h
// 	  setReg16(QDR, 0x4000U);  
// }//fin de init v2-----------------------------------------

 
 /*CLOCK EDGE->RISING EDGE+++++++++++++++++++PARAMETROS+PARA+EL+PUERTO+QSPI++CONECTADO+++
  * SHIFT CLOCK IDLE POLARITY= HIGH++++++++++DIRECTAMENTE+ EL+PROCESADOR+AL+DSS+++++++++
  * SHIFT-CLOCK=12.7useg+++++++++++++++++++++SIN+USAR+INVERSOR+Y+MULTIPLEXOR++++++++++++
  * QSPI_DOUT-17++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  * QSPI_SCK_21+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  * **********************INIT**DDS**************************************************
  * frecuency  calculada    real      error   %error   
  *   1786000   893000    909.1khz   16.1hz    +1.8%
  *   1980000   990000    990.1khz     0%      0%
  *   1982000   991000    990.1khz
  *   1998000   999000    990.1khz
  *  System Clock:58.98MHz.
  * tiempo de Funcion:5.1useg Duracion de QSPI 6-bytes:862useg */
static void Programar_DDS(void){//Direct Digital Synthesis  Generator chip
#if DDS_GENERADOR == AD9834_DDS		 
unsigned long int frecuency; //=875000;//si es cadrada el reslado es la mitad de la frecencia
unsigned short int MSB,LSB,phase=0; //2 bytes   
unsigned long int calculated_freq_word; //4 bytes   
float AD9833val;
   	 
     disable_ADC();//detener el ADC()
     SPIselect(DDS);AD9833val=0.00000;
     frecuency=getFrecuency();
     //frecuency=1000000*2;
	 if(frecuency>1100000){
	                return;}//ya se programo DDS, dara error mas adelante
     AD9833val=((float)(frecuency))/XTAL_ANALOGA;
	 calculated_freq_word=(unsigned long int)(AD9833val*0x10000000);
    MSB=(unsigned short int)((calculated_freq_word & 0xFFFC000)>> 14); // 14 bits
    LSB=(unsigned short int)( calculated_freq_word & 0x3FFF);
	//set cntrl bits D15 & D14 to 0 & 1, repectivel, for frecq reg, 0
	LSB |=0x4000;
	MSB |=0x4000;
	phase &=0xC000;
	setReg16(QDLYR,0x0007U);//delay=3.80us con Fsys=58.98MHz
	setReg16(QMR, 0x82FAU);//master,16bits,CPOL=1,CPHA=0,BAUD=250
	setReg16(QWR, 0x0500U);//new=0,end=5->6words
	setReg16(QIR, 0x110DU);//limpiar flag QSPI
	setReg16(QAR, 0x0020U);//comando RAM[0],inicio:20h
	setReg16(QDR, 0xE000U);//comando[0]:CONT=1,BITSE=1,DT=1->delay despues de RESET
	setReg16(QDR, 0xC000U);//comando[1]:CONT=1,BITSE=1
	setReg16(QDR, 0xC000U);//comando[2]:CONT=1,BITSE=1
	setReg16(QDR, 0xC000U);//comando[3]:CONT=1,BITSE=1
	setReg16(QDR, 0xC000U);//comando[4]:CONT=1,BITSE=1
	setReg16(QDR, 0x4000U);//comando[5]:CONT=0,BITSE=1->fin,FSYNC=HIGH
	setReg16(QAR, 0x0000U);//TX RAM[0],inicio:00h
	setReg16(QDR, 0x0100U);//word[0]:RESET
	setReg16(QDR, 0x2100U);//word[1]:control
	setReg16(QDR, LSB);//word[2]:FREQ LSB
	setReg16(QDR, MSB);//word[3]:FREQ MSB
	setReg16(QDR, 0xC000U);//word[4]:PHASE
	setReg16(QDR, 0x2028U);//word[5]:salida cuadrada
	

	// WriteRegisterAD9833(0x0100); //reset //(0x2100);
	// //delay_ms(20);
	// WriteRegisterAD9833(0x2100); //square
	// WriteRegisterAD9833(LSB);// LOWER 14 bits
	// WriteRegisterAD9833(MSB); // upper 14 bits
	// WriteRegisterAD9833(0xC000);//phase);//mid-low
	// WriteRegisterAD9833(0x2028); //waveform cuadrada
	// delay_ms(1000);
	// WriteRegisterAD9833(0x2000); //sin
	 //AD9837Write(0x2002); //triangle
#endif	 
 }//fin initializae DDS--------------------------------------------------------------------
 

 void WriteRegisterAD9833(unsigned short int dat){
#if DDS_GENERADOR == AD9834_DDS		 
word16 buffer;
//unsigned int i;
     buffer.word16=dat;
	 FSYNC_PutVal(FALSE);
	 delay_us(1);
	   SM1_SendChar(buffer.byte[0]);
	   delay_us(1);//SIN ESTE DELAY NO FUNCIONA--------<--IMPORTANTE
	   SM1_SendChar(buffer.byte[1]);
	 delay_us(1);
	 FSYNC_PutVal(TRUE);
#endif	 
	 
 }// fin de write to register AD9833------------------------------------------
 
/*control de la signall de driver y monitoreo de la misma
 * generacion y programacion del DDS */ 
void vTask15_Control_de_Reloj_Analogo(void){
enum{INIT_DRIVER=1,APAGAR_DRIVER=0x23 };
static uint8 estado15;
static uint8 status15;
static uint16 cont15;
const uint16 TIEMPO_DE_MONITOR=1000;
enum{ SIZE_MEMO8=2};//Memoria de los subprocesos
static uint8 mem8[SIZE_MEMO8];

  if(cont15++>TIEMPO_DE_MONITOR){
    if(sys.u.bits.init_offset){
      switch(estado15){
    	  case 1:  //init de monitoreo de la driver y signals
				  if(!sys.u.bits.error_int_offset){	
					if(!sys.u.bits.DDS_encendido){
						sys.u.bits.error_offset_unset=1;
						sys.u.bits.Driver_out=1;//encender driver
						sys.u.bits.Encender_DDS=1;
						ADC_BAL_Init();//encender DDS
					    CLK_AN_COUNT_Init();
					    CLK_AN_COUNT_DisableEvent();}}//monitor reloj analogo
				  else{sys.u.bits.Encender_DDS=0;
					   sys.u.bits.error_offset_unset=0;
					   sys.u.bits.error_Bal=0;
					   sys.u.bits.Driver_out=0;}
				  estado15++;break;
    	  case 2:	  
				 if(sys.u.bits.error_Bal)
					  FailsCtl.LedDriver=ERROR_BALANCE_ALTO;
				 if(sys.u.bits.error_offset_unset)
					  FailsCtl.LedDriver=ERROR_OFFSET_UNSET;
				 if(sys.u.bits.error_Reloj_Analogo)
					  FailsCtl.LedDriver=ERROR_RELOJ_ANALOGO;
				 if(sys.u.bits.error_Driver_Signal)
					  FailsCtl.LedDriver=ERROR_DRIVER_APAGADO;
				 estado15++;break;
    	  case 3:if(sys.u.bits.Encender_DDS && //Debe estar encendido y no lo esta
    		        !sys.u.bits.DDS_encendido){
    		            //vTask19_encender_DDS_y_Driver(&mem8[0]);
    	                 }
    	         else{estado15++;}break;
    	  case 4:detector_DDS();estado15++;break;		//monitor de voltajes Direct Digital Synthetizer    
    	  case 5:detector_BAL_DRV();estado15++;break;//Monitor de Balance y voltaje de driver            
    	  case 6:estado15=2;break;
    	  default:estado15=1;break;}}}//fin switch-------------
    
} //fin de tarea 15 de reloj analogo---------------------
 


static void detector_BAL_DRV(void){
	
	
	
}//fin del detector de BAlance y Driver  voltajes





//Esta funcion detecta si hay signal en DDS  sino levana una bandera
/*Cada vez que llega un pulso (flanco de subida)  reseteas un contador de timeout.
En un loop periódico  incrementas el contador. Si supera un límite  no hay señal. */
static void detector_DDS(void){
static uint8 estado;  	
uint8 ret=0; 	
const uint16 VALOR_MIN=20000;//PARA 1Mhz
const uint8 NUM_MUESTRAS=5;//Muestras a evaluar
static uint16 valor,valorBuf[5];
static uint8 iVal;//indice del valor   
   switch(estado){
	   case 1:iVal=0;estado++;break;
	   case 2:CLK_AN_COUNT_Reset();
	          delay_RelojAnalogo_IRQ=0;
              estado++;break;
	   case 3:if(delay_RelojAnalogo_IRQ>(RELOJ_ANALOGO_MAX-2)){
		          CLK_AN_COUNT_GetCaptureValue(&valor);
		          estado++;}
	          break;
	   case 4:if(iVal>(NUM_MUESTRAS-1)) iVal=0;
		      valorBuf[iVal]=valor;
		      if(++iVal>(NUM_MUESTRAS-1)){iVal=0;estado++;}
		      else{estado=2;}
		      break;
	   case 5:if(iVal<(NUM_MUESTRAS-1)){
		         if(valorBuf[iVal++]>VALOR_MIN)estado++;}
		      else{sys.u.bits.error_Reloj_Analogo=1;
		           estado=1;}
	          break; 
	   case 6:sys.u.bits.error_Reloj_Analogo=0;	        	 
	   default:estado=1;break;}
}//fin detector_DDS--------------------------------


/* 
Encender  el Direct Digital Sinthetizer por   comandos de bits por QSPI,
a la frecuencia que guarde las variables globales de frecq, y controlar
encendido de Driver y lectura de voltajes balance y driver. * */
static void vTask19_encender_DDS_y_Driver(uint8 *mem8){
uint8 *estado19;	
	 
	estado19=mem8+0;//Memoria Requerida=1bytes
	switch(*estado19){
		case 1:Programar_DDS();(*estado19)++;break;
		case 2:
	default:*estado19=1;break;}
}// fin vTask19_encender_DDS_y_Driver--------------------

 

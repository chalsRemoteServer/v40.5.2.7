/*
 * errorController.c
 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */


/*
 * errorController.c
 *
 *  Created on: Jul 14, 2015
 *      Author: chals
 */
#include "errorController.h"
#include "LED3.h"
#include "LED2.h"
#include "TI1.h"
#include "delay.h"
#include "LEDERRCOMM.h"
#include "system.h"
#include "VFDmenu.h"
#include "LED_BOOT.h"
//#include "LED_COMM.h"
#include "LED12_SYS.h"
#include "LED3_Process.h"
#include "LED56_COMM_WARN_MON.h"
#include "queue.h"
#include "IOcomms.h"
#include "LED4_SYS_MON.h"


#define COMMI2CFAILS  100
#define COMMSPIFAILS  1
#define COMMI2COK     20
#define COMMSPIOK     30 

#define LEDERRTIME    300//tiempo que queda encendido el led de errores de communicacion cuando sale un error
#define VEL           2//constante, 
#define VEL1          3//velocidad de 3, primer ciclo no entra hasta el segundo, por tanto velocidad de 2
#define VEL2          4

#define LED2  0x0C
#define LED3  0xBB


unsigned long int ledloopvar; //variable para controlar el ledloop
unsigned char ledErrCommStatus;//varible que controla los led de errores de comunicaciones
unsigned short int  ledErrCommVar=0;//varible que controla los led de errores de comunicaciones
unsigned short int  ledvel,ledvel1=0; //velocidad de parpadeo del led de errores de comunicacion
//unsigned char ErrorHardware; //error de hardware se muestra en los led de flip-flop
unsigned char enableErrorControls;//habilitar el control de los errores
unsigned char realTimeError;//guarda el error que se despliega en pantalla en tiempo real
extern struct _SERIAL_ serial;//_Vector_ serial;


struct ErrorList_nodo eList[SIZE_LIST_ERRORS],*ehead;
struct ErrorHardware  eHw[SIZE_ERROR_HW];//lista de errores de hardware
//struct _Error323_ sysMon;
unsigned char ErrorStatusLeds;
struct _Error323_ e;//error controller data struct
extern  volatile unsigned char  TM1_IRQ1;

 uint8_t flip;   


// Definición de variables globales
LedSystemControl_t ledSystem;
LedWarningControl_t ledWarning;


// Variables externas para control de LEDs (debes adaptar según tu hardware)
// Estas funciones las debes implementar según tu hardware específico
extern void LED_System_On(void);    // Enciende LED System
extern void LED_System_Off(void);   // Apaga LED System
extern void LED_Warning_On(void);   // Enciende LED Warning
extern void LED_Warning_Off(void);  // Apaga LED Warning

static uint8_t global_error_state = ERROR_NONE;
static uint8_t global_flop_type = FLOP_NORMAL;

 // Variables para control de tiempo
static dlong  tiempo_anterior = 0;
static dlong tiempo_anterior_aux = 0;  // Para estados que lo requieran


volatile ErrorControl_t FailsCtl;


void init_ErrorController(void){
	  init_Leds();	
	  ledloopvar=0;
	  init_ListaError();
	  init_Lista_errorHardware();
	  ErrorStatusLeds=0;	
//	  sysMon.debug1=verificar_head_FIFOs_;
}// fin del controllador maestro de errores-+++++++++++++++++++++++++++++++++++++++

void init_Leds(void){
	  //-----------------inicia  prueba de leds
	 
}//fin init leds error indicators, system health indicators++++++++++++++++++


/* desabilita el control de los errores para evitar fallas y errores en la inizializacion si se
 *  detecta un error antes de inizializar todo el sistema   */
void disable_ErrorControls(void){enableErrorControls=FALSE;}
void enable_ErrorControls(void){ enableErrorControls=TRUE;} //se habilita el contol de los errores


void init_ListaError(void){
unsigned short int i;    
	  for(i=0;i<=SIZE_LIST_ERRORS;i++)//debug pero codigo incompleto
	      eList[0].status=NONE;  //debug   
}// initialization de lista errores------------------------------------------------

/* Lista Doblemente Ligada que guarda los Errores de Hardware Registrados
 * */
void init_Lista_errorHardware(void){
struct ErrorHardware *head,*p;	

	       head=getheadeHw(); //&eHw[0];
		   for(p=head;p<=&eHw[SIZE_ERROR_HW-1];p++){	   
			   if(p==head){
				   p->next=&eHw[1];
				   p->prev=&eHw[SIZE_ERROR_HW-1];
				   p->error=CODE_ERROR_STATUS;//nos dice que no hay errores
				   p->prioridad=NONE;}
			   else{
			    if(p==&eHw[SIZE_ERROR_HW-1]){
				    p->next=&eHw[0];
				    p->prev=&eHw[SIZE_ERROR_HW-2];
				    p->error=NONE;
				    p->prioridad=NONE;}
			    else{
			         p->next=p+1;
			         p->prev=p-1;
			         p->error=NONE;
			         p->prioridad=NONE;}}}
	      
		
}//fin initializacione de lista error hardware------------------------------------------------


//obtenemos la direccion de la cabeza de la lista de errores de hardware
struct ErrorHardware *getheadeHw(void){return &eHw[0];} 
	


void LedController_IRQ(void){//interrupcion proveniente de la IRQ principal del TIMER
	                  //entra aqui cada 200milisegundos
static unsigned short int  i,j;	
static unsigned char ErrorHardware0; //ver que el ErrorHardware no pase de 255 OJO  --> debug 
//struct ErrorHardware *p;
static unsigned char errorADC;
unsigned char modoerror=0;//modo que van a desplegar los leds el error leds 3 y 2	
static unsigned char errorVFD, errorIO;//error de comunicaciones
const unsigned char ERROR=0xAA;
const unsigned char LED_COMM_MAX_ON=300;//tiempo encendido el LED
const unsigned char LED_COMM_MAX_OFF=30;//tiempo apagado el LE


if((TM1_IRQ1&0x40)==0x40){
	TM1_IRQ1&=0xBF; //1011 1111
 if(enableErrorControls==TRUE){	 
	  //LEDs FLIP-FLOP status del sistema
	         if((ErrorStatusLeds & 0x80)==0){ // axxx xxxx a=bandera de que funcionan los ADC,
	               if(errorADC++>250){//no detectamos ADC desde hace mucho
	            	   modoerror=CODE_ERROR_0001_ADC;//contamos las veces que detecta no haber ADCs
	            	   ErrorHardware0=CODE_ERROR_0001_ADC;
	                   errorADC=0;}}
	         else {errorADC=0;//se limpia porque detecto funcionando los ADC
	               if(ErrorHardware0==CODE_ERROR_0001_ADC){//Si ya no hay error
	            	   ErrorHardware0=0;}}
	         //LEDs STATUS DE SISTEMA PRINCIPALES BOOT o bios
	         if((ErrorStatusLeds & 0x40)==0x40)// xbxx xxxx b= LED BOOT
	        	  ErrorHardware0=CODE_ERROR_0002_BOOT;
	         else {ErrorHardware0=0;}
	        //LED status error de COMMS comunicaciones VFD  e IO         
	         if((ErrorStatusLeds & 0x20)==0x20){//xxcx xxxx c=LED COMM error de Comm VFD = blink slow,fast error IO, conter blink
	               if(errorVFD++>250){
	            	   errorVFD=0; 
	            	   ErrorHardware0=CODE_ERROR_0003_VFD;}}
	         else  {errorVFD=0;
	                if(ErrorHardware0==CODE_ERROR_0003_VFD){
	                   ErrorHardware0=0;
	                   
	                   errorVFD=LED_COMM_MAX_OFF;}}//when no error, it blinks, duty on=90%
	         //LED status error de COMMS comunicaciones  IO         
	         if((ErrorStatusLeds & 0x10)==0x10){//xxxD xxxx c=LED COMM error de Comm IO = blink slow,fast error IO, conter blink
	               if(errorIO++>250){
	                   errorIO=0; 
	                   ErrorHardware0=CODE_ERROR_0004_IO;}}
	         else{errorVFD=0;
	               if(ErrorHardware0==CODE_ERROR_0004_IO){
	                   ErrorHardware0=0;
	                   errorVFD=LED_COMM_MAX_ON;}}//when no error, it blinks, duty on=90%
	                         
	         
	         //Leds FLIP-FLOP Controller
	         switch(ErrorHardware0){
	           case CODE_ERROR_0001_ADC: if(i++>2200){i=0;}break; 
	           case CODE_ERROR_0002_BOOT:if(i++>2200){i=0; break;}	
//	           case CODE_ERROR_0003_VFD: if(i++>2200){i=0; LED_COMM_NegVal(); break;}
//	           case CODE_ERROR_0004_IO:  if(i++>1500){i=0; LED_COMM_NegVal(); break;}
	                      
	           default:if(i++>2200){i=0;}//si no hay error se ejecutan normal
//	                   if(errorIO++>LED_COMM_MAX_ON){
//	                	   if(errorVFD++>LED_COMM_MAX_OFF){
//	                		      errorVFD=0; errorIO=0;
//	                        	  LED_COMM_PutVal(TRUE);}
//	                	   else{LED_COMM_PutVal(FALSE); }}

	                   break;//fin default
	           }//fin del switch-----------------------------------------------------------------
	         
	        	 
	     ErrorStatusLeds=0; //se resetea el estatus para volver a revisar el estado de todo
	         
	
   }//fin enable control de errores
    controlador_LED12(EJECUTAR);  
    controlador_LED_COMM_SYS_MON();
 
 
}//fin flag tmier1
}// fin Interrupt Request Querry OF lED cONTROLLER



//cuarto led AMARILLO MUESTRA
/* STATUS DE COMUNICACION CON LA IO Y EL ESTATUS
 * DEL MONITOR DEL LOS WARNINGS*/
void  controlador_LED_COMM_SYS_MON(void){
static unsigned char estado;
static unsigned short int ncount;
const unsigned short int DELAY_NO_ACK=500;

    
	if(e.ERROR.LEDS==0)
		 estado=0;
	else{if(e.ERROR.bits.aLed4_Comm_Warnning_Sys){//ERROR_RECEPCION_ACK_IO
		       estado=10;}} 
	switch(estado){
		case 0 :LED56_COMM_WARN_MON_PutVal(1);//apagar
		        break; 	   
	    case 10:if(ncount++>DELAY_NO_ACK){
	    	        LED56_COMM_WARN_MON_NegVal();
	    	        ncount=0;}
	            break;
	            
		default:estado=0;break;}//fin switch-----------
	
}//fin controlador_LED_COMM_SYS_MON-------------------------------





/* led 1 y 2  ON:indica el que se esta cargando el sistema
 * OFF: ocnfiguracion de sistema no valida
 * flash(1.75) system monitor OK,
 * los led se encienden con ZERO*/
void controlador_LED12(unsigned char n){
static unsigned char estado,semaforo,estado0;	
static unsigned short int cont;	
const unsigned char INIT2=0x55,STOP1=0xFF;
    switch(n){
    	case INIT: estado0=estado=INIT;break;
    	case WAIT: estado0=estado=WAIT;break;
    	case EJECUTAR:if(semaforo==STOP1)
    		              estado=estado0;
    				  else estado=n;
    	              break;
    	default:break;}//fin first switch
	switch(estado){
		case INIT: LED12_SYS_PutVal(FALSE);//se prende el sistem
		           semaforo=STOP1;//nose ejecuta EJEUTAR
		           break;
		case WAIT: cont=0;semaforo=STOP1;
		           estado0=estado=INIT2;
		           break;
		case INIT2:if(cont++>350){//Un rato y se apaga
					  LED12_SYS_PutVal(TRUE);//APAGAR LED
					  estado=EJECUTAR;
		              semaforo=0;cont=0;
		              break;}//liberams la maquina de estados al ejecutor
		           semaforo=STOP1; 
			       break;
		case EJECUTAR:if(cont++>3200){
			               LED12_SYS_NegVal();
		                   cont=0;}
		            break;
		default:break;}//fin switch--------------------------		
}//fin controlador de led1 1 y 2




/*Aqui acomulamos los errores de hardware en una pila y los vamos a sacar uno por uno 
 *  para limpiar todos los errores tiene que apagar y prender el sistema
 *  
 *  Se manda llamar para decirle que hay un error de hardware, lo revisa y
 *  busca si ya lo tiene registrado sino lo registra
 * */
struct ErrorHardware *ErrorController(unsigned char e){//aqui se acomulan la pila de errores solo de hardware 
struct ErrorHardware *p,*p2;
unsigned char prioridad;
 if(enableErrorControls==TRUE){
	 
	 
	switch(e){
	     case CODE_ERROR_STATUS:return getheadeHw();// en head en errores debe estar este mismo constante si no ha habido algun error
	                            break;
	     case CODE_ERROR_GET://buscamos el error con mayor prioridad  en la lista y regresa el pointer del error
	    	                 prioridad=0;
	    	                 p=getheadeHw();p2=p;
	    	                 do{if(p->error&&CODE_ERROR_HARDWARE==CODE_ERROR_HARDWARE){
	    	                	   if(p->prioridad>prioridad){
	    	                	        prioridad=p->prioridad;
	    	                	        p2=p;}
                                   p=p->next;}
                                else
                                	 p=p->next;
	    	                        }while(p!=getheadeHw());
	    	                 return (p2);
	    	                 break;
	     case CODE_ERROR_COMM_IO:prioridad=10;//error con mayor prioridad porque es el unico orita
	    	                     break;
	     default:break;
	  }//fin switch
	 
	  //insertErrorHw(e);
	  p=getheadeHw();  //controlador que guarda los errores y con su prioridad
loopec:if(p->error==NONE) {//busca un lugar libre para meter el error en la lista
		  p->error=e;
		  p->prioridad=prioridad;}
	   else{ 
		   p=p->next;
	       if(p!=getheadeHw())
	           goto loopec;}  
       }//fin enable control de errores	 
return 0;
}// fin error controller--------------------------------------------



// esta funcion ya quedo obsoleta, tiempo de vida:3.5 horas
void LedLoop(void){//nos indica si el programa esta un loop que puede ser infinito
	  
	  
}//fin de led en loop----------------------------------------------

/*If comm I2C comm error then LED=ON
 * if  comm  SPI comm error then LED FLASH fast
 * if comm  SPI & I2C fails then LED FLASHing slow
 * */
void LedErrorCommunicaciones(unsigned char status){
	  
	 switch(status){
	   case COMMI2CERR:if(ledvel1==0) ledvel1=VEL1;//es el primer fallo?
	                     else ledvel1=VEL2+1; //hay dos fallos de comunicacion
	                     ledErrCommStatus=ON;//activar alerta de fallo
	                     break;
	   case COMMSPIFAILS:if(ledvel1==0) ledvel1=VEL2;
                         else ledvel1=VEL2+1;
                         ledErrCommStatus=ON;
                         break;
	   case ERROR02_I2CCOMMFAILS:
		                 ledErrCommStatus=ON;//activar alerta de fallo
	                     ledloopvar=0;
                          break;                
	   default: eLog(CODE_ERROR_0001_ADC);  break;
	 }
	 
}//fin control de leds de Errores de Comunicaciones------------------------------------




/* FUNCION PRINCIPAL DEL CONTROL MAESTRO QUE ORIGINA LA DISTRIBUCION CENTRAL  
 *  EN EL MANEJADOR PADRE DE LOS ERRORES DEL SISTEMA 
 *  
 *  Distribuye los errores de hardware y los errores de software 
 * */
void eLog(unsigned short int error){
	
	
	switch(error){
	  case CODE_ERROR_0001_ADC: if(eList[CODE_ERROR_0001_ADC].status!=SAVED){
		                         // save_eLog(CODE_ERROR_0001);
		                          eList[CODE_ERROR_0001_ADC].status=SAVED;}
	                        break;
	  case CODE_ERROR_COMM_IO: ErrorController(CODE_ERROR_COMM_IO);
	                           break;
	  case CODE_ERROR_BUFFER_RX_FULL:
		                     if(eList[CODE_ERROR_BUFFER_RX_FULL].status!=SAVED){
		                    	 eList[CODE_ERROR_BUFFER_RX_FULL].status=SAVED;}
		                    break;
	  case  ERROR_DE_RTS_DE_UP:__asm(nop);
			                       break;
      case  ERROR_DE_COMUNICACION_DE_RECEPCION:__asm(nop);
			                       break;
      case  ERROR_RECEPCION_DE_ACK:__asm(nop);
			                        break;
	  case  ERROR_RECEPCION_EN_ESPERA_TIMEOUT:__asm(nop);
			                       break;
	  case ERROR_CTS_CERRO_SIN_ECIBIR_ACK:__asm(nop);
			                       break;
	  case CODE_ERROR_COMM_IO_RX:
		                         break;
	  case CODE_ERROR_ANSWER_DATOS:
	                              break;
	  case CODE_ERROR_FIFO_FULL: 
		                        break;
	  default:break;                      
	
	
	}//fin switch-----------------------------
	
}//fin de error log---------------------------------------------------------


void setLEDerrorHardware(void){
	  
	
	
}//fin set LED error de hardware para debugear, en teoria este led nunca deberia de prender si prende hay un error de software

/* CONTROLADOR PRINCIPAL CENTRAL OPERATIVO DEL MONITOREO DE ERRORES PARA
 * DESPLEGARLOS EN PANTALLA HASTA QUE SE APRIETE EL SELET*/
void ErrorMonitor(void){
	
	  
	  
	
	
}//FIN  DE ERROR MONITOR+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//el error se maneja por bit si un bit esta activado hay un error
void Monitor_System_Diagnostico_LEDs(unsigned char status){
const uint8_t ERROR_ALIVE=0x01;//Esperamos contestacion del otro uP	
const uint8_t WARNNING_COM=0x01;//Esperamos contestacion del otro uP
const uint8_t ERR_SEV_DAQ=0x80;//error severo no hay DAQ
const uint8_t ERR_SEV_ANCLK=0x40;//error severo no hay reloj analogo
const uint8_t ERR_SEV_AD9833=0x30;//ERROR severo no hay programacion en AD9833
static unsigned char led_Error,Led_Warnning;//si es Cero es NOrmal.	
static unsigned int cont1,cont2;
static uint16_t Led_Error_Counter,Led_No_Error_Counter;
	switch(status){
		case INIT:led_Error=0;Led_Warnning=0;
				  cont1=0;cont2=0;//se inicia systema,
  			      LedControl_System(LED_SYSTEM_NORMAL);
				  LedControl_Warning(LED_WARNING_NORMAL);break;
		case EJECUTAR:if(led_Error>0){Led_Error_Counter++;Led_No_Error_Counter=0;}
					  else{Led_Error_Counter=0;Led_No_Error_Counter++;}
					  if(Led_Error_Counter>50000)//<-tiempo de espera dela respuesta antes de poner la falla
			          {//aqui entra por INT IRQ ISR tmr1
						 if(led_Error>0xEF){//>EF es critico
							 LedControl_System(LED_SYSTEM_SERIOUS_ERROR);
							 break;}
						 else{if(cont1++>24500){//8000
							    LedControl_System(LED_SYSTEM_SYSTEM_ERROR);
							    Task9_SetErrorState(ERROR1);
							    Task9_SetFlopType(FLOP_INVERTIDO);
							    LedControl_Warning(LED_WARNING_ERROR_COMM);
							    FailsCtl.LedStatus=ERROR1;}}}
					  else{if(Led_No_Error_Counter>10000){
							      cont2=0;
								  Task9_SetErrorState(ERROR_NONE);
								  Task9_SetFlopType(FLOP_NORMAL);
								  LedControl_System(LED_SYSTEM_NORMAL);
								  LedControl_Warning(LED_WARNING_NORMAL);
								  FailsCtl.LedStatus=ERROR_NONE;}}	
		              break;  
		case CMD_IM_ALIVE:led_Error|=ERROR_ALIVE;//indica que mando el comando alive y que debe esperar respuesta
						  Led_Warnning|=WARNNING_COM; break;
		case CMD_IM_ALIVE_ANS:led_Error &= ~ERROR_ALIVE;cont1=0;
							  Led_Warnning&=~WARNNING_COM;break; 
		case ERR_SEVERO_DAQ:led_Error|=ERR_SEV_DAQ;break;//se actuva error severo
		case ERR_SEVERO_AN_CLK:led_Error|=ERR_SEV_ANCLK;break;
		case ERR_SEVERO_AD9833:led_Error|=ERR_SEV_AD9833;break;
		case ERR_SEVERO_AD9833_CLEAR:led_Error&=~ERR_SEV_AD9833;break;
		default:break;}

}//--------------------------------------------------------------------



// Controlar LED System
void LedControl_System(LedSystemParam_t param) {
	if(ledSystem.currentMode==param)return;
    switch(param) {
        case LED_SYSTEM_METAL:
            ledSystem.metalActive = true;// Activar modo metal
            ledSystem.metalCounter = 0;
            ledSystem.nextMode = ledSystem.currentMode;  // Guardar modo actual para restaurar
            ledSystem.currentMode = LED_SYSTEM_METAL;
            ledSystem.timer = 0;
            ledSystem.state = true;// Iniciar con LED ON
            LED_System_On();
            break;
        case LED_SYSTEM_SYSTEM_ERROR:
            ledSystem.metalActive = false;
            ledSystem.currentMode = LED_SYSTEM_SYSTEM_ERROR;
            ledSystem.timer = 0;
            break;
        case LED_SYSTEM_SERIOUS_ERROR:
            ledSystem.metalActive = false;
            ledSystem.currentMode = LED_SYSTEM_SERIOUS_ERROR;
            ledSystem.timer = 0;
            ledSystem.state = true;// Encender LED permanentemente
            LED_System_On();
            break;
        case LED_SYSTEM_NORMAL:
        default:
            ledSystem.metalActive = false;
            ledSystem.currentMode = LED_SYSTEM_NORMAL;
            ledSystem.timer = 0;
            break;}//FIN SWITCH----------------------
}//-----------------------------------------------------------------



// Controlar LED Warning
void LedControl_Warning(LedWarningParam_t param) {
	if(ledWarning.currentMode==param)return;
    switch(param) {
        case LED_WARNING_ERROR_COMM:
            ledWarning.currentMode = LED_WARNING_ERROR_COMM;
            ledWarning.timer = 0;
            ledWarning.state = true;// Encender LED permanentemente
            break;
        case LED_WARNING_SYSMON_WARNING:
            ledWarning.currentMode = LED_WARNING_SYSMON_WARNING;
            ledWarning.timer = 0;
            break;
        case LED_WARNING_NORMAL:
        default:ledWarning.currentMode = LED_WARNING_NORMAL;
            	ledWarning.timer = 0;
            	break;}//fin switch--------------------------------
}//----------------------------------------------------------------

// Función llamada desde Timer1 cada 1ms para controlar LED System
void LedSystem_Timer(void) {
    // Si hay un modo metal activo, controlar la duración
    if (ledSystem.metalActive && ledSystem.currentMode == LED_SYSTEM_METAL) {
        ledSystem.metalCounter++;
        // Verificar si ya pasó la duración del modo metal (300ms)
        if (ledSystem.metalCounter >= LED_SYSTEM_METAL_DURATION) {
            // Terminar modo metal y restaurar modo anterior
            ledSystem.metalActive = false;
            ledSystem.currentMode = ledSystem.nextMode;
            ledSystem.timer = 0;
            // Asegurar que el LED esté apagado al salir
            if (ledSystem.currentMode != LED_SYSTEM_SERIOUS_ERROR){
                ledSystem.state = false;
                LED_System_Off();}
            return;}}
    
    // Incrementar timer según el modo actual
    switch(ledSystem.currentMode){
        case LED_SYSTEM_NORMAL:
            ledSystem.timer++;
            if (ledSystem.state) {
                // LED está ON, esperar tiempo ON
                if (ledSystem.timer >= LED_SYSTEM_NORMAL_ON_TIME) {
                    ledSystem.state = false;
                    ledSystem.timer = 0;
                    LED_System_Off();}}
                else {
                // LED está OFF, esperar tiempo OFF
                if (ledSystem.timer >= LED_SYSTEM_NORMAL_OFF_TIME){
                    ledSystem.state = true;
                    ledSystem.timer = 0;
                    LED_System_On();}}
            break;
        case LED_SYSTEM_METAL:
            ledSystem.timer++;
            if (ledSystem.state) {
                // LED está ON, esperar tiempo ON (50ms)
                if (ledSystem.timer >= LED_SYSTEM_METAL_ON_TIME) {
                    ledSystem.state = false;
                    ledSystem.timer = 0;
                    LED_System_Off();}}
                else {
                // LED está OFF, esperar tiempo OFF (50ms)
                if (ledSystem.timer >= LED_SYSTEM_METAL_OFF_TIME) {
                    ledSystem.state = true;
                    ledSystem.timer = 0;
                    LED_System_On();}}
            break;
        case LED_SYSTEM_SYSTEM_ERROR:
            ledSystem.timer++;
            if (ledSystem.state) {
                // LED está ON, esperar tiempo ON (1.75ms  1-2ms)
                if (ledSystem.timer >= LED_SYSTEM_ERROR_ON_TIME) {
                    ledSystem.state = false;
                    ledSystem.timer = 0;
                    LED_System_Off();}}
            else{// LED está OFF, esperar tiempo OFF (1.75ms  1-2ms)
                if (ledSystem.timer >= LED_SYSTEM_ERROR_OFF_TIME) {
                    ledSystem.state = true;
                    ledSystem.timer = 0;
                    LED_System_On();}}
            break;
        case LED_SYSTEM_SERIOUS_ERROR:
            // Error grave: LED siempre ON (no se hace nada aquí)
            break;
    }//fin second switch-----------
}//LedSystem_Timer------------------------------------------------

// Función llamada desde Timer1 cada 1ms para controlar LED Warning
void LedWarning_Timer(void) {
    ledWarning.timer++;
    switch(ledWarning.currentMode){
        case LED_WARNING_NORMAL:
            if(ledWarning.state){// LED está ON, esperar tiempo ON (50ms)
                if (ledWarning.timer >= LED_WARNING_NORMAL_ON_TIME) {
                    ledWarning.state = false;
                    ledWarning.timer = 0;
                    LED_Warning_Off();}}
            else{// LED está OFF, esperar tiempo OFF (700ms)
                if (ledWarning.timer >= LED_WARNING_NORMAL_OFF_TIME) {
                    ledWarning.state = true;
                    ledWarning.timer = 0;
                    LED_Warning_On();}}
            break;
        case LED_WARNING_SYSMON_WARNING:
            if (ledWarning.state) {
                // LED está ON, esperar tiempo ON (500ms)
                if (ledWarning.timer >= LED_WARNING_SYSMON_ON_TIME) {
                    ledWarning.state = false;
                    ledWarning.timer = 0;
                    LED_Warning_Off();}} 
            else {// LED está OFF, esperar tiempo OFF (500ms)
                if (ledWarning.timer >= LED_WARNING_SYSMON_OFF_TIME) {
                    ledWarning.state = true;
                    ledWarning.timer = 0;
                    LED_Warning_On();}}
            break;
        case LED_WARNING_ERROR_COMM:
        	if (ledWarning.state) {// LED está ON, esperar tiempo ON (500ms)
				if (ledWarning.timer >= LED_WARNING_COMM_ON_TIME) {
					ledWarning.state = false;
					ledWarning.timer = 0;
					LED_Warning_Off();}} 
			else {// LED está OFF, esperar tiempo OFF (500ms)
				if (ledWarning.timer >= LED_WARNING_COMM_OFF_TIME) {
					ledWarning.state = true;
					ledWarning.timer = 0;
					LED_Warning_On();}}
        	break;}//fin de switch------------------------------------------
}//fin  --------LedWarning_Timer-----------------------------------


// Inicializar control de LEDs
void LedControl_Init(void) {// Inicializar LED System
    ledSystem.currentMode = LED_SYSTEM_NORMAL;
    ledSystem.nextMode = LED_SYSTEM_NORMAL;
    ledSystem.timer = 0;
    ledSystem.state = false;
    ledSystem.metalActive = false;
    ledSystem.metalCounter = 0;
    
    ledWarning.currentMode = LED_WARNING_NORMAL;// Inicializar LED Warning
    ledWarning.timer = 0;
    ledWarning.state = false;
    
    LED_System_Off();// Apagar LEDs al inicio
    LED_Warning_Off();
}//-----------------------------------------------------------

// FUNCIÓN PARA ACTUALIZAR ESTADO DE ERROR
void Task9_SetErrorState(ErrorState_t state){
    global_error_state = state;
    tiempo_anterior = millis();  // Resetear temporizadores
    tiempo_anterior_aux = millis();
    if(state == ERROR_NONE){
        LED2_SetVal();
        LED2_ClrVal();}
}//------------------------------------



void LED_Hardware_Init(void) {
//unsigned long int i;
	LedControl_Init();                         // Inicializar control de LEDs 
	LED_System_Off();
    LED_Warning_Off();
    //delay_ms(800);
    LED_System_On();
	LED_Warning_On();
    //delay_ms(800);
	LED_System_Off();
	LED_Warning_Off();
	LedControl_System(LED_SYSTEM_NORMAL);
	LedControl_Warning(LED_WARNING_NORMAL);
	//Monitor_System_Diagnostico_LEDs(INIT);
}//------------------------------------------------


void LED_System_On(void) {LED56_COMM_WARN_MON_SetVal(); }  // Encender LED System
void LED_System_Off(void) {LED56_COMM_WARN_MON_ClrVal(); }  // Apagar LED System
void LED_Warning_On(void) {LED4_SYS_MON_SetVal();}  // Encender LED Warning
void LED_Warning_Off(void) {LED4_SYS_MON_ClrVal();}  // Apagar LED Warning



 
 // FUNCIÓN PARA ACTUALIZAR TIPO DE FLOP
 void Task9_SetFlopType(FlopType_t type){
     global_flop_type = type;
     tiempo_anterior = millis();
 }//----------------------------------------------------------


void vTask9_LEDs_Monitor(void){
static uint32_t elapsed1,elapsed2,elapsed3;
uint32_t elapsed;
uint32_t tiempo_actual = 0;
//bool estado_led1_temp, estado_led2_temp;
//static word counter;
static uint8_t initLED;
uint64_t ahora; 

    switch(initLED){
	   case 0xBB:initLED++;break;
	   case 0xBC:Task9_SetErrorState(ERROR_NONE);
		   	     Task9_SetFlopType(FLOP_NORMAL);
		   	     LED_Hardware_Init();
		   	     initLED++;break; 
	   case 0xBD:goto vT9;break;
	   default:initLED=0xBB;return;break;}	
vT9:   ahora = millis(); 
	   elapsed=(uint32_t)(ahora-tiempo_anterior);// Verificar si ha pasado el tiempo para esta iteración
	   elapsed1+=elapsed;elapsed2+=elapsed;elapsed3+=elapsed;
	   if(elapsed1>350){elapsed=0;//LEDS DIAGNOSTICO
		   LedSystem_Timer();   // Controlar LED System cada 1ms
		   LedWarning_Timer();}  // Controlar LED Warning cada 1ms
	   if(elapsed2>225){elapsed2=0;
	       Monitor_System_Diagnostico_LEDs(EJECUTAR);}//ejeuta timer cada 500useg
	   Monitor_de_Error_de_ADCs();
}//-------------------------------------------------------------------------------------



// ============================================================
// FUNCIÓN: Monitor_System_status_LEDs_v2 (FLIP-FLOP PERFECTO)
// ============================================================
/*   NORM_DELAY: 1600->800mseg           */
void IRQ_Monitor_System_status_LEDs_v3(void){//IRQ cada 500useg
enum{ ERR_DELAY=700,  NORM_DELAY=1250 /*1250*/};
static uint16_t delay;
  
  switch(FailsCtl.LedStatus){//leds status
    	case ERROR_NONE:
			  if(delay++>NORM_DELAY){
				if(flip)flip=0;else{flip=1;}
			    delay=0;}
			  break;
    	case ERROR1:
    			if(delay++>ERR_DELAY){
    				if(flip)flip=0;else{flip=1;}
    				delay=0;}
    			break;
    	default:break;}
  if(flip){LED2_SetVal();LED3_ClrVal();}
  else{LED2_ClrVal();LED3_SetVal();}
}//fin -----------------------------------------------------------------
//500MSEG DE INTERRUPCON
void IRQ_Monitor_System_Driver_LEDs_v1(void){//IRQ cada 500useg
enum{NORM_DELAY_ON=200,NORM_DELAY_OFF=1800, //ON=10% OFF=90% Normal
	 ERR1_DELAY_ON=1000,ERR1_DELAY_OFF=1000,//ON=50% OFF=50% Fast
	 ERR2_DELAY_ON=2700,ERR2_DELAY_OFF=100,//ON=100%
	 ERR3_DELAY_ON=2700,ERR3_DELAY_OFF=100,//ON=100%
	 ERR4_DELAY_ON=2000,ERR4_DELAY_OFF=2000,//ON=50% OFF=50% Normal
	 ERR5_DELAY_ON=1500,ERR5_DELAY_OFF=600};// ON=80% OFF=20% Fast
static uint16_t delay1,var;
const uint16 CONTADOR_RESET=10000;//cada 5seg se resetea
static uint16_t contReset;

  if(contReset++>CONTADOR_RESET){
	  FailsCtl.LedDriver=ERROR_NONE1;
	  contReset=0;} 
  switch(FailsCtl.LedDriver){//leds status
    	case ERROR_NONE1:
			  if(delay1++>var){
				if(flip){flip=0;var=NORM_DELAY_ON;}
				else{flip=1;var=NORM_DELAY_OFF;}
			    delay1=0;}
			  break;
    	case ERROR_BALANCE_ALTO:  
    			if(delay1++>var){
    				if(flip){flip=0;var=ERR1_DELAY_ON;}
    				else{flip=1;var=ERR1_DELAY_OFF;}
    				delay1=0;}
    			break;
    	case ERROR_BALANCE_ALTO_INIT:
         		if(delay1++>var){
    		    	if(flip){flip=0;var=ERR2_DELAY_ON;}
    		    	else{flip=1;var=ERR2_DELAY_OFF;}
    		        delay1=0;}
    		    	break;
    	case ERROR_OFFSET_UNSET:
    			if(delay1++>var){
					if(flip){flip=0;var=ERR3_DELAY_ON;}
					else{flip=1;var=ERR3_DELAY_OFF;}
					delay1=0;}
					break;
    	case ERROR_DRIVER_APAGADO:
				if(delay1++>var){
					if(flip){flip=0;var=ERR4_DELAY_ON;}
					else{flip=1;var=ERR3_DELAY_OFF;}
					delay1=0;}
					break;
    	case ERROR_RELOJ_ANALOGO:
				if(delay1++>var){
					if(flip){flip=0;var=ERR5_DELAY_ON;}
					else{flip=1;var=ERR3_DELAY_OFF;}
					delay1=0;}
					break;
    						
					
    	default:break;}
  if(flip){LED2_SetVal();LED3_ClrVal();}
  else{LED2_ClrVal();LED3_SetVal();}
}//FIN IRQ_Monitor_System_Driver_LEDs_v1---------------


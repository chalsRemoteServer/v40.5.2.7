/*
 * errorController.h
 *
 *  Created on: Jul 14, 2015
 *      Author: chals
 */

#ifndef ERRORCONTROLLER_H_
#define ERRORCONTROLLER_H_

#include "PE_Types.h"




#define ERROR01                    0x01 //error en el switch de init_Control_deFrecuencias
#define ERROR02_I2CCOMMFAILS       0xFF //error I2C comm fails
#define COMMI2CERR                 0xFE //error I2C comm error no ack
#define SIZE_LIST_ERRORS           100 //cantidad de errorres guardados
#define SAVED                      0xFD//ha sido salvado el error?




#define Fault_Save_Prod_EEPROM     0x02 //para abrebiar se pone el codigo
#define CODE_ERROR_DIGITAL_FILTER1 0x03 //error de codigo en filtro digital1

#define CODE_ERROR_0000            0xFE //NO HAY NINGUN ERROR
//ERRORES DE SOFWARE   00XX XXXX  <--RESPETAR MASCARA MASCARA
#define CODE_ERROR_0001_ADC        0x01//no funcionan los ADCs
#define CODE_ERROR_0002_BOOT       0x02// error no funciona el BIOS algo no initizalizo
#define CODE_ERROR_0003_VFD        0x03// error de ccomunicacion de keypad recepcion o transmision de VFD
#define CODE_ERROR_0004_IO         0x04// error de comunicacion con la IO inea
#define CODE_ERROR_0005            0x05//
#define CODE_ERROR_0006            0X06//getANS_DATOS
#define CODE_ERROR_BUFFER_RX_FULL  0x76
//ERRORES COMANDOS     01XX XXXX  <--RESPETAR MASCARA
#define CODE_ERROR_STATUS          0x77//se guarda en el head y nos dice que no hay errores
#define CODE_ERROR_GET             0x78//comando para obtener el error con mayor prioridad si lo hay
//ERRORES DE HARDWARE  10XX XXXX  <--RESPETAR MASCARA       0x60U //aqui inician los errores por hardware que se indican en los LED's
#define CODE_ERROR_HARDWARE        0x80//INICIo de errores de harware
#define CODE_ERROR_COMM_IO         0x81//NO DETECTAMOS comunicacion de recepcion con la IO
#define CODE_ERROR_COMM_IO_RX      0x82
#define CODE_ERROR_ANSWER_DATOS    0x83 //ERROR DE RESPUESTA DE DATOS  
#define CODE_ERROR_FIFO_FULL       0x84
#define CODE_ERROR_EEPROM_BLOQUE1  0x85 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE2  0x86 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE3  0x87 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE4  0x88 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE5  0x89 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE6  0x8A //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE7  0x8B //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE8  0x8C //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE9  0x8D //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE10 0x8E //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE11 0x8F //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE12 0x90 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE13 0x91 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE14 0x92 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE15 0x93 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE16 0x94 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE17 0x95 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE18 0x96 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE19 0x97 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE20 0x98 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE21 0x99 //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE22 0x9A //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE23 0x9B //No se escribio o leyo el bloque en la EEPROM
#define CODE_ERROR_EEPROM_BLOQUE24 0x9C //No se escribio o leyo el bloque en la EEPROM

#define CODE_ERROR_EEPROM_BYTE1    0xB0//No se escribio o leyo un byte de memoria



// ver que esta lista no se mayor a SIZE_LIST_ERRORS
#define VEL_LED_NORMAL            120 //VELOCIDAD DEL LED NORMAL
#define VEL_LED_ERR_IO            60  //velocidad de error comm con la IO
#define SIZE_ERROR_HW             20//TAMAÑO DE LA lista que guarda los errores de hardware

/* DEFINICIONES COMUNES DE IO Y PROCESADORA
 * 
     version 1.0.0 */
#define ERROR_DE_RTS_DE_UP                   0x30
#define ERROR_DE_COMUNICACION_DE_RECEPCION   0x31
#define ERROR_RECEPCION_DE_ACK               0x32 //deprecated
#define ERROR_RECEPCION_EN_ESPERA_TIMEOUT    0x33 //no mandaron nada de la otra tareta como habian dicho
#define ERROR_CTS_CERRO_SIN_ECIBIR_ACK       0x34//la otra tarjeta cerro el puerto de escucha sin recibir ACK 
#define ERROR_RECEPCION_ACK_IO               0x01


/* fin de definiciones comunnes de IO  y UP  */


/* version 310322-1430    add aLed
 * */



/* fin de definiciones comunnes de IO  y UP  */
// Definiciones de tiempos para LED System (en milisegundos)
#define LED_SYSTEM_NORMAL_ON_TIME        8000      // ms OFF
#define LED_SYSTEM_NORMAL_OFF_TIME       600     // ms ON

#define LED_SYSTEM_METAL_ON_TIME         50      // 50ms ON
#define LED_SYSTEM_METAL_OFF_TIME        50      // 50ms OFF
#define LED_SYSTEM_METAL_DURATION        300     // 300ms de duración total del patrón

#define LED_SYSTEM_ERROR_ON_TIME         1600//300  // ms ON (aproximado)
#define LED_SYSTEM_ERROR_OFF_TIME        1600//300  // ms OFF (aproximado)

// Definiciones de tiempos para LED Warning
#define LED_WARNING_NORMAL_ON_TIME       12300      // 50ms OFF
#define LED_WARNING_NORMAL_OFF_TIME      500     // 700ms ON

#define LED_WARNING_SYSMON_ON_TIME       1600     // 500ms ON
#define LED_WARNING_SYSMON_OFF_TIME      1600     // 500ms OFF

#define LED_WARNING_COMM_ON_TIME       50     // 500ms ON estan al revez el on off en warnning
#define LED_WARNING_COMM_OFF_TIME      1800     // 500ms OFF

// ============================================
 // CONFIGURACIÓN DE TIEMPOS (fáciles de ajustar) de los LEDs de Diagnostico
 // ============================================
 #define VEL_FLOP_NORMAL        223     // ms para flip-flop normal
 #define VEL_FLOP_RAPIDO        4980     // ms para flop rápido
 #define VEL_FLOP_MAS_RAPIDO    3755      // ms para flop rapidísimo
 #define VEL_ERROR4           551510     // ms para error4 (ambos blink iguales)
 #define VEL_ERROR5           551510     // ms para error5 (ambos blink iguales)
 #define VEL_FLOP_ERROR1        3843
 #define VEL_FLOP_ERROR2        2843
 #define VEL_FLOP_ERROR3        3843
 #define VEL_FLOP_ERROR4        4843

typedef enum{
	APAGADO=1,ENCENDIDO=0}estadoLed;
typedef struct{
  union {
	 uint8 monADCbyte;
	 struct {
		 uint8 adcSample:1;
		 uint8 busyIRQ:1;
		 uint8 txQSPI:1; //transmit QSPI
		 uint8 tmr2IRQ:1;
		 uint8 readQSPI:1;
		 uint8 ADC_enable:1;//habilita que se ejecute en ADC1 solamente
		 uint8 reserved:2;}bits;}monADC;
  uint32 control;	
  estadoLed status;
  uint32 LedControl;
}Led_ADC;




// Definición de parámetros para LED System
typedef enum {
    LED_SYSTEM_NORMAL = 0,      // Parpadeo normal: 50ms ON / 800ms OFF
    LED_SYSTEM_METAL,           // Parpadeo metal: 50ms ON/50ms OFF por 300ms, luego vuelve a Normal
    LED_SYSTEM_SYSTEM_ERROR,    // Parpadeo error sistema: 1.75ms ON/OFF aprox
    LED_SYSTEM_SERIOUS_ERROR    // Error grave: ON permanente
} LedSystemParam_t;

// Definición de parámetros para LED Warning
typedef enum {
    LED_WARNING_NORMAL = 0,     // Parpadeo normal: 50ms ON / 700ms OFF
    LED_WARNING_SYSMON_WARNING, // Advertencia monitoreo: 500ms ON / 500ms OFF
    LED_WARNING_ERROR_COMM      // Error comunicación: ON permanente
} LedWarningParam_t;

// Estructura para control de LED System
typedef struct {
    LedSystemParam_t currentMode;   // Modo actual
    LedSystemParam_t nextMode;      // Modo al que cambiar después de Metal
    unsigned short int timer;       // Timer en milisegundos
    bool state;                     // Estado actual del LED (ON/OFF)
    bool metalActive;               // Indicador de modo metal activo
    unsigned short int metalCounter;// Contador para duración del modo metal
} LedSystemControl_t;

// Estructura para control de LED Warning
typedef struct {
    LedWarningParam_t currentMode;  // Modo actual
    unsigned short int timer;       // Timer en milisegundos
    bool state;                     // Estado actual del LED (ON/OFF)
} LedWarningControl_t;


// Estados de error
typedef enum {
    ERROR_NONE = 0,
    ERROR1 = 1,   // Un LED Encendido, el otro blink normal, ERROR COM
    ERROR2 = 2,   // Un LED apagado, el otro blink rapidísimo,-ERROR ADC
    ERROR3 = 3,   // Ambos LEDs encendidos,ERROR DRIVER
    ERROR4 = 4,   // Ambos blink iguales (no flip-flop), un poco más rápido
    ERROR5 = 5    // Ambos blink iguales, un poco más rápido (variante)
} ErrorState_t;

// Estados de error
typedef enum {//LED ROJO
    ERROR_NONE1             = 0,   //ON=10% OFF=90% Normal   sin errores                
    ERROR_BALANCE_ALTO      = 1,   //ON=50% OFF=50% Fast     se detecto en trabajo normal depues de los inits 
    ERROR_BALANCE_ALTO_INIT = 2,   //ON=100%                 se detecto desbalance en el init del offset 
    ERROR_OFFSET_UNSET      = 3,   //ON=100%                no se logro el offset al init 
    ERROR_DRIVER_APAGADO    = 4,   //ON=50% OFF=50% Normal   se detecto el driver que no funciona
    ERROR_RELOJ_ANALOGO     = 5  // ON=80% OFF=20% Fast     se detecto el reloj que no funciona
} ErrorDriver_t;


// Tipos de flop
typedef enum {
    FLOP_NORMAL = 0,
    FLOP_INVERTIDO = 1,
    FLOP_DERECHA = 2,
    FLOP_IZQUIERDA = 3
} FlopType_t;

// Prototipos
void Task9_LEDs_Monitor(void);
void Task9_SetErrorState(ErrorState_t state);
void Task9_SetFlopType(FlopType_t type);

typedef struct{
  uint8_t LedStatus;
  uint8_t LedDriver;
}ErrorControl_t;

 


// Variables globales (para acceso desde otros archivos)
extern LedSystemControl_t ledSystem;
extern LedWarningControl_t ledWarning;



/*version 070422-1210  add union bit bytes error leds*/
struct _Error323_{
//	unsigned char status;
//	void (*debug1)(signed short int*,unsigned char);
//	signed short int *head[100];
//	unsigned char position[100];
	union _LEDS{
	    unsigned char LEDS;
		struct {
	    unsigned char aLed4_Comm_Warnning_Sys:1; 
		unsigned char i:1;
		unsigned char Led4_Comm:1;//status
		unsigned char Led4_Comm_sem:1;//semaforo
		unsigned char x5:1;
		unsigned char x6:1;
		unsigned char x7:1;
		unsigned char x8:1; 
	   }bits;
     }ERROR;
};

struct ErrorList_nodo{
	   
	 //  unsigned short int errorNum;//numero de eror  ,, ver arriba
	   unsigned char status;// status salvado, borrado, listo o que
	 //  struct ErrorList_nodo *next;
};

struct ErrorHardware{//guarda los errores de hardware en una lista
	  struct ErrorHardware *next;
	  struct ErrorHardware *prev;
	  unsigned char error;
	  unsigned char prioridad;
};


//procedimientos y funciones++++++++++++++++++++++++++++++++++++++++
void init_ErrorController(void); // controlador maestro de errores
void init_Leds(void);
void LedController_IRQ(void);
struct ErrorHardware *ErrorController(unsigned char e);
void LedLoop(void);
void LedErrorCommunicaciones(unsigned char status);
void eLog(unsigned short int error);
void init_ErrorController(void);
void init_ListaError(void);
void init_Lista_errorHardware(void);
void disable_ErrorControls(void);
void enable_ErrorControls(void);
struct ErrorHardware *getheadeHw(void);
void setLEDerrorHardware(void);
void ErrorMonitor(void);
void verificar_head_FIFOs_(signed short int *p,unsigned char n);
void controlador_LED12(unsigned char n);
void  controlador_LED_COMM_SYS_MON(void);
void Monitor_System_Diagnostico_LEDs(unsigned char status);
void LedControl_Init(void);                         // Inicializar control de LEDs
void LedControl_System(LedSystemParam_t param);     // Controlar LED System
void LedControl_Warning(LedWarningParam_t param);   // Controlar LED Warning
void LedSystem_Timer(void);                         // Función llamada desde Timer1 (cada 1ms)
void LedWarning_Timer(void);                        // Función llamada desde Timer1 (cada 1ms)
void LED_Hardware_Init(void);
void Monitor_System_Diagnostico_LEDs(unsigned char status);
void vTask9_LEDs_Monitor(void);
void Monitor_System_status_LEDs_v2(void);
void LED_SetVal_Debug(uint8_t led);
void LED_ClrVal_Debug(uint8_t led);
void LED_NegVal_Debug(uint8_t led);
uint8_t LED_GetVal_Debug(uint8_t led);
void IRQ_Monitor_System_status_LEDs_v3(void);
void IRQ_Monitor_System_Driver_LEDs_v1(void);
void Monitor_de_Error_de_ADCs(void);	




#endif /* ERRORCONTROLLER_H_ */

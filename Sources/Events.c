/* ###################################################################
**     Filename    : Events.c
**     Project     : 17_IN1UP
**     Processor   : MCF52233CAF60
**     Component   : Events
**     Compiler    : CodeWarrior MCF C Compiler
**     Date/Time   : 2015-07-13, 17:20, # CodeGen: 0
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Settings    :
**     Contents    :
**         Cpu_OnCoreWatchdogINT - void Cpu_OnCoreWatchdogINT(void);
**
** ###################################################################*/
/*!
** @file Events.c
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         
/* MODULE Events */

#include "Cpu.h"
#include "Events.h"
#include "errorController.h"
#include "VFD.h"
#include "Reloj.h"
#include "ADC.h"
#include "DSP.h"
#include "VFDserial.h"
#include "IOcomms.h"
#include "VFDmenu.h"
#include "Memoria.h"
#include "system.h"
#include "AnalogCntrl.h"
#include "SistemaOperativo.h"
#include "queue.h"
#include "TFTmenu.h"
//#include "keypad.h"
#include "frecqController.h"


extern struct _Comando_DDS dds; //extern struct _DDS_ DDS1;
extern struct _Detection Deteccion;
unsigned int  delaycount;
extern struct _SERIAL_ serial;    //extern struct _Vector_ serial;
volatile unsigned char  TM1_IRQ1;
extern struct _COMMs_ com;
volatile uint64_t Tick;//reloj del systema
static uint8_t milisecond;//milisegundo control para ADC
extern volatile uint8 delay_RelojAnalogo_IRQ;//cuenta 100mseg y alli se queda si llega alos 100mseg.,
extern volatile  unsigned char ADCstatus;//maquina de estados del control del adc adquisicion de datos


/* User includes (#include below this line is not maintained by Processor Expert) */

/*
** ===================================================================
**     Event       :  Cpu_OnCoreWatchdogINT (module Events)
**
**     Component   :  Cpu [MCF52233_112_LQFP]
**     Description :
**         This event is called when the OnCoreWatchdog interrupt had
**         occurred. This event is automatically enabled when the <Mode>
**         is set to 'Interrupt'.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void Cpu_OnCoreWatchdogINT(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  TI1_delays_OnInterrupt (module Events)
**
**     Component   :  TI1_delays [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void TI1_delays_OnInterrupt(void)
{
  /* Write your code here ... */
	delaycount++;
	
	
}// fin TI1 delays OnInterrupt fin---VFDserial_OnTxChar------------------------------------

/*
** ===================================================================
**     Event       :  VFDserial_OnError (module Events)
**
**     Component   :  VFDserial [AsynchroSerial]
**     Description :
**         This event is called when a channel error (not the error
**         returned by a given method) occurs. The errors can be read
**         using <GetError> method.
**         The event is available only when the <Interrupt
**         service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void VFDserial_OnError(void)
{
  /* Write your code here ... */
	VFDserial_ClearRxBuf();
}

/*
** ===================================================================
**     Event       :  VFDserial_OnRxChar (module Events)
**
**     Component   :  VFDserial [AsynchroSerial]
**     Description :
**         This event is called after a correct character is received.
**         The event is available only when the <Interrupt
**         service/event> property is enabled and either the <Receiver>
**         property is enabled or the <SCI output mode> property (if
**         supported) is set to Single-wire mode.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void VFDserial_OnRxChar(void)
{
  /* Write your code here ... */
/*   se�al del teclado    */
	 //serialKeyPad2.Byte1.bit.IRQ_rx=TRUE;
unsigned char c;	
	 
    VFDserial_RecvChar(&c);
    //VFDkeypad_ISR(c);
   com.RxDisp.append(c,&com.RxDisp);
}// fin VFDserial_OnRxChar-----------------------------------------------

/*
** ===================================================================
**     Event       :  VFDserial_OnTxChar (module Events)
**
**     Component   :  VFDserial [AsynchroSerial]
**     Description :
**         This event is called after a character is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void VFDserial_OnTxChar(void)
{
  /* Write your code here ... */
//	DDStx_flag_INT=0xAA;
	//DDSplotflag=0xAA;
	
}

/*
** ===================================================================
**     Event       :  VFDserial_OnFullRxBuf (module Events)
**
**     Component   :  VFDserial [AsynchroSerial]
**     Description :
**         This event is called when the input buffer is full;
**         i.e. after reception of the last character 
**         that was successfully placed into input buffer.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void VFDserial_OnFullRxBuf(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  VFDserial_OnFreeTxBuf (module Events)
**
**     Component   :  VFDserial [AsynchroSerial]
**     Description :
**         This event is called after the last character in output
**         buffer is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void VFDserial_OnFreeTxBuf(void)
{
  /* Write your code here ... */
}



/*
** ===================================================================
**     Event       :  TACHO_OnCapture (module Events)
**
**     Component   :  TACHO [Capture]
**     Description :
**         This event is called on capturing of Timer/Counter actual
**         value (only when the component is enabled - <Enable> and the
**         events are enabled - <EnableEvent>.This event is available
**         only if a <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void TACHO_OnCapture(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  RTC1_OnAlarm (module Events)
**
**     Component   :  RTC1 [RTC]
**     Description :
**         This event is called whenever an actual time is equal to the
**         alarm time. (only when the component is enabled - <"Enable">
**         and the events are enabled - <"EnableEvent"> (event is
**         available only when the property Interrupt service/event is
**         set to - <"Enabled">)
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void RTC1_OnAlarm(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  RTC1_OnSecond (module Events)
**
**     Component   :  RTC1 [RTC]
**     Description :
**         This event is called on every increment of the second
**         counter. (only when the component is enabled - <"Enable">
**         and the events are enabled - <"EnableEvent"> (event is
**         available only when the property Interrupt service/event is
**         set to - <"Enabled">)
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void RTC1_OnSecond(void)
{  /* Write your code here ... */
	displaySegundero_IRQ();
	if(serial.vars.var.timercomms<250)
		serial.vars.var.timercomms++;//serial.cntrl.timercomms++;
	IRQ_comunicacion_IOUP();
}

/*
** ===================================================================
**     Event       :  SM1_OnTxChar (module Events)
**
**     Component   :  SM1 [SynchroMaster]
**     Description :
**         This event is called after a character is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void SM1_OnTxChar(void)
{
  /* Write your code here ... */
	
	QSPI_OnTxChar_IRQ();//interrupcion de Maquina Automatica de DAQ

}

/*
** ===================================================================
**     Event       :  Busy_OnInterrupt (module Events)
**
**     Component   :  Busy [ExtInt]
**     Description :
**         This event is called when an active signal edge/level has
**         occurred.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void Busy_OnInterrupt(void)
{
  /* place your Busy interrupt procedure body here */
	Busy_Interrupt_IRQ();//llego se�al de busy vamos a leer los adc de t=0,

}


/*
** ===================================================================
**     Event       :  TI2_OnInterrupt (module Events)
**
**     Component   :  TI2 [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/*
** ===================================================================
**     Event       :  TI2_OnInterrupt (module Events)
**
**     Component   :  TI2 [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void TI2_OnInterrupt(void)
{
  /* Write your code here ... */
//145useg -  leer segundo ADC-2
	QSPI_On_TMR2_IRQ();


}//TI2_OnInterrupt----------------------------------
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
/*
** ===================================================================
**     Event       :  IOUPserial_OnError (module Events)
**
**     Component   :  IOUPserial [AsynchroSerial]
**     Description :
**         This event is called when a channel error (not the error
**         returned by a given method) occurs. The errors can be read
**         using <GetError> method.
**         The event is available only when the <Interrupt
**         service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing TI2_OnInterrupt
** ===================================================================
*/
void IOUPserial_OnError(void)
{
  /* Write your code here ... */
	__asm(nop);
}

/*
** ===================================================================
**     Event       :  IOUPserial_OnRxChar (module Events)
**
**     Component   :  IOUPserial [AsynchroSerial]
**     Description :
**         This event is called after a correct character is received.
**         The event is available only when the <Interrupt
**         service/event> property is enabled and either the <Receiver>
**         property is enabled or the <SCI output mode> property (if
**         supported) is set to Single-wire mode.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void IOUPserial_OnRxChar(void)
{
  /* Write your code here ... */
//unsigned char c;
//static unsigned char buf[20],i;
       // if(IOUPserial_RecvChar(&c)==ERR_OK){
        //	menu_TFT_touch_ISR(c);}
		//else IOrecv_char_ISR(0);
		
}






/*
** ===================================================================
**     Event       :  IOUPserial_OnTxChar (module Events)
**
**     Component   :  IOUPserial [AsynchroSerial]
**     Description :
**         This event is called after a character is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void IOUPserial_OnTxChar(void)
{
  /* Write your code here ... */

	//IOUPserial_TransmitedChar_IRQ();
	//istxchar=TRUE;
	__asm(nop);
	//SemaforoTX(TRUE,'w');
}

/*
** ===================================================================
**     Event       :  IOUPserial_OnFullRxBuf (module Events)
**
**     Component   :  IOUPserial [AsynchroSerial]
**     Description :
**         This event is called when the input buffer is full;
**         i.e. after reception of the last character 
**         that was successfully placed into input buffer.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void IOUPserial_OnFullRxBuf(void)
{
  /* Write your code here ... */
	__asm(nop);
	
}



/*
** ===================================================================
**     Event       :  TI1_OnInterrupt (module Events)
**
**     Component   :  TI1 [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void TI1_OnInterrupt(void)
{
  /* Write your code here ... */
	//500useg confirmado 25-jun-26
	//500usegundos el intervalo de tiempo
static uint8 flipflopADC;//uno si otro no, para que sea cada 1mseg
	 // version v40.++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 Tick++;
	 if(ADCstatus){
		if(flipflopADC){flipflopADC=0;//1 milisecond
	                   adcSample_IRQ();}//manda orden de conversion a  los ADCs y enable IRQ input BUSY
		else{flipflopADC=1;}}
	 TM1_IRQ1=0xFF;
	 IRQ_Monitor_System_status_LEDs_v3();
	 IRQ_Monitor_System_Driver_LEDs_v1();//IRQ cada 500useg
	 if(delay_RelojAnalogo_IRQ<RELOJ_ANALOGO_MAX){
		  delay_RelojAnalogo_IRQ++;}
	 
	 //version 39.13.21.xx ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	 
//FIN TIMER-1 IRQ-------------------------------------------------------------------------------------------------
}

/*
** ===================================================================
**     Event       :  IOUPserial_OnFreeTxBuf (module Events)
**
**     Component   :  IOUPserial [AsynchroSerial]
**     Description :
**         This event is called after the last character in output
**         buffer is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void IOUPserial_OnFreeTxBuf(void)
{
  /* Write your code here ... */
//	IO_TX.Flags.bitFlag.freeTXx2=TRUE;//esta libre la transmision.
	serial.vars.Flags.bitF.FreeBuffTX=TRUE;//serial.Flags.bitFlag.FreeBuffTX=TRUE;
	
	
}

/*
** ===================================================================
**     Event       :  CLK_AN_COUNT_OnCapture (module Events)
**
**     Component   :  CLK_AN_COUNT [Capture]
**     Description :
**         This event is called on capturing of Timer/Counter actual
**         value (only when the component is enabled - <Enable> and the
**         events are enabled - <EnableEvent>.This event is available
**         only if a <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void CLK_AN_COUNT_OnCapture(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  Busy_OnInterrupt (module Events)
**
**     Component   :  Busy [ExtInt]
**     Description :
**         This event is called when an active signal edge/level has
**         occurred.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/


/*
** ===================================================================
**     Event       :  Cpu_OnIntRelatedEXP (module Events)
**
**     Component   :  Cpu [MCF52233_80_LQFP]
**     Description :
**         This event is called when one of the interrupt related
**         exceptions occurs. This event is automatically enabled when
**         the <Interrupt related reset> property is disabled and at
**         least one of the <Interrupt related exceptions> is enabled.
**         Disabling this event also disables all the <Interrupt
**         related exceptions>.
**     Parameters  :
**         NAME            - DESCRIPTION
**         ExceptionType   - This parameter
**                           specifies which exception caused this event.
**                           Possible values are:
**                           - SPURIOUS_INTERRUPT - Spurious Interrupt
**                           Exception 
**                           - UNINITIALIZED_INTERRUPT - Uninitialized
**                           Interrupt Exception
**     Returns     : Nothing
** ===================================================================
*/
void Cpu_OnIntRelatedEXP(byte ExceptionType)
{byte n;
  /* Write your code here ... */
	n=ExceptionType;
}

/*
** ===================================================================
**     Event       :  SM1_OnRxChar (module Events)
**
**     Component   :  SM1 [SynchroMaster]
**     Description :
**         This event is called after a correct character is received.
**         The event is available only when the <Interrupt
**         service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void SM1_OnRxChar(void)
{ 
  /* Write your code here ... */
	
}

/*
** ===================================================================
**     Event       :  SM1_OnError (module Events)
**
**     Component   :  SM1 [SynchroMaster]
**     Description :
**         This event is called when a channel error (not the error
**         returned by a given method) occurs. The errors can be read
**         using <GetError> method.
**         The event is available only when the <Interrupt
**         service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void SM1_OnError(void)
{
  /* Write your code here ... */
}

/* END Events */

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.3 [05.09]
**     for the Freescale MCF series of microcontrollers.
**
** ###################################################################
*/

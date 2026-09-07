/* ###################################################################
**     Filename    : Events.h
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
** @file Events.h
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         

#ifndef __Events_H
#define __Events_H
/* MODULE Events */

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "VFDserial.h"
#include "LED2.h"
#include "LED3.h"
#include "LED4_SYS_MON.h"
#include "SDATA.h"
#include "FSYNC.h"
#include "TI1.h"
#include "DB0.h"
#include "DB3.h"
#include "LED_BOOT.h"
#include "LED12_SYS.h"
#include "FSYNC.h"
#include "LED_BOOT.h"
#include "LEDERRCOMM.h"
#include "LED56_COMM_WARN_MON.h"
#include "LED3_Process.h"
#include "LED56_COMM_WARN_MON.h"
#include "TACHO.h"
#include "CLK_AN_COUNT.h"
#include "CNTR_G_AMP1.h"
#include "CNTR_G_AMP2.h"
#include "CNTR_G_AMP3.h"
#include "PIN_DRIVE_AMPLIF.h"
#include "SM1.h"
#include "FREC_SEL0_PIN.h"
#include "FREC_SEL1_PIN.h"
#include "ADC_BAL.h"
#include "Busy.h"
#include "DRIVER_OUT.h"
#include "CS0.h"
#include "I2C.h"
#include "RTC1.h"
#include "CS1.h"
#include "CTS_UART0.h"
#include "DB0.h"
#include "RC.h"
#include "LED3.h"
#include "WP.h"
#include "IOUPserial.h"
#include "CTS_UART0.h"
#include "RTS_UART0.h"
#include "LED12_SYS.h"
#include "TI2.h"

void Cpu_OnCoreWatchdogINT(void);
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


void TI1_delays_OnInterrupt(void);
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

void VFDserial_OnError(void);
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

void VFDserial_OnRxChar(void);
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

void VFDserial_OnTxChar(void);
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

void VFDserial_OnFullRxBuf(void);
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

void VFDserial_OnFreeTxBuf(void);
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

void TI1_OnInterrupt(void);
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

void TACHO_OnCapture(void);
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

void RTC1_OnAlarm(void);
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

void RTC1_OnSecond(void);
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

void SM1_OnTxChar(void);
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

void Busy_OnInterrupt(void);
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
void TI2_OnInterrupt(void);
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

void IOUPserial_OnError(void);
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
**     Returns     : Nothing
** ===================================================================
*/

void IOUPserial_OnRxChar(void);
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

void IOUPserial_OnTxChar(void);
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

void IOUPserial_OnFullRxBuf(void);
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

void IOUPserial_OnFreeTxBuf(void);
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

void CLK_AN_COUNT_OnCapture(void);
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

void EInt1_OnInterrupt(void);
/*
** ===================================================================
**     Event       :  EInt1_OnInterrupt (module Events)
**
**     Component   :  Busy [ExtInt]
**     Description :
**         This event is called when an active signal edge/level has
**         occurred.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void Cpu_OnIntRelatedEXP(byte ExceptionType);
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

void SM1_OnRxChar(void);
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

void SM1_OnError(void);
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

/* END Events */
#endif /* __Events_H*/

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

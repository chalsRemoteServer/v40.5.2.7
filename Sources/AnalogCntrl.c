/*
 * AnalogCntrl.c
 *
 *  Created on: May 18, 2017
 *      Author: chals
 */


/*
 * AnalogCntrl.c
 *
 *  Created on: Apr 19, 2017
 *      Author: chals
 */



#include "AnalogCntrl.h"
#include "Memoria.h"
#include "CNTR_G_AMP1.h"
#include "CNTR_G_AMP2.h"
#include "CNTR_G_AMP3.h"
#include "system.h"


extern unsigned char Parte11AnalogVer;//guarda la version de la Analoga que vamos a controlar
extern unsigned char GananciaAnaloga;//ganancia de la analoga en  AJUSTE_DE_SISTEMA/INGENIERIA_GRACIDA/GANANCIA
unsigned char DriverOutcontrol;//variable que determina si prende ó no la salida driver


//Actualiza el control para controlar la tarjeta analoga los switches digitales
//void update_AnalogaConfig(void){
	
	
	
//}//fin actualizar controles de la tareta anaoga para controlar los switches digitales



/* DriverOutcontrol=ACTIVADO 
 * Activamos el timer para activar el switch que activa la oscilacion hacia la driver
 * Lo activamos despues de 3 segundos de mandar llamar esta funcion
 * */
void init_Analoga(void){
	    DriveOutAttPin(ON);//when init system attenuation DriveOut signal,
	    DriverOutcontrol=ACTIVADO;//activamos la banera que da conteo a los 3 segundos para quitar la atenuacion
	    init_AnalogGainControl();
	    init_FrecuencyControl();//controla el desfasador-select capacitors y la señal de frec select a Driver y pickup
}//fin initializatione de Driver OUT-----------------------------------------------------------




/*fin de la inicializacion de la tarjeta Analoga
  GANANCIA PARA TODOS LOS PRODUCTOS
     ALTA, MEDIA Y BAJA */
void init_AnalogGainControl(void			){
	  if(Parte11AnalogVer=='1'){
		switch(GananciaAnaloga){
		  case ALTA: CNTR_G_AMP1_PutVal(FALSE);//APAGAR todos los switches digitales
		             CNTR_G_AMP2_PutVal(FALSE);//NO ATENUA NINGUN AMPLIFICADOR
		             CNTR_G_AMP3_PutVal(FALSE);
		             break;
		  case MEDIA:CNTR_G_AMP1_PutVal(FALSE);//debug NO ESTA VERIFICADO ESTE VALOR MEDIO COTEJADO CON LOCK
                     CNTR_G_AMP2_PutVal(TRUE);//atenua  una amplificador
                     CNTR_G_AMP3_PutVal(FALSE);
                     break;
		  case BAJA:CNTR_G_AMP1_PutVal(TRUE);//MINIMA GANANCIA EN CADA AMPLIFICADOR
                    CNTR_G_AMP2_PutVal(TRUE);
                    CNTR_G_AMP3_PutVal(TRUE);
                    break;
		  default:GananciaAnaloga=ALTA;
		          CNTR_G_AMP1_PutVal(FALSE);//APAGAR todos los switches digitales
		  		  CNTR_G_AMP2_PutVal(FALSE);//NO ATENUA NINGUN AMPLIFICADOR
		  		  CNTR_G_AMP3_PutVal(FALSE);
		          break; }//fin switch 1
	      }//fin if principal
	     /* CNTR_G_AMP1_PutVal:66    CNTR_G_AMP2_PutVal:64    CNTR_G_AMP3_PutVal:63                         */
	  else {Parte11AnalogVer='2';
	    switch(GananciaAnaloga){
          case ALTA: CNTR_G_AMP1_PutVal(FALSE);//APAGAR todos los switches digitales
		             CNTR_G_AMP2_PutVal(FALSE);//NO ATENUA NINGUN AMPLIFICADOR
		             CNTR_G_AMP3_PutVal(FALSE);
		             break;
		  case MEDIA:CNTR_G_AMP1_PutVal(FALSE);//debug NO ESTA VERIFICADO ESTE VALOR MEDIO COTEJADO CON LOCK
                     CNTR_G_AMP2_PutVal(TRUE);//atenua  una amplificador
                     CNTR_G_AMP3_PutVal(FALSE);
                     break;
		  case BAJA:CNTR_G_AMP1_PutVal(TRUE);//MINIMA GANANCIA EN CADA AMPLIFICADOR
                    CNTR_G_AMP2_PutVal(TRUE);
                    CNTR_G_AMP3_PutVal(TRUE);
                    break;
		  default:GananciaAnaloga=ALTA;
		          CNTR_G_AMP1_PutVal(FALSE);//APAGAR todos los switches digitales
		  		  CNTR_G_AMP2_PutVal(FALSE);//NO ATENUA NINGUN AMPLIFICADOR
		  		  CNTR_G_AMP3_PutVal(FALSE);
		          break; }//fin switch 1
	  }//fin else de analoga version-2	
}//fin initializatione del control de la tarjeta analoga--------------------------------------


/*  Procesadora: AN4 PIN61  Conector3 hacia Analoga 16CONN3
 *   Analoga: 16CONN5 conecta al switch que le da menor ganancia a la señal de salida de la driver
 *            con un 1: se activa el swith por tanto la resistencia  loop de menor valor y por tanto menos ganancia
 *            con 0: hay mayor ganancia porque esta abierto el switch
 * */
void DriveOutAttPin(unsigned char estado){
	
	 if(estado==ON)
		 setReg8(SETAN, 0x10U);
	 else
		 setReg8(CLRAN, 0xEFU); 
		 
}//fin del driver de bajo nivel del pin  de salida fisica el driver out signel---------------

//controla el desfasador-select capacitors y la señal de frec select a Driver y pickup
//Para 1Mhz de pharma debe ser siempre PIN9=1  & PIN13=0;
void init_FrecuencyControl(void){
	  
	   /*NO HACEMOS NADA AQUI PORQUE LOS PINES YA ESTAN CONFIGURADOS CON LOS VALORES
	    *  PIN9=1 & PIN13=0 DESDE EL INIT DE PROCESS EXPERT
	    * */
	
}//fin del control de frecuencia fisico hacia la tarjeta Analoga----------------------------







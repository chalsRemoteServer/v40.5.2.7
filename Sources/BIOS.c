/*
 * BIOS.c
 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */


/*
 * BIOS.c
 *
 *  Created on: Nov 22, 2016
 *      Author: chals
 *      
 *      Basic Input Output System and Subsistem manager
 */

#include "BIOS.h"
#include "VFDmenu.h"
#include "delay.h"
#include "VFD.h"
#include "Memoria.h"



extern void displayBIOS_MEMO_test(unsigned char status,unsigned char memo);

void init_BIOS(void){
	
	         init_VFD_BIOS();
	         Display_BIOS(); //despliega pantalla del bios donde dice que es el bios 
/*DEBUG*/	 evaluar_NVRAM();//si pones primero la EEPROM no funciona despues la NVRAM
	         delay_ms(100);//con esto no hay problemas
	         evaluar_EEPROM();
	
	         delay_ms(500);//tiempo que dura la pantalla BIOS para que la veas
	
	
}//fin BIOS



void evaluar_EEPROM(void){
	
	displayBIOS_MEMO_test(writeReadEEprom(),_EEPROM_);
	
}//fin de evalar eeproms


void evaluar_NVRAM(void){
	
	displayBIOS_MEMO_test(writeReadNVRAM(),_NVRAM_);
}//fin evaluar NVRAM------------------------------------------






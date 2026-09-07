/*
 * delay.h
 *
 *  Created on: Sep 12, 2015
 *      Author: chals
 */

#ifndef DELAY_H_
#define DELAY_H_

#include "PE_Types.h"



extern volatile uint64_t Tick;



void delay_ms(unsigned short int t);
void delay1ms(void);
void delay1us(void);
void delay_us(unsigned short int t);
//dlong gettime(dlong *time);
dlong millis(void);
dlong millis_GetTimeMS(void);
float get_tick_ms_float(void);
uint64_t get_tick_ms_int(void);
uint64_t get_tick_sec(void);
uint8_t get_tick_fraction(void);






#endif /* DELAY_H_ */


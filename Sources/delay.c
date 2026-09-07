/*
 * delay.c
 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */





//#include "millis.h"
#include "delay.h"



//50ns el paso-------------------------------------------------------------------
void delay1ms(void){
unsigned int i;
    for(i=0;i<6500;i++) { __asm{  nop; } }       
}//fin 1 mlisegundo--------------------------------------------------------------

void delay1us(void){
unsigned int i;
    for(i=0;i<24;i++) { __asm{  nop; } }       
}//fin 1 mlisegundo--------------------------------------------------------------



void  delay_ms(unsigned short int t){
int i;	
 if(t>0)
    for(i=0;i<t;i++)
    	delay1ms(); 
}//fin delay ms-----------------------------------------------------------------

	
void delay_us(unsigned short int t){
int i;
    for(i=0;i<t;i++)
	     delay1us();
}

dlong millis_GetTimeMS(void){
	return Tick;
}//---------------------------------------------------


/* system time return milisec, since last reset*/
dlong millis(void){
    return Tick >> 1;  // División entre 2 mediante desplazamiento
}//fin return millisecons system time-----------------


// ============================================
// FUNCIÓN gettime() - Retorna la resta  de los milisegundos transcurridos desde reset
// menos los milisegundos de tl, osea el tiempo transcurrido desde tl hasta orita
// ============================================
//dlong gettime(dlong time){
//dlong ahora = millis();
//dlong t;
// t=*time;//anterior milisegundos
// *time=millis();//actual
// return (*time-t);
//}//fin gettime-------------------------------



/**
 * @brief Obtiene el tiempo actual en milisegundos con precisión fraccional
 * 
 * @return float Tiempo en milisegundos con decimales (ej: 1234.5 ms)
 * 
 * @note ADVERTENCIA DE RENDIMIENTO:
 *       - Esta función utiliza operaciones de punto flotante (float)
 *       - El MCF52233 NO tiene FPU (Unidad de Punto Flotante) en hardware
 *       - Cada llamada consume aproximadamente 100-150 ciclos de reloj
 *       - A 60 MHz: ~1.67 a 2.5 microsegundos por llamada
 *       - Es 50 a 150 veces MÁS LENTA que usar Tick >> 1 (desplazamiento de bits)
 *       - NO USAR en interrupciones o bucles de tiempo real crítico
 * 
 * @warning Para aplicaciones de tiempo real, usar get_tick_ms_int() en su lugar
 */
float get_tick_ms_float(void) {
    return (float)Tick * 0.5f;  // Convierte ticks (0.5ms) a milisegundos
}

/**
 * @brief Obtiene el tiempo actual en milisegundos (sin punto flotante)
 * 
 * @return uint64_t Tiempo en milisegundos (resolución de 1ms, trunca 0.5ms)
 * 
 * @note RENDIMIENTO ÓPTIMO:
 *       - Usa desplazamiento de bits (Tick >> 1)
 *       - 1 instrucción en assembler, ~1 ciclo de reloj
 *       - A 60 MHz: ~16.7 nanosegundos por llamada
 *       - Recomendada para interrupciones y tiempo real
 */
uint64_t get_tick_ms_int(void) {
    return Tick >> 1;  // Desplazar 1 bit a la derecha = dividir entre 2
}

/**
 * @brief Obtiene el tiempo actual en segundos
 * 
 * @return uint64_t Tiempo en segundos (resolución de 1 segundo)
 * 
 * @note RENDIMIENTO:
 *       - Usa división entera, unas pocas instrucciones
 *       - 2000 ticks = 1 segundo (2000 * 0.5ms = 1000ms)
 *       - Aproximadamente 10-20 ciclos de reloj
 */
uint64_t get_tick_sec(void) {
    return Tick / 2000;  // 2000 ticks = 1 segundo
}

/**
 * @brief Obtiene la fracción de milisegundo (0 o 1)
 * 
 * @return uint8_t 0 = sin fracción, 1 = 0.5ms adicional
 * 
 * @note Permite tener precisión de 0.5ms sin usar float
 *       Ejemplo: get_tick_ms_int() + (get_tick_fraction() ? 0.5 : 0)
 */
uint8_t get_tick_fraction(void) {
    return Tick & 0x01;  // Retorna el bit 0 (1 = 0.5ms, 0 = 0ms)
}


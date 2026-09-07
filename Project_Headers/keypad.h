/*
 * keypad.h
 *
 *  Created on: Mar 17, 2022
 *      Author: desarrollo-1
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_



#endif /* KEYPAD_H_ */



struct _SerialControl_{
	union _Bytew_{
		unsigned char ByteF;
		struct{
			unsigned char IRQ_rx:1;//bandera de que ya se envio el buffer completo
			unsigned char x2:1;//seleccionar el buffer que llenamos en tiempo t
			unsigned char x3:1;//indica si borramos registro de datos repetidos de DDS
			unsigned char x4:1;
			unsigned char x67:1;//recargar DDS por cambio de de zoom u otra cosa, se recarga los puntos de deteccion guardados
			unsigned char x5:1;
			unsigned char x6:1;
			unsigned char x7:1;
		}bit;
	  }Byte1;
};


void Controlador_de_Teclado_Keypad(void);

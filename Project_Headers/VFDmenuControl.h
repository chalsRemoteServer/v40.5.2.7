/*
 * VFDmenuControl.h
 *
 *  Created on: Oct 5, 2020
 *      Author: desarrollo-1
 */

#ifndef VFDMENUCONTROL_H_
#define VFDMENUCONTROL_H_



#endif /* VFDMENUCONTROL_H_ */



void ReporteProductoVarRotate(void);
void ErrorAnalogoVarRotate(void);
void PSU_IO_var_rotate(void);
unsigned char SearchSpace(unsigned char mode);
void fillScreenNames(void);
unsigned char iProduct_Mark(unsigned char op,unsigned char *p,unsigned char rowy);	
unsigned char  LimitesMenuPROD(unsigned char sup, unsigned char mode);
unsigned char isSameScreeniProduct(unsigned char dir,unsigned char iProd);
unsigned char is_iProduct_1(void);
unsigned char convertPOSY2iProduct(unsigned char sup,unsigned char pos);
unsigned char convertiProduct2POSY(unsigned char iProd);
unsigned char searchNextiProduct(unsigned char dir,unsigned char iProd);
unsigned char getiProducSelected(unsigned char posy);
char rotateEntradasSistemasVars(unsigned char var);
unsigned short int  digito2posicion(unsigned short int digito);
void rotateContrasenaVars(unsigned char psw);
void configReloj(unsigned char sens); //configura el reoj del sistema
void gananciaValuesRotary(unsigned char tarjeta);
unsigned char iProductEmpty(void);

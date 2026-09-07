/*
 * VFDisplay.h
 *
 *  Created on: Nov 8, 2019
 *      Author: desarrollo-1
 */

#ifndef VFDISPLAY_H_
#define VFDISPLAY_H_



#endif /* VFDISPLAY_H_ */

extern struct _GLOBAL_CONTEXTO gxc,gxc2,gxc3,gxc4,gxc5;




void displayControladorPantalla(unsigned char destino);
void varcontexto(unsigned char destino);
void saveTextBuffer(unsigned char *p);
void Display_rotate_Ganancia_var(unsigned char posx,unsigned char posy);
void Display_rotate_MarcarAltura(unsigned char posx,unsigned char posy);
//unsigned short int f(unsigned char x);
void convert2ascii(unsigned char *array,unsigned char size);	
void display_pushFIFO_UShortInt(unsigned short *ShortNum,unsigned char index,unsigned char op,unsigned char Row,unsigned char contxt);
void display_UInt_CharsSelected(unsigned short int var,unsigned char *index,unsigned char posy,unsigned char posInit);
unsigned short int f2(unsigned char x);
void calculoOrdenCharsUInt_wSelectChar(unsigned char B1,unsigned char *b,unsigned char *a);
void displayOrdenCharsUInt_wSelectChar(unsigned char *bb,unsigned char *aa,unsigned char posInit,unsigned char posy);
unsigned char rotateSensor(unsigned char sensor);
void displaySensorActivation(unsigned char sensor, unsigned char pos);
unsigned char rotateSensorHighNibble(unsigned char sensor);
void displayUChar(unsigned char *p,unsigned short posx,unsigned short posy);
unsigned char getHighNibbleSensor(unsigned char sensor);
void displayAltoSiBajoSi_Sensor(unsigned char sensor,unsigned short posy);
unsigned char rotateCursorBloquedo(unsigned char sensorbloqueado,unsigned char *posicion);
void displayrotateCursorBloquedo(unsigned char sensorBloqueado,unsigned short posx,unsigned short posy,unsigned char *cursorSelect);
unsigned char displayBloqueoDigitoControl(unsigned char op,unsigned char *bloqueado,unsigned char *digito,unsigned char posx,unsigned char posy);
void displayControlCursorSubMenusKey(unsigned char direccion,unsigned char *posx, unsigned char *posy,unsigned char esqx,unsigned char *posiciones,unsigned char posx0);
void display_Prueba_de_Sistema(unsigned char segundo,unsigned char mseg,unsigned char *digito,unsigned char posx,unsigned char posy);
unsigned char rotate_Prueba_de_Sistema(unsigned char direccion,unsigned char *seg,unsigned char *mseg,unsigned char digito);
unsigned char *getLabelAux(unsigned char aux,unsigned char index);
void display_Permitir_No(unsigned char sensor,unsigned char posy);
void display_Alto_Bajo_Si(unsigned char sensor,unsigned char posy);
void display_LabelAux(unsigned char aux,unsigned char label,unsigned char posy);
void display_auxRele(unsigned char sensorRele,unsigned char posy);
void display_tiempo_activo_aux_label(unsigned char posy);
unsigned char rotateLabelAux(unsigned char label);
unsigned char rotate_auxRele(unsigned char sensorRele);
unsigned char displayPruebaSistema_float(unsigned char dir,unsigned char *seg,unsigned char *mseg,unsigned char *digito,unsigned char posy);
unsigned char DisplayInsertSpaceTextProc(unsigned char indice);
void DisplayCharTextProc(unsigned char pos,unsigned char dato);
void DisplayCursorTextProc(unsigned char pos_actual, unsigned char pos_anterior);
unsigned char DisplayRecorrerIndiceIzqTextProc(unsigned char ii,unsigned char IzqDer);	
void displayUINT_var(unsigned short int posx,unsigned short int posy,unsigned short  int *pn,unsigned char operacion);
void display_Voltage_Mediciones(unsigned char *p,unsigned char voltaje,struct _GLOBAL_CONTEXTO *g,unsigned char signo);
void display_B_bold_DDS(unsigned char disp);
void displayPSU_IO_var(void);
void displayErrorAnalogoVar(void);
void displayReporteProductoVar(void);
void displayBorrarContadores_var(void);
void display_pushFIFOcOP_BorrarContadores_var(void);
void displayCountingBIOS_(unsigned char subsystem);
void displayBIOS_MEMO_test(unsigned char status,unsigned char memo);
void displayNuevoProductoTitle(void);
void displayNuevoProductoXY(unsigned short int y,void (*func)());
void displayNumbers(unsigned char n);
void  displayEntradadeSistemas(unsigned char var);	
void displayZoomDDS_var(unsigned char operacion);
unsigned short int displayZoomDDS_varxDigito(unsigned short int posx,unsigned char operacion);
void display_pushFIFOcOP_Phase_var(unsigned short int posx,unsigned short int posy);
void displayPhase_var(unsigned short int posx,unsigned short int posy);
void displayControlContrasenaVal(unsigned char varnum);
unsigned char  displayTime(unsigned char t);//desplega un valor de tiempo a la vez
void displayGananciaValues(unsigned char tarjeta);
void displaySeleccioniProd_lugarOcupado(void(*func)(void));
unsigned char setLimitesMenuPROD(unsigned char mode);
void displayAnguloCompletado(void);
void displayAnguloVibracion(unsigned char Ang,unsigned char frac,unsigned char veces);
void displayZoom(unsigned char posx,unsigned char posy);
void display_Phase_DDS(unsigned char posx,unsigned char posy,unsigned char *p);
void display_Name_Product(unsigned posx,unsigned char posy);
void Display_Ajuste_Tiempo_Rechazo(void);
void save_Pixels(signed short int x1,signed short int y1,unsigned char ux,unsigned char uy);
unsigned char get_Pixels_Saved(signed short int *x,signed short int *y,unsigned char *ux4,unsigned char *uy4,unsigned char *swap);
unsigned char getRegistro_X_bit(unsigned char *Reg,unsigned char x);
void display_Linea_10_DDS(unsigned char DDSaux);
void display_Sensibilidad_Geometrico(unsigned char v);
void display_Sensibility_Geometry_Line(unsigned char v);
void Display_Cuenta_Rechazos(unsigned short int rechazos);
unsigned short int f(unsigned char x);
void display_cursor_main(unsigned char *index);

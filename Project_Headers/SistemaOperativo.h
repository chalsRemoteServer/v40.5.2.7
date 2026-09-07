/*
 * SistemaOperativo.h
 *
 *  Created on: Oct 3, 2017
 *      Author: chals
 */

#ifndef SISTEMAOPERATIVO_H_
#define SISTEMAOPERATIVO_H_



#endif /* SISTEMAOPERATIVO_H_ */




#define AJVIB1   0x11
#define AJVIB2   0x12
#define AJVIB3   0x13
#define POIN1    0x14
#define POIN2    0x15
#define POIN3    0x16
#define AJPPES1  0x17
#define AJPPES2  0x18
#define PDDES1   0x19
#define PDDES2   0x1A
#define PDD1     0x1B
#define PDD2     0x1C
#define GRAFICANDO 0x1D

//timepo con interrupcion de 1milisegundo
#define _1min_           3600000//1ms=1000*60seg=60000=1seg*60=3600000=1min*10=10min=36000000
#define _10min_          _1min_*10
#define MAXIMO_LONG_INT  4200000000// CUATRO bytes para tipo long int
#define MENU_ESPERA_SIN_ACTIVIDAD _10min_//tiempo de espera antes de volver all portal inicio
#if(MENU_ESPERA_SIN_ACTIVIDAD>MAXIMO_LONG_INT)
  #error "Constante de Menu espera es mayor que el tipo Long Int"
#endif

//maquina de estados TRANSMITIR to DDS
#define DDSe1      0
#define DDSe_wait  0x10
#define DDSe_wait2 0x11
#define DDSe_wait3 0x12
#define DDSe_wait4 0x13
#define DDSe_wait5 0x14
#define DDSe_wait5x 0x15
#define DDSe2      0x16
#define DDSe3      0x17
#define DDSe4      0x18
#define DDSe5      0x19
#define DDSe6      0x1A
#define DDSe7      0x1B
#define DDSe8      0x1C
#define DDSe9      0x1D
#define DDSe10     0x1E
#define DDSe11     0x1F
#define DDSp1      0x21
#define DDSp2      0x22 
#define DDSp3      0x23
#define DDSp4      0x24 
#define DDSp5      0x25
#define DDSp6      0x26
#define DDSp7      0x27
#define DDSp8      0x28
#define DDSp9      0x29
#define DDSp10     0x2A
#define DDSp11     0x2B
#define DDSb1      0x2C
#define DDSb2      0x2D
#define DDSb3      0x2E
#define DDSb4      0x2F
#define DDSb5      0x30
#define DDbd1      0x31
#define DDbd2      0x32
#define DDbd3      0x33 
#define DDbd4      0x34
#define DDbd5      0x35
#define DDbd6      0x36
#define DDbd7      0x37
#define DDbd8      0x38
#define DDbd9      0x39
#define DDbd10     0x3A
#define DDbd11     0x3B
#define DDbd12     0x3C
#define BDe_wait   0x3D//MAquina de estados de BARA DE DETECCION
#define BDe_wait1  0x3E
#define BDe_wait2  0x3F
#define BDe_wait3  0x40
#define BDe_wait4  0x41
#define BDe_wait5  0x42
#define BDe1       0x43
#define BDe2       0x44
#define BDe3       0x45
#define BDe4       0x46
#define BDe5       0x47
#define BDe6       0x48
#define BDe7       0x49
#define BDe8       0x4A
#define BDe9       0x4B
#define BDe10      0x4C
#define BDe11      0x4D
#define BDe12      0x4E
#define BDe13      0x4F
#define BDe14      0x50
#define BDe15      0x51
#define BOXe_wait  0x52
#define BOXe_wait2 0x53
#define BOXe_wait3 0x54
#define BOXe_wait4 0x55
#define BOXe1      0x56
#define BOXe2      0x57
#define BOXe3      0x58
#define BOXa_wait1 0x59
#define BOXa_wait2 0x5A
#define BOXa_wait3 0x5B
#define BOXa_wait4 0x5C
#define DDSe_wait3x 0x5D
#define BOXa1      0x5E
#define BOXa2      0x5F
#define BOXa3      0x60
#define BDN01      0x81     //baRRA DETECCION comando POSICION Y 5 CHARS 
#define BDN02      0x82
#define BDN03      0x83
#define BDN04      0x84
#define BDN05      0x85
#define BDN06      0x86
#define BDN07      0x87
#define BDN08      0x88
#define BDN09      0x89
#define BDN10      0x8A
#define BDN11      0x8B
#define BDN12      0x8C
#define BDN13      0x8D
#define BDN14      0x8E
#define BDN15      0x8F
#define BDPC01     0x91  //Barra detectora comando  pos char 
#define BDPC02     0x92  //Barra detectora comando  pos char 
#define BDPC03     0x93  //Barra detectora comando  pos char 
#define BDPC04     0x94  //Barra detectora comando  pos char 
#define BDPC05     0x95  //Barra detectora comando  pos char 
#define BDPC06     0x96  //Barra detectora comando  pos char 
#define BDPC07     0x97  //Barra detectora comando  pos char 
#define BDPC08     0x98  //Barra detectora comando  pos char 
#define BDPC09     0x99  //Barra detectora comando  pos char 
#define BDPC10     0x9A  //Barra detectora comando  pos char 
#define BDPC11     0x9B  //Barra detectora comando  pos char 
#define BDP01      0xA1  //barra detectora comando solo position
#define BDP02      0xA2
#define BDP03      0xA3
#define BDP04      0xA4
#define BDP05      0xA5
#define BDP06      0xA6
#define BDP07      0xA7
#define BDP08      0xA8
#define BDP09      0xA9
#define BDC01      0xB1 //BARRA DETECTORA  CARACTER
#define BDC02      0xB2
#define BDC03      0xB3
#define BDC04      0xB4
#define BDC05      0xB5
#define BDC06      0xB6
#define DDSp1x      0xC1
#define DDSp2x      0xC2 
#define DDSp3x      0xC3
#define DDSp4x      0xC4 
#define DDSp5x      0xC5
#define DDSp6x      0xC6
#define DDSp7x      0xC7
#define DDSp8x      0xC8
#define DDSp9x      0xC9
#define DDSp10x     0xCA
#define DDSp11x     0xCB
#define BDe_wait6   0xCD

void Control_Operativo_de_Escape_de_Menus(void);
unsigned char SemaforosTemporizadosController(unsigned char semaforo,unsigned char status,unsigned short int delay1);
void monitorDelayDisplay(unsigned short int delay1,unsigned short int delay2);
void menuTemporizadorOperativo(void);
unsigned long int SOmenuEntradaTacoMonitor(unsigned long int counter,unsigned long int time);
void DDSsignalProcessor_Operativo(void);
void Display_DDS_TRANSMISSOR_CONTROLLER(void);
void FIFO_Display_DDS_Char_clean(void);
void FIFO_Display_DDS_Pixel_clean(void);
void Display_Y_Barra_Deteccion_Controlador_Operativo(void);
void Display_Nums_Chars_BarraDeteccion_DriverController(void);
unsigned char Display_Construir_Box(unsigned char box2build,unsigned char metabox,unsigned char modo);
//void isCommand(unsigned char c);
void  Procesador_Operativo_de_Graphicos(void);
void DDS_repaint_Zoom(void);



/*
 * queue.h
 *
 *  Created on: Oct 3, 2017
 *      Author: chals
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include "PE_Types.h"


#ifndef SIZE_FIFO_OFFSET //tamaño del array del filtro digital del offset
#define SIZE_FIFO_OFFSET 32
#endif


#if(SIZE_FIFO_OFFSET==32)
#define  DIV_OFFSET 5 //DIVISOR PARA offset  2^5=32
#else
#error "No coincide el tamaño matematico del filtro con el divisor"
#endif




#define SIZE_IO_TX_BUFFER 85 //ARRAY donde se guarda la direccion de la cabeza del paquete a enviar
#define SIZE_B 60//ARRAY donde se guarda los paquetes que se van a enviar

#define SIZE_FIFO_BOX 10
#define SIZE_FIFO SIZE_FIFO_BOX//30  //array size where the ADC's values are saved

#define SIZE_BARR_NUM 22//size fifo to displat numeros and position where menus has barra detection
#define SIZE_FIFO_CMD_DDS 30 //tamaño de la FIFO para transmitir comando DDS al touch

#define SIZE_BUFFER_TX 90 //BUFER DE TRANSMISION SERIAL ALA O
#define SIZE_BUFFER_DDS_FIFO 50
#define SIZE_TEMP_PIXEL  6//tamaño del buffer para guardar memoria de pixels del display DD
#define SIZE_DDS_PIXEL   40//para pintar el pixel en tiempo real
#define SIZE_DDS_ZOOM   2000
#define SIZE_X  24//array zoom repaint char pantalla 0
#define SIZE_Y  129//array zoom repaint char pantalla0

#define SIZE_SER_KEYPAD 10 //TAMAÑO de la fifo del serial del keypad de entraada
#define SIZE_KEYPAD_IN 10 //buffer de entrada de chars de serial de keypad
#define SIZE_BUFFER1  20 //buffers de rx y tx de serial a IOUP
#define SIZE_BUFFER2  10//tamaño de rx y tx de i2c fifo memorias
#define SIZE_BUFFER5  10//TAMAÑO del fifo para transmitir a MCU-TFT-IO
#define SIZE_BUFFER6  30//TAMAÑO del fifo para recibir desde MCU-TFT-IO

#define SIZE_DSP_FIFOs 10


struct NodoBox{
	struct NodoBox *next;
//	struct NodoBox *prev;
	unsigned char box;
};

// version-1
/* DEPRECATED
struct _Vector__{
	union _Byte_{
		unsigned char ByteFlags;
		struct{
			unsigned char FreeBuffTX:1;//bandera de que ya se envio el buffer completo
			unsigned char flipflopBuff:1;//seleccionar el buffer que llenamos en tiempo t
			unsigned char resetDDS:1;//indica si borramos registro de datos repetidos de DDS
			unsigned char DDS_ACTIVA:1;
			unsigned char x5:1;
			unsigned char x6:1;
			unsigned char x7:1;
		}bitFlag;
	  }Flags;
   struct _Control_{
	  unsigned char timercomms;//timer de comunicaciones	  
      unsigned char ncountNopeAns;//contamos las no respuestas numero.
      unsigned char ledComunicaciones;
	  }cntrl;	  
	  
//	unsigned char (*appendDDS)(signed short int x,signed short int y);//agrega dato al final de la lista	
//	void (*popDDS)(signed short int *x,signed short int *y);//quitar el prmer elemento de la fifo DDS
    unsigned char buffer[SIZE_BUFFER_TX];//buffer de transmission
//    unsigned char buffer2[SIZE_BUFFER_TX];
    unsigned char *pop;//primer de la FIFO para salir
//    unsigned char *last;//primer de la FIFO para entrar
    unsigned char *head;//indice 0 del buffer
    unsigned char *tail;//ultimo indice del buffer
    unsigned char *push;//pointer buffer 2
//    unsigned char *tail2;//pointer to tail buff2
//    unsigned char *head2;//pointer to head buff2
    void (*appendByte)(unsigned char n);//agregar byte al buffer de transmision
    void (*vaciarbuff)(unsigned char *p1,unsigned char *p2,unsigned char size);
    void (*resetFIFO)(void);
    unsigned char ncount;//cuenta los nodos llenos en la fifo
    unsigned char test1;
    //unsigned char test2[50];
};*/


/*  version 290322-1747  add EnableComunicationsTX 
 *  version 310322-1238  modify ledcomms control
 * */
struct Serial_Bandera{
    union _Byte1_{
		unsigned char ByteF;
		struct{
			unsigned char FreeBuffTX:1;//bandera de que ya se envio el buffer completo
			unsigned char flipflopBuff:1;//seleccionar el buffer que llenamos en tiempo t
			unsigned char resetDDS:1;//indica si borramos registro de datos repetidos de DDS
			unsigned char DDS_ACTIVA:1;
			unsigned char x4:1;//recargar DDS por cambio de de zoom u otra cosa, se recarga los puntos de deteccion guardados
			unsigned char EnableComunicationsTX:1;
			unsigned char x6:1;
			unsigned char x7:1;//timercomms
		}bitF;
	  }Flags;
   struct _VAR_{
		   unsigned char ncountNopeAns;	 
		   unsigned char timercomms;
	  }var;    	  
};

/* version 110422-1119 */
struct IO_Bandera{
    union _Byte2_{
		unsigned char Outs2;
		struct{
			unsigned char Rechazo:1;//bandera de que ya se envio el buffer completo
			unsigned char Warnning:1;//seleccionar el buffer que llenamos en tiempo t
			unsigned char Fail:1;//indica si borramos registro de datos repetidos de DDS
			unsigned char Lock:1;
			unsigned char Aux_out:1;//recargar DDS por cambio de de zoom u otra cosa, se recarga los puntos de deteccion guardados
			unsigned char PFVE:1;
			unsigned char Gotis:1;
			unsigned char x7:1;//timercomms
		}bitA;
	  }Salidas;
	union _Byte3_{
	  		unsigned char IN2;
	  		struct{
	  			unsigned char PACK_CHK:1;//bandera de que ya se envio el buffer completo
	  			unsigned char GATING:1;//seleccionar el buffer que llenamos en tiempo t
	  			unsigned char AUX1:1;//indica si borramos registro de datos repetidos de DDS
	  			unsigned char AUX2:1;
	  			unsigned char SYS_CHK:1;//recargar DDS por cambio de de zoom u otra cosa, se recarga los puntos de deteccion guardados
	  			unsigned char LockSW:1;
	  			unsigned char ResetSW:1;
	  			unsigned char x7:1;//timercomms
	  		}bitB;
	  	  }Ent;
   unsigned char TACHO;	  	    
};





/*version 310322-1641 add reset genaral */
struct _FIFO_1byte_{//FIFO PARA UNA VARIABLE para un byte
    unsigned char *pop;//primer de la FIFO para salir
    unsigned char *head;//indice 0 del buffer
    unsigned char *tail;//ultimo indice del buffer
    unsigned char *push;//pointer buffer 2
	unsigned char (*popf)(unsigned char *n,struct _FIFO_1byte_ *s);//quitar el prmer elemento de la fifo DDS
	unsigned char (*appendByte)(unsigned char  n,struct _FIFO_1byte_ *s);//agregar byte al buffer de transmision
    void (*vaciarbuff)(unsigned char *p1,unsigned char *p2,unsigned char size);
    void (*resetFIFO)(struct _FIFO_1byte_ *s);
    unsigned char ncount;//cuenta los nodos llenos en la fifo
    unsigned char size; //size de la fifo
};

// Nueva estructura para appendByte con un solo parámetro
struct _FIFO_1byte_v2{
    unsigned char *pop; //apunta al siguiente a sacar
    unsigned char *head; //elemento 0
    unsigned char *tail;  //ultimo elemento
    unsigned char *push; //apunta al siguiente location libre
    unsigned char mode; //modo debug=1 o modo normal=0 {en debug el pop siempre es  elemento=0}
	unsigned char (*dequeue)(unsigned char *n,struct _FIFO_1byte_v2 *s);// Saca el dato del buffer (head avanza)
	unsigned char (*append)(unsigned char n,struct _FIFO_1byte_v2 *s);  // SOLO UN PARÁMETRO
    unsigned char (*peek)(struct _FIFO_1byte_v2 *s);//Solo mira el dato (head NO avanza)
    unsigned char (*search)(unsigned char n,struct _FIFO_1byte_v2 *s);
	void (*vaciarbuff)(unsigned char *p1,unsigned char *p2,unsigned short int size);
    void (*resetFIFO)(struct _FIFO_1byte_v2 *s,unsigned char size);
    unsigned char ncount;
    unsigned char size;
};//_FIFO_1byte_v2---------------------------------------------

// Nueva estructura para appendByte con un solo parámetro
struct _FIFO_volatile_int16_v1{//se modifica por HW o IRQ
    volatile int16  *pop; //apunta al siguiente a sacar
    volatile int16  *head; //elemento 0
    volatile int16  *tail;  //ultimo elemento
    volatile int16  *push; //apunta al siguiente location libre
    unsigned char mode; //modo debug=1 o modo normal=0 {en debug el pop siempre es  elemento=0}
	uint8  (*dequeue)(int16 *n,volatile struct _FIFO_volatile_int16_v1 *s);// Saca el dato del buffer (head avanza)
	uint8  (*append)(int16 n,volatile struct _FIFO_volatile_int16_v1 *s);  // SOLO UN PARÁMETRO
	int16  (*peek)(volatile struct _FIFO_volatile_int16_v1 *s);//Solo mira el dato (head NO avanza)
	uint8  (*search)(int16 n,volatile struct _FIFO_volatile_int16_v1 *s);
	void (*vaciarbuff)(int16 *p1,int16 *p2,uint16  size);
    void (*resetFIFO)(volatile struct _FIFO_volatile_int16_v1 *s,uint8 size);
    volatile uint8  ncount;
    uint8  size;
};//fin _FIFO_int16_v1-------------------------------------------

struct _FIFO_int16_v1{//No se modifica por HW o IRQ
    int16  *pop; //apunta al siguiente a sacar
    int16  *head; //elemento 0
    int16  *tail;  //ultimo elemento
    int16  *push; //apunta al siguiente location libre
    unsigned char mode; //modo debug=1 o modo normal=0 {en debug el pop siempre es  elemento=0}
	uint8  (*dequeue)(int16 *n,struct _FIFO_int16_v1 *s);// Saca el dato del buffer (head avanza)
	uint8  (*append)(int16 n,struct _FIFO_int16_v1 *s);  // SOLO UN PARÁMETRO
	int16  (*peek)(struct _FIFO_int16_v1 *s);//Solo mira el dato (head NO avanza)
	uint8  (*search)(int16 n,struct _FIFO_int16_v1 *s);
	void (*vaciarbuff)(int16 *p1,int16 *p2,uint16  size);
    void (*resetFIFO)(struct _FIFO_int16_v1 *s,uint8 size);
    uint8  ncount;
    uint8  size;
};//fin _FIFO_int16_v1-------------------------------------------





struct etapaDSP_volatile{
   volatile struct _FIFO_volatile_int16_v1 X;	
   volatile struct _FIFO_volatile_int16_v1 Y;	
   volatile uint8 sem;//1: stop, esta siendo ocupado.
};


struct etapaDSP{
   struct _FIFO_int16_v1 X;	
   struct _FIFO_int16_v1 Y;	
   volatile uint8 sem;//1: stop, esta siendo ocupado.
};


struct _Signale_{
   volatile signed short int Raw_X[SIZE_DSP_FIFOs];//Datos crudos del ADC
   volatile signed short int Raw_Y[SIZE_DSP_FIFOs];
   signed short int Offset_X[SIZE_DSP_FIFOs];
   signed short int Offset_Y[SIZE_DSP_FIFOs];
   signed short int Cent_X[SIZE_DSP_FIFOs];//centrado X
   signed short int Cent_Y[SIZE_DSP_FIFOs];
   signed short int Filt_X[SIZE_DSP_FIFOs];
   signed short int Filt_Y[SIZE_DSP_FIFOs]; 
   signed short int Rot_x[SIZE_DSP_FIFOs];
   signed short int Rot_y[SIZE_DSP_FIFOs];//config de vibracion
   signed short int DDS_X[SIZE_DSP_FIFOs];
   signed short int DDS_Y[SIZE_DSP_FIFOs];
   signed short int Barr[SIZE_DSP_FIFOs];

   struct etapaDSP_volatile Raw;
   struct etapaDSP CentOffset;
   struct etapaDSP Filt;
   struct etapaDSP Rot;
   struct etapaDSP DDS_; 
      
};//_Signale_ -----------------















/* version 110422-1119 */
struct  _SERIAL_{
	struct _FIFO_1byte_ rx;
	struct _FIFO_1byte_ tx;
	struct Serial_Bandera vars;
	struct IO_Bandera IO;
};//serial-------------------------------------------------------------


struct _CONTROL_IIC_{
    union _Byte11_{
		unsigned char ByteF;
		struct{
			unsigned char x0:1;//bandera de que ya se envio el buffer completo
			unsigned char x1:1;//seleccionar el buffer que llenamos en tiempo t
			unsigned char x2:1;//indica si borramos registro de datos repetidos de DDS
			unsigned char x3:1;
			unsigned char x4:1;//recargar DDS por cambio de de zoom u otra cosa, se recarga los puntos de deteccion guardados
			unsigned char x5:1;
			unsigned char x6:1;
			unsigned char x7:1;//timercomms
		}bitG;
	  }Flags;
   struct _VAR19_{
		   unsigned char y0;	 
		   unsigned char y1;
	  }var;    	  
};


struct  _IIC_{
	struct _FIFO_1byte_ rx;
	struct _FIFO_1byte_ tx;
	struct  _CONTROL_IIC_ cntrl;
};//serial-------------------------------------------------------------


struct _COMMs_{ //comunicaciones
   struct _FIFO_1byte_v2 TxDisp; //FIFO serial a display-IO
   struct _FIFO_1byte_v2 RxDisp; //FIFO serial a display-IO
      
};//fin de estructura de control de comunicaciones



struct _Recursos{
	 //struct _FIFO_func_  f;//funciones de la fifo de VFDprocesos
	 struct _FIFO_1byte_ VFDprocesos;//procesos que quieren el recurso
	 struct _FIFO_1byte_ VFD;//fifo de VFD 
	 struct _FIFO_1byte_ IO;//fifo de IO
	 struct _FIFO_1byte_v2 COMM_IO;//commnicacion con IO
	 unsigned char (*insertVFDProcess)(unsigned char proceso);
	 unsigned char (*solicitar)(unsigned char recurso,unsigned char PID);
	 	 	  void (*devolver)(unsigned char,unsigned char);
   	 unsigned char (*get_inst)(void);//obtener instancia
   	          void (*dev_inst)(unsigned char inst);//devolver instancia
	 struct{
		unsigned short int control;//VFD numero de registros,que guarda quien esta en la FIFO   
		unsigned short int control_IO;//IO numero de procesos/hilos a manejar
		unsigned char VFD_pid;//pid que ocupa el VFD
		unsigned char control_uchar;
		unsigned char char1[8];
		unsigned char asignado;//a quien es asigando el recurso de VFD
		unsigned char asignadoIO;//a quien es asiganado el recurso IO
	 }v;//variable
	
};//fin recursos ------------------------------------------
	




struct _Comando_DDS{//comando DDS para graficar en touch
	union Bytex{
	   unsigned char Bnderas;
	   struct{
		   unsigned char DDS_Reload:1;//cuando se recarga el DDS por un zoom CON LOS valores anteriores guardadso de las detecciones
		   unsigned char Apagando:1;//le dice que nos vamos a salir del menu DDS, para que no mande datos del DDS en otro menu
	       unsigned char EventDDS:1;//nos dice que tenemos autorizacion para ejecutar graficacion en DDS		 
		   unsigned char DDS1_BORRAR:1;//Borrando de el DDS
		   unsigned char DDS1_TIMER:1;
		   unsigned char clean_Buffers:1;//para decirle cuando limpiar los buffers que guardan los datos de los puntos pintados en el DDS para repintrlos
		   unsigned char x7:1;
		   unsigned char debug:1;
		   unsigned char x6:1;
	   }bit;
	}Bnderas;
	
	struct _Display_{
		unsigned char flag_TX_Pixel; //timer para desplegar en DDS
		
	}display;
	
	struct _TOUCHSCREEN_{
		signed short int bufferX[SIZE_BUFFER_DDS_FIFO];//buffer DDS fifo TRansmision Serial
		signed short int bufferY[SIZE_BUFFER_DDS_FIFO];//buffer y DDS FIFO Transmision Serial
		signed short int *headx; //head de buffer x
		signed short int *tailx; //head de buffer y
		signed short int *popx;  //pointer to pop from fifo
		signed short int *pushx; //
		signed short int *heady; //head de buffer x
		signed short int *taily; //head de buffer y
		signed short int *popy;  //
		signed short int *pushy;
	    unsigned char ncount;//numero de nodos ocupados en la fifo
	}touch;
	
	struct _SAVE_PIXELS_{
		signed short int xPixel[SIZE_TEMP_PIXEL];
		signed short int yPixel[SIZE_TEMP_PIXEL];
		signed short int *p[8];
#if(SIZE_TEMP_PIXEL<254)		
		unsigned char ncount;
#else   
	    unsigned short int ncount;
#endif	    
	}SaveTempPix; 	
		
//	unsigned char (*pop)(signed short int *x,signed short int *y);//pop function
//    unsigned char (*push)(signed short int x,signed short int y);
    unsigned char (*remove)(signed short int *x,signed short int *y);//quita el nodo primer en indicce 0
    unsigned char (*append)(signed short int x,signed short int y);//pone un nodo al final dela cola
    
    struct _DispayPixel_{//para pintar el pixel
    	unsigned char *pop,*popx,*popy;
    	unsigned char *push,*pushx,*pushy;
    	unsigned char *tail,*tailx,*taily;
    	unsigned char *head,*headx,*heady;
    	unsigned char buffX[SIZE_DDS_PIXEL];
    	unsigned char buffY[SIZE_DDS_PIXEL];
    	unsigned char pen[SIZE_DDS_PIXEL];
#if(SIZE_DDS_PIXEL<254)
    	unsigned char ncount;
#else 
    	unsigned short int ncount;
#endif    	
    }pixel;
        
    struct _Repaint_ZOOM{
    	//signed short int xx[SIZE_DDS_ZOOM];
        //signed short int yy[SIZE_DDS_ZOOM];
    	unsigned char  xy[SIZE_Y][SIZE_X];
    	unsigned char xy0[SIZE_Y][SIZE_X];
    	unsigned short int ii;
    	unsigned char j,k,uy;
    	unsigned char ix,iy,b;
    	unsigned char x1[100],y1[100];//depurar
    	unsigned short int debug5;
    	unsigned short int debug6;
    }rePaint;    
}; 


unsigned char FIFO_DDS_pop(unsigned char *dato);
void init_FIFO_DDS(void);
unsigned char isFIFO_DDS_empty(void);
unsigned char isFIFO_DDS_pixel_llena(void);
unsigned char isFIFO_DDS_pixel_empty(void);	
unsigned char FIFO_DDS_pixel_pop(unsigned char *x,unsigned char *y,unsigned char *pen);
void display_DDS_transmitter_Controller(void);
void init_FIFO_DDS_Display_Pixel2(void);
void init_FIFO_DDS_Display_Pixel2_v2(void);
unsigned char isFIFO_DDS_Display_Pixel_llena2(void);
unsigned char isFIFO_DDS_Display_Pixel_empty2(void);
unsigned char FIFO_DDS_Display_Pixel_push(unsigned char datox,unsigned char datoy,unsigned char pen);
unsigned char FIFO_DDS_Display_Pixel_pop2(unsigned char *x,unsigned char *y,unsigned char *p);
unsigned char isFIFO_DDS_Display_Pixel_empty2_v2(void);
unsigned char *FIFO_DDS_next_X(unsigned char *p);
unsigned char isFIFO_DDS_Display_Pixel_empty2(void);
void init_FIFO_DDS_pixel(void);
void FIFO_DDS_pixels_push(unsigned char x,unsigned char y,unsigned char pen);
unsigned char FIFO_DDS_Display_Pixel_push(unsigned char datox,unsigned char datoy,unsigned char pen);
unsigned char isFIFO_DDS_Display_Pixel_llena2_v2(void);
unsigned char isFIFO_DDS_pixel_llena(void);
unsigned char isFIFO_DDS_Display_Pixel_empty_v2(void);
unsigned char isFIFO_DDS_pixel_empty(void);
unsigned char FIFO_DDS_Display_Pixel_pop2(unsigned char *x,unsigned char *y,unsigned char *p);	
unsigned char FIFO_DDS_pixel_pop(unsigned char *x,unsigned char *y,unsigned char *pen);
void init_queues(void);
unsigned char FIFO_Display_DDS_Pixel_pop2(unsigned char *x,unsigned char *y,unsigned char *p);
unsigned char FIFO_Display_DDS_Pixel_push_v2(unsigned char datox,unsigned char datoy,unsigned char pen);
unsigned char FIFO_Display_DDS_Pixel_pop2_v2(unsigned char *x,unsigned char *y,unsigned char *p);
unsigned char FIFO_Display_DDS_Pixel_push(unsigned char datox,unsigned char datoy,unsigned char pen);
unsigned char *FIFO_DDS_next_X(unsigned char *p);	
unsigned char isFIFO_DDS_Display_Char_empty2(void);
void init_FIFO_DDS_Display_Char2(void);
unsigned char FIFO_Display_DDS_Char_pop2(unsigned char *x,unsigned char *y);
unsigned char FIFO_Display_DDS_Char_push(unsigned char datox,unsigned char datoy);
unsigned char *gotonext(unsigned char *last,unsigned char *first,unsigned char *tail);
unsigned char FIFO_displayBox_pop(void);
void BarraDet_displayUINT_var(unsigned char posx,unsigned char posy,unsigned short int *usint);
unsigned char FIFO_displayBox_push(unsigned char box);
unsigned char FIFO_displayBox_isEmpty(void);
void FIFO_Display_DDS_Char_clean(void);
void FIFO_Display_DDS_Pixel_clean(void);
unsigned char FIFObox_nextLast(void);
void malloc_display_Box(void);
unsigned char FIFOboxChars_nextLast(void);
unsigned char FIFO_displayBoxChars_pop(unsigned char *x,unsigned char *y);
unsigned char FIFO_displayBoxChar_push(unsigned char x,unsigned char y);
void malloc_display_Box_Chars(void);
signed short int popPushFIFO_OFFSET(signed short int *ry,signed short int xn,signed short int yn);
void init_FIFO_OFFSET(void);
void init_FIFOs_TX_IOUP(void);
unsigned char getComand_FIFO_B(unsigned char n);
unsigned char pushFIFO_TX(unsigned char cmd,unsigned char *dato);
signed short int **next2(signed short int *inicio,signed short int *final,signed short int **pos);
//unsigned char pushFIFO_IO_TX(unsigned char *data,unsigned char size);
unsigned char isLoad_next(unsigned char **p);
void reset_pointer(void);
void pushFIFO_average_Offset(signed short int **pos,signed short int **pop,signed short int *tail,signed short int *ini,signed short int dato);
signed short int popFIFO_average_Offset(signed short int **pos,signed short int **pop,signed short int *tail,signed short int *ini);
unsigned char *next(unsigned char *inicio,unsigned char *final,unsigned char *pos,unsigned char *pop);
void init_FIFO_BARRA_NUMERO(void);
unsigned char FIFO_barraNum_push(unsigned char c);
void FIFO_barraNum_VFDposicion(unsigned char posx,unsigned char posy);
unsigned char  FIFO_barraNum_pop(void);
unsigned char isFIFO_num_Not_Empty(void);
void FIFO_barraNum_VFDserial_SendChar(unsigned char c);
void init_FIFO_CMD_DDS(void);
void init_FIFO_TX_serial_General(void);
void vaciarBuffer(unsigned char *p,unsigned char *p2,unsigned char size);
void push_FIFO_TRANSMISION_serial_IO(unsigned char byte);
unsigned char pop_FIFO_CMD_DDS(signed short int *x,signed short int *y);
unsigned char push_FIFO_CMD_DDS(signed short int x,signed short int y);
unsigned char pop_FIFO_TRANSMISION_serial_IO(unsigned char *status);
void reset_FIFO_serial_TX(void);
unsigned char search_NO_Repetido(signed short int x,signed short int y);
void init_FIFO_SaveTemp_pixel_DDS(void);
unsigned char pop_FIFO_save_Temp_pixel_DDS(signed short int *x,signed short int *y);
unsigned char push_FIFO__save_Temp_pixel_DDS(signed short int x,signed short int y);
unsigned char is_FULL_FIFO_save_Temp_pixel_DDS(void);
unsigned char is_Empty_FIFO_save_Temp_pixel_DDS(void);
void clean_repaint_Pixels_DDS(void);
unsigned char IIC_FIFO_push_RX(unsigned char);
unsigned char IIC_FIFO_push_TX(unsigned char);
unsigned char IIC_FIFO_pop_RX(void);
unsigned char IIC_FIFO_pop_TX(void);
unsigned char get_Paquete(unsigned char *p,unsigned char (*pop)(void));
unsigned char is_device(unsigned char (*pop)(void),unsigned char *n);
unsigned char is_Lenght(unsigned char(*pop)(void),unsigned char *n);
unsigned char is_App(unsigned char(*pop)(void),unsigned char *n);
unsigned char is_CMD(unsigned char(*pop)(void),unsigned char *n,unsigned char app);
void get_parametros(unsigned char (*pop)(void),unsigned char *p);
void IIC_stop(void);
void init_FIFO_IIC(void);
void init_FIFO_RX_serial_Keypad(struct _FIFO_1byte_ *s);	
unsigned char FIFO_general_1byte_push(unsigned char dato,struct _FIFO_1byte_ *s);
unsigned char get_case_FIFO_general(struct _FIFO_1byte_ *s);
unsigned char FIFO_general_1byte_pop(unsigned char *dato,struct _FIFO_1byte_ *s);
void init_FIFO_General_1byte(struct _FIFO_1byte_ *s,unsigned char *h,unsigned char *t,unsigned char size);
void reset_FIFO_general_UChar(struct _FIFO_1byte_ *s);

void init_FIFO_General_1byte_v2(struct _FIFO_1byte_v2 *s, unsigned char *buff,unsigned char size);
unsigned char FIFO_general_1byte_push_v2(unsigned char dato, struct _FIFO_1byte_v2 *s);
unsigned char FIFO_general_1byte_pop_v2(unsigned char *dato, struct _FIFO_1byte_v2 *s);
void reset_FIFO_general_UChar_v2(struct _FIFO_1byte_v2 *s, unsigned char size);
unsigned char FIFO_push_1byte_v2(unsigned char x);
unsigned char FIFO_general_1byte_peek_v2(struct _FIFO_1byte_v2 *s);
unsigned char busqueda2(unsigned char *arr,unsigned char size,unsigned char dato);
unsigned char FIFO_general_1byte_search_v2(unsigned char dato,struct _FIFO_1byte_v2 *s);
static unsigned char *shift_left(unsigned char *arr,unsigned char size);
unsigned char solicitarRecurso(unsigned char id);
unsigned char liberarRecurso(unsigned char id);
void cleanArray_int16(int16 *arr,uint8 size,uint8 k);


#endif /* QUEUE_H_ */

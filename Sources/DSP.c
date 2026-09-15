/*
 * DSP.c
 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */


/*
 * DSP.c
 *
 *  Created on: Nov 9, 2016
 *      Author: chals
 *      
 *      
 *      
 *        Digital Signal Processor Main controller
 *        
 *        
 *        UNIDAD DE PROCESAMIENTO DIGITAL Y
 *        CALCULOS MATEMATICOS
 *        
 *        modificado el 8 de mayo 2017
 *        
 *        
 *       DDS   MAX Vp  Max Vpp  Max-Vp   MaxVpp
 *       ZOOM  EJE-X   Eje-x    Eje-Y    Eje-y
 *       99     14mVp   27mVpp   8mVp    16mVpp
 *            
 *        
 *        
 *        
 *        
 */


#include "ADC.h"
#include "DSP.h"
#include "errorController.h"
#include "system.h"
#include "VFDmenu.h"
//#include "Memoria.h"
#include "IOcomms.h"
#include "maths.h"
#include "queue.h"
#include "VFDisplay.h"
#include "ADC_BAL.h"
#include "strings.h"
#include "RTS_UART0.h"
#include <stdlib.h>  //  Necesario para abs()



#if (SIZE_FIFO>250)
#error "Se reuiere que la constante SIZE_FIFO sea menor de 250"
#endif




/* MODO DE PROCESAMIENTO PARA GIRAR LOS DATOS A LA VIBRACION DE LAS ORDENADAS*/
#define MODO_DE_VIBRACION   GIRO_DE_PLANO_CARTESIANO// GIRO_DE_COORDENADAS_POLARES //GIRO_DE_PLANO_CARTESIANO, GIRO_DE_COORDENADAS_POLARES
#define DDS_X    192

#define DDS_X_1d8 23  //1/4 de pantalla en las X lado negativo de X
#define DDS_X_2d8 45  // 1/2 de la pantalla de las X lado negativo de X
#define DDS_X_3d8 71  //3/4 de pantalla X lado negativo de X
#define DDS_X_4d8 96
#define DDS_X_CX  DDS_X_4d8 //TAMAÑO DE LA MITAD DE RESOLUCION PUNTOS DE LA PANTALLA EN X 192/2 la mitad del ordenada  poitivo  y negativo
#define DDS_X_5d8 120
#define DDS_X_6d8 144
#define DDS_X_7d8 168
#define DDS_X_8d8 192

#define DDS_Y_1d8 16//128/2  la mitad de la absisa  positivo y negativo
#define DDS_Y_2d8  32
#define DDS_Y_3d8  48
#define DDS_Y_4d8  64
#define DDS_Y_5d8  80
#define DDS_Y_6d8  96  
#define DDS_Y_7d8  112
#define DDS_Y_8d8  128
#define DDS_Y_CY   DDS_Y_4d8 



//variables global
//signed short int A[SIZE_FIFO]; //array where ADC's values are saved for later processing FOR FIFO construct
//signed short int B[SIZE_FIFO]; //array where ADC's values are saved for later processing FOR FIFO construct
//signed short int *head,*tail; //pointer to the FIFO parameters adc X
//signed short int *headB,*tailB; //pointer to the FIFO parameters adc y

//unsigned char SignalProcessor_Flag; //bandera que si es true se ejecuta la funcion la flagea un timer

//typedef signed short xxx;

unsigned short int x4,y4;
unsigned char Semb;  //Semaforo de Vibracion Setup 
//variables activas globales por menu

unsigned char igxm5[IGXM_SIZE];//variable generica array generico
unsigned short int pixelConversionX[DDS_X_8d8];//convierte el pixel Analogo a pixel del zoom actual
unsigned short int pixelConversionY[DDS_Y_8d8];//convierte el pixel Analogo a pixel del zoom actual
extern unsigned char DDSmap[DDSMAPSIZEX][DDSMAPSIZEY];
//extern unsigned char igxc5;

extern struct _Menu_  menu;//contexto Actual que se despliega
//extern unsigned char EventDDS;

//unsigned char anguloVibracion;//debug esta variable debeser guardada
double anguloVibracion;//angulo de vibracion, cantidad de angulos en Sexahesimales que gira la señal
//double anguloVibracionRadianes;
//double anguloGiro;//elangulo que van a girar todos los datos
//static double ang[SIZE_ANGLE_ARR];

struct _GLOBAL_CONTEXTO g,*AjustVibr,*DDSpant,*ParamDet;
struct _GLOBAL_CONTEXTO *AjParamProd,*NewProd,*textProc;
struct NodoLista ListaFIFO[SIZE_FIFO],*head,*tail,*uhead,*utail;
struct ZoomControl zoom;
struct NodoCV FIFOdisplayCV[VIB_DISP_SIZE],*headCV,*tailCV;
struct NodoCharOp ListaFIFOcharOP[SIZE_FIFO_CHAROP],*headcOP,*tailcOP;
struct _Detection Deteccion;
//struct _DDS_ DDS1;	
struct _Mesurements_ Mediciones;
extern struct NodoBox FIFObox[SIZE_FIFO_BOX];
struct _GLOBAL_CONTEXTO gxc,gxc2,gxc3,gxc4,gxc5,gxc6,gxc7,gxc8,gxc9,gxc10,gxc11,gxc12;
struct _GLOBAL_PASO_OTRO_CONTEXTO gpoc;//global paso a otro contexto
unsigned char igxc3;
extern struct _Signale_ Signale;
struct _Signal_ Signal;//version deprecated

Detection_s det;//control de deteccion.
volatile extern ErrorControl_t FailsCtl;

struct _OFFSET_ Offset;//CONTROL DE  las variables de offset
extern struct _Comando_DDS dds;

void (*pDigitalFilter)(signed short int *x,signed short int *y); //apuntador global a funciones  para cambiar el tipo de filtro

static uint8_t init_DSP(void);
static void vTask13_Centrar_Signal_y_OffSet(void);
static void vTask14_Control_Offset_init(uint32 *mem32,uint16 *mem16,uint8 *mem8);
			
// funciones externas
extern void ADCsRead(signed short int *adc8,signed short int *adc7);
//extern void displayDDSsignalProcessor_IRQ(unsigned short int x,unsigned short int y);
//extern unsigned char ZoomDDS; //variable que guarda el Zoom actual en la DDS del producto actual
//extern unsigned char ZoomDDS; //variable que guarda el Zoom actual en la DDS del producto actual
extern unsigned char getZoomDDS(void);
extern unsigned char semb; //semaforo para la barra de deteccion para mandar comandos y esperar que termine de desplegar una deteccion(varios boxes)
//Metodos---------------------------------------------------------------------------
static uint8_t  init_DSP(void){
int i,j;
static uint8 init;
     if(init)return; 
	 //headA=&A[0];  //init pointer for the FIFO
	 //tailA=&A[SIZE_FIFO-1];
	 //headB=&B[0];
	 //tailB=&B[SIZE_FIFO-1];
	 //malloc(&ListaFIFO[0]);  
	 pDigitalFilter=FiltroDigital_passWideBand;  //assign function address to pointer to funcion
//	 SignalProcessor_Flag=FALSE; //bandera que si es true se ejecuta la funcion la flagea un timer
//	 Zoom_init(); este se ejuta cuando entramos ala pantalla DDS
	 semb=VERDE;//listos para ejecutar el despliegue de la barra de deteccion
	 malloc_display_Box();//&FIFObox[0]);
	 mallocCV(&FIFOdisplayCV[0]);
	 mallocCharOp(&ListaFIFOcharOP[0]);
	 anguloVibracion=0;//debug esta variable debeser sacada de la memoria.
	 //extern unsigned char DDSmap[DDSMAPSIZEX][DDSMAPSIZEY];
//     for(i=0;i<DDSMAPSIZEX;i++)
  //  	 for(j=0;j<DDSMAPSIZEY;j++)
    //		 DDSmap[i][j]=0;
     Deteccion.tipo=NORMAL;//DEBUG  ESTO DEBE ser gestionado en la memoria.
     dds.Bnderas.bit.DDS_Reload=FALSE;
//     dds.Bnderas.bit.debug=FALSE;
     dds.Bnderas.bit.clean_Buffers=FALSE;//libre para meter en la fifo un pixel
     init_ADC();
     init=1;
return init;
}// fin init DSP digital Signal Processor----------------------------------





/*run in timer_IRQ, execute when the ADC's  has been just Read  
 * PROCESADOR DIGITAL DE SEÑALES  */
void vTask12_Signal_Processor_Controller(void){
int16 x,y; //where we save the ADC's now data	
  
  
    if(init_DSP()){
       Read_QSPI2(&x,&y); //Señal delos ADC's
       vTask13_Centrar_Signal_y_OffSet();//señal Centrado y remover Offset
	   
//	   Comm_DDS_FIFO(Signal.CentX,Signal.CentY);//Meter datos a lista para transmision
	   //ControlMaestrodeVibracionXgiro(Signal.CentX,Signal.CentY,&(Signal.Rotx),&(Signal.Roty));//GIRAMOS LA señal a vibracion CORRECTA                      
       //pushFIFO(Signal.Rotx,Signal.Roty); //push data in fifo for digital filters
       //pDigitalFilter(&datax,&datay);//DESACTIVADO digital filter running, return data filtered real time
       //Memoria_Pixeles_DDS(Signal.Rotx,Signal.Roty);//Signal.Rotx,Signal.Roty);
       //Escalar_al_ZoomSelected(Signal.Rotx,Signal.Roty,&(Deteccion.signalDDSx),&(Deteccion.signalDDSy));//Escalamos para DDS
       //Comm_DDS_FIFO(Signal.Rotx,Signal.Roty);
       ////detectionAnalisis(Signal.Rotx,Signal.Roty); //search for deteccion signals based upon parameters seting ups
       //BarraDeteccion(&(Signal.Rotx),&(Signal.Roty));//Control Maestro de la Barra de deteccion
       ////AnguloVibracionProcesadorCentral(Signal.Rotx,Signal.Roty);//Signal.CentX,Signal.CentY);//datax,datay);//Menu Config Vibracion, Procesamiento de datos y calculo de Angulo
	   //Procesador_de_Producto()
//	   SignalProcessor_Flag=FALSE;//no quitar esta linea, esperamos otra IRQ del timer para ejecutar
     //si no hay flag no se ejecuta
	
	  }//init dsp 
}// fin Signal Processor IRQ----------------------------------------------------------------------


/* ++FIFOS+++++++++++++++++++++++++++++++++++++++++++++++FIFOS+++++++++++++++++++++++++FIFOS++++++++
 * ++FIFOS+++++++++++++++++++++++++++++++++++++++++++++++FIFOS+++++++++++++++++++++++++FIFOS++++++++
 * ++FIFOS+++++++++++++++++++++++++++++++++++++++++++++++FIFOS+++++++++++++++++++++++++FIFOS++++++++
 * ++FIFOS+++++++++++++++++++++++++++++++++++++++++++++++FIFOS+++++++++++++++++++++++++FIFOS++++++++
 * ++FIFOS+++++++++++++++++++++++++++++++++++++++++++++++FIFOS+++++++++++++++++++++++++FIFOS++++++++
 * ++FIFOS+++++++++++++++++++++++++++++++++++++++++++++++FIFOS+++++++++++++++++++++++++FIFOS++++++++
 * ++FIFOS+++++++++++++++++++++++++++++++++++++++++++++++FIFOS+++++++++++++++++++++++++FIFOS++++++++
 * ++FIFOS+++++++++++++++++++++++++++++++++++++++++++++++FIFOS+++++++++++++++++++++++++FIFOS++++++++
 * */

void Generador_de_Señal_Debug_Tool(signed short int *x11,signed short int  *y11){
static signed short int init,x12,y12;
const signed short int limP=20,limN=-20; 
const unsigned char fase1=0xA1;
const unsigned char fase2=0xB2;
const unsigned char fase3=0xC3;
const unsigned char fase4=0xD4;
static unsigned char fase;
const unsigned short int limi4=-55,limSup=80;
const unsigned char OPTION1=0xF4,OPTION2=0xF3;
unsigned char Option=OPTION1;

 switch(Option){  
  case OPTION2:if(init!=0xAA){
	                 init=0xAA;
	                 x12=0;	y12=limi4;}
               if(++y12>limSup){
            	     y12=limi4;}
               *x11=x12;*y11=y12;
		       break;
  case OPTION1://para pintar un cuadrado
       if(init!=0xAA){
	 		    init=0xAA;
	 		   x12=20;y12=20;fase=fase1;}
	 	   switch(fase){
	 		   case fase1:if(y12==-20){
	 			                 fase=fase2;
	 		                     y12=20;x12=-20;} 
	 		   	   	   	  else{--(y12);}
	 		              break;
	 		   case fase2:if(x12==20){
	 			                 fase=fase3;
	 			                 x12=20;y12=-20;}
	 		              else{++(x12);}
	 		              break;
	 		   case fase3:if(x12==-20){
	 			                fase=fase4;
	 		                    x12=-20;y12=-20;}
	 		              else{--(x12);}
	 		              break;
	 		   case fase4:if(y12==20)
	 			                init=0;
	 		              else{++(y12); } 
	 		              break;
	 		   default:break;}
	 	  *x11=x12;*y11=y12;
	 	   break;
	 		   default:break;}
               
}//fin de generador de numeros para simular datos espeecificos para pruebas



/* Construccion de la lista FIFO
 * */
//void malloc(struct NodoLista *N){
// struct NodoLista *p;
//        head=N; //&ListaFIFO[0]; en vez de
//        head->next=&ListaFIFO[1];
//        head->prev=&ListaFIFO[SIZE_FIFO-1];
//        tail=&ListaFIFO[SIZE_FIFO-1]; //oldest
//        tail->next=N;// &ListaFIFO[0]; instead
//        tail->prev=&ListaFIFO[SIZE_FIFO-2];
//        uhead=N;
//        utail=tail;
//    	for(p=head+1;p<tail;p++){//verificar que es menos 2
//    	    	p->next=p+1;
//    	    	p->prev=p-1;}
	    
//}// fin malloc----------------------------------------------------------------------------------------------------------



/* Construccion de la lista FIFO
 * Memory Allocation of  stack for display number in Vibration Configuration Menu*/
void mallocCV(struct NodoCV *N){//Memory allocation  Config Vibration
 struct NodoCV *p;//FIFOdisplayCV
 unsigned char i;
        headCV=N; //&ListaFIFO[0]; en vez de
        headCV->next=&FIFOdisplayCV[1];
        headCV->prev=&FIFOdisplayCV[VIB_DISP_SIZE-1];
        tailCV=&FIFOdisplayCV[VIB_DISP_SIZE-1]; //oldest
        tailCV->next=N;// &ListaFIFO[0]; instead
        tailCV->prev=&FIFOdisplayCV[VIB_DISP_SIZE-2];
        for(p=headCV+1;p<tailCV;p++){//verificar que es menos 2
    	    	p->next=p+1;
    	    	p->prev=p-1;}
	    for(i=0;i<VIB_DISP_SIZE;i++)
	    	FIFOdisplayCV[i].datCV=0;
}// fin malloc----------------------------------------------------------------------------------------------------------

/*construccion de la lista FIFO de la Memory 
 * */
void mallocCharOp(struct NodoCharOp *N){
struct NodoCharOp *p;
//unsigned char i;
       headcOP=N;
       headcOP->next=&ListaFIFOcharOP[1];
       headcOP->prev=&ListaFIFOcharOP[SIZE_FIFO_CHAROP-1];
       headcOP->data.id=0;
       tailcOP=&ListaFIFOcharOP[SIZE_FIFO_CHAROP-1];
       tailcOP->next=N;
       tailcOP->prev=&ListaFIFOcharOP[SIZE_FIFO_CHAROP-2];
       tailcOP->data.id=0;      
       for(p=headcOP+1;p<tailcOP;p++){
    	       p->next=p+1;
    	       p->prev=p-1;
               p->data.id=0;
               p->data.id=0;}
       tailcOP=headcOP;//esta vacia la fifo
       
}//fin malloc de char display Operative------------------------------------

//el dato lo metes por la cabeza y la sacas por la cola
char  pushFIFOcOPc(struct overLoad d){
struct 	NodoCharOp *p;
        
	  p=headcOP;
	  if(headcOP->data.id==0){//Esta vacia la FIFO?
		   //headcOP;//agregamos el primer elemento
		   headcOP->data.id=d.id;
		   headcOP->data.x=d.x;
		   headcOP->data.u.y=d.u.y;
		   tailcOP=headcOP->next;
	        return TRUE;}
	  else{if(tailcOP->data.id==0){  //hay un lugar en la FIFO
		       tailcOP->data.id=d.id;
		       tailcOP->data.x=d.x;
		       tailcOP->data.u.y=d.u.y;
		       if(tailcOP->next->data.id==0){//hay otro lugar en la FIFO
		    	     tailcOP=tailcOP->next;  
		             return TRUE;}}//se guardo exitoso
	       else return FALSE;}//YA no hay lugar en la FIFO 
}//fin del push 

void pushFIFOcOP_inicioCMD(void){
struct overLoad dato;
    dato.id=INICIO_CMD;
	pushFIFOcOPc(dato);
}//fin isertar en la FIFO un comando de inicio de escritura de chars

void pushFIFOcOP_VFDposicion(unsigned short int x, unsigned short int y){
struct 	overLoad dato;
   dato.id=VFD_POSICION;
   dato.x=x;
   dato.u.y=y;
   pushFIFOcOPc(dato);
}//fin de insertar en la FIFO un comando de posicion en el VFD-------------------

unsigned char pushFIFOcOP_VFDserial_SendChar(unsigned char c){
struct overLoad dato;
  dato.id=SEND_CHAR;
  dato.u.c=c;
  if(!pushFIFOcOPc(dato))
	  return FALSE;
  return TRUE;
}//fin de insertar en la FIFO un char para la graficacion en el VFD

void pushFIFOcOP_finCMD(void){
struct overLoad dato;
     dato.id=FIN_CMD;
     pushFIFOcOPc(dato);
}//fin de insertar en la FIFO un comando de fin de graficacion de conjunto de chars y comandos


unsigned char pushFIFOcOP_VFDserial_SendBlock(unsigned char *Ptr,unsigned short Size,unsigned short *Snd)
{
  register unsigned short count;                 /* Number of sent chars */
  register unsigned char result = FALSE;       /* Last error */

  for (count = 0x00; count < Size; count++) {
    result = pushFIFOcOP_VFDserial_SendChar(*Ptr++);
    if (result != FALSE) {            /* Sending given number of chars */
      *Snd = count;                    /* Return number of sent chars */
      return result;                   /* Return last error */
    }
  }
  *Snd = count;                        /* Return number of sended chars */
  return result;                       /* Return error code */
}//fin insertar en la FIFO un comando para graficar varios carateres.------------------------



void pushFIFOdisplayCV(unsigned char dato){
//struct NodoCV *p;
        // p=headCV;
         headCV=headCV->prev;
         headCV->datCV=dato;
}//fin insert in FIFO the box number-------------------------------------------------------------



void pushFIFO(signed short int x, signed short int y){
struct NodoLista *p;	 
	   
         p=head;
         head=p->prev;
         head->x=x;
         head->y=y;
         
}//fin insert in FIFO  the two ADC's data values--------------------------------------------------




struct overLoad popFIFOcOP(void){
struct overLoad n;	
	   if(headcOP==tailcOP){//si solo hay un dato en la FIFO
		   n=headcOP->data;
		   headcOP->data.id=0;//vaciamos el nodo
	       return n;}
	   else{n=headcOP->data;
	        headcOP->data.id=0;//vaciamos el nodo
	        headcOP=headcOP->next;//nos vamos uno para abajo
	        return n;}
}//fin de sacar de la FIFO de char opereativa------------------------------------



unsigned char popFIFOdisplayCV(void){//unsigned char *box1){
//static 	struct NodoCV *h;     
//static unsigned char AutoINIT;//auto iniztializador	  
unsigned char b;
      
       b=headCV->datCV;
       headCV->datCV=0;//borramos el dato que sacamos
	   headCV=headCV->next;
	   return b;
}//fin pop from FIFO ---------------------------------------------------------------------------------------



unsigned char popFIFOdata(signed short int *xx,signed short int *yy,unsigned char mode){
static 	struct NodoLista *h;     
	   if(mode==_INIT_)
		   h=head;
	   *xx=h->x;
	   *yy=h->y;
	   h=h->next;
	   if(h->prev==tail)
		   return TRUE;
return 0;	   
}//fin pop from FIFO ---------------------------------------------------------------------------------------





/*  Filtros  Digitales----------------------------------------------------------------------
 * ----------------------------------------------------------------------------------
 * ******************************************************************************************/
/*  Filtros  Digitales----------------------------------------------------------------------
 * ----------------------------------------------------------------------------------
 * ******************************************************************************************/
/*  Filtros  Digitales----------------------------------------------------------------------
 * ----------------------------------------------------------------------------------
 * ******************************************************************************************/


/* FIR filter with kons as ones to test speed and performance
 * */
void FiltroDigital_passWideBand(signed short int *x,signed short int *y){
//signed short int x=0,y=0; //data from adc FIFO
signed short int x1=0,y1=0;//buffer to calculus filter fir 
signed short int i=0; //for operations compatibility                                     â†“->30 
signed short int k[33]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};//digital filter konstants in fixed point notation
               //SIZE_FIFO
unsigned char modo=_INIT_;
       
       
loop:  return;
       modo=popFIFOdata(x,y,modo);
       x1=*x*k[i]+x1;   //X digital filter sum serie, no optimized mode
       y1=*y*k[i++]+y1;   //Y digital filter sum serie,  no optimized mode
       if(modo)  //ya leimos el tail
    	    return;
       else{modo=_RUN_; 
    	    goto loop;}
}//fin Filtro Digital pass Wide LARGE band Filter kind FIR------------------------------------------------


/*CONTROLADOR CENTRAL DEL CALCULO MAESTRO DEL VOLTAJE PRINCIPAL 
 * DE LA SEÑAL DE OFFSET PARA AMBOS CANALES DE DETECCION * * */
void vTask13_Centrar_Signal_y_OffSet(void){
static estado13;
enum{SIZE_MEMO32=4, SIZE_MEMO16=4,SIZE_MEMO8=1};
static uint32 memo32[SIZE_MEMO32];
static uint16 memo16[SIZE_MEMO16];
static uint8  memo8[SIZE_MEMO8];


  if(sys.u.bits.init_offset){
     vTask14_Control_Offset_init(&memo32[0],&memo16[0],&memo8[0]);}
  else{		
	 // Control_de_OffSet_Normal(datainX,datainY,Xout,Yout,&Offset);
	 // Procesador_Central_de_Cetrado_de_Datos(datainX,datainY,Xout,Yout);
      }
}//fin Control_de_OffSet----------------------------------------------------------------





//calcula el offset por primera vez que se enciende el equipo y lo guarda en EEPROM
static void vTask14_Control_Offset_init(uint32 *mem32,uint16 *mem16,uint8 *mem8){
uint8 *estado14;//cuenta un tiempo para que estabilizen las señales
int16 x,y;	
uint16 *Num_M,*cont;//Num muestras
int16 *sumX,*sumY;
int32 *offsetX[2],*offsetY[2];
const uint16 CONT_MAX=3000;//Muestras,para empezar a calcular el offset
const int16 MAX_OFFSET=550;//maximo offset permitido sino es error de balance
const int16 MAX_DIF_OFFSET=100;//diferencia entre los dos muestras de offset
uint8 *loop;
const uint8 LOOP_SIZE=2;//Numero de Busquedas de offset 

  /*   memo32=4 memo16 SIZE=4  memo8=1 */
  offsetX[0]=(int32*)(mem32+0); Num_M=mem16+0;       estado14=mem8+0; //memoria del S.O.C.M=Sistema Operativo Cooperativo Multitasking 
  offsetX[1]=(int32*)(mem32+1);  sumX=(int16*)(mem16+1); loop=mem8+1;
  offsetY[0]=(int32*)(mem32+2);  sumY=(int16*)(mem16+2);
  offsetY[1]=(int32*)(mem32+3);  cont=mem16+3;
    switch(*estado14){
       case 1:sys.u.bits.Driver_out=0;*sumX=0;*sumY=0;*Num_M=0;
              *cont=0;loop=0;
              (*estado14)++;break;//apagar driver
       case 2:if(Signale.CentOffset.X.dequeue(&x,&Signale.CentOffset.X))
    	       if(Signale.CentOffset.Y.dequeue(&y,&Signale.CentOffset.Y)){
    	    	    *sumX+=x;*sumY+=y;(*Num_M)++;
    	            (*cont)++;}
              if(*cont>CONT_MAX){(*estado14)++;*cont=0;}
              break;
       case 3:if(*loop>(LOOP_SIZE-1))*loop=0;//sacamos 2 evaluaciones
    	      *offsetX[(uint8)(*loop)]=*sumX/(*Num_M);//calcular el offset
              *offsetY[(uint8)(*loop)]=*sumY/(*Num_M);
              (*loop)++;
              if(*loop>(LOOP_SIZE-1))estado14++;
              *sumX=0;*sumY=0;*Num_M=0;*cont=0;
              (*estado14)--;
              break;
       case 4:if((*offsetX[0]-*offsetX[1])>MAX_DIF_OFFSET){//deteccion de desBalance
    	            FailsCtl.LedDriver=ERROR_BALANCE_ALTO_INIT;
    	            *estado14=1;}
       	   	  if((*offsetY[0]-*offsetY[1])>MAX_DIF_OFFSET){//deteccion de desBalance
           	        FailsCtl.LedDriver=ERROR_BALANCE_ALTO_INIT;
                    *estado14=1;}
              if(abs(*offsetX[0])>MAX_OFFSET){//offset fuera de rango
            	    FailsCtl.LedDriver=ERROR_BALANCE_ALTO_INIT;
            	    *estado14=1;}
              if(abs(*offsetY[0])>MAX_OFFSET){//offset fuera de rango
                    FailsCtl.LedDriver=ERROR_BALANCE_ALTO_INIT;
                    *estado14=1;}
              else{ det.offsetX=(int16)*offsetX[0];
                    det.offsetY=(int16)*offsetY[0];
                    sys.u.bits.init_offset=1;//offset calculado con exito
                    *estado14=0;}//poner estado en =0 importante la memo se reusa
              break;
       default:*estado14=1;break;}//fin switch          
}//fin Control_de_Offset_PrimeraVez----------------------------------------------------------------



//Procesador Central del Algotimo Maestro Principal del Nucleo de Calculos Maestros para
//el centrado de datos fuera de centro u de  offet
void Procesador_Central_de_Cetrado_de_Datos(signed short int datainX,signed short int datainY,signed short int *Xout,signed short int *Yout){
const unsigned char SIZE_AV=10;//TAMAÑO DE los array para sacar el promedio de los offsets
static unsigned short int MAX; //MAXMO NUMEOR ABsoluto en positivo y negativo para hacer el calc de l av
static signed short int X[SIZE_AV];
static signed short int Y[SIZE_AV];
static signed short int sumx,sumy;//promedio  average de los datos x y los datos y dentro de n rango menor a 100
static signed short int *popx,*popy; //indice del dato a sacar
static signed short int *posx,*posy,init,avx,avy;//indice de la posicion actual de la FIFO
static signed short int *tailx,*taily,ssi;
         if(init!=0xAA){
        	 init=0xAA;
        	 posx=&X[0];posy=&Y[0];
        	 popx=&X[0];popy=&Y[0];
        	 tailx=&X[SIZE_AV-1];taily=&Y[SIZE_AV-1];
             MAX=1000;  }//debug guardar en memoria EEPROM la variable MAx
         
         if(evaluar_ssint(datainX,MAX)){
        	       ssi=popFIFO_average_Offset(&posx,&popx,tailx,&X[0]);//sacmos el ultimo valor
        	       pushFIFO_average_Offset(&posx,&popx,tailx,&X[0],datainX);//metemos el valor nuevo
        	       sumx=sumx-ssi+datainX;///quitamos el elemento First y pones el nuevo elemento que acada de entrar
        	       avx=(sumx/SIZE_AV);}
        if(evaluar_ssint(datainY,MAX)){
				   ssi=popFIFO_average_Offset(&posy,&popy,taily,&Y[0]);//sacmos el ultimo valor
				   pushFIFO_average_Offset(&posy,&popy,taily,&Y[0],datainY);//metemos el valor nuevo
				   sumy=sumy-ssi+datainY;///quitamos el elemento First y pones el nuevo elemento que acada de entrar
				   avy=(sumy/SIZE_AV);}
         	     
 	   *Xout=datainX-avx;
 	   *Yout=datainY-avy;
}//fin del procesador central de centrado de datos-------------------------------------------------


//evaluar el numero short integer dividimos la funcion en 2 porque no
//acepta paro de parametros mas de 4 o 5, Si regresa falso no debe evaluarse el ajuste al centro. 
unsigned char evaluar_ssint(signed short int dato, unsigned short int max){
signed short int ssi;
	     if(dato==0)
	    	 return FALSE;  //cero no se calcula
	     if(dato<0) //es negativo???  
	     	   ssi=dato*-1;
	     if(ssi>max)  //esta dentro del rango del calculo?
	          return FALSE; 
	     return TRUE;
}//fin  evaluar_ssint



void Control_de_OffSet_Normal(signed short int datainX,signed short int datainY,signed short int *Xout,signed short int *Yout,struct _OFFSET_ *o){
	     if(Deteccion.EnCurso==EJECUTANDOSE){
		    	     CounterOffsetController(RESET);//resetea el contador que ejecuta la evaluacion del offset
		    	     setOFFSET(datainX,datainY,Xout,Yout,Offset.X,Offset.Y);
		             return;}
	     if(absSSInt(datainX)>MAX_OFFSET1){
	    	      setOFFSET(datainX,datainY,Xout,Yout,o->X,o->Y);
	              return;}
	     if(absSSInt(datainY)>MAX_OFFSET1){
	       	      setOFFSET(datainX,datainY,Xout,Yout,Offset.X,Offset.Y);
	              return;}
	     if(CounterOffsetController(GET)){//No hubo deteccion en este tiempo?
	          if(evaluar_Offset(datainX,datainY,Offset.X,Offset.Y))//esta mal?
	        	   getOFFset(datainX,datainY,&(Offset.X),&(Offset.X));//AutoAjustmos el OFFSET de la señal  debug poner esto en Configuracion de Vibracion despues
	          setOFFSET(datainX,datainY,Xout,Yout,Offset.X,Offset.Y);}
	     else 	 
	  	   setOFFSET(datainX,datainY,Xout,Yout,Offset.X,Offset.Y);
return;
}//FIN Control_de_OffSet--++---------------------------------------------------------------

/** evalua si el offset que actualmente esta configurado esta
 *  bien configurado. regresa TRUE si esta bien evaluado el OFFSET, esta mal el offset ret=FALSE*/
unsigned char evaluar_Offset(signed short int inX,signed short int inY,signed short int offsetX,signed short int offsetY){
const signed short int ERROR_OFFSET=21;//el error maximo del offset
//unsigned char n;
    if(inY==0)
    	if(inX==0)
    		if((Offset.Status&FIRST_START)==0)
    			 return FALSE;//nos esperamos a que lleguen las primeras señales
    
    if(Offset.ConfigFifoCounter<(SIZE_FIFO_OFFSET-1))
    	return FALSE;
	if((absSSInt(inX-offsetX))<ERROR_OFFSET){
		if((absSSInt(inY-offsetY))<ERROR_OFFSET){
			 if((++(Offset.ConfigFifoCounter))>60)
				   return TRUE;//ya esta listo el offset
			 else  return FALSE;}
		else return FALSE;}
	Offset.Status=0x80;//contador en cero y bandera en cero para recalcular el offset
return TRUE;	
}//fin evaluar_Offset---------------------------------------------------------------



/*cuenta  el tiempo que ha pasado sin haber una deteccion si no ha habido 
 * una deteccion en un tiempo t, entonces regresa TRUE*/
unsigned char CounterOffsetController(unsigned char var){
	 if(var==RESET){
	    Offset.Status&=FIRST_START;//SE LIMPIA exepto el bit MSB 
	    Offset.Counter=0;
	    return FALSE;}
	 else  {if((Offset.Status&0x81)==0x81)//bandera de first-start y bandera de deteccion
		         return FALSE;//No esta listo para evaluar OFFSET
	 	    else return TRUE; }//SI esta listo para evaluar OFFSET
}//fin CounterOffsetController-------------------------------------------------------------



//filtro pasa bajos para extraer el offset de la señal de ambos canales
void getOFFset(signed short int xin,signed short int  yin,signed short int *offsetX,signed short int *offsetY){      
	/*
	 * filtro digital: con ese filtro se obtiene el promedio se sacara y se garda en EEPROM para 
	 * restarlo en esta funcion cada vez. pero se llamara remove_OFFSET
	 * Moving Average Filter ( MA filter )<<--- aplicar este filtro en un menu profundo para quitar los offsets
	 * es el filtro que tenemos que implementar
	 * pero dentro del menu "get the offset" y desplegar 
	 * la señal como osciloscopio de  x y y para comprobr que se se quita el 
	 * offset.
	 * */
//	const unsigned char SIZE_OFFSET=64;
// 	static signed short int  xz[SIZE_OFFSET],yz[SIZE_OFFSET];//sumatoria de zeta
 	static signed short int pasx,pasy;//indice
 	signed short int f,ff;
// 	unsigned char n; // Status de Offset axxx xxxb a=bandera de first-start b=bandera de ?.,xx..x=conteo de llenado de la FIFO
 	
 	     if(xin==0)
 	        if(yin==0)
 	    	    return;
		 f=xin+pasx;
		 *offsetX=f>>DIV_OFFSET;//5
	     f=popPushFIFO_OFFSET(&ff,xin,yin);//sacamos el mas antiguo y metemos el mas nuevo en fifo
		 pasx=pasx-f+xin;//quitamos de la suma el mas antiguo y metemos el mas nuevo
		 if((Offset.ConfigFifoCounter)<(SIZE_FIFO_OFFSET-1))  //SIZE_FIFO_OFFSET
			 Offset.ConfigFifoCounter++;
		f=yin+pasy;
		*offsetY=f>>DIV_OFFSET;//5
	    pasy=pasy-ff+yin;
	    
//	*xout=xin-OffsetX;//este es el offset que se resta
//    *yout=yin-OffsetY;//este es el offset que se resta
}//fin del filtro pasa bajos principal de entrada para extraer el offset de la señal


inline void setOFFSET(signed short int xin,signed short int yin,signed short int *xout,signed short int *yout,signed short int offsetx,signed short int offsety){
	  *xout=xin-offsetx;//este es el offset que se resta
	  *yout=yin-offsety;//este es el offset que se resta 
}//fin setOFFSET------------------------------------------------------------------------


/**  ANALISIS DE DETECCION********************************************************************************************
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


/*  PROCESAMIENTO DE SEÑAL DE DETECCION PARA DESPLIEGUE CON INTERFAZ HUMANA
 *  solo nos escargamos de desplegar y contraDesplegar los cuadritos dinamicos
 *  otra funcion despliega el cuadro-Madre y los dos cuadros-EStaticos-Hijos-Vacios. 
 *  0cho cuadros dinamicos vacios
 *  nueve cuadros dinamicos llenos */
void BarraDeteccion(signed short int *datax,signed short int *datay){//fin de barra de deteccion------------------------------------------------------
signed short int datoy,datox;


static unsigned short int EstadoBarra0; //2 bytes estado de la barra de deteccion cada bit representa un cuadro
unsigned short int EstadoBarra;//, bitbox;
static unsigned char EstadoBarraMSB0;
unsigned char EstadoBarraMSB,R;
   datox=*datax;
   if(menu.contexto.Actual==PORTAL_INICIO)
	   goto BD1;
   else{if(menu.contexto.Actual==AJUSTE_DE_VIBRACION)
	       goto BD1;
	    else{if(menu.contexto.Actual==AJUSTE_PARAMETRICO_DE_PRODUCTO)
	    	  goto BD1;
	         else {if(menu.contexto.Actual==PARAMETROS_DE_DETECCION){
BD1:    if(Deteccion.tipo==NORMAL){//NORMAL det is when threshold lines is in horizolt without height lines
		  if(*datay>0)
	        	datoy=*datay;         
		  else{if(*datay<0)
      	          datoy=(*datay)*-1;
		       else datoy=0;}
        if(datoy<=Deteccion.LIM1)
	        	EstadoBarra=0x0000; //0000 0000 0000 0000
	    else{EstadoBarra=0x0001;//0000 0000 0000 0001
	             if(datoy<=Deteccion.LIM2)
	        	    EstadoBarra=EstadoBarra&0x0001; //0000 0000 0000 0001
	             else{EstadoBarra=0x0003;//0000 0000 0000 0011
	                  if(datoy<=Deteccion.LIM3) //cuadro 3
	                	  EstadoBarra=EstadoBarra&0x0003; ////0000 0000 0000 0011
	                  else{EstadoBarra=0x0007; ////0000 0000 0000 0111
	                       if(datoy<=Deteccion.LIM4)//cuadro 4 0000 0000 0000 x000
	                    	   EstadoBarra=EstadoBarra&0x0007; //0000 0000 0000 0111
	                       else{EstadoBarra=0x000F; //0000 0000 0000 1111
	                            if(datoy<=Deteccion.LIM5) //cuadro 5 0000 0000 000x 0000
	                            	EstadoBarra=EstadoBarra&0x000F; //0000 0000 0000 1111
	                            else{EstadoBarra=0x001F; //0000 0000 0001 1111
	                            	 if(datoy<=Deteccion.LIM6) //cuadro 6 0000 0000 00x0 0000
	                            		 EstadoBarra=EstadoBarra&0x001F; //0000 0000 0001 1111
	                            	 else{EstadoBarra=0x003F; //0000 0000 0011 1111
	                            	      if(datoy<=Deteccion.LIM7)// cuadro 7 0000 0000 01xx xxxx
	                            	    	  EstadoBarra=EstadoBarra&0x003F; //0000 0000 0011 1111
	                            	      else {EstadoBarra=0x007F; //0000 0000 0111 1111
	                            	    	    if(datoy<=Deteccion.LIM8) //cuadro 8 //0000 0000 1xxx xxxx
	                            	    	    	EstadoBarra=EstadoBarra&0x007F; //0000 0000 0111 1111
	                            	    	    else{EstadoBarra=0x00FF; //0000 0000 1111 1111
	                            	    	    	 if(datoy<=Deteccion.LIM9)//cuadro 9 //0000 0001 xxxx xxxx
	                            	    	    		 EstadoBarra=EstadoBarra&0x00FF; //0000 0000 1111 1111
	                            	    	    	 else{EstadoBarra=EstadoBarra&0x01FF; //0000 0001 1111 1111
	                            	    	    	      if(datoy<=Deteccion.LIM10)//cuadro 10  0000 00x1 1111 1111
	                            	    	    	    	  EstadoBarra=EstadoBarra&0x01FF; //0000 0001 1111 1111
	                            	    	    	      else{EstadoBarra=0x03FF; //0000 0011 1111 1111
	                            	    	    	           if(datoy<=Deteccion.LIM11)//cuadro 11  0000 01xx xxxx xxxx
	                            	    	    	        	   EstadoBarra=EstadoBarra&0x03FF; //0000 0011 1111 1111
	                            	    	    	           else{EstadoBarra=0x07FF; //0000 0111 1111 1111
	                            	    	    	        	    if(datoy<=Deteccion.LIM12) //cuadro 12 0000 1xxx xxxx xxxx
	                            	    	    	        	    	EstadoBarra=EstadoBarra&0x07FF; //0000 0111 1111 1111
	                            	    	    	        	    else{EstadoBarra=0x0FFF; //0000 1111 1111 1111
	                            	    	    	        	         if(datoy<=Deteccion.LIM13)//Cuadro 13   0001 xxxx xxxx xxxx
	                            	    	    	        	        	 EstadoBarra=EstadoBarra&0x0FFF; //0000 1111 1111 1111
	                            	    	    	        	         else{EstadoBarra=0x1FFF; //0001 1111 1111 1111
	                            	    	    	        	              if(datoy<=Deteccion.LIM14)//cuadro 14 001x xxxx xxxx xxxx
	                            	    	    	        	            	  EstadoBarra=EstadoBarra&0x1FFF; //0001 1111 1111 1111
	                            	    	    	        	              else{EstadoBarra=0x3FFF; //0011 1111 1111 1111
	                            	    	    	        	                   if(datoy<=Deteccion.LIM15)//cuadro 15 01xx xxxx xxxx xxxx
	                            	    	    	        	                	   EstadoBarra=EstadoBarra&0x3FFF; //0011 1111 1111 1111
	                            	    	    	        	                   else {EstadoBarra=0x7FFF; //0111 1111 1111 1111
	                            	    	    	        	                         if(datoy<=Deteccion.LIM16) //cuadro 16 1xxx xxxx xxxx xxxx
	                            	    	    	        	                        	 EstadoBarra=EstadoBarra&0x7FFF;
	                            	    	    	        	                         else{EstadoBarra=0xFFFF;
	                            	    	    	        	                              if(datoy<=Deteccion.LIM17)//Cuadro 17  1 xxxx xxxx xxxx xxxx
	                            	    	    	        	                            	  EstadoBarraMSB=OFF;
	                            	    	    	        	                              else {EstadoBarraMSB=ON;
	                            	    	    	        	                                    EstadoBarra=0xFFFF;}}}}}}}}}}}}}}}}}}//18->} 
	       
      
     switch(EstadoBarra){//conversion de bits individuales de boxes a numero de box
       case 0x0001:R=FIFO_displayBox_push(1);//displayBox(1); break;  //0000 0000 0000 0001
       case 0x0003:R=FIFO_displayBox_push(2); break;  //0000 0000 0000 0011
       case 0x0007:R=FIFO_displayBox_push(3); break;  //0000 0000 0000 0111
       case 0x000F:R=FIFO_displayBox_push(4); break;  //0000 0000 0000 1111
       case 0x001F:R=FIFO_displayBox_push(5); break;  //0000 0000 0001 1111
       case 0x003F:R=FIFO_displayBox_push(6); break;  //0000 0000 0011 1111
       case 0x007F:R=FIFO_displayBox_push(7); break;  //0000 0000 0111 1111
       case 0x00FF:R=FIFO_displayBox_push(8); break;  //0000 0000 1111 1111
       case 0x01FF:R=FIFO_displayBox_push(9); break;  //0000 0001 1111 1111
       case 0x03FF:R=FIFO_displayBox_push(10); break;  //0000 0011 1111 1111
       case 0x07FF:R=FIFO_displayBox_push(11); break;  //0000 0111 1111 1111
       case 0x0FFF:R=FIFO_displayBox_push(12); break;  //0000 1111 1111 1111
       case 0x1FFF:R=FIFO_displayBox_push(13); break;  //0001 1111 1111 1111
       case 0x3FFF:R=FIFO_displayBox_push(14); break;  //0011 1111 1111 1111
       case 0x7FFF:R=FIFO_displayBox_push(15); break;  //0111 1111 1111 1111
       case 0xFFFF:if(EstadoBarraMSB==OFF)
    	   	   	   	    R=FIFO_displayBox_push(16);  //1111 1111 1111 1111
                   else R=FIFO_displayBox_push(17);
                   break;
       case 0x0000:R=FIFO_displayBox_push(0); break;
       default:break;
        }//FIN SWITCH-------------------------------------------
    	//}//fin de WAIT
      // if(R==FALSE)
    	//   __asm(Halt);//Debug Error de diseño de software.
     }}}}//fin de contexto de configuracion de vibracion y del 
     
}//fin de barra de deteccion--------------------------------------------------------------------------------------------

//ret TRUE: No detection detected
/*  vamos a validar o no vale ó no cumple*/
unsigned char Validar_detectionAnalisis(signed short int  y){
	
	if((Offset.Status&FIRST_START)==0)//No se habia prendido nunca ni configurado nada
	    return TRUE;
	if(y==0)
	    return TRUE;
	if(Deteccion.tipo==NORMAL)//deteccion normal
		return FALSE;    
return TRUE;
}//fin Validar_detectionAnalisis-----------------------------------------



void detectionAnalisis(signed short int x, signed short int  y){ //search for deteccion signals based upon parameters seting ups
static unsigned char det;//Detection,DetectionFilter;
const unsigned char ANCHO_PULSO=4;//ando de pulso del pulso de rechazo

	x=0;//para quitar error de no used var
	if(Validar_detectionAnalisis(y))
		return;
	
	if(RTS_UART0_GetVal()){
	  if(det--<1)
	  	 ReleRechazo(FALSE);}	    
	  
	y=absSSInt(y);	 
    if(y>Deteccion.Sensibilidad){
	     if(!Deteccion.Deteccion){//hay deteccion actual en progreso??
	    	  Deteccion.countRechazos++; 
	          Deteccion.Deteccion=1;
	          ReleRechazo(TRUE);
	          det=ANCHO_PULSO;}}
    else{if(Deteccion.Deteccion)
              Deteccion.Deteccion=0;
         else return;}
    
   Contador_Rechazos_Controller(Deteccion.countRechazos);	    
	     
	    
}// fin de ANALISIS de  DETECCION DE seÃ±al



/* cR: conteo de rechazos, rechazoscontado a en este momento
 * */
void Contador_Rechazos_Controller(unsigned short int cR){
static unsigned short int count0;//rechazos en el pasado inmediato

    if(cR==count0)
    	return;
    count0=cR;//hubo un cambio en el numero
    Display_Cuenta_Rechazos(count0);
}//fin Contador_Rechazos_Controller----------





/*Giramos las coordenadas cartesianas para que las absisas coincidan paralelamente con la pendiente del ruido de 
 * vibracion mecanica y convertimos cada  punto de adquision de señal a unas nuevas coordenas*/
void ControlMaestrodeVibracionXgiro(signed short int xin,signed short int yin,signed short int *xout,signed short int *yout){
//signed short int x1,y1;

const double PI_2=1.5707963267949; //PI/2
const double PI_4=0.78539816339745; // PI/4
const double _180_PI=57.2958;//180/PI
const double _2PI=6.28831853072;  //2*PI
double anguloVibracionRad;
#if  (MODO_DE_VIBRACION==GIRO_DE_PLANO_CARTESIANO)
             //x1=*x;
             //y1=*y;   //MODO GIRO DEL PLANO CARTESIANO
             anguloVibracionRad=(anguloVibracion*_2PI)/360;
	         *xout=(signed short int)(xin*cosd(anguloVibracionRad)+yin*sind(anguloVibracionRad));
	         *yout=(signed short int)((-1)*(xin)*sind(anguloVibracionRad)+yin*cosd(anguloVibracionRad));
	         __asm(nop);
#else      
 double m,m1;
 double r;
 unsigned char theta;
 double thetad;
 unsigned long int xul,yul;
	            //MODO DE CONVERSION A COORDENADAS POLARES
	         if(*x==0) //proteccion contra division por ZERO
	        	 return;
	         x1=*x;y1=*y;
	         xul=(unsigned long int)(x1*x1);
	         yul=(unsigned long int)(y1*y1);
	         r=(double)(SquareRoot(xul+yul));//return ULong int r de las coordenadas polares del punto (x,y)
	         x1=*x;y1=*y;// x>=1? arctan(x)=pi/2-arctan(1/x)
	         m=(double)y1/(double)x1;  //comienza arcTan(y/x)+++++++++++++++++++++++++++++++++++++++++
             m1=0;
	         if(m==1)   //angulo de las coordenadas polares del (x,y)
	        	 theta=45;
	         else{if(absd(m)>1){ //ver en bitacora 11 de agosto de 2017-->>arcTan(y/x)
	        	    m1=1/m; //el numero de arctan no debe ser mayor a ±1
	        	    //thetad=PI_2-(PI_4*m-m*(absf(m)-1)+(0.2447+0.063*absf(m)));} //thetad=90-(45*m)-(m*(absf(m)-1)*(0.2447+0.063*absf(m)));}//}//para m>1,-1
	        	    thetad=PI_2-(m1/((m1*m1)+0.28125));}//sacamos tangente
	             else { thetad=m/(1+(0.28125*(m*m))); }}//ver 12 de septiembre en bitacora
	         thetad=thetad+0;//anguloVibracion;
	         *x=(signed short int)(r*cosd(thetad));//lo convertimos a coordenadas polares
	         *y=(signed short int)(r*sind(thetad));//convertimos coordenadas polares a cartesianas
	         	         
	         
       		 
	         
#endif       
}//fin de control maestro de vibracion por giro de las coordenadas cartesianas---------------------


/* PROCESAMIENTO CENTRAL DEL ANGULO DE VIBRACION MAESTRO+++++++++++++++++++++++++++++++++++++++++++++++++
 *   +Obtener el angulo de la pendiente de la recta de cada punto x,y con respecto al origen
 *   +guardar todos los angulos obtenidos de cada punto 
 *   +determinar si hay una congruencia de angulos 
 *   +desplegar el valor del angulo.
 *   NO FUNCIONA   ATAN ASIN ATANf vamos a probar con Series de Potencia.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void AnguloVibracionProcesadorCentral(signed short int datax,signed short int datay){
//float theta;//pendiente de cada punto con respecto al origen		

static double ang[SIZE_ANGLE_ARR];//,ang0;
static unsigned char k;
static unsigned char AutoConfig;
//static unsigned long long int xcount;
//signed short int ;
//const unsigned char SERIE_POT_SIZE=10;
//do ang[SERIE_POT_SIZE];
float m;//pendiente y absoluto de pendiente
//unsigned char i;
const float PI=3.14159265358979;
const float PI_4=0.78539816339745; // PI/4
const float PI_2=1.5707963267949; //PI/2
const float f180_PI=57.2957795130823;//   180/PI
static signed short int x0,y0;
float term1,term2,m0;
static unsigned char fangStack;//flag angle stack status full 
const signed short int THRESHOLD_DET=400;//estaba en 200, umbral de deteccion
	   if(menu.contexto.Actual==AJUSTE_DE_VIBRACION){
		if(AjustVibr->igxc1!=_COMPLETADO_CCV){ //Ya se COMPLETO el Calculo de Config de Vibracion?  
		   if(AutoConfig!=SAVE){//Auto-Configuracion
			   AutoConfig=SAVE;
					 k=0; x0=0; y0=0; fangStack=FALSE;}
		   if(datay==0)
			   return;
	       if((datay==y0)&&(datax==x0))//son las mismas coordenadas que las anteriores?
	    	    return;
	       if(datay<0){//es negativo?
	         if((datay*(-1))<THRESHOLD_DET)   //SOLO LAS SEÑALES GRANDES SON ACEPTADAS
	              goto ev1;}
	       else {if(datay<THRESHOLD_DET)//es positivo?
	    	      goto ev1;}
	       goto ev3;
ev1:	   if(datax<0){//es negativo?
              if((datax*(-1))<THRESHOLD_DET)   //SOLO LAS SEÑALES GRANDES SON ACEPTADAS
                    return;}
           else {if(datax<THRESHOLD_DET)//es positivo?
         	        return;}
ev3:       if(datax==0){
	      	   ang[k]=90;//el angulo de de 90° porque x=0
	           if(++k>SIZE_ANGLE_ARR-2)//si se lleno el array empieza por el primero
						k=0;
	      	   y0=datay;x0=datax;
	      	   normalizeAngle(&ang[k]);//convierte el angulo en positivo  de 0 a 180°
	      	   return;}
	       else{y0=datay;x0=datax;}
	       m=(float)datay/(float)datax;
	       if(absf(m)>1){//SI ES MAYOR a 1  // arctan(x)=pi/2-arctan(1/x)
	    	   m0=1/m;
	    	   term1=(PI_4*m0)-(m0*(absf(m0)-1)*(0.2447f+(0.063f*absf(m0))));
	    	   term2=PI_2-term1;
	    	   ang[++k]=term2*f180_PI;}//(PI_2-((PI_4*m)-(m*(absm-1)*(0.2447f+(0.063f*absm)))))*f180_PI;}// arctan(x)=pi/2-arctan(1/x)
	       else 
	           ang[++k]=((PI_4*m)-(m*(absf(m)-1)*(0.2447f+(0.063f*absf(m)))))*f180_PI;// arctan m=theta
		   if(k>SIZE_ANGLE_ARR-2){
				 k=0;fangStack=TRUE;	 }
      	   normalizeAngle(&ang[k]);//convierte el angulo en positivo  de 0 a 180°
           if(fangStack==TRUE){//ya se lleno el stack de angulos?    
     //   	   c=getAngleVibracion(&fangStack,&ang[0],&ang[SIZE_ANGLE_ARR-1]);deprecated
        	   girarSignals(getAngleVibracion(&fangStack,&ang[0],&ang[SIZE_ANGLE_ARR-1]));//obtiene el angulo en base a la densidad de angulos
           }    
	}}//fin contexto AJUSTE DE VIBRACION-------------------------------------------------------
}//+++++++++++FIN DEL PROCESADOR CENTRAL DEL ANGULO DE VIBRACION++++++++++++++++++++++++++++++++++++++++++



/* parametro0, parametro1, parametro2,
 * parametro0:badera para ver si ya se lleno el stak y se ejecuta este algoritmo
 * parametro1: pointer inicial del array y final el param2
 * */
double getAngleVibracion(unsigned char *fangStack,double *ang,double *angLast){//obtiene el angulo en base a la densidad de angulos
unsigned char aV;//anguloVibracion; 
unsigned char aVfrac;  //anguloVibracion Decimal Fraccion.
double ang2[SIZE_ANGLE_ARR];
	 if(*fangStack==TRUE){
		  fangStack=FALSE;//se ejecuta este algorithm una vez cada vez que se llena el array
		       //buscqueda por cuadrante     2-|-1  <-circulo
		       //                            3-|-4
		  bubbleSort(ang,angLast);  //los ordenamos , 8 horas de distraccion 0.5 horas de concentracion y exito
		  convert2int(ang,angLast,&ang2[0]);//les quitamos los decimales
		  aV=countingMayoria(&ang2[0],&ang2[SIZE_ANGLE_ARR-1]);//contamos al mayoria, la mayor densidad 
		  aVfrac=getFraccionAngVibr(ang,angLast,aV);//obtener fraccion del angulo de vibración
		  //pushFIFOdisplayCV(180+aVfrac);// es 180 mas fraccion osea de 180 a 189 se resta y sacamos la fraccion
		  //if(aV==0)
	    	//	  pushFIFOdisplayCV(191);//significa que el angulo es menor a 1 o cero y fraccion
		//  else pushFIFOdisplayCV(aV);//insertamos el numero del angulo	  
		  if(++(AjustVibr->igxc0)>14)//>14)//el 15 lo vamos a usar para detener la conf. vibracion.
			  AjustVibr->igxc0=0;//
		  displayAnguloVibracion(aV,aVfrac,AjustVibr->igxc0);
		  __asm(nop);
		  return reBuildAV(aV,aVfrac);//RECONSTRUIR el numero en char a double
		  //return //regresamos el angulo de vibracion obtenido
	 }//fin flag angle Stack---------------
return 0;
}// obtener el angulo de vibracion -----------------------------------------------------------------------------

/*funcion que determina las funciones matematicas para girar cada uno de los datos que entran al sistema */
/* Si necesitamos girar la señal al derecha es negativo el angulo
 *  y si necesitamos girar a la izquierda es positiva el angulo de vibracion
 * */
void girarSignals(double angle){//los ANGULOSDE VIBRACION NO PUEDEN SER MAYORES A 180°  Y MENORES A 0°
//double c,n; 
   	 if(angle<0)
		 angle=180+angle;
	 if(anguloVibracion==0){
		   if(angle>90){
	           anguloVibracion=angle-180;
	           return;}
		   else anguloVibracion=angle;
		        return;}
	 else{
	   if(angle<4){
		  if(angle<0.5){
			    displayAnguloCompletado();//pushFIFOdisplayCV(0xFF);//indica que ha sido completado el angulo de vibracion.
		        return;}
		  else{ anguloVibracion+=0.5;
		        return;}}
	   else{anguloVibracion=angle;
		    if(anguloVibracion>180){
		        anguloVibracion=0;
		        return;}}}
}//-------------------------------------------------------------------------------------------------------------



//los angulos ya son positivos de 0 a 180°
unsigned char getFraccionAngVibr(double *ang,double *angMaxInd,unsigned char av){//obtenemos la fracciond el angulode Vibracion solo el decimal
double *i,avd,frac,f2=0;	
unsigned char count=0;
    avd=(double)av;//cnvertimos el uchar a double, del angulo de vibracion para desplegar
	for(i=ang;i<=angMaxInd;i++){
		frac=absd(avd-*i);//separamos el entero de los decimales
		if(frac<1){
			count++;//contamos cuantos elementos tienen el mismo valor entero e.g. 127 cuantos son 127.34, 127.432,127.433=3
	        f2+=frac;}}//sumamos los decimales para promediarlos
    frac=f2/count; //sacamos el promedio de los decimales
    for(f2=0.1,count=1;f2<=0.9;f2+=0.1,count++)//
       if((unsigned char)(frac/f2)==1) //divimos el decimal desde 0.1 a 0.9 donde nos de 1.xx ->uchar =1 ese es
          return count;
    return 0;//el decimal es x.0xxx
                   //          ^
}//finobtener la fraccion del angulo de vibracion----------------------------------------

//funcion que reconstruye el numero en double otravez
double reBuildAV(unsigned char Entero, unsigned char Frac){
double intd;	   
	   intd=(double)Entero;
	   switch (Frac){
	     case 0:return(intd); break;
	     case 1:return(intd+0.1);break;
	     case 2:return(intd+0.2);break;
	     case 3:return(intd+0.3);break;
	     case 4:return(intd+0.4); break;
	     case 5:return(intd+0.5);break;
	     case 6:return(intd+0.6);break;
	     case 7:return(intd+0.7);break;
	     case 8:return(intd+0.8); break;
	     case 9:return(intd+0.9);break;
	     default:return(intd);break;} 	     	     	     	   	    
}//fin de la funcion de reconstruccion de numero a dobles.-------------------------------------------

void convert2int(double *ang,double *angMaxInd,double*ang2){
double *i,*j;	
	      for(i=ang,j=ang2;i<=angMaxInd;i++,j++)
	    	   *j=(int)*i; //*i=(int)*i;            
}//fin convert to int-----------------------------------------------------

//algoritmo que encuentra el numero con mayor repeticiones en el array Ya ordenado..(tiene que estar ordenado)-------------------------------
unsigned char countingMayoria(double *a,double *aM){//*angulo array  *anguloMaximo
double n0,n;
unsigned char c0,c;
          n0=*a;
          c0=1;n=0;c=0;
cm2:      a++;
          if(n0==*a){
        	  c0++;
        	  if(a>=aM)
        		  goto cm1;
        	  else goto cm2;}
          if(a>=aM)
        	  goto cm1;
cm4:      n=*a;
          c=1;
cm3:      a++;
          if(n==*a){
        	  c++;
              if(a>=aM)
                 goto  cm1;
              else goto cm3;}
          if(a>=aM)
        	  goto cm1;
          if(c0>c)
        	  goto cm4;
          c0=c;
          n0=n;
          goto cm4;
cm1:      if(c0>c)
	          return (unsigned char)n0;//debug convertir el array en char desde atras o display float con decimales
          else return (unsigned char)n; 
}//fin counting la mayotia de los datos--------------------------



void bubbleSort(double *ang,double *angMaxInd){
unsigned char fswapped;
 double *i,n;

   do{fswapped=FALSE;
      for(i=ang;i<=angMaxInd;i++)
        if(*i>*(i+1)){
            n=*(i+1);
            *(i+1)=*i;
            *i=n;
            fswapped=TRUE;}
        }while(fswapped);

}//fin bubble sort--------------------------------





void normalizeAngle(double *angulo){//convierte el angulo en positivo  de 0 a 180°
double angle;
     angle=*angulo;
	 if(angle==90)
		 return;
	 if((angle>90) && (angle<180))
		 return;
	 if(angle>180){
		 *angulo=angle-180;
	      return;}
	 if(angle<0){
		 *angulo=180+angle;
		 return;}	  
}//normalizacion del angulo ha de 0a a 179° antes era de 0 a -4 -60° etc..


/* ************+MATEMATICAS++++++++++++++++++++++++++++++++++++++++MATEMATICAS++++CALCULUS+++++++++++++++++
 * ************+MATEMATICAS++++++++++++++++++++++++++++++++++++++++MATEMATICAS++++CALCULUS+++++++++++++++++ 
 *  ************+MATEMATICAS++++++++++++++++++++++++++++++++++++++++MATEMATICAS++++CALCULUS+++++++++++++++++
 *  ************+MATEMATICAS++++++++++++++++++++++++++++++++++++++++MATEMATICAS++++CALCULUS+++++++++++++++++
 *  ************+MATEMATICAS++++++++++++++++++++++++++++++++++++++++MATEMATICAS++++CALCULUS+++++++++++++++++
 * ************+MATEMATICAS++++++++++++++++++++++++++++++++++++++++MATEMATICAS++++CALCULUS++++++++++++++++
 * /
 */

/* Numero x, ulong sacamos el  rootsquare y lo regresamos en  unsigned short int */
unsigned long int SquareRoot(unsigned long int x){
	unsigned  long int G=0x8000;
	unsigned  long int G2;
	unsigned char pos=15; //posicion del bit actual, empesamos el MSB

	        G2=G*G;
	        if(G2==x)//la raiz cuadrada es 8000h
	           return G;
	sr1:    if(x>G2)
	             newbit(&pos,&G);
	        else shiftright(&pos,&G);
	        G2=G*G;
	        if(G2==x)
	            return G;
	        if(pos==0){
	        	 if((G*G)>x)
	        		  return (G-1);
	        	 else return G;}
	        goto sr1;

}//fin de la Raiz Cuadra de un numero --------------------------------------------------------------------


/* Agrega un bit en la posicion dada   */
void newbit(unsigned char *pos,unsigned long int *n){
const long int longNum[]={0x0001,0x0002,0x0004,0x0008,
                          0x0010,0x0020,0x0040,0x0080,
                          0x0100,0x0200,0x0400,0x0800,
                          0x1000,0x2000,0x4000,0x8000};   
	 *pos=*pos-1;
	 *n=longNum[*pos]|*n;
}//fin de la funcion que agrega un nuevo bit en la posicion designda por pos-----------------------------

/*  Quita el bit en la posicion pos y lo poner en la derecha*/
void shiftright(unsigned char *pos,unsigned long int *n){

	const long int longNum[]={0x0001,0x0002,0x0004,0x0008,
	                          0x0010,0x0020,0x0040,0x0080,
	                          0x0100,0x0200,0x0400,0x0800,
	                          0x1000,0x2000,0x4000,0x8000}; 
	const long int longNu2[]={0xFFFE,0xFFFD,0xFFFB,0xFFF7,
		                      0xFFEF,0xFFDF,0xFFBF,0xFF7F,
		                      0xFEFF,0xFDFF,0xFBFF,0xF7FF,
		                      0xEFFF,0xDFFF,0xBFFF,0x7FFF}; 
    		
	*n=*n&longNu2[*pos];   //QUITAR elbit en la posicion actual
	*pos=*pos-1;//incrementamos posicion   
	*n=longNum[*pos]|*n;
}//fin de right shift----------------------------------------------


	
//always wrap input angle to -PI..PI
double sind(double x){//x en radianess
double sin;
	
	
	if (x < -3.14159265)
         x += 6.28318531; 
    else
      if (x >  3.14159265)
           x -= 6.28318531;
//compute sine
    if (x < 0){
       sin = 1.27323954 * x + .405284735 * x * x;
       if (sin<0)
          sin = 0.225 * (sin *-sin - sin) + sin;
       else
          sin = 0.225 * (sin * sin - sin) + sin;}
    else{
        sin = 1.27323954 * x - 0.405284735 * x * x;
        if (sin < 0)
            sin = .225 * (sin *-sin - sin) + sin;
        else
            sin = .225 * (sin * sin - sin) + sin;}
return sin;
}//fin sinus double  in radins--------------------------------------

double cosd(double x){//coseno en radines input::output
//compute cosine: sin(x + PI/2) = cos(x)
double cos;	
        x += 1.57079632;
          if (x >  3.14159265)
        x -= 6.28318531;
        if (x < 0){
           cos = 1.27323954 * x + 0.405284735 * x * x;
           if (cos < 0)
              cos = .225 * (cos *-cos - cos) + cos;
           else
              cos = .225 * (cos * cos - cos) + cos;}
        else{
           cos = 1.27323954 * x - 0.405284735 * x * x;
           if (cos < 0)
             cos = .225 * (cos *-cos - cos) + cos;
           else
             cos = .225 * (cos * cos - cos) + cos;}
return cos;
}//fin cosinus in radians---------------------------------------------





/*  Se escala el valor ADC al zoom seleccionado 
*   99 es mas cercano y 1 es el mas lejano,
*   1 va de,. //http://www.dcode.fr/lagrange-interpolating-polynomial
*   AREA DE DESPLEIGUE DE PIXELS ES  192 x 128 pixels
*   Escalar_al_ZoomSelected, ESCALA:               
*   99
*   usamos ux en  gxc.gxc0 y uy en gxc.gxc1
*   */ 
void Escalar_al_ZoomSelected(signed short int x,signed short int y,unsigned char *ux,unsigned char *uy){

	//if(DDS1.Banderas.bit.EventDDS==TRUE){ //.EventDDS==TRUE){//SE EJEcuta cuando estamos en la pantalla DDS		
   if(dds.Bnderas.bit.EventDDS==TRUE){
  	   *ux=getCoord_pixel(_X_,x);//Numeros digitales del limite del zoom actual
  	   *uy=getCoord_pixel(_Y_,y);//Numeros digitales del limite del zoom actual
  	    }//fin if------------------------------------
   if((*ux==zoom.Cx)||(*uy==zoom.Cy)){
	   *ux=0;*uy=0;}
}//FIN  del Acondicionamiento MATEMATICO  de la seÃ±al del ADC canal X-----------------------------------


/*eje:  x|y es el eje al cual vamos a calcuar el zoom
 *  xy: es el valor que se ah detectado sin offset
 *  Maximo es el valor desplegable en a pantalla que se puede ver
 *  zoomax: es el valor del zoom maximo en el eje X|Y en el zoom dado
 * */
unsigned char zoomXY(unsigned char eje,signed short int xy,float div){
//byte ret;
signed short int n;

 if(eje==_X_){
   if(xy==0)
	  return(zoom.Cx);//centro
   if(xy>0){ //valor positivo
	   if(xy>zoom.Maxx){
		   return zoom.Cx;}//ZOOM_MAX_X_99=96,
	   else{ n=((signed short int)(xy/div));
			if(n==0)
				return zoom.Cx;
			else 	
				return ((unsigned char)(n+ZOOM_X_CENTER));}}//ec. recta (0,96),(96,192)-->(signed short int,DDS@char)
   else{if(xy<(zoom.Maxx*-1)){
	       return(zoom.Cx);}//centro
		else{n=((signed short int)(xy/div));
			 return ((unsigned char)(n+ZOOM_X_CENTER));}}
          }//FIN X

  if(eje==_Y_){
      if(xy==0)
		 return zoom.Cy;
	  if(xy>0){
		 if(xy>zoom.Maxy)
			 return zoom.Cy;
		 else{
			  n=((signed short int)(xy/div));
			  if(n==0)
				  return zoom.Cy;
			  else
				  return ((unsigned char)((-1*n)+ZOOM_Y_CENTER));}}//ec.recta (0,64),(64,0)
	  else {if(xy<(zoom.Maxy*-1)){
			  return zoom.Cy;}
			else{
				n=((signed short int)(xy/div));
				if(n==0)
					return ((byte)zoom.Cy);
				else
				   return ((unsigned char)((-1*n)+ZOOM_Y_CENTER));}}}	
return 0;		   
}//fin zoomXY------------------------------------------------


/*convierte el numero xy si esta dentro del limite min y max a numero para desplegar en
 * el display  
 * xy: numero digital de la señal de deteccion
 * min: minimo que se en la pantalla con el zoom seleccionado
 * max: maximo numero que se ve en pantalla con el zoom seleccinado
 * insignea: valor de X|Y indica si es para x o y */
unsigned char getCoord_pixel(unsigned char insignea,signed short int xy){
//unsigned short int xy1;
//unsigned char jj;
//unsigned short int uxy=0,umax;
unsigned char ret;
//signed short int n;

   switch(insignea){
	   case _X_:
	   		   switch(zoom.Zoom){
	   			   case 99:if(xy==0)
	   				            ret=zoom.Cx;
	   			           if(xy>0){ //valor positivo
	   			        	   if(xy>zoom.Maxx){
	   			        		   ret=zoom.Cx;}//ZOOM_MAX_X_99=96,
	   			        	   else ret=((unsigned char)(xy+ZOOM_X_CENTER));}//ec. recta (0,96),(96,192)-->(signed short int,DDS@char)
	   			           else{if(xy<(zoom.Maxx*-1))
	   			        	        ret=zoom.Cx;//centro
	   			           	    else
	   			        	        ret=((unsigned char)(xy+ZOOM_X_CENTER));}//ec. recta (0,96),(-96,0)
	   			           break;
	   			case 98:ret=zoomXY(_X_,xy,1.2);break;
	   			case 97:ret=zoomXY(_X_,xy,1.3);break;
				case 96:ret=zoomXY(_X_,xy,1.6);break;
				case 95:ret=zoomXY(_X_,xy,1.7);break;
				case 94:ret=zoomXY(_X_,xy,1.8);break;
				case 93:ret=zoomXY(_X_,xy,1.9);break;
				case 92:ret=zoomXY(_X_,xy,2.1);break;
				case 91:ret=zoomXY(_X_,xy,2.3);break;
				case 90:ret=zoomXY(_X_,xy,2.5);break;
				case 89:ret=zoomXY(_X_,xy,3.1);break;
				case 88:ret=zoomXY(_X_,xy,3.5);break;
				case 87:ret=zoomXY(_X_,xy,3.8);break;
				case 86:ret=zoomXY(_X_,xy,4.1);break;
				case 85:ret=zoomXY(_X_,xy,4.2);break;
				case 84:ret=zoomXY(_X_,xy,4.3);break;
				case 83:ret=zoomXY(_X_,xy,4.8);break;
				case 82:ret=zoomXY(_X_,xy,5.2);break;
				case 81:ret=zoomXY(_X_,xy,5.5);break;
				case 80:ret=zoomXY(_X_,xy,5.8);break;
				case 79:ret=zoomXY(_X_,xy,5.9);break;
				case 78:ret=zoomXY(_X_,xy,6.1);break;
				case 77:ret=zoomXY(_X_,xy,6.2);break;
				case 76:ret=zoomXY(_X_,xy,6.5);break;
				case 75:ret=zoomXY(_X_,xy,6.9);break;
				case 74:ret=zoomXY(_X_,xy,7.2);break;
				case 73:ret=zoomXY(_X_,xy,7.5);break;
				case 72:ret=zoomXY(_X_,xy,7.8);break;
				case 71:ret=zoomXY(_X_,xy,8.1);break;
				case 70:ret=zoomXY(_X_,xy,8.2);break;
				case 69:ret=zoomXY(_X_,xy,8.6);break;
				case 68:ret=zoomXY(_X_,xy,9.1);break;
				case 67:ret=zoomXY(_X_,xy,9.5);break;
				case 66:ret=zoomXY(_X_,xy,9.8);break;
				case 65:ret=zoomXY(_X_,xy,10);break;
				case 64:ret=zoomXY(_X_,xy,10.5);break;
				case 63:ret=zoomXY(_X_,xy,11);break;
				case 62:ret=zoomXY(_X_,xy,12);break;
				case 61:ret=zoomXY(_X_,xy,14);break;
				case 60:ret=zoomXY(_X_,xy,18);break;
				case 59:ret=zoomXY(_X_,xy,11);break;
				case 58:ret=zoomXY(_X_,xy,20);break;
				case 57:ret=zoomXY(_X_,xy,22);break;
				case 56:ret=zoomXY(_X_,xy,24);break;
				case 55:ret=zoomXY(_X_,xy,26);break;
				case 54:ret=zoomXY(_X_,xy,27);break;
				case 53:ret=zoomXY(_X_,xy,28);break;
				case 52:ret=zoomXY(_X_,xy,32);break;
				case 51:ret=zoomXY(_X_,xy,34);break;
				case 50:ret=zoomXY(_X_,xy,36);break;
				case 49:ret=zoomXY(_X_,xy,37);break;
				case 48:ret=zoomXY(_X_,xy,38);break;
				case 47:ret=zoomXY(_X_,xy,39);break;
				case 46:ret=zoomXY(_X_,xy,40);break;
				case 45:ret=zoomXY(_X_,xy,42);break;
				case 44:ret=zoomXY(_X_,xy,44);break;
				case 43:ret=zoomXY(_X_,xy,46);break;
				case 42:ret=zoomXY(_X_,xy,48);break;
				case 41:ret=zoomXY(_X_,xy,52);break;
				case 40:ret=zoomXY(_X_,xy,54);break;
				case 39:ret=zoomXY(_X_,xy,56);break;
				case 38:ret=zoomXY(_X_,xy,58);break;
				case 37:ret=zoomXY(_X_,xy,59);break;
				case 36:ret=zoomXY(_X_,xy,61);break;
				case 35:ret=zoomXY(_X_,xy,62);break;
				case 34:ret=zoomXY(_X_,xy,64);break;
				case 33:ret=zoomXY(_X_,xy,66);break;
				case 32:ret=zoomXY(_X_,xy,68);break;
				case 31:ret=zoomXY(_X_,xy,72);break;
				case 30:ret=zoomXY(_X_,xy,74);break;
				case 29:ret=zoomXY(_X_,xy,76);break;
				case 28:ret=zoomXY(_X_,xy,78);break;
				case 27:ret=zoomXY(_X_,xy,79);break;
				case 26:ret=zoomXY(_X_,xy,85);break;
				case 25:ret=zoomXY(_X_,xy,80);break;
				case 24:ret=zoomXY(_X_,xy,85);break;
				case 23:ret=zoomXY(_X_,xy,80);break;
				case 22:ret=zoomXY(_X_,xy,85);break;
				case 21:ret=zoomXY(_X_,xy,80);break;
				case 20:ret=zoomXY(_X_,xy,90);break;
				case 19:ret=zoomXY(_X_,xy,100);break;
				case 18:ret=zoomXY(_X_,xy,110);break;
				case 17:ret=zoomXY(_X_,xy,120);break;
				case 16:ret=zoomXY(_X_,xy,130);break;
				case 15:ret=zoomXY(_X_,xy,140);break;
				case 14:ret=zoomXY(_X_,xy,150);break;
				case 13:ret=zoomXY(_X_,xy,160);break;
				case 12:ret=zoomXY(_X_,xy,170);break;
				case 11:ret=zoomXY(_X_,xy,180);break;
				case 10:ret=zoomXY(_X_,xy,190);break;
				case 9: ret=zoomXY(_X_,xy,210);break;
				case 8: ret=zoomXY(_X_,xy,220);break;
				case 7: ret=zoomXY(_X_,xy,230);break;	
				case 6: ret=zoomXY(_X_,xy,250);break;
				case 5: ret=zoomXY(_X_,xy,260);break;
				case 4: ret=zoomXY(_X_,xy,270);break;
				case 3: ret=zoomXY(_X_,xy,280);break;
				case 2: ret=zoomXY(_X_,xy,300);break;
				case 1: ret=zoomXY(_X_,xy,330);break;
			
	   			default:ret=0;break;//error si entra aqui
	   		   }//fin switch zoom _X_
	   		   break;
	   	   case _Y_:
	   		  switch(zoom.Zoom){ 
	   			  case 99:if(xy==0)
	   		             	 ret=zoom.Cy;
	   	            	  if(xy>0){
	   	            		  if(xy>zoom.Maxy)
	   	            			 ret=zoom.Cy;
	   	            		  else 
	   	            		     return ((unsigned char)((-1*xy)+ZOOM_Y_CENTER));}//ec.recta (0,64),(64,0)
	   	            	  else {if(xy<(zoom.Maxy*-1))
	   	            		         ret= zoom.Cy;
	   	            		    ret= ((unsigned char)((-1*xy)+ZOOM_Y_CENTER));}	 //ec.recta(0,64),(-64,128)
	   	                  break;
	   			case 98:ret=zoomXY(_Y_,xy,1.2);break;
				case 97:ret=zoomXY(_Y_,xy,1.3);break;  	
				case 96:ret=zoomXY(_Y_,xy,1.6);break;
				case 95:ret=zoomXY(_Y_,xy,1.7);break;
				case 94:ret=zoomXY(_Y_,xy,1.8);break;
				case 93:ret=zoomXY(_Y_,xy,1.9);break;
				case 92:ret=zoomXY(_Y_,xy,2.1);break;
				case 91:ret=zoomXY(_Y_,xy,2.3);break;
				case 90:ret=zoomXY(_Y_,xy,2.5);break;
				case 89:ret=zoomXY(_Y_,xy,3.1);break;
				case 88:ret=zoomXY(_Y_,xy,3.5);break;
				case 87:ret=zoomXY(_Y_,xy,3.8);break;
				case 86:ret=zoomXY(_Y_,xy,4.1);break;
				case 85:ret=zoomXY(_Y_,xy,4.2);break;
				case 84:ret=zoomXY(_Y_,xy,4.3);break;
				case 83:ret=zoomXY(_Y_,xy,4.8);break;
				case 82:ret=zoomXY(_Y_,xy,5.2);break;
				case 81:ret=zoomXY(_Y_,xy,5.5);break;
				case 80:ret=zoomXY(_Y_,xy,5.8);break;
				case 79:ret=zoomXY(_Y_,xy,5.9);break;
				case 78:ret=zoomXY(_Y_,xy,6.1);break;
				case 77:ret=zoomXY(_Y_,xy,6.2);break;
				case 76:ret=zoomXY(_Y_,xy,6.5);break;
				case 75:ret=zoomXY(_Y_,xy,6.9);break;
				case 74:ret=zoomXY(_Y_,xy,7.2);break;
				case 73:ret=zoomXY(_Y_,xy,7.5);break;
				case 72:ret=zoomXY(_Y_,xy,7.8);break;
				case 71:ret=zoomXY(_Y_,xy,8.1);break;
				case 70:ret=zoomXY(_Y_,xy,8.2);break;
				case 69:ret=zoomXY(_Y_,xy,8.6);break;
				case 68:ret=zoomXY(_Y_,xy,9.1);break;
				case 67:ret=zoomXY(_Y_,xy,9.5);break;
				case 66:ret=zoomXY(_Y_,xy,9.8);break;
				case 65:ret=zoomXY(_Y_,xy,10);break;
				case 64:ret=zoomXY(_Y_,xy,10.5);break;
				case 63:ret=zoomXY(_Y_,xy,11);break;
				case 62:ret=zoomXY(_Y_,xy,12);break;
				case 61:ret=zoomXY(_Y_,xy,14);break;
				case 60:ret=zoomXY(_Y_,xy,18);break;
				case 59:ret=zoomXY(_Y_,xy,11);break;
				case 58:ret=zoomXY(_Y_,xy,20);break;
				case 57:ret=zoomXY(_Y_,xy,22);break;
				case 56:ret=zoomXY(_Y_,xy,24);break;
				case 55:ret=zoomXY(_Y_,xy,26);break;
				case 54:ret=zoomXY(_Y_,xy,27);break;
				case 53:ret=zoomXY(_Y_,xy,28);break;
				case 52:ret=zoomXY(_Y_,xy,32);break;
				case 51:ret=zoomXY(_Y_,xy,34);break;
				case 50:ret=zoomXY(_Y_,xy,36);break;
				case 49:ret=zoomXY(_Y_,xy,37);break;
				case 48:ret=zoomXY(_Y_,xy,38);break;
				case 47:ret=zoomXY(_Y_,xy,39);break;
				case 46:ret=zoomXY(_Y_,xy,40);break;
				case 45:ret=zoomXY(_Y_,xy,42);break;
				case 44:ret=zoomXY(_Y_,xy,44);break;
				case 43:ret=zoomXY(_Y_,xy,46);break;
				case 42:ret=zoomXY(_Y_,xy,48);break;
				case 41:ret=zoomXY(_Y_,xy,52);break;
				case 40:ret=zoomXY(_Y_,xy,54);break;
				case 39:ret=zoomXY(_Y_,xy,56);break;
				case 38:ret=zoomXY(_Y_,xy,58);break;
				case 37:ret=zoomXY(_Y_,xy,59);break;
				case 36:ret=zoomXY(_Y_,xy,61);break;
				case 35:ret=zoomXY(_Y_,xy,62);break;
				case 34:ret=zoomXY(_Y_,xy,64);break;
				case 33:ret=zoomXY(_Y_,xy,66);break;
				case 32:ret=zoomXY(_Y_,xy,68);break;
				case 31:ret=zoomXY(_Y_,xy,72);break;
				case 30:ret=zoomXY(_Y_,xy,74);break;
				case 29:ret=zoomXY(_Y_,xy,76);break;
				case 28:ret=zoomXY(_Y_,xy,78);break;
				case 27:ret=zoomXY(_Y_,xy,79);break;
				case 26:ret=zoomXY(_Y_,xy,85);break;
				case 25:ret=zoomXY(_Y_,xy,80);break;
				case 24:ret=zoomXY(_Y_,xy,85);break;
				case 23:ret=zoomXY(_Y_,xy,80);break;
				case 22:ret=zoomXY(_Y_,xy,85);break;
				case 21:ret=zoomXY(_Y_,xy,80);break;
				case 20:ret=zoomXY(_Y_,xy,90);break;
				case 19:ret=zoomXY(_Y_,xy,100);break;
				case 18:ret=zoomXY(_Y_,xy,110);break;
				case 17:ret=zoomXY(_Y_,xy,120);break;
				case 16:ret=zoomXY(_Y_,xy,130);break;
				case 15:ret=zoomXY(_Y_,xy,140);break;
				case 14:ret=zoomXY(_Y_,xy,150);break;
				case 13:ret=zoomXY(_Y_,xy,160);break;
				case 12:ret=zoomXY(_Y_,xy,170);break;
				case 11:ret=zoomXY(_Y_,xy,180);break;
				case 10:ret=zoomXY(_Y_,xy,190);break;
				case 9: ret=zoomXY(_Y_,xy,210);break;
				case 8: ret=zoomXY(_Y_,xy,220);break;
				case 7: ret=zoomXY(_Y_,xy,230);break;	
				case 6: ret=zoomXY(_Y_,xy,250);break;
				case 5: ret=zoomXY(_Y_,xy,150);break;
				case 4: ret=zoomXY(_Y_,xy,280);break;
				case 3: ret=zoomXY(_Y_,xy,300);break;
				case 2: ret=zoomXY(_Y_,xy,400);break;
				case 1: ret=zoomXY(_Y_,xy,500);break;
				default:break;
		  }//fin switch 99
	   default:break; 
   }//fin switch xy----------------------------
 
return ret;   
}//fin getCoord_pixel----------------------------------------------------------------------------------------------------


/*evalua si el valor digital del voltage de deteccion esta dentro del rango 
 * del zoom del DDS correspondiente para que se despliegue, solo acepta valores positivos*/
unsigned char isDDS_pixel_outRange(signed short int signal,signed short int min,signed short int  max){
signed short int s;		
	if(signal<0)
		s=signal*-1;//lo convertimos en positivo
	else s=signal;
	//es positivo el valor
		if(s<min)
			return TRUE;//salio del rango
		else {if(s>max)
		    	   return TRUE;//salio del rango
		      else{return FALSE;}}//esta en rango
}//fin isDDS_pixel_outRange-------------------------------------------------------------



/*  convierte el valor de digital de complento-2 a coordenadas reales de 
 * despliegue de la pantalla DDS *  e.g vaor digital 1200  a  valor DDS 32 en rango de hori de 0 a 192/2 */
unsigned char convert_val2DDS_pixel(unsigned char insignea,signed short int n){
signed short int ret;	 
	 if(n==0){
	    if(insignea==_X_)
	        ret=zoom.Cx; /*CENTRO_X*/
	    else
		    ret=zoom.Cy; /*CENTRO_Y;*/}
	 else{
		 if(insignea==_X_){
			  ret=getDDSpixelX(n);}
		 else { 
			  ret=getDDSpixelY(n);}}	    
    
return(unsigned char)ret;         
}//fin convert_coord_to_DDS_pixel------------------------------------------------------


void ProcesamientoDigitalPolinomico(signed short int *x){
	float  b1,b2,b3,b4;
	signed short int b;    
             b=*x;
		       if(*x>0){          
		        if(*x<255){  //   (5,90)(49,793)(182,2924)(230,3681)(250,4008)
		    	     b1=((2072357753)*((float)b)*((float)b)*((float)b)*((float)b))/8968693307112000;
		    	     b2=((-57297367131)*((float)b)*((float)b)*((float)b))/498260739284000;
		    	     b3=(13740598861579*((float)b)*((float)b))/815335755192000;
		    	     b4=(54609761473439*((float)b))/3559005280600;
		             *x=(signed short int)(b1+b2+b3+b4+12.872372);
		             return;}
		       return;}//x positivo
		       if(*x<0){
		    	  if(*x>-255){//   (-3,-39)(-42,-664)(-104,-1657)(-223,-3564)(-252,-4016)
		    		  b1=(6216901211*((float)b)*((float)b)*((float)b)*((float)b))/103063218754455960;  
		    		  b2=((1190155319891)*((float)b)*((float)b)*((float)b))/51531609377227980;
		    		  b3=(1415633106493*((float)b)*((float)b))/569410048367160;
		    		  b4=(829580141958292051*((float)b))/51531609377227980;
		    		  *x=(signed short int)(b1+b2+b3+b4+9.2736);
		    	     return;}
		    return;}	  
}//fin del proceamiento digital de interpolacion polinomica------------------------------------------------------------








/*************************************************************************************************
 * **********************************funciones de*****CONTROL*DEL**ZOOM*++************************
 * ********funciones*********CONTROL*DEL**ZOOM***************************************************
 * ***********************************************************************************************
 * *****************funciones+******************CONTROL*DEL**ZOOM*********************************
 * ***********************************************************************************************
 * */


/*    32678=ADC_MAX		/*  tenemos,queremos     x0,x   x0,x   x0,x
 *  zoomMax=99		 * *            //X: (_MAX,LIM_X)(0,CENTRO_X)(-_MAX,0)     
 *  ADC_MAX-g=64@zoom=1		 *  tenemos,queremos    y0,y    y0,y  y0,y 
 *  g=ADC_MAX-64=32614		               //Y: (_MAX,0),(0,CENTRO_Y)(-_MAX,LIM_Y)
 *  zfactor=g/(zoomMAx-1)=332.7959		 
 *  Max=ADC_MAX-zfactor(zoom-1)		                                  
 *  		   LIM_X     190
 *  Max@zoom->99=64		  LIM_Y     127
 *  indice conversion= (Max@zoom->99)/ADC_MAX=0.00195850
 *  para sacar el algoritmo:https://www.onlinegdb.com/online_c_compiler
*   codigo:
*   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*   
*   
#include <stdio.h>
    int main()
    {unsigned char i;
     int j,k,n,m,kk;
     printf("Hello World\n");
     
     j=0;
     k=300;
     n=0;
     m=200;
     printf("\n\n\n\n\n");
     for(i=1;i<=99;i++){
        printf("case %i:maxx=ZOOM_MAX_X_%i; maxx=%i; miny=%i; maxy=%i;break; \n",i,j,k,n,m);
        j=k-(k/100)*45;
        k=k+290;
        n=m-(m/100)*45;
        m=m+244; }
     
     printf("\n\n******************************++++++++++++++++++**********************\n\n");
     printf("******************************++++++++++++++++++**********************\n\n");
     
     j=28720;
     k=15650;
     n=24112;
     m=13158;
     for(i=1;i<=99;i++){
        printf("case %i:maxx=ZOOM_MAX_X_%i; maxx=%i; miny=%i; maxy=%i;break; \n",i,k,j,m,n);
        j=k+(k/100)*85;//la profundidad de comienzo a apartir del ultimo del futuro
        k=k-159;//
        n=m+(m/100)*77;
        m=m-134;}
       
    return 0;}
*   
*   
    ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Zoom_init  SE DEBE EJECUTAR CUANDO CAMBIA EL ZOOM  y se esta en el contexto pantalla DDS*/
void Zoom_init(void){          //unsigned short int  x99= (32000,190)(0,95)(-32000,0)
unsigned short int maxx,maxy,ymax,ymin;	
	     if(zoom.Zoom==0)//el zoom no puede ser cero o crearia un error
	    	 zoom.Zoom=99;
	     zoom.zFactor=32704/98;// Los zooms no son lineales los incrementos
	     zoom.Max=(signed short int)(32767-zoom.zFactor*(unsigned short int)(zoom.Zoom-1));//Maximo numero graficable en este zoom seleccionado			     zoom.Max=LIM_Y;//tamaÃ±o del radio del plano cartesiano a dibujar
	     zoom.indiceConversion= 64/((float)zoom.Max);
	     
	     zoom.Cx=CENTRO_X;
	     zoom.Cy=CENTRO_Y;
	     
	     switch(zoom.Zoom){
			case 0:__asm(Halt);//debug error de software        
			case 1: maxx=ZOOM_MAX_X_01;    maxy=ZOOM_MAX_Y_01;ymax=23919;ymin=369; break;                                 
			case 2: maxx=ZOOM_MAX_X_02;    maxy=ZOOM_MAX_Y_02;ymax=2649;ymin=81; break;                                 
			case 3: maxx=ZOOM_MAX_X_03;    maxy=ZOOM_MAX_Y_03;ymax=1407;ymin=43; break;                                 
			case 4: maxx=ZOOM_MAX_X_04;    maxy=ZOOM_MAX_Y_04;ymax=956;ymin=29;break;                                 
			case 5: maxx=ZOOM_MAX_X_05;    maxy=ZOOM_MAX_Y_05;ymax=724;ymin=22; break;                                 
			case 6: maxx=ZOOM_MAX_X_06;    maxy=ZOOM_MAX_Y_06;ymax=583;ymin=17; break;                                 
			case 7: maxx=ZOOM_MAX_X_07;    maxy=ZOOM_MAX_Y_07;ymax=488;ymin=15; break;                                 
			case 8: maxx=ZOOM_MAX_X_08;    maxy=ZOOM_MAX_Y_08;ymax=419;ymin=12; break;                                 
			case 9: maxx=ZOOM_MAX_X_09;    maxy=ZOOM_MAX_Y_09;ymax=367;ymin=11; break;                                 
			case 10:maxx=ZOOM_MAX_X_10;    maxy=ZOOM_MAX_Y_10;ymax=327;ymin=10; break;                                
			case 11:maxx=ZOOM_MAX_X_11;    maxy=ZOOM_MAX_Y_11;ymax=295;ymin=9; break;                                
			case 12:maxx=ZOOM_MAX_X_12;    maxy=ZOOM_MAX_Y_12;ymax=268;ymin=8; break;                                
			case 13:maxx=ZOOM_MAX_X_13;    maxy=ZOOM_MAX_Y_13;ymax=246;ymin=7; break;                                
			case 14:maxx=ZOOM_MAX_X_14;    maxy=ZOOM_MAX_Y_14;ymax=227;ymin=7; break;                                
			case 15:maxx=ZOOM_MAX_X_15;    maxy=ZOOM_MAX_Y_15;ymax=211;ymin=6; break;                                
			case 16:maxx=ZOOM_MAX_X_16;    maxy=ZOOM_MAX_Y_16;ymax=197;ymin=6; break;                                
			case 17:maxx=ZOOM_MAX_X_17;    maxy=ZOOM_MAX_Y_17;ymax=185;ymin=5; break;                                
			case 18:maxx=ZOOM_MAX_X_18;    maxy=ZOOM_MAX_Y_18;ymax=174;ymin=5; break;                                
			case 19:maxx=ZOOM_MAX_X_19;    maxy=ZOOM_MAX_Y_19;ymax=164;ymin=5; break;                                
			case 20:maxx=ZOOM_MAX_X_20;    maxy=ZOOM_MAX_Y_20;ymax=156;ymin=4; break;                                
			case 21:maxx=ZOOM_MAX_X_21;    maxy=ZOOM_MAX_Y_21;ymax=148;ymin=4; break;                                
			case 22:maxx=ZOOM_MAX_X_22;    maxy=ZOOM_MAX_Y_22;ymax=141;ymin=4; break;                                
			case 23:maxx=ZOOM_MAX_X_23;    maxy=ZOOM_MAX_Y_23;ymax=135;ymin=4; break;                                
			case 24:maxx=ZOOM_MAX_X_24;    maxy=ZOOM_MAX_Y_24;ymax=129;ymin=3; break;                                
			case 25:maxx=ZOOM_MAX_X_25;    maxy=ZOOM_MAX_Y_25;ymax=123;ymin=3; break;                                 
			case 26:maxx=ZOOM_MAX_X_26;    maxy=ZOOM_MAX_Y_26;ymax=119;ymin=3; break; 
			case 27:maxx=ZOOM_MAX_X_27;    maxy=ZOOM_MAX_Y_27;ymax=114;ymin=3; break; 
			case 28:maxx=ZOOM_MAX_X_28;    maxy=ZOOM_MAX_Y_28;ymax=110;ymin=3; break; 
			case 29:maxx=ZOOM_MAX_X_29;    maxy=ZOOM_MAX_Y_29;ymax=106;ymin=3; break; 
			case 30:maxx=ZOOM_MAX_X_30;    maxy=ZOOM_MAX_Y_30;ymax=102;ymin=3; break; 
			case 31:maxx=ZOOM_MAX_X_31;    maxy=ZOOM_MAX_Y_31;ymax=99;ymin=3; break; 
			case 32:maxx=ZOOM_MAX_X_32;    maxy=ZOOM_MAX_Y_32;ymax=96;ymin=2; break; 
			case 33:maxx=ZOOM_MAX_X_33;    maxy=ZOOM_MAX_Y_33;ymax=93;ymin=2; break; 
			case 34:maxx=ZOOM_MAX_X_34;    maxy=ZOOM_MAX_Y_34;ymax=89;ymin=2; break; 
			case 35:maxx=ZOOM_MAX_X_35;    maxy=ZOOM_MAX_Y_35;ymax=87;ymin=2; break; 
			case 36:maxx=ZOOM_MAX_X_36;    maxy=ZOOM_MAX_Y_36;ymax=85;ymin=2; break; 
			case 37:maxx=ZOOM_MAX_X_37;    maxy=ZOOM_MAX_Y_37;ymax=82;ymin=2; break; 
			case 38:maxx=ZOOM_MAX_X_38;    maxy=ZOOM_MAX_Y_38;ymax=80;ymin=2; break; 
			case 39:maxx=ZOOM_MAX_X_39;    maxy=ZOOM_MAX_Y_39;ymax=78;ymin=2; break; 
			case 40:maxx=ZOOM_MAX_X_40;    maxy=ZOOM_MAX_Y_40;ymax=75;ymin=2; break; 
			case 41:maxx=ZOOM_MAX_X_41;    maxy=ZOOM_MAX_Y_41;ymax=74;ymin=2;  break; 
			case 42:maxx=ZOOM_MAX_X_42;    maxy=ZOOM_MAX_Y_42;ymax=72;ymin=2;  break; 
			case 43:maxx=ZOOM_MAX_X_43;    maxy=ZOOM_MAX_Y_43;ymax=70;ymin=2;  break; 
			case 44:maxx=ZOOM_MAX_X_44;    maxy=ZOOM_MAX_Y_44;ymax=69;ymin=2;  break; 
			case 45:maxx=ZOOM_MAX_X_45;    maxy=ZOOM_MAX_Y_45;ymax=67;ymin=2;  break; 
			case 46:maxx=ZOOM_MAX_X_46;    maxy=ZOOM_MAX_Y_46;ymax=66;ymin=2;  break; 
			case 47:maxx=ZOOM_MAX_X_47;    maxy=ZOOM_MAX_Y_47;ymax=64;ymin=1;  break; 
			case 48:maxx=ZOOM_MAX_X_48;    maxy=ZOOM_MAX_Y_48;ymax=63;ymin=1;  break; 
			case 49:maxx=ZOOM_MAX_X_49;    maxy=ZOOM_MAX_Y_49;ymax=61;ymin=1;  break; 
			case 50:maxx=ZOOM_MAX_X_50;    maxy=ZOOM_MAX_Y_50;ymax=60;ymin=1;  break; 
			case 51:maxx=ZOOM_MAX_X_51;    maxy=ZOOM_MAX_Y_51;ymax=59;ymin=1;  break; 
			case 52:maxx=ZOOM_MAX_X_52;    maxy=ZOOM_MAX_Y_52;ymax=58;ymin=1;  break; 
			case 53:maxx=ZOOM_MAX_X_53;    maxy=ZOOM_MAX_Y_53;ymax=57;ymin=1;  break; 
			case 54:maxx=ZOOM_MAX_X_54;    maxy=ZOOM_MAX_Y_54;ymax=56;ymin=1;  break; 
			case 55:maxx=ZOOM_MAX_X_55;    maxy=ZOOM_MAX_Y_55;ymax=55;ymin=1;  break;                                  
			case 56:maxx=ZOOM_MAX_X_56;    maxy=ZOOM_MAX_Y_56;ymax=54;ymin=1;  break;                                  
			case 57:maxx=ZOOM_MAX_X_57;    maxy=ZOOM_MAX_Y_57;ymax=53;ymin=1;  break;                                  
			case 58:maxx=ZOOM_MAX_X_58;    maxy=ZOOM_MAX_Y_58;ymax=52;ymin=1;  break;                                   
			case 59:maxx=ZOOM_MAX_X_59;    maxy=ZOOM_MAX_Y_59;ymax=51;ymin=1;  break;                                   
			case 60:maxx=ZOOM_MAX_X_60;    maxy=ZOOM_MAX_Y_60;ymax=50;ymin=1;  break;                                   
			case 61:maxx=ZOOM_MAX_X_61;    maxy=ZOOM_MAX_Y_61;ymax=49;ymin=1;  break;                                   
			case 62:maxx=ZOOM_MAX_X_62;    maxy=ZOOM_MAX_Y_62;ymax=48;ymin=1; break;                                    
			case 63:maxx=ZOOM_MAX_X_63;    maxy=ZOOM_MAX_Y_63;ymax=48;ymin=1; break;                                   
			case 64:maxx=ZOOM_MAX_X_64;    maxy=ZOOM_MAX_Y_64;ymax=47;ymin=1; break;                                   
			case 65:maxx=ZOOM_MAX_X_65;    maxy=ZOOM_MAX_Y_65;ymax=46;ymin=1; break;                                  
			case 66:maxx=ZOOM_MAX_X_66;    maxy=ZOOM_MAX_Y_66;ymax=45;ymin=1; break;                                   
			case 67:maxx=ZOOM_MAX_X_67;    maxy=ZOOM_MAX_Y_67;ymax=45;ymin=1; break;                                    
			case 68:maxx=ZOOM_MAX_X_68;    maxy=ZOOM_MAX_Y_68;ymax=44;ymin=1; break;                                     
			case 69:maxx=ZOOM_MAX_X_69;    maxy=ZOOM_MAX_Y_69;ymax=43;ymin=1; break;                                     
			case 70:maxx=ZOOM_MAX_X_70;    maxy=ZOOM_MAX_Y_70;ymax=43;ymin=1; break;                                    
			case 71:maxx=ZOOM_MAX_X_71;    maxy=ZOOM_MAX_Y_71;ymax=42;ymin=1; break;                                    
			case 72:maxx=ZOOM_MAX_X_72;    maxy=ZOOM_MAX_Y_72;ymax=42;ymin=1; break;                                    
			case 73:maxx=ZOOM_MAX_X_73;    maxy=ZOOM_MAX_Y_73;ymax=41;ymin=1; break;                                     
			case 74:maxx=ZOOM_MAX_X_74;    maxy=ZOOM_MAX_Y_74;ymax=40;ymin=1; break;                                     
			case 75:maxx=ZOOM_MAX_X_75;    maxy=ZOOM_MAX_Y_75;ymax=40;ymin=1; break;                                    
			case 76:maxx=ZOOM_MAX_X_76;    maxy=ZOOM_MAX_Y_76;ymax=39;ymin=1; break;                                     
			case 77:maxx=ZOOM_MAX_X_77;    maxy=ZOOM_MAX_Y_77;ymax=39;ymin=1; break;                                     
			case 78:maxx=ZOOM_MAX_X_78;    maxy=ZOOM_MAX_Y_78;ymax=38;ymin=1; break;                                    
			case 79:maxx=ZOOM_MAX_X_79;    maxy=ZOOM_MAX_Y_79;ymax=38;ymin=1; break;
			case 80:maxx=ZOOM_MAX_X_80;    maxy=ZOOM_MAX_Y_80;ymax=37;ymin=1; break;                                    
			case 81:maxx=ZOOM_MAX_X_81;    maxy=ZOOM_MAX_Y_81;ymax=37;ymin=1; break;                                    
			case 82:maxx=ZOOM_MAX_X_82;    maxy=ZOOM_MAX_Y_82;ymax=36;ymin=1; break;
			case 83:maxx=ZOOM_MAX_X_83;    maxy=ZOOM_MAX_Y_83;ymax=36;ymin=1; break;                                    
			case 84:maxx=ZOOM_MAX_X_84;    maxy=ZOOM_MAX_Y_84;ymax=35;ymin=1; break;                                    
			case 85:maxx=ZOOM_MAX_X_85;    maxy=ZOOM_MAX_Y_85;ymax=35;ymin=1; break;
			case 86:maxx=ZOOM_MAX_X_86;    maxy=ZOOM_MAX_Y_86;ymax=35;ymin=1; break;                                   			                                  
			case 87:maxx=ZOOM_MAX_X_87;    maxy=ZOOM_MAX_Y_87;ymax=34;ymin=1; break;
			case 88:maxx=ZOOM_MAX_X_88;    maxy=ZOOM_MAX_Y_88;ymax=34;ymin=1; break;
			case 89:maxx=ZOOM_MAX_X_89;    maxy=ZOOM_MAX_Y_89;ymax=33;ymin=2; break;
			case 90:maxx=ZOOM_MAX_X_90;    maxy=ZOOM_MAX_Y_90;ymax=33;ymin=2; break;  			
			case 91:maxx=ZOOM_MAX_X_91;    maxy=ZOOM_MAX_Y_91;ymax=33;ymin=2; break;                                    
			case 92:maxx=ZOOM_MAX_X_92;    maxy=ZOOM_MAX_Y_92;ymax=32;ymin=2;  break;                                     
			case 93:maxx=ZOOM_MAX_X_93;    maxy=ZOOM_MAX_Y_93;ymax=32;ymin=2;  break;
			case 94:maxx=ZOOM_MAX_X_94;    maxy=ZOOM_MAX_Y_94;ymax=32;ymin=2;  break;	
			case 95:maxx=ZOOM_MAX_X_95;    maxy=ZOOM_MAX_Y_95;ymax=31;ymin=2;  break;   
			case 96:maxx=ZOOM_MAX_X_96;    maxy=ZOOM_MAX_Y_96;ymax=31;ymin=2;  break;                                       
			case 97:maxx=ZOOM_MAX_X_97;    maxy=ZOOM_MAX_Y_97;ymax=31;ymin=2;  break;                                        
			case 98:maxx=ZOOM_MAX_X_98;    maxy=ZOOM_MAX_Y_98; break;
			case 99:maxx=ZOOM_MAX_X_99;    maxy=ZOOM_MAX_Y_99; break;
			        
			default:break;	
	      	     }//fin zoom		
      zoom.Maxx=maxx;//limite del numero digital desplegable en DDS en funciona del Zoom
      zoom.Maxy=maxy; 
      zoom.Ymax=ymax;
      zoom.Ymin=ymin;
	  generarPixelsAnalogos();//(0,0);//genera una lista de valores analogos que corresponden en una lista a un pixel digital en un zoom dado
      
      
}//fin zoom init -----------------------------------------------------------------------------------------



void init_Sensibilidad(void){
unsigned short int cuadroBase;//conversor de sensilidad para 	
	    
 Zoom_init();  
	         Deteccion.EnCurso=NOP;//no hay deteccion
	         Offset.Status=0;//debug poner control con EEPROM AQUI FIRST_START;//no estamos listos para evaluar el offset
	         if(Deteccion.tipo==NORMAL){
	        			cuadroBase=(Deteccion.Sensibilidad)/8;//7 cuadros base
	        			Deteccion.LIM1=cuadroBase;
	        			Deteccion.LIM2=cuadroBase*2;
	        			Deteccion.LIM3=cuadroBase*3;
	        			Deteccion.LIM4=cuadroBase*4;
	        			Deteccion.LIM5=cuadroBase*5;
	        			Deteccion.LIM6=cuadroBase*6;
	        			Deteccion.LIM7=cuadroBase*7;
	        			Deteccion.LIM8=cuadroBase*8;
	        			Deteccion.LIM9=cuadroBase*9;
	        			Deteccion.LIM10=cuadroBase*10;
	        			Deteccion.LIM11=cuadroBase*11;
	        			Deteccion.LIM12=cuadroBase*12;
	        			Deteccion.LIM13=cuadroBase*13;
	        			Deteccion.LIM14=cuadroBase*14;
	        			Deteccion.LIM15=cuadroBase*15;
	        			Deteccion.LIM16=cuadroBase*16;
	        			Deteccion.LIM17=cuadroBase*17;	        
	        			   }//FIN DETECCION normal
	        			else 
	        				__asm(Halt);//Debug it
	        	    
	
}//inizializa la sensibilidad



/*version1:   generar pixel analogos generamos una lista del pixel digital del DDS coordenadas versus el pixel analogo
 * entre que rango y que rango corresponde un pixel digital DDS
 * version2: vamos a rellenar los arrays con los valores corrrespondienes de analogo signed a pixel DDS para
 * la conversion mas facil y rapida corespondiente al zoom seleccionado */
void generarPixelsAnalogos(void){//signed short int Min,signed short int miny){

unsigned short int pixelSize;//tamaño del pixel de analogo versus DDS por zoom selected
unsigned short int jj,i,j;
unsigned char k=0;
       pixelSize=zoom.Maxx/DDS_X_4d8;//DDS_X_4d8=96;  //95..192  ->0..Max
       if(pixelSize==0)//menor al 96,tamaño del pixel es portanto 1
    	   pixelSize=1;
      for(i=DDS_X_4d8,j=0;i<DDS_X_8d8;i++,j+=pixelSize)//parte poitiva de 95 a 192 en x
    		   pixelConversionX[i]=j;
    	   /* genera la parte negativa de las ordenadas  x*/
       jj=zoom.Maxx;//Rellenar  Y  0..95 con->-Max..0
       for(i=DDS_X_4d8-1,j=pixelSize;i>=0;i--,j+=pixelSize){
    	   if(i>DDS_X_4d8)
    		   break;
    	   pixelConversionX[i]=j;}
       zoom.pixelSizeX=pixelSize;
       /* genera la parte positiva de las absisas Y  con calculo inteligente*/
       pixelSize=(unsigned char)(zoom.Maxy/DDS_Y_4d8);
       if(pixelSize==0)
    	   pixelSize=1;//seguro anti fallos.
       for(i=DDS_Y_4d8-1,j=1;i>=0;i--,j+=pixelSize){ // 0..64  +Max..0
           if(i>DDS_Y_4d8)
        	   break;
    	   pixelConversionY[i]=j;}
      /* gener las ordenadas negativas*/   //  64..128 0..-Max      
        for(i=DDS_Y_4d8,j=0;i<=DDS_Y_8d8;i++,j+=pixelSize)    
        	pixelConversionY[i]=j;
       zoom.pixelSizeY=pixelSize;
       
}//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++   	   
           
//para atrapar los valores de loas adc en tiempo real para convertirlo en voltaje       
//void voltage_monitor_catcher(signed short int x,signed short int y){
//	
//	switch(menu.contexto.Actual){
//		case MEDIDAS_DE_VOLTAJES:
//		case MEDIDAS_DE_VOLTAJE:Signal.RawX=x;//gxc3.ssigxc=x;
//		                        Signal.RawY=y;//gxc4.ssigxc=y;
//			                    break;
//		default:break;		
//	}//fin swithc-------------------------------------------------------
//}//FIN E voltage_moniotor_catcher---------------------------------------


//convierte el valor de una signed short int en unsigned short int
//y el parametro por referencia lo pone en 0 si es positivo y en  0xAAAA si es negativo
unsigned short int ADC_getValue16(signed short int *ssi,unsigned char *signo){
signed short int s;
      s=*ssi;//g->ssigxc;
	  if(s==0){
		  *ssi=0;//g->igxc5=0;
		  return 0;}
	  if(s>0){
		  *ssi=0;//g->igxc5=0;
		  return ((unsigned short int)s);}
	  else {
		  *signo='-';//g->igxc5=0xAA;//el numero es negativo
		  s=s*(-1); 
		  return((unsigned short int)s);}
		  	
}//fin ADC_getValue16----------------------------------------



/*CONTROLADOR DE ADQUISICION DE VOLTAJES DE BALANCE Y DRIVER
 * */
void Monitor_Maestro_de_Voltajes(void){
word NumeroADC[2];//aqui se guarda la conversion	
const unsigned char SIZE_NUM=5;
double voltaje;
unsigned char num[SIZE_NUM],signo;
static unsigned char BalVoltmeterFlag;
const unsigned char SIZE3=14;//                x     x                                               x
const double volt[SIZE3]={       8,15.1232,  0.003, 0.231,45.234, 23.123,    93,34.01,9.323,5.323, 0.032,1.104, 1.083, 2.342};
const double volt2[SIZE3]={  0.023, -9.832,-11.232,45.674,   -56,-38.002,-0.001,-5.67,   -8, 9.45,12.342,2.311,-1.012,-0.212};
static unsigned char ii;
signed short int ss;
//static unsigned char iii;
  if(Mediciones.flagMesurements){
     switch(menu.contexto.Actual){
    	 case MEDIDAS_DE_VOLTAJES:  ss=Signal.RawX;//gxc3.ssigxc;
    	                            signo=convertir_SignedShortInt_A_Char(ss,&num[0]);
    		                        display_Voltage_Mediciones(&num[0],RAW_X,&gxc5,signo);
    	                            ss=Signal.RawY;//gxc4.ssigxc;
									signo=convertir_SignedShortInt_A_Char(ss,&num[0]);
									display_Voltage_Mediciones(&num[0],RAW_Y,&gxc6,signo);
									ss=Signal.CentX;//gxc5.ssigxc;
									signo=convertir_SignedShortInt_A_Char(ss,&num[0]);
									display_Voltage_Mediciones(&num[0],CENTRADO_X,&gxc7,signo);
									ss=Signal.CentY;//gxc6.ssigxc;
									signo=convertir_SignedShortInt_A_Char(ss,&num[0]);
									display_Voltage_Mediciones(&num[0],CENTRADO_Y,&gxc8,signo);
									
									ss=gxc7.ssigxc;
									signo=convertir_SignedShortInt_A_Char(ss,&num[0]);
									display_Voltage_Mediciones(&num[0],ROT_X,&gxc9,signo);
									ss=gxc8.ssigxc;
									signo=convertir_SignedShortInt_A_Char(ss,&num[0]);
									display_Voltage_Mediciones(&num[0],ROT_Y,&gxc10,signo);
									
									ss=gxc9.ssigxc;
									signo=convertir_SignedShortInt_A_Char(ss,&num[0]);
									display_Voltage_Mediciones(&num[0],ROT_X,&gxc11,signo);
									ss=gxc10.ssigxc;
									signo=convertir_SignedShortInt_A_Char(ss,&num[0]);
									display_Voltage_Mediciones(&num[0],ROT_Y,&gxc12,signo);
																		
									
    	 case MEDIDAS_DE_VOLTAJE:
    		 	 	 	 	 	 	 ADC_BAL_GetValue16(&NumeroADC[0]);//ADC's internos del micro-controlador
									 voltaje=convertir_ADC_word_A_miliVolt(NumeroADC[0],_PICKUP_);
									 convertir_Voltaje_A_Chars(voltaje,&num[0],_PICKUP_);
									 display_Voltage_Mediciones(&num[0],POSY2,&gxc,0);
									 voltaje=convertir_ADC_word_A_miliVolt(NumeroADC[1],_DRIVER_);
									 convertir_Voltaje_A_Chars(voltaje,&num[0],_DRIVER_);
									 display_Voltage_Mediciones(&num[0],POSY4,&gxc2,0);
									 NumeroADC[0]=ADC_getValue16(&(Signal.RawX),&(Signal.signoX));//numero canal X del ADC deteccion
									 NumeroADC[1]=ADC_getValue16(&(Signal.RawY),&(Signal.signoY));//numero canal Y del ADC deteccion 
									 voltaje=convertir_ADC_word_A_miliVolt(NumeroADC[0],CANAL_X);
									 if(Signal.signoX=='-')//gxc3.igxc5==0xAA)
										 voltaje=voltaje*-1;
									 signo=convertir_Voltaje_A_Chars(voltaje,&num[0],CANAL_X);
									 display_Voltage_Mediciones(&num[0],POSY6,&gxc3,signo);
									 voltaje=convertir_ADC_word_A_miliVolt(NumeroADC[1],CANAL_Y);
									 if(Signal.signoY=='-')//gxc4.igxc5==0xAA)
										 voltaje=voltaje*-1;
									 signo=convertir_Voltaje_A_Chars(voltaje,&num[0],CANAL_Y);
									 display_Voltage_Mediciones(&num[0],POSY8,&gxc4,signo);
									 Mediciones.flagMesurements=FALSE;//Se activa por timer
								 break;	 
    	 default:if(BalVoltmeterFlag++>250){
    		 	 	 BalVoltmeterFlag=0;
    	 	 	 	 ADC_BAL_GetValue16(&NumeroADC[0]);}
    		     break;}
         }//fin if--------------------------------------------------
			
		
}//fin monitor maestro de voltajes-----------------------------------------------------------------------

/* respond true if in the DDS zoom is fit within
 *  the parameters to draw the sensitivity line
 * */
unsigned char is_drawable_Sensibility_DDS(void){
	   
	if((zoom.Zoom>99)||(zoom.Zoom<1)){
	     __asm(nop);__asm(Halt);}
	switch(zoom.Zoom){
		case 99:
	    case 98:if(Deteccion.Sensibilidad<(ZOOM_LIM_99+1))
			      return TRUE;
			    break;
	    default:if(Deteccion.Sensibilidad<zoom.Ymax)
	    	      return TRUE;
	            break;}//fin switch-------------
return FALSE; //no se puede dibujar fuera de rango de zoom	
}//	fin is_drawable_Sensibility-----------------------	


//Regresa TRUE si la phase esta en 000.0 grados
unsigned char is_Phase_0(void){
	
return TRUE;	
}//fin is_Phase_0----------------------------





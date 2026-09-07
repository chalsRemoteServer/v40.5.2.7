/*
 * DSP.h
 *
 *  Created on: Nov 23, 2016
 *      Author: chals
 */

/*
 * DSP.h
 *
 *  Created on: Nov 9, 2016
 *      Author: chals
 *      
 *      Digital Signal Processor Main controller
 */

#ifndef DSP_H_
#define DSP_H_


//#include "queue.h"


//definiciones-----------------------------------------------
#define  _X_        'X'   //vector where adc X values are saved
#define  _Y_        'Y'  //vector where adc Y values are saved
#define  _INIT_   0x30  //to  setup popFIFOdata as in init modo or run mode
#define  _RUN_    0x31  //run mode in 

#define SIZE_FIFO_VIB 15 //tamaño de la pila de  despliegue de lineas para la vibracion
#define SIZE_FIFO_CHAROP 30//10 //Tamaño del la FIFO del char operativo



#define  MAX_OFFSET1 40 //valor maximo que debe haber con offset sin deteccion ni ruido
#define  LIM_X     190
#define  LIM_Y     127
#define  CENTRO_X  95
#define  CENTRO_Y  64
#define  ZOOM_SIZE 100
#define  VIB_DISP_SIZE 10
#define  SIZE_ANGLE_ARR 100//100 //cantidad de datos paa hacer calculo de densidad de angulos para el menu de configuracion de vibracion

#define  INICIO_CMD    0x23
#define  VFD_POSICION  0x24
#define  SEND_CHAR     0x25
#define  FIN_CMD       0x26
#define  SEND_BLOCK    0x27


//para +10v -10v sera 305uV por bit, para +5v -5v sera 150uV por bit
#define ZOOM_X_CENTER  96
#define ZOOM_Y_CENTER  64


//ecuacion  e^((54-663x)/(34*x^2))+((344323-x)/(4x-23))-8x ecuacion para valores maximo de zoom
#define ZOOM_MAX_X_99  96//valor digital Maximo convertido que se desppliega en pantalla
#define ZOOM_MAX_X_98  99
#define ZOOM_MAX_X_97  105
#define ZOOM_MAX_X_96  110
#define ZOOM_MAX_X_95  120
#define ZOOM_MAX_X_94  150
#define ZOOM_MAX_X_93  210 
#define ZOOM_MAX_X_92  290
#define ZOOM_MAX_X_91  370
#define ZOOM_MAX_X_90  410//---
#define ZOOM_MAX_X_89  420
#define ZOOM_MAX_X_88  430
#define ZOOM_MAX_X_87  440
#define ZOOM_MAX_X_86  450
#define ZOOM_MAX_X_85  460
#define ZOOM_MAX_X_84  470
#define ZOOM_MAX_X_83  480
#define ZOOM_MAX_X_82  490
#define ZOOM_MAX_X_81  510
#define ZOOM_MAX_X_80  512 
#define ZOOM_MAX_X_79  532
#define ZOOM_MAX_X_78  557
#define ZOOM_MAX_X_77  587
#define ZOOM_MAX_X_76  622
#define ZOOM_MAX_X_75  662
#define ZOOM_MAX_X_74  702
#define ZOOM_MAX_X_73  710
#define ZOOM_MAX_X_72  747
#define ZOOM_MAX_X_71  769
#define ZOOM_MAX_X_70  795
#define ZOOM_MAX_X_69  855
#define ZOOM_MAX_X_68  920
#define ZOOM_MAX_X_67  990
#define ZOOM_MAX_X_66  1020
#define ZOOM_MAX_X_65  1160
#define ZOOM_MAX_X_64  1250
#define ZOOM_MAX_X_63  1350
#define ZOOM_MAX_X_62  1550
#define ZOOM_MAX_X_61  1850
#define ZOOM_MAX_X_60  2000
#define ZOOM_MAX_X_59  2100
#define ZOOM_MAX_X_58  2250
#define ZOOM_MAX_X_57  2450
#define ZOOM_MAX_X_56  2750
#define ZOOM_MAX_X_55  3100
#define ZOOM_MAX_X_54  3500
#define ZOOM_MAX_X_53  3900
#define ZOOM_MAX_X_52  4320
#define ZOOM_MAX_X_51  4700
#define ZOOM_MAX_X_50  5000
#define ZOOM_MAX_X_49  5300
#define ZOOM_MAX_X_48  5700
#define ZOOM_MAX_X_47  6200
#define ZOOM_MAX_X_46  6800
#define ZOOM_MAX_X_45  7500
#define ZOOM_MAX_X_44  8000
#define ZOOM_MAX_X_43  8500
#define ZOOM_MAX_X_42  9200
#define ZOOM_MAX_X_41  10200
#define ZOOM_MAX_X_40  10500
#define ZOOM_MAX_X_39  10630
#define ZOOM_MAX_X_38  10770
#define ZOOM_MAX_X_37  10780
#define ZOOM_MAX_X_36  10930
#define ZOOM_MAX_X_35  11080
#define ZOOM_MAX_X_34  11240
#define ZOOM_MAX_X_33  11410 
#define ZOOM_MAX_X_32  11580
#define ZOOM_MAX_X_31  11750
#define ZOOM_MAX_X_30  11920
#define ZOOM_MAX_X_29  12070
#define ZOOM_MAX_X_28  12220
#define ZOOM_MAX_X_27  12370
#define ZOOM_MAX_X_26  12530
#define ZOOM_MAX_X_25  12690
#define ZOOM_MAX_X_24  13180
#define ZOOM_MAX_X_23  13360
#define ZOOM_MAX_X_22  13550
#define ZOOM_MAX_X_21  14130
#define ZOOM_MAX_X_20  14340
#define ZOOM_MAX_X_19  14640
#define ZOOM_MAX_X_18  14770
#define ZOOM_MAX_X_17  15030
#define ZOOM_MAX_X_16  15160
#define ZOOM_MAX_X_15  15290
#define ZOOM_MAX_X_14  15420
#define ZOOM_MAX_X_13  15550
#define ZOOM_MAX_X_12  15680
#define ZOOM_MAX_X_11  15810
#define ZOOM_MAX_X_10  15940
#define ZOOM_MAX_X_09  16070
#define ZOOM_MAX_X_08  18050
#define ZOOM_MAX_X_07  19970
#define ZOOM_MAX_X_06  21920
#define ZOOM_MAX_X_05  23870
#define ZOOM_MAX_X_04  25829
#define ZOOM_MAX_X_03  27770
#define ZOOM_MAX_X_02  29720
#define ZOOM_MAX_X_01  32000


#define ZOOM_MAX_Y_99  30//valor digital Maximo convertido que se desppliega en pantalla
#define ZOOM_MAX_Y_98  80
#define ZOOM_MAX_Y_97  94
#define ZOOM_MAX_Y_96  100
#define ZOOM_MAX_Y_95  120
#define ZOOM_MAX_Y_94  150
#define ZOOM_MAX_Y_93  210 
#define ZOOM_MAX_Y_92  290
#define ZOOM_MAX_Y_91  370
#define ZOOM_MAX_Y_90  410//---
#define ZOOM_MAX_Y_89  420
#define ZOOM_MAX_Y_88  430
#define ZOOM_MAX_Y_87  440
#define ZOOM_MAX_Y_86  450
#define ZOOM_MAX_Y_85  460
#define ZOOM_MAX_Y_84  470
#define ZOOM_MAX_Y_83  480
#define ZOOM_MAX_Y_82  490
#define ZOOM_MAX_Y_81  510
#define ZOOM_MAX_Y_80  512 
#define ZOOM_MAX_Y_79  532
#define ZOOM_MAX_Y_78  557
#define ZOOM_MAX_Y_77  587
#define ZOOM_MAX_Y_76  622
#define ZOOM_MAX_Y_75  662
#define ZOOM_MAX_Y_74  702
#define ZOOM_MAX_Y_73  710
#define ZOOM_MAX_Y_72  747
#define ZOOM_MAX_Y_71  769
#define ZOOM_MAX_Y_70  795
#define ZOOM_MAX_Y_69  855
#define ZOOM_MAX_Y_68  920
#define ZOOM_MAX_Y_67  990
#define ZOOM_MAX_Y_66  1020
#define ZOOM_MAX_Y_65  1160
#define ZOOM_MAX_Y_64  1250
#define ZOOM_MAX_Y_63  1350
#define ZOOM_MAX_Y_62  1550
#define ZOOM_MAX_Y_61  1850
#define ZOOM_MAX_Y_60  2000
#define ZOOM_MAX_Y_59  2100
#define ZOOM_MAX_Y_58  2250
#define ZOOM_MAX_Y_57  2450
#define ZOOM_MAX_Y_56  2750
#define ZOOM_MAX_Y_55  3100
#define ZOOM_MAX_Y_54  3500
#define ZOOM_MAX_Y_53  3900
#define ZOOM_MAX_Y_52  4320
#define ZOOM_MAX_Y_51  4700
#define ZOOM_MAX_Y_50  5000
#define ZOOM_MAX_Y_49  5300
#define ZOOM_MAX_Y_48  5700
#define ZOOM_MAX_Y_47  6200
#define ZOOM_MAX_Y_46  6800
#define ZOOM_MAX_Y_45  7500
#define ZOOM_MAX_Y_44  8000
#define ZOOM_MAX_Y_43  8500
#define ZOOM_MAX_Y_42  9200
#define ZOOM_MAX_Y_41  10200
#define ZOOM_MAX_Y_40  10500
#define ZOOM_MAX_Y_39  10630
#define ZOOM_MAX_Y_38  10770
#define ZOOM_MAX_Y_37  10780
#define ZOOM_MAX_Y_36  10930
#define ZOOM_MAX_Y_35  11080
#define ZOOM_MAX_Y_34  11240
#define ZOOM_MAX_Y_33  11410 
#define ZOOM_MAX_Y_32  11580
#define ZOOM_MAX_Y_31  11750
#define ZOOM_MAX_Y_30  11920
#define ZOOM_MAX_Y_29  12070
#define ZOOM_MAX_Y_28  12220
#define ZOOM_MAX_Y_27  12370
#define ZOOM_MAX_Y_26  12530
#define ZOOM_MAX_Y_25  12690
#define ZOOM_MAX_Y_24  13180
#define ZOOM_MAX_Y_23  13360
#define ZOOM_MAX_Y_22  13550
#define ZOOM_MAX_Y_21  14130
#define ZOOM_MAX_Y_20  14340
#define ZOOM_MAX_Y_19  14640
#define ZOOM_MAX_Y_18  14770
#define ZOOM_MAX_Y_17  15030
#define ZOOM_MAX_Y_16  15160
#define ZOOM_MAX_Y_15  15290
#define ZOOM_MAX_Y_14  15420
#define ZOOM_MAX_Y_13  15550
#define ZOOM_MAX_Y_12  15680
#define ZOOM_MAX_Y_11  15810
#define ZOOM_MAX_Y_10  15940
#define ZOOM_MAX_Y_09  16070
#define ZOOM_MAX_Y_08  18050
#define ZOOM_MAX_Y_07  19970
#define ZOOM_MAX_Y_06  21920
#define ZOOM_MAX_Y_05  23870
#define ZOOM_MAX_Y_04  25829
#define ZOOM_MAX_Y_03  27770
#define ZOOM_MAX_Y_02  29720
#define ZOOM_MAX_Y_01  32000














//*******************************************************************++++++++++++++
//+++KONSTANTES++PARA--LA++CONVERSION++A++NUMEROS++PARA++PANTALLA++DDS
/*  tenemos,queremos     x0,x   x0,x   x0,x
 * *            //X: (_MAX,LIM_X)(0,CENTRO_X)(-_MAX,0)  
 *  tenemos,queremos    y0,y    y0,y  y0,y 
               //Y: (_MAX,0),(0,CENTRO_Y)(-_MAX,LIM_Y)
 
                                  //X: (_MAX,LIM_X)(0,CENTRO_X)  (-_MAX,0)
                                  //Y: (_MAX,0)    (0,CENTRO_Y)  (-_MAX,LIM_Y)
 */
//#define ZOOM99_MAX  32000 //(32000,190)(0,95)(-32000,0)
/*#define ZOOM99_N4         
#define ZOOM99_D4
#define ZOOM99_N3
#define ZOOM99_D3
#define ZOOM99_N2
#define ZOOM99_D2
#define ZOOM99_N1
#define ZOOM99_D1
#define ZOOM99_C     */
//typedef signed short xx;

struct ZoomControl{//para crear  el zoom y las ecuaciones  polinomicas de acondicionamiento de señal para el DDS
	   signed short int Max;//valor maximo del cuadro central de la pantalla
	   //signed short int Maxn;//valor negativo
	   //float a; //coeficiente del termino grado2
	   //float b;//coeficiente del termino grado 1
	   //float c; //coeficiente del termino grado 0
	   unsigned short int zFactor;// son los mas menos 32768 entre los 100 zoom's
	   float indiceConversion;//lo que se multiplica para escalar al zoom seleccionado
	   unsigned char Cx; //coordenada x del centro de la pantalla
	   unsigned char Cy;//coordenadas y del centro de la pantalla
	   unsigned char Zoom;//es el zoom se seleccionado actual
	   unsigned short int pixelSizeX;//limite del numero digital despleglable en DDS en funcion del Zoom
	   unsigned short int Maxx;//limite del numero digital desplegable en DDS en funciona del Zoom
	   unsigned short int pixelSizeY;//limite del numero digital despleglable en DDS en funcion del Zoom
	   unsigned short int Maxy;//limite del numero digital desplegable en DDS en funciona del Zoom
	   signed short int lapso;//es el Maximo - Minimo	
	   unsigned short int Ymax;//maximo valor a graficar el zoom en Y, linea de sensibilidad para este zoom
	   unsigned short int Ymin;//minimo valor en la posicion-1 de linea de sesibilidad 
};

//variables usadas en el mismo contexto
struct _GLOBAL_CONTEXTO{
	  unsigned char igxc0;//variable global por contexto sin pasar a otro contexto
	  unsigned char igxc1;//variable global por contexto sin pasar a otro contexto
	  unsigned char igxc2;//variable global por contexto sin pasar a otro contexto
	  unsigned char igxc3;//variable global por contexto sin pasar a otro contexto
	  unsigned char igxc4;//variable global por contexto sin pasar a otro contexto
	  unsigned char igxc5;//variable global por contexto sin pasar a otro contexto
	  unsigned char igxc6;//variable global por contexto sin pasar a otro contexto
	  signed short int ssigxc;
	  double dgxc0;//doble global x contexto
	 // signed short int ssigxc7;//signed short int globlal por contexto
};

struct _GLOBAL_PASO_OTRO_CONTEXTO{
	  unsigned char igpoc0;//i global paso a otro contexto
	  unsigned char igpoc1;
};

typedef struct{
	int16 offsetX;//valor del offset calcualdo actual
	int16 offsetY;
	
}Detection_s;


struct _Detection{
	unsigned short int thresholdX;//deteccion umbral en  ordenas
	unsigned short int thresholdY;//deteccion umbral en absisas
	unsigned char tipo;//tipo de deteccion normal|amplia  normal=solo las absisas amplia=eje x y y; a 0°
    unsigned short int countRechazos;//cuenta rechazos 
    unsigned short flag_TX_box;//bandera de transmision de datos al VFD
    unsigned short int LIM1;//LIMite del cuadro 1 de la barra de deteccion
    unsigned short int LIM2;//LIMite del cuadro 2 de la barra de deteccion
    unsigned short int LIM3;//LIMite del cuadro 3 de la barra de deteccion
    unsigned short int LIM4;//LIMite del cuadro 4 de la barra de deteccion
    unsigned short int LIM5;//LIMite del cuadro 5 de la barra de deteccion
    unsigned short int LIM6;//LIMite del cuadro 6 de la barra de deteccion
    unsigned short int LIM7;//LIMite del cuadro 7 de la barra de deteccion
    unsigned short int LIM8;//LIMite del cuadro 8 de la barra de deteccion
    unsigned short int LIM9;//LIMite del cuadro 9 de la barra de deteccion
    unsigned short int LIM10;//LIMite del cuadro 10 de la barra de deteccion
    unsigned short int LIM11;//LIMite del cuadro 11 de la barra de deteccion
    unsigned short int LIM12;//LIMite del cuadro 12 de la barra de deteccion
    unsigned short int LIM13;//LIMite del cuadro 13 de la barra de deteccion
    unsigned short int LIM14;//LIMite del cuadro 14 de la barra de deteccion
    unsigned short int LIM15;//LIMite del cuadro 15 de la barra de deteccion
    unsigned short int LIM16;//LIMite del cuadro 16 de la barra de deteccion
    unsigned short int LIM17;//LIMite del cuadro 17 de la barra de deteccion
    unsigned short int Sensibilidad; // variable de sensibilidad, Sensibilidad2 es la misma variable
    unsigned short int Altura; //variable de altura de la deteccion
    unsigned char BarraDeteccionStatus;//se esta dibujando una caja? indica si esta ocupada dibujando, no podemos cambiarnos de menu mientras este mandando datos al DDS 
    unsigned char CuadroMadreReady;//el cuadro madre esta construido y listo?.
    unsigned char boxinit;//init box, init dibujo de boxes
    unsigned char signalDDSx; //señal para dds ya acondicionada
    unsigned char signalDDSy;//señal para dds ya acondicionada
    unsigned char EnCurso;//Hay una deteccion en curso
    unsigned char Rechazo_mseg;//6000mseg tiempo en se rechaza despues de detectar en milisegundos
//    unsigned char Rechazo_Seg;//tiempo en que se rechaza despues de detectar en segundos
    unsigned char duracion_mseg;//6000ms  tiempo en que se rechaza despues de detectar en milisegundos
//    unsigned char duracion_Seg;//tiempo en que se rechaza despues de detectar en segundos
    unsigned char Deteccion;//hay deteccion actual en proceso
}; 




struct _Mesurements_{
	unsigned short int BalanceVoltage;//voltage del balance.
	unsigned char flagMesurements;//Bandera para iniciar las lecturas del ADC 
	
};


//structuras
struct NodoLista{//lista de lostos digitales de deteccion y procesar, para crear una lista
    struct NodoLista *next ;
    struct NodoLista *prev;
    signed short int x;
    signed short int y;
    unsigned short int ux;
    unsigned short int uy;
};




struct NodoCV{//Nodo para lista doblemente ligada de Config Vibration
	struct NodoCV *next;
	struct NodoCV *prev;
	unsigned char datCV;
};

struct overLoad{
	unsigned char id;
	unsigned short int x;
	union{
	  //int i;
	  unsigned char c;
	  unsigned short int y;
	  //float f;
	}u;
};

struct overLoad2{
	unsigned char id;
	unsigned short int x;
	unsigned char c;
	unsigned short int y;
};

/*-------NodoCharOp--------------------------------------------------------------------
 *  para despliegue de chars cuando esta el mismo menu con la barra de deteccion online
    data.id     funcion
       INICIO_CMD     pushFIFOcOP_inicioCMD();
       VFD_POSICION   pushFIFOcOP_VFDposicion
       SEND_CHAR      pushFIFOcOP_VFDserial_SendChar
       FIN_CMD        pushFIFOcOP_finCMD();
       SEND_BLOCK     pushFIFOcOP_VFDserial_SendBlock
       
 */
struct NodoCharOp{//nodo char operativo 
	struct NodoCharOp *next;
	struct NodoCharOp *prev;
	//unsigned char datoc;
	//unsigned  short int datosint;
	struct overLoad data; 
};

//struct _DDS_{
//  unsigned char flag_TX_Pixel;	
//  union _Flag_{
//	   unsigned char BanderasByte;
//	   struct{
//		   unsigned char xx:1; //libre
//		   unsigned char Apagando:1;//le dice que nos vamos a salir del menu DDS, para que no mande datos del DDS en otro menu
//	       unsigned char EventDDS:1;//nos dice que tenemos autorizacion para ejecutar graficacion en DDS
//		   unsigned char yyy:1;
//		   unsigned char xxxx:1;//esta activado el Borrado
//		   unsigned char DDS1_BORRAR:1;//Borrando de el DDS
// 		   unsigned char DDS1_TIMER:1;
//		   unsigned char x:1;//para decirle al portal inicio que sali
//		   }bit; 
//       }Banderas;
//};
//














struct _Signal_{

  
   
   
      signed short int RawX;//Datos crudos del ADC
      signed short int RawY;
      signed short int CentY;
      signed short int CentX;
      signed short int Rotx;
      signed short int Roty;//config de vibracion
      
      unsigned char signoX,signoY;
};



struct _OFFSET_{
	signed short int X;//ofset del canal X
	signed short int Y;//offset del canal Y
//	unsigned char PrimeraVezEncendido;//bandera para ver si nunca se ha calculado el offset
    unsigned char Status;//bit MSB=PRIMERA VEZ ENCENDIDO=1 como esta el offset, bandera contadora para sacar el offset
    unsigned short int Counter;//contador de evento para activar el calculo del offset
    unsigned char ConfigFifoCounter;//para ver si ya se ha llenado la fifo 
};

//declaracion de las funciones
//void SignalProcessor_IRQ(void); // data processor from adc's data 
//void init_DSP(void); crearNodoLista initNodoLista
//void malloc(struct NodoLista *N);
void pushFIFO(signed short int x, signed short int y);
void FiltroDigital_passWideBand(signed short int *x,signed short int *y);
unsigned char popFIFOdata(signed short int *xx,signed short int *yy,unsigned char mode);
	void pushFIFO_DDS(unsigned short int x,unsigned short int y);
void Zoom_init(void);
signed short int getMaximun(unsigned char zoom);
void ProcesamientoDigitalPolinomico(signed short int *x);
void BarraDeteccion(signed short int *datax,signed short int *datay);
void AnguloVibracionProcesadorCentral(signed short int datax,signed short int datay);	
void normalizeAngle(double *angulo);
void qsort1(double *ang,double *angMaxInd);
void bubbleSort(double *ang,double *angMaxInd);
unsigned char countingMayoria(double *a,double *aM);//*angulo array  *anguloMaximo
void mallocCV(struct NodoCV *N);
void pushFIFOdisplayCV(unsigned char dato);
unsigned char popFIFOdisplayCV(void);
void newbit(unsigned char *pos,unsigned long int *n);
void shiftright(unsigned char *pos,unsigned long int *n);
float tangente(unsigned short int a);
float cosinous(unsigned char a);
float sinous(unsigned char a);
double getAngleVibracion(unsigned char *fangStack,double *ang,double *angLast);
void girarSignals(double angle);
unsigned long int SquareRoot(unsigned long int x);
double sind(double x);
double cosd(double x);
unsigned char getFraccionAngVibr(double *ang,double *angMaxInd,unsigned char av);
void convert2int(double *ang,double *angMaxInd,double*ang2);
double reBuildAV(unsigned char Entero, unsigned char Frac);
void mallocCharOp(struct NodoCharOp *N);
char  pushFIFOcOPc(struct overLoad d);
struct overLoad popFIFOcOP(void);
void pushFIFOcOP_inicioCMD(void);
void pushFIFOcOP_VFDposicion(unsigned short int x, unsigned short int y);
void pushFIFOcOP_finCMD(void);
unsigned char pushFIFOcOP_VFDserial_SendBlock(unsigned char *Ptr,unsigned short Size,unsigned short *Snd);
unsigned char pushFIFOcOP_VFDserial_SendChar(unsigned char c);
void Signal_Processor_Controller(void);
//unsigned char getCoord_pixel(unsigned char insignea,signed short int xy,unsigned char min,unsigned char max);
unsigned char convert_coord_to_DDS_pixel(unsigned char insignea,signed short int n);
void CentrarDatosZoom_converter(unsigned char *udataDDSx,unsigned char *udataDDSy);
unsigned char convert_val2DDS_pixel(unsigned char insignea,signed short int n);
unsigned char isDDS_pixel_outRange(signed short int signal,signed short int min,signed short int  max);
//unsigned char getCoord_pixel(unsigned char insignea,signed short int xy,signed short int min,signed short int  max);
unsigned char getDDSpixelX(signed short int n);
unsigned char getDDSpixelY(signed short int n);
void Escalar_al_ZoomSelected(signed short int x,signed short int y,unsigned char *ux,unsigned char *uy);
void generarPixelsAnalogos(void);
void detectionAnalisis(signed short int x,signed short int y);
void init_Sensibilidad(void);
void Monitor_Maestro_de_Voltajes(void);
//void voltage_monitor_catcher(signed short int x,signed short int y);	
//unsigned short int ADC_getValue16(struct _GLOBAL_CONTEXTO *g);
//void Centrar_Signal_y_OffSet(signed short int datainX,signed short int datainY,signed short int *Xout,signed short int *Yout);
inline void setOFFSET(signed short int xin,signed short int yin,signed short int *xout,signed short int *yout,signed short int offsetx,signed short int offsety);
unsigned char CounterOffsetController(unsigned char var);
unsigned char evaluar_Offset(signed short int inX,signed short int inY,signed short int offsetX,signed short int offsetY);
void getOFFset(signed short int xin,signed short int  yin,signed short int *offsetX,signed short int *offsetY);
unsigned short int ADC_getValue16(signed short int *ssi,unsigned char *signo);
void Control_de_OffSet_Normal(signed short int datainX,signed short int datainY,signed short int *Xout,signed short int *Yout,struct _OFFSET_ *o);
//void Control_Offset_init(signed short int datainX,signed short int datainY,signed short int *Xout,signed short int *Yout);
void ControlMaestrodeVibracionXgiro(signed short int xin,signed short int yin,signed short int *xout,signed short int *yout);
unsigned char getCoord_pixel(unsigned char insignea,signed short int xy);
void Procesador_Central_de_Cetrado_de_Datos(signed short int datainX,signed short int datainY,signed short int *Xout,signed short int *Yout);
unsigned char evaluar_ssint(signed short int dato, unsigned short int max);
unsigned char zoomXY(unsigned char eje,signed short int xy,float div);
void Generador_de_Señal_Debug_Tool(signed short int *x11,signed short int  *y11);
unsigned char is_drawable_Sensibility_DDS(void);
unsigned char is_Phase_0(void);
unsigned char Validar_detectionAnalisis(signed short int  y);
void Rechazo_Controller(unsigned char deteccion);
void Contador_Rechazos_Controller(unsigned short int cR);
void vTask12_Signal_Processor_Controller(void);



#endif /* DSP_H_ */

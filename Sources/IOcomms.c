/*
 * IOcomms.c
 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */
/*
 * IOcomms.c
 *
 *  Created on: Dec 5, 2016
 *      Author: chals
 *      
 *      
 *      PARIDAD DE COMUNICACION RS232 DE LA PROCESADORA A LA IO 
 *       ES DE  "EVEN" POR HARDWARE
 *       
 *       UART0 es para comunicarse con la tarjeta IO
 *       
 */



#include "IOcomms.h"
#include "PE_Types.h"
#include "IOUPserial.h"
#include "errorController.h"
#include "VFDmenu.h"
#include "I2C_.h"
#include "system.h"
#include "CTS_UART0.h"
#include "RTS_UART0.h"
#include "TI2.h"
#include "maths.h"
#include "queue.h"
#include "string.h"
#include "VFDserial.h"
#include "delay.h"
#include "DSP.h"

//#include "millis.h"


//definiciones
//#define  SIZEIOBUFFRX   10
//#define  SIZEIOBUFFTX   10  //NO HACER MAYOR A 250 TAMAÃ‘O HABRIA PROBLEMAS EN CODIGO uchar using compare del buffer de transmision


//variables globales internas
unsigned char *prx,*lastrx;
unsigned char RXdata[2];
extern unsigned  char buffer1[SIZE_BUFFER_TX];//del la transmision FIFO serial IOUP

//VARIABLES LIBRERIA PROCESADORA-IO
// version  1.0.3
//unsigned char isGotisEnable;//Software de recepcion de datos en la DeskTop
//unsigned char Requerimiento_en_Proceso;
//unsigned char EnableComunicationsTX;
unsigned char rxEnableFlag_IRQ;//hablita interrupcion de recepcion
//struct COMMUNICACION IO_TX;
extern unsigned char *popTX;//head y last de los array A y B
extern struct _Comando_DDS dds;
extern unsigned short int sizeof1(unsigned char *p,unsigned short int size);
extern struct _Error323_ e;//error controller data struct
extern struct _COMMs_ com;
extern struct _Signal_ Signal;

//FIN DE VARIABLES PROCESADORA -IO

typedef union{
	signed short int sigshortint;
	unsigned char byte[2];
}SigShortInt;


//estructuras
//struct TX_BUFFER txbuff[SIZEIOBUFFTX]; //arreglo de estructuras para hacer la lista doblemente ligada 
//struct Tx_Control txCon; //control de transmision
//struct Rx_Control rxCon;//control de recepcion
//struct RX_BUFFER rxbuff[SIZEIOBUFFRX];
struct COMMs_IOUP *iuComms;
struct COMMs_IOUP FIFO_IUComms[SIZE_FIFO_IOUP];
//struct SALIDAS_UP  IOouts;
struct ENTRADAS_UP IOins;
extern struct _SERIAL_ serial;//extern struct _Vector_ serial;
struct _Error323_ sysMon;
static void vTask1_Transmision_al_CPU_TFT(void);
static void vTask3_ImAlive_Transm_al_CPU_TFT(void);
static void vTask2_ADC_to_FIFO_tx_CPU_TFT(void);
static void vTask10_Comando_Deteccion_Prod(void);
static void vTask16_ErrorBAlance_Transm_al_CPU_TFT(void);
static void vTask17_Error_Offset_Unset_Transm_al_CPU_TFT(void);
static void vTask18_Error_Driver_OFF_Transm_al_CPU_TFT(void);

void init_IOcomms(void){
//unsigned char i;
	      IOUPserial_TurnRxOff();//EnableComunicationsTX=FALSE;
		  rxEnableFlag_IRQ=FALSE;
		  serial.IO.Salidas.bitA.Gotis=FALSE;
		  //rxCon.paquete=RX_FREE;
		  //RTS_UART0_PutVal(TRUE);
		  RTS_UART0_ClrVal();//NO ready to send
		  //RTS_UART0_PutVal(FALSE);//al iniciarse debe estar apagado el RTS
		  init_FIFO_comms_IOUP(); //init fifo de communication con la board uproc
		  SemaforoTX(TRUE,'w');
		  //IOouts.state=BYTE_COMMAND;
		  //IOouts.ANS_DATOS=NONE;
//		  IO_TX.Flags.ByteFlags=0;//limpiamos bandera para transmitir
//		  IO_TX.Libres=SIZE_IO_TX_BUFFER;//nodos libres en la fifo
//		  IO_TX.Flags.bitFlag.freeTXx2=TRUE; //esta libre la transmision, HASTA QUE HAYAA UN PAQUETE
//          serial.appendDDS=push_FIFO_CMD_DDS;
/*          serial.popDDS=pop_FIFO_CMD_DDS;*/
          serial.tx.appendByte=FIFO_general_1byte_push;// push_FIFO_TRANSMISION_serial_IO;//agregar byte al buffer  de tx
          serial.tx.vaciarbuff=vaciarBuffer;
          serial.vars.Flags.bitF.FreeBuffTX=TRUE;
          dds.remove=pop_FIFO_CMD_DDS;//apuntando a la funcion
          dds.append=push_FIFO_CMD_DDS;//apuntando a la funcion
          //serial.tx.resetFIFO=reset_FIFO_ser
}//fin init IO communications



/*Driver principal de transmision al display * por serial* */
static void vTask1_Transmision_al_CPU_TFT(void){
static unsigned char estado;
static uint8_t dato;
unsigned char *x;
	x=&dato;
    switch(estado){
    	case 1:if(com.TxDisp.ncount>0){estado++;}break;
    	case 2:if(com.TxDisp.dequeue(x,&com.TxDisp)){estado++;}break;
    	case 3:if(VFDserial_SendChar(*x)==0){estado++;}break;
    	//case 4:if(VFDserial_GetTxComplete())estado++;break;//buffer tx vacio?
       	default:estado=1;break;}//fin de estado
}//fin de transmision del display tft-----------------------------------



//STX,CMD,LEN,ETX Manda comando, que dice que estamos conectados
static void vTask3_ImAlive_Transm_al_CPU_TFT(void){
static unsigned char estado72;
const unsigned char cmd=CMD_IM_ALIVE;
const unsigned char len=0xFF,SIZE_TRAMA=4;
//const unsigned long int TIME_MONITOR=160000;
static unsigned char  datos[SIZE_TRAMA],i;
//static unsigned int count;
static dlong delay,pasado;
dlong ahora; 
const word TIME_SEND=2500;//miliseconds, tiempo, lapso para transmitir comando ImAlive
		
    switch(estado72){
    		case 1:delay=0;estado72++;break;
	    	case 2:ahora=millis();
	    		   delay+=ahora-pasado;
	    	       if(delay>TIME_SEND){pasado=0;
	    	    	         estado72++;}
	    	       pasado=ahora;break;
	    	case 3:if(solicitarRecurso(ID_vTask_3))estado72++;break;
	    	case 4:datos[0]=STX;datos[1]=cmd;datos[2]=len;estado72++;break;
	    	case 5:datos[3]=ETX;i=0;estado72++;break;
	    	case 6:if(com.TxDisp.append(datos[i],&com.TxDisp))estado72++;break;
	    	case 7:if(++i<SIZE_TRAMA)estado72--;
	    	       else{estado72++;}break;
			case 8:Monitor_System_Diagnostico_LEDs(CMD_IM_ALIVE);estado72++;break;
			case 9:if(liberarRecurso(ID_vTask_3))estado72++;break;
			default:estado72=1;break;}//fin switch------------
}//fin de transmision del display tft------------------------------------


static void vTask16_ErrorBAlance_Transm_al_CPU_TFT(void){
static unsigned char estado16;
const unsigned char cmd=CMD_ERR_BAL;
const unsigned char len=0xFF,SIZE_TRAMA=4;
static unsigned char  datos[SIZE_TRAMA],i;
static dlong delay,pasado;
dlong ahora; 
const word TIME_SEND=6500;//miliseconds, tiempo, lapso para transmitir comando ImAlive
		
  
	  switch(estado16){
		case 1:if(sys.u.bits.error_Bal){
			        delay=0;estado16++;}
		       break;
		case 2:ahora=millis();
			   delay+=ahora-pasado;
			   if(delay>TIME_SEND){pasado=0;
						 estado16++;}
			   pasado=ahora;break;
		case 3:if(solicitarRecurso(ID_vTask_16))estado16++;break;
		case 4:datos[0]=STX;datos[1]=cmd;datos[2]=len;estado16++;break;
		case 5:datos[3]=ETX;i=0;estado16++;break;
		case 6:if(com.TxDisp.append(datos[i],&com.TxDisp))estado16++;break;
		case 7:if(++i<SIZE_TRAMA)estado16--;
			   else{estado16++;}break;
		case 8://Monitor_System_Diagnostico_LEDs(CMD_IM_ALIVE);
			   estado16++;break;
		case 9:if(liberarRecurso(ID_vTask_16))estado16++;break;
		default:estado16=1;break;}//fin switch------------
}//fin vTask16_ErrorBAlance_Transm_al_CPU_TFT---------------


static void vTask17_Error_Offset_Unset_Transm_al_CPU_TFT(void){
static unsigned char estado17;
const unsigned char cmd=CMD_ERR_OFFS_UNSET;
const unsigned char len=0xFF,SIZE_TRAMA=4;
static unsigned char  datos[SIZE_TRAMA],i;
static dlong delay,pasado;
dlong ahora; 
const word TIME_SEND=6500;//miliseconds, tiempo, lapso para transmitir comando ImAlive
		
  
	  switch(estado17){
		case 1:if(sys.u.bits.error_offset_unset){
			        delay=0;estado17++;}
		       break;
		case 2:ahora=millis();
			   delay+=ahora-pasado;
			   if(delay>TIME_SEND){pasado=0;
						 estado17++;}
			   pasado=ahora;break;
		case 3:if(solicitarRecurso(ID_vTask_17))estado17++;break;
		case 4:datos[0]=STX;datos[1]=cmd;datos[2]=len;estado17++;break;
		case 5:datos[3]=ETX;i=0;estado17++;break;
		case 6:if(com.TxDisp.append(datos[i],&com.TxDisp))estado17++;break;
		case 7:if(++i<SIZE_TRAMA)estado17--;
			   else{estado17++;}break;
		case 8://Monitor_System_Diagnostico_LEDs(CMD_IM_ALIVE);
		       estado17++;break;
		case 9:if(liberarRecurso(ID_vTask_17))estado17++;break;
		default:estado17=1;break;}//fin switch------------
}//fin vTask17_Error_Offset_Unset_Transm_al_CPU_TFT-------


static void vTask18_Error_Driver_OFF_Transm_al_CPU_TFT(void){
static unsigned char estado18;
const unsigned char cmd=CMD_ERR_DRIVER_OFF;
const unsigned char len=0xFF,SIZE_TRAMA=4;
static unsigned char  datos[SIZE_TRAMA],i;
static dlong delay,pasado;
dlong ahora; 
const word TIME_SEND=6500;//miliseconds, tiempo, lapso para transmitir comando ImAlive
		
  
	  switch(estado18){
		case 1:if(sys.u.bits.error_Driver_Signal){
			        delay=0;estado18++;}
		       break;
		case 2:ahora=millis();
			   delay+=ahora-pasado;
			   if(delay>TIME_SEND){pasado=0;
						 estado18++;}
			   pasado=ahora;break;
		case 3:if(solicitarRecurso(ID_vTask_18))estado18++;break;
		case 4:datos[0]=STX;datos[1]=cmd;datos[2]=len;estado18++;break;
		case 5:datos[3]=ETX;i=0;estado18++;break;
		case 6:if(com.TxDisp.append(datos[i],&com.TxDisp))estado18++;break;
		case 7:if(++i<SIZE_TRAMA)estado18--;
			   else{estado18++;}break;
		case 8://Monitor_System_Diagnostico_LEDs(CMD_IM_ALIVE);
		       estado18++;break;
		case 9:if(liberarRecurso(ID_vTask_18))estado18++;break;
		default:estado18=1;break;}//fin switch------------
}//fin vTask18_Error_Driver_OFF_Transm_al_CPU_TFT-------



        // STX,CMD,LEN,XH,XL,YH,YL,CRC,ETX
static unsigned char calculaCRC(const unsigned char *data, unsigned char len) {
enum {CRC_POLY=0x07};
unsigned char crc = 0x00;                                       // valor inicial del CRC
unsigned char i, j;                                             // contadores de bytes y bits

for (i = 0; i < len; i++) { crc ^= data[i];                    // XOR con el byte actual
        for (j = 0; j < 8; j++) {                               // procesar cada bit
            if (crc & 0x80) { crc = (crc << 1) ^ CRC_POLY; }    // bit MSB=1, desplazar y XOR
            else { crc <<= 1; }}}                                 // bit MSB=0, solo desplaza}
return crc;                                                     // devolver CRC calculado
} // fin calculaCRC8 ------------------------------------------ posicion 66



/* los datos crudos del ADC se meten ala fifo TX SERIAL
 * ya con el protocolo 
 * STX,CMD,LEN,dato1H,dato1L,dato2H,datos2L,CRC,ETX * */
static void vTask2_ADC_to_FIFO_tx_CPU_TFT(void){
static unsigned char estado;
static unsigned short int x0,y0,x,y;
const unsigned char cmd=CMD_DDS;
enum{ SIZE_DATA=9 };
const unsigned char len=5,SIZE_TRAMA=9;
static unsigned char datos[SIZE_TRAMA],i;
const signed short int dataX[SIZE_DATA]={1050,35,789,110,220,1,0,12,22};
const signed short int dataY[SIZE_DATA]={100,568,323,12,1112,33,11,44,149};
static uint8_t j;

l21:switch(estado){
    	case 1://x=(unsigned short int)Signal.RawX;//Deteccion.RawX;
    		   //y=(unsigned short int)Signal.RawY;break;//Deteccion.RawY;
    		   Signal.RawX=dataX[j];
    		   Signal.RawY=dataY[j];
    		   if(++j>(SIZE_DATA-1)){j=0;}
    		   x=(unsigned short int)Signal.RawX;//Deteccion.RawX;
			   y=(unsigned short int)Signal.RawY;
			   estado++;break;
    	case 2:if(x!=x0)estado=4;else{estado++;}break;
    	case 3:if(y!=y0)estado=4;else{estado=1;}break;
    	case 4:x0=x;y0=y;estado++;break;
    	case 5:if(solicitarRecurso(ID_vTask_2))estado++;break;
    	case 6:datos[0]=STX;datos[1]=cmd;datos[2]=len;estado++;break;
    	case 7:datos[3]=(unsigned char)(x >> 8);// Enviar BYTE_XH (byte alto de X)
               datos[4]=(unsigned char)(x & 0xFF);
               datos[5]=(unsigned char)(y >> 8);// Enviar BYTE_YH
               datos[6]=(unsigned char)(y & 0xFF);
               datos[7]=calculaCRC(&datos[1],6);
               datos[8]=ETX;estado++;i=0;break;//Enviar BYTE_YL
    	case 8:if(com.TxDisp.append(datos[i],&com.TxDisp))estado++;break;
		case 9:if(++i<SIZE_TRAMA)estado--;else{estado++;}break;
		case 10:liberarRecurso(ID_vTask_2);
		 	 	estado++;break;
		default:estado=1;break;
    }//fin switch-----------------------------------------------
}//fin prueba---------------------------------------------------


/* Comando deteccion de Producto, se envia cuando hay una deteccion 
 * uP---->IO Prod-Det STX,F5,len,AmpH,AmpL,AngH,AngL,FrecqH,FrecqL,G,CRC,ETX  */
static void vTask10_Comando_Deteccion_Prod(void){
static unsigned char estado10;
static unsigned char i,j;
const unsigned char cmd=CMD_DET_PROD;
enum{ SIZE_DATA=9,SIZE_PACKET=11};
const unsigned char len=8;
static unsigned char datos[14];
const unsigned short int dataAmp[SIZE_DATA]={  50, 135,789,1109,2220,331,555,612,722};
const unsigned short int dataAng[SIZE_DATA]={  30, 68, 23,   12,  11, 92,110, 44, 19};
const unsigned short int dataFre[SIZE_DATA]={1000,568,323,  612, 812,333,111,444,149};
const unsigned char      dataGan[SIZE_DATA]={  10, 56, 23,   12,  12, 33, 11, 44, 14};
static unsigned int contador;


    
	switch(estado10){
	    	case 1:i=0;estado10++;break;
	    	case 2:if(i>(SIZE_DATA-1)){i=0;}estado10++;break;
	    	case 3:if(solicitarRecurso(ID_vTask_10))estado10++;break;//0    1  2    3   4     5   6     7      8    9  10  11 
	    	case 4:datos[0]=STX;datos[1]=cmd;datos[2]=len;         //STX,F5,len,AmpH,AmpL,AngH,AngL,FrecqH,FrecqL,G,CRC,ETX
	    	       datos[3]=(unsigned char)(dataAmp[i] >> 8);// Enviar BYTE_XH (byte alto de X)
	               datos[4]=(unsigned char)(dataAmp[i] & 0xFF);
	               datos[5]=(unsigned char)(dataAng[i] >> 8);// Enviar BYTE_YH
	               datos[6]=(unsigned char)(dataAng[i] & 0xFF);
	               datos[7]=(unsigned char)(dataFre[i] >> 8);// Enviar BYTE_YH
				   datos[8]=(unsigned char)(dataFre[i] & 0xFF);
				   datos[9]=(unsigned char)(dataGan[i]);// Enviar BYTE_YH
				   estado10++;break;
	    	case 5:datos[10]=calculaCRC(&datos[1],10);//posicion del crc=10
	               datos[11]=ETX;j=0;estado10++;break;//Enviar BYTE_YL
	    	case 6:if(com.TxDisp.append(datos[j],&com.TxDisp))estado10++;break;
			case 7:if(++j<SIZE_PACKET){estado10--;}else{estado10++;}break;
			case 8:i++;estado10++;break;
			case 9:liberarRecurso(ID_vTask_10);
			 	 	estado10++;break;
			case 10:if(contador++>1000){contador=0;estado10++;}
			default:estado10=1;break;
	    }//fin switch-----------------------------------------------
}//fin vTask10_Comando_Deteccion_Prod-----------------------------






void vTask2_Receptor_Procesador_MCU_TFT_IO(void){
static uint8_t estado,estado2;
static uint8_t c;
static uint8_t buffer_patron[4];

	switch(estado){
		case 1:estado2=1;estado++;break;
		case 2:if(com.RxDisp.dequeue(&c,&com.RxDisp))estado++;break;
		case 3:switch(estado2){
				case 1:if(c==STX)estado2++;else{estado2=1;}break;
				case 2:if(c==CMD_IM_ALIVE_ANS)estado2++;else{estado2=1;}break;
				case 3:if(c==0xFF)estado2++;else{estado2=1;}break;
				case 4:if(c==ETX){Monitor_System_Diagnostico_LEDs(CMD_IM_ALIVE_ANS);}
					   else{estado2=1;}
					   break;
				default:estado2=1;break;}
			   estado=2;break;
		default:estado=1;break;}//fin switch---------------------
//task4---------------------------------------------------	
}//find de receptor de mcd tft io--------------------------------



void enable_Comms_TX(void){
	  IOUPserial_TurnRxOn();   
	  serial.vars.Flags.bitF.EnableComunicationsTX=TRUE;}
void enableIO_reciv(void){rxEnableFlag_IRQ=TRUE;}
void disableIO_reciv(void){rxEnableFlag_IRQ=FALSE;}


/* 
 *  version 300322-930     restructuracion completa*/
void IOUP_BOARD_SERIAL_CONTROLLER(void){

	IOUP_BOARD_TRANSMISSION_CONTROLLER();
	pop_DDSpacket_to_FIFO_serial_TX_IOUP();//METER Comandos de dds al fifo de transmision
    
	
	
	IOUP_RECEPTION_COMUNICATIONS_CONTROLLER();    
}//fin del controlador principal de transmision de datos----------------------------------------





/* Transmite todo el buffer FIFO completo de espera de Serial, 
 *  debe ir antes de cada comado que mete a la fifo serial
 *  version  280322-1713     estructura de datos a serial.rx|tx|vars
 *  version  300322-919      nombre y reorganizacion general
 *  version  300322-924      cambia nombre del method
 *  version  310322-1012     cambia reenvio de ACK en vacio
 *  version  310322-1251     control de errores, led error. comms
 *  version  060422-1514     constante de aaCK, cambio de 14 a 2 seg
 *  version  070422-1042     estructiura de datos de error y control de ans
 *  */ 
void IOUP_BOARD_TRANSMISSION_CONTROLLER(void){
word n;
static unsigned char count; 
static unsigned char i;
const unsigned char TIME_ACK=3;//tiempo en segundos de mandar ACK
const unsigned char TIME_AWAIT=3;//TIMEPO DE espera antes de marcar ERROR, son vueltas,
	  if(serial.tx.ncount>0){
			if(sizeof1(&buffer1[0],SIZE_BUFFER1)==0){
				__asm(nop);__asm(Halt);__asm(nop);}
			if(IOUPserial_GetCharsInTxBuf()==0)
	 			 if(!serial.vars.Flags.bitF.FreeBuffTX)
					  if(count++>20){
						  serial.vars.Flags.bitF.FreeBuffTX=TRUE;
						  count=0;}
			if(serial.vars.Flags.bitF.FreeBuffTX){
				serial.vars.Flags.bitF.FreeBuffTX=FALSE;	
			    IOUPserial_SendBlock(&buffer1[0],serial.tx.ncount,&n);
			    if(serial.vars.var.ncountNopeAns<250)
			    	    serial.vars.var.ncountNopeAns++;//mandamos datos
			    serial.vars.var.timercomms=0;//conteo a 0 de monitor de canal
				serial.tx.resetFIFO(&serial.tx);}}
	  else{if(serial.vars.var.timercomms>TIME_ACK){//manda Acknowkledge cada TIME_ACK segundos
	    	        Transmission_Acknowledge();//mandar: "aqui estamos"
	    	       	serial.vars.var.timercomms=0;
	    	       	if(serial.vars.var.ncountNopeAns>TIME_AWAIT)
	    	       	     e.ERROR.bits.aLed4_Comm_Warnning_Sys=1;// .LEDS|=0x01;//ERROR_RECEPCION_ACK_IO;                
	                 }}    
	  
}//fin  IOUP_BOARD_TRANSMISSION_CONTROLLER------------------------------------




/*  Transmite comando de ggraficacion
 *  stx=03h,CMD=13h,xh,xl,yh,yl,ETX=02h
 *     version-1   */
void pop_DDSpacket_to_FIFO_serial_TX_IOUP(void){
signed short int x,y,seguro=-100;
const unsigned char SIZE_CMD=4; //tamaño del comando 
unsigned char bytes[SIZE_CMD],i;
	   
l1:  if(pop_FIFO_CMD_DDS(&x,&y)){       
		   if((x!=0)&&(y!=0)){      
			  serial.tx.appendByte(STX,&serial.tx);
			  serial.tx.appendByte(CMD_DDS,&serial.tx);
			  getBytes_from_SSInt(&bytes[0],&bytes[1],x);
			  getBytes_from_SSInt(&bytes[2],&bytes[3],y);
			  for(i=0;i<SIZE_CMD;i++)
				 serial.tx.appendByte(bytes[i],&serial.tx);
			  i=getCheckSUM(CMD_IN3UP_GOTIS,&bytes[0],SIZE_CMD);
			  serial.tx.appendByte(i,&serial.tx);
			  serial.tx.appendByte(ETX,&serial.tx);}
		   if(seguro++>0){	   
				__asm(Halt);//debug Error de ingenieria de software
				__asm(nop);}
		   goto l1;}//vaciar la fifo
}//fin comando  DDS meter a  buffer el paquete o los paquetes---------




/*  FIFO MANAGEMENT+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/



void init_FIFO_comms_IOUP(void){
	
	    FIFO_IUComms[0].MaquinaEstados=NODO_FREE;
	    FIFO_IUComms[0].next=&FIFO_IUComms[1];
	    FIFO_IUComms[0].prev=&FIFO_IUComms[SIZE_FIFO_IOUP-1];
	  
	    //FIFO_IUComms[0].status=NODO_FREE;
	    FIFO_IUComms[SIZE_FIFO_IOUP-1].MaquinaEstados=NODO_FREE;
	    FIFO_IUComms[SIZE_FIFO_IOUP-1].next=&FIFO_IUComms[0];
	    FIFO_IUComms[SIZE_FIFO_IOUP-1].prev=&FIFO_IUComms[SIZE_FIFO_IOUP-2];
	    
	    //FIFO_IUComms[SIZE_FIFO-1].status=NODO_FREE;
	    for(iuComms=&FIFO_IUComms[1];iuComms<=&FIFO_IUComms[SIZE_FIFO_IOUP-2];iuComms++){
	    	//iuComms->status=NODO_FREE;
	        iuComms->MaquinaEstados=NODO_FREE;
	        iuComms->next=iuComms+1;
	        iuComms->prev=iuComms-1;
	        iuComms->MaquinaEstados=NODO_FREE;}//fin for
	        iuComms->next=&FIFO_IUComms[0];
	        iuComms->prev=&FIFO_IUComms[SIZE_FIFO_IOUP-2];
	        
	    
}//FIN de initzializatione de la FIFO comunication to processor



/* LIBRERIA  COMUNICACION IO-UP
 *  insertar un nodo en la cola de la FIFO
 * version  1.0.5 */
void insertFIFOcommsIOUp(struct COMMs_IOUP datos){
  static struct COMMs_IOUP *p;//debug quitar el static
  unsigned char j=0;
	p=iuComms;
	if(serial.vars.Flags.bitF.EnableComunicationsTX){	
		if(p->MaquinaEstados==NODO_FREE){
			p->MaquinaEstados=datos.MaquinaEstados;
			//p->ndatos=datos.ndatos;
			//for(i=0;i<datos.ndatos;i++)
		        //p->datos[i]=datos.datos[i];}
			p->xh=datos.xh;
			p->xl=datos.xl;
			p->yh=datos.yh;
			p->yl=datos.yl;}
		else{
l62:        if((p->MaquinaEstados==NODO_FREE)&&(p->prev->MaquinaEstados!=NODO_FREE)){//buscamos el ultimo nodo de la FIFO
				 //p=p->prev;
				 p->MaquinaEstados=datos.MaquinaEstados;
				// p->ndatos=datos.ndatos;
				 //for(i=0;i<datos.ndatos;i++)
                     //p->datos[i]=datos.datos[i];}
				 p->xh=datos.xh;
				 p->xl=datos.xl;
				 p->yh=datos.yh;
				 p->yl=datos.yl;}
			 else {p=p->prev;
				   if(j++<SIZE_FIFO_IOUP+10) //debug
					   goto l62;}  
		}}//fin else
}//insert FIFO comunications uProcessor--------------------------------------------------


struct COMMs_IOUP popFIFOcommsIOUp(void){//POP from the FIFO top
static struct COMMs_IOUP r,*p,*ptail;//debug quitar el static	
       p=iuComms;
       ptail=iuComms->prev;
       if(serial.vars.Flags.bitF.EnableComunicationsTX){
l145:      if(p->MaquinaEstados==NODO_FREE){
	         if(p==ptail){
	        	  iuComms->MaquinaEstados=NODO_FREE;
	        	  r.xh=iuComms->xh;r.xl=iuComms->xl;
	        	  r.yh=iuComms->yh;r.yl=iuComms->yl;
	        	  return r;}
	         else { p=p->next;
    	            goto l145;}}
           else{if(p->MaquinaEstados==READY){
        	         p->MaquinaEstados=NODO_FREE;
        	         r.xh=p->xh;r.xl=p->xl;
        	         r.yh=p->yh;r.yl=p->yl;
        	        return r;}
                }//fin else
       }//fin enable------------------------
}//end fin de la pop del FIFO de transmision, datos del ADC


/*insertOnTopFIFO_IOUPcomms  inserta con prioridad
 *    LIBRERIA PROCESADORAA  IO
 *      version  2.0.0    */
void insertOnTopFIFO_IOUPcomms(struct COMMs_IOUP data){
	 if(iuComms->MaquinaEstados==NODO_FREE){
		 //iuComms->ndatos=data.ndatos;
		 iuComms->MaquinaEstados=READY;//data.MaquinaEstados;
		 //for(i=0;i<iuComms->ndatos;i++)
			// iuComms->datos[i]=data.datos[i];
		 iuComms->xh=data.xh;
		 iuComms->xl=data.xl;
		 iuComms->yh=data.yh;
		 iuComms->yl=data.yl;
	    }//endif 
	 else{
		 if(iuComms->prev->MaquinaEstados==NODO_FREE){
			 //iuComms->ndatos=data.ndatos;
			 iuComms->MaquinaEstados=READY;//data.MaquinaEstados;
			 //for(i=0;i<iuComms->ndatos;i++)
			 	//		 iuComms->datos[i]=data.datos[i];}
			 iuComms->xh=data.xh;
			 iuComms->xl=data.xl;
			 iuComms->yh=data.yh;
			 iuComms->yl=data.yl;}
		 else 
			  eLog(CODE_ERROR_FIFO_FULL);
	 }//endelse
}//end insert on top of FIFO communications



/*  END  FIFO MANAGEMENT+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +++++++++++++++++++++++END FIFO MANAGEMENT+++++++++++++++++++++++++++++++++++++++++++++++++*/


/*  CONTROLADRES MAESTROS DE COMUNICACIONES PROCESADORA-ENTRADAS/SALIDAS++++++++++++
 *   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  CONTROLADRES MAESTROS DE COMUNICACIONES PROCESADORA-ENTRADAS/SALIDAS++++++++++++
 *   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */




/*  INTERRUPT SERVICE ROUTINE 
 *     cada vez que recibe un caracter
 *     GUARDA EL DATO EN  una FIFO            
 *          version  5.0.10         */
void IOrecv_char_ISR(unsigned char c){
//struct COMMs_IOUP RXdata[];
static unsigned char i;	
  if(rxEnableFlag_IRQ){
	           if((c&0x80)==0x80)
	        	     IOins.byteH=c;
	           else  IOins.byteL=c;
        }//end flag
}//fin de recepcion de char de la IO INTERRUPT SERVICE ROUTIN----------------------------------


void MonitorCentralEntradas(void){
static unsigned char byteH0,byteL0;

       if(byteH0!=IOins.byteH){
    	   byteH0=IOins.byteH;
    	   if((byteH0&0x80)==0x80){   //1000 0000
              if((byteH0&0x40)==0x40)
                   IOins.Tacho=TRUE;
              else IOins.Tacho=FALSE;
              if((byteH0&0x20)==0x20)
    		       IOins.Gatin=TRUE;
              else IOins.Gatin=FALSE;
              if((byteH0&0x10)==0x10)
            	  IOins.SysCHK=TRUE;
              else IOins.SysCHK=FALSE;
              if((byteH0&0x08)==0x08)
            	  IOins.PackCHK=TRUE;
              else IOins.PackCHK=FALSE;
              if((byteH0&0x02)==0x02)
            	  IOins.ResetSW=TRUE;
              else IOins.ResetSW=FALSE;
    		  if((byteH0&0x01)==TRUE)
    			  IOins.Aux1IN=TRUE;
    		  else IOins.Aux1IN=FALSE; }}
       if(byteL0!=IOins.byteL){
    	   byteL0=IOins.byteL;
    	   if((byteL0&0x00)==0x00){
    		 if((byteL0&0x40)==0x40)
    			 IOins.gotis=TRUE;
    		 else IOins.gotis=FALSE;
    		 if((byteL0&0x20)==0x20)
    			 IOins.askdat=TRUE;
    		 else IOins.askdat=FALSE;
    		 if((byteL0&0x10)==0x10)
    			 IOins.LockSW=TRUE;
    		 else IOins.LockSW=FALSE;
    		 if((byteL0&0x08)==0x08)
    			 IOins.Aux2IN=TRUE;   //aqui falta poner los 3 bits de requerimientos de datos
    		 else IOins.Aux2IN=FALSE;}}
       
 
	
	
}//fin del monitor central de las entradas centrales de entrada-----------------------------------

//metodo para enviar los datos para graficas el Adc
unsigned char Comm_DDS_FIFO(signed short int datax,signed short int datay){
const signed short int x[]={-11, 22,  35,  -8,  -9, 254, 1123,-3421};
const signed short int y[]={ 22,-93,   4,   3, -34, -5,  234,   213};
static signed short int j,h,v;
static unsigned char i;	// FFF5 0016 0023 FFF8 FFF7 00FE 0463 F2A3
                        // 0016 FFA3 0004 0003 FFDE FFFB 00EA 00D5	
                         //  E2


    if(++h>1000){
    	h=0;
        if(++v>500)
             v=0;}
    datax=h-500; datay=v-200;
    if(serial.vars.Flags.bitF.DDS_ACTIVA){
	    dds.append(datax,datay);//   serial.appendDDS(datax,datay
  }
	
	
}//fin del metodo de envio de datos para graficar--union------------------------------


//busca si esta repetido el valor que se quiere transmitir
unsigned char isRepeated(signed short int x,signed short int y){
const unsigned char SIZE11=200;
static signed short int xx[SIZE11];
static signed short int yy[SIZE11];
unsigned char i;
static unsigned char ii;

        for(i=0;i<SIZE11;i++)
        	 if(yy[i]==x)
        		 goto is339;
        goto is345;
is339:  for(i=0;i<SIZE11;i++)
         	 if(yy[i]==x)
         		 return TRUE;
is345:  if(ii>(SIZE11-1))//hace una cola circular
        	ii=0;
        xx[ii]=x; yy[ii++]=y;
        return FALSE; //la y no hay igual
}//fin is repeated 




unsigned char swapNibbles(unsigned char x){
	 return((x&0x0F)<<4 | (x&0xF0)>>4);
}//fin de swap nibbles--------------------------------------------------------

unsigned char  SemaforoTX(unsigned char s, char modo){
static 	unsigned char Semaforo_TX;
   if(modo=='w'){
	  if(s==TRUE) 
	    Semaforo_TX=TRANSMITED; 
	  else 
		Semaforo_TX=NO_TRANSMITED;
      return 0;}
   else
	   return Semaforo_TX;
}//


unsigned char isGotisEnabled(void){
	 return TRUE;//DEBUG
     if((RXdata[1]&0x80)==0x00){//es el byte correcto?
    	 if((RXdata[1]&0x40)==0x40)//is enabled?
    	         return TRUE;
    	 else return FALSE;}         
     else{ 
    	 if((RXdata[0]&0x80)==0x00){//entons se voltearon los bytes 
    		 if((RXdata[0]&0x40)==0x40)//is enabled?
    			 return TRUE;
    	     else return FALSE;}
    	 else
    		  eLog(CODE_ERROR_COMM_IO_RX);
         }//fin else	
return 0;     
}//is gotis enabed? -------------------------------------------------



/*bytes de Recepcion UP:  A,B
*                MSN(7)       6    5     4      3       2       1       0
*      bits de A:  --1-- ,TACHO,  Gatin, SysChk,PackCHk,ResetEn,ResetSW,AuXin
*      bits de B:  --0-- ,GOTIS,ASK-Dat,Lock-SW, Aux2IN,datReq0,datReq1,n/a
* 
*      datReq0   datReq1:
*         0           0:Solo valores ADC
*         0           1:parametros de display, sensibilidad, phase, altura, ganancia
*         1           0:Reporte,
*         1           1:Respaldo de Productos
*/
unsigned char  getANS_DATOS(void){
unsigned char c;
         if((RXdata[0]&0x80)==0x00){
              c=RXdata[0]&0x06;  //0000 0110
        	  goto swloop;} 
         else{if((RXdata[1]&0x80)==0x00){
        	     c=RXdata[0]&0x06;  //0000 0110
        	     goto swloop;} 
              else{
            	  eLog(CODE_ERROR_COMM_IO_RX);
                  return 0; }}
swloop:   switch(c){
              case 0x00:return 0x00; //IOouts.ANS_DATOS=0x00;  //0000 0000
            	        break;
              case 0x02:return 0x02; //IOouts.ANS_DATOS=0x02;	  //0000 0010
            	        break;
              case 0x04:return 0x04; //IOouts.ANS_DATOS=0x04;	  //0000 0100
            	        break;
              case 0x06:return 0x06; //IOouts.ANS_DATOS=0x06; 	  //0000 0110
                        break;
              default:eLog(CODE_ERROR_0006);
            	      break;
           }//endswithc         
	
}//fin get answer datos--------------------------------------------



/*  fin CONTROLADRES MAESTROS DE COMUNICACIONES PROCESADORA-ENTRADAS/SALIDAS+++++++++++++++++++++++++++++++++
 *   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++ +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/*  fin CONTROLADRES MAESTROS DE COMUNICACIONES PROCESADORA-ENTRADAS/SALIDAS+++++++++++++++++++++++++++++++++
 *   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++ +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/*  fin CONTROLADRES MAESTROS DE COMUNICACIONES PROCESADORA-ENTRADAS/SALIDAS+++++++++++++++++++++++++++++++++
 *   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++ +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */





/*Insercion de un numero proveniente del procesamiento del
 * ADC para ser transmitido al  Goptix    
 * lo mete a unas fifos y una maquina de de interrupciones los va transmitiendo
 *  con su control de protocolo, claro  
 *  transmite la informacion del DSP ala IO */
void Tx_DSP2IO(signed short int x1,signed short int y1){   
struct COMMs_IOUP nodo;	
sword16 sWord; 
	//signed short int x2=(signed short int)0x1234 ,y2=(signed short int)0x5678;//debug	
 if(serial.vars.Flags.bitF.EnableComunicationsTX){	 
	if(serial.IO.Salidas.bitA.Gotis==TRUE){ //las transmisiones al IO estan activadas? 
			nodo.MaquinaEstados=READY;//esta listo el nodo para ser transitido
			//nodo.datos[0]=READY;
			sWord.coord16=x1;
			nodo.xh=sWord.byte[_HI_];
			nodo.xl=sWord.byte[_LO_];
			sWord.coord16=y1;
		    nodo.yh=sWord.byte[_HI_];
			nodo.yl=sWord.byte[_LO_];
			//nodo.ndatos=5;
		    insertFIFOcommsIOUp(nodo);}}
	        
}// transmision to IO tarjeta------------------------------------------------------------






/*************************++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     CONTROLES DE SALIDAS Y ENTRADAS DE HARDWARE A NIVEL NUCLEO++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

unsigned char PIN_CTS_UART0(void){
        //if(((unsigned char)(getReg8(SETUA) & 0x08))==0x08)
	    if((CTS_UART0_GetVal() & 0x08)==0x08)
        	return TRUE;
        else return FALSE;
        
}//fin  read CTS  port from the UART0 belonging to UP-IO  control comunications


//Description	Resource	Path	Location	Type
//fined : "Rechazo_Salida" Referenced from "detectionAnalisis" in	39_16_49_IN1UP		 	C/C++ Problem


/* deteccion es la deteccion en tiemo real en cada instante
 * y con timers y controladores activa el tren de 
 * rechaos que se guardan en la fifo, aqui se van 
 * guardar en la fifo, y en tiempo real el 
 * sistema operativo los va a ir sacando.
 * */
void ReleRechazo(unsigned char status){
unsigned char dato[7];//={STX,0x05,CMD_RELE_FALLA_FLASH,1,status&0x7F};	
unsigned char crc;

	if((status&0x80)==0x80){//comando flash de testing prueba menu
		 RTS_UART0_PutVal(FALSE);
		 serial.IO.Salidas.bitA.Rechazo=0;
		 dato[0]=STX;dato[1]=0x05;//lenght
		 dato[2]=CMD_RELE_RECHAZO_FLASH;//comando de flash falla
		 dato[3]=2;// Numero de pulsos flash
		 dato[4]=status&0x7F; //sacamos la duracion delos pulsos
		 serial.tx.appendByte(dato[0],&serial.tx);
		 serial.tx.appendByte(dato[1],&serial.tx);//len
		 serial.tx.appendByte(dato[2],&serial.tx);//COMANDO RELLE ALARMA ON
		 serial.tx.appendByte(dato[3],&serial.tx);//solo 1 flash
		 serial.tx.appendByte(dato[4],&serial.tx);//duracion 1500mseg
		 crc=getCheckSUM(dato[2],&dato[3],2);
		 serial.tx.appendByte(crc,&serial.tx);//CRC
		 serial.tx.appendByte(ETX,&serial.tx);
	     RTS_UART0_PutVal(TRUE);}
	else{serial.IO.Salidas.bitA.Rechazo=status&0x01;//filtramos y vaciamos estado
		 RTS_UART0_PutVal(serial.IO.Salidas.bitA.Rechazo);}
}//funcione de encapsulacion para de manejo de salidas de la Tarjeta IO




        
void ReleFalla(unsigned char status){
unsigned char dato[7];//={STX,0x05,CMD_RELE_FALLA_FLASH,1,status&0x7F};
unsigned char crc;
     dato[0]=STX;dato[1]=0x05;//lenght
     dato[2]=CMD_RELE_FALLA__FLASH;//comando de flash falla
     dato[3]=1;// Numero de pulsos flash
     dato[4]=0x32; //duracion delos pulsos
	 if((status&0x80)==0x80){
		 serial.tx.appendByte(dato[0],&serial.tx);
	  	 serial.tx.appendByte(dato[1],&serial.tx);//len
	 	 serial.tx.appendByte(dato[2],&serial.tx);//COMANDO RELLE ALARMA ON
		 serial.tx.appendByte(dato[3],&serial.tx);//solo 1 flash
		 serial.tx.appendByte(dato[4],&serial.tx);//duracion 1500mseg
		 crc=getCheckSUM(dato[2],&dato[3],2);
		 serial.tx.appendByte(crc,&serial.tx);//CRC
		 serial.tx.appendByte(ETX,&serial.tx);
		 serial.IO.Salidas.bitA.Fail=0;}
	 else{		   
		 serial.IO.Salidas.bitA.Fail=status;
		 if(status){serial.tx.appendByte(STX,&serial.tx);
					serial.tx.appendByte(0x03,&serial.tx);//len
					serial.tx.appendByte(CMD_RELE_FALLA_ON,&serial.tx);//COMANDO RELLE ALARMA ON
					serial.tx.appendByte(CMD_RELE_FALLA_ON_CRC,&serial.tx);
					serial.tx.appendByte(ETX,&serial.tx);}
		 else{serial.tx.appendByte(STX,&serial.tx);
			  serial.tx.appendByte(0x03,&serial.tx);//len
			  serial.tx.appendByte(CMD_RELE_FALLA_OFF,&serial.tx);//COMANDO RELLE ALARMA ON
			  serial.tx.appendByte(CMD_RELE_FALLA_OFF_CRC,&serial.tx);
			  serial.tx.appendByte(ETX,&serial.tx);  }}
}//fin ReleFalla-------------------------------------------

void ReleAlarma(unsigned char status){
	    serial.IO.Salidas.bitA.Warnning=status;}
void ReleBloqueo(unsigned char status){
	    serial.IO.Salidas.bitA.Lock=status;}
void SalidaPHVF(unsigned char status){
	    serial.IO.Salidas.bitA.PFVE=status;}
void SalidaAux(unsigned char status){
	    serial.IO.Salidas.bitA.Aux_out=status;}


unsigned char getTacho(void){return serial.IO.TACHO;  }
unsigned char getGatin(void){return serial.IO.Ent.bitB.GATING;  }
unsigned char sysCHK(void)  {return serial.IO.Ent.bitB.SYS_CHK; }
unsigned char packCHK(void) {return serial.IO.Ent.bitB.PACK_CHK;}
unsigned char Aux1IN(void)  {return serial.IO.Ent.bitB.AUX1; }
unsigned char Aux2IN(void)  {return serial.IO.Ent.bitB.AUX2; }
unsigned char LockSW(void)  {return serial.IO.Ent.bitB.LockSW; }
unsigned char ResetSW(void) {return serial.IO.Ent.bitB.ResetSW;}



void IN3UP_test(void){
//static unsigned long int count;
//const unsigned char STX=0x03,ETX=0x02;
//const unsigned char CMD_DDS=0x13;
//const unsigned char SIZE_CO=14;
//word n;
//unsigned char a[]=" Hola ely ";
//unsigned char b[]={STX,CMD_DDS,0x32,0x00,0x32,0x00,0x88,ETX};
//unsigned char c[]={"123456789"};
//signed short int x[SIZE_CO]={-301,500,32,15,-25,99,-88,192,102,-56,-43,45,22,-66};
//signed short int y[SIZE_CO]={ 22,13,-123,44,-12,-34,78,-73,22, 43, -22,34,12,56};
//unsigned char d[8];
//unsigned char h,l;
//static unsigned char i;
//    if(++count>4825){//con 50000 se envian cada 2.5seg
//    	count=0;
//    	if(i>SIZE_CO)
//    		i=0;
//    	d[0]=STX;d[1]=CMD_DDS;
//    	convert_SI_byte(&h,&l,x[i]);
//    	d[2]=h;d[3]=l;
//    	convert_SI_byte(&h,&l,y[i++]);
//    	d[4]=h;d[5]=l;
//    	d[6]=getCRC3(&d[1],6);
//    	d[7]=ETX;
//    	IOUPserial_SendBlock(&d[0],sizeof(d),&n);
//    }
}//fin IN3UP test ------------------------------------------------

/* cnversion de signed int a byte dvidiendo el valor a 2 bytes */
void convert_SI_byte(unsigned char *h,unsigned char *l,signed short int a){
union _valor_{
  signed short int signo;
  unsigned char   byte[2];
}v;
        v.signo=a;
        *h=v.byte[0];
        *l=v.byte[1];
}//fin conversion de SIGNED INT  a bytes--------------------------

/* obtener el CRC, parameter pointer de los parametros
 * y comando, n: es e numero de bytes desde el primero*/
unsigned char getCRC3(unsigned char *p,unsigned char n){
unsigned char i,suma=0,ret;
	for(i=0;i<n;i++){
		 suma+=*(p+i);}
	ret=((unsigned char)~suma);	
return ret;
}//fin get CRC ----------------------------



/// nos regresa el numero de byet a enviar en funcion del comando contando el comando
unsigned char getLenghtCMD(unsigned char comando){
unsigned char ret;	
      comando&=Mask_CMD;  
      switch(comando){
    	  case CMD_GOTIS_XY: ret=5;//<stx><cmd><xl,xh,yl,yh,><etx>
    	                     break;
    	  default:ret=0;break;
      }	//fin de switch-------------------------
	      
return ret;
}//fin de obtener numero de bytes de un comando a enviar.





/*prcesar los bytes que llegron en la recepcion serial para
//ejecutar comandos los procesamos de manera operativa
//protocolo
//  STX,LEN,CMD,PARAM0,..PARMn,CRC,ETX
   version 3 */
void IOUP_RECEPTION_COMUNICATIONS_CONTROLLER(void){
static unsigned char estado,estado0,cmd,len;	
unsigned char a;	
static unsigned char (*fp)(unsigned char,unsigned char,unsigned short int);//comando a procesar
static unsigned short int sum;//para el crc
static unsigned char i;
word n;
IOUPserial_TError e;
const BUFF_SIZE =50;
static unsigned char  buff[BUFF_SIZE];
//static unsigned char  buff2[BUFF_SIZE];

#if (SIZE_BUFFER1>30)
  #define LIMIT 15
#else
  #define LIMIT SIZE_BUFFER1-(SIZE_BUFFER1/2)
#endif
	
   if(!estado){  
    if(IOUPserial_GetCharsInRxBuf()>15){
       switch(IOUPserial_RecvBlock(&buff[0],sizeof(buff),&n)){
    	  case ERR_OK:estado=1;break;
    	  case ERR_SPEED://This device does not work in the active speed mode
    	  case ERR_RXEMPTY://The receive buffer didn't contain the requested number of data. Only available data has been returned.
    	  case ERR_COMMON://common error occurred (the GetError method can be used for error specification)
    		              IOUPserial_GetError(&e);
        		          if(e.errName.OverRun){
    		            	        IOUPserial_TurnRxOff();
    		            	        estado=1;}
						  else{if(!(e.err)){
    		            	        estado=1;}//__asm(nop);} 
    		                   else{if(e.errName.OverRun){
   		            	                  IOUPserial_TurnRxOff();
   		            	                  estado=1;}
    		                        else{__asm(nop); 
										__asm(Halt);
										__asm(nop);}}}
    	                  break;}}}//fin if------------------------------------
   
    switch(estado){
      case 0:enable_Comms_TX();break;
	  case 1:i=0;estado++;break;
lx4c: case 2:if(i<BUFF_SIZE){
				 if(buff[i]!=STX){
					  if(++i<BUFF_SIZE)
							  goto lx4c;
					  else {estado=0;}}
				 else{cmd=buff[i];
					  estado++;}}
			 else{estado=0;}
             break;
	  case 3:if(++i<BUFF_SIZE){
				  len=buff[i];
				  if(len>0){
					   cmd=0;sum=0;
					   estado++;}
				  else{if(++i<BUFF_SIZE)
				    	   estado=0;
				       else estado=2;}}
			 break;
	  case 4:if(++i<BUFF_SIZE){
		           a=buff[i];
	               if(cmd==0){//estmos en byte comando
	            	   sum=cmd=a;//para guardar el tercer byte del paquete
	            	   fp=Buqueda_de_Comandos_RX(cmd);//comndo a procesar,regresa apuntador a funcion de comando
	            	   if(is_Longitud_wrong(cmd,len)){
	            	            estado=2;}
	            	   len--;
	            	   break;}
	               else{sum+=a;
	                    fp(len,a,sum);}
	               if(!(--len))
	                    estado=2;}
	  	  	  else estado=2;
	          break;
	  default:estado=0;break;}
}//fin IOUP_RECEPTION_COMUNICATIONS_CONTROLLER----------------------------------



// version:28-03-22-1448
unsigned char is_Longitud_wrong(unsigned char cmd,unsigned char len){
	
	switch(cmd){	 
	case CMD_RELE_FALLA_ON:      //      0x14//03 03 14 13 02
	case CMD_RELE_FALLA_OFF:     //0x15//03 03 15 E4 02
	case CMD_RELE_ALARMA_ON:     //0x16//03 03 16 E3 02
	case CMD_RELE_ALARMA_OFF:    //0x17//03 03 17 E2 02
	case CMD_RELE_LOCK_ON:      // 0x18//03 03 18 E1 02
	case CMD_RELE_LOCK_OFF:      //0x19//03 03 19 E0 02
	case CMD_TX_ACK_UPIO:
		                   if(len==0x03)
		                	    return FALSE;//isnt wrong
		                   break;
            //cc: CRC cuenta apertir del comando  //3,5,1A,32,1E,2 Delay:5seg,duration:3seg
	case CMD_RELE_FALLA__FLASH:
	case CMD_RELE_ALARMA_FLASH:
    case CMD_RELE_LOCK_FLASH:
	case CMD_RELE_REJECT_CONFIG: // 0x1A//03 05 1A aa bb cc 02  aa:rechazo en 255*100=25.5segundos  bb:delay
                                if(len==0x05)
                                	 return FALSE;//nothing is wrong
                                break;
	
	default:return TRUE; break;	}//fin switch, true somthing is wrong
return TRUE;	
}//



/*busqued de comandos de recepcion, 
 param c. numero de comando num hexadecimal
*  regresa un appuntador a una funcion con parametro uchar
*  // version:28-03-22-900 */
unsigned char (*Buqueda_de_Comandos_RX(unsigned char c))(unsigned char,unsigned char,unsigned short int){
	
	  switch(c){//				    return Commando_Rele_Falla_Flash; break;
	   case CMD_RELE_FALLA_ON:  //return Comando_Rele_Falla_ON;  break;
	   case CMD_RELE_FALLA_OFF: //return Comando_Rele_Falla_OFF; break;
	   case CMD_RELE_ALARMA_ON: //return Comando_Rele_Alarma_ON; break;
	   case CMD_RELE_ALARMA_OFF://return Comando_Rele_Alarma_OFF;break; 
	   case CMD_RELE_LOCK_ON:   //return Comando_Rele_Lock_ON;   break;
	   case CMD_RELE_LOCK_OFF:  //return Comando_Rele_Lock_OFF;  break; 
	   case CMD_RELE_REJECT_CONFIG:// return Comando_Rele_Reject_Conf;  break;
	   case CMD_RELE_ALARMA_FLASH: // return Commando_Rele_Alarm_Flash; break;
	   case CMD_RELE_LOCK_FLASH:   // return Commando_Rele_Lock_Flash;  break;
	   case CMD_RELE_FALLA__FLASH://return Commando_Rele_Falla_Flash; break;
	   case CMD_TX_ACK_UPIO: return Commando_TX_ACK_UPIO;break;
                                  break;
	   default: return 0;} 
}//fin Buqueda_de_Comandos_RX------------------------------


/*    3   3-> 1   2   3
 *    3h  3h 14h 13h  2h
 *   STX,LEN,CMD,CRC,ETX 
 *   para c. indica que posicion de byte parametro es
 *   d: es el byte del parametro valor
 *    version 070422-1201*/
unsigned char Commando_TX_ACK_UPIO(unsigned char c,unsigned char d,unsigned short int s){
	s=0; 
	switch(c){
	   case 2:if(d==CMD_TX_ACK_UPIO_CRC){
		                serial.vars.var.ncountNopeAns=0;//hay respuesta
		                //serial.vars.Flags.bitF.ledComunicaciones=0;
		                e.ERROR.bits.aLed4_Comm_Warnning_Sys=0;
	                 return TRUE;}//COMAndo ejecutado just fine
	          else return FALSE;         
		      break;
	   case 1:return TRUE;
	   default:break;}
}//fin Comando_Rele_Falla_ON---------------------------------


/*se interrupe cada segundo*/
void IRQ_comunicacion_IOUP(void){
//static unsigned char ncount1;	
//const unsigned char TIEMPO_MONITOR=5;//TIEMPO para declarar no comunicaciones
       
//      if(ncount1++>TIEMPO_MONITOR){
//    	   ncount1=0;
//    	   serial.vars.Flags.bitF.ledComunicaciones=ERROR_RECEPCION_ACK_IO;}
//      
	
}//fin IRQ_comunicacion_IOUP------------------------------------



/*Transmision de acknowledge hacia la IO la IO manda el akw
 * y en la UP  se pone la bandera en cero y vuelve a mandar el Ack
 *  si no recive ack, un led es mas lento
 *   verison 280322-1520 */
void Transmission_Acknowledge(void){
	 

	    serial.tx.appendByte(STX,&serial.tx);
	    serial.tx.appendByte(0x03,&serial.tx);
	    serial.tx.appendByte(CMD_TX_ACK_UPIO,&serial.tx);
	    serial.tx.appendByte(CMD_TX_ACK_UPIO_CRC,&serial.tx);
	    serial.tx.appendByte(ETX,&serial.tx);
	
}//fin Transmission_Acknowledge-------------------------------


void vtask11_Transmision_General_hacia_la_IO(void){
	vTask1_Transmision_al_CPU_TFT();
	vTask3_ImAlive_Transm_al_CPU_TFT();
	vTask2_ADC_to_FIFO_tx_CPU_TFT();
	vTask10_Comando_Deteccion_Prod();
	vTask16_ErrorBAlance_Transm_al_CPU_TFT();
	vTask17_Error_Offset_Unset_Transm_al_CPU_TFT();
    vTask18_Error_Driver_OFF_Transm_al_CPU_TFT();

}//fin vtask11_Transmision_General_hacia_la_IO---------




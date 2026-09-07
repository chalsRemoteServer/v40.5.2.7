/*
 * IOcomms.h
 *
 *  Created on: Dec 4, 2018
 *      Author: desarrollo-1
 */

/*
 * IOcomms.h
 *
 *  Created on: Dec 5, 2016
 *      Author: chals
 *      
 85         decimal 
0213        octal
0x4b        hexadecimal 
30          int 
30u         unsigned int 
30l         long
30ul        unsigned long 

 */

#ifndef IOCOMMS_H_
#define IOCOMMS_H_





/**
 * ESTOS DEFINES TIENE QUE SE IDENTICOS EN LA TARJETA IO
            VERSION  B0 */
#define _HI_         0
#define _LO_         1

#define  NODO_FREE     0
#define  READY        0x88 //NO MOVER VALOR, indica que esta listo para ser transmitido
#define  TRANSMITING  0x89 //estamos transmitiendo
#define  libre         0x8A //comando transmitir del ADC a la IO
#define  IOUP_ACK     0x8B //YA tenemos Ack libera el nodo
#define  BYTE_COMMAND 0x8C
#define  BYTE_XL      0x8D
#define  BYTE_XH      0x8E
#define  BYTE_YL      0x8F
#define  BYTE_YH      0x90

#define  TRANSMITED   0xAA
#define  NO_TRANSMITED 0xBB


//#define  STOP     0x8AU //definido ya en I2C.h LE DECIMOS  que noesta abierto el  GOPIS ppor tanto la IO no lo requiere
#define  TTL_TIME          100   //TIEMPO  necesario de loop en IO_TRANSMISSION_CONTROLLER para espera la respuesta de la IO de cada paquete enviado


//NO MOVER VALORES DE COEF DE RECEPCION
#define  DATOS_ADC    0x00 //NO MOVER VALORESse transmiten coordenadas xy del ADC
#define  DATOS_DISP   0x02// datos de despliegue del adc y coordenadas del ADC
#define  DATOS_REPORT 0x04 // se transmiten datos de informe y reportes
#define  DATOS_PROD   0x06//respaldo de productos

//tamaños
//#define SIZE_DATOS     10 //NUMERO DE DATOS DE LA CADENA a mandar o recibir
#define SIZE_FIFO_IOUP 10 //tamaño de la LISTA  fifo de datos de Recepcion y Transmision


#define NO_RELE              0xB5
#define RELE_DE_RECHAZO      0xB6
#define RELE_DE_FALLO        0xB7
#define RELE_DE_ADVERTENCIA  0xB8
#define SALIDA_DE_REPUESTO   0xB9

//COMANDOS DE COMUNICACION  CON LA TARJETA IO
//version: 280322-300
#define Mask_CMD        0xFE  //1111 1110 mascara de comando para quitar el  rechazo
#define CMD_GOTIS_XY    0x82   //1000 001R: R:bit de rechazo
#define CMD_IN3UP_GOTIS 0x13 //COMANDO PARA ver ell DDS E  EEL touch sreen
//#define CMD_DDS 0x13

#define CMD_RELE_FALLA_ON      0x14//03 03 14 13 02
#define CMD_RELE_FALLA_OFF     0x15//03 03 15 E4 02
#define CMD_RELE_ALARMA_ON     0x16//03 03 16 E3 02
#define CMD_RELE_ALARMA_OFF    0x17//03 03 17 E2 02
#define CMD_RELE_LOCK_ON       0x18//03 03 18 E1 02
#define CMD_RELE_LOCK_OFF      0x19//03 03 19 E0 02  //CRC cuenta apertir del comando 
            //cc: CRC cuenta apertir del comando  //3,5,1A,32,1E,2 Delay:5seg,duration:3seg
#define CMD_RELE_REJECT_CONFIG 0x1A//03 05 1A aa bb cc 02  aa:rechazo en 255*100=25.5segundos  bb:delay                            
#define CMD_RELE_FALLA__FLASH  0x1B//03 05 1B nn dd CD 02 .CMD:1A,nn:NumFlashes,dd:DurationFlashes,CRC,ETX; duration*100=mseg
#define CMD_RELE_ALARMA_FLASH  0x1C//03 05 1C 01 0F CC 02 //NumFlashes=0; then infinite
#define CMD_RELE_LOCK_FLASH    0x1D//03 05 1D 01 0F CB 02 
#define CMD_TX_ACK_UPIO        0x1E//03 03 1E E1 02
#define CMD_RELE_RECHAZO_FLASH 0x1F//03 05 1F 02 28 B6 02 //2 pulsos y 4seg=28h, 40d*100d=4000mseg


//CRC es  suma, luego se toma LSB  byte, y se hace complemento 1
#define CMD_RELE_FALLA_ON_CRC   0x13  //3h,3h,14h
#define CMD_RELE_FALLA_OFF_CRC  0xE4  //3h,3h,15h  
#define CMD_RELE_ALARMA_ON_CRC  0xE3  //3h,3h,16h
#define CMD_RELE_ALARMA_OFF_CRC 0xE2  //3h,3h,17h
#define CMD_RELE_LOCK_ON_CRC    0xE1  //3h,3h,18h
#define CMD_RELE_LOCK_OFF_CRC   0xE0  //3h,3h,19h
#define CMD_TX_ACK_UPIO_CRC     0xE1  //1E


struct COMMs_IOUP{
	  //unsigned char status;//esta lleno y listo=READY, ha sido transmitido, ya ha sido reconosido, esta libre
	  unsigned char MaquinaEstados;//estado de maquina del aecepcion para completar el paquete  
      //unsigned char datos[SIZE_DATOS]; //cadena de datos a mandar
      struct COMMs_IOUP *next;
      struct COMMs_IOUP *prev;
      //unsigned char ndatos; //numero de bytes a recibir o transmitir
      unsigned char xh;
      unsigned char xl;
      unsigned char yh;
      unsigned char yl;
   
};





//Control de banderas 
struct COMMUNICACION {
	    unsigned char Libres;//nodos lbres en la FIFO
		union comms{
			unsigned char ByteFlags;//banderas
			struct{
				unsigned char writingFIFO:1; //Se esta escribiendo la fifo de transmision
				unsigned char resetRepeated:1;//resetea el control de repeticion de datos a transmitir de x & y
				unsigned char freeTXx2:1; //esta libre el buffer de trnsmision,
				unsigned char DDS_ACTIVA:1;
				unsigned char txAck:1;//transmission Acknowledge
				unsigned char x5:1;
				unsigned char x6:1;
				unsigned char x7:1;
			}bitFlag;//mover baderas por 1 bit
		 }Flags;
};





struct ENTRADAS_UP{
	     unsigned char LockSW;
	     unsigned char Tacho;
	     unsigned char Gatin;
	     unsigned char SysCHK;
	     unsigned char PackCHK;
	     unsigned char ResetSW;
	     unsigned char Aux1IN;
	     unsigned char Aux2IN;
	     unsigned char byteH;
	     unsigned char byteL;
	     unsigned char gotis;
	     unsigned char askdat;
	     unsigned char datreq0;
	     unsigned char datareq1;
	     unsigned char datareq2;
};



//DEFICIONES COMUNES EN LA IO Y EN LA PROCESADORA--------------------------------------------
//TERMINAN DEFINICIONES IDENTICAS DE LA TARJETA IO Y PROCESADORA
//DEFICIONES COMUNES EN LA IO Y EN LA PROCESADORA--------------------------------------------
//TERMINAN DEFINICIONES IDENTICAS DE LA TARJETA IO Y PROCESADORA
//DEFICIONES COMUNES EN LA IO Y EN LA PROCESADORA--------------------------------------------
//TERMINAN DEFINICIONES IDENTICAS DE LA TARJETA IO Y PROCESADORA





typedef union{//access word: 
	signed  short int coord16;   //   	0xaabb   
	unsigned char byte[2];        //byte[0]=aa,byte[1]=bb
}sword16; //coordenadas de 2 bytes 



void init_IOcomms(void);
void IOrecv_char_ISR(unsigned char c);
void enableIO_reciv(void);
void disableIO_reciv(void);
void IOserial_TransmitedChar_IRQ(void);
void IO_TRANSMISSION_CONTROLLER_ISR(void);
void Tx_DSP2IO(signed short int x1,signed short int y1);
void pushFIFOrx(unsigned char c);
void disableIO_Rx(void);
void enableIO_Rx(void);
void IO_RECEPTION_CONTROLLER(void);
void Procesador_de_Comandos_IO(void);
void activate_var_timer(void);
unsigned char CRC(unsigned char *c);
unsigned char CRC2_txCon_TX_ADC_IO(void);
struct TX_BUFFER *popFIFOtx_DSP(void);
unsigned char getRTS_UART0(void);
void init_FIFO_comms_IOUP(void);
void insertFIFOcommsUp(struct COMMs_IOUP datos);
void insertOnTopFIFO_IOUPcomms(struct COMMs_IOUP data);
void COMMUNCATIONS_CONTROLLER_IOUP(void);
unsigned char PIN_CTS_UART0(void);
void RTS_UART0_PutVal(unsigned char val);
unsigned char getCRC(struct COMMs_IOUP	node);
void insertFIFOcommsIOUp(struct COMMs_IOUP datos);
void enable_Comms_IO(void);
void insertCRC(struct COMMs_IOUP *node);
unsigned char CTSmonitor(unsigned char estadoESTOY, unsigned char estadoVOY);
unsigned char  SemaforoTX(unsigned char s, char modo);
unsigned char isGotisEnabled(void);
unsigned char  getANS_DATOS(void);	
struct COMMs_IOUP popFIFOcommsIOUp(void);
unsigned char swapNibbles(unsigned char x);
void ReleRechazo(unsigned char status);
void ReleFalla(unsigned char status);
void ReleAlarma(unsigned char status);
void ReleBloqueo(unsigned char status);
void SalidaPHVF(unsigned char status);
void SalidaAux(unsigned char status);
unsigned char getTacho(void);
unsigned char getGatin(void);
unsigned char sysCHK(void);
unsigned char packCHK(void);
unsigned char Aux1IN(void);
unsigned char Aux2IN(void);
unsigned char LockSW(void);
unsigned char ResetSW(void);
void MonitorCentralEntradas(void);
void IN3UP_test(void);
unsigned char getLenghtCMD(unsigned char comando);
unsigned char Comm_DDS_FIFO(signed short int datax,signed short int datay);
unsigned char isRepeated(signed short int x,signed short int y);
void IO_BOARD_TRANSMISSION_CONTROLLER(void);
void convert_SI_byte(unsigned char *h,unsigned char *l,signed short int a);
unsigned char getCRC3(unsigned char *p,unsigned char n);
void pop_DDSpacket_to_FIFO_serial_TX_IOUP(void);
void IOUP_BOARD_TRANSMISSION_CONTROLLER(void);
void Transmission_Acknowledge(void);
void IOUP_BOARD_SERIAL_CONTROLLER(void);
void Transmission_Acknowledge(void);
void IOUP_RECEPTION_COMUNICATIONS_CONTROLLER(void);
unsigned char is_Longitud_wrong(unsigned char cmd,unsigned char len);
unsigned char (*Buqueda_de_Comandos_RX(unsigned char c))(unsigned char,unsigned char,unsigned short int);
unsigned char Commando_TX_ACK_UPIO(unsigned char c,unsigned char d,unsigned short int s);
void IRQ_comunicacion_IOUP(void);
void vTask2_Receptor_Procesador_MCU_TFT_IO(void);
unsigned char calculaCRC(const unsigned char *data, unsigned char len);
void vtask11_Transmision_General_hacia_la_IO(void);

#endif /* IOCOMMS_H_ */



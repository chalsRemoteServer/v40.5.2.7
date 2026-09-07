/*
 * I2C_.h
 *
 *  Created on: Sep 3, 2015
 *      Author: chals
 */

#ifndef I2C__H_
#define I2C__H_



#endif /* I2C__H_ */

#define TRANSMITIR  0x10
#define RECIBIR     0xEF
#define STOP        0x33
#define START       0x34
#define WRITE       0x35  //write device from from memory address pointer
//#define READ        0x36  //read  from current pointer location ds1338
#define READP       0x37  //read from pointer address location ds1338
#define READP2      0x38  //READ pointer DS1338 paso2
#define READP2A     0x28
#define READP3      0x39  //read pointer DS1338 paso3
#define READP4      0x3A
#define PAGEWRITE   0x3B  // page writing for the cat24L01
#define SEQREAD     0x3C  //secquencial read
#define SEQREAD2    0x3D  //secquencial read
#define SEQREAD3    0x3E  //secquencial read
#define SEQREAD4    0x3F  //secquencial read
#define SEQREAD5    0x40  //secquencial read

#define BYTEWRITE   0x41 //ESCRIBIR Solo un byte
#define BYTEWRITE1  0x42
#define BYTEWRITE2  0x43
#define BYTEWRITE3  0x44
#define BYTEREAD    0x45
#define BYTEREAD1   0x46
#define BYTEREAD2   0x47
#define BYTEREAD3   0x48
#define BYTEREAD4   0x49

#define IEN         0x80 //1xxx xxxx Enable module I2C reg ICR
#define IIEN        0x40 //x1xx xxxx Enable interrupt module I2C register ICR
#define MSTA        0x20 //xx1x xxxx Master/Slave Mode Select MAster=1, 0->1:Signals START, 1->0:Signals STOP (ICR)
#define MTX         0x10 //xxx1 xxxx (0:Receive, 1:Transmit)MASTER_MODE
#define TXAK        0x08 //xxxx 1xxx Tx Ack/ 0:Ack sent after receive 1 byte data,.Receive MODE
                         //1:No Ack signal receive, IF is receiver-> Write this bit as 0. Register ICR
#define RSTA        0x04 //xxxx x1xx Repeat START, 0:No repeat START, 1:Generates  repeat START  conditions. Sets it without mastership causes loss of arbitration reG(ICR)
#define ICF         0x80 //1xxx xxxx 0:Transfer in progress  0:Transfer Complete reg(ISR)
#define IAAS        0x40 //x1xx xxxx  I2C addresses as Slave Reg(ISR)  for SLAVE_MODE
#define IBB         0x20 //xx1x xxxx  0:Bus is idle,STOP detected clear it, 1:bus is Busy->START detect IBB is set
#define IAL         0x10 //xxx1 xxxx  ARBITRATION LOSS
#define SRW         0x04 //XXXX x1xx  Slave R/W for SLAVE MODE ONLY
#define IIF         0x02 //xxxx xx1x  I2C interrupt, must cleared by software,.1:one transfer Complete, arbitration loss
#define RXAK        0x01 //xxxx xxx1  Receive ACknowlegde 0:ACK signal was received  after byte transmission.

//NVRAM OPERATIONS
#define DS1338_NVRAM_READ_SLAVE_TX     0x50 //operacion READ FROM CURRENT POINTER LOCATION,, Slave Transmit Mode
#define DS1338_NVRAM_WRITE_SLAVE_RX    0x51//write slave receive,, Slave ReCeive Mode
#define DS1338_NVRAM_READ_SLAVE_RX_TX  0x52//write pointer then Read, Slave receive then transmit


#define I2C_OS1     0xA0


#define ENABLE      0x01
#define DISABLE     0x00
//direcciones de dispositivos->>NO MOVER
#define MMA8453     0x38//esta direccion no esta verificada 
#define CAT2401     0xA0//direccion fijas->NO MOVER ESTE DATO
#define DS1338      0xD0//direccion fijas->NO MOVER ESTE DATO

#define RXAK        0x01//flag from I2SR@bit0, 0:an ACK WAS receiver, 1: no Ack from receiver
#define ICF         0x80 // bandera only Read DATA transferrind bit form I2SR 0:trasfer in progress,1:complet transfer
#define TXACK       0x08
#define RSTA        0x04
#define MTX         0x10 //bit 4  Transmit/receive mode
#define MSTA        0x20 //0010 0000 MAster/Slave mode   1->0:STOP  0->1:START


#define NVRAMREGCONTRLSIZE     8// numero de registros de tiempo y cntrol de la NVRAM
#define I2CBUFSIZE       50//maximo tamaño del buffer I2C->NO DEBE SER MAS DE 0xFF por la funcion de abajo
//#define I2CFIFOSIZE       5 //tamaño del buffer de FIFO
//#define I2C_RECIBIENDO   0x68
//#define I2C_LIBRE        0

#define NODO_FREE    0  // /70xA1  

#define I2Cn1           0xDE
#define I2Cinit         0xDF
#define I2Cerror        0xE0
#define I2CstateMachine 0xE1
#define I2C_OS2         0xE2

// IIC-----------------------
#define EEPROM  0xA0 // add=1010 0000
#define ACELER  0x1C // add=0001 1100
#define RTC_IC  0x68 // add=0110 1000
#define ARDUINO 0x17 // ADD=0001 0111  Arduino as DDS
#define CAMBIO_FRECQ_APP     0x88 //APLICACIONES QUE USAN IIC
#define      NUEVA_FRECQ_CMD 0x81// comando nueva frecuencia.


#define BUFFER_TX_IIC   50
#define BUFFER_RX_IIC   50
#define PACK_SIZE       10//STX,DEVICE,SIZE,CMD,PARAM0,PARAM1...PARAM2,ETX

#define e0    0
#define e1    0x32
#define e2    0x33
#define e3    0x34
#define e4    0x35
#define e5    0x36

//struct ----------------------------------------------------------------------------

struct I2C_NODE{
   unsigned char  device;//dispositivo
   unsigned char ndata;//numero de datos 
   unsigned char operacion;//que operacion es
   unsigned long int address;
   unsigned char id;//id de la operacion
   struct I2C_NODE *next;
   struct I2C_NODE *prev;
   unsigned char data[I2CBUFSIZE];
};


 struct _I2C_CONTROL{
   unsigned char ncountTX;//tamaño de la cola de TX
   unsigned char ncountRX;//Tamaño cola de RX
   unsigned char buffTX[BUFFER_TX_IIC];
   unsigned char buffRX[BUFFER_RX_IIC];
   unsigned char *headTx;//apunta al array de tx al head
   unsigned char *lasTx;
   unsigned char *popTx;
   unsigned char *pushTx;
   unsigned char *headRx;//apunta al array de tx al head
   unsigned char *lasRx;
   unsigned char *popRx;
   unsigned char *pushRx;
   unsigned char (*PushTX)(unsigned char a);//apunta ala funcion push de TXde iic
   unsigned char (*popTX)(void);
   unsigned char (*PushRX)(unsigned char a);//apunta ala funcion push de TXde iic
   unsigned char (*popRX)(void);
   unsigned char device;//device actual selected
   unsigned char semaforo;
   unsigned char estado;
   unsigned char nbytes;//cuenta de bytes enviados
   unsigned char paquete[PACK_SIZE];
   void (*start)(void);//funcion start de I2C
   unsigned char (*Ack)(void);//FUNCION to know if there are acknowledge after ninth pulse 
   void (*stop)(void);//stop de IIC
   void (*sendPack)(unsigned char *p,unsigned char len);
   unsigned char delayCnt;//delay count
 };



void init_I2C(void);
inline void clearflagIIF(void);
void setI2CMode(unsigned char m);
void signalBus(unsigned char n);
inline unsigned char isRXAK(void);
void I2Ctransmit(unsigned char chipAdd);
unsigned char getI2CMode(void);
unsigned char I2CbeginTransmission(unsigned char add,unsigned rw );
//unsigned char I2Cwrite(unsigned char datos);
void configureReloj(unsigned char controlRegister);
void saveClockNVRAM(unsigned char reg[]);
unsigned char *getCalendarRTC(void);
void I2Cinterrupts(char e);
unsigned char I2Cread(void);
unsigned char pruebaI2C(void);
//unsigned char noACK(unsigned short int t);
void I2Cstop(void);
void I2Cstart(void);
void I2Crepeatedstart(void);
void I2Csend(unsigned char d);
unsigned char GestionI2C(void);
void txACK(unsigned char a);
unsigned char I2Cget(void);
void I2Cinterrupts(char e);
unsigned char I2Ccomm(unsigned char device1);
unsigned char prueba2I2C(void);
void pruebatxe2prom(void);
void pruebarxe2prom(void);
//unsigned char *readEEPROM(unsigned long int add1,unsigned char nbytes);
void I2CliberarModulo(void); //liberamos el modulo I2C desde el exterior,
unsigned char *getPointerI2CBuffTx(unsigned char device1);
unsigned char *readNVRAM(unsigned char add1,unsigned char nbytes,unsigned char id);
void writeNVRAM(unsigned char *p,unsigned char add1,unsigned char nbytes);
void initI2C(void);
void start(void);
unsigned char noACK(void);
void beginTransmissions(unsigned char add);	
unsigned char getI2CdevAddress(unsigned char Device, unsigned long int e2promAdd);
void writeEEPROMbyte(unsigned long int eadd, unsigned char data);
void I2Cwrite(unsigned char data);
void I2C_polling_ISR(void);
void init_I2C_FIFO(void);
void ControladorMaestroEEPROM(void);
unsigned char *getPointerI2CBuff(unsigned char device1);
//unsigned char *readEEPROM(unsigned long int eadd);
//void i2cFIFOpush(unsigned long int eadd,unsigned char op,unsigned char id,unsigned char *data,unsigned char nd);
char i2cFIFOpush(unsigned long int eadd, unsigned char op,unsigned char id,unsigned char *data, unsigned char nd);
//void ControladorOperativoComunicacionI2C(void);
void i2cFIFOpop(struct I2C_NODE *i2cNodo);
unsigned char I2Cgetid(void);
void I2Cfreeid(unsigned char id);
void Controlador_Principal_Central_Communicacion_IIC(void);
void Controlador_Operativo_de_Recepcion_IIC(void);
void Controlador_Operativo_de_Transmision_IIC(void);
void control_Transito_IIC(void);
unsigned char ACK_IIC(void);
unsigned char  procesador_Paquete_IIC(unsigned char *p);
void II2_enviar_Paquete(unsigned char *p,unsigned char len);
void Cambio_de_Frecuencia_por_IIC(unsigned short int f);
void I2C_Monitor (void);

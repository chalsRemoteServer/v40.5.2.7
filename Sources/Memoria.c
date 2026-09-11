/*
 * Memoria.c
 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */


/*
¿'
0´+} * MemoConfig.c
 *
 *  Created on: Jul 15, 2015
 *      Author: chals
 */

#include "Memoria.h"
#include "VFDmenu.h"
#include "Reloj.h"
#include "system.h"
#include "I2C_.h"
#include "errorController.h"
#include "delay.h"
#include "frecqController.h"
#include "VFD.h"
#include "DSP.h"
#include "IOcomms.h"
#include "strings.h"
#include "VFDisplay.h"
#include "queue.h"

#define INITIALIZED_MEMO_VARS  //INDICA QUE YA SE inzializaron las variables que sacamos de la EEPROM
#define EEPROM_ADD_BIOS  I2CBUFSIZE+2
#define NVRAM_ADD_BIOS   0x09 //DIRECCIOnes de uso general en NVRAM 08 al 3Fh


/*#define  TACO    1
#define FOTOCEL  2
#define RECHAZO  3
#define PRODUCTO 4
#define AUX1     5
#define AUX2     6*/

typedef union{//access word: 
	unsigned  short int wordx;   //   	0xaabb   
	unsigned char byteH;      //         aa
	unsigned char byte[2];        //byte[0]=aa,byte[1]=bb
}uword;

//variabes por contexto 
extern volatile unsigned char  TM1_IRQ1;
extern unsigned char arg0,arg1,arg2,arg5,arg6,arg8,arg9,arg10;//argumentos para entrar en un contexto
extern unsigned short int arg7,arg3;
//extern unsigned char EventDDS;
//estructura de variabes por  contexto solo en el contexto Actual
signed short int mem[SIZE_MEM];
//signed short int memy[SIZE_MEM];
signed short int x9[SIZE_9];
signed short int y9[SIZE_9];
const unsigned short int SIZE_EEPROM=2000;
unsigned char EEPROM_1Mb[SIZE_EEPROM];	

extern struct ZoomControl zoom;
extern struct _Detection Deteccion;
//extern struct _DDS_ DDS1;
extern struct _Comando_DDS dds;
extern struct _KeyPad_ keypad;



struct EntSyst{//variables de entradas de sistema 
		unsigned char taco;
		unsigned char fotocel;
        unsigned char fotocelBloqueado;//de 1 seg a 60 segundos
		unsigned char rechazo;
		unsigned char rechazoBloqueado;
		unsigned char rechazoPruebasist_seg;
		unsigned char rechazoPruebasist_mseg;
		unsigned char producto; 
		unsigned char productoBloqueado;
		unsigned char productoPruebasist_seg;
		unsigned char productoPruebasist_mseg;
		unsigned char aux1;// xxxx=rele
		unsigned char aux1Rele;//nombre del rele seleccionado
		unsigned char aux1Label;//label seleccinado
		unsigned char aux1_seg;
		unsigned char aux1_mseg;
		unsigned char aux2;
		unsigned char aux2Rele;
		unsigned char aux2_seg;
		unsigned char aux2_mseg;
		unsigned char aux2Label;
		unsigned char byten;	
		unsigned char PulsosPorSegundo;//pulsos medidos en tiempo real por segundo
}EntradasSistemaReg;

struct DiagnosticoErrores{
       unsigned short int label;
       unsigned short int number;
};

struct SEMAFOROS{
       unsigned char I2Cdevice;//cual dispositivo se esta usando
       unsigned char operacion;//que se le va hacer
       unsigned char *p;//puntador de datos
       unsigned char semTemporizado;
}Sem;

//struct _Memoria_{
//	union _Byte_{
//		unsigned char Byte;
//		struct{
//			unsigned char a:1;//bandera de que ya se envio el buffer completo
//			unsigned char b:1;//seleccionar el buffer que llenamos en tiempo t
//			unsigned char c:1;
//			unsigned char d:1;
//			unsigned char e:1;
//			unsigned char f:1;
//			unsigned char g:1;
//			unsigned char h:1;
//		}bit;
//	  }BYTE;
//};
//
//struct _Memoria_ mem[SIZE_MEM];

unsigned char modificado; //guarda cual menu ha sido mosificado
//EntSyst EntradasSistemaReg; //estructura que guarda las variables de entrada de sistema
unsigned char IDcomm; //ID  de comunicaciones, variable
unsigned char Alta; //variable que guarda el ultimo digito de la frecuencia alta
unsigned char Media;//variable que guarda el ultimo digito de la frecuencia media
unsigned char Baja;//variable que guarda el ultimo digito de la frecuencpia baja
unsigned char PSU_ES;  //Power supply unit  ENTRADAS SALIDAS variable AJUSTE_DE_SISTEMA/INGENIERIA GRACIDA/INGENIERIA GRACIDA2
unsigned char ErrorRelojAnalogo;//variable AJUSTE_DE_SISTEMA/INGENIERIA GRACIDA/INGENIERIA GRACIDA2
unsigned char ReporteProductos;// variable menu AJUSTE_DE_SISTEMA/INGENIERIA GRACIDA/INGENIERIA_GRACIDA2
unsigned short int CuentaProducto; //variable de activacion de  cuenta de producto
unsigned short int CuentaRechazo; //variable que cuenta los rechazos
unsigned char ControlContrasena1,ControlContrasena2,ControlContrasena3,ControlContrasena4;
unsigned char Control_de_usuario; //VARIABLE que guarda el control de usuario de /AJUSTE_DE_SISTEMA/MENU_ADMINISTRATIVO/control_de_usuario                                    
unsigned char Guardia; // variable de guardia en /AJUSTE_DE_SISTEMA/MENU_ADMINSTRATIVO/Ajuste_de_Guardia
unsigned char tipo_deMaquina; // garda el tipo de maquina que esta configurado
unsigned char GananciaDriver; //ganancia de la driver en AJUSTE_DE_SISTEMA/INGENIERIA_GRACIDA/GANANCIA
unsigned char GananciaAnaloga;//ganancia de la analoga en  AJUSTE_DE_SISTEMA/INGENIERIA_GRACIDA/GANANCIA
unsigned char filtroConfig; // variable filtro  de filtro en ingenieria  gracida
unsigned char PSUIOconfig; // confifura si permite o no las salidas/entradas en la tarjeta IO
unsigned char errorAnalogoPerm; // permitir error analogo
unsigned char ReporteProductoPerm; //permitir reporte de producto
unsigned char BorrarContadores; //variable que indica si se borran los contadores
unsigned char frecuenciaSeleccion;//SELECCIONA EL NIVEL de la frecuencia actual configurada y activada
unsigned long int frecuency; //guarda el valor actual de la frecuencia activada
//unsigned short int Sensibilidad; // variable de sensibilidad, Sensibilidad2 es la misma variable
unsigned char phase,phasefrac;//phase de ajustes de deteccion la variable fraccionaria es de la misma vriable
unsigned short int  Altura; //deprecated,    variable de parametros de deteccion------para el DDS
unsigned char MarcarAltura; //variable de parametros de deteccion para el DDS
unsigned char GananciaDDS; //ganancia del DDS, esta no se ha guardado
//unsigned char ZoomDDS; //variable que guarda el Zoom actual en la DDS del producto actual
unsigned char cuadritoDDS; //variable cuadrito poco estudiada
unsigned char errorLista[SIZE_ERROR]; //guarda el estado de los errores y cada  byte es un error
unsigned char statusFactory; //nos dice si la memoria ha sido inizializada con setup
unsigned char indiceProducto; //Producto Set TestUP=1, que producto esta actualmete seleccionado
unsigned char name[NOMBRE_PRODUCTO_SIZE]; 
unsigned short int PulsosTiempoRechazo;//tiempo o pulsos del producto que tarda encendido de el rechazo
unsigned short int PulsosTiempoEspera;//tiempo o pulsos del producto que espera antes de activar el rechazzo
unsigned char Ganancia;//parametro de aplicacion desconocido por el momento.
//unsigned char FotocelBloqueado;//de 1 seg a 60 segundos
//unsigned char VIDEO_DDS[VIDX][VIDY]; //64X16   bytes   memoria de video delos puntitos actuales desplegados
unsigned char   MAPA_DDS_I[VIDX][VIDY];//es el mapa de bits para el DDS
unsigned char  MAPA_DDS_II[VIDX][VIDY];//es el mapa de bits para el DDS
unsigned char MAPA_DDS_III[VIDX][VIDY];//es el mapa de bits para el DDS
unsigned char  MAPA_DDS_IV[VIDX][VIDY];//es el mapa de bits para el DDS
unsigned char DDSmap[DDSMAPSIZEX][DDSMAPSIZEY];

extern unsigned short int arg3;//argumentos para entrar en un contexto



unsigned char Parte11AnalogVer;//guarda la version de la Analoga que vamos a controlar

extern unsigned char realTimeError;//guarda el error que se despliega en pantalla en tiempo real

//funciones externas
extern unsigned char is_FULL_FIFO_save_Temp_pixel_DDS(void);
extern void displayEEPROMdata1(unsigned char *p);
extern void init_AnalogGainControl(void);
extern unsigned char Text[NOMBRE_PRODUCTO_SIZE];//texto que regresa  del procesador de textos
//extern unsigned char igxm4; //variable generica gloabal activa solo mientras esta activo el menu actual


char  writeReadEEprom(void){// funciones del BIOS para escribir y leer la EEPROM y revisar que funcinoes
 unsigned char   a[]="prueba_de_BIOS", *p,i,k;
       p=&a[0];
//       writeEEPROM(p,EEPROM_ADD_BIOS,sizeof(a));
       displayCountingBIOS_(_EEPROM_); 
//       p=readEEPROMblock(EEPROM_ADD_BIOS,sizeof(a));
       k=(unsigned char)sizeof(a);//n lo convierte bien si no lo pones asi
       for(i=0;i<k;i++)
           if(*(p+i)!=a[i]){
                 I2CliberarModulo();
                 return FALSE;} 
       I2CliberarModulo();    
       return TRUE;    

}/// fin evaluar memOroria EEPROM

char writeReadNVRAM(void){//Funcion para la BIOS write and read and check performance
unsigned char *p=0,i=0;//son 30 espacions hrizontal, 6 renglones      
unsigned char a[]={2,3,4,5,6,7,8,9};
    pruebaI2C();
    displayCountingBIOS_(_NVRAM_); 
    delay_ms(300);
    //p=readNVRAM(0,56);nosabemos que onda
    //VFDposicion(0,2);
    for(i=0;i<0x0A;i++)
             p++;
    i=0;//NO mee lee el *p
    do{ if(*p!=a[i]){
        I2CliberarModulo(); 
           return FALSE;}
    p++;
    }while(i++<7);   
    I2CliberarModulo();
    return TRUE;
}//fin e evaluar la memria NVRAM en el BIOS---------------------------


//llenamos con pructos para hacer pruebas simulando la 
// memoria EEPROM
void init_Products(void){
unsigned char a1[]=" Salchichas";
unsigned char a2[]=" Queso";
unsigned char a3[]=" Pastillas";
unsigned char a4[]=" Pan";
unsigned char a5[]=" chocolate";		
unsigned char *p2[5],*p;		
unsigned char buffer[PROD_VAR_SIZE];//buffer para escribir variables del sistema
unsigned char i,iProd[]={3,7,10,15,20,25,33};//son los indices de productos a escribir, los iProd a escribir


	 p2[0]=&a1[0];
	 p2[1]=&a2[0];
	 p2[2]=&a3[0];
	 p2[3]=&a4[0];
	 p2[4]=&a5[0];
     p=&buffer[0];//PROD_VAR0_ADD+(PROD_VAR_SIZE*(MAXIMO_NUMERO_PROD-1))
     for(i=0;i<5;i++){
//		// AsignVarProdAndGetPointer(p,p2[i]);//vars Factory Settings to pointer
    	 NombreiProd_AND_defaultValues_to_buffer(p,p2[i]);
		 writeEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(iProd[i]-1)),PROD_VAR_SIZE,p);//escribimos variables de producto en cache
         } 
    
}//fin en init Products-------------------------------------------------------------------------


void getUserDefinedAux(unsigned char *p,unsigned char auxiliar){
unsigned char *pp,i,j,n;	
	  pp=readEEPROMblock(ESPACIO_B_ADD,ESPACIO_B_SIZE);//variables del sistema
	  if(auxiliar==1)
		  n=22;///direccion en byte dela memmoria 
	  else n=42;
	  for(i=n,j=0;i<NOMBRE_PRODUCTO_SIZE;i++,j++)	
	    	*(p+j)=*(pp+i);
}//fin getUserDefinedAux--------------------------------------------------------------------------------


/* METODO PRINCIPAL DE INICIALIZACION DE VARIABLES AL ENCENDER O REINICIAR EL EQUIPO
 * SACA LOS DATOS DE LA EEPROM  DE LA CACHE  Y LOS INSERTA EN LAS 
 * VARIABLES DE PRODUCTO DE LA RAM
 * */
void init_MemoVars(void){//init vars from the eeprom
unsigned char   *p,*p2,i;   
//Dword word32;    //disponemos de 128kbytes
//unsigned short int address;//tamaño de las variables del sistema
unsigned char buffer[PROD_VAR_SIZE];//buffer para escribir variables del sistema
unsigned char TestSetup[]={"1Test SetUp\n\0"};
unsigned char userdefined[]={"user defined      "};

//    indiceProducto=1;//producto Set TestUp, first run.
    modificado=NOP;//variable que indica que no se ha modfificado la configuracion en el menu
    realTimeError=CODE_ERROR_0000;//NO HAY ningun error
    
    //PRIMER VEZ QUE SE ENCIENDE LA PROCESADORA+++++++++++++++++++++++++++++++++++++++++++++++++
    if(readEEPROMbyte(STATUS_FACTORY_ADD)!=STATUS_FACTORY){//Es la primera vez que lo encienden?
   //ESPACIO A+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        indiceProducto=1;//producto Set TestUp, first run.
         writeEEPROMbyte2(STATUS_FACTORY_ADD,STATUS_FACTORY);//escribimos byte de FactoryReset
         if(readEEPROMbyte(STATUS_FACTORY_ADD)!=STATUS_FACTORY)
                realTimeError=CODE_ERROR_EEPROM_BYTE1;//debug mandar a controlador de erroress
         //Comenzamos a llenar Variables del Sistema POR PRIMERA VEZ
   //ESPACIO B++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
         
         p=&buffer[0];p2=p;
         *p=indiceProducto=1;                       //00H  Test SetUp
        // *(p+1) =;       //01h COMINEZAN variables en ORDEN de MEMORIA NO REVOLVER QUITAR O PONER¡¡¡¡
        // *(p+2) =       //02h variables en ORDEN de MEMORIA NO REVOLVER QUITAR O PONER¡¡¡¡
        // *(p+3) =;    //03h          
        // *(p+4) =;   //04h
        // *(p+5) =;    //05h variables en ORDEN de MEMORIA NO REVOLVER QUITAR O PONER¡¡¡¡
        // *(p+6) =';       //06h variables en ORDEN de MEMORIA NO REVOLVER QUITAR O PONER¡¡¡¡   
         *(p+7) =IDcomm='0';                        //07h
         *(p+8) =Alta=1;                            //08h devision es uno 
         *(p+9) =Media=1;                           //09h ultimo digito de la frecuencia seleccionada
         *(p+10)=Baja=1;                            //0AH
//         *(p+11)=CuentaProducto='0';                //0BH 
        // *(p+12)=CuentaProducto2='0';               //0CH
         *(p+13)=ControlContrasena2='0';            //0DH
         *(p+14)=ControlContrasena3='0';            //0Eh
         *(p+15)=ControlContrasena4='0';            //0FH
         *(p+16)=Control_de_usuario='0';            //10H MENU_ADMINISTRATIVO
         *(p+17)=Guardia='1';                       //11H MENU_ADMINISTRATIVO
         *(p+18)=tipo_deMaquina='P';              
         *(p+19)=errorAnalogoPerm=PERMITIR;         //17H  INGENIERIA GRACIDA2
         *(p+20)=ReporteProductoPerm=PERMITIR;
         *(p+21)=PSUIOconfig=PERMITIR;             //16H  INGENIERIA GRACIDA2
         *(p+22)=EntradasSistemaReg.taco='0'; //Permitir=NO  pulsosRechazo=OFF(nible high)
		 *(p+23)=EntradasSistemaReg.fotocel='0';
		 *(p+24)=EntradasSistemaReg.fotocelBloqueado=1;
		 *(p+25)=EntradasSistemaReg.rechazo='0';
		 *(p+26)=EntradasSistemaReg.rechazoBloqueado=1;
		 *(p+27)=EntradasSistemaReg.rechazoPruebasist_seg=0;
		 *(p+28)=EntradasSistemaReg.rechazoPruebasist_mseg=1;
		 *(p+29)=EntradasSistemaReg.producto='0';
		 *(p+30)=EntradasSistemaReg.productoBloqueado=1;
		 *(p+31)=EntradasSistemaReg.productoPruebasist_seg=0;
		 *(p+32)=EntradasSistemaReg.productoPruebasist_mseg=1;
		 *(p+33)=EntradasSistemaReg.aux1='0';
		 *(p+34)=EntradasSistemaReg.aux1Rele=0;
		 *(p+35)=EntradasSistemaReg.aux1Label=0;
		 *(p+36)=EntradasSistemaReg.aux1_seg=0;
		 *(p+37)=EntradasSistemaReg.aux1_mseg=1;
		 *(p+38)=EntradasSistemaReg.aux2='0';
		 *(p+39)=EntradasSistemaReg.aux2Rele=0;
		 *(p+40)=EntradasSistemaReg.aux2_seg=0;
		 *(p+41)=EntradasSistemaReg.aux2_mseg=1;
		 *(p+42)=EntradasSistemaReg.aux2Label=0;
		 *(p+43)=zoom.Zoom=97;
		
         		 
        		 
        		 
        						 
   //       >^<----EL ULTIMO NUMERO DEBE coincidir con SISTEM_VAR_SIZE+1
         //escribimos las variables del sistema bloque B
         writeEEPROMblock(SYSTEM_VAR_ADD,SYSTEM_VAR_SIZE,p);//escribimos variables del sistema
         p2=&TestSetup[0];
         NombreiProd_to_defaultValues_to_buffer(p,p2);
//         NombreiProd_AND_defaultValues_to_buffer(p,p2);
         writeEEPROMblock(ESPACIO_C_ADD,ESPACIO_C_SIZE,p);//escribimos variables de producto en cache 
         writeEEPROMblock(ESPACIO_D_ADD,ESPACIO_D_SIZE,p);//escribimos var de cache a TestSetUp SPACE
         //escribimos los password de default del sistema
         //password default seran Level1=1 Level2=2 Level3=3 Level4=5
#if (PROD_VAR_SIZE < NOMBRE_PRODUCTO_SIZE)
  #error " El tamaño de las constantes es Incorrecto NOMBRE_PRODUCTO_SIZE,PROD_VAR_SIZE"
#endif
         
         
         
		 for(i=0;i<NOMBRE_PRODUCTO_SIZE;i++){
			 buffer[i]=0;}//limpiamos las passwords
//			 PasswordLevel1[0]=0;
//			 PasswordLevel2[0]=0;
//			 PasswordLevel2[0]=0;
//			 PasswordLevel2[0]=0;}
         buffer[0]='1';//password factory default level1
         writeEEPROMblock(PASSWORD_LEVE1_ADD,PASSWORD_SIZE,p);//escribimos las pssword de default
		 buffer[0]='2';//password factory default level2
		 writeEEPROMblock(PASSWORD_LEVE2_ADD,PASSWORD_SIZE,p);
		 buffer[0]='3';//password factory default level5
         writeEEPROMblock(PASSWORD_LEVE3_ADD,PASSWORD_SIZE,p);
         buffer[0]='4';//password factory default level4
         writeEEPROMblock(PASSWORD_LEVE4_ADD,PASSWORD_SIZE,p);
         buffer[0]=0;//DEBUG JUST for debug
         p=&userdefined[0];
         writeEEPROMblock(AUX1_USERDEFINED_ADD,AUX1_USERDEFINED_SIZE,p);
         writeEEPROMblock(AUX2_USERDEFINED_ADD,AUX1_USERDEFINED_SIZE,p);
         
         //FIN ES LA PRIMERA VEZ QUE SE ENCIENDE
          }//if fin de es la primera vez que se enciende la procesadora
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+++ EL SISTEMA YA ESTA INIZIALIZADO++++++++++++++++++++++++++++++++++++++++++++++++++++++
    else{// El Sistema YA ESTA INIZIALIZADO DE ANTES., 
    //ESPACIO "B"  Leemos Variables del Sistema y las cargamos
    	
        p=&buffer[0]; 
        //Si esta seleccionado Test SetUp Mandamos el Producto 1 a cache
        p2=readEEPROMblock(ESPACIO_C_ADD,PROD_VAR_SIZE);
		downloadPointer2ProductVars(p);//Download  CACHE a las variables        
        p=readEEPROMblock(ESPACIO_B_ADD,ESPACIO_B_SIZE);//leemos variables del sistema de la EEPROM
        SacarVar_Espacio_B_to_systemVarRAM(p);//Metemos variables de system leida de EEPROM a las var RAM
        p=readEEPROMblock(ESPACIO_C_ADD,ESPACIO_C_SIZE);
        downloadPointer2ProductVars(p);//Download  CACHE a las variables  
        p=readEEPROMblock(PASSWORD_LEVE1_ADD,PASSWORD_SIZE);
        
    }//FIN else, SISTEMA YA SE INICIALIZO  ++++++++++++++++++++++++++++++++++++++++++++++++++++++
}//FIN INIT MEMO VARS+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//FIN INIT MEMO VARS+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//FIN INIT MEMO VARS+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//FIN INIT MEMO VARS+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    


//return TRUE: si la password ingresada no esta duplicada en algun otro espacio de passwords
unsigned char validatePasswords(unsigned char *Level,unsigned char *T){//validamos la password
unsigned char *p;
unsigned char i;	

    if((*T=='5')&&(*(T+1)==0)){//la clave es 5=servicio
    	*Level=5;
    	return FALSE;} 
    for(i=1;i<=4;i++){// cuatro niveles de password
       p=getPasswords(i); 
       if(compareString(T,p)){//si son iguales minimo una se rompe el for y regresara false todo
	        *Level=i;//el nivel de la password que coincidio,
    	    return FALSE;}} //regresa false, porque hay una igual, eso es malo poreso regresa false
return TRUE; //no hubo ninguna password igual------------    
}//fin validatePassword-------------------------------

//TRUE: una de las 4 passwords es valida, el pointer regresa que nivel 
//conicide la pass, de que nivel es la password.  *T=apunta al array donde guarda la password a comparar.
//FALSE: NO Coincide con ninguna password, *p=aqui se guarda a nivel se accedio
unsigned char validatePassword(unsigned char *p,unsigned char *T){
	if(validatePasswords(p,T))
	    return FALSE; //la password no coincide con ninguna guardada
	else 
		return TRUE; 
}//fin validatePassword-------------------------------------------------------------------------------



/*+++  getPasswords+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               (*ptr)      (*ptr+1)     (*ptr+2)
                                  |            |            |
            __________      ______v____________v____________v____________
 ptr------>|   *ptr   |--->|  *(*ptr)   |  *(*ptr+1)  |*(*ptr+2) |       |
           |__________|    |____________|_____________|__________|_______|
(ptr+1)--->| *(ptr+1) |     ____________ _____________ __________________
           |__________|--->|*(*(ptr+1)) |*(*(ptr+1)+1)|          |       |
           |          |    |____________|_____________|__________|_______|
           |__________|          ^             ^
                                 |             |
                             *(ptr+1)     *(ptr+1)+1
*/
//regresamos apuntador de una clave solamente seleccionada
unsigned char *getPasswords(unsigned char level){
	return(readEEPROMblock(getAddPassword(level),PASSWORD_SIZE));
}//fin getPasswords--------------------------------------------------------------------------------------------

void setPasswords(unsigned char level,unsigned char *p){
    writeEEPROMblock(getAddPassword(level),PASSWORD_SIZE,p);
}//fin setPasswords-----------------------------------------------


unsigned short int getAddPassword(unsigned char level){
unsigned short int add=0;   
	   switch(level){
		   case 1:add=PASSWORD_LEVE1_ADD;break;
		   case 2:add=PASSWORD_LEVE2_ADD;break;
		   case 3:add=PASSWORD_LEVE3_ADD;break;
		   case 4:add=PASSWORD_LEVE4_ADD;break;
		   default: __asm(Halt); }//debug error de software   		
return add;
}//fin getAddPassword-------------------------------------------------------



/*cesta funcion hace el cambio de producto hace operacines de memoria
 * salvando o no el cache
 * */
void cargarNuevoProducto(unsigned char iProd,unsigned char op){
unsigned char *p;
unsigned char buffer[PROD_VAR_SIZE];//buffer para escribir variables del sistema

    p=&buffer[0];//guardamos cache del priducto actual en su espacio  
 switch(op){
   case SAVE://y cambiamos al otro producto iProd
			   downloadProductVars2Pointer(p,&name[0]);//cargamos las variables de producto al buffer
	//		   p=readEEPROMblock(ESPACIO_C_ADD,ESPACIO_C_SIZE);//leemos espacio Cache
			   writeEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(indiceProducto-1)),PROD_VAR_SIZE,p);//escribimos espacio de producto con sus valores de cache
   case BORRAR:p=readEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(iProd-1)),PROD_VAR_SIZE);//sacamos el prodcuto que queremos trabajar
			   writeEEPROMblock(ESPACIO_C_ADD,ESPACIO_C_SIZE,p);//lo escribimos en la cache
			   p=readEEPROMblock(ESPACIO_C_ADD,ESPACIO_C_SIZE);//lo samos de cache
			   downloadPointer2ProductVars(p);//vaciamos los valores a las variables de producto
			   break;
   default:break;}
		 	
}//cargarNuevoProducto ------------------------------


//*Sacar los bytes de el Espacio B a las variableas del systema en la RAM
void SacarVar_Espacio_B_to_systemVarRAM(unsigned char *p){
	
	                      indiceProducto=*p;                  //00H  Test SetUp
	        //EntradasSistemaReg.aux1     =*(p+1);       //01h COMINEZAN variables en ORDEN de MEMORIA NO REVOLVER QUITAR O PONER¡¡¡¡
	        //EntradasSistemaReg.aux2     =*(p+2);       //02h variables en ORDEN de MEMORIA NO REVOLVER QUITAR O PONER¡¡¡¡
	        //EntradasSistemaReg.fotocel  =*(p+3);    //03h          
	        //EntradasSistemaReg.producto =*(p+4);   //04h
	        //EntradasSistemaReg.rechazo  =*(p+5);  //05h variables en ORDEN de MEMORIA NO REVOLVER QUITAR O PONER¡¡¡¡
	        //EntradasSistemaReg.taco     =*(p+6);     //06h variables en ORDEN de MEMORIA NO REVOLVER QUITAR O PONER¡¡¡¡  
	        IDcomm                      =*(p+7);  //07h
	        Alta                        =*(p+8);    //08h devision es uno 
	        Media                       =*(p+9);    //09h ultimo digito de la frecuencia seleccionada
	        Baja                        =*(p+10);    //0AH
	        CuentaProducto              =*(p+11);  //0BH 
	        ControlContrasena1          =*(p+12);  //0CH
	        ControlContrasena2          =*(p+13);  //0DH
	        ControlContrasena3          =*(p+14);  //0Eh
	        ControlContrasena4          =*(p+15);  //0FH
	        Control_de_usuario          =*(p+16);  //10H MENU_ADMINISTRATIVO
	        Guardia                     =*(p+17);      //11H MENU_ADMINISTRATIVO
	        tipo_deMaquina              =*(p+18);
	        errorAnalogoPerm            =*(p+19);
	        ReporteProductoPerm         =*(p+20);
	        PSUIOconfig                 =*(p+21);
 EntradasSistemaReg.taco                =*(p+22); //Permitir=NO  pulsosRechazo=OFF(nible high)
 EntradasSistemaReg.fotocel             =*(p+23);
 EntradasSistemaReg.fotocelBloqueado    =*(p+24);
 EntradasSistemaReg.rechazo                =*(p+25);
 EntradasSistemaReg.rechazoBloqueado       =*(p+26);
 EntradasSistemaReg.rechazoPruebasist_seg  =*(p+27);
 EntradasSistemaReg.rechazoPruebasist_mseg =*(p+28);
 EntradasSistemaReg.producto               =*(p+29);
 EntradasSistemaReg.productoBloqueado      =*(p+30);
 EntradasSistemaReg.productoPruebasist_seg =*(p+31);
 EntradasSistemaReg.productoPruebasist_mseg=*(p+32);
 EntradasSistemaReg.aux1                   =*(p+33);
 EntradasSistemaReg.aux1Rele               =*(p+34);
 EntradasSistemaReg.aux1Label              =*(p+35);
 EntradasSistemaReg.aux1_seg               =*(p+36);
 EntradasSistemaReg.aux1_mseg              =*(p+37);
 EntradasSistemaReg.aux2                   =*(p+38);
 EntradasSistemaReg.aux2Rele               =*(p+39);
 EntradasSistemaReg.aux2_seg               =*(p+40);
 EntradasSistemaReg.aux2_mseg              =*(p+41);
 EntradasSistemaReg.aux2Label              =*(p+42);
                                  zoom.Zoom=*(p+43);		
	             
}//SacarVar_Espacio_B_to_systemVarRAM---------------------------------------------------------


/*dEL Menu del Nuevo Producto
 * guardamos el nuevo producto con los valores estandard vaciamos el producto actual
  de la cache a su espacion Pn ó espacio TestSetUp si es iProd=1,     */
void saveNewProduct(unsigned char iProd){
unsigned char *p,*p2;
//Dword word32;    //disponemos de 128kbytes



       p=readEEPROMblock(CACHE_SPACE,PROD_VAR_SIZE);//leemos el cache=C
      //Guardar los valores del producto actual que esta en cache en su espacio Pn ó Espacio Test SetUp
      if(indiceProducto==1)//esta actualmete el TestSeptup puesto
          //pasar los valores de Cache a TestSetUP_Space C->D
          writeEEPROMblock(TEST_SETUP_SPACE,PROD_VAR_SIZE,p);//escribimos espacio D=TEstSEtUp  
      //fin el producto que estaba antes era el Test SetUp
      else//el seleccionado no es el SetUp es otro.
          saveCacheProductPlaceEEPROM();    //sacamos el cache y metermos a su espacio Pn  
      indiceProducto=iProd;//actualizamose el producto nuevo que ahora es el seleccionado
      p2=&Text[0];//vamosa darle el nombre a cache
//      AsignVarProdAndGetPointer(p,p2);//escribimos el nombre p2 y obtenemos los valores estndard *p de las var RAM de prod
      NombreiProd_AND_defaultValues_to_buffer(p,p2);
      writeEEPROMblock(CACHE_SPACE,PROD_VAR_SIZE,p);//escribimos el cache con los valores p     
      writeEEPROM_Pn(indiceProducto,p);//escribimos producto en espacion Pn 
      
    
}//fin de save ne product-------------------------------------------------

//Guarda las variables de producto al espacio de cache
void Guardar_Var_Prod_a_Cache(unsigned char *pp){
unsigned char *p,*p2;	   
unsigned char buffer[PROD_VAR_SIZE];//buffer para escribir variables del sistema

    p2=pp;//=&name[0];
    p=&buffer[0];
	downloadProductVars2Pointer(p,p2);
	writeEEPROMblock(ESPACIO_C_ADD,ESPACIO_C_SIZE,p);	 
}//Guardar_Var_Prod_a_Cache------------------------------------------------

/*Pasamos el espacio de Cache a su correspondiente espacio en memoria EEPROM
 * tomando en cuanta la actua lvariable de  iProduct seleccionado */
void Copy_Cache_to_SpaceProduct(unsigned char iProd){
unsigned char *p,buffer[PROD_VAR_SIZE];//buffer para escribir variables del sistema
    if(iProd==1)
    	return;
	p=&buffer[0];	
    p=readEEPROMblock(CACHE_SPACE,PROD_VAR_SIZE);//leemos el cache=C
	writeEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(iProd-1)),PROD_VAR_SIZE,p);
}//fin------------------------------------------------------------------

/* bajamos la memoria cache a las variables de producto */
void Descargar_Cache_to_Var_Product(void){
unsigned char *p,buffer[PROD_VAR_SIZE];//buffer para escribir variables del sistema
		p=&buffer[0];	
	    p=readEEPROMblock(CACHE_SPACE,PROD_VAR_SIZE);//leemos el cache=C
	    downloadPointer2ProductVars(p);
}//fin Descargar_Cache_to_Var_Product----------------------------------
	
/*cargamos el producto seleccionado y lo sacamos de la EEPROM y lo mandamos a la
 *  CACHE espacio */
void Cargar_SpaceiProduct_to_Cache(unsigned char iProd){
unsigned char *p,buffer[PROD_VAR_SIZE];//buffer para escribir variables del sistema
		p=&buffer[0];		
	    p=readEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(iProd-1)),PROD_VAR_SIZE);//leemos el cache=C
		writeEEPROMblock(CACHE_SPACE,PROD_VAR_SIZE,p);	
}//fin Cargar_SpaceiProduct_to_Cache-----------------------------------


void NombreString_to_nameProduct_Var(unsigned char *T){
//Asisgnacion de VAriables de Producto POrPRIMERA VEZ A ESPACIO CACHE
                        name[0] =*T=NAME_INIT;       //48H
                        name[1] =*(T+1);
                        name[2] =*(T+2);
                        name[3] =*(T+3);
                        name[4] =*(T+4);
                        name[5] =*(T+5);
                        name[6] =*(T+6);
                        name[7] =*(T+7);
                        name[8] =*(T+8);
                        name[9] =*(T+9);
                        name[10]=*(T+10);
                        name[11]=*(T+11);
                        name[12]=*(T+12);
                        name[13]=*(T+13);
                        name[14]=*(T+14);
                        name[15]=*(T+15);
                        name[16]=*(T+16);
                        name[17]=*(T+17);
                        name[18]=*(T+18);
                        name[19]=*(T+19);
}//---------------------------------------------------------------------------



/*esta funcion  asigna la variable de nombre a las variables de producto en ram
 * y asigna los valores default a las variables ram de producto y tambien obtiene los apuntadores 
 * de las variables para vaciarlas en algun lado*/
void NombreiProd_to_defaultValues_to_buffer(unsigned char *p,unsigned char *T){
uword word32;    //disponemos de 128kbytes
    
//Asisgnacion de VAriables de Producto POrPRIMERA VEZ A ESPACIO CACHE
                        *p=      name[0] =*T=NAME_INIT;       //48H
                        *(p+1)  =name[1] =*(T+1);
                        *(p+2)  =name[2] =*(T+2);
                        *(p+3)  =name[3] =*(T+3);
                        *(p+4)  =name[4] =*(T+4);
                        *(p+5)  =name[5] =*(T+5);
                        *(p+6)  =name[6] =*(T+6);
                        *(p+7)  =name[7] =*(T+7);
                        *(p+8)  =name[8] =*(T+8);
                        *(p+9)  =name[9] =*(T+9);
                        *(p+10) =name[10]=*(T+10);
                        *(p+11) =name[11]=*(T+11);
                        *(p+12) =name[12]=*(T+12);
                        *(p+13) =name[13]=*(T+13);
                        *(p+14) =name[14]=*(T+14);
                        *(p+15) =name[15]=*(T+15);
                        *(p+16) =name[16]=*(T+16);
                        *(p+17) =name[17]=*(T+17);
                        *(p+18) =name[18]=*(T+18);
                        *(p+19) =name[19]=*(T+19);
                        *(p+20) =GananciaDriver=ALTA;     //13H  GANANCIA
                        *(p+21) =GananciaAnaloga=ALTA;     //14H GANANCIA
                        *(p+22) =filtroConfig=NORMAL;             //15H  FILTRO
                        *(p+23) =BorrarContadores=APAGAR;
                        *(p+24) =phase=1;     
                        *(p+25) =phasefrac=5;         //1DH
                        *(p+26) =frecuenciaSeleccion=MEDIA;                           //1EH
                        *(p+27) =MarcarAltura='0';                      //1FH
                        *(p+28) =GananciaDDS='0'; 
                        *(p+29) =zoom.Zoom=99;               //29H    
                        *(p+30) =cuadritoDDS='0';             //2AH  
                         word32.wordx=Deteccion.Sensibilidad=25;// OOM_MAX_Y_99;
                         *(p+31)=word32.byte[0];
                         *(p+32)=word32.byte[1];
                         word32.wordx=Altura=32000;
                         *(p+33)=word32.byte[0];
                         *(p+34)=word32.byte[1];
//                         word32.wordx=frecuency=1000000;                //MERGE variable 32bits frecuency    
                         *(p+35)=word32.byte[0];
                         *(p+36)=word32.byte[1];
                         word32.wordx=PulsosTiempoRechazo=15;
                         *(p+37)=word32.byte[0];
                         *(p+38)=word32.byte[1];
                         word32.wordx=PulsosTiempoEspera=30;//rechazo espera de rechazo
                         *(p+39)=word32.byte[0];
                         *(p+40)=word32.byte[1];
                         *(p+41)=Ganancia=ALTA;
                         
                         
                         
                   //       >^<----EL ULTIMO NUMERO DEBE coincidir con PROD_VAR_SIZE+1
}//fin de asignar valores default a las variables de producto en ram y su nombre especifico y obitene pointer 
//de las varibles-



/*esta funcion  asigna la variable de nombre a las variables de producto en ram
 * y asigna los valores default a las variables ram de producto y tambien obtiene los apuntadores 
 * de las variables para vaciarlas en algun lado*/
void NombreiProd_AND_defaultValues_to_buffer(unsigned char *p,unsigned char *T){
uword word32;    //disponemos de 128kbytes
//Asisgnacion de VAriables de Producto POrPRIMERA VEZ A ESPACIO CACHE
                        *p=     *T=NAME_INIT;       //48H
                        *(p+1)  =*(T+1);
                        *(p+2)  =*(T+2);
                        *(p+3)  =*(T+3);
                        *(p+4)  =*(T+4);
                        *(p+5)  =*(T+5);
                        *(p+6)  =*(T+6);
                        *(p+7)  =*(T+7);
                        *(p+8)  =*(T+8);
                        *(p+9)  =*(T+9);
                        *(p+10) =*(T+10);
                        *(p+11) =*(T+11);
                        *(p+12) =*(T+12);
                        *(p+13) =*(T+13);
                        *(p+14) =*(T+14);
                        *(p+15) =*(T+15);
                        *(p+16) =*(T+16);
                        *(p+17) =*(T+17);
                        *(p+18) =*(T+18);
                        *(p+19) =*(T+19);
                        *(p+20) =GananciaDriver=ALTA;     //13H  GANANCIA
                        *(p+21) =GananciaAnaloga=ALTA;     //14H GANANCIA
                        *(p+22) =filtroConfig=NORMAL;             //15H  FILTRO
                        *(p+23) =BorrarContadores=APAGAR;
                        *(p+24) =phase=1;     
                        *(p+25) =phasefrac=5;         //1DH
                        *(p+26) =frecuenciaSeleccion=MEDIA;                           //1EH
                        *(p+27) =MarcarAltura='0';                      //1FH
                        *(p+28) =GananciaDDS='0'; 
                        *(p+29) =zoom.Zoom=99;               //29H    
                        *(p+30) =cuadritoDDS='0';             //2AH  
                         word32.wordx=Deteccion.Sensibilidad=ZOOM_MAX_Y_99;
                         *(p+31)=word32.byte[0];
                         *(p+32)=word32.byte[1];
                         word32.wordx=Altura=31234;
                         Deteccion.Altura=31234;
                         *(p+33)=word32.byte[0];
                         *(p+34)=word32.byte[1];
//                         word32.wordx=frecuency=1000000;                //MERGE variable 32bits frecuency    
                         *(p+35)=word32.byte[0];
                         *(p+36)=word32.byte[1];
                         word32.wordx=PulsosTiempoRechazo=15;
						  *(p+37)=word32.byte[0];
						  *(p+38)=word32.byte[1];
						  word32.wordx=PulsosTiempoEspera=30;//rechazo espera de rechazo
						  *(p+39)=word32.byte[0];
						  *(p+40)=word32.byte[1];
						  *(p+41)=Ganancia=ALTA;
//						  *(p+42)=EntradasSistemaReg.taco='0';
//						  *(p+43)=EntradasSistemaReg.fotocel='0';
//						  *(p+44)=EntradasSistemaReg.fotocelBloqueado=1;
//						  *(p+45)=EntradasSistemaReg.rechazo='0';
//					      *(p+46)=EntradasSistemaReg.rechazoBloqueado=1;
//					      *(p+47)=EntradasSistemaReg.rechazoPruebasist_seg=0;
//					      *(p+48)=EntradasSistemaReg.rechazoPruebasist_mseg=11;
//						  *(p+49)=EntradasSistemaReg.producto='0';
//						  *(p+50)=EntradasSistemaReg.productoBloqueado=1;
//						  *(p+51)=EntradasSistemaReg.productoPruebasist_seg=0;
//						  *(p+52)=EntradasSistemaReg.productoPruebasist_mseg=11;
//						  *(p+53)=EntradasSistemaReg.aux1='0';
//						  *(p+54)=EntradasSistemaReg.aux1Rele=NO_RELE;
//						  *(p+55)=EntradasSistemaReg.aux1_seg=0;
//						  *(p+56)=EntradasSistemaReg.aux1_mseg=11;
//						  *(p+57)=EntradasSistemaReg.aux2='0';
//					   	  *(p+58)=EntradasSistemaReg.aux2Rele=NO_RELE;
//					  	  *(p+59)=EntradasSistemaReg.aux2_seg=0;
//						  *(p+60)=EntradasSistemaReg.aux2_mseg=11;
						  //*(p+46)=
						 // *(p+46)=
						 // *(p+46)=
						  //*(p+46)=
						 // *(p+46)=
						 // *(p+46)=
																																  
                   //       >^<----EL ULTIMO NUMERO DEBE coincidir con PROD_VAR_SIZE+1
}//fin de asignar valores default a las variables de producto en ram y su nombre especifico y obitene pointer 
//de las varibles-


/*Se usa esta funcion para actualizar los valores de una lectura de la EEPROM
 * en las variables de producto
 * */
void downloadPointer2ProductVars(unsigned char *p){
uword word32;  
       name[0]            =*p;                   //48H
       name[1]            =*(p+1);               //49H
       name[2]            =*(p+2);             //4AH
       name[3]            =*(p+3);             //4BH
       name[4]            =*(p+4);             //4CH
       name[5]            =*(p+5);             //4DH
       name[6]            =*(p+6);             //4EH
       name[7]            =*(p+7);             //4FH
       name[8]            =*(p+8);             //50H
       name[9]            =*(p+9);             //51H
       name[10]           =*(p+10);            //52H
       name[11]           =*(p+11);             //53H
       name[12]           =*(p+12);              //54H
       name[13]           =*(p+13);              //55H       
       name[14]           =*(p+14);              //56H
       name[15]           =*(p+15);              //57H
       name[16]           =*(p+16);              //58H
       name[17]           =*(p+17);              //59H
       name[18]           =*(p+18);              //5AH
       name[19]           =*(p+19);              //5BH
       GananciaDriver     =*(p+20);     //13H  GANANCIA
       GananciaAnaloga    =*(p+21);     //14H GANANCIA
       filtroConfig       =*(p+22);             //15H  FILTRO
       BorrarContadores   =*(p+23);
       phase              =*(p+24);
       phasefrac          =*(p+25);         //1DH
       frecuenciaSeleccion=*(p+26);                           //1EH
       MarcarAltura       =*(p+27);                   //1FH
       GananciaDDS        =*(p+28); 
       zoom.Zoom          =*(p+29);              //29H  
       cuadritoDDS        =*(p+30);             //2AH  
       word32.byte[0]     =*(p+31);
       word32.byte[1]     =*(p+32);
       Deteccion.Sensibilidad=word32.wordx;
       word32.byte[0]     =*(p+33);
       word32.byte[1]     =*(p+34);
       Altura=word32.wordx;
       word32.byte[0]     =*(p+35);
       word32.byte[1]     =*(p+36);
	   word32.wordx=PulsosTiempoRechazo=15;
	   word32.byte[0]     =*(p+37);
	   word32.byte[1]	  =*(p+38);
	   word32.wordx=PulsosTiempoEspera=30;//rechazo espera de rechazo
	   word32.byte[0]     =*(p+39);
	   word32.byte[1]     =*(p+40);
	   Ganancia           =*(p+41);
	   	   
       
                           //disponible hasta el 42
       frecuency=word32.wordx;
                 //       >^<----EL ULTIMO NUMERO DEBE coincidir con PROD_VAR_SIZE+1  
}//fin del la descarg del apuntador en las varibles de producto actual----------------------

/* downloading the var product to the pointer to be saved */
void downloadProductVars2Pointer(unsigned char *p,unsigned char *p2){
uword word32;   
                *p     =*p2;//name[0];
                *(p+1) =*(p2+1);//name[1];            //49H
                *(p+2) =*(p2+2);            //4AH
                *(p+3) =*(p2+3);            //4BH
                *(p+4) =*(p2+4);            //4CH
                *(p+5) =*(p2+5);            //4DH
                *(p+6) =*(p2+6);            //4EH
                *(p+7) =*(p2+7);            //4FH
                *(p+8) =*(p2+8);            //50H
                *(p+9) =*(p2+9);            //51H
                *(p+10) =*(p2+10);            //52H
                *(p+11) =*(p2+11);              //53H
                *(p+12) =*(p2+12);              //54H
                *(p+13) =*(p2+13);              //55H
                *(p+14) =*(p2+14);              //56H
                *(p+15) =*(p2+15);              //57H
                *(p+16) =*(p2+16);              //58H
                *(p+17) =*(p2+17);              //59H
                *(p+18) =*(p2+18);              //5AH
                *(p+19) =*(p2+19);              //5BH
                *(p+20) =GananciaDriver;     //13H  GANANCIA
                *(p+21) =GananciaAnaloga;     //14H GANANCIA
                *(p+22) =filtroConfig;             //15H  FILTRO
              //  *(p+23) //=BorrarContadores; libre se paso a varible de sistema
                *(p+24) =phase;     
                *(p+25) =phasefrac;         //1DH
                *(p+26) =frecuenciaSeleccion;                           //1EH
                *(p+27) =MarcarAltura;                      //1FH
                *(p+28) =GananciaDDS; 
                *(p+29) =zoom.Zoom;               //29H   
                *(p+30) =cuadritoDDS;             //2AH  
                 word32.wordx=Deteccion.Sensibilidad;
                *(p+31)=word32.byte[0];
                *(p+32)=word32.byte[1];
                 word32.wordx=Altura;
                *(p+33)=word32.byte[0];
                *(p+34)=word32.byte[1];
//                 word32.wordx=frecuency;                //MERGE variable 32bits frecuency    
                *(p+35)=word32.byte[0];
                *(p+36)=word32.byte[1];
			     word32.wordx=PulsosTiempoRechazo=15;
			    *(p+37)=word32.byte[0];
			    *(p+38)=word32.byte[1];
			     word32.wordx=PulsosTiempoEspera=30;//rechazo espera de rechazo
			    *(p+39)=word32.byte[0];
			    *(p+40)=word32.byte[1];
			    *(p+41)=Ganancia=ALTA;
			    
                 //NO MOVER ESTOS NUMEROES DE OFFSET PORQUE DEJARA DE FUNCIONAR EEPROM guardar
           //       >^<----EL ULTIMO NUMERO DEBE coincidir con PROD_VAR_SIZE+1
}//fin de download product variable to pointer to be saved--------------------------------


void EEPROMmonitor(unsigned char issue){
    
	unsigned char issue1;
	issue1=issue;
           //meter el issue en una fifo
        //y monitorear NAME_INIT para descargar las variables  ala cache
    
}//fin activate eeprom monitor


//escribimos el apuntador en el espacion de memoria de producto indicado por iProd
void writeEEPROM_Pn(unsigned char iProd, unsigned char *p){
     if(iProd==0)
          return;
     writeEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(iProd-1)),PROD_VAR_SIZE,p); 
}//fin de escritura en el espacio Pn de productos  iProd=1 -> Settestup,


//Leemos todos los bytes de un producto en particular, desde el Pn=1(testsetup a Pn)
unsigned char *readEEPROM_Pn(unsigned char iProd){
unsigned long add;
    if(iProd==0)//test-setup es el producto numero 1 
         return;
    add=PROD_VAR0_ADD+(PROD_VAR_SIZE*(iProd-1));
    return (readEEPROMblock(add,PROD_VAR_SIZE));
}//fin de leer los bytes de un producto en particular--------------------------










void configModificado(unsigned char m){// encapsula la variable de configracion modificada  
    modificado=m;   //se modifico una configuracion del contexto m
}// fin controlador de configuracion modificada----------------------------------------------------- 


void MemoEEPROM_DDS(unsigned operacion,unsigned char contexto1,unsigned char mode){//con CONTROL DE DDS
     MemoEEPROM(operacion,contexto1);
     if(mode==RESET_DDS_PWG){}
        //init_DDS(0);//se cambia la frecuecia en la analoga
}//fin memor EEPROM para inizializar el DDS-PWG--------------------------------------------------------



/*  IMPORTANTE: EL NUMERO DE BYTES "_SIZE" TIENE QUE COINCIDIR CON EL NUMERO DE ESCRITURAS O POINTERS
 *   dar de alta en VFDmenu las constantes de la pantalla y el numero de size de vfdmenu debe conicidir con el 
 *   pointer de aqui arriba
 * */
void MemoEEPROM(unsigned operacion,unsigned char contexto1){//leer la eeprom y reatuarar valores a las variables  
unsigned char *p=0,*p2=0;//tamaño maximo de bytes a guardar
unsigned short int add=0; //direccion donde se va escribir o leer de determinado producto
unsigned short int iProd;
unsigned char buffer[PROD_VAR_SIZE];//buffer para escribir variables del sistema

 uword word32;
 word32.wordx=0;
//unsigned char dat[10]; // ESCRITURA Y LECTRURA ES POR PANTALLA  <<---NOTA!!!!!!!!!!!!
    //add=(unsigned short int)(ESPACIO_D_ADD+(indiceProducto)*PRODUCT_SIZE);
    iProd=(unsigned short int)arg0;
    p=&buffer[0];
    switch(contexto1){//ES POR PANTALLA NO POR VARIABLE INDIVIDUAL
    	
       case AJUSTE_DE_PRODUCTO:
    	                    if(operacion==SAVE){
    	                         if(phasefrac!=arg0)
    	                        	 writeEEPROMbyte2(ESPACIO_C_ADD+25,phasefrac);
    	                         if(phase!=arg1)
    	                        	 writeEEPROMbyte2(ESPACIO_C_ADD+24,phase);
    	                         if(Deteccion.Sensibilidad!=arg3){
    	                             word32.wordx=Deteccion.Sensibilidad;
                                     p=&word32.byte[0];
                                     writeEEPROMblock(ESPACIO_C_ADD+31,2,p);}
    	                         if(Altura!=arg7){
    	                        	 word32.wordx=Altura;
    	                        	 p=&word32.byte[0];
    	                        	 writeEEPROMblock(ESPACIO_C_ADD+33,2,p);}
    	                         if(MarcarAltura!=arg8)
    	                        	 writeEEPROMbyte2(ESPACIO_C_ADD+27,MarcarAltura);
    	                         if(Ganancia!=arg9)
    	                        	 writeEEPROMbyte2(ESPACIO_C_ADD+41,Ganancia);
    	                         if(frecuenciaSeleccion!=arg10)
    	                        	 writeEEPROMbyte2(ESPACIO_C_ADD+26,frecuenciaSeleccion);
                                 if(arg5==CAMBIAR){
                                	 //for(i=0;i<NOMBRE_PRODUCTO_SIZE;i++) name ya es el modificado
                                		 // name[i]=Text[i];//el nombre modificado se pasa al name
                                	 p=&name[0];
                                	 SaveNameCache(p);}//sacamos el nmbre de Text y lo pasamos a name
                                 }//fin if SAVE
    	                    break;
       case NUEVO_PRODUCTO:if(operacion==SAVE){
    	   	   	   	   	   	    p2=&Text[0];//nombre del prod nuevo
    	   	   	   	   	   	    if(indiceProducto==0)
    	   	   	   	   	   	    	__asm(Halt);//debug error de diseño de software
    	   	   	   	   	   	    if(indiceProducto>1){
    	   	   	   	   	   	    	Guardar_Var_Prod_a_Cache(&name[0]);//guardar variables de producto en cache
    	   	   	    				Copy_Cache_to_SpaceProduct(indiceProducto);}//pasar el cache  a su lugar de memoria
    	   	   	   	            NombreiProd_to_defaultValues_to_buffer(p,p2);//write NEW text iProd to name and iProdvar to buffer		
    	   	   	    			writeEEPROMblock(ESPACIO_C_ADD,ESPACIO_C_SIZE,p);//write new iProd to cache
    	   	                    writeEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(arg0-1)),PROD_VAR_SIZE,p);//write actual iProd to its memory space  	    			
                                   }//FIN IF-------------------------
    	                   	   	break;
       case SELECCIONAR_PRODUCTO:
    	   	   	   	   	   	 if(operacion==CAMBIAR){
     	   	   	   	   	   	    if(indiceProducto>1){
 	   	   	   	   	   	    	  Guardar_Var_Prod_a_Cache(&name[0]);//guardar variables de producto en cache
	   	   	    				  Copy_Cache_to_SpaceProduct(indiceProducto);}//pasar el cache  a su lugar de memoria
								p=readEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(arg0-1)),PROD_VAR_SIZE);
								writeEEPROMblock(ESPACIO_C_ADD,ESPACIO_C_SIZE,p);//write new iProd to cache
								downloadPointer2ProductVars(p); 
    	   	   	   	   	   	    }//cambiar fin 
    	   	   	   	   	   	    break;
       case CONFIGURAR_ENTRADAS_DE_SISTEMA:/*aqui va procedimiento para resturar*/ 
                            if(operacion==RESTAURAR){//restaurar, cambio la variable y se arrepintio
                                   p=readEEPROMblock(add,CONFIGURAR_ENTRADAS_DE_SISTEMA_SIZE);
                                   EntradasSistemaReg.aux1=*p;
                                   EntradasSistemaReg.aux2=*(p+1);
                                   EntradasSistemaReg.fotocel=*(p+2);          
                                   EntradasSistemaReg.producto=*(p+3);
                                   EntradasSistemaReg.rechazo=*(p+4);
                                   EntradasSistemaReg.taco=*(p+5);  
                                   I2CliberarModulo();}//ya pueden usar el modulo otros proc//fin config ent sistema restaurar  
                            else{  //SAVE seccion
                                   p=getPointerI2CBuffTx(CAT2401);
                                   *p=EntradasSistemaReg.aux1;
                                   *(p+1)=EntradasSistemaReg.aux2;
                                   *(p+2)=EntradasSistemaReg.fotocel;          
                                   *(p+3)=EntradasSistemaReg.producto;
                                   *(p+4)=EntradasSistemaReg.rechazo;
                                   *(p+5)=EntradasSistemaReg.taco;
                                   writeEEPROMblock(add,CONFIGURAR_ENTRADAS_DE_SISTEMA_SIZE,p);}
                              break;//fin SAVE
       case ID_COMUNICACIONES:if(operacion==RESTAURAR){/*IDcomm=0;aqui va procedimiento para resturar*/ 
                                    p=readEEPROMblock(add,ID_COMUNICACIONES_SIZE);
                                    IDcomm=*p;
                                    I2CliberarModulo();}//ya pueden usar el modulo}
                              else{p=getPointerI2CBuffTx(CAT2401);
                                   *p=IDcomm;
                                   writeEEPROMblock(add,ID_COMUNICACIONES_SIZE,p);}
                              break;
       case CONTROL_DE_FRECUENCIA://Alta=5;Media=6;Baja=0;/*aqui va procedimiento para resturar*/ break;//--en-desarollo
                              if(operacion==RESTAURAR){
                                    p=readEEPROMblock(add,CONTROL_DE_FRECUENCIA_SIZE);
                                    Alta=*p;
                                    Media=*(p+1);
                                    Baja=*(p+2);
                                    I2CliberarModulo();}//ya pueden usar el modulo}
                              else{p=getPointerI2CBuffTx(CAT2401);
                                   *p=Alta;
                                   *(p+1)=Media;
                                   *(p+2)=Baja;
                                   writeEEPROMblock(add,CONTROL_DE_FRECUENCIA_SIZE,p);
                                   //init_DDS(0);//NO DESCOMENTAR FALLA SAVE PRODUCT//se cambia la frecuecia en la analoga
                              }break;              

       case INFORMACION_DE_USUARIO: //CuentaProducto=APAGAR;break; 
                                  if(operacion==RESTAURAR){
                                      p=readEEPROMblock(add,INFORMACION_DE_USUARIO_SIZE);
                                      CuentaProducto=*p;
                                      I2CliberarModulo();}//ya pueden usar el modulo}
                                  else{p=getPointerI2CBuffTx(CAT2401);
//                                       *p=CuentaProducto;
                                       writeEEPROMblock(add,INFORMACION_DE_USUARIO_SIZE,p);}
                                  break;
       case CONTROL_PASSWORD://ControlContrasena1=NO;ControlContrasena2=NO;
                             //ControlContrasena3=NO;ControlContrasena4=NO;break;
                             if(operacion==RESTAURAR){
                                  p=readEEPROMblock(add,CONTROL_PASSWORD_SIZE);
                                  ControlContrasena1=*p;
                                  ControlContrasena2=*(p+1);
                                  ControlContrasena3=*(p+2);
                                  ControlContrasena4=*(p+3);
                                  I2CliberarModulo();}//ya pueden usar el modulo}
                             else{p=getPointerI2CBuffTx(CAT2401);
                                  *p=ControlContrasena1;
                                  *(p+1)=ControlContrasena2;
                                  *(p+2)=ControlContrasena3;
                                  *(p+3)=ControlContrasena4;
                                  writeEEPROMblock(add,CONTROL_PASSWORD_SIZE,p);}
                            break;
       case MENU_ADMINISTRATIVO://  Control_de_usuario='0';Guardia='1'; break;
                            if(operacion==RESTAURAR){
                                p=readEEPROMblock(add,MENU_ADMINISTRATIVO_SIZE);
                                Control_de_usuario=*p;
                                Guardia=*(p+1);
                                I2CliberarModulo();}//ya pueden usar el modulo}
                            else{p=getPointerI2CBuffTx(CAT2401);
                                 *p=Control_de_usuario;
                                 *(p+1)=Guardia;
                                 writeEEPROMblock(add,MENU_ADMINISTRATIVO_SIZE,p);}
                            break;
       case TIPO_DE_MAQUINA://  tipo_deMaquina=SINGLE_875_286; break;
                            if(operacion==RESTAURAR){
                                p=readEEPROMblock(add,TIPO_DE_MAQUINA_SIZE);
                                tipo_deMaquina=*p;
                                I2CliberarModulo();}//ya pueden usar el modulo}
                            else{p=getPointerI2CBuffTx(CAT2401);
                                 *p=tipo_deMaquina;      
                                 writeEEPROMblock(add,TIPO_DE_MAQUINA_SIZE,p); }
                            break;
       case GANANCIA_: //GananciaDriver=AJUSTABLE; GananciaAnaloga=MEDIA; break;
                           if(operacion==RESTAURAR){
                               p=readEEPROMblock(add,GANANCIA__SIZE);
                               GananciaDriver=*p;
                               GananciaAnaloga=*(p+1);
                               I2CliberarModulo();}//ya pueden usar el modulo}
                           else{p=getPointerI2CBuffTx(CAT2401);
                                *p=GananciaDriver;       
                                *(p+1)=GananciaAnaloga;
                                //writeEEPROM(p,add,GANANCIA__SIZE); 
                                init_AnalogGainControl();}//activar salidas fisicas hacia Analoga Board
                           break;
       case FILTRO_: //filtroConfig=NORMAL; break;
                         if(operacion==RESTAURAR){
                             p=readEEPROMblock(add,FILTRO__SIZE);
                             filtroConfig=*p;
                             I2CliberarModulo();}//ya pueden usar el modulo}
                        else{p=getPointerI2CBuffTx(CAT2401);
                             *p=filtroConfig;
                             writeEEPROMblock(add,FILTRO__SIZE,p);}            
                        break; 
       case INGENIERIA_GRACIDA2: //PSUIOconfig=PERMITIR; errorAnalogoPerm=PERMITIR; 
                                 //ReporteProductoPerm='0';  break;
                       if(operacion==RESTAURAR){
                           p=readEEPROMblock(add,INGENIERIA_GRACIDA2_SIZE);
                           PSUIOconfig=*p;
                           errorAnalogoPerm=*(p+1);
                           ReporteProductoPerm=*(p+2);
                           I2CliberarModulo();}//ya pueden usar el modulo
                       else{p=getPointerI2CBuffTx(CAT2401);
                            *p=PSUIOconfig;
                            *(p+1)=errorAnalogoPerm;
                            *(p+2)=ReporteProductoPerm;
                            writeEEPROMblock(add,INGENIERIA_GRACIDA2_SIZE,p);}
                       break;
       case AJUSTE_PARAMETRICO_DE_PRODUCTO: //Deteccion.Sensibilidad=20;
                                            //BorrarContadores=APAGAR; break;
                      if(operacion==SAVE){
                    	  if(Deteccion.Sensibilidad!=arg3){
                    		 *p=word32.byte[0];
							 *(p+1)=word32.byte[1];
							 writeEEPROMblock(ESPACIO_C_ADD+31,2,p);}
                    	  if(phasefrac!=arg0){
                    		 writeEEPROMbyte2(ESPACIO_C_ADD+25,phasefrac);}
                    	  if(phase!=arg1){
                    		  writeEEPROMbyte2(ESPACIO_C_ADD+24,phase);}
                          if(arg2==RESET){//se quiere resetear los contadores 
                        	  p=&buffer[0];
                        	  buffer[0]=0;buffer[1]=0;buffer[2]=0;buffer[3]=0;
                        	  writeNVRAMblock(ESPACIO_E_ADD,4,p);//borrar 4 bytes de ambos contadores
                              break;}}
                      break;
       case FRECUENCIA_SELECT: //frecuenciaSeleccion=MEDIA; frecuency=280000; break;
                      if(operacion==RESTAURAR){
                          p=readEEPROMblock(add,FRECUENCIA_SELECT_SIZE);
                          frecuenciaSeleccion=*p;
                          word32.byte[0]=*(p+1);
                          word32.byte[1]=*(p+2);
                          word32.byte[2]=*(p+3);
                          word32.byte[3]=*(p+4);
                          frecuency=word32.wordx;
                          I2CliberarModulo();}
                     else{p=getPointerI2CBuffTx(CAT2401);//escribir variables de frecuencia seleccion
                          *p=frecuenciaSeleccion;
//                          word32.wordx=frecuency;
                          *(p+1)=word32.byte[0];
                          *(p+2)=word32.byte[1];
                          *(p+3)=word32.byte[2];
                          *(p+4)=word32.byte[3];
                          writeEEPROMblock(add,FRECUENCIA_SELECT_SIZE,p);
                          //init_DDS(0); //NO DESCOMENTAR//se cambia la frecuencia del DDS xq se acepto guardar los cambios hechos
                    }break;
       case RELOJ_DE_SISTEMA: saveTimeNVRAM(); //año,mes,dia,fecha,hora,minutos,segundos;break; 
                            break;
       case PARAMETROS_DE_DETECCION://1<Altura<32000, MarcarAtura=No|Si
                      if(operacion==RESTAURAR){
                         p=readEEPROMblock(add,PARAMETROS_DE_DETECCION_SIZE);
                         word32.byte[0]=*p;
                         word32.byte[1]=*(p+1);
                         word32.byte[2]=*(p+2);
                         word32.byte[3]=*(p+3);
                         Altura=word32.wordx;//deprecated
                         MarcarAltura=*(p+4);
                         I2CliberarModulo();
                         p=readEEPROMblock(add,FRECUENCIA_SELECT_SIZE);
                         frecuenciaSeleccion=*p;
                         word32.byte[0]=*(p+1);
                         word32.byte[1]=*(p+2);
                         word32.byte[2]=*(p+3);
                         word32.byte[3]=*(p+4);
                         frecuency=word32.wordx;
                         I2CliberarModulo();
                         p=readEEPROMblock(add,AJUSTE_PARAMETRICO_DE_PRODUCTO_SIZE);
                         word32.byte[0]=*p;
                         word32.byte[1]=*(p+1);
                         word32.byte[2]=*(p+2);
                         word32.byte[3]=*(p+3);                       
                         Deteccion.Sensibilidad=word32.wordx;
                         BorrarContadores=*(p+4);
                         phase=*(p+5);
                         phasefrac=*(p+6);
                         I2CliberarModulo(); 
                         if(Altura>32000)
                                Altura=32000;
                         if(Deteccion.Sensibilidad>32000)
                                 Deteccion.Sensibilidad=32000;}
                      else{p=getPointerI2CBuffTx(CAT2401);
                            word32.wordx=Altura;
                           *p=word32.byte[0];
                           *(p+1)=word32.byte[1];
                           *(p+2)=word32.byte[2];
                           *(p+3)=word32.byte[3];
                           *(p+4)=MarcarAltura;
                           writeEEPROMblock(add,PARAMETROS_DE_DETECCION_SIZE,p);
                          p=getPointerI2CBuffTx(CAT2401);//traemos la dir del buffer de I2C tx
                          *p=frecuenciaSeleccion;
//                          word32.wordx=frecuency;
                         *(p+1)=word32.byte[0];
                         *(p+2)=word32.byte[1];
                         *(p+3)=word32.byte[2];
                         *(p+4)=word32.byte[3];
                         writeEEPROMblock(add,FRECUENCIA_SELECT_SIZE,p);
                         p=getPointerI2CBuffTx(CAT2401);
                         word32.wordx=Deteccion.Sensibilidad;
                         *p=word32.byte[0];
                         *(p+1)=word32.byte[1];
                         *(p+2)=word32.byte[2];
                         *(p+3)=word32.byte[3];           
                         *(p+4)=BorrarContadores;
                         *(p+5)=phase;
                         *(p+6)=phasefrac;
                         writeEEPROMblock(add,AJUSTE_PARAMETRICO_DE_PRODUCTO_SIZE,p);
                         //init_DDS(0); NO DESCOMENTAR//se cambia la frecuencia del DDS xq se acepto guardar los cambios hechos
                     }break;
       case PANTALLA_DDS:
                      if(operacion==RESTAURAR){
                         p=readEEPROMblock(add,PANTALLA_DDS_SIZE);
                         GananciaDDS=*p;
                         zoom.Zoom=*(p+1);
                         cuadritoDDS=*(p+2);
                         I2CliberarModulo();//para poder usar el i2c otravez
                         MemoEEPROM(RESTAURAR,AJUSTE_PARAMETRICO_DE_PRODUCTO);}
                      else{p=getPointerI2CBuffTx(CAT2401);
                           *p=GananciaDDS;
                           *(p+1)=zoom.Zoom;
                           *(p+2)=cuadritoDDS;
                           writeEEPROMblock(add,PANTALLA_DDS_SIZE,p);
                           word32.wordx=Deteccion.Sensibilidad;
                           *p=word32.byte[0];
                           *(p+1)=word32.byte[1];
                           *(p+2)=word32.byte[2];
                           *(p+3)=word32.byte[3];           
                           *(p+4)=BorrarContadores;
                           *(p+5)=phase;
                           *(p+6)=phasefrac;
                           writeEEPROMblock(add,AJUSTE_PARAMETRICO_DE_PRODUCTO_SIZE,p);
                           p=getPointerI2CBuffTx(CAT2401);
                           word32.wordx=Altura;
                           *p=word32.byte[0];
                           *(p+1)=word32.byte[1];
                           *(p+2)=word32.byte[2];
                           *(p+3)=word32.byte[3];
                           *(p+4)=MarcarAltura;
                           writeEEPROMblock(add,PARAMETROS_DE_DETECCION_SIZE,p);}
                      break;
          case NOMBRE_PRODUCTO:
                         if(operacion==RESTAURAR){
                                p=readEEPROMblock(add,NOMBRE_PRODUCTO_SIZE);
                                name[0]=*p;
                                name[1]= *(p+1);
                                name[2]= *(p+2);
                                name[3]= *(p+3);
                                name[4]= *(p+4);
                                name[5]= *(p+5);
                                name[6]= *(p+6);
                                name[7]= *(p+7);
                                name[8]= *(p+8);
                                name[9]= *(p+9);
                                name[10]=*(p+10);
                                name[11]=*(p+11);
                                name[12]=*(p+12);
                                name[13]=*(p+13);
                                name[14]=*(p+14);
                                name[15]=*(p+15);
                                name[16]=*(p+16);
                                name[17]=*(p+17);
                                name[18]=*(p+18);
                                name[19]=*(p+19);
                                I2CliberarModulo();}//para poder usar el i2c otravez
                          else{p=getPointerI2CBuffTx(CAT2401);
                                *p=name[0];
                                *(p+1) =name[1];
                                *(p+2) =name[2];
                                *(p+3) =name[3];
                                *(p+4) =name[4];
                                *(p+5) =name[5];
                                *(p+6) =name[6];
                                *(p+7) =name[7];
                                *(p+8) =name[8];
                                *(p+9) =name[9];
                                *(p+10)=name[10];
                                *(p+11)=name[11];
                                *(p+12)=name[12];
                                *(p+13)=name[13];
                                *(p+14)=name[14];
                                *(p+15)=name[15];
                                *(p+16)=name[16];
                                *(p+17)=name[17];
                                *(p+18)=name[18];
                                *(p+19)=name[19];
                                writeEEPROMblock(add,NOMBRE_PRODUCTO_SIZE,p);}
                        break;      
          case ERROR_LISTA://codigo incompleto
                        switch(operacion){//codigo incompleto
                          case RESTAURAR: //falta debug
                                         break;
                               
                          case CODE_ERROR_0001_ADC:
                          case CODE_ERROR_0002_BOOT:
                          case CODE_ERROR_0003_VFD:
                          case CODE_ERROR_0004_IO:
                          case CODE_ERROR_0005:   
                                               break;
                          default:break;
                        }//fin switch
                      if(operacion==RESTAURAR){//codigo incompleto
                        p=readEEPROMblock(0,0);  //falta debug
                          
                      }  
                      break;//fin ERROR_LISTA
         case PARTE_11:
                       init_AnalogGainControl();//actualiza las salidas para controlar la nueva Analoga
                       break;//fin PARTE 11
         case BORRAR_PRODUCTO:
//                      if(operacion==RESTAURAR)
//                          break;//no vamos a borrar Producto nada,
//                      else//vamos a borrar Producto
//                          writeEEPROMbyte2(PROD_VAR0_ADD+(PROD_VAR_SIZE*(igxm-1)),0);//se borra el bit é, para borrar el prod con esto 
//                      break;
         case COPY_PRODUCTO:
//                     if(operacion==RESTAURAR)//no queremos copiar nada
//                         break;
//                     else{//si estamos seguros de copiar el producto
//                          if(igxm==indiceProducto)//copiar el actual selected}
//						  downloadiProductVars2Cache();//bajamos las variables a la cache
//                          else 
//                              getiProductEEPROM2vars(igxm); //sacamos el iProduct  a las variables
//                          setVar2iProduct(igxm4);//guardamos las variables en el espacio seleccionado         
//                            break;}
                     break;//fin COPY_PRODUCTO
                      
       default:break;            
  }//fin switch contexto----------------------------------------------------------------------------    

}//fin restaurar contexto---------------------------------------------------------------------------


/*save the space CACHE  to the  spacE product Pn*/
void saveCacheProductPlaceEEPROM(void){
unsigned char *p;   
    p=readEEPROMblock(PROD_CACHE_VAR_ADD,PROD_VAR_SIZE);
    writeEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(indiceProducto-1)),PROD_VAR_SIZE,p);
       
    
}//fin de save CACHE memory in Product space Memoriy EEPROM

/*valida que el nombre seleccionado no se un espacio en blanco o que haya sido guardado con anterioridad */
unsigned char validarName(unsigned char iProduct){
unsigned char *p;   
         p=readEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(iProduct-1)),PROD_VAR_SIZE);//leemos el producto que queremos seleccionar
         if(*p==NAME_INIT)
              return TRUE;
         else return FALSE;  
         
}//fin de validar producto seleccionado para activar aver si no es solo espacios o nombre basura

unsigned char SaveNameCache(unsigned char *p){
         	
	    writeEEPROMblock(ESPACIO_C_ADD,NOMBRE_PRODUCTO_SIZE,p);
	
}// fin SaveNameCache---------------------------------------

/* sacamos el valor de las variables de producto del espacio Pn a las variables*/
void getiProductEEPROM2vars(unsigned char iProduct){
unsigned char *p;
unsigned long int add;
         add=PROD_VAR0_ADD+(PROD_VAR_SIZE*(iProduct-1));
         p=readEEPROMblock(add,PROD_VAR_SIZE);
         downloadPointer2ProductVars(p);
}//fin download the EEPROM vars product to PRODUCT vars


//mandar las variables de producto al Producto Pn 
void setVar2iProduct(unsigned char iProduct){
unsigned char buffer[PROD_VAR_SIZE],*p;
         p=&buffer[0];
//         downloadProductVars2Pointer(p);
         writeEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(iProduct-1)),PROD_VAR_SIZE,p);
}//fin de pasar las variables al producto Pn directamente---------------------------------


unsigned char *getiProductEEPROM(unsigned char iProduct){
         return(readEEPROMblock(PROD_VAR0_ADD+(PROD_VAR_SIZE*(iProduct-1)),PROD_VAR_SIZE));
}//fin download the EEPROM vars to pointer


/* bajamos las variables del producto a la area de CACHE EEPROM*/
void downloadiProductVars2Cache(void){
unsigned char *p;
unsigned char buffer[PROD_VAR_SIZE];//buffer para escribir variables del sistema
          p=&buffer[0];
//          downloadProductVars2Pointer(p);
          writeEEPROMblock(PROD_CACHE_VAR_ADD,PROD_VAR_SIZE,p); 
}///fin del methodo que descarga las variables de PRoducto ala CACHE eeprom


unsigned char  *memoEEPROM_SelProd(unsigned char iProduct){
//unsigned char *p;//tamaño maximo de bytes a guardar
//unsigned long int add; //direccion donde se va escribir o leer de determinado producto 
//static unsigned char name1[NOMBRE_PRODUCTO_SIZE]; //debug revisar si debe ser static
//unsigned char contexto1=NOMBRE_PRODUCTO;
      //  addressVar=(unsigned long int)(contexto1+(iProduct-1)*PRODUCT_SIZE);
        
            return (readEEPROM_Pn(iProduct));
 //return &name1[0];           
}//leer el nombre del producto del indice dado-------------------------------------------------------
        

/*Esta funcion me va a dar el indice del espacio vacio para poner un nuevo producto, y vamos a buscar del
 * indice actual hacia arriba o hacia abajo  y regresamos el indice para 
 * */
unsigned char  memoEEPROM_SearchPlace(unsigned char iniProd){
unsigned char *p;//tamaño maximo de bytes a guardar
//unsigned long int addressVar; //direccion donde se va escribir o leer de determinado producto 
unsigned char name1[NOMBRE_PRODUCTO_SIZE];  
unsigned char contexto1=NOMBRE_PRODUCTO;//buscamos el nombre de producto
unsigned char iProduct;
      //de cual iProduct empezamos
     for(iProduct=iniProd;iProduct<CANTIDAD_PRODUCTOS+1;iProduct++){        
            p=getiProductEEPROM(iProduct);
            name1[0]=*p;
            if(name1[0]!=NAME_INIT)
                   return iProduct;}
 return (CANTIDAD_PRODUCTOS+1);//si pasa esto es porque no hay espacio para mas productos           
}//leer el nombre del producto del indice dado-------------------------------------------------------

unsigned char  memoEEPROM_SearchPlaceUP(unsigned char iniProd){
unsigned char *p;//tamaño maximo de bytes a guardar
//unsigned long int addressVar; //direccion donde se va escribir o leer de determinado producto 
unsigned char name1[NOMBRE_PRODUCTO_SIZE];  
unsigned char contexto1=NOMBRE_PRODUCTO;//buscamos el nombre de producto
unsigned char iProduct;
      //de cual iProduct empezamos
     if(iniProd==2)//es el segundo iProd
         return 0;  //que se vaa a a la X
     for(iProduct=iniProd-1;iProduct>1;iProduct--){     
            p=getiProductEEPROM(iProduct);
            name1[0]=*p;
            if(name1[0]!=NAME_INIT)
                 return iProduct;}
 return 0;//si pasa esto es porque no hay espacio para mas productos           
}//leer el nombre del producto del indice dado-------------------------------------------------------




//igual que restaurar. restaura las variables en la eeprom
//unsigned char getEntradadeTaco(void){return(EntradadeTaco); }//fin get entrada de tacho porque el compilador no agarra las variables extern
//void setEntradadeTaco(unsigned char n){EntradadeTaco=n;}
//unsigned char getmodificado(void){return modificado;}
//void setmodificado(unsigned char m){ modificado=m;}

void initMemoria_y_Restaurar(void){//inizializar memoria y restaurar a estado de fabrica
    
    /*  CODIGO EN DESARROLLO */
    
    
}// fin init memoria y restaurar----------------------------------------------------

//variabes encasuladas en el methodo --------------
/*
unsigned char getEntradasSistemaReg(unsigned char v){
         switch(v){ 
            case TACO: return(EntradasSistemaReg.taco);break;
            case FOTOCEL:return(EntradasSistemaReg.fotocel);break;
            case RECHAZO:return(EntradasSistemaReg.rechazo);break;
            case PRODUCTO:return(EntradasSistemaReg.producto);break;
            case AUX1:return(EntradasSistemaReg.aux1);break;    
            case AUX2:return(EntradasSistemaReg.aux2);break;    
}//get char entradas sistema registro----------------------------------------------
         
void setTaco(unsigned char n){EntradasSistemaReg }       
*/


void init_ErrorsLogs(void){
unsigned char   *p; 
//Dword word32;    //disponemos de 128kbytes
    modificado=NOP;//variable que indica que no se ha modfificado la configuracion en el menu
    p=readEEPROMblock(0,LASTADDRESS_VARS_EEPROM);
    
}// initizializar errors logs--------------------------------------------------------------------------

void init_Semaforos(void){
     //SEMdisplayEEPROM=VERDE+1;//en rojo
      Sem.operacion=VERDE; 
    
}//fin inizializar los semaforos de los hilos


//funciones que se ejecutan en el hilo principal una vez cuando alguien las activa
void ControladorOperativoPrincipal(void){ 
    
     if(Sem.operacion==READ_EEPROM_DISPLAY){//se ejecuta la funcion de despliegue de la EEPROM
          keypad.Status.bits.enable=FALSE;// disable_keyboard_IRQ();
          Sem.operacion=ROJO; //ejecutar el despliegue de los datos leidos
          //Sem.p=readEEPROMbyte(10);
          Sem.operacion=DISPLAY_EEPROM;}
          
          
}//fin del controlador operativo principal de funciones que se e---------------------------


//para ejecutar h se ejecuta timer1
void ControladorOperativoTemporizado_IRQ(void){
    
//  if((TM1_IRQ1&0x00)==0x00){//num no calcularo yet
//	  TM1_IRQ1&=0xFE;
//	 switch(Sem.operacion){
//		case DISPLAY_EEPROM:Sem.operacion=ROJO;
//							displayEEPROMdata1(Sem.p);
//							enable_keyboard_IRQ();
//							Sem.operacion=VERDE;
//							break;// fin DISPLAY_EEPROM
//		default:break;
//		  }//fin switch principal
//  }//tm1 fin

}//fin del controlador Operativo Temporizado--------------------------------------------

//MEMORIA ES DE 128Kbytes=1F400h
//leemos si solo esta el producto de TestSetUp o hay mas productos
//true si solo esta guardado el producto TestSetUp
unsigned char EEPROMreadProductsCantidad(void){
unsigned short int i;	
	   for(i=PRODUCTO1_ADD;i<MAXIMO_NUMERO_PROD+1;i+=PROD_VAR_SIZE){
		    if(EEPROMreadByte(i)==NAME_INIT)
	                   return FALSE;}
return TRUE;	
}//fin EEPROMreadProductsCantidad------------------------------------

//MEMORIA ES DE 128Kbytes=1F400h
//le un solo byte de la direccion dada en la EEPROM
unsigned char EEPROMreadByte(unsigned short int add){
	         return(DebugVirtualEEPROMread(add));      
}//fin unsigned char EEPROMreadByte(unsigned char add){----------------------


//MEMORIA ES DE 128Kbytes=1F400h
//SOLO PARA LEER DATOS NO REPORTES, por eso add no es long
unsigned char DebugVirtualEEPROMread(unsigned short int add){
static unsigned char c,*p;
	           p=DebugVirtualEEPROM(READ,add,0,1);
	           c=*p;
	           return c;
}//fin DebugVirtualEEPROMread-------------------------------------------------

//MEMORIA ES DE 128Kbytes=1F400h
unsigned char readEEPROMbyte(unsigned long int add){
unsigned short int i;
    i=(unsigned short int)add;
	return(DebugVirtualEEPROMread(i));    
}//fin de la escritura por bloques del driver de escritura EEPROM+++++++++++++++++++++++++++


//MEMORIA ES DE 128Kbytes=1F400h  SOLO PARA LEER DATOS NO REPORTES, por eso add no es long
//lectura de blocque de la memoria EEPRIM VIRTUAL.
unsigned char *DebugVirtualEEPROMreadBlock(unsigned short int add,unsigned short int nbytes){
	
	     return (DebugVirtualEEPROM(READ,add,0,nbytes));
	
}//fin DebugVirtualEEPROMreadBlock---------------------------------------------


//MEMORIA ES DE 128Kbytes=1F400h
unsigned char *readEEPROMblock(unsigned long int add,unsigned char buffSize){
//static unsigned char buffer[PROD_VAR_SIZE],j;
unsigned short int i;
           
           i=(unsigned short int)add;
           return(DebugVirtualEEPROMreadBlock(i,buffSize));
           
}//fin de la escritura por bloques del driver de escritura EEPROM+++++++++++++++++++++++++++




//MEMORIA ES DE 128Kbytes=1F400h
//SOLO PARA LEER DATOS NO REPORTES, por eso add no es long
void DebugVirtualEEPROMwrite(unsigned short int add,unsigned char *w,unsigned short int nbytes){
	DebugVirtualEEPROM(WRITE,add,w,nbytes);
}//fin DebugVirtualEEPROMwrite-----------------------------------------------

void writeEEPROMbyte2(unsigned long int add,unsigned char byte){
unsigned short int i;
unsigned char c;
    c=byte;
    i=(unsigned short int)add;
	DebugVirtualEEPROMwrite(i,&c,1);
}//fin writeEEPROMbyte2------------------------------------------------------


//
//MEMORIA ES DE 128Kbytes=1F400h
void writeEEPROMblock(unsigned long int add,unsigned char buffSize,unsigned char *p){
unsigned short int i;
          i=(unsigned short int)add;
	      DebugVirtualEEPROMwrite(i,p,buffSize);
}//fin de la escritura por bloques del driver de escritura EEPROM+++++++++++++++++++++++++++

//56 bytes de tamaño apartir de 8h
void writeNVRAMblock(unsigned char add,unsigned char size,unsigned char *p){
	
	    DebugVirtualNVRAM(WRITE,add,p,size);
	
} //fin writeNVRAMblock-------------------------------------------------------------------


//MEMORIA ES DE 128Kbytes=1F400h
//SOLO PARA LEER DATOS NO REPORTES, por eso add no es long
//simula la memoria EEPROM  dentro del procesador.
//op= operacion READ|WRITE add:address donde empieza, *w:apunta al array que va esribir, el char
//nbytes: numero de bytes que va ejecutar la operacion
//regresa un apuntador donde estan guardados los datos de la memoria EEPROM.
unsigned char *DebugVirtualEEPROM(unsigned char op,unsigned short int add,unsigned char *w, unsigned short int nbytes)
{
unsigned short int i;	      
static unsigned char c[MAXIMO_NUMERO_PROD];

         if(nbytes>SIZE_EEPROM)//no podemos leer mas del tamaño de la eeprom o buffer virtual
        	 __asm(Halt);
         if((op==WRITE)&& (w==0))//proteccion contra errores, apuntadores nulos
        	  __asm(Halt);
         if(add>PASSWORD_LEVE1_ADD-2){//se quiere acceder al espacio de las PASSWORDS
        	 if(add<(PASSWORD_LEVE4_ADD+PASSWORD_SIZE+1))
        	      return(DebugVirtualEEPROMpasswords(op,add,w,nbytes));
        	  else __asm(Halt);}
         else{
        	 if(add==AUX1_USERDEFINED_ADD)//se qiuere acceder a aux1 user defined
                 return(DebugVirtualEEPROMaux_userdef(op,add,w,nbytes));
        	 if(add==AUX2_USERDEFINED_ADD)//se qiuere acceder a aux1 user defined
        	     return(DebugVirtualEEPROMaux_userdef(op,add,w,nbytes)); 		 
			 if(add>SIZE_EEPROM){//proteccion contra leer mas de lo que hay
				  for(i=0;i<MAXIMO_NUMERO_PROD;i++)//para menu NUEVO PROD, BORRAR PROD, ETC
						c[i]=0xFF; 
				  return &c[0];}}
	     if(op==READ)
	    	 return &EEPROM_1Mb[add];
	     if(op==WRITE)
	    	 for(i=add;i<(add+nbytes);i++,w++)
	    		 EEPROM_1Mb[i]=*w;	
return 0;	     
} //fin de memoeria en debug virtual EEPROM------------------------------------


//SIMULA LA MEMORIA EEPROM EN EL ESPACIO DE LAS PASSWORDS
unsigned char *DebugVirtualEEPROMpasswords(unsigned char op,unsigned short int add,unsigned char *w,
		unsigned short int nbytes){
static const unsigned short int SIZE_EEPROM=1000;
static unsigned char EEPROM2[SIZE_EEPROM];	
unsigned short int i,j,k;

         if(add<(PASSWORD_LEVE1_ADD-1))
        	 __asm(Halt);//debug error de software
         k=PASSWORD_LEVE1_ADD;
         j=add-k;//hacemos el oofset de  8630d a 0d
         if(op==READ)
	    	 return &EEPROM2[j];
	     if(op==WRITE)
	    	 for(i=j;i<(j+nbytes);i++,w++)
	    		 EEPROM2[i]=*w;	
}//fin de unsigned char *DebugVirtualEEPROMpasswords(unsigned char op,unsigned short int add,unsigned char *w,



unsigned char *DebugVirtualNVRAM(unsigned char op,unsigned short int add,unsigned char *w,
		unsigned short int nbytes){
static const unsigned short int SIZE_EEPROM=56;
static unsigned char NVRAM[SIZE_EEPROM];	
unsigned short int i;	      

         if(nbytes>SIZE_EEPROM)//no podemos leer mas del tamaño de la eeprom o buffer virtual
        	 __asm(Halt);
         
         if((op==WRITE)&& (w==0))//proteccion contra errores, apuntadores nulos
        	  __asm(Halt);
         
         if(add>0x3F)
        	 __asm(Halt);
         if(add<8)
        	   return 0;
	     if(op==READ)
	    	 return &NVRAM[add];
	     if(op==WRITE)
	        for(i=add;i<(add+nbytes);i++,w++)
	    		 NVRAM[i]=*w;	
return 0;	     
} //fin de memoeria en debug virtual EEPROM------------------------------------


/* escribe el nombre de user defined de entradas de auxiliar
 * n: es el 1|2, 
 *  *text es donde se guarda el texto que se ha modificado */
unsigned char writeAUXuserdefined(unsigned char n, unsigned char *text){
  if(n==1)
	 writeEEPROMblock(AUX1_USERDEFINED_ADD,AUX1_USERDEFINED_SIZE,text);
  else
     writeEEPROMblock(AUX2_USERDEFINED_ADD,AUX1_USERDEFINED_SIZE,text);
}//fin writeAUXuserdefined

unsigned char *readAUXuserdefined(unsigned char n){
  if(n==1)
	  return(readEEPROMblock(AUX1_USERDEFINED_ADD,AUX1_USERDEFINED_SIZE));
  else
	  return(readEEPROMblock(AUX2_USERDEFINED_ADD,AUX1_USERDEFINED_SIZE));
}//fin writeAUXuserdefined-----------------------------------------------------


unsigned char *DebugVirtualEEPROMaux_userdef(unsigned char op,unsigned short add,unsigned char *w,unsigned short int nbytes){
static const unsigned short int SIZE_EEPROM3=NOMBRE_PRODUCTO_SIZE+NOMBRE_PRODUCTO_SIZE+2;
static unsigned char NVRAM3[SIZE_EEPROM3];	
unsigned short int i,add1;	      
	   
         add1=add-AUX1_USERDEFINED_ADD;
         if(op==READ)
	    	 return &NVRAM3[add1];
	     if(op==WRITE)
	        for(i=add1;i<(add+nbytes);i++,w++)
	    		 NVRAM3[i]=*w;	
}//fin DebugVirtualEEPROMaux_userdef----------------------------------------------


void MEMORIA_VIDEO_DDS_clear(void){
//unsigned char i,j;	
//	for(i=0;i<VIDX;i++)
//		for(j=0;j<VIDY;j++)
//			VIDEO_DDS[i][j]=0; 
}//fin MEMORIA_VIDEO_DDS_clear-----------------------------------------------------



/*buscamos si el pixel que queremos desplegar no se ha desplegado ya
 * las direcciones son char las 2 porque el display es 512x128=ventala de 192x128
 * x:coordenas x de DDS del pixel a desplegar
 * y:coordenada y de DDS del pixel a desplegar
 * reg:es el registro donde reside el pixel que queremos consultar
 * lo: es el valor minimo del registro del pixel a consultr e.g. pixel=124  min=120, registro=15 max=min+8*/
unsigned char isDDS_MemoriaVideo_emptyPixel(unsigned char x,unsigned char y,unsigned char *regx,unsigned char *regy,unsigned char *lo){
//unsigned char regxx,regyy,loo;
	   if(dds.Bnderas.bit.EventDDS==TRUE){//DDS1.Banderas.bit.EventDDS==TRUE){//.EventDDS==TRUE){//SE EJEcuta cuando estamos en la pantalla DDS  
             if(DDS_MemoVid_getPixel(x,y,regx,regy,lo))
                  return FALSE;//SI EL pixel esta lleno con '1
             else { return TRUE;}}//TRUE: el bit pixel estaba en '0' pero ya no
return FALSE;
}//isDDS_MemoriaVideo_emptyPixel--------------------------------------------------


/* obtenemo el valor del pixel seleccionado, guardado en la memoria de VideoDDS
 * x: valor pixelDDS de 0 a 192 para graficar puntos
 * y: valor pixelDDS de 0 a 128 para grficar puntos
 * reg:es el registro donde reside el pixel que queremos consultar
 * lo: es el valor minimo del registro del pixel a consultr e.g. pixel=124  min=120, registro=15 max=min+8
 * ret: 1:true,  0:false*/
unsigned char DDS_MemoVid_getPixel(unsigned char x, unsigned char  y,unsigned char *regx,unsigned char *regy,unsigned char *lo1){
unsigned char x1,y1,xlo,xhi,ylo,yhi,reg,ret; 
	  x1=DDS_MemoVid_RegistrosPixel(x,&xlo,&xhi);//sacar el registro donde esta el bit susodicho
	  y1=DDS_MemoVid_RegistrosPixel(y,&ylo,&yhi);
	  if(isWrongPixel(x,xlo,xhi))//Debug esta funcion existe mientras se valida el software	  
	        __asm(Halt);//debug  error de software
	  if(isWrongPixel(y,ylo,yhi))//Debug esta funcion existe mientras se valida el software	  
	        __asm(Halt);//debug  error de software	  
//	  reg=VIDEO_DDS[x1][y1]; //desconcentracion->//DDS_MemoVid_RegistrosPixel(x,&xlo,&xhi)][DDS_MemoVid_RegistrosPixel(y,&ylo,&yhi)];
	  *regx=x1;//lo guadamos por si se usa para modificarlo
	  *regy=y1;
	  *lo1=xlo;//lo guardamos por si se usa para mdificarlo
	  reg=DDSmap[x1][y1];//sacamos el registro donde esta el pixel
	  if(setPixel(&reg,xlo,x))
		  ret=TRUE;
	  else ret=FALSE;
	  DDSmap[x1][y1]=reg;
      return ret;
	  
}//fin DDS_MemoVid_getPixel-------------------------------------------------

/*this method saves the bit pixel into video memory, reisters=24x16 registers
 * reG:REGISTER where w'll be saved the bit pixel
 * lo: the register lsb, represent number, e.g: reg15 ,lo: number nimimo, 
 * x: the number to be saved represented by a bit, e.g number 124, in regiter 15, lo=120, then bit 3 o 4 will be*/
void setDDS_MemoriaVideo_bitPixel(unsigned char *regx,unsigned char  *regy,unsigned char *lo, unsigned char x){
//unsigned char reg1;
//unsigned char bits[]={1,2,4,8,16,32,64,128};//2 exponente, el exponente es el bit resultante de la resta

//        reg1=VIDEO_DDS[*regx][*regy];//sacamos el registro donde esta el bit
//        VIDEO_DDS[*regx][*regy]=reg1 | bits[x-*lo];//resta y encendido del bit
	     *regx=*regy=*lo=x;
}//fin setDDS_MemoriaVideo_bitPixel-----------------------------------------------------


/* obtenemos el estado del pixel dentro del registro que sacamos
 * registro, es el registro que zacamos con coredenadas x,y
 * x: es el bit en al ordenada x que queremo revisar
 * lo: es el limite inferior del bit del registro, la coordenada que del registro de pixeles
 * TRUE | FALSE : true si el pixel esta prendido
 *  */
unsigned char setPixel(unsigned char *registro1,unsigned char lo,unsigned char x){
unsigned char i,registro;    
	
    registro=*registro1;
	i=x-lo;
	if(i>7)
		__asm(Halt);//debug error de software
	switch(i){
		case 0:if((registro&0x01)==0x01)
			        return TRUE;
			   else {*registro1=registro | 0x01;//prendemos el pixel
				     return FALSE;}
		       break;
		case 1:if((registro&0x02)==0x02)//0000 0010
	        		return TRUE;
		       else {*registro1=registro | 0x02;//prendemos el pixel
				    return FALSE;}
			   break;
		case 2:if((registro&0x04)==0x04)//0000 0100
					return TRUE;
		       else {*registro1=registro | 0x04;//prendemos el pixel
				    return FALSE;}
			   break;
		case 3:if((registro&0x08)==0x08)//0000 1000
					return TRUE;
		       else {*registro1=registro | 0x08;//prendemos el pixel
				    return FALSE;}
			   break;
		case 4:if((registro&0x10)==0x10)//0001 0000
					return TRUE;
		       else {*registro1=registro | 0x10;//prendemos el pixel
		            return FALSE;}
			   break;
		case 5:if((registro&0x20)==0x20)//0010 0000
					return TRUE;
		       else {*registro1=registro | 0x20;//prendemos el pixel
				    return FALSE;}
			   break;
		case 6:if((registro&0x40)==0x40)//0100 0000
					return TRUE;
		       else {*registro1=registro | 0x40;//prendemos el pixel
				    return FALSE;}
			   break;
		case 7:if((registro&0x80)==0x80)//1000 0000
					return TRUE;
			   else {*registro1=registro | 0x80;//prendemos el pixel
			        return FALSE;}
			   break;
		default:__asm(Halt);//debug error de software
			     break;}
}//fin de get pixel-------------------------------------------------------


//VEMOS si el regitro es indeed el registro que tiene los limites que neesitamos
unsigned char isWrongPixel(unsigned char x1,unsigned char low,unsigned char hi){
	if(x1==low)
		return FALSE;
	if(x1==hi)
		return FALSE;
	if(x1>low){
		if(x1<hi)
			return FALSE;}
	else return TRUE;
}//fin isWorngPixel---------------------------------------------------------




/* VERSION 1:Busca el registro  de Memoria Video DDS Pixel, 32 registros en x y 16 registros en y guardan los bits del estatus de cada pixel
 * del DDS, status actual en el zoom actual.
   Los registros gurdan el bit de cada pixel desplegado en la pantalla 
   registros0, 1, 2 guardan cada uno un byte o sea 8 bits de 8 pixels y si esta ON|OFF
   x: valor de pixelDDS de 0 a 192 ó de 0 a 128, son coord de pantalla centro =Cx,Cy.
   lo: es el valor minimo del registro seleccinado  que vamos a sacar y ver el pixel
   hi: es el valor maximo del regitro el valor decimal.
   si el registro es el registro cero entons tendra  minimo=0,1,2,3,4,5,6,7=maximo valor
   VERSION2:*/
unsigned char DDS_MemoVid_RegistrosPixel(unsigned char x,unsigned char *lo,unsigned char *hi){       
//unsigned char n,m,l,h,r;	
  	
    
           	
       switch(x){
		    case 0:
		    case 1:
		    case 2:
		    case 3:
		    case 4:
		    case 5:
		    case 6:
		    case 7:*lo=0;*hi=7;return 0;break;//registro 0,
		    case 8:
		    case 9:
		    case 10:
		    case 11:
			case 12:
			case 13:
			case 14:
			case 15:*lo=8;*hi=15;return 1;break; //registro 1
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
			case 22:
			case 23:*lo=16;*hi=23;return 2;break; //registro 1
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:
			case 30:
			case 31:*lo=24;*hi=31;return 3;break; //registro 1
			case 32:
			case 33:
			case 34:
			case 35:
			case 36:
			case 37:
			case 38:
			case 39:*lo=32;*hi=39;return 4;break; //registro 1
			case 40:
			case 41:
			case 42:
			case 43:
			case 44:
			case 45:
			case 46:
			case 47:*lo=40;*hi=47;return 5;break; //registro 1
			case 48:
			case 49:
			case 50:
			case 51:
			case 52:
			case 53:
			case 54:
			case 55:*lo=48;*hi=55;return 6;break; //registro 1
			case 56:
			case 57:
			case 58:
			case 59:
			case 60:
			case 61:
			case 62:
			case 63:*lo=56;*hi=63;return 7;break; //registro 1
			case 64:
			case 65:
			case 66:
			case 67:
			case 68:
			case 69:
			case 70:
			case 71:*lo=64;*hi=71;return 8;break; //registro 1
			case 72:
			case 73:
			case 74:
			case 75:
			case 76:
			case 77:
			case 78:
			case 79:*lo=72;*hi=79;return 9;break; //registro 1
			case 80:
			case 81:
			case 82:
			case 83:
			case 84:
			case 85:
			case 86:
			case 87:*lo=80;*hi=87;return 10;break; //registro 1
			case 88:
			case 89:
			case 90:
			case 91:
			case 92:
			case 93:
			case 94:
			case 95:*lo=88;*hi=95;return 11;break; //registro 1
			case 96:
			case 97:
			case 98:
			case 99:
			case 100:
			case 101:
			case 102:
			case 103:*lo=96;*hi=103;return 12;break; //registro 1
			case 104:
			case 105:
			case 106:
			case 107:
			case 108:
			case 109:
			case 110:
			case 111:*lo=104;*hi=111;return 13;break; //registro 1
			case 112:
			case 113:
			case 114:
			case 115:
			case 116:
			case 117:
			case 118:
			case 119:*lo=112;*hi=119;return 14;break; //registro 1
			case 120:
			case 121:
			case 122:
			case 123:
			case 124:
			case 125:
			case 126:
			case 127:*lo=120;*hi=127;return 15;break; //registro 1
			case 128:
			case 129:
			case 130:
			case 131:
			case 132:
			case 133:
			case 134:
			case 135:*lo=128;*hi=135;return 16;break; //registro 1
			case 136:
			case 137:
			case 138:
			case 139:
			case 140:
			case 141:
			case 142:
			case 143:*lo=136;*hi=143;return 17;break; //registro 1
			case 144:
			case 145:
			case 146:
			case 147:
			case 148:
			case 149:
			case 150:
			case 151:*lo=144;*hi=151;return 18;break; //registro 1
			case 152:
			case 153:
			case 154:
			case 155:
			case 156:
			case 157:
			case 158:
			case 159:*lo=152;*hi=159;return 19;break; //registro 1
			case 160:
			case 161:
			case 162:
			case 163:
			case 164:
			case 165:
			case 166:
			case 167:*lo=160;*hi=167;return 20;break; //registro 1
			case 168:
			case 169:
			case 170:
			case 171:
			case 172:
			case 173:
			case 174:
			case 175:*lo=168;*hi=175;return 21;break; //registro 1
			case 176:
			case 177:
			case 178:
			case 179:
			case 180:
			case 181:
			case 182:
			case 183:*lo=176;*hi=183;return 22;break; //registro 1
			case 184:
			case 185:
			case 186:
			case 187:
			case 188:
			case 189:
			case 190:
			case 191:*lo=184;*hi=191;return 23;break; //registro 1
			case 192:
			case 193:
			case 194:
			case 195:
			case 196:
			case 197:
			case 198:
			case 199:*lo=192;*hi=199;return 24;break;
			default:break;
			    	   
       }//fin switch--------------

return 0;
}//FIN isMemoriaVideo_emptyPixel-----------------------------------


//memoriza 300 valores de pixees que hay desplegados para
//executar el zoom
void Memoria_Pixeles_DDS(signed short int x,signed short int y){
signed short int x1;
signed short int y1;	


 

   if(dds.Bnderas.bit.EventDDS){
	   if(x==0)
		   if(y==0)
			   return;
	   if(is_Repetidos_Busqueda(x,y))
		   return;
	   if(is_FULL_FIFO_save_Temp_pixel_DDS()){
		    pop_FIFO_save_Temp_pixel_DDS(&x1,&y1);
		    push_FIFO__save_Temp_pixel_DDS(x,y); }
	   else{push_FIFO__save_Temp_pixel_DDS(x,y);}}
   if((y==1)&&(x==32)){
	    __asm(nop);
	    __asm(nop);
   }
}//fin de memoria DDS-9 -----------------------------------
	

void Cambio_de_Frecuencia_por_IIC_1(unsigned short int){//frecuencia en khz
	Cambio_de_Frecuencia_por_IIC(1000);}//frecuencia en khz}



unsigned char is_Repetidos_Busqueda(signed short int x,signed short int y){
const unsigned char SIZE1=40;
static signed short int bufferX[SIZE1];
static signed short int bufferY[SIZE1];
static signed short int *p,*py;
unsigned char i;	

    for(i=0;i<SIZE1;i++){
    	if(bufferX[i]==x)
    		if(bufferY[i]==y)
    			return TRUE;}
    if((p==0)||(p==&bufferX[SIZE1-1])){
    	 p= &bufferX[0];
         py=&bufferY[0];}
    *p=x;*py=y;
    p++;py++;
return 0;
}//fin is_Repetidos_Busqueda------------------------------------------



//unsigned char is_NO_repetido(signed short int x,signed short int y){
//unsigned short int i;	
//    for(i=0;i<SIZE_PIXEL;i++){
//    	  if(dds.SavePix.xPixel[i]==x){
//    		  if(dds.SavePix.yPixel[i]==y){
//    			  return FALSE;}}}//SI es repetido
//}//fin buscar si esta repetido las coordenads---------------------




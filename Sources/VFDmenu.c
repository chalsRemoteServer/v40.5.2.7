/*
 * VFDmenu.c
 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */


/*
 * VFDmenu.c
 *
 *  Created on: Jul 14, 2015
 *      Author: chals
 *      *      
7	 *      
8	 *  Controlador Principal de los menus y despliegue en pantalla usando
9	 *  el VFD 256x128    
 */

#include "VFDmenu.h"
#include "PE_Types.h"
#include "VFD.h"
#include "VFDserial.h"
#include "Memoria.h"
#include "Reloj.h"
#include "frecqController.h"
#include "I2C_.h"
#include "delay.h"
#include "system.h"
#include "ADC.h"
//#include "ProductController.h"
#include "errorController.h"
#include "DSP.h"
#include "SistemaOperativo.h"
#include "AnalogCntrl.h"
#include "VFDkey.h"
#include "strings.h"
#include "ADC_BAL.h"
#include "VFDisplay.h"
#include "BarraDisplay.h"
#include "VFDmenuControl.h"
#include "queue.h"

unsigned char cursorx,cursory; //posicion actual del cursor,
unsigned char EventDDS;
//unsigned char GraphDDS[SIZE_GRAPH_X][SIZE_GRAPH_Y];
unsigned short int contextobuffer;//variable auxiliar que permanece en contexto hasta que cambie
unsigned char DDSaux;//variable auxiliar usada para refrescar la pantalla DDS y poner el cursor en una Row determinada por la variable paso por valor	

//volatile unsigned char menu.contexto.Actual;


unsigned char Usuario;
unsigned char BorrarDDS; //variable que determina si hacemos el update DDS
unsigned char DDSstatus; //Nos dice si se ejecuta o no cada ciertos milisegundos el despleigue de los ADC's en el DDS
unsigned char pruebasSalidastimerControl;//variable de control de timer para  dar delay smart al la pruebas de las salidas, para prender y apagar por timer las prueas en el menu
unsigned char isMenuPruebaEntradasON;//nos dice sis esta activo el menu de pruebas de entradas 
unsigned char iDisplayProdSel;// indice de Display del menu de seleccion de porducto esta variable debe ser guardada en la NVRAM su valor es de 11 a 17 row=1->11 row 7->17
unsigned char arg0,arg1,arg2;//argumentos para entrar en un contexto
unsigned char arg5,arg6,arg8,arg9,arg10;//argumentos para entrar en un contexto
unsigned short int arg3,arg7;//argumentos para entrar en un contexto
unsigned char Text[NOMBRE_PRODUCTO_SIZE];//texto que regresa  del procesador de textos
unsigned char stateDDSplot;
unsigned short int countLineSwitch;//maquina de estados, en graficacion de puntitos del DDS
unsigned char timerPortalInicioFlag;
unsigned char semaforoDisplay;//para que  nos permita desplegar antes de activar la  barra de deteecion.
unsigned char semb; //semaforo para la barra de deteccion para mandar comandos y esperar que termine de desplegar una deteccion(varios boxes)
//extern unsigned char SemVib;  //Semaforo de Vibracion Setup 
//variable generica global activa solo mientras esta activa el menu actual
extern double igxm2; //variable generica global activa solo mientras esta activa el menu actual


extern double anguloVibracion;//angulo de vibracion, cantidad de angulos en Sexahesimales que gira la se�al

extern unsigned char modificado;
extern unsigned char semMenu2;
extern union wordReg EntradasSistemaUnion;
extern unsigned char DDSplotflag;//bandera para ver si ya se mando un byte TX al VFD		
extern unsigned char DDSplotTimer;//variable para generar un delay en mandar comandos para despliegue de los puntitos
extern unsigned char CVdispTimer;//variable para generar un delat en mandar comandos para desplegar los numeros en configuracion de vibracion Menu
extern void restaurar(unsigned char contexto1);//restaura las variables de un contexto, las saca de la memo eeprom y las vacia en las variables coresponidetes
extern void saveConfig(unsigned char contexto1);//guarda las variables de un contexto
//extern unsigned char popLIFOdata_DDS(unsigned short int *xx,unsigned short int *yy);
extern unsigned char BARplotTimer;//variable para generar un delay en mandar comandos para display boxes
extern void configModificado(unsigned char m);
//extern unsigned char  memoEEPROM_SearchPlaceUP(unsigned char iniProd);


extern void ReleRechazo(unsigned char status);
extern void ReleFalla(unsigned char status);
extern void ReleAlarma(unsigned char status);
extern void ReleBloqueo(unsigned char status);
extern void SalidaPHVF(unsigned char status);
extern void SalidaAux(unsigned char status);


extern unsigned char IDcomm;
extern unsigned char Alta; //variable que guarda el ultimo digito de la frecuencia alta
extern unsigned char Media;//variable que guarda el ultimo digito de la frecuencia media
extern unsigned char Baja;//variable que guarda el ultimo digito de la frecuencpia baja
extern unsigned char CuentaProducto; //variable de activacion de  cuenta de producto
extern unsigned char Control_de_usuario;
extern unsigned char Guardia;
extern unsigned char tipo_deMaquina;
extern unsigned char filtroConfig; // variable filtro  de filtro en ingenieria  gracida
extern unsigned short int  Sensibilidad; // variable de sensibilidad
extern unsigned char frecuenciaSeleccion;//SELECCIONA  la frecuencia actual configurada y activada
extern unsigned char frecuencyMode;//SELECCCIONA SI ES ALTA MEDIA ó BAJA la frecuencia seleccionada
extern unsigned int short Altura; //variable de parametros de deteccion------para el DDS
extern unsigned char GananciaDDS; //ganancia del DDS, esta no se ha guardado
extern unsigned char cuadritoDDS; //variable cuadrito poco estudiada
extern unsigned char name[NOMBRE_PRODUCTO_SIZE]; 
extern unsigned char Parte11AnalogVer;//guarda la version de la Analoga que vamos a controlar

extern unsigned char displayADCvalores;// dice si se ejecuta desplegar los valores del adc en pruebas o no.


extern unsigned char getTacho(void);
extern unsigned char getGatin(void);
extern unsigned char sysCHK(void);
extern unsigned char packCHK(void);
extern unsigned char Aux1IN(void);
extern unsigned char Aux2IN(void);
extern unsigned char LockSW(void);
extern unsigned char ResetSW(void);

extern char  pushFIFOcOPc(struct overLoad d);
extern unsigned char passControl[PASSCNTRL_SIZE];


//extern struct ProductDataBaseNode{
	//       unsigned char ID; //ID DEL producto para encontrarlo en la base de datos
	  //     char name[NOMBRESIZEPROD];    //nombre del producto
	    //   unsigned int Sensibilidad;  //sensibilidad 
	      // unsigned int Altura;
	     //  unsigned char phase;
	      // unsigned char phasefrac;
	      // char     zoomDDS;
	      // char     gananciaDDS;
	      // char     saved;   //se han salvado alguno de estos parametros??
//}ProdDBNode;//producto Actual traido de la memoria al init
	


extern unsigned char EventDDS;
extern unsigned char DDSstatus; //Nos dice si se ejecuta o no cada ciertos milisegundos el despleigue de los ADC's en el DDS
extern unsigned char timerEnableKeypad,timerEnableKeypad2;
extern unsigned char pruebasSalidastimerControl;//variable de control de timer para  dar delay smart al la pruebas de las salidas, para prender y apagar por timer las prueas en el menu
extern unsigned char DriverOutcontrol;//variable que determina si prende � no la salida driver

unsigned char DDSplotflag;//bandera para ver si ya se mando un byte TX al VFD		
unsigned char DDSplotTimer;//variable para generar un delay en mandar comandos para despliegue de los puntitos
unsigned char BARplotTimer;//variable para generar un delay en mandar comandos para despliegue de las boxes
unsigned char CVdispTimer;//variable para generar un delat en mandar comandos para desplegar los numeros en configuracion de vibracion Menu
extern volatile unsigned char  TM1_IRQ1;

extern struct _GLOBAL_CONTEXTO g,*AjustVibr,*DDSpant,*AjParamProd,*ParamDet;
extern struct _GLOBAL_CONTEXTO  *textProc;

extern struct ZoomControl zoom;
extern struct _Detection Deteccion;
extern struct overLoad popFIFOcOP(void);
extern struct _Comando_DDS dds; //extern struct _DDS_ DDS1;

extern struct _Mesurements_ Mediciones;
extern struct _OFFSET_ Offset;//CONTROL DE  las variables de offset
extern struct _SERIAL_ serial;// _Vector_ serial;


extern struct EntSyst{//variables de entradas de sistema 
		unsigned char taco;	
		unsigned char fotocel;
		unsigned char rechazo;
		unsigned char producto; 
		unsigned char aux1;
		unsigned char aux2;
		unsigned char byten;	
}EntradasSistemaReg;

extern struct SEMAFOROS{
	   unsigned char I2Cdevice;//cual dispositivo se esta usando
	   unsigned char operacion;//que se le va hacer
	   unsigned char *p;//puntador de datos
	   unsigned char semTemporizado;
}Sem;


struct _KeyPad_ keypad;
struct _Menu_  menu;

//--DECLARACION DE LOS MENUS--------------------------------poner nombre de menu--------------------------------2a

//struct ArbolMenu MenuInsight,AjusteProducto,AjusteParamProducto,AjusteSistema;
//struct ArbolMenu InformacionUsuario,HacerPruebaEjemplo,SeleccionarProducto,NuevoProducto,AjusteProducto;
//struct ArbolMenu BorrarProducto,CopiarProducto;
//struct ArbolMenu MenuAdministrativo,AjusteVibracion,ControlFrecuencias,IDcomunicaciones;
//struct ArbolMenu IngenieriaGracida,InformacionUsuario;

//struct ArbolMenu PantallaDDS,HacerPruebaRuido,MedidasVoltaje,AjusteProductoInfo,TiemposRechazo,AjusteSistemaEntradas;
//struct ArbolMenu IDmaquina,CuentaProductoMenu,AjusteParametricoProd;
//struct ArbolMenu Portalinicio;
//struct ArbolMenu *var1,*var2,*var3; //pantalla contexto actual---------------------------------------------
//struct ArbolMenu MenuAdminstrativo; //----------------------MENU@1:DECLARAR LA ESTRUCTURA DEL MENU A CREAR
//struct ArbolMenu ConfigurarEntradasSistema;
//struct ArbolMenu EstablecerIdioma;
//struct ArbolMenu Controlpasswords,RelojSistema,initMemorioaYrest;
//struct ArbolMenu TipoMaquina;
//struct ArbolMenu Ganancia,Filtro,IngenieriaGracida2,AjusteParamProd;
//struct ArbolMenu AjustesAvanzados;
//struct ArbolMenu FrecuenciaSelect;
//struct ArbolMenu pruebaMenu,I2CMenu,I2Ce2PROMRead,I2CNVRAMread,SPIMenu;
//struct ArbolMenu AjustesAvanzadosAPP,ParametrosdeDeteccion;
//struct ArbolMenu displayADCs,pruebasSalidas,pruebasEntradas;
//struct ArbolMenu SeleccionarProducto,NuevoProducto;
//struct ArbolMenu TextoProcessor,AjusteProductoHijo,Parte11,AjusteProductoMode,BorrarProducto,CopyProducto;
//struct ArbolMenu AjusteProductoSelect,NombreProductoEdit;
struct ArbolMenu MenuActualScreen;//la estrucrura del menu actual en pantalla.

void init_Menu(void){
	//  initArbolMenu();deprecated se movio a otro lado y se modifico la de version
	  BorrarDDS=NONE;  //el DDs no se activado el boton de borrar el DDS
	  DDSstatus=NONE; //no se ejecuta el despleigue en pantalla de los valores del ADC's en puntitos
	  Usuario=UserDesarrollador;//este se tiene que asignar despeus en el control de usuarios	 
	  isMenuPruebaEntradasON=FALSE;
}//fin intialization Menu VFD-----------------------------------------------

void run_Menu(void){
	  menu.contexto.Actual=PORTAL_INICIO;//-----revisar porque ya  esta en menu co
      initArbolMenu(menu.contexto.Actual);//menu que se va desplegar crear los pointers de la estructura de menu
	  MenuControl();
}//fin run menu------------------------------------------------------------


//   "v39.15.5.35"
void initArbolMenu(unsigned char destino){// initializar estructura de datos de los menus
	 
	 switch(destino){
	  /* PANTALLA NIVEL 0************************************************************** */
		 case PORTAL_INICIO: MenuActualScreen.menuNodo=PORTAL_INICIO;
		                     MenuActualScreen.permisos=DESARROLLADOR_PERMISO;//------------------------------------EN-REVISION
		                     MenuActualScreen.func=PortalinicioDisplay; // -------------------------------------------------------EN-REVISION    		 
		                     MenuActualScreen.funcKeyUP=PortaliniciokeyUP;
		                     MenuActualScreen.funcKeyRT=PortaliniciokeyRT;
		                     MenuActualScreen.funcKeyLF=PortaliniciokeyLF;
		                     MenuActualScreen.funcKeyDN=PortaliniciokeyDN;//Insert the function pointer
		                     MenuActualScreen.funcKeyEN=PortaliniciokeyEN;//---poner en los apuntadores apuntar a las funciones--------------4
							 break;		  
	/* MENUS NIVEL 1 ***************************************************************/	
		 case MENU_INSIGHT:  MenuActualScreen.menuNodo=MENU_INSIGHT;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=displayMenuInsight;
							 MenuActualScreen.funcKeyUP=MenuInsightkeyUP;
							 MenuActualScreen.funcKeyDN=MenuInsightkeyDN;
							 MenuActualScreen.funcKeyRT=MenuInsightkeyRT;
							 MenuActualScreen.funcKeyLF=MenuInsightkeyLF;
							 MenuActualScreen.funcKeyEN=MenuInsightkeyEN;//MenuInsightkeyEN;
		                     break;			           
	/* MENUS NIVEL 2  ******************************************************************/     
		 case AJUSTE_DE_PRODUCTO:MenuActualScreen.menuNodo=AJUSTE_DE_PRODUCTO;
		 	 	 	 	 	 	 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
		 	 	 	 	 	 	 MenuActualScreen.func=displayMenuAjustedeProducto;//no tiene porque es un menu
		 	 	 	 	 	 	 MenuActualScreen.funcKeyUP=AjusteProductokeyUP;
		 	 	 	 	 	     MenuActualScreen.funcKeyRT=AjusteProductokeyRT;	 
		 	 	 	 	 	     MenuActualScreen.funcKeyLF=AjusteProductokeyLF;
		 	 	 	 	 	     MenuActualScreen.funcKeyDN=AjusteProductokeyDN;
		 	 	 	 	 	     MenuActualScreen.funcKeyEN=AjusteProductokeyEN;
		 	 	 	 	 	     break;
		 case AJUSTE_PARAMETRICO_DE_PRODUCTO:
							 MenuActualScreen.menuNodo=AJUSTE_PARAMETRICO_DE_PRODUCTO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayAjusteParamProd;
							 MenuActualScreen.funcKeyUP=AjusteParamProdkeyUP;
							 MenuActualScreen.funcKeyRT=AjusteParamProdkeyRT;	 
							 MenuActualScreen.funcKeyLF=AjusteParamProdkeyLF;
							 MenuActualScreen.funcKeyDN=AjusteParamProdkeyDN;
							 MenuActualScreen.funcKeyEN=AjusteParamProdkeyEN;
							 break;
		 case AJUSTE_DE_SISTEMA:
			  	  	  	  	 MenuActualScreen.menuNodo=AJUSTE_DE_SISTEMA;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayMenuAjustedeSistema;
							 MenuActualScreen.funcKeyUP=AjustedeSistemakeyUP;
							 MenuActualScreen.funcKeyRT=AjustedeSistemakeyRT;	 
							 MenuActualScreen.funcKeyLF=AjustedeSistemakeyLF;
							 MenuActualScreen.funcKeyDN=AjustedeSistemakeyDN;
							 MenuActualScreen.funcKeyEN=AjustedeSistemakeyEN;
							 break;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-------- PASO-2							 
		 case PANTALLA_DDS://---------------------------------------------------------- PASO 3 IR A VFDkey.h-----
							 MenuActualScreen.menuNodo=PANTALLA_DDS;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DDS_HANDLER;
							 MenuActualScreen.funcKeyUP=PANTALLA_DDSkeyUP;
							 MenuActualScreen.funcKeyRT=PANTALLA_DDSkeyRT;	 
							 MenuActualScreen.funcKeyLF=PANTALLA_DDSkeyLF;
							 MenuActualScreen.funcKeyDN=PANTALLA_DDSkeyDN;
							 MenuActualScreen.funcKeyEN=PANTALLA_DDSkeyEN;
							 break;
		 case INFORMACION_DE_USUARIO:			 
			 	 	 	 	 MenuActualScreen.menuNodo=INFORMACION_DE_USUARIO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayMenuInformacionUsuario;
							 MenuActualScreen.funcKeyUP=MenuInformacionUsuariokeyUP;
							 MenuActualScreen.funcKeyRT=MenuInformacionUsuariokeyRT;	 
							 MenuActualScreen.funcKeyLF=MenuInformacionUsuariokeyLF;
							 MenuActualScreen.funcKeyDN=MenuInformacionUsuariokeyDN;
							 MenuActualScreen.funcKeyEN=MenuInformacionUsuariokeyEN;
							 break;
		 case MENU_ADMINISTRATIVO:
			 	 	 	 	 MenuActualScreen.menuNodo=MENU_ADMINISTRATIVO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayMenuAdministrativo;
							 MenuActualScreen.funcKeyUP=MenuAdministrativokeyUP;
							 MenuActualScreen.funcKeyRT=MenuAdministrativokeyRT;	 
							 MenuActualScreen.funcKeyLF=MenuAdministrativokeyLF;
							 MenuActualScreen.funcKeyDN=MenuAdministrativokeyDN;
							 MenuActualScreen.funcKeyEN=MenuAdministrativokeyEN;
							 break;
		 case CONFIGURAR_ENTRADAS_DE_SISTEMA:
			 	 	 	 	 MenuActualScreen.menuNodo=CONFIGURAR_ENTRADAS_DE_SISTEMA;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayConfigIOsistem;
							 MenuActualScreen.funcKeyUP=ConfigIOsistemkeyUP;
							 MenuActualScreen.funcKeyRT=ConfigIOsistemkeyRT;	 
							 MenuActualScreen.funcKeyLF=ConfigIOsistemkeyLF;
							 MenuActualScreen.funcKeyDN=ConfigIOsistemkeyDN;
							 MenuActualScreen.funcKeyEN=ConfigIOsistemkeyEN;
							 break;
		 case ID_COMUNICACIONES:
			 	 	 	 	 MenuActualScreen.menuNodo=ID_COMUNICACIONES;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayIDcomunicaciones;
							 MenuActualScreen.funcKeyUP=IDcomunicacioneskeyUP;
							 MenuActualScreen.funcKeyRT=IDcomunicacioneskeyRT;	 
							 MenuActualScreen.funcKeyLF=IDcomunicacioneskeyLF;
							 MenuActualScreen.funcKeyDN=IDcomunicacioneskeyDN;
							 MenuActualScreen.funcKeyEN=IDcomunicacioneskeyEN;
							 break;
		 case CONTROL_DE_FRECUENCIA:
			 	 	 	 	 MenuActualScreen.menuNodo=CONTROL_DE_FRECUENCIA;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayControlFrecuencias;
							 MenuActualScreen.funcKeyUP=ControlFrecuenciaskeyUP;
							 MenuActualScreen.funcKeyRT=ControlFrecuenciaskeyRT;	 
							 MenuActualScreen.funcKeyLF=ControlFrecuenciaskeyLF;
							 MenuActualScreen.funcKeyDN=ControlFrecuenciaskeyDN;
							 MenuActualScreen.funcKeyEN=ControlFrecuenciaskeyEN;
							 break;
		 case MEDIDAS_DE_VOLTAJE:       
							 MenuActualScreen.menuNodo=MEDIDAS_DE_VOLTAJE;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayMedidasVoltaje;//menuPadre=INFORMACION_DE_USUARIO;
							 MenuActualScreen.funcKeyUP=MedidasVoltajekeyUP;
							 MenuActualScreen.funcKeyRT=MedidasVoltajekeyRT;	 
							 MenuActualScreen.funcKeyLF=MedidasVoltajekeyLF;
							 MenuActualScreen.funcKeyDN=MedidasVoltajekeyDN;
							 MenuActualScreen.funcKeyEN=MedidasVoltajekeyEN;
							 break;
		 case MEDIDAS_DE_VOLTAJES:       
							 MenuActualScreen.menuNodo=MEDIDAS_DE_VOLTAJES;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayMedidasVoltajes;//menuPadre=INFORMACION_DE_USUARIO;
							 MenuActualScreen.funcKeyUP=MedidasVoltajeskeyUP;
							 MenuActualScreen.funcKeyRT=MedidasVoltajeskeyRT;	 
							 MenuActualScreen.funcKeyLF=MedidasVoltajeskeyLF;
							 MenuActualScreen.funcKeyDN=MedidasVoltajeskeyDN;
							 MenuActualScreen.funcKeyEN=MedidasVoltajeskeyEN;
							 break;		 
		 case AJUSTE_DE_PRODUCTOINFO:
							 MenuActualScreen.menuNodo=AJUSTE_DE_PRODUCTOINFO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayAjusteProductoInfo;//menuPadre=INFORMACION_DE_USUARIO;
							 MenuActualScreen.funcKeyUP=AjusteProductoInfokeyUP;
							 MenuActualScreen.funcKeyRT=AjusteProductoInfokeyRT;	 
							 MenuActualScreen.funcKeyLF=AjusteProductoInfokeyLF;
							 MenuActualScreen.funcKeyDN=AjusteProductoInfokeyDN;
							 MenuActualScreen.funcKeyEN=AjusteProductoInfokeyEN;
							 break;
		 case TIEMPO_DE_RECHAZOINFO:
							 MenuActualScreen.menuNodo=TIEMPO_DE_RECHAZOINFO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayTiempoRechazo;
							 MenuActualScreen.funcKeyUP=TiempoRechazoInfokeyUP;
							 MenuActualScreen.funcKeyRT=TiempoRechazoInfokeyRT;	 
							 MenuActualScreen.funcKeyLF=TiempoRechazoInfokeyLF;
							 MenuActualScreen.funcKeyDN=TiempoRechazoInfokeyDN;
							 MenuActualScreen.funcKeyEN=TiempoRechazoInfokeyEN;
//							 TiemposRechazo.menuPadre=INFORMACION_DE_USUARIO;
							 break;
		 case AJUSTE_SISTEMA_ENTRADAS:
							 MenuActualScreen.menuNodo=AJUSTE_SISTEMA_ENTRADAS;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayAjusteSistemaEntradas;
							 MenuActualScreen.funcKeyUP=AjusteSistemaEntradaskeyUP;
							 MenuActualScreen.funcKeyRT=AjusteSistemaEntradaskeyRT;	 
							 MenuActualScreen.funcKeyLF=AjusteSistemaEntradaskeyLF;
							 MenuActualScreen.funcKeyDN=AjusteSistemaEntradaskeyDN;
							 MenuActualScreen.funcKeyEN=AjusteSistemaEntradaskeyEN;
//							 AjusteSistemaEntradas.menuPadre=INFORMACION_DE_USUARIO;
							 break;
		 case ID_MAQUINA:
							 MenuActualScreen.menuNodo=ID_MAQUINA;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayIDmaquina;
							 MenuActualScreen.funcKeyUP=IDmaquinakeyUP;
							 MenuActualScreen.funcKeyRT=IDmaquinakeyRT;	 
							 MenuActualScreen.funcKeyLF=IDmaquinakeyLF;
							 MenuActualScreen.funcKeyDN=IDmaquinakeyDN;
							 MenuActualScreen.funcKeyEN=IDmaquinakeyEN;
 //							 IDmaquina.menuPadre=INFORMACION_DE_USUARIO;
							 break;
		 case CUENTA_PRODUCTO:
							 MenuActualScreen.menuNodo=CUENTA_PRODUCTO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayCuentaProducto;
							 MenuActualScreen.funcKeyUP=CuentaProductokeyUP;
							 MenuActualScreen.funcKeyRT=CuentaProductokeyRT;	 
							 MenuActualScreen.funcKeyLF=CuentaProductokeyLF;
							 MenuActualScreen.funcKeyDN=CuentaProductokeyDN;
							 MenuActualScreen.funcKeyEN=CuentaProductokeyEN;
  //						 CuentaProductoMenu.menuPadre=INFORMACION_DE_USUARIO;
							 break;
		 case ESTABLECER_IDIOMA:
							 MenuActualScreen.menuNodo=ESTABLECER_IDIOMA;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayEstablecerIdioma;
							 MenuActualScreen.funcKeyUP=EstablecerIdiomakeyUP;
							 MenuActualScreen.funcKeyRT=EstablecerIdiomakeyRT;	 
							 MenuActualScreen.funcKeyLF=EstablecerIdiomakeyLF;
							 MenuActualScreen.funcKeyDN=EstablecerIdiomakeyDN;
							 MenuActualScreen.funcKeyEN=EstablecerIdiomakeyEN;
   //						 EstablecerIdioma.menuPadre=MENU_ADMINISTRATIVO;
							 break;
		 case CONTROL_PASSWORD:
							 MenuActualScreen.menuNodo=CONTROL_PASSWORD;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayControlpassword;
							 MenuActualScreen.funcKeyUP=ControlpasswordkeyUP;
							 MenuActualScreen.funcKeyRT=ControlpasswordkeyRT;	 
							 MenuActualScreen.funcKeyLF=ControlpasswordkeyLF;
							 MenuActualScreen.funcKeyDN=ControlpasswordkeyDN;
							 MenuActualScreen.funcKeyEN=ControlpasswordkeyEN;
	//						 Controlpasswords.menuPadre=MENU_ADMINISTRATIVO;
							 break;
		 case RELOJ_DE_SISTEMA:
							 MenuActualScreen.menuNodo=RELOJ_DE_SISTEMA;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayRelojSistema;
							 MenuActualScreen.funcKeyUP=RelojSistemakeyUP;
							 MenuActualScreen.funcKeyRT=RelojSistemakeyRT;	 
							 MenuActualScreen.funcKeyLF=RelojSistemakeyLF;
							 MenuActualScreen.funcKeyDN=RelojSistemakeyDN;
							 MenuActualScreen.funcKeyEN=RelojSistemakeyEN;
	//						 Controlpasswords.menuPadre=MENU_ADMINISTRATIVO;
							 break;
		 case INGENIERIA_GRACIDA:
							 MenuActualScreen.menuNodo=INGENIERIA_GRACIDA;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayIngenieriaGracida;
							 MenuActualScreen.funcKeyUP=IngenieriaGracidakeyUP;
							 MenuActualScreen.funcKeyRT=IngenieriaGracidakeyRT;	 
							 MenuActualScreen.funcKeyLF=IngenieriaGracidakeyLF;
							 MenuActualScreen.funcKeyDN=IngenieriaGracidakeyDN;
							 MenuActualScreen.funcKeyEN=IngenieriaGracidakeyEN;
	//						 IngenieriaGracida.menuPadre=AJUSTE_DE_SISTEMA;
							 break;
		 case INIT_MEMO_Y_REST:
							 MenuActualScreen.menuNodo=INIT_MEMO_Y_REST;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayInitMemoria_y_Restaurar;
							 MenuActualScreen.funcKeyUP=InitMemoria_y_RestaurarkeyUP;
							 MenuActualScreen.funcKeyRT=InitMemoria_y_RestaurarkeyRT;	 
							 MenuActualScreen.funcKeyLF=InitMemoria_y_RestaurarkeyLF;
							 MenuActualScreen.funcKeyDN=InitMemoria_y_RestaurarkeyDN;
							 MenuActualScreen.funcKeyEN=InitMemoria_y_RestaurarkeyEN;
	//						 nitMemorioaYrest.menuPadre=MENU_ADMINISTRATIVO;
							 break;
		 case TIPO_DE_MAQUINA:
							 MenuActualScreen.menuNodo=TIPO_DE_MAQUINA;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayTipodeMaquina;
							 MenuActualScreen.funcKeyUP=TipodeMaquinakeyUP;
							 MenuActualScreen.funcKeyRT=TipodeMaquinakeyRT;	 
							 MenuActualScreen.funcKeyLF=TipodeMaquinakeyLF;
							 MenuActualScreen.funcKeyDN=TipodeMaquinakeyDN;
							 MenuActualScreen.funcKeyEN=TipodeMaquinakeyEN;
	//						 TipoMaquina.menuPadre=AJUSTE_DE_SISTEMA;
							 break;
		 case GANANCIA_:
							 MenuActualScreen.menuNodo=GANANCIA_;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayMenuGanancia;
							 MenuActualScreen.funcKeyUP=MenuGananciakeyUP;
							 MenuActualScreen.funcKeyRT=MenuGananciakeyRT;	 
							 MenuActualScreen.funcKeyLF=MenuGananciakeyLF;
							 MenuActualScreen.funcKeyDN=MenuGananciakeyDN;
							 MenuActualScreen.funcKeyEN=MenuGananciakeyEN;
	//						 Ganancia.menuPadre=INGENIERIA_GRACIDA;
							 break;
		 case FILTRO_:
							 MenuActualScreen.menuNodo=FILTRO_;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayMenuFiltro;
							 MenuActualScreen.funcKeyUP=MenuFiltrokeyUP;
							 MenuActualScreen.funcKeyRT=MenuFiltrokeyRT;	 
							 MenuActualScreen.funcKeyLF=MenuFiltrokeyLF;
							 MenuActualScreen.funcKeyDN=MenuFiltrokeyDN;
							 MenuActualScreen.funcKeyEN=MenuFiltrokeyEN;
	//						 Filtro.menuPadre=INGENIERIA_GRACIDA;
							 break;
		 case INGENIERIA_GRACIDA2:
							 MenuActualScreen.menuNodo=INGENIERIA_GRACIDA2;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayIngenieriaGracida2;
							 MenuActualScreen.funcKeyUP=IngenieriaGracida2keyUP;
							 MenuActualScreen.funcKeyRT=IngenieriaGracida2keyRT;	 
							 MenuActualScreen.funcKeyLF=IngenieriaGracida2keyLF;
							 MenuActualScreen.funcKeyDN=IngenieriaGracida2keyDN;
							 MenuActualScreen.funcKeyEN=IngenieriaGracida2keyEN;
	//						 IngenieriaGracida2.menuPadre=INGENIERIA_GRACIDA;
							 break;
//		 case AJUSTE_PARAMETRICO_DE_PRODUCTO:
//							 MenuActualScreen.menuNodo=AJUSTE_PARAMETRICO_DE_PRODUCTO;
//							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
//							 MenuActualScreen.func=DisplayAjusteParamProd;
//							 MenuActualScreen.funcKeyUP=AjusteParamProdkeyUP;
//							 MenuActualScreen.funcKeyRT=AjusteParamProdkeyRT;	 
//							 MenuActualScreen.funcKeyLF=AjusteParamProdkeyLF;
//							 MenuActualScreen.funcKeyDN=AjusteParamProdkeyDN;
//							 MenuActualScreen.funcKeyEN=AjusteParamProdkeyEN;
//	//						  AjusteParamProd.menuPadre=MENU_INSIGHT;
//							 break;
		 case AJUSTES_AVANZADOS:
							 MenuActualScreen.menuNodo=AJUSTES_AVANZADOS;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayAjustesAvanzados;
							 MenuActualScreen.funcKeyUP=AjustesAvanzadoskeyUP;
							 MenuActualScreen.funcKeyRT=AjustesAvanzadoskeyRT;	 
							 MenuActualScreen.funcKeyLF=AjustesAvanzadoskeyLF;
							 MenuActualScreen.funcKeyDN=AjustesAvanzadoskeyDN;
							 MenuActualScreen.funcKeyEN=AjustesAvanzadoskeyEN;
	//						 AjustesAvanzados.menuPadre=AJUSTE_PARAMETRICO_DE_PRODUCTO;
							 break;//#if
		 case FRECUENCIA_SELECT:
							 MenuActualScreen.menuNodo=FRECUENCIA_SELECT;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayFrecuenciaSelect;
							 MenuActualScreen.funcKeyUP=FrecuenciaSelectkeyUP;
							 MenuActualScreen.funcKeyRT=FrecuenciaSelectkeyRT;	 
							 MenuActualScreen.funcKeyLF=FrecuenciaSelectkeyLF;
							 MenuActualScreen.funcKeyDN=FrecuenciaSelectkeyDN;
							 MenuActualScreen.funcKeyEN=FrecuenciaSelectkeyEN;
	//						 FrecuenciaSelect.menuPadre=AJUSTES_AVANZADOS;
							 break;
		 case PRUEBA:
							 MenuActualScreen.menuNodo=PRUEBA;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayPruebaMenu;
							 MenuActualScreen.funcKeyUP=PruebaMenukeyUP;
							 MenuActualScreen.funcKeyRT=PruebaMenukeyRT;	 
							 MenuActualScreen.funcKeyLF=PruebaMenukeyLF;
							 MenuActualScreen.funcKeyDN=PruebaMenukeyDN;
							 MenuActualScreen.funcKeyEN=PruebaMenukeyEN;
	//						 pruebaMenu.menuPadre=INGENIERIA_GRACIDA2;
							 break;
		 case I2CMENU:
							 MenuActualScreen.menuNodo=I2CMENU;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayI2CMenu;
							 MenuActualScreen.funcKeyUP=I2CMenukeyUP;
							 MenuActualScreen.funcKeyRT=I2CMenukeyRT;	 
							 MenuActualScreen.funcKeyLF=I2CMenukeyLF;
							 MenuActualScreen.funcKeyDN=I2CMenukeyDN;
							 MenuActualScreen.funcKeyEN=I2CMenukeyEN;
	//						 I2CMenu.menuPadre=PRUEBA;
							 break;
		 case I2CEEPROMREAD:
							 MenuActualScreen.menuNodo=I2CEEPROMREAD;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayI2CeepromData;
							 MenuActualScreen.funcKeyUP=I2CeepromDatakeyUP;
							 MenuActualScreen.funcKeyRT=I2CeepromDatakeyRT;	 
							 MenuActualScreen.funcKeyLF=I2CeepromDatakeyLF;
							 MenuActualScreen.funcKeyDN=I2CeepromDatakeyDN;
							 MenuActualScreen.funcKeyEN=I2CeepromDatakeyEN;
	//						 I2Ce2PROMRead.menuPadre=PRUEBA;
							 break;
		 case I2CNVRAMDATA:
							 MenuActualScreen.menuNodo=I2CNVRAMDATA;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayI2CNVRAMdata;
							 MenuActualScreen.funcKeyUP=I2CNVRAMdatakeyUP;
							 MenuActualScreen.funcKeyRT=I2CNVRAMdatakeyRT;	 
							 MenuActualScreen.funcKeyLF=I2CNVRAMdatakeyLF;
							 MenuActualScreen.funcKeyDN=I2CNVRAMdatakeyDN;
							 MenuActualScreen.funcKeyEN=I2CNVRAMdatakeyEN;
	//						 I2CNVRAMread.menuPadre=PRUEBA;
							 break;
		 case SPI_MENU:
							 MenuActualScreen.menuNodo=SPI_MENU;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplaySPIMenu;
							 MenuActualScreen.funcKeyUP=SPIMenukeyUP;
							 MenuActualScreen.funcKeyRT=SPIMenukeyRT;	 
							 MenuActualScreen.funcKeyLF=SPIMenukeyLF;
							 MenuActualScreen.funcKeyDN=SPIMenukeyDN;
							 MenuActualScreen.funcKeyEN=SPIMenukeyEN;
	//						 SPIMenu.menuPadre=PRUEBA;
							 break;
		 case PRUEBAS_SALIDAS:
							 MenuActualScreen.menuNodo=PRUEBAS_SALIDAS;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayPruebasSalidasMenu;
							 MenuActualScreen.funcKeyUP=PruebasSalidasMenukeyUP;
							 MenuActualScreen.funcKeyRT=PruebasSalidasMenukeyRT;	 
							 MenuActualScreen.funcKeyLF=PruebasSalidasMenukeyLF;
							 MenuActualScreen.funcKeyDN=PruebasSalidasMenukeyDN;
							 MenuActualScreen.funcKeyEN=PruebasSalidasMenukeyEN;
	//						 pruebasSalidas.menuPadre=PRUEBA;
							 break;
		 case PRUEBAS_ENTRADAS:
							 MenuActualScreen.menuNodo=PRUEBAS_ENTRADAS;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayPruebasEntradasMenu;
							 MenuActualScreen.funcKeyUP=PruebasEntradasMenukeyUP;
							 MenuActualScreen.funcKeyRT=PruebasEntradasMenukeyRT;	 
							 MenuActualScreen.funcKeyLF=PruebasEntradasMenukeyLF;
							 MenuActualScreen.funcKeyDN=PruebasEntradasMenukeyDN;
							 MenuActualScreen.funcKeyEN=PruebasEntradasMenukeyEN;
	//						  pruebasEntradas.menuPadre=PRUEBA;
							 break;
//		 case AjustesAvanzadosAPP:
//							 MenuActualScreen.menuNodo=AjustesAvanzadosAPP;
//							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
//							 MenuActualScreen.func=DisplayAjustesAvanzadosAPP;
//							 MenuActualScreen.funcKeyUP=AjustesAvanzadosAPPkeyUP;
//							 MenuActualScreen.funcKeyRT=AjustesAvanzadosAPPkeyRT;	 
//							 MenuActualScreen.funcKeyLF=AjustesAvanzadosAPPkeyLF;
//							 MenuActualScreen.funcKeyDN=AjustesAvanzadosAPPkeyDN;
//							 MenuActualScreen.funcKeyEN=AjustesAvanzadosAPPkeyEN;
//	//						 AjustesAvanzadosAPP.menuPadre=AJUSTE_PARAMETRICO_DE_PRODUCTO;
//							 break;
		 case PARAMETROS_DE_DETECCION:
							 MenuActualScreen.menuNodo=PARAMETROS_DE_DETECCION;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayParametrosdeDeteccion;
							 MenuActualScreen.funcKeyUP=ParametrosdeDeteccionkeyUP;
							 MenuActualScreen.funcKeyRT=ParametrosdeDeteccionkeyRT;	 
							 MenuActualScreen.funcKeyLF=ParametrosdeDeteccionkeyLF;
							 MenuActualScreen.funcKeyDN=ParametrosdeDeteccionkeyDN;
							 MenuActualScreen.funcKeyEN=ParametrosdeDeteccionkeyEN;
	//						  ParametrosdeDeteccion.menuPadre=AJUSTES_AVANZADOS;
							 break;
		 case DISPLAY_ADC:
							 MenuActualScreen.menuNodo=DISPLAY_ADC;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=displayADCsValues;
							 MenuActualScreen.funcKeyUP=ADCsValueskeyUP;
							 MenuActualScreen.funcKeyRT=ADCsValueskeyRT;	 
							 MenuActualScreen.funcKeyLF=ADCsValueskeyLF;
							 MenuActualScreen.funcKeyDN=ADCsValueskeyDN;
							 MenuActualScreen.funcKeyEN=ADCsValueskeyEN;
	//						 displayADCs.menuPadre=SPI_MENU;
							 break;
		 case SELECCIONAR_PRODUCTO:
							 MenuActualScreen.menuNodo=SELECCIONAR_PRODUCTO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=displaySeleccionarProducto;
							 MenuActualScreen.funcKeyUP=SeleccionarProductokeyUP;
							 MenuActualScreen.funcKeyRT=SeleccionarProductokeyRT;	 
							 MenuActualScreen.funcKeyLF=SeleccionarProductokeyLF;
							 MenuActualScreen.funcKeyDN=SeleccionarProductokeyDN;
							 MenuActualScreen.funcKeyEN=SeleccionarProductokeyEN;
	//						  SeleccionarProducto.menuPadre=AJUSTE_DE_PRODUCTO;
							 break;    
		 case NUEVO_PRODUCTO:
							 MenuActualScreen.menuNodo=NUEVO_PRODUCTO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=displayNuevoProducto;
							 MenuActualScreen.funcKeyUP=NuevoProductokeyUP;
							 MenuActualScreen.funcKeyRT=NuevoProductokeyRT;	 
							 MenuActualScreen.funcKeyLF=NuevoProductokeyLF;
							 MenuActualScreen.funcKeyDN=NuevoProductokeyDN;
							 MenuActualScreen.funcKeyEN=NuevoProductokeyEN;
	//						   NuevoProducto.menuPadre=AJUSTE_DE_PRODUCTO;
							 break;    
		 case TEXT_PROCESSOR:
							 MenuActualScreen.menuNodo=TEXT_PROCESSOR;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=displayTextoProcessor;
							 MenuActualScreen.funcKeyUP=TextoProcessorkeyUP;
							 MenuActualScreen.funcKeyRT=TextoProcessorkeyRT;	 
							 MenuActualScreen.funcKeyLF=TextoProcessorkeyLF;
							 MenuActualScreen.funcKeyDN=TextoProcessorkeyDN;
							 MenuActualScreen.funcKeyEN=TextoProcessorkeyEN;
	//						  TextoProcessor.menuPadre=NONE;
							 break;    
		 case AJUSTE_DE_PRODUCTO_HIJO:
							 MenuActualScreen.menuNodo=AJUSTE_DE_PRODUCTO_HIJO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							// MenuActualScreen.func=displayAjusteProductoHijo; debug ver v39.12.0
							 MenuActualScreen.funcKeyUP=AjusteProductoHijokeyUP;
							 MenuActualScreen.funcKeyRT=AjusteProductoHijokeyRT;	 
							 MenuActualScreen.funcKeyLF=AjusteProductoHijokeyLF;
							 MenuActualScreen.funcKeyDN=AjusteProductoHijokeyDN;
							 MenuActualScreen.funcKeyEN=AjusteProductoHijokeyEN;
	//						 AjusteProductoHijo.menuPadre=AJUSTE_DE_PRODUCTO;
							 break;  
		 case PARTE_11:
							 MenuActualScreen.menuNodo=PARTE_11;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayParte11Menu; 
							 MenuActualScreen.funcKeyUP=Parte11MenukeyUP;
							 MenuActualScreen.funcKeyRT=Parte11MenukeyRT;	 
							 MenuActualScreen.funcKeyLF=Parte11MenukeyLF;
							 MenuActualScreen.funcKeyDN=Parte11MenukeyDN;
							 MenuActualScreen.funcKeyEN=Parte11MenukeyEN;
	//						 Parte11.menuPadre=INGENIERIA_GRACIDA;
							 break;  
		 case AJUSTE_DE_VIBRACION://Saliendo a ESTAS_SEGURO2
							 MenuActualScreen.menuNodo=AJUSTE_DE_VIBRACION;//igxm:bandera de demora para activar seguridad de ejecucion
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;//counter by timer to time delay
							 MenuActualScreen.func=DisplayAjusteVibracionMenu;   //igxm4:flag if configVib is Done.
							 MenuActualScreen.funcKeyUP=AjusteVibracionMenukeyUP;
							 MenuActualScreen.funcKeyRT=AjusteVibracionMenukeyRT;	 
							 MenuActualScreen.funcKeyLF=AjusteVibracionMenukeyLF;
							 MenuActualScreen.funcKeyDN=AjusteVibracionMenukeyDN;
							 MenuActualScreen.funcKeyEN=AjusteVibracionMenukeyEN;
	//						 AjusteVibracion.menuPadre=AJUSTE_DE_SISTEMA; //igxm1:flag ativated by timer to display menu EStas-Seguro2
							 break;  
		 case AJUSTE_DE_PRODUCTO_MODE:
							 MenuActualScreen.menuNodo=AJUSTE_DE_PRODUCTO_MODE;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayAjusteProductoMode; 
							 MenuActualScreen.funcKeyUP=AjusteProductoModekeyUP;
							 MenuActualScreen.funcKeyRT=AjusteProductoModekeyRT;	 
							 MenuActualScreen.funcKeyLF=AjusteProductoModekeyLF;
							 MenuActualScreen.funcKeyDN=AjusteProductoModekeyDN;
							 MenuActualScreen.funcKeyEN=AjusteProductoModekeyEN;
	//						 	AjusteProductoMode.menuPadre=AJUSTE_DE_PRODUCTO;
							 break;
		 case BORRAR_PRODUCTO:
							 MenuActualScreen.menuNodo=BORRAR_PRODUCTO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=displayBorrarProducto; 
							 MenuActualScreen.funcKeyUP=BorrarProductokeyUP;
							 MenuActualScreen.funcKeyRT=BorrarProductokeyRT;	 
							 MenuActualScreen.funcKeyLF=BorrarProductokeyLF;
							 MenuActualScreen.funcKeyDN=BorrarProductokeyDN;
							 MenuActualScreen.funcKeyEN=BorrarProductokeyEN;
	//						 BorrarProducto.menuPadre=AJUSTE_DE_PRODUCTO;
							 break;
		 case COPY_PRODUCTO:
							 MenuActualScreen.menuNodo=COPY_PRODUCTO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=displayCopyProducto; 
							 MenuActualScreen.funcKeyUP=CopyProductokeyUP;
							 MenuActualScreen.funcKeyRT=CopyProductokeyRT;	 
							 MenuActualScreen.funcKeyLF=CopyProductokeyLF;
							 MenuActualScreen.funcKeyDN=CopyProductokeyDN;
							 MenuActualScreen.funcKeyEN=CopyProductokeyEN;
				//			 CopyProducto.menuPadre=AJUSTE_DE_PRODUCTO;
							 break;                                          
		 case AJUSTE_PRODUCTO_SEL:
							 MenuActualScreen.menuNodo=AJUSTE_PRODUCTO_SEL;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=displayAjusteProductoSelect; 
							 MenuActualScreen.funcKeyUP=AjusteProductoSelectkeyUP;
							 MenuActualScreen.funcKeyRT=AjusteProductoSelectkeyRT;	 
							 MenuActualScreen.funcKeyLF=AjusteProductoSelectkeyLF;
							 MenuActualScreen.funcKeyDN=AjusteProductoSelectkeyDN;
							 MenuActualScreen.funcKeyEN=AjusteProductoSelectkeyEN;
				//			 AjusteProductoSelect.menuPadre=AJUSTE_DE_PRODUCTO;
							 break;
		 case NOMBRE_PRODUCTO:
							 MenuActualScreen.menuNodo=NOMBRE_PRODUCTO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=displayNombreProductoEdit; 
							 MenuActualScreen.funcKeyUP=NombreProductoEditkeyUP;
							 MenuActualScreen.funcKeyRT=NombreProductoEditkeyRT;	 
							 MenuActualScreen.funcKeyLF=NombreProductoEditkeyLF;
							 MenuActualScreen.funcKeyDN=NombreProductoEditkeyDN;
							 MenuActualScreen.funcKeyEN=NombreProductoEditkeyEN;
				//			 NombreProductoEdit.menuPadre=AJUSTES_AVANZADOS;
							 break; 
		 case ESTAS_SEGURO:
			 	 	 	 	 MenuActualScreen.menuNodo=ESTAS_SEGURO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayEstaSeguro; 
							 MenuActualScreen.funcKeyUP=EstaSeguro2keyUP;
							 MenuActualScreen.funcKeyRT=EstaSeguro2keyRT;	 
							 MenuActualScreen.funcKeyLF=EstaSeguro2keyLF;
							 MenuActualScreen.funcKeyDN=EstaSeguro2keyDN;
							 MenuActualScreen.funcKeyEN=EstaSeguro2keyEN;
				//			 EstaSeguro2.menuPadre=AJUSTES_AVANZADOS;
		 case ESTAS_SEGURO2:		 
							 MenuActualScreen.menuNodo=ESTAS_SEGURO2;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayEstaSeguro2; 
							 MenuActualScreen.funcKeyUP=EstaSeguro2keyUP;
							 MenuActualScreen.funcKeyRT=EstaSeguro2keyRT;	 
							 MenuActualScreen.funcKeyLF=EstaSeguro2keyLF;
							 MenuActualScreen.funcKeyDN=EstaSeguro2keyDN;
							 MenuActualScreen.funcKeyEN=EstaSeguro2keyEN;
				//			 EstaSeguro2.menuPadre=AJUSTES_AVANZADOS;
							 break;
		 case PASTE_PRODUCTO:		 
							 MenuActualScreen.menuNodo=PASTE_PRODUCTO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=DisplayPasteProduct; 
							 MenuActualScreen.funcKeyUP=PasteProductkeyUP;
							 MenuActualScreen.funcKeyRT=PasteProductkeyRT;	 
							 MenuActualScreen.funcKeyLF=PasteProductkeyLF;
							 MenuActualScreen.funcKeyDN=PasteProductkeyDN;
							 MenuActualScreen.funcKeyEN=PasteProductkeyEN;
				//			 EstaSeguro2.menuPadre=AJUSTES_AVANZADOS;
							 break;
		 case AJUSTE_TIEMPO_RECHAZO:
			 	 	 	 	 MenuActualScreen.menuNodo=AJUSTE_TIEMPO_RECHAZO;
							 MenuActualScreen.permisos=DESARROLLADOR_PERMISO;
							 MenuActualScreen.func=Display_Ajuste_Tiempo_Rechazo; 
							 MenuActualScreen.funcKeyUP=AjusteTiempoRechazokeyUP;
							 MenuActualScreen.funcKeyRT=AjusteTiempoRechazokeyRT;	 
							 MenuActualScreen.funcKeyLF=AjusteTiempoRechazokeyLF;
							 MenuActualScreen.funcKeyDN=AjusteTiempoRechazokeyDN;
							 MenuActualScreen.funcKeyEN=AjusteTiempoRechazokeyEN;
				//			 EstaSeguro2.menuPadre=AJUSTES_AVANZADOS;
							 break;     
	    
	 }//fin switch-----------------------------------------------------
	    
}// fin intializacione de Arbol Menu-------------------------------------------






/* PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU*******************************************************/
/* PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU*******************************************************/
/* PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU*******************************************************/


void DebugSimulationKeyboard(void){
static  char tecla;

#if(DONDE_ESTOY==EN_EL_TRABAJO)
	if(tecla!=0){
		switch(tecla){
	    	case 'w':menus(keyUP);break;
	    	case 'x':menus(keyDN);break;
	    	case 'a':menus(keyLF);break;
	    	case 'd':menus(keyRT);break;
	    	case 's':menus(keyEN);break;
	    	default:break;
	        }//keyDN keyUP keyRT keyLF
		tecla=0;  
	    }
#else
	if(tecla!=0){
			switch(tecla){
		    	case 8:menus(keyUP);break;
		    	case 2:menus(keyDN);break;
		    	case 4:menus(keyLF);break;
		    	case 6:menus(keyRT);break;
		    	case 5:menus(keyEN);break;
		    	case 'p': displayControladorPantalla(PORTAL_INICIO);break;
		    	default:break;
		        }//keyDN keyUP keyRT keyLF
			tecla=0;  
		    }
#endif	    
}//fin DebugSimulationKeyboard-----------------------------------------------


//MANEJADOR  PRINCIPAL DE EJECUCION DEL MENU++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void menus(unsigned char key){//MANEJADOR DEL LAS FUNCIONES COORDINADAS Y SUBORDINADAS DE LOS MENUS
          /* y tambien tiene entradas a SUB-MENUS EL MENUCONTROL SE ejecuta en el enter del case*/
     
	 switch(menu.contexto.Actual){
		    case PORTAL_INICIO:
		    case MENU_INSIGHT:
		    case AJUSTE_DE_PRODUCTO:
		    case AJUSTE_PARAMETRICO_DE_PRODUCTO: 	
			case AJUSTE_DE_SISTEMA:
			case MENU_ADMINISTRATIVO:
			case CONFIGURAR_ENTRADAS_DE_SISTEMA:
			case ESTAS_SEGURO:
			case ID_COMUNICACIONES:
			case CONTROL_DE_FRECUENCIA:
			case INFORMACION_DE_USUARIO:
			case MEDIDAS_DE_VOLTAJE:
			case MEDIDAS_DE_VOLTAJES:
			case ID_MAQUINA:
			case TIEMPO_DE_RECHAZOINFO:
			case AJUSTE_SISTEMA_ENTRADAS:
			case ESTABLECER_IDIOMA:
			case CONTROL_PASSWORD:
			case RELOJ_DE_SISTEMA:
			case INGENIERIA_GRACIDA:
			case INIT_MEMO_Y_REST:
			case TIPO_DE_MAQUINA:
			case GANANCIA_:
			case FILTRO_:
			case INGENIERIA_GRACIDA2:
			
			case AJUSTES_AVANZADOS:
			case FRECUENCIA_SELECT:
			case PRUEBA:
			case I2CMENU:
			case I2CEEPROMREAD:
			case I2CNVRAMDATA:
			case SPI_MENU:
			case PARAMETROS_DE_DETECCION:
			case DISPLAY_ADC:
			case PANTALLA_DDS:
			case PRUEBAS_SALIDAS:
			case PRUEBAS_ENTRADAS:
			case SELECCIONAR_PRODUCTO:
			case NUEVO_PRODUCTO:
			case TEXT_PROCESSOR:
			case AJUSTE_DE_PRODUCTO_HIJO:
			case PARTE_11:
			case AJUSTE_DE_VIBRACION:
			case ESTAS_SEGURO2:
			case AJUSTE_DE_PRODUCTO_MODE:
			case BORRAR_PRODUCTO:
			case COPY_PRODUCTO:
			case AJUSTE_PRODUCTO_SEL://Agregar el nombre del menu nuevo
			case NOMBRE_PRODUCTO://--------------------------------------------------------------------------------------3
			case PRUEBAS_SW_ANALOGA: 
			case PASTE_PRODUCTO:
			case AJUSTE_TIEMPO_RECHAZO:cursorMenuControl(key); return;
			default: break;
		 }//fin switch  
		  
//fin de  void menu(unsigned char key){//MANEJADOR DEL LAS FUNCIONES COORDINADAS Y SUBORDINADAS DE LOS MENUS	   
}// fin del Manejador principal de ejecucion del menu+++++++++++++++++++++++++++++++++++++++++++++++++++++



//ACTOR CONJUNTO EN ASOCIACION CON EL MENU MAESTRO-----------------------------
void MenuControl(void){//CONTROL MAESTRO DEL DESPLIEGUE DE LOS MENUS
	
	 
	 keypad.Status.bits.enable=FALSE;// disable_keyboard_IRQ(); 
	 DisplayMenu();//despliega el menu actual que se quiere crear
	 
	 

}//fin menu control------------------------------------------------------------



/*Description	Resource	Path	Location	Type
Undefined : "enable_keyboard_IRQ" Referenced from "displayTimer_IRQ" in	39_16_50_IN1UP		 	C/C++ Problem
 * 
 * */


void DisplayMenu(void){//DRIVER PRINCIPAL DEL DESPLIGUE DE LA PANTALLA-+++++++++++++++++++++++++++++++++++
 //-------------------------------------------m2a:poner bien su respectiv funcion
    contextobuffer=0;//variable auxiliar por contexto para manejo de desplieue de variables con digito
	initArbolMenu(menu.contexto.Actual);//menu que se va desplegar crear los pointers de la estructura de menu
    switch(menu.contexto.Actual){
       case PORTAL_INICIO:       
       case MENU_INSIGHT:       
       case AJUSTE_DE_PRODUCTO: 
       case AJUSTE_PARAMETRICO_DE_PRODUCTO: 	   
       case AJUSTE_DE_SISTEMA:  
       case MENU_ADMINISTRATIVO: 
       case CONFIGURAR_ENTRADAS_DE_SISTEMA: 
    	   //     case ESTAS_SEGURO:        DisplayEstaSeguro();        return;
       case ID_COMUNICACIONES:   
       case CONTROL_DE_FRECUENCIA:
       case INFORMACION_DE_USUARIO:
       case MEDIDAS_DE_VOLTAJE: 
       case MEDIDAS_DE_VOLTAJES: 
       case ID_MAQUINA:    		 
       case TIEMPO_DE_RECHAZOINFO:
       case AJUSTE_SISTEMA_ENTRADAS:
       case ESTABLECER_IDIOMA:    
       case CONTROL_PASSWORD:     
       case RELOJ_DE_SISTEMA: 	  
       case INIT_MEMO_Y_REST:     
       case INGENIERIA_GRACIDA:   
       case TIPO_DE_MAQUINA: 
       case GANANCIA_:         	 
       case FILTRO_:         
       case INGENIERIA_GRACIDA2: 
       

       case FRECUENCIA_SELECT: 
       case PRUEBA:       	   
       case I2CMENU:       	   
       case I2CEEPROMREAD:     
       case I2CNVRAMDATA:
       case SPI_MENU:    	 
       case AJUSTES_AVANZADOS:   // AjustesAvanzadosAPP.func();return;
       case PARAMETROS_DE_DETECCION:
       case DISPLAY_ADC:      
       case PANTALLA_DDS:  	      DDSaux=POSY0;
                                  
       case PRUEBAS_SALIDAS:      
       case PRUEBAS_ENTRADAS: 
       case SELECCIONAR_PRODUCTO: 
       case NUEVO_PRODUCTO:       
       case TEXT_PROCESSOR:       
       case AJUSTE_DE_PRODUCTO_HIJO: 
       case PARTE_11:             
       case AJUSTE_DE_VIBRACION:  
//     case ESTAS_SEGURO2:        DisplayEstaSeguro2();return;
       case AJUSTE_DE_PRODUCTO_MODE:
       case BORRAR_PRODUCTO:  	
       case COPY_PRODUCTO:   	  
       case AJUSTE_PRODUCTO_SEL:  
       case ESTAS_SEGURO2:
       case ESTAS_SEGURO:
       case NOMBRE_PRODUCTO:  
       case PASTE_PRODUCTO: 
       case AJUSTE_TIEMPO_RECHAZO:MenuActualScreen.func();return;//--Poner el case y el pointer function-----------5
//       case PRUEBAS_SW_ANALOfGA:PruebaSWanaloga.func();return;--------------
       default: break;    }//fin switch--------------------------------------------------------------
return;
}//FIN display menu DRIVER PRINCIPAL DE DESPLIEGUE DE PANTALLAS-----------------------------------------------


     
//CONTROL DEL CURSOR EN CADA MENU, keyx, ES LA  TECLA QUE SE PRESIONO
void cursorMenuControl(unsigned char keyx){//control del cursosr por cada menu
 const short int xpos=235,xpos2=239;
// unsigned char aux1;
 struct ArbolMenu *p=0;  
 
 //agregar el apuntador de funcion al apuntador
    switch(menu.contexto.Actual){//------------------------------------PONER EL APUNTADOR A ESTRUCTURA------------------------5
	    case PORTAL_INICIO:                  
	    case MENU_INSIGHT:                   
	    case AJUSTE_DE_PRODUCTO:  
	    case AJUSTE_PARAMETRICO_DE_PRODUCTO: 	
		case AJUSTE_DE_SISTEMA:              
		case MENU_ADMINISTRATIVO:            
		case CONFIGURAR_ENTRADAS_DE_SISTEMA: 
		case ESTAS_SEGURO:                   
		case ID_COMUNICACIONES:              
		case CONTROL_DE_FRECUENCIA:          
		case INFORMACION_DE_USUARIO:         
		case MEDIDAS_DE_VOLTAJE:  
		case MEDIDAS_DE_VOLTAJES: 
		case ID_MAQUINA:                     
		case TIEMPO_DE_RECHAZOINFO:          
		case AJUSTE_SISTEMA_ENTRADAS:        
		case ESTABLECER_IDIOMA:              
		case CONTROL_PASSWORD:               
		case RELOJ_DE_SISTEMA:                 
		case INGENIERIA_GRACIDA:             
		case INIT_MEMO_Y_REST:               
		case TIPO_DE_MAQUINA:                
		//case GANANCIA:                     p=0;break;
		//case FILTRO:                       p=0;break; 
		case INGENIERIA_GRACIDA2:            
		
		case AJUSTES_AVANZADOS:              
		case FRECUENCIA_SELECT:              
		case PRUEBA:                        
		case I2CMENU:                        
		case I2CEEPROMREAD:                  
		case I2CNVRAMDATA:                   
		case SPI_MENU:                       
		case PARAMETROS_DE_DETECCION:        
		case DISPLAY_ADC:                    
		case PANTALLA_DDS:                   
		case PRUEBAS_SALIDAS:                
		case PRUEBAS_ENTRADAS:               
		case SELECCIONAR_PRODUCTO:           
		case NUEVO_PRODUCTO:                 
		case TEXT_PROCESSOR:                 
		case AJUSTE_DE_PRODUCTO_HIJO:        
		case PARTE_11:                       
		case AJUSTE_DE_VIBRACION:            
		case ESTAS_SEGURO2: 
		case AJUSTE_DE_PRODUCTO_MODE:        
		case BORRAR_PRODUCTO:                
		case COPY_PRODUCTO:                  
		case AJUSTE_PRODUCTO_SEL:            
		case NOMBRE_PRODUCTO:                 
		case PRUEBAS_SW_ANALOGA: 
		case PASTE_PRODUCTO: 
		case AJUSTE_TIEMPO_RECHAZO:p=&MenuActualScreen;        
		default:                     break;        
	 }//fin switch--menu.contexto.Actual-------------
   if(p!=0)//diferente a cero
      cursorMenuControlService(keyx,p);
   else{
   
	switch(menu.contexto.Actual){
   //--CONTEXTO: ESTAS SEGURO???-------------------------------------------------------------------------------------
           case ESTAS_SEGURO2:   
           case ESTAS_SEGURO:menu.contexto.Actual=ESTAS_SEGURO;					     
                      switch(keyx){//SWITCH-7
                           case keyRT: break; 
                           case keyLF: break;
                           case keyEN:	break;//break del keyEN       	        
                                   }//fin switch 7
                          break; //break case ESTAS_SEGURO                     
                  
        default:break;//default del switch principal--------------------------------                        
  //**CONTEXTO:--------------------------------------------------------------------------------------------------------------------------------                                
   }//fin switch1 context menu ****************************************************************************

	
   }//FIN ELSE DE CURSO CONTROL SERVICE -------ESTE CODIGO SE IRA DISMINUYENDO
}//FIN CURSOR MENU CONTROL-------------------------------------------------------------------------------




//*****************************************************************************************************
//***FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE********************************************
//*****************************************************************************************************
//*****************************************************************************************************
//********************************* FUNCIONES EJECUTIVAS MAESTRAS***DE DESPLIEGUE*************************
//******************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU*****************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU*****
//*******************************************************************************************************
//***********FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE***************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*****************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU**
//*******************************************************************************************************
//***********FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE******************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE******************************************
//*******************************************************************************************************
//******************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU
//*******************************************************************************************************
//*****************************************************************************************************
//***FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE********************************************
//*****************************************************************************************************
//*****************************************************************************************************
//********************************* FUNCIONES EJECUTIVAS MAESTRAS***DE DESPLIEGUE*************************
//******************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU*****************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU*****
//*******************************************************************************************************
//***********FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE***************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*****************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU**
//*******************************************************************************************************
//***********FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE******************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE******************************************
//*******************************************************************************************************
//******************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU
//*******************************************************************************************************
//*****************************************************************************************************
//***FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE********************************************
//*****************************************************************************************************
//*****************************************************************************************************
//********************************* FUNCIONES EJECUTIVAS MAESTRAS***DE DESPLIEGUE*************************
//******************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU*****************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU*****
//*******************************************************************************************************
//***********FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE***************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*****************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU**
//*******************************************************************************************************
//***********FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE******************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******************************************************************************************************
//*******FUNCIONES DE  EJECUTIVAS*MAESTRAS*DE*DESPLIEGUE******************************************
//*******************************************************************************************************
//******************************************************PROCEDIMIENTOS PRINCIPALES DE EJECUCION DEL MENU
//*******************************************************************************************************





/* this method is filted to be executed w/o error of context failure, 'cos
 * the filter it has been pass through, it control the execution concequence of every key */
void cursorMenuControlService(unsigned char key,struct ArbolMenu *Pantalla){//(key,void (*a)())
	   switch(key){case keyUP:Pantalla->funcKeyUP();break;
		           case keyDN:Pantalla->funcKeyDN();break;
		           case keyRT:Pantalla->funcKeyRT();break;
		           case keyLF:Pantalla->funcKeyLF();break;
		           case keyEN:Pantalla->funcKeyEN();break;
		         }//fi		           default:break;//fin default de segundo switch de PORTAL INICIO
}//fin cursorMenuControlService------------------------------------------------------













void PortalinicioDisplay(void){//Funcion Suprema de Despliegue de la Pantalla0
	//unsigned char s[]=ss;//"Modo Pruebas1  v35.0.2";
	unsigned char s[]=VERSION2;
	unsigned char *ss;
	unsigned char  a[]="Test Set-up";
    unsigned char  b[]="S                P";
    unsigned char  c[]="Cuenta rechazos 0";
    unsigned char  d[]="Cuenta producto 0";
    unsigned char x[4]={80,24,24,24};
    unsigned char y[4]={ 2,10,12,14};
  		  
	word n,m;  //evitar quese vean puntos cuando salimos del DDS1Z3257XY0491765951
	    Deteccion.CuadroMadreReady=FALSE;
	    keypad.Status.bits.enable=FALSE; //  disable_keyboard_IRQ();
	    if(menu.contexto.Anterior==PANTALLA_DDS)	
	    	VFDclrscr();
	    else{
	    	VFDclrscr();
	    	delay_ms(200);}
		VFDposicion(65,0); //no borra el puntito lo de  arriba  
	    VFDserial_SendBlock(&s[0],sizeof(s),&n);
	    
	    ss=&name[1];m=length(&name[0],sizeof(name));
	    x[0]=display_centrarNombres((unsigned char)m);
	    VFDposicion(x[0],2);
	    VFDserial_SendBlock(ss,m,&n);//nombre de producto display
	    
	    //nombre de producto y cuentas
	    //VFDposicion(x[0],y[0]);   delay1us(); deprected
	    //VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us(); deprecated, display a fix name.
	    VFDposicion(x[1],y[1]);   delay1us();
	    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
	    VFDposicion(x[2],y[2]);   delay1us();
	    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
	    VFDposicion(x[3],y[3]);   delay1us();
	    VFDserial_SendBlock(&d[0],sizeof(d),&n);delay1us();
	      
	    displayCuadroMadre();
	    delay_ms(10);//esperar que se despliegue el cuadro madre.
	    Deteccion.CuadroMadreReady=TRUE;
	             	    
	    //VFDclrscr();
	   	//VFDposicion(65,2);   
	   	//VFDserial_SendBlock(&s[0],sizeof(s),&n);
	    
	    init_Sensibilidad();
	   	//Timer_enable_keyboard_IRQ();
	    passControl[_ORIGEN_]=0;//Resetear las claves y su control
	    passControl[_DESTIN_]=0;
	    passControl[_AUTHOR_]=0;
	    
}// FIN DESPLIEGUE DEL PORTAL INICIO-------------------------------------------------------------------




void displayMenuInsight(void){// MESPLIEGUE DEL MENU INSGHT----------------------------------------------
//unsigned short int i;
	  unsigned char z[]="Menu de Insight       x";
	  unsigned char a[]=">Ajuste de Producto";
	  unsigned char c[]="Ajuste Parametrico de Producto";
	  unsigned char w[]="Ajuste de sistema";
	  unsigned char v[]="Pantalla DDS";
	  unsigned char b[]="Informacion del usuario";
	  unsigned char x[7]={65,0,8,8, 8, 8,8};
	  unsigned char y[7]={0 ,4,6,8,10,12,4};
		word n;
		    
		    VFDclrscrForce();
		    VFDposicion(x[0],y[0]);   delay1us();
		    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
			VFDposicion(x[1],y[1]);   delay1us();
		    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
			VFDposicion(x[2],y[2]);   delay1us();
		    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			VFDposicion(x[3],y[3]);   delay1us();
			VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
			VFDposicion(x[4],y[4]);   delay1us();
		    VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
			VFDposicion(x[5],y[5]);   delay1us();
		    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		    VFDposicion(x[6],y[6]);//posiCION del cursor despues de desplegar el menu  
		    cursorx=POSX0;cursory=POSY4;
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.
}//fin display menu inicio display----------------------------------------------------------------------------




void displayMenuAjustedeProducto(void){
	  unsigned char z[]="Menu de Ajuste de Producto  x";
	  unsigned char a[]=">Hacer prueba de ejemplo";
	  unsigned char c[]="Seleccionar producto";
	  unsigned char w[]="Nuevo producto";
	  unsigned char v[]="Ajuste de producto";
	  unsigned char b[]="Borrar Producto";
	  unsigned char q[]="Copiar Producto"; 
	  unsigned char x[7]={20,0,8,8, 8, 8, 8};
	  unsigned char y[7]={0 ,4,6,8,10,12,14};
		word n;
		    VFDclrscr();
			VFDposicion(x[0],y[0]);   delay1us();
		    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
			VFDposicion(x[1],y[1]);   delay1us();
		    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
			VFDposicion(x[2],y[2]);   delay1us();
		    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			VFDposicion(x[3],y[3]);   delay1us();
			VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
			VFDposicion(x[4],y[4]);   delay1us();
		    VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
			VFDposicion(x[5],y[5]);   delay1us();
		    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		    VFDposicion(x[6],y[6]);   delay1us();
		    VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
		    //VFDposicion(x[7],y[7]);//posiCION del cursor despues de desplegar el menu  
		    cursorx=POSX0;cursory=POSY4;
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin display menu del ajuste de producto------------------------------------------------------------------


void DisplayMenuAjustedeSistema(void){//despliega el menu del AJUSTE DE SISTEMA-------------
	  unsigned char z[]="Menu de Ajuste de Sistema   x";
	  unsigned char a[]=">Menu Administrativo           ";
	  unsigned char c[]=" Ajuste de vibracion           ";
	  unsigned char w[]=" Configurar entradas de sistema";
	  unsigned char v[]=" Control de frecuencia";
	  unsigned char b[]=" ID de comunicaciones";
	  unsigned char q[]=" Ingenieria Gracida"; 
	  unsigned char x[7]={20,0,0,0,0, 0, 0};
	  unsigned char y[7]={0 ,4,6,8,10,12,14};
	  word n;
      VFDclrscr();
			  VFDposicion(x[0],y[0]);   delay1us();
		      VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
		  	  VFDposicion(x[1],y[1]);   delay1us();
		      VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
			  VFDposicion(x[2],y[2]);   delay1us();
		      VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			  VFDposicion(x[3],y[3]);   delay1us();
			  VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
			  VFDposicion(x[4],y[4]);   delay1us();
		      VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
			  VFDposicion(x[5],y[5]);   delay1us();
		      VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		      VFDposicion(x[6],y[6]);   delay1us();
		      VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
		      //VFDposicion(x[7],y[7]);//posiCION del cursor despues de desplegar el menu  
		      cursorx=POSX0;cursory=POSY4;
			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

	
}//fin display del menu de AJUSTE DE SISTEMA-----------------------------------------------------------------


void  DisplayMenuAdministrativo(void){//---------------------5M:CREACION DEL PROCEDIMIENTO DE DESPLIEGUE DEL MENU
//---------------------------------------------------6M:llenado del procemiento de despliegue, copiando uno parecido anterior y modificando lo que se necesite
	  unsigned char r[]=" x";
	  unsigned char z[]=">Control de contrasenas";
	  unsigned char a[]="Reloj del sistema";
	  unsigned char c[]="Recordatorio de pruebas";
	  unsigned char w[]="Establecer idioma";
	  unsigned char v[]="Inicializar Memoria y restaurar";
	  unsigned char b[]="Ajuste de guardia";//en revision--en desarrollo, debemmos leer el valor de la variable en 1 y0
	  unsigned char q[]="Control de usuario"; 
	  unsigned char x[8]={235,0,8,8,8, 8, 8, 8};
	  unsigned char y[8]={  0,2,4,6,8,10,12,14};
		word n;
		    VFDclrscr();
			VFDposicion(x[0],y[0]);   delay1us();
			VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
			VFDposicion(x[1],y[1]);   delay1us();
		    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
			VFDposicion(x[2],y[2]);   delay1us();
		    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
			VFDposicion(x[3],y[3]);   delay1us();
		    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			VFDposicion(x[4],y[4]);   delay1us();
			VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
			VFDposicion(x[5],y[5]);   delay1us();
		    VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
			VFDposicion(x[6],y[6]);   delay1us();
		    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		    VFDposicion(x[7],y[7]);   delay1us();
		    VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
		    //VFDposicion(x[7],y[7]);//posiCION del cursor despues de desplegar el menu
		    displayAjusteGuardiaControl();
		    displayControldeUsuariovar();
		    cursorx=POSX0;cursory=POSY2;
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// fin del procedimiento de despliegue del menu administrativo---------------------------------------------	
	
//Display
void  DisplayConfigIOsistem(void){//--------------------------------------------------------------------5M:CREACION DEL PROCEDIMIENTO DE DESPLIEGUE DEL MENU
//----------------------------------------------------------------------------------------------------------6M:llenado del procemiento de despliegue, copiando uno parecido anterior y modificando lo que se necesite
	  unsigned char r[]="Configurar entradas de sistema x";
      unsigned char z[]="   Entradas          Permitir";
	  unsigned char a[]=">Entrada de taco";
	  unsigned char c[]="Entrada de fotocel";
	  unsigned char w[]="Test entrada rechazo";
	  unsigned char v[]="Test entrada producto";//en revision, debemos leer el valor de la variable si es uno o cero
	  unsigned char b[]="Entrada auxiliar1";
	  unsigned char q[]="Entrada auxiliar2"; 
	  unsigned char x[8]={0,0,0,8,8, 8, 8, 8};	
	  unsigned char y[8]={0,2,4,6,8,10,12,14};
		word n;
		    VFDclrscr();
			VFDposicion(x[0],y[0]);   delay1us();
			VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
			VFDposicion(x[1],y[1]);   delay1us();
		    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
			VFDposicion(x[2],y[2]);   delay1us();
		    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
			VFDposicion(x[3],y[3]);   delay1us();
		    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			VFDposicion(x[4],y[4]);   delay1us();
			VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
			VFDposicion(x[5],y[5]);   delay1us();
		    VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
			VFDposicion(x[6],y[6]);   delay1us();
		    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		    VFDposicion(x[7],y[7]);   delay1us();
		    VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
		    //VFDposicion(x[7],y[7]);//posiCION del cursor despues de desplegar el menu  
		    displayEntradadeSistemas(_TACOMETRO_);
		    displayEntradadeSistemas(_FOTOCELDA_);
		    displayEntradadeSistemas(_RECHAZO_);
		    displayEntradadeSistemas(_PRODUCTO_);
		    displayEntradadeSistemas(_AUX1_);
		    displayEntradadeSistemas(_AUX2_);
		    cursorx=POSX0;cursory=POSY4;
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// fin del procedimiento de despliegue dE CONFIGuracion de entradas del sistema------------------------------	
	

void DisplayEstaSeguro(void){
    	  unsigned char r[]="  Guardar Cambios";
		  unsigned char z[]="    Estas Seguro?";
		  unsigned char a[]=">NO        si";
		 // unsigned char c[]="si";
		  unsigned char x[3]={0,0, 0};
		  unsigned char y[3]={4,6,10};
		  word n,w;
		  unsigned char *p;
			    VFDclrscr();
				VFDposicion(x[0],y[0]);   delay1us();
				VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
				VFDposicion(x[1],y[1]);   delay1us();
			    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
				VFDposicion(x[2],y[2]);   delay1us();
			    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
				cursorx=POSX0;cursory=POSY10;
				
//este codigo debe ser el mismo para ESTAS-SEGURO y ESTAS-SEGURO2
		//Version-6
	    switch(menu.contexto.Modificado){			
	      case NUEVO_PRODUCTO://desplegar el nuevo nombre que queremos guardar
			                    p=&Text[0];
			                    w=sizeof(Text);//el cero no se debe imprimir tama�o del nombre a imprimir
			                    break;
	      case SELECCIONAR_PRODUCTO:
	      case BORRAR_PRODUCTO:
	      case PASTE_PRODUCTO:
			                  p=memoEEPROM_SelProd(arg0);//saca el nombre del Prod numero arg0
			                  w=length2(p);
			                  break;
	      default:break;}		   
		 n=length2(p);
		 x[0]=display_centrarNombres((unsigned char)n);
		 VFDposicion(x[0],y[0]); delay1us();
		 VFDserial_SendBlock(++p,w,&n);delay1us();
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.


}//fin esta sseguro----------------------------------------------------------------------------------

/* FUNCION ESPECIAL PARA TAPAR LA BARRA DE DETECCION DE CUADROS DINAMICOS QUE NO SE LIMPIAN TAN FACIL*/
void DisplayEstaSeguro2(void){//Este display es esepcial para despues de una barra de deteccion
	      unsigned char a[]="                                ";
		  unsigned char b[]="                           ";    	      
    	  unsigned char c[]="  Guardar Cambios              ";
		  unsigned char d[]="    Estas Seguro?              ";
		  unsigned char e[]="                           ";
		  unsigned char f[]=">NO        si              ";
		  unsigned char g[]="                           ";
		  unsigned char h[]="                           ";
		 // unsigned char c[]="si";
		  unsigned char x[8]={0,0,0,0,0, 0, 0, 0};
		  unsigned char y[8]={0,2,4,6,8,10,12,14};
		  word n,w;
		  unsigned char *p;
			    //VFDclrscr();delay1us();
			    VFDclrscrForce();
				VFDposicion(x[0],y[0]);   delay1us();
				VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
				VFDposicion(x[1],y[1]);   delay1us();
			    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
				VFDposicion(x[2],y[2]);   delay1us();
			    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			    VFDposicion(x[3],y[3]);   delay1us();
			    VFDserial_SendBlock(&d[0],sizeof(d),&n);delay1us();
			    VFDposicion(x[4],y[4]);   delay1us();
			    VFDserial_SendBlock(&e[0],sizeof(e),&n);delay1us();
			    VFDposicion(x[5],y[5]);   delay1us();
			    VFDserial_SendBlock(&f[0],sizeof(f),&n);delay1us();
			    VFDposicion(x[6],y[6]);   delay1us();
			    VFDserial_SendBlock(&g[0],sizeof(g),&n);delay1us();
			    VFDposicion(x[7],y[7]);   delay1us();
			    VFDserial_SendBlock(&h[0],sizeof(h),&n);delay1us();		    
				cursorx=POSX0;cursory=POSY10;
			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

//este codigo debe ser el mismo para ESTAS-SEGURO y ESTAS-SEGURO2
		//Version-6
		switch(menu.contexto.Modificado){			
		  case NUEVO_PRODUCTO://desplegar el nuevo nombre que queremos guardar
								p=&Text[0];
								w=sizeof(Text);//el cero no se debe imprimir tama�o del nombre a imprimir
								break;
		  case SELECCIONAR_PRODUCTO:
		  case BORRAR_PRODUCTO:
		  case PASTE_PRODUCTO:
							  p=memoEEPROM_SelProd(arg0);//saca el nombre del Prod numero arg0
							  w=length2(p);
							  break;
		  default:break;}		   
		 n=length2(p);
		 x[0]=display_centrarNombres((unsigned char)n);
		 VFDposicion(x[0],y[0]); delay1us();
		 VFDserial_SendBlock(++p,w,&n);delay1us();
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

	
}//fin esta sseguro----------------------------------------------------------------------------------


unsigned char display_centrarNombres(unsigned char nchars){
const unsigned char middle=256/2;
const unsigned char charpixel=8;
	switch(nchars){
		case 0:return(middle);break;
		case 1:return(middle);break;//---*---
		case 2:return(middle-charpixel);break;
		case 3:return(middle-charpixel);break;
		case 4:return(middle-(charpixel*2));break;
		case 5:return(middle-(charpixel*3));break;
		default: return(middle-((nchars/2)*charpixel));break;	
	  }

}//fin unsigned char display_centrarNombres(unsigned char nchars){------------------------------------





void DisplayIDcomunicaciones(void){//esta es una hoja ejecutiva
	// unsigned char num=109;
	  unsigned char r[]="  x";
	  unsigned char z[]="ID de comunicaciones:>";
	  //unsigned char a[]="";
			 // unsigned char c[]="si";
			  unsigned char x[3]={POSXESQUINA,0,POSXNUM};
			  unsigned char y[3]={0,4,4};
				word n;
				    VFDclrscr();
					VFDposicion(x[0],y[0]);   delay1us();
					VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
					VFDposicion(x[1],y[1]);   delay1us();
				    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
					VFDposicion(x[2],y[2]);   delay1us();
					if(IDcomm>9){IDcomm=0;}
					VFDserial_SendChar(0x30U+IDcomm);
				    //VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
					cursorx=POSXNUM;cursory=POSY4;
	
}//fin display EL  id  de comunicaciones-------------------------------------------------------------------------------



void DisplayControlFrecuencias(void){
	      unsigned char r[]="Graduar frecuencia       x";
		  unsigned char z[]=">Alta        87 khz";
		  unsigned char a[]= "Media       28 khz";
		  unsigned char c[]= "Baja        10 khz";
		  unsigned char w[]= "Pantalla DDS";
		  unsigned char x[8]={35,0,8, 8, 8};
		  unsigned char y[8]={ 0,4,8,12,14};
			word n;
			    VFDclrscr();
				VFDposicion(x[0],y[0]);   delay1us();
				VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
				VFDposicion(x[1],y[1]);   delay1us();
			    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
				VFDposicion(x[2],y[2]);   delay1us();
			    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
				VFDposicion(x[3],y[3]);   delay1us();
			    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
				VFDposicion(x[4],y[4]);   delay1us();
				VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
				
				VFDposicion(POSXCFNUM,POSY4);   delay1us();
				VFDserial_SendChar(Alta+0x30U);
				VFDposicion(POSXCFNUM,POSY8);   delay1us();
				VFDserial_SendChar(Media+0x30U);
				VFDposicion(POSXCFNUM,POSY12);   delay1us();
				VFDserial_SendChar(Baja+0x30U);
				
				cursorx=POSXCFNUM;cursory=POSY4;   
			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//display control de frecuencias-----------------------------------------------------------------------------------------------------------

void DisplayMenuInformacionUsuario(void){
	      unsigned char r[]=" x";
		  unsigned char z[]=">Hacer pruebas de ruido";
		  unsigned char a[]="Medidas de voltaje";
		  unsigned char c[]="Ajustes de producto";
		  unsigned char w[]="Tiempos de Rechazo";
		  unsigned char v[]="Ajuste sistema de entradas";
		  unsigned char b[]="ID de maquinas";//en revision--en desarrollo, debemmos leer el valor de la variable en 1 y0
		  unsigned char q[]="Cuenta producto"; 
		  unsigned char x[8]={POSXESQUINA,0,8,8,8, 8, 8, 8};
		  unsigned char y[8]={          0,2,4,6,8,10,12,14};
			word n;
			    VFDclrscr();
				VFDposicion(x[0],y[0]);   delay1us();
				VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
				VFDposicion(x[1],y[1]);   delay1us();
			    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
				VFDposicion(x[2],y[2]);   delay1us();
			    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
				VFDposicion(x[3],y[3]);   delay1us();
			    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
				VFDposicion(x[4],y[4]);   delay1us();
				VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
				VFDposicion(x[5],y[5]);   delay1us();
			    VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
				VFDposicion(x[6],y[6]);   delay1us();
			    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
			    VFDposicion(x[7],y[7]);   delay1us();
			    VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
			    //VFDposicion(x[7],y[7]);//posiCION del cursor despues de desplegar el menu  
			    cursorx=POSX0;cursory=POSY2;//POSICION DE LA FLECHA LA DEJAMOS EN 0,2
			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin Display Menu INFORMATION  usuarioo-------------------------------------------------------------


void DisplayIngenieriaGracida(void){
	unsigned char r[]=">Establecer tipo de maquina   x";
	unsigned char z[]="Ganancia";
	unsigned char a[]="Filtro";
	unsigned char c[]="Escribir identificacion";
	unsigned char w[]="Control de frecuencias";
	unsigned char v[]="Parte-11";//en revision, debemos leer el valor de la variable si es uno o cero
	unsigned char b[]="Paso Gracida";
	unsigned char q[]="Ingenieria Gracida2"; 
	unsigned char x[8]={0,8,8,8,8, 8, 8, 8};
	unsigned char y[8]={0,2,4,6,8,10,12,14};
	word n;
	    VFDclrscr();
		VFDposicion(x[0],y[0]);   delay1us();
		VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
		VFDposicion(x[1],y[1]);   delay1us();
	    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
		VFDposicion(x[2],y[2]);   delay1us();
	    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
		VFDposicion(x[3],y[3]);   delay1us();
	    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
		VFDposicion(x[4],y[4]);   delay1us();
		VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
		VFDposicion(x[5],y[5]);   delay1us();
	    VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
		VFDposicion(x[6],y[6]);   delay1us();
	    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
	    VFDposicion(x[7],y[7]);   delay1us();
	    VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
	    //VFDposicion(x[7],y[7]);//posiCION del cursor despues de desplegar el menu  
	    cursorx=POSX0;cursory=POSY0; 
	    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

	
}// fin del display de ingenieria-Gracida------------------------------------------


void DisplayInitMemoria_y_Restaurar(void){ // funcion principal de despliegue de inicializar memoria y restaurar
     	unsigned char r[]="Inicializar memoria";
		unsigned char z[]="Estas seguro?";
		unsigned char c[]="NO";
		unsigned char w[]="si";
		unsigned char x[4]={8,10, 8,125};
		unsigned char y[4]={4, 8,12, 12};
		word n;
		    VFDclrscr();
			VFDposicion(x[0],y[0]);   delay1us();
			VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
			VFDposicion(x[1],y[1]);   delay1us();
		    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
			VFDposicion(x[2],y[2]);   delay1us();
		    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			VFDposicion(x[3],y[3]);   delay1us();
			VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();  
		    cursorx=POSX0;cursory=POSY4; 
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

} //fin display initialize memoria and---------------------------------------------



void DisplayTipodeMaquina(void){
//---------------------123456789ABCDEF123456789ABCDEF123		
	 unsigned char r[]="Tipo de Maquina               x";
     unsigned char z[]="Multi(100,286,875)";
	 unsigned char a[]="Multi(30,100,286)";
	 unsigned char c[]="Single(875,286)";
	 unsigned char w[]="Single(100,30)";
	 unsigned char v[]="VF(875,286)";//en revision, debemos leer el valor de la variable si es uno o cero
	 unsigned char b[]="VF(100,30)";
	 unsigned char q[]="Pharmaceutical"; 
	 unsigned char x[8]={0,8,8,8,8, 8, 8, 8};
	 unsigned char y[8]={0,2,4,6,8,10,12,14};
		word n;
		    VFDclrscr();
			VFDposicion(x[0],y[0]);   delay1us();
			VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
			VFDposicion(x[1],y[1]);   delay1us();
		    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
			VFDposicion(x[2],y[2]);   delay1us();
		    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
			VFDposicion(x[3],y[3]);   delay1us();
		    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			VFDposicion(x[4],y[4]);   delay1us();
			VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
			VFDposicion(x[5],y[5]);   delay1us();
		    VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
			VFDposicion(x[6],y[6]);   delay1us();
		    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		    VFDposicion(x[7],y[7]);   delay1us();
		    VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
		    //VFDposicion(x[7],y[7]);//posiCION del cursor despues de desplegar el menu 
		    cursorx=POSX0;
		    switch(tipo_deMaquina){
		      case MULTI_100_286_875:cursory=POSY2; break;
		      case MULTI_30_100_26:cursory=POSY4;break;
		      case SINGLE_875_286: cursory=POSY6;break;
		      case SINGLE_100_30:  cursory=POSY8;break;  
		      case VF_875_286:     cursory=POSY10;break;
		      case VF_100_30:      cursory=POSY12;break;
		      case PHARMACEUTICAL: cursory=POSY14;break;
		      default:tipo_deMaquina=PHARMACEUTICAL;cursory=POSY14; break;
		    }//FIN witch tipo_deMaquina
		    VFDposicion(cursorx,cursory);
		    VFDserial_SendChar('>'); 
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// fin display Establecer tipo de Maquina-------------------------------------------


void DisplayIngenieriaGracida2(void){
	unsigned char a[]=" x";
	unsigned char b[]=VERSION_FIRMWARE;
	unsigned char c[]=">Inizializar memoria"; //2
	unsigned char d[]="PSU E/S";              //4
	unsigned char e[]="Error reloj Analogo"; //6
	unsigned char f[]="Ajustar tolerancia";// 8en revision, debemos leer el valor de la variable si es uno o cero
	unsigned char h[]="Reporte de produstos"; //10
	unsigned char i[]="Prueba";               //12
	unsigned char j[]="Auto-secuencia BIOS";   //14
	unsigned char x[9]={POSXESQ235,0,0,8,8, 8, 8, 8, 8};
	unsigned char y[9]={         0,0,2,4,6, 8,10,12,14};
	word n;
	    VFDclrscr();
		VFDposicion(x[0],y[0]);   delay1us();
		VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
		VFDposicion(x[1],y[1]);   delay1us();
	    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		VFDposicion(x[2],y[2]);   delay1us();
	    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
		VFDposicion(x[3],y[3]);   delay1us();
	    VFDserial_SendBlock(&d[0],sizeof(d),&n);delay1us();
		VFDposicion(x[4],y[4]);   delay1us();
		VFDserial_SendBlock(&e[0],sizeof(e),&n);delay1us();
		VFDposicion(x[5],y[5]);   delay1us();
	    VFDserial_SendBlock(&f[0],sizeof(f),&n);delay1us();
		VFDposicion(x[6],y[6]);   delay1us();
	    VFDserial_SendBlock(&h[0],sizeof(h),&n);delay1us();
	    VFDposicion(x[7],y[7]);   delay1us();
	    VFDserial_SendBlock(&i[0],sizeof(i),&n);delay1us();
	    VFDposicion(x[8],y[8]);
	    VFDserial_SendBlock(&j[0],sizeof(j),&n);delay1us();
	    displayPSU_IO_var();
	    displayErrorAnalogoVar();
	    displayReporteProductoVar();
	    cursorx=POSX0;cursory=POSY2; 
	    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin display ingenieria2 gracida---------------------------------------------------





void DisplayAjusteParamProd(void){
	unsigned char a[]=" x";
	//unsigned char b[]=VERSION_FIRMWARE;
	//unsigned char c[]=">Inizializar memoria"; //2
	//unsigned char d[]="PSU E/S";              //4
	unsigned char e[]=">Sensibilidad"; //6
	unsigned char f[]="Fase         ";// 8en revision, debemos leer el valor de la variable si es uno o cero
	unsigned char h[]="Borrar contadores"; //10
	unsigned char i[]="Pantalla DDS";               //12
	unsigned char j[]="Ajustes Avanzados";   //14
	unsigned char x[9]={POSXESQ235,0,0,8,0, 8, 8, 8, 8};
	unsigned char y[9]={         0,0,2,4,6, 8,10,12,14};
	word n; //MENU AJUSTE PARAMETRICO DE PRODUCTO
	    VFDclrscr();
	    AjParamProd=&g;
		VFDposicion(x[0],y[0]);   delay1us();
		VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
		VFDposicion(x[4],y[4]);   delay1us();
	    VFDserial_SendBlock(&e[0],sizeof(e),&n);delay1us();
		VFDposicion(x[5],y[5]);   delay1us();
	    VFDserial_SendBlock(&f[0],sizeof(f),&n);delay1us();
		VFDposicion(x[6],y[6]);   delay1us();
	    VFDserial_SendBlock(&h[0],sizeof(h),&n);delay1us();
	    VFDposicion(x[7],y[7]);   delay1us();
	    VFDserial_SendBlock(&i[0],sizeof(i),&n);delay1us();
	    VFDposicion(x[8],y[8]);
	    VFDserial_SendBlock(&j[0],sizeof(j),&n);delay1us();
	    displayUINT_var(POSXAJUSPROD,POSY6,&Deteccion.Sensibilidad,NONE);
	    displayPhase_var(POSXAJUSPROD,POSY8);
	   // displayFaseVar();
	    displayCuadroMadre();//despliega el cuadro de barra de deteccion
	    cursorx=POSX0;cursory=POSY6; 
	    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.
        AjParamProd->igxc0=0; //variable que indica si se esta editando el numero sensibildad y Fase
}//fin display ingenieria2 gracida---------------------------------------------------



void DisplayAjustesAvanzados(void){
unsigned short int t;	
	    unsigned char a[]=" x";
		//unsigned char b[]=VERSION_FIRMWARE;
		//unsigned char c[]=">Inizializar memoria"; //2
		//unsigned char d[]="PSU E/S";              //4
		//unsigned char e[]=">Sensibi "; //6
	        
		unsigned char f[]=">Nombre de producto";// 8en revision, debemos leer el valor de la variable si es uno o cero
		unsigned char h[]="Parametros de deteccion"; //10
		unsigned char i[]="Ajustes tiempo de rechazo";//12     
#if (_TIPO_DE_MAQUINA_==TRIPLE_FRECUENCIA)	
		unsigned char j[]="Frecuencia";   //14
#endif	
		unsigned char x[9]={POSXESQ235,0,0,8,0, 0, 8, 8, 8};
		unsigned char y[9]={         0,0,2,4,6, 8,10,12,14};
		word n;
		    t=20;
		    delay_us(t);
		    VFDclrscr();
		    delay_us(t);
		    VFDclrscrForce();
			VFDposicion(x[0],y[0]);   delay1us();
			VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
			VFDposicion(x[5],y[5]);   delay1us();
		    VFDserial_SendBlock(&f[0],sizeof(f),&n);delay1us();
			VFDposicion(x[6],y[6]);   delay1us();
		    VFDserial_SendBlock(&h[0],sizeof(h),&n);delay1us();
		    VFDposicion(x[7],y[7]);   delay1us();
		    VFDserial_SendBlock(&i[0],sizeof(i),&n);delay1us();
#if (_TIPO_DE_MAQUINA_==TRIPLE_FRECUENCIA)		    
		    VFDposicion(x[8],y[8]);
		    VFDserial_SendBlock(&j[0],sizeof(j),&n);delay1us();
#endif
		    //desplegar el nombre del producto actual seleccionado
		    VFDposicion(40,POSY0);//desplegar el producto actual
		    //deprecated VFDserial_SendBlock(&name[1],sizeof(name),&n);delay1us();
		    display_Name_Product(CENTRAL,POSY0);
		    cursorx=POSX0;cursory=POSY8; 
		    
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

		    
}// fin display Ajustes Avanzados-----------------------------------------------------


void  DisplayPruebaMenu(void){//---------------------5M:CREACION DEL PROCEDIMIENTO DE DESPLIEGUE DEL MENU
//---------------------------------------------------6M:llenado del procemiento de despliegue, copiando uno parecido anterior y modificando lo que se necesite
	  unsigned char r[]=" x";
	  unsigned char z[]=">I2C devices";
	  unsigned char a[]="I2C eeprom data";
	  unsigned char c[]="I2C NVRAM data";
	  unsigned char w[]="SPI tests";
	  unsigned char v[]="Salidas Pruebas";
	  unsigned char b[]="Entradas Pruebas";//en revision--en desarrollo, debemmos leer el valor de la variable en 1 y0
	  unsigned char q[]="Medidas de voltaje"; 
	  unsigned char x[8]={235,0,8,8,8, 8, 8, 8};
	  unsigned char y[8]={  0,2,4,6,8,10,12,14};
		word n;
		    VFDclrscr();
			VFDposicion(x[0],y[0]);   delay1us();
			VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
			VFDposicion(x[1],y[1]);   delay1us();
		    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
			VFDposicion(x[2],y[2]);   delay1us();
		    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
			VFDposicion(x[3],y[3]);   delay1us();
		    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			VFDposicion(x[4],y[4]);   delay1us();
			VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
			VFDposicion(x[5],y[5]);   delay1us();
		    VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
			VFDposicion(x[6],y[6]);   delay1us();
		    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		    VFDposicion(x[7],y[7]);   delay1us();
		    VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
		    //VFDposicion(x[7],y[7]);//posiCION del cursor despues de desplegar el menu  
		    cursorx=POSX0;cursory=POSY2;
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// fin del procedimiento de despliegue del menu administrativo---------------------------------------------	
	
void DisplayAjustesAvanzadosAPP(void){
	  unsigned char r[]=" x";
	  unsigned char z[]=" ";
	  unsigned char a[]=" ";
	  unsigned char c[]=" ";
	  unsigned char w[]=" ";
	  unsigned char v[]=">Nombre de Producto";
	  unsigned char b[]="Parametros de deteccion";//en revision--en desarrollo, debemmos leer el valor de la variable en 1 y0
	  unsigned char q[]="Ajustar tiempos de rechazo"; 
	  unsigned char x[8]={235,0,8,8,8, 8, 8, 8};
	  unsigned char y[8]={  0,2,4,6,8,10,12,14};
		word n;
		    VFDclrscr();
			VFDposicion(x[0],y[0]);   delay1us();
			VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
			VFDposicion(x[1],y[1]);   delay1us();
		    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
			VFDposicion(x[2],y[2]);   delay1us();
		    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
			VFDposicion(x[3],y[3]);   delay1us();
		    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			VFDposicion(x[4],y[4]);   delay1us();
			VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
			VFDposicion(x[5],y[5]);   delay1us();
		    VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
			VFDposicion(x[6],y[6]);   delay1us();
		    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		    VFDposicion(x[7],y[7]);   delay1us();
		    VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us(); 
		    cursorx=POSX0;cursory=POSY10;
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// fin display ajustes avanzados de Ajustes Parametricos de Producto------------------------------

/*Basic Input Output System, asses that the memory works fine, te DDS-PWG, dISPLAY, KEYPAD
  ADC's, comm, io etcc
*/
void  Display_BIOS(void){
	
	      unsigned char  m0[]="BIOS v0.1";
		  unsigned char  m1[]="Copyright(c) 2016, MhM techologies";
		  
		  unsigned char  m2[]="Insight Processor v2-G ";
		  unsigned char  m3[]="Main Processor: MCF52233 Freescale NXP    ";
		  unsigned char  m4[]="                coldfire v2";
		  unsigned char  m5[]="EEPROM: ";
		  unsigned char  m6[]="NVRAM:";
		  unsigned char  m7[]="ADC's control:";//en revision--en desarrollo, debemmos leer el valor de la variable en 1 y0
		  unsigned char  m8[]="keyPad:";
		  unsigned char  m9[]="IO comm:";
		  //unsigned char m10[]="comms:";
		  //unsigned char m11[]="comms:";
		  		  
		  unsigned char x[12]={0,0,0,0,0,0,0,0,0 ,0};
		  unsigned char y[12]={0,1,3,4,6,EEPROM_NUM_Y,NVRAM_NUM_Y,9,10,11};
			word n;
			    VFDclrscr();
				VFDposicion(x[0],y[0]);   delay1us();
				VFDserial_SendBlock(&m0[0],sizeof(m0),&n);delay1us();
				VFDposicion(x[1],y[1]);   delay1us();
			    VFDserial_SendBlock(&m1[0],sizeof(m1),&n);delay1us();
				VFDposicion(x[2],y[2]);   delay1us();
			    VFDserial_SendBlock(&m2[0],sizeof(m2),&n);delay1us();
				VFDposicion(x[3],y[3]);   delay1us();
			    VFDserial_SendBlock(&m3[0],sizeof(m3),&n);delay1us();
				VFDposicion(x[4],y[4]);   delay1us();
				VFDserial_SendBlock(&m4[0],sizeof(m4),&n);delay1us();
				VFDposicion(x[5],y[5]);   delay1us();
			    VFDserial_SendBlock(&m5[0],sizeof(m5),&n);delay1us();
				VFDposicion(x[6],y[6]);   delay1us();
			    VFDserial_SendBlock(&m6[0],sizeof(m6),&n);delay1us();
			    VFDposicion(x[7],y[7]);   delay1us();
			    VFDserial_SendBlock(&m7[0],sizeof(m7),&n);delay1us();
			    VFDposicion(x[8],y[8]);   delay1us();
			    VFDserial_SendBlock(&m8[0],sizeof(m8),&n);delay1us();
			    VFDposicion(x[9],y[9]);   delay1us();
			    VFDserial_SendBlock(&m9[0],sizeof(m9),&n);delay1us();
			//    VFDposicion(x[10],y[10]);   delay1us();
			  //  VFDserial_SendBlock(&m10[0],sizeof(m10),&n);delay1us();
			   // VFDposicion(x[11],y[11]);   delay1us();
			   // VFDserial_SendBlock(&m11[0],sizeof(m11),&n);delay1us();
			    			    			    
			    cursorx=POSX0;cursory=POSY10;
			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// ends Basis Input Output System display screeen......







//desplega la pantalla para crear el nuevo producto a  crear (*
//void (*func)(void) apuntador a funcion
void DisplayNuevoProducto(void (*func)(void)){
unsigned char iProduct;	
	    //VFDclrscr()	  
		iProduct=SearchSpace(INIT_NEW_PROD_MENU);
		if(iProduct>5){
			LimitesMenuPROD(iProduct-4,ESCRIBIR);  //ver que prducto esta seleccionado para asignar lo limites superiores
			cursorx=POSXSELPRODCURSORX;cursory=POSY10;}
		else{ LimitesMenuPROD(1,ESCRIBIR);
			  cursorx=POSXSELPRODCURSORX;
			  cursory=getPosYNewProdDN(1,iProduct);}
	    fillScreenNames();
	    func(); //displayNuevoProductoTitle();
    	VFDposicion(cursorx,cursory);
	    VFDserial_SendChar('>');												
}//desplega DisplayNuevoProducto para seleccionar el espacio del nuevo producto a crear-------------



void DisplayPasteProduct(void){
	  DisplayNuevoProducto(&displayPasteProductoTitle); 
}//fin DisplayPasteProduct-------------------------------------



void displaySeleccionarProductoTitle(void){
	      unsigned char  m0[]="Seleccionar producto    x";                                    
	                        
		  unsigned char x[9]={ 41,8,8,8,8, 8, 8, 8};
		  unsigned char y[9]={  0,2,4,6,8,10,12,14};
			word n;
		  VFDposicion(41,0); delay1us();
		  VFDserial_SendBlock(&m0[0],sizeof(m0),&n);delay1us();
}//fin display Seleccionar Producto title----------------



void displayAjusteProductoSelectTitle(void){
	      unsigned char  m0[]="Ajuste de Producto      x";                                    
	                        
		  unsigned char x[9]={ 41,8,8,8,8, 8, 8, 8};
		  unsigned char y[9]={  0,2,4,6,8,10,12,14};
			word n;
		  VFDposicion(41,0); delay1us();
		  VFDserial_SendBlock(&m0[0],sizeof(m0),&n);delay1us();
}//fin display Seleccionar Producto title----------------


void displayBorrarProductoTitle(void){
	      unsigned char  m0[]="Borrar producto         x";                                    
	      
		  unsigned char x[9]={ 41,8,8,8,8, 8, 8, 8};
		  unsigned char y[9]={  0,2,4,6,8,10,12,14};
			word n;
		  VFDposicion(41,0); delay1us();
		  VFDserial_SendBlock(&m0[0],sizeof(m0),&n);delay1us();
}//fin display Seleccionar Producto title----------------

void displayCopyProducto(void){
	displaySeleccioniProd_lugarOcupado(&displayCopyProductoTitle);
}//fin display  copiar el producto -----------------------------------------------------------------


void displayBorrarProducto(void){
	displaySeleccioniProd_lugarOcupado(&displayBorrarProductoTitle);
}// fin display  seleccionar Producto----------------------------------------------------------------

void displaySeleccionarProducto(void){
	displaySeleccioniProd_lugarOcupado(&displaySeleccionarProductoTitle);
}// fin display  seleccionar Producto----------------------------------------------------------------

void displayAjusteProductoSelect(void){
	displaySeleccioniProd_lugarOcupado(&displayAjusteProductoSelectTitle);
}//fin displayAjusteProductoSelect-------------------------------------------------------------------





void displayCopyProductoTitle(void){
	      unsigned char  m0[]="Copiar producto         x";                                    
	      
		  unsigned char x[9]={ 41,8,8,8,8, 8, 8, 8};
		  unsigned char y[9]={  0,2,4,6,8,10,12,14};
			word n;
		  VFDposicion(41,0); delay1us();
		  VFDserial_SendBlock(&m0[0],sizeof(m0),&n);delay1us();
}//fin display Seleccionar Producto title----------------
     
void displayPasteProductoTitle(void){
	 unsigned char  m0[]="Pegar  producto         x";                                    
		      
     unsigned char x[9]={ 41,8,8,8,8, 8, 8, 8};
	 unsigned char y[9]={  0,2,4,6,8,10,12,14};
	word n;
		  VFDposicion(41,0); delay1us();
		  VFDserial_SendBlock(&m0[0],sizeof(m0),&n);delay1us();
	
}//fin display paste producto title









/* Regresamos el siguiente iProducto que encontremos hacia la direccion
 * que se indica  buscamos a partir del siguiente iProduct que estamos
 * */
unsigned char BuscarSiguienteiProduct(unsigned char direccion){
unsigned char i,j;	
	switch(menu.contexto.Actual){
		case COPY_PRODUCTO:
		case SELECCIONAR_PRODUCTO:	
		case BORRAR_PRODUCTO:
		case AJUSTE_PRODUCTO_SEL:
			  if(direccion==PABAJO){
				  if(cursorx==POSXESQUINA)//
					    __asm(Halt);//debug error de dise�o de software
				  i=LimitesMenuPROD(0,READ_LIM_SUP);//get limit sup actual screen
				  return(searchNextiProduct(PABAJO,convertPOSY2iProduct(i,cursory)));
			  }//fin if abajo
			  else{if(direccion==PARRIBA){
				     if(cursorx==POSXESQUINA)
				    	 __asm(Halt);
				     i=LimitesMenuPROD(0,READ_LIM_SUP);
				     j=searchNextiProduct(direccion,convertPOSY2iProduct(i,cursory));
				     if(j==0)
					    __asm(Halt);//debug error de software
				     return(j);				  
			  }}//fin else
			  break;//fin  de borrar producto-------------
		default:break;
	}//fin switch
}//BuscarSiguienteiProduct--------------------------------------------------


/*Cuando buscamos un espacion de nuevo producto hacia abajo y encontramos un espacio de indice de producto
 * y estamos en la misma pantalla necesitamos saber ahora que fila es  */
unsigned char getPosYNewProdDN(unsigned char sup,unsigned char iProd){
const unsigned char y[7]={POSY2,POSY4,POSY6,POSY8,POSY10,POSY12,POSY14};	
      if(iProd==0)
        	__asm(Halt);//debug error de dise�o de sofware
      return y[iProd-sup];
}//fin de dar la posicion y del cursos cuando buscamos el siguiente espacion para nuevo producto hacia abajo




void DisplayParte11Menu(void){//despliega el menu del PARTE-11------------
	  unsigned char z[]="   Menu  de Parte-11        x";
	  unsigned char a[]=">Analoga version";
	  unsigned char c[]="En desarrollo";
	  unsigned char w[]="En desarrollo";
	  unsigned char v[]="En desarrollo";
	  unsigned char b[]="En desarrollo";
	  unsigned char q[]="En desarrollo"; 
	  unsigned char x[7]={20,0,8,8, 8, 8, 8};
	  unsigned char y[7]={0 ,4,6,8,10,12,14};
	  word n;
      VFDclrscr();
			  VFDposicion(x[0],y[0]);   delay1us();
		      VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
		  	  VFDposicion(x[1],y[1]);   delay1us();
		      VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
			  VFDposicion(x[2],y[2]);   delay1us();
		      VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			  VFDposicion(x[3],y[3]);   delay1us();
			  VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
			  VFDposicion(x[4],y[4]);   delay1us();
		      VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
			  VFDposicion(x[5],y[5]);   delay1us();
		      VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		      VFDposicion(x[6],y[6]);   delay1us();
		      VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
		      //VFDposicion(x[7],y[7]);//posiCION del cursor despues de desplegar el menu  
		      cursorx=POSX0;cursory=POSY4;
		      displayvarParte11();
			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin display del menu de AJUSTE DE SISTEMA-----------------------------------------------------------------

//
void DisplayAjusteVibracionMenu(void){//CONFIGURACION DE VIBRACION-----------------------
	unsigned char z[]="   Ajuste de Vibracion     >X";
		  unsigned char a[]="Pasar Ferrita";  //"Pasar Ferrita
		  unsigned char x[2]={20,LABEL1_CV_Y};    //       Completado       
		  unsigned char y[2]={0 ,LABEL1_CV_Y};
		  word n;
	  VFDclrscr();
	  displayCuadroMadre();//despliega el cuadro Madre de deteccion
	  VFDposicion(x[0],y[0]);   delay1us();
	  VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
	  VFDposicion(x[1],y[1]);   delay1us();
	  VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
	  cursorx=POSX0;cursory=POSY0; 			
	  AjustVibr=&g;
	  AjustVibr->igxc0=0;////se pone a cero, es el numero de despleigue, numero de veces
	  AjustVibr->igxc1=0;//=0;//Indica si Paramos el procesamiento para calcular el angulo de vibracion
	  AjustVibr->dgxc0=anguloVibracion;//=anguloVibracion;//En caso de que guardemos el Angulo de Vibracion se Regresa el valor a anguloVibracion, dentro de Memoria Handlers
	  AjustVibr->igxc4=WAIT;//igxm4=WAIT;
}//fin de display de configuracion de vibracion-------------------------------------------------------------
 

//*******************************************************************************************************
/******FIN DE FUNCIONES EJECUTIVAS*DE DESPLIEGUE***********************************************************************************


//***************HOJAS EJECUTIVAS******************************************************************************/
//***************HOJAS EJECUTIVAS******************************************************************************/
///***************HOJAS EJECUTIVAS******************************************************************************/
//***************HOJAS EJECUTIVAS******************************************************************************/
//***************HOJAS EJECUTIVAS******************************************************************************/
///***************HOJAS EJECUTIVAS******************************************************************************/
	




void displayIDcommNUM(void){
  VFDposicion(POSXNUM+1,POSY4); 
  if(++IDcomm>9){IDcomm=0;}
  VFDserial_SendChar(IDcomm+0x30U);
  configModificado(ID_COMUNICACIONES);//indica que ha sido modificado la configuracnio de entradas de sistema	
}// fin ID DE  comunicaciones-----------------------------------------------------------------------------------------


void refreshDDS(unsigned char posy){
	  
	  DDSaux=posy; 
	  DDS_HANDLER();
	 
}//final of function refresh DDS screen and position cursor in Y row.--------------------------------------

void DDS_HANDLER(void){//MANEJADOR MAESTRO DEL DDS--------------------------------------------------------
	//const unsigned char x1=200;//posicion de las letras en elDDS
	          unsigned char pen=1;         
	          unsigned char  r[]=">X";
	          unsigned char r0[]=" x";//para cuando esta activo el zoom en enter
	          unsigned char z[]="S ";
	          unsigned char a[]="P ";
	          unsigned char c[]="A ";
	          unsigned char w[]="G ";
	          unsigned char q[]="C ";
	          unsigned char b[]="Borrar";
	          unsigned char x[8]={236,POSXDDS,POSXDDS,POSXDDS,POSXDDS,POSXDDS,POSXDDS,POSXDDS};//235,200,200,200,200,200,200};
	          unsigned char y[8]={  0,  2,  4,  6,  8, 10, 12, 14};
            word n;
            //ActividadMonitor(DESACTIVADO); //debug 
             DDSpant=&g;
             DDSpant->igxc0=0xAA;//habilitar el despliegue de las display normales al display VFD
	         VFDclrscr();// limpiar pantalla
	         VFDposicion(x[0],y[0]);   delay1us(); 
             if(DDSaux!=POSY10){
            	 if((!BorrarDDS)&&(!DDSpant->igxc2))
            	      VFDserial_SendBlock(&r[0],sizeof(r),&n);
            	 else  VFDserial_SendBlock(&r0[0],sizeof(r),&n);}
             else VFDserial_SendBlock(&r0[0],sizeof(r),&n);
             delay1us(); 	 
     	     VFDposicion(x[1],y[1]);   delay1us();
             VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
      	     VFDposicion(x[2],y[2]);   delay1us();
    	     VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
    	     VFDposicion(x[3],y[3]);   delay1us();
             VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
	         VFDposicion(x[4],y[4]);   delay1us();
    	     VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
    	     display_Linea_10_DDS(DDSaux);
	         VFDposicion(x[6],y[6]);   delay1us();
    	     VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
             VFDposicion(x[7],y[7]);   delay1us();
    	     VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
	          
    	    
	         //despliegue de numeros y variables 
    	     displayUINT_var(POSXDDSNUMS,2,&Deteccion.Sensibilidad,NONE); //DEsplegar la sensibilidad
	         displayPhase_var(POSXDDSNUMS,4);//desplega la variable de pHase
    	     displayUINT_var(POSXDDSNUMS,6,&Altura,NONE);//despliega variable Altura deprecated	     displaySwitchGanancia_var(DDS,NONE);//el NONE es para decirle que solo lo despliegue sin el switching
    	     displaySwitchGanancia_var(DDS,NONE);//display Ganancia var
    	     displayZoomDDS_var(NONE);//despliega la var Zoom 
	         displaySwitchCuadro_var(NONE);//no se usa como un switch solo se despliega
    	     
	         //dribujo de las  lineas coordenadas graficas en el DDS
    	     VFDdrawLine(pen,0,DDS_CENTRO_X,190,DDS_CENTRO_X);//DIBUJAMOS linea horizontal mitad pantalla, horizontal eje
	         VFDdrawLine(pen,DDS_CENTRO_Y,0,DDS_CENTRO_Y,127);//Linea Vertical Eje Y
    	     
    	     
    	     
	     /* Asignacion de coordenadas de los cursores segun se la primera vez que se entra o se venga de un botonazo de borrado   */
    	     if(dds.Bnderas.bit.DDS1_BORRAR==TRUE){
	            VFDposicion(236,POSY0);
   	            VFDserial_SendChar(' ');
	            VFDserial_SendChar('x');
    	        //VFDposicion(POSXDDSCURSORMENU-8,POSY14);
	            //VFDserial_SendChar('>');
	            
	            if(dds.Bnderas.bit.clean_Buffers==TRUE)
	                clean_repaint_Pixels_DDS();//lean buffes repaint zoom
	            if(dds.Bnderas.bit.DDS_Reload)
	            	if(cursory==POSY10){
	            		 display_Linea_10_DDS(POSY10);
	            		 cursory=POSY10;}
	            	else {display_B_bold_DDS(TRUE);
	            		  cursory=POSY14;}
	            else {display_B_bold_DDS(TRUE);cursory=POSY14;}
	            cursorx=POSXDDSCURSORMENU;}
	         else{
	        	  if(DDSaux==POSY10){
	        		  cursorx=POSXDDS,cursory=POSY10;} //ESTAMOS EN el zoom y el cursor debe permancesa alli
	        	  else{cursorx=POSXESQUINA;cursory=POSY0;}}     
    	     dds.Bnderas.bit.DDS1_TIMER=TRUE; //.EventDDS=0xAA; //dice que se espere un rato el Graficador de Puntitos a comenzar en timer int
    	    //^--Funciono!! funcion bien no quitar
    	     switch(DDSpant->igxc2){
    	         	    	case 0:  display_Sensibilidad_Geometrico(ESTATICO);
    	         	    		     if(cursory>POSY0){
    	         	    		    	 VFDposicion(POSXDDSCURSORMENU,cursory);
    	         	    		    	 VFDserial_SendChar('>');
    	         	    		    	 cursorx=POSXDDSCURSORMENU;}
    	         	    	         break;
    	         	    	case 'S':display_Sensibilidad_Geometrico(ESTATICO);
    	         	    	         VFDposicion(POSXDDSCURSORMENU,POSY2);
    	         	    	         VFDserial_SendChar('>');
    	         	    	         cursory=POSY2;cursorx=POSXDDSCURSORMENU;
    	         	    	         break;
    	         	    	case 'P':display_Sensibilidad_Geometrico(ESTATICO);
    	         	    		     VFDposicion(POSXDDSCURSORMENU,POSY4);
									 VFDserial_SendChar('>');
								 	 cursory=POSY4;cursorx=POSXDDSCURSORMENU;
    	         	    		     break;
    	         	    	default:break;}
    	     //Activacion de los controladoes Principales del despliegue de los puntitos de los valores del ADC en el DDS
    	    DDSstatus=0xBBU; //Se autoriza activacion del displayDDSsignalProcessor_IRQ()
    	    semaforoDDS(VERDE);//se autoriza graficacion de puntitos
    	    stateDDSplot=0;//iniziamos el state machine, para graficar puntitos.
    	    countLineSwitch=0;//debug-line inizializa el conteo de puntos antes de desplegar la linea
            //dds.Bnderas.bit.Apagando=FALSE;//lo queremos prender no apaga
    	    
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.
		    dds.Bnderas.bit.Apagando=FALSE;
            dds.Bnderas.bit.EventDDS=TRUE;
            dds.Bnderas.bit.DDS1_BORRAR=FALSE;
            DDSpant->igxc1=0;//es el caracter que se va a manipular =5 es el mas significativo si es de 5 cifras.
            DDSpant->igxc4=0;//es el contrl del cuadro variable C=1|0
            DDSpant->igxc5=0;//control de digitos sensibilidad
            DDSpant->igxc2=0;//indica si se ha hecho un cambio en sensibilidd o  phase u otra cosa y se tiene que repintar el DDS
}// FIN DEL manejador maestro del DDS---------------------------------------------------------------------
	


    	
 


//------------------------------------------------------------------------MENU@4:
void DisplayMedidasVoltaje(void){
	      unsigned char r[]=">X";
	      unsigned char b[]="Volts";
		  unsigned char z[]="Voltaje de balance";
		  unsigned char a[]="Voltaje driver";
		  unsigned char c[]="Canal X";
		  unsigned char w[]="Canal Y";
		  unsigned char x[8]={235,0,0, 0, 0,POSX_VOLT_BALANCE};
		  unsigned char y[8]={  0,2,4, 6, 8,0};
			word n;
			    VFDclrscr();
				VFDposicion(x[0],y[0]);   delay1us();
				VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
				VFDposicion(x[1],y[1]);   delay1us();
			    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
				VFDposicion(x[2],y[2]);   delay1us();
			    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
				VFDposicion(x[3],y[3]);   delay1us();
			    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
				VFDposicion(x[4],y[4]);   delay1us();
				VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
				VFDposicion(x[5],y[5]);   delay1us();
				VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
				
				VFDposicion(POSX185,y[1]);   delay1us();
				VFDserial_SendChar('0');  //desplagar datos aqui
				VFDposicion(POSX185,y[2]);   delay1us();
				VFDserial_SendChar('0');  //desplagar datos aqui
				VFDposicion(POSX185,y[3]);   delay1us();
				VFDserial_SendChar('0');  //desplagar datos aqui
				VFDposicion(POSX185,y[4]);   delay1us();
				VFDserial_SendChar('0');  //desplagar datos aqui
				cursorx=POSXESQUINA;cursory=POSY0;   
				Mediciones.flagMesurements=TRUE;
			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//display control de frecuencias-----------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------MENU@4:
void DisplayMedidasVoltajes(void){
	      unsigned char a[]=">X";
	      unsigned char s[]="Volts";
		  unsigned char d[]="Voltaje de balance";
		  unsigned char f[]="Voltaje driver";
		  unsigned char g[]="Canal X:           Y:";
		  unsigned char h[]="Raw X:         Raw Y:";
		  unsigned char j[]="Off X:         Off Y:";
		  unsigned char k[]="Rot X:         Rot Y:";
		  unsigned char z[]="Flt X:         Flt Y:";
		  unsigned char x[9]={235,170,0,0, 0, 0,  0, 0, 0};//POSX_VOLT_BALANCE};
		  unsigned char y[9]={  0,  0,2,4, 6, 8, 10,12,14};
		  word n;
			    VFDclrscr();
				VFDposicion(x[0],y[0]);   delay1us();
				VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
				VFDposicion(x[1],y[1]);   delay1us();
			    VFDserial_SendBlock(&s[0],sizeof(s),&n);delay1us();
				VFDposicion(x[2],y[2]);   delay1us();
			    VFDserial_SendBlock(&d[0],sizeof(d),&n);delay1us();
				VFDposicion(x[3],y[3]);   delay1us();
			    VFDserial_SendBlock(&f[0],sizeof(f),&n);delay1us();
				VFDposicion(x[4],y[4]);   delay1us();
				VFDserial_SendBlock(&g[0],sizeof(g),&n);delay1us();
				VFDposicion(x[5],y[5]);   delay1us();
				VFDserial_SendBlock(&h[0],sizeof(h),&n);delay1us();
				VFDposicion(x[6],y[6]);   delay1us();
				VFDserial_SendBlock(&j[0],sizeof(j),&n);delay1us();
				VFDposicion(x[7],y[7]);   delay1us();
				VFDserial_SendBlock(&k[0],sizeof(k),&n);delay1us();
				VFDposicion(x[8],y[8]);   delay1us();
				VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
																		
				cursorx=POSXESQUINA;cursory=POSY0;   
				Mediciones.flagMesurements=TRUE;
			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//display control de frecuencias-----------------------------------------------------------------------------------------------------------



void DisplayAjusteProductoInfo(void){
	 
	
}// fin display Auste de Producto informacion-----------------------------------------------------


void DisplayTiempoRechazo(void){
	unsigned char r[]=">X";
	unsigned char z[]="Prueba de sistema: ";
	unsigned char a[]="  ";
	unsigned char c[]="Tiempo de espera:  ";
	unsigned char w[]="Tiempo de rechazo: ";
	unsigned char x[8]={235,0,8, 8, 8};
	unsigned char y[8]={  0,2,4, 8,12};
	word n;
	VFDclrscr();
	VFDposicion(x[0],y[0]);   delay1us();
	VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
	VFDposicion(x[1],y[1]);   delay1us();
	VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
	
	VFDposicion(x[2],y[2]);   delay1us();
	VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
	VFDposicion(x[3],y[3]);   delay1us();
	VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
	
	VFDposicion(x[4],y[4]);   delay1us();
	VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
				
	VFDposicion(POSX152,POSY2);   delay1us();
	VFDserial_SendChar('0');  //desplagar datos aqui
	VFDposicion(POSX152,POSY4);   delay1us();
	VFDserial_SendChar('0');  //desplagar datos aqui
	VFDposicion(POSX152,POSY8);   delay1us();
	VFDserial_SendChar('0');  //desplagar datos aqui
	VFDposicion(POSX152,POSY12);   delay1us();
	VFDserial_SendChar('0');  //desplagar datos aqui
	cursorx=POSXESQUINA;cursory=POSY2;   
    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// fin de Display Tiempo de Rechazo------------------------------------------------------------------


void DisplayAjusteSistemaEntradas(void){
	  unsigned char r[]=" {fecha aqui}                 >X";
      unsigned char z[]="   Entradas         Permitir";
	  unsigned char a[]="Entrada de taco        ";
	  unsigned char c[]="Entrada de fotocel     ";
	  unsigned char w[]="Test entrada rechazo   ";
	  unsigned char v[]="Test entrada producto  ";//en revision, debemos leer el valor de la variable si es uno o cero
	  unsigned char b[]="Entrada auxiliar1      ";
	  unsigned char q[]="Entrada auxiliar2      "; 
	  unsigned char x[8]={0,0,8,8,8, 8, 8, 8};
	  unsigned char y[8]={0,2,4,6,8,10,12,14};
		word n;
		    VFDclrscr();
			VFDposicion(x[0],y[0]);   delay1us();
			VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
			VFDposicion(x[1],y[1]);   delay1us();
		    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
			VFDposicion(x[2],y[2]);   delay1us();
		    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
		    VFDserial_SendChar(EntradasSistemaReg.taco);
			VFDposicion(x[3],y[3]);   delay1us();
		    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
		    VFDserial_SendChar(EntradasSistemaReg.fotocel);
		    VFDposicion(x[4],y[4]);   delay1us();
			VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
			VFDserial_SendChar(EntradasSistemaReg.rechazo);
			VFDposicion(x[5],y[5]);   delay1us();
		    VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
		    VFDserial_SendChar(EntradasSistemaReg.producto);
		    VFDposicion(x[6],y[6]);   delay1us();
		    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		    VFDserial_SendChar(EntradasSistemaReg.aux1);
		    VFDposicion(x[7],y[7]);   delay1us();
		    VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
		    VFDserial_SendChar(EntradasSistemaReg.aux2);
		    //VFDposicion(x[7],y[7]);//posiCION del cursor despues de desplegar el menu  
		    cursorx=POSX0;cursory=POSY4;
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin de display de ajuste de sistema de entradas--------------------

void DisplayIDmaquina(void){
	unsigned char r[]="  ID de maquina            >X";
	unsigned char z[]="   nombre/numero-aqui";
	unsigned char a[]="Version aqui        ";
	unsigned char c[]="Version aqui     ";
	unsigned char w[]="  Alta   #aqui";
	unsigned char v[]="  Media  #aqui";//en revision, debemos leer el valor de la variable si es uno o cero
	unsigned char b[]="  Baja   #aqui";
	//unsigned char q[]="Entrada auxiliar2      0"; 
	unsigned char x[8]={0,0,0, 8}; //, 8, 8};	
	unsigned char y[8]={0,4,8,10}; //,10,12};
	word n;
	VFDclrscr();
	VFDposicion(x[0],y[0]);   delay1us();
	VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
	VFDposicion(x[1],y[1]);   delay1us();
    VFDserial_SendBlock(&z[0],sizeof(r),&n);delay1us();
	VFDposicion(x[2],y[2]);   delay1us();
	VFDserial_SendBlock(&a[0],sizeof(r),&n);delay1us();
	VFDposicion(x[3],y[3]);   delay1us();
	VFDserial_SendBlock(&c[0],sizeof(r),&n);delay1us();
	VFDposicion(x[4],y[4]);   delay1us();
	VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
	VFDposicion(x[5],y[5]);   delay1us();
	VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
	VFDposicion(x[6],y[6]);   delay1us();
	VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
	cursorx=POSX0;cursory=POSXESQUINA;
    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// fin display ID maquina -----------------------------------------


void DisplayCuentaProducto(void){//-----------------------------------
	unsigned char r[]="Apagar  ";
    unsigned char z[]="Permitir";
	word n;
	VFDclrscr();
	
	VFDposicion(POSX192,POSY14);   delay1us();
	if(CuentaProducto==APAGAR){ 
		CuentaProducto=PERMITIR; 
		VFDserial_SendBlock(&z[0],sizeof(r),&n);}
	else {CuentaProducto=APAGAR;
	    VFDserial_SendBlock(&r[0],sizeof(z),&n);}
	delay1us();
	cursorx=POSX0;cursory=POSY14;
    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// fin Display Centa de Producto-------------------------------------

void DisplayEstablecerIdioma(void){
//---------------------123456789ABCDEF123456789ABCDEF123	
	unsigned char r[]="Establecer idioma            x";
	unsigned char z[]=">Espanol";
	unsigned char a[]="English";
	unsigned char c[]="Português";
	unsigned char w[]="Français ";
	unsigned char v[]="Italiano ";//en revision, debemos leer el valor de la variable si es uno o cero
	unsigned char b[]="Türkçe "; //mas idomas mas para bajo en la flecha
	unsigned char m[]="Svenska";	
	unsigned char x[8]={0,0,8, 8, 8, 8, 8, 8}; //, 8, 8};
	unsigned char y[8]={0,2,4, 6, 8,10,12,14}; //,10,12};
	word n;
	VFDclrscr();
	VFDposicion(x[0],y[0]);   delay1us();
	VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
	VFDposicion(x[1],y[1]);   delay1us();
	VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
	VFDposicion(x[2],y[2]);   delay1us();
	VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
	VFDposicion(x[3],y[3]);   delay1us();
	VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
	VFDposicion(x[4],y[4]);   delay1us();
	VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
	VFDposicion(x[5],y[5]);   delay1us();
	VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
	VFDposicion(x[6],y[6]);   delay1us();
	VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
	VFDposicion(x[7],y[7]);   delay1us();
    VFDserial_SendBlock(&m[0],sizeof(m),&n);delay1us();
		
	cursorx=POSX0;cursory=POSXESQUINA;
    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// fin display establecer idioma-------------------------------------



void DisplayControlpassword(void){
//---------------------123456789ABCDEF123456789ABCDEF123                 
	unsigned char r[]="Control de contrasenas        x";
	unsigned char z[]="Nivel"; 
	unsigned char a[]=" > 1  ";
	unsigned char c[]="   2  ";
	unsigned char w[]="   3  ";
	unsigned char v[]="   4  ";
	unsigned char x[8]={0,0,0, 0,  0 , 0}; 
	unsigned char y[8]={0,2,6, 8, 10 , 12}; 
	word n;
	VFDclrscr();
	VFDposicion(x[0],y[0]);   delay1us();
	VFDserial_SendBlock(&r[0],sizeof(r),&n); delay1us();
	VFDposicion(x[1],y[1]);   delay1us();
	VFDserial_SendBlock(&z[0],sizeof(z),&n); delay1us();
	VFDposicion(x[2],y[2]);   delay1us();
	VFDserial_SendBlock(&a[0],sizeof(a),&n); displayControlContrasenaVal(1);
	VFDposicion(x[3],y[3]);   delay1us();
	VFDserial_SendBlock(&c[0],sizeof(c),&n); displayControlContrasenaVal(2); 
	
	VFDposicion(x[4],y[4]);   delay1us();
	VFDserial_SendBlock(&w[0],sizeof(w),&n); displayControlContrasenaVal(3);
	
	VFDposicion(x[5],y[5]);   delay1us();
	VFDserial_SendBlock(&v[0],sizeof(v),&n); displayControlContrasenaVal(4);
	//VFDserial_SendChar(ControlContrasena4);
	
	cursorx=POSX0CNTRLPASSW;cursory=POSY6;
    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.
}// fin display establecer idioma-------------------------------------


/*void DisplayContrasenaVars(unsigned char psw){//Permitir------------------------------
	                           //               Apagar 
	unsigned char r[]="Permitir";
    unsigned char z[]="Apagar  ";//ControlContrasena1
    word n;
	VFDposicion(POSXNUM,psw);   // ControlContrasena1
	switch(psw){//falta gardar variables y poner el estas seguro?
	   case POSY6:if(ControlContrasena1==PERMITIR){
		                 VFDserial_SendBlock(&z[0],sizeof(z),&n);break;}
	              else {VFDserial_SendBlock(&r[0],sizeof(r),&n);break;}
	   case POSY8:if(ControlContrasena2==PERMITIR){
                       VFDserial_SendBlock(&z[0],sizeof(z),&n);break;}
                  else {VFDserial_SendBlock(&r[0],sizeof(r),&n);break;}
	   case POSY10:if(ControlContrasena3==PERMITIR){
                      VFDserial_SendBlock(&z[0],sizeof(z),&n);break;}
                  else {VFDserial_SendBlock(&r[0],sizeof(r),&n);break;}
	   case POSY12:if(ControlContrasena4==PERMITIR){VFDserial_SendBlock(&z[0],sizeof(z),&n);break;}
                   else {VFDserial_SendBlock(&r[0],sizeof(r),&n);break;}	    
	}//fin switch psw-----------------------------------------------
}// fin control contrasenas variables control-------------------------------------------
*/

void DisplayRelojSistema(void){
	      unsigned char r[]=" x";
	      unsigned char z[]=">ano";
		  unsigned char a[]="mes";
		  unsigned char c[]="dia";
		  unsigned char w[]="horas";
		  unsigned char v[]="minutos";//en revision, debemos leer el valor de la variable si es uno o cero
		  unsigned char b[]="segundos";
		 // unsigned char q[]="Entrada auxiliar2      0"; 
		  unsigned char x[8]={POSXESQUINA,0,8,8,8, 8, 8};
		  unsigned char y[8]={          0,2,4,6,8,10,12};
			word n;
			    VFDclrscr();
				VFDposicion(x[0],y[0]);   delay1us();
				VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
				VFDposicion(x[1],y[1]);   delay1us();
			    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
				VFDposicion(x[2],y[2]);   delay1us();
			    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
				VFDposicion(x[3],y[3]);   delay1us();
			    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
				VFDposicion(x[4],y[4]);   delay1us();
				VFDserial_SendBlock(&w[0],sizeof(w),&n);delay1us();
				VFDposicion(x[5],y[5]);   delay1us();
			    VFDserial_SendBlock(&v[0],sizeof(v),&n);delay1us();
				VFDposicion(x[6],y[6]);   delay1us();
			    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
			    //VFDposicion(x[7],y[7]);   delay1us();
			    //VFDserial_SendBlock(&q[0],sizeof(q),&n);delay1us();
			    //VFDposicion(x[7],y[7]);//posiCION del cursor despues de desplegar el menu  
			    
			    getTimeNVRAM();//gets time var and pours into embedded time vars
			    DisplayRelojSistemaVariables();
			    segundero(ACTIVADO);
			    cursorx=POSX0;cursory=POSY2;

			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

			    //display los valores AQUI¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¿
}// fin de display el reloj del sistema----------------------------------------------------------

void DisplayRelojSistemaVariables(void){//despliega las variables cuando se entra al menu de ajuste de hora
	//readclock(); falta traes variables//traer las variables del tiempo al procesador DE LA NVRAM
	displayTime(_YEAR_);
	displayTime(_MES_);
	displayTime(_DIA_);
	displayTime(_HORA_);
	displayTime(_MIN_);	
	displayTime(_SEG_);
	
	
}// fin display reloj sistema de variables -------------------------------------------------------



void rotateControldeUsuariovar(void){
	if(Control_de_usuario=='1') 
		Control_de_usuario='0';
		 else Control_de_usuario='1';
}//fin rotate ajuste de guardia var


void displayControldeUsuariovar(void){
	  VFDposicion(160,POSY14);
	  VFDserial_SendChar(Control_de_usuario);
}//fin display control de Usuario------------------------------------------------------



void displayAjusteGuardiaControl(void){
	 VFDposicion(160,POSY12);
	 VFDserial_SendChar(Guardia);
}//fin display de Ajuste de Guardia de Control-----------------------------------------

void rotateAjusteGuardiavar(void){
	if(Guardia=='1') 
			  Guardia='0';
		 else Guardia='1';
}//fin rotate ajuste de guardia var


void DisplayMenuGanancia(void){
	 unsigned char r[]=" x";
     unsigned char z[]=">Voltaje principal";
     unsigned char a[]="Ganancia analoga";
	 unsigned char x[3]={POSXESQ235,0,8};
	 unsigned char y[3]={         0,2,4};
				word n;
				    VFDclrscr();
					VFDposicion(x[0],y[0]);   delay1us();
					VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
					VFDposicion(x[1],y[1]);   delay1us();
				    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
					VFDposicion(x[2],y[2]);   delay1us();
				    VFDserial_SendBlock(&a[0],sizeof(a),&n);
				    displayGananciaValues(DRIVER);
				    displayGananciaValues(ANALOGA);
				    cursorx=POSX0;cursory=POSY2;
				    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// fin displat menu ganancia---------------------------------------------------------



void DisplayMenuFiltro(void){
//----------------------123456789ABCDEF123456789ABCDEF123	
	 unsigned char r[]="    Filtro                   >X";
	 word n;
	 VFDclrscr();
	 VFDposicion(0,0);   delay1us();
	 VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
	 displayfiltroVar();
	 cursorx=POSX0;cursory=POSY0;
	    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}// fin de display Menu filtro--------------------------------------------------------

void displayfiltroVar(void){
	unsigned char z[]="Normal   ";
    unsigned char a[]="Alta velocidad";
	unsigned char c[]="Intermdia     ";
	unsigned char w[]="Baja(64) ";
	unsigned char v[]="Baja(128)";//en revision, debemos leer el valor de la variable si es uno o cero
	unsigned char b[]="Despacio "; 
	word n;
	VFDposicion(16,POSY2);
		 switch(filtroConfig){
			case NORMAL:VFDserial_SendBlock(&z[0],sizeof(z),&n);break; 
			case ALTA_VELOCIDAD:VFDserial_SendBlock(&a[0],sizeof(a),&n);break;
			case INTERMEDIA:VFDserial_SendBlock(&c[0],sizeof(c),&n);break; 
			case BAJA_64:VFDserial_SendBlock(&w[0],sizeof(w),&n);break;
			case BAJA_128: VFDserial_SendBlock(&v[0],sizeof(v),&n);break;
			case DESPACIO:VFDserial_SendBlock(&b[0],sizeof(b),&n);break;
			default:filtroConfig=NORMAL; 
			        VFDserial_SendBlock(&z[0],sizeof(z),&n); break;}
		 delay1us();
}//fin desplegar las varibles rotadas del filtro-------------------------------------

void filtroVarRotate(void){
	if(menu.contexto.Actual==FILTRO_){
	 switch(filtroConfig){
	    case NORMAL:filtroConfig=ALTA_VELOCIDAD;    break; 
	 	case ALTA_VELOCIDAD:filtroConfig=INTERMEDIA;break;
	 	case INTERMEDIA:filtroConfig=BAJA_64;       break; 
	 	case BAJA_64:  filtroConfig= BAJA_128;      break;
	 	case BAJA_128: filtroConfig= DESPACIO;      break;
	 	case DESPACIO: filtroConfig=NORMAL;         break;
	 	default:filtroConfig=NORMAL; break;//esto es al inicio de todos los tiempos
	 } }//fin switch filtroConfig
}// fin de despiegue el filtro de la variable rotada---------------------------------



void DisplayFrecuenciaSelect(void){
          unsigned char r[]=" x";
	      unsigned char z[]="Alta";
		  unsigned char a[]="Media";
		  unsigned char c[]="Baja"; 
		  unsigned char x[4]={POSXESQUINA,8,8,8};
		  unsigned char y[4]={          0,2,4,6};
			word n;
			    VFDclrscr();
				VFDposicion(x[0],y[0]);   delay1us();
				VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
				VFDposicion(x[1],y[1]);   delay1us();
			    VFDserial_SendBlock(&z[0],sizeof(z),&n);delay1us();
				VFDposicion(x[2],y[2]);   delay1us();
			    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
				VFDposicion(x[3],y[3]);   delay1us();
			    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			    cursorx=POSX0;cursory=displayFrecuenciaSelect_var();
			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin display frecuencia select variable------------------------------------------------------


unsigned char displayFrecuenciaSelect_var(void){
unsigned char rt;	
     switch(frecuenciaSeleccion){
       case ALTA: VFDposicion(0,POSY2);rt=POSY2;break;
       case MEDIA:VFDposicion(0,POSY4);rt=POSY4;break;
       case BAJA: VFDposicion(0,POSY6);rt=POSY6;break;
       default:frecuenciaSeleccion=MEDIA;
               VFDposicion(0,POSY4);break;	   
     }//fin switch	
     VFDserial_SendChar('>');
     displayFrecuenciaSelect_active_var();
return rt;} // display frecuency select variable---------------------------------

void displayFrecuenciaSelect_active_var(void){
	unsigned char r[]="Activada";
	unsigned char a[]="        ";
	word n;
	VFDposicion(POSXLABELFRECPOS,POSY2);delay1us();
	VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
	VFDposicion(POSXLABELFRECPOS,POSY4);delay1us();
	VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
	VFDposicion(POSXLABELFRECPOS,POSY6);delay1us();
	VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
	switch(frecuenciaSeleccion){
	       case ALTA: VFDposicion(POSXLABELFRECPOS,POSY2);break;
	       case MEDIA:VFDposicion(POSXLABELFRECPOS,POSY4);break;
	       case BAJA: VFDposicion(POSXLABELFRECPOS,POSY6);break;
	       default:frecuenciaSeleccion=MEDIA;//cuando el procesador esta recien prendido First StartUp FStU
	               VFDposicion(POSXLABELFRECPOS,POSY4);break;	   
	     }//fin switch	-----------------------------
	VFDserial_SendBlock(&r[0],sizeof(r),&n);delay1us();
}// display Frecuencia Select activate var--------------------------------

void rotate_frecuencySelect_var(void){
	if(menu.contexto.Actual==PARAMETROS_DE_DETECCION){	
		 switch(frecuenciaSeleccion){
			case ALTA:frecuenciaSeleccion=BAJA; break;
			case MEDIA:frecuenciaSeleccion=ALTA;break;
			case BAJA:frecuenciaSeleccion=MEDIA;break;
			default:frecuenciaSeleccion=MEDIA;break; 	
		 }//fin switch	
	}}// fin rotar variable de la frecuency select--------------------------------------------------------------


void displayfrecuencySelect_var(void){
	//unsigned char a[]="      ";
	unsigned char b[]="Alta ";
    unsigned char c[]="Media";
	unsigned char d[]="Baja"; 
		word n;
		if(menu.contexto.Actual==PARAMETROS_DE_DETECCION){		
				VFDposicion(POSXPARAMDETECT,POSY12);
			switch(frecuenciaSeleccion){
				  case  ALTA: VFDserial_SendBlock(&b[0],sizeof(b),&n);break;
				  case  MEDIA:VFDserial_SendBlock(&c[0],sizeof(c),&n);break;
				  case  BAJA: VFDserial_SendBlock(&d[0],sizeof(d),&n);break;
				  default:frecuenciaSeleccion=MEDIA;
				          displayfrecuencySelect_var();break;
				}//fin switch
				delay1us();}
}// fin display de frecuency select var-------------------------------------------------------------------

void DisplayI2CMenu(void){
          unsigned char a[]=" x";
	      unsigned char b[]=">prueba I2C tx";
		  unsigned char c[]="prueba I2C RX  ";
		  unsigned char d[]="prueba I2C tx CAT2401  ";
		  unsigned char e[]="prueba I2C rx CAT2401";
		  unsigned char f[]="en desarrollo";		
		  unsigned char g[]="en desarrollo";
		  unsigned char x[7]={POSXESQUINA,0,8,8,8 , 8, 8};
		  unsigned char y[7]={          0,2,4,6,8,10,12};
			word n;
			    VFDclrscr();
				VFDposicion(x[0],y[0]);   delay1us();
				VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
				VFDposicion(x[1],y[1]);   delay1us();
			    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
				VFDposicion(x[2],y[2]);   delay1us();
			    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
				VFDposicion(x[3],y[3]);   delay1us();
			    VFDserial_SendBlock(&d[0],sizeof(d),&n);delay1us();
			    VFDposicion(x[4],y[4]);   delay1us();
			    VFDserial_SendBlock(&e[0],sizeof(e),&n);delay1us();
			    VFDposicion(x[5],y[5]);   delay1us();
			    VFDserial_SendBlock(&f[0],sizeof(f),&n);delay1us();
			    VFDposicion(x[6],y[6]);   delay1us();
			    VFDserial_SendBlock(&g[0],sizeof(g),&n);delay1us();
			    cursorx=POSX0;cursory=POSY2;
			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin display frecuencia select variable------------------------------------------------------

void DisplayI2CNVRAMdata(void){
//unsigned char *p,y,i=0,j=0;//son 30 espacions hrizontal, 6 renglones 
unsigned char j=0;//son 30 espacions hrizontal, 6 renglones   

          //j=I2Csockets(NULL,menu.contexto.Actual,GET);//obtenemos el id Y hacemos el socket
		  readNVRAM(0,63,j);
		  /*
		  VFDposicion(0,2);
		  for(y=0;y<56;y++){
	           	  VFDserial_SendChar(getdecenaHex(*(p+i)));
	           	  VFDserial_SendChar(getunidadHex(*(p+i++)));
	           	  VFDserial_SendChar(' '); } 
		  I2CliberarModulo();*/		
		  cursorx=POSXESQUINA;cursory=POSY0;   
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin method display I2C NVRAM data-------------------------------------------------------


void DisplaySPIMenu(void){
	          unsigned char a[]=" x";
		      unsigned char b[]=">SPI DDS comm";
			  unsigned char c[]="ADC values";
			  unsigned char d[]="en desarrollo";
			  unsigned char e[]="en desarrollo";
			  unsigned char f[]="en desarrollo";		
			  unsigned char g[]="en desarrollo";
			  unsigned char x[7]={POSXESQUINA,0,8,8,8 , 8, 8};
			  unsigned char y[7]={          0,2,4,6,8,10,12};
				word n;
				    VFDclrscr();
					VFDposicion(x[0],y[0]);   delay1us();
					VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
					VFDposicion(x[1],y[1]);   delay1us();
				    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
					VFDposicion(x[2],y[2]);   delay1us();
				    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
					VFDposicion(x[3],y[3]);   delay1us();
				    VFDserial_SendBlock(&d[0],sizeof(d),&n);delay1us();
				    VFDposicion(x[4],y[4]);   delay1us();
				    VFDserial_SendBlock(&e[0],sizeof(e),&n);delay1us();
				    VFDposicion(x[5],y[5]);   delay1us();
				    VFDserial_SendBlock(&f[0],sizeof(f),&n);delay1us();
				    VFDposicion(x[6],y[6]);   delay1us();
				    VFDserial_SendBlock(&g[0],sizeof(g),&n);delay1us();
				    cursorx=POSX0;cursory=POSY2;
				    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//display SPI menu.-----------------------------------------------------------------------

void DisplayPruebasSalidasMenu(void){
	 unsigned char a[]=" x"; 
	 unsigned char b[]=">Rele Rechazo    0";
     unsigned char c[]="Rele de Falla   0";
	 unsigned char d[]="Rele de Alarma  0";
	 unsigned char e[]="Rele de Bloqueo 0";
     unsigned char f[]="Aux Salida      0";		
	 //unsigned char g[]="Aux Salida      0";
	 unsigned char x[7]={POSXESQUINA,0,8,8,8 , 8, 8};
	 unsigned char y[7]={          0,2,4,6,8 ,10,12};
	 word n;
	        VFDclrscr();
			VFDposicion(x[0],y[0]);   delay1us();
			VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
			VFDposicion(x[1],y[1]);   delay1us();
			VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
			VFDposicion(x[2],y[2]);   delay1us();
			VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
			VFDposicion(x[3],y[3]);   delay1us();
			VFDserial_SendBlock(&d[0],sizeof(d),&n);delay1us();
			VFDposicion(x[4],y[4]);   delay1us();
			VFDserial_SendBlock(&e[0],sizeof(e),&n);delay1us();
			VFDposicion(x[5],y[5]);   delay1us();
			VFDserial_SendBlock(&f[0],sizeof(f),&n);delay1us();
			//VFDposicion(x[6],y[6]);   delay1us();
			//VFDserial_SendBlock(&g[0],sizeof(g),&n);delay1us();
			cursorx=POSX0;cursory=POSY2;
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//display Pruebas de salidas Menu----------------------------------------------------------


void DisplayPruebasEntradasMenu(void){
	unsigned char a[]=">X"; 
    unsigned char b[]="Tacometro   0";
	unsigned char c[]="Disparo     0";
	unsigned char d[]="Sist CHK    0";
	unsigned char e[]="Pack-CHK    0";
	unsigned char f[]="Aux1 IN     0";		
	unsigned char g[]="Aux2 IN     0";
	unsigned char h[]="LockSW      0";
	unsigned char m[]="ResetSW     0";
	unsigned char x[9]={POSXESQUINA,8,8,8,8,8, 8, 8, 8};
	unsigned char y[9]={          0,0,2,4,6,8,10,12,14};
	word n;
		        VFDclrscr();
				VFDposicion(x[0],y[0]);   delay1us();
				VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
				VFDposicion(x[1],y[1]);   delay1us();
				VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
				VFDposicion(x[2],y[2]);   delay1us();
				VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
				VFDposicion(x[3],y[3]);   delay1us();
				VFDserial_SendBlock(&d[0],sizeof(d),&n);delay1us();
				VFDposicion(x[4],y[4]);   delay1us();
				VFDserial_SendBlock(&e[0],sizeof(e),&n);delay1us();
				VFDposicion(x[5],y[5]);   delay1us();
				VFDserial_SendBlock(&f[0],sizeof(f),&n);delay1us();
				VFDposicion(x[6],y[6]);   delay1us();
				VFDserial_SendBlock(&g[0],sizeof(g),&n);delay1us();
				VFDposicion(x[7],y[7]);   delay1us();
				VFDserial_SendBlock(&h[0],sizeof(h),&n);delay1us();
				VFDposicion(x[8],y[8]);   delay1us();
	            VFDserial_SendBlock(&m[0],sizeof(m),&n);delay1us();
				cursorx=POSX0;cursory=POSY2;
				isMenuPruebaEntradasON=0xAA;
			    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//dislay pruebas de Entradas Menu-----------------------------------------------------------


void DisplayI2CeepromData(void){
	         /* unsigned char a[]=" x";
		      unsigned char b[]="00 01 02 03 04 05 06 07 08 09 0A";
			  unsigned char c[]=" ";
			  unsigned char d[]=" ";
			  unsigned char e[]=" ";
			  unsigned char f[]=" ";		
			  unsigned char g[]=" ";
			  unsigned char x[8]={POSXESQUINA,0,8,8,8 , 8, 8,8};
			  unsigned char y[8]={          0,2,4,6, 8,10,12,14};
				word n;
				    VFDclrscr();
					VFDposicion(x[0],y[0]);   delay1us();
					VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
					VFDposicion(x[1],y[1]);   delay1us();
				    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
					VFDposicion(x[2],y[2]);   delay1us();
				    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
					VFDposicion(x[3],y[3]);   delay1us();
				    VFDserial_SendBlock(&d[0],sizeof(d),&n);delay1us();
				    VFDposicion(x[4],y[4]);   delay1us();
				    VFDserial_SendBlock(&e[0],sizeof(e),&n);delay1us();
				    VFDposicion(x[5],y[5]);   delay1us();
				    VFDserial_SendBlock(&f[0],sizeof(f),&n);delay1us();
				    VFDposicion(x[6],y[6]);   delay1us();
				    VFDserial_SendBlock(&g[0],sizeof(g),&n);delay1us();*/
				    DisplayEEPROMdata();
				    cursorx=POSXESQUINA;cursory=POSY0;
				    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin display de i2c eeprom data----------------------------------------------------------------

//se activa la ejecucion del proceso en el hilo principal
void DisplayEEPROMdata(void){
	 if(Sem.operacion==VERDE)
		 Sem.operacion=READ_EEPROM_DISPLAY; //acivamos la lectura  	
	    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin display data stored into the EEPROM-------------------------------------------------------

void displayEEPROMdata1(unsigned char *p){
unsigned char y,i=0;//son 30 espacions hrizontal, 6 renglones  	
	 VFDposicion(0,2);
		  for(y=0;y<75;y++){
	           	  VFDserial_SendChar(getdecenaHex(*(p+i)));
	           	  VFDserial_SendChar(getunidadHex(*(p+i++)));
	           	  VFDserial_SendChar(' '); } 
}//despleigue por temporizacion del contenido de la memoria EEPROM-------------------------------



unsigned char getunidadHex(unsigned char n){
unsigned char x;	
	  x=n&0x0f;
	  switch(x){
	   case 0x00:return('0');break;
	   case 0x01:return('1');break;
	   case 0x02:return('2');break;
	   case 0x03:return('3');break;
	   case 0x04:return('4');break;
	   case 0x05:return('5');break;
	   case 0x06:return('6');break;
	   case 0x07:return('7');break;
	   case 0x08:return('8');break;
	   case 0x09:return('9');break;
	   case 0x0A:return('A');break;
	   case 0x0B:return('B');break;
	   case 0x0C:return('C');break;
	   case 0x0D:return('D');break;
	   case 0x0E:return('E');break;
	   case 0x0F:return('F');break; 
	  }
return 0;	  
}//fin get decena to hexadecimal char---------------------------

unsigned char getdecenaHex(unsigned char n){
unsigned char x;	
	  x=n&0xf0;
	  switch(x){
	   case 0x00:return('0');break;
	   case 0x10:return('1');break;
	   case 0x20:return('2');break;
	   case 0x30:return('3');break;
	   case 0x40:return('4');break;
	   case 0x50:return('5');break;
	   case 0x60:return('6');break;
	   case 0x70:return('7');break;
	   case 0x80:return('8');break;
	   case 0x90:return('9');break;
	   case 0xA0:return('A');break;
	   case 0xB0:return('B');break;
	   case 0xC0:return('C');break;
	   case 0xD0:return('D');break;
	   case 0xE0:return('E');break;
	   case 0xF0:return('F');break; 
	  }
return 0;
}//fin get decena to hexadecimal char---------------------------

void sendSPI_DSScomm(void){
	 init_DDS(1);
}// fin send SPI DSS---------------------------------------------------------------------------------------


void DisplayParametrosdeDeteccion(void){
	 unsigned char a[]=" x";
     unsigned char b[]=">Sensibilidad";
	 unsigned char c[]="Fase    ";
	 unsigned char d[]="Altura";
	 unsigned char e[]="Marcar altura";
	 unsigned char f[]="Ganancia";		
	 unsigned char h[]="Frecuencia";
	 unsigned char x[8]={POSXESQUINA,0,0,8,8, 8,8,8};
	 unsigned char y[8]={          0,2,4,6,8,10,12,14};
	 word n;
	    VFDclrscr();
	    VFDposicion(x[0],y[0]);   delay1us();
		VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
		VFDposicion(x[2],y[2]);   delay1us();
		VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
		VFDposicion(x[3],y[3]);   delay1us();
		VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
		VFDposicion(x[4],y[4]);   delay1us();
		VFDserial_SendBlock(&d[0],sizeof(d),&n);delay1us();
		VFDposicion(x[5],y[5]);   delay1us();
		VFDserial_SendBlock(&e[0],sizeof(e),&n);delay1us();
		VFDposicion(x[6],y[6]);   delay1us();
		VFDserial_SendBlock(&f[0],sizeof(f),&n);delay1us();
		VFDposicion(x[7],y[7]);   delay1us();
		VFDserial_SendBlock(&h[0],sizeof(h),&n);delay1us();
		displayUINT_var(POSXPARAMDETECT,POSY4,&Deteccion.Sensibilidad,NONE);
		//displayFase_var();falta de poner
		//displayAltura_var(); falta de poner
		//displayMarcarAltura(); falta de poner
		//displayGanancia(); falta de poner    
		displayPhase_var(POSXPARAMDETECT,POSY6);
		displayCuadroMadre();
		cursorx=POSX0;cursory=POSY4;
	    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.
	    ParamDet=&g;           
	    ParamDet->igxc0=0;//Digito seleccionado 
	    ParamDet->igxc1=0;//No asignado
	    ParamDet->igxc2=0;//No asignado
	    ParamDet->igxc3=0;//No asignado
	    ParamDet->igxc4=0;//No asignadog

}//fin de display de parametros de deteccion del menu Ajustes de avanzados del menu Ajuste parametriso del priducto


//funcion que ACTIVA todos los procesos para que desplieguen los valores de los ADC
void displayADCsValues(void){
	 
   cursory=POSY0;//dejamos el cursory en 0 porque si hay enter se sale de la hoja ejecutiva
   VFDclrscr();			
   displayADCvalores=ACTIVADO;//ACTIVAMOS LA FUNCION de despliague de los numeros en el TIMER_IRQ
   displayADC_IRQ();  
	 
}//fin display ADC valores de los display en numero en la pantalla


//display valor del adc x en pantalla, v es el del resultado converso
void displayADCvalor(signed short int *v,unsigned char posx,unsigned char posy){
unsigned char num[6],*p;
word n;
    p=&num[0];
	VFDposicion(posx,posy);delay1us();
	 //  9/10/15  ver configuracion del adc de la entrada
	convertBTC2chars(p,v);//convert Binary Two Complement to char 
	VFDserial_SendBlock(p,6,&n);delay1us();
}//fin display del valor del ADC en pantalla---------------------------------------------------------------


//regresa el apuntador de los char de los numeros convertidos
void convertBTC2chars(unsigned char *p,signed short int *dato){
//unsigned char arr[]="     0";
char i;
signed short int b[4]={999,99,9,0},k;           

if(*dato==0){
		for(i=0;i<4;i++)
			*(p+i)=' ';
		*(p+5)='0';
		return;}
if(*dato>0){*(p+0)=' ';
	    *(p+1)=getDM(dato);//obtener el char de decena de millar
	    for(i=0,k=1000;i<4;i++,k/=10){
	    	       convert4((p+2+i),b[i],dato,k);
	    	 if(k<1) break;}
	    return;}
if(*dato<0){*(p+0)='-';              //valores negativos
        if(*dato==-32768){*(p+0)='-';*(p+1)='3';*(p+2)='2';*(p+3)='7';*(p+4)='6';
                          *(p+5)='8';//  arr[]="-32768";
                          return;} // (&arr[0]);}
        *dato*=-1;
        *(p+1)=getDM(dato);
        for(i=0,k=1000;i<4;i++,k/=10){
        		    convert4((p+2+i),b[i],dato,k);
        		    	 if(k<1) break;}
        return; }//(&arr[0]);}

	 
}//fin del conversor de Binary Twos Complement to chars -----------------------------------------------------

void convert4(unsigned char *p,signed short int b,signed short int *n, signed short int k){
	 unsigned char j=0;
     unsigned short int i=0;
	 unsigned char a[]={"0123456789"};
	 if(*n<b){i=0;
	          *p='0';}
	 else{
		 while(*n>(b+i)){
			 i+=k;
			 j++;}
		 *n-=i;
		 *p=a[j];}
}//fin convert4----------------------------------------------------------------------------------------


char getDM(signed short int *n){//obtener el char de decena de millar de un signed short integer a comple de dos positivo
//signed short int i;	
	 if(*n>19999){
		  if(*n>29999){ *n-=30000;  return ('3');}
		  else{*n-=20000; return('2');}}
	 else{if(*n>9999){ *n-=10000; return ('1');}
	      else{return ('0'); }}		  
}//fin de get el chat de get Decena de millar--------------------------------------------------------------------

	
void display5UChars(unsigned short int posx,unsigned short int posy,unsigned char *p){
	     VFDposicion(posx,posy);  
	     VFDserial_SendChar(*(p+0)+0x30); //decmil+0x30);
	     VFDserial_SendChar(*(p+1)+0x30);//mil+0x30);
	     VFDserial_SendChar(*(p+2)+0x30);//cent+0x30);
	     VFDserial_SendChar(*(p+3)+0x30);//dec+0x30);
	     VFDserial_SendChar(*(p+4)+0x30);//unidad+0x30);
	     
}//fin de despleigue de 5 chars en array que representan un numero UINT de maximo 99999-------------------------
    	
void pushFIFOcOP_displayUINT_var(unsigned short int posx,unsigned short int posy,unsigned short int *pn,unsigned char operacion){
unsigned char a[5],*p;

	           p=&a[0];
	          if(operacion==RESTAR)
	             if(*pn==0)
	                  *pn=32000;
	             else *pn-=1;
	          if(operacion==SUMAR)
	                if(*pn>31999)
	  	                   *pn=0;
	                else  *pn+=1;
	           getCharsFromUINT_var(p,*pn);
	           pushFIFOcOP_display5UChars(posx,posy,p);
}// fin display sensibilidad variable-----------------------------------------------

void pushFIFOcOP_display5UChars(unsigned short int posx,unsigned short int posy,unsigned char *p){
	         pushFIFOcOP_inicioCMD();
		     pushFIFOcOP_VFDposicion(posx,posy);  
		     pushFIFOcOP_VFDserial_SendChar(*(p+0)+0x30); //decmil+0x30);
		     pushFIFOcOP_VFDserial_SendChar(*(p+1)+0x30);//mil+0x30);
		     pushFIFOcOP_VFDserial_SendChar(*(p+2)+0x30);//cent+0x30);
		     pushFIFOcOP_VFDserial_SendChar(*(p+3)+0x30);//dec+0x30);
		     pushFIFOcOP_VFDserial_SendChar(*(p+4)+0x30);//unidad+0x30);
		     pushFIFOcOP_finCMD();
}//fin de despleigue de 5 chars en array que representan un numero UINT de maximo 99999-------------------------



	
    	
	
    	
	/* setUINT_varxDigito
   	 * resta o suma a un solo digito visto de la forma char de 5 chars en pantalla e.g. 23454 sumar uno a digito 2
 	   tenemos   24454  -> digitos orden:  1,2,3,4,5  
    	   operacion: sumar, restar o none=nada,
	              en opcion none solo se despliega el digito en pantalla en la posicion deseada
    	   posicion x,y: si la posicion es 0,0: no se despliega en pantalla el digito.      
	   digito: indica la posicion del digito 1,2,3,4,5     decmill, mill, cent, dec, unidad      */
unsigned int displaytUINT_varxDigito(unsigned short int posx,unsigned short int posy,char operacion,unsigned short int digito,unsigned short int *pvar){
unsigned char a[5],*p;
unsigned short int var;
	       p=&a[0];                         
   	       var=*pvar;  //se neceSita un apuntador en convertCharstoUINT
	       getCharsFromUINT_var(p,var);    
  	       if (digito==0) 
	            return (unsigned short int)(-1); //error de programa no debe ser cero nunca
  	       posx=digito2posicion(posx);
	       
 	       if(operacion==SUMAR){                // 1,2,3,4,5    //posiciones de DDS
	         if(++a[digito-1]>9)               // 0,1,2,3,4         //POSICIONES EN ARRAY
    	            a[digito-1]=0;
	         if((digito==1)&&(a[digito-1]>3))
    	               a[digito-1]=3;   //evitar que se aumente el decima de millar mas de 30000
    	     *pvar=convertCharstoUINT(p);}
	       if(operacion==RESTAR){
    	         if((a[digito-1]==0)&&(digito>1))
	                a[digito-1]=9;
    	         else{ if((a[digito-1]==0)&&(digito==1))
	                       a[digito-1]=3;
    	               else
	                       a[digito-1]--;}
    	         *pvar=convertCharstoUINT(p); }       
    	    VFDposicion(posx,posy); 
	        VFDserial_SendChar(a[digito-1]+0x30);//solo desplegamos un digito
    	return posx;        
	}// fin de la funcion especial de sumar o restar un digito individual en pantalla a una var UINT 
    	
 	


/* despliega G de ganancia tiene dos valores A|B
* la opcion de pantalla:  DDS es para desplearse cuando el contexto sea el DDS
* operacion es para decirle si hacemos el switch y lo desplegamos o si solo lo desplegamos si hacer el switch
*/ 
void displaySwitchGanancia_var(char pantalla,unsigned char operacion){
	     
      if(pantalla==DDS){//por si esta funcion se usa en el futuro en otra pantalla
				  if(operacion==NONE){//solo queremos desplear la variable
						  VFDposicion(POSXDDSNUMS,POSY8);
						 if(GananciaDDS==YES){
							 VFDserial_SendChar('S');
							 VFDserial_SendChar('i');
							 return;}
						 else{VFDserial_SendChar('N');
							  VFDserial_SendChar('o');return; }}               
				  if((GananciaDDS==YES)||(GananciaDDS==NOP)){//la invertimos y la desplegamos
					  VFDposicion(POSXDDSNUMS,POSY8);
				  if(GananciaDDS==YES){
					  GananciaDDS=NEL;
					  VFDserial_SendChar('N');
					  VFDserial_SendChar('o'); }
				  else {GananciaDDS=YES;
						  VFDserial_SendChar('S');
						  VFDserial_SendChar('i');}
						  displayDDS_Update(); }//fin DDS
					  }
      else
         GananciaDDS=NONE;
	}// fin de despliegue en pantall del cambio de variable actual de la Ganancia
    	
	
void displaySwitchCuadro_var(unsigned char operacion){
	    
    	    if((cuadritoDDS>0x31)||(cuadritoDDS<0x30))
	          cuadritoDDS=0x30;
    	    if(operacion==YES){
	        if(cuadritoDDS=='0')
    	            cuadritoDDS='1';
	        else cuadritoDDS='0';}
    	    VFDposicion(POSXDSSDIGITO1,POSY12);
	        VFDserial_SendChar(cuadritoDDS);
    	    
	}// fin display de la variable cuadrito--------------------------------------------

/*Semaforo para escribir en pantalla DDS, para ya sea escribir puntitos � mover cursor,
 *  no al mismo tiempo porque  se crean characteres raros, ya que los puntitos se crean en el hilo
 *  principal sin delays y el char '>' se mueve en la interrupcion de serialVFD receive
 * */	
unsigned char semaforoDDS(unsigned char operacion){
static unsigned char semDDS;	
   switch(operacion){
    case READ: return semDDS; break;
    case ROJO: semDDS=ROJO;break;
    case VERDE:semDDS=VERDE;break;
    case BUSY:semDDS=BUSY;break;//estamos dentro de un editado de una variable
       }//fin switch
return semDDS;   
}//fin semaforo DDS------------------------------------------------------


/*  Controllador principal que despliegua los valores delos ADC's en la pantalla del DDS 
 	 * en forma de puntitos la funcion se ejecuta como interrupcion en Events.h cada x milisegundos
 	 * 
 	 * DEBE ESTAR INIZIALIZADO EN DSP LA LISTA FIFO
 	 * */
//void displayDDSsignalProcessor(void){
//static unsigned short int x,y;    			
////static unsigned char stateDDSplot;    			
//static unsigned char dat[13],iii;			
//coordn16 coordenadas;
////static unsigned short int countLineSwitch;
//
//if(semaforoDDS(READ)==VERDE){semaforoDDS(ROJO);
// if(menu.contexto.Actual==PANTALLA_DDS){
//   if((DDSstatus==TRUE)&&(EventDDS==TRUE)){//SE EJEcuta cuando estamos en la pantalla DDS  
//	   	        	 switch(stateDDSplot){		
//	   	        	     case DDSPOINTS0:// //disable_keyboard_IRQ();
////	   	        	                     popLIFOdata_DDS(&x,&y);//sacamos los datos ADC de la pila CIRCULAR,                    		
//	   	        	                     stateDDSplot=DDSPOINTS1;		
//	   	        	                     dat[0]=0x28;dat[1]=0x64;dat[2]=0x10;dat[3]=0x01;//dot drawing 1Fh,28h,64h,10h,pen,xL,xH,yL,yH		
//	   	        	                     coordenadas.coord16=x;		
//	   	        	                     dat[4]=coordenadas.byte[LO];		
//	   	        	                     dat[5]=coordenadas.byte[HI];		
//	   	        	                     coordenadas.coord16=y;		
//	   	        	                     dat[6]=coordenadas.byte[LO];		
//	   	        	                     dat[7]=coordenadas.byte[HI];		
//	   	        	                     iii=0;		
//	   	        	                     DDSplotflag=0;//its sent the byte?	 la activa VFDserial_OnTxChar	
//	   	        	                     DDSplotTimer=0xBB;//its spent the delay		
//	   	        	                     VFDserial_SendChar(0x1F);	//first command byte for drawing dot	
//	   	        	                     break;		
//	   	        	     case DDSPOINTS1:if(DDSplotflag!=0xAA)// si se manda char en irq TX se pone 0xAAh		
//	   	        	    	                 break;		
//	   	        	                     if(DDSplotTimer!=0xAA)		
//	   	        	                         break;		
//	   	        	                     if((iii==8)||(iii>7)){		
//	   	        	                    	 stateDDSplot=0;		
//	   	        	                    	 //enable_keyboard_IRQ();		
//	   	        	                         stateDDSplot=0;
//	   	        	                    	 break;}		
//	   	        	                     stateDDSplot=DDSPOINTS1;		
//	   	        	                     DDSplotflag=0;//its sent the byte?		
//	   	        	                     DDSplotTimer=0xBB;//its spent the delay		
//	   	        	                     VFDserial_SendChar(dat[iii++]);		
//	   	        	                     //enable_keyboard_IRQ();		
//	   	        	                     break;//el gap actual es de 1ms entre bytes	
//	   	        	                     
//	   	        	     default:stateDDSplot=DDSPOINTS0;		
//	   	        	             DDSplotTimer=0; 		
//	   	        	             break;		
//	   	        	 }//end switch----------------------		
//	                // }//fin ends eventDDS		
//	    	     }//fin DDSstatus if-----------------------------		
//               }//fin contexto pantala DDS arewe in
//              semaforoDDS(VERDE);}//fin semaforo DDS
//	
//}////display de la senal de los ADC'S en el DDS por interrupcion---------------------------------------
//



/*Convert signed short int  to unsigned short int
* –32,768 to 32,767.    32767----------0------ -32768
                       65535--------32767--------- 0                                   */
void convertSINT2UINT(signed short int sx,signed short int sy,unsigned short int *ux,unsigned short int *uy){
unsigned short int a,b;    
	      a=(unsigned short int)(sx);
   	      b=(unsigned short int)(sy);
	      *ux=a+32768;
  	      *uy=b+32768;
	      
    	     
}// convert SINGNED integer to  Unsigned Integer-----------------------------------------------------------------
    	
	
/*control maestro de despliegue y accion de parametros del DDS  displayDDS_Update
ver las opciones en VFDmenu.h*/ 
void displayDDS_Update(void){
	   // unsigned char a;  
    	    
//	    if(BorrarDDS==YES){
//	    	     DDSaux=POSY0;
//    	         DDS_HANDLER(); //Se ejecuta la pantalla DDS
//	         
//                	         }
	   // else{if(ProdDBNode.Altura==Altura)  a=1;
    	 //    else{if(ProdDBNode.Sensibilidad==Sensibilidad) a=2;
	       //       else{if(ProdDBNode.gananciaDDS==ProdDBNode.gananciaDDS)  a=3;
    	     //           else{if(ProdfDBNode.zoomDDS==ZoomDDS) a=4;
	           //              else{if(ProdDBNode.phase==phase) a=5;
    	         //                 else{if(ProdDBNode.phasefrac==phasefrac) a=6; }}}}}}
	    //aqui va un switch para procesar la a
    	    
	     
    	             
}//fin de control maestro de despliegue y accion de parametros del DDS
    	
//void prueba_SalidasOFF(unsigned char posy){
//	    VFDposicion(POSXDIGPRUEBASSAL,posy);
//	    VFDserial_SendChar('0');
//}//fin pruebas de display del cero del menu de salidas---------------------------------------------------------

//pointer a function como parametro--------------------------------------------
void prueba_SalidasOFF(unsigned char posy,void (*func)(unsigned char)){
	     __asm(nop);
	     VFDposicion(POSXDIGPRUEBASSAL,posy);//delay1us();
	     VFDserial_SendChar('0');
	     func(0xA8);//duracion 28h|0x80 de los puslso  28h=4seg
	     pruebasSalidastimerControl= ACTIVAR_SALIDAS_IO;
}//fin pruebas de rele de rechazo-------------------------------------------------------------------------------


/* edo: es el estado de la salida qu se desea, 0|1|Flash
 *  estado Flash 1xxx xxxx; x:duracion*100=milisegundos
 *   */
void prueba_SalidasON(unsigned char cursory,void (*func)(unsigned char)){
	     VFDposicion(POSXDIGPRUEBASSAL,cursory);delay1us();
	     VFDserial_SendChar('1');
	     func(0x8F);//flash de 15*100=1500mseg //func(TRUE);
	     pruebasSalidastimerControl= ACTIVAR_SALIDAS_IO;
}//fin pruebas de rele de rechazo-------------------------------------------------------------------------------




void displayPruebasEntradas(void){
unsigned char PosicionDigito;
static unsigned char  tacho0,gatin0,sysCHK0,packCHK0,Aux1IN0,Aux2IN0,LockSW0,ResetSW0;
     PosicionDigito=POSXDIGPRUEBAENT;
	 if(isMenuPruebaEntradasON==0xAA){
		   if(tacho0!=getTacho()){
			   tacho0=getTacho();
			   VFDposicion(PosicionDigito,POSY0);
			   if(tacho0==TRUE)
				    VFDserial_SendChar('1');
			   else VFDserial_SendChar('0');}
		   if(gatin0!=getGatin()){
			   gatin0=getGatin();
			   VFDposicion(PosicionDigito,POSY2);
			   if(gatin0==TRUE) 
				   VFDserial_SendChar('1');
			   else VFDserial_SendChar('0');}
		   if(sysCHK0!=sysCHK()){
			   sysCHK0=sysCHK();
		   		VFDposicion(PosicionDigito,POSY4);
		   		if(sysCHK0==TRUE) 
		   		    VFDserial_SendChar('1');
		   	    else VFDserial_SendChar('0');}
		   if(packCHK0!=packCHK()){
			   packCHK0=packCHK();
		   	   VFDposicion(PosicionDigito,POSY6);
		       if(packCHK0==TRUE) 
		   		    VFDserial_SendChar('1');
	     	   else VFDserial_SendChar('0');}
		   if(Aux1IN0!=Aux1IN()){
			   Aux1IN0=Aux1IN();
		   	    VFDposicion(PosicionDigito,POSY8);
		   		if(Aux1IN0==TRUE) 
		   		     VFDserial_SendChar('1');
		   	    else VFDserial_SendChar('0');}
		   if(Aux2IN0!=Aux2IN()){
			   Aux2IN0=Aux2IN();
		   	    VFDposicion(PosicionDigito,POSY10);
		   	    if(Aux2IN0==TRUE) 
		   		     VFDserial_SendChar('1');
		   	    else VFDserial_SendChar('0');}
		   if(LockSW0!=LockSW()){
			   LockSW0=LockSW();
		   	    VFDposicion(PosicionDigito,POSY12);
		   	    if(LockSW0==TRUE) 
		   	       VFDserial_SendChar('1');
		    	   else VFDserial_SendChar('0');}
		   if(ResetSW0!=ResetSW()){
			   ResetSW0=ResetSW();
		   	 	VFDposicion(PosicionDigito,POSY14);
		   		if(ResetSW0==TRUE) 
		   		    VFDserial_SendChar('1');
		   	    else VFDserial_SendChar('0');}
		   		   		   		   		   
	 }}//fin display de los numeros del Menu de pruebas de Entradas

//despleiga Menu del Modo de Ajuste de Producto
void DisplayAjusteProductoMode(void){
	unsigned char a[]="Ajuste de Producto";
	unsigned char b[]=" Automatico";
	unsigned char c[]="Manual";
	unsigned char d[]=">X";
	//unsigned char d[]="x";
	unsigned char x[4]={56,0,8,POSX_ESQ239};//7*8
	unsigned char y[4]={ 0,2,4,0};
	word n;
		
		VFDclrscr();
		VFDposicion(x[0],y[0]);   delay1us();
	    VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();
	    VFDposicion(x[1],y[1]);   delay1us();
	    VFDserial_SendBlock(&b[0],sizeof(b),&n);delay1us();
	    VFDposicion(x[2],y[2]);   delay1us();
	    VFDserial_SendBlock(&c[0],sizeof(c),&n);delay1us();
	    VFDposicion(x[3],y[3]);   delay1us();
	    VFDserial_SendBlock(&d[0],sizeof(d),&n);delay1us();
		cursorx=0;cursory=POSY0;
	    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin de despliegue de Ajuste de producto Modeo


//El ascii dela   Raiz Cuadrada es el decimal 251d.
void displayTextoProcessor(void){//ascci  251=raiz cuadrada
                                 //ascci de x=120  X=88
unsigned char i,*s,j;	
    unsigned char tx[]= {251,' ',' ','x'};//" x";	 
	unsigned char t0[]= "Nombre de Producto";
	unsigned char t1[]= "Escribir clave de acceso:";//Contrasena";
	unsigned char r1[]= "  1  2  3  4  5  6  7  8  9  0";//Y6
	unsigned char r2[]= "  Q  W  E  R  T  Y  U  I  O  P";        //�=A4h
    unsigned char r3[]={' ',' ','A',' ',' ','S',' ',' ','D',' ',' ','F',' ',' ','G',' ',' ',
    		                    'H',' ',' ','J',' ',' ','K',' ',' ','L',' ',' ',0xA5};
	unsigned char r4[]= "  Z  X  C  V  B  N  M  .  ,   ";
	unsigned char r5[]= "  Ma Mi S           <  >  +  -";		
	unsigned char x[7]={ POSX_TEXT_PROCS,0,0,0,0 , 0,  0};
	unsigned char y[7]={   0,0,6,8,10,12, 14};
	word n,m;

	textProc=&g;
	VFDclrscr();	
	VFDposicion(0,0);   delay1us();
	switch(menu.contexto.Anterior){
	  
	  case NUEVO_PRODUCTO:VFDserial_SendBlock(&t0[0],sizeof(t0),&n);delay1us();
	                      for(i=0;i<(NOMBRE_PRODUCTO_SIZE-1);i++)   
	  	                               Text[i]=0;//limpiamos la var.de texto
	  	                  textProc->igxc0=MAYUSCULAS;//iniciamos desplegando mayuculas
	  	                  textProc->igxc1=0;//indice del arreglo del Texto. 
	  	                  textProc->igxc4=0;//no ha habido escritura de ascii  
		                  break;     
	  case NOMBRE_PRODUCTO:VFDserial_SendBlock(&t0[0],sizeof(t0),&n);delay1us();
	                       VFDposicion(0,POSY2);//Posicion del nombre
	                       s=&name[1];
	                       m=length(&name[0],sizeof(name));
	                       VFDserial_SendBlock(s,m,&n);//nombre de producto display
	                       for(i=0,j=1;j<(NOMBRE_PRODUCTO_SIZE-1);i++,j++)//transferimos el nombre a la variable texto
	                             Text[i]=name[j];
	                       textProc->igxc0=MAYUSCULAS;
	                       s=&Text[0];//puntamos al 0
	                       textProc->igxc1=findLastChar(s,NOMBRE_PRODUCTO_SIZE-1);//encontramos el ultimo espacio del ultimo caracter 
	                       textProc->igxc4=0;//no ha habido una escritura de ascii,puede escribir en el espacio
	                       break;    
	  case SUPERVISOR:    VFDserial_SendBlock(&t1[0],sizeof(t1),&n);delay1us();
	  	  	  	  	  	  for(i=0;i<(NOMBRE_PRODUCTO_SIZE-1);i++)   
	  	  	                   Text[i]=0;//limpiam{os la var.de texto
	  	  	  	  	      textProc->igxc0=MAYUSCULAS;//iniciamos desplegando mayuculas
	  	  	  	  	      textProc->igxc1=0;//indice del arreglo del Texto. 
	  	  	  	  	      textProc->igxc4=0;//no ha habido escritura de ascii  
						  break;     
	  default:break;
	}//fin switch	
	VFDposicion(x[0],y[0]);   delay1us();
	VFDserial_SendBlock(&tx[0],sizeof(tx),&n);delay1us();//display the exit control
	displayTextoProcessorMayusculas();//despliega la pantalla de  mayusculas
	
	cursorx=POSX_COL1;cursory=POSY6;
	VFDposicion(cursorx,cursory);
	VFDserial_SendChar('>');
	
}//fin de hoja ejecutiva de despleigue del controlador de texto 


void displayNombreProductoEdit(void){
	 
	   menu.contexto.Actual=TEXT_PROCESSOR;// en donde estaremos
	   menu.contexto.Anterior=NOMBRE_PRODUCTO;//de donde venimos
	   MenuControl();
			   
}//fin de despleigue de edicion de modificacion del productoactual seleccionado



void displayTextoProcessorMayusculas(void){
	word n;

	    unsigned char r1[]= "  1  2  3  4  5  6  7  8  9  0";//Y6
		unsigned char r2[]= "  Q  W  E  R  T  Y  U  I  O  P";        //�=A5h
	    unsigned char r3[]={' ',' ','A',' ',' ','S',' ',' ','D',' ',' ','F',' ',' ','G',' ',' ',
	    		                    'H',' ',' ','J',' ',' ','K',' ',' ','L',' ',' ',0xA5};
		unsigned char r4[]= "  Z  X  C  V  B  N  M  ,  .   ";
		unsigned char r5[]= "  Ma Mi S           <  >  +  -";		
		unsigned char x[7]={ POSX_TEXT_PROCS,0,0,0,0 , 0,  0};
		unsigned char y[7]={   0,0,6,8,10,12, 14};
	
	    VFDposicion(x[2],y[2]);   delay1us();
	    VFDserial_SendBlock(&r1[0],sizeof(r1),&n);delay1us();
		VFDposicion(x[3],y[3]);   delay1us();
		VFDserial_SendBlock(&r2[0],sizeof(r2),&n);delay1us();
		VFDposicion(x[4],y[4]);   delay1us();
		VFDserial_SendBlock(&r3[0],sizeof(r3),&n);delay1us();
		VFDposicion(x[5],y[5]);   delay1us();
		VFDserial_SendBlock(&r4[0],sizeof(r4),&n);delay1us();
		VFDposicion(x[6],y[6]);   delay1us();
		VFDserial_SendBlock(&r5[0],sizeof(r5),&n);delay1us();  
	    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin de depliegue del procesadorr de textos para mayusculas

//despliegue de minusculas del procesador de textos
void displayTextoProcessorMinusculas(void){
	word n;

	        unsigned char r1[]= "  1  2  3  4  5  6  7  8  9  0";//Y6
			unsigned char r2[]= "  q  w  e  r  t  y  u  i  o  p";        //�=A4h
		    unsigned char r3[]={' ',' ','a',' ',' ','s',' ',' ','d',' ',' ','f',' ',' ','g',' ',' ',
		    		                    'h',' ',' ','j',' ',' ','k',' ',' ','l',' ',' ',0xA4};
			unsigned char r4[]= "  z  x  c  v  b  n  m  ,  .   ";
			unsigned char r5[]= "  Ma Mi S           <  >  +  -";		
			unsigned char x[7]={ POSX_TEXT_PROCS,0,0,0,0 , 0,  0};
			unsigned char y[7]={   0,0,6,8,10,12, 14};
		
		    VFDposicion(x[2],y[2]);   delay1us();
		    VFDserial_SendBlock(&r1[0],sizeof(r1),&n);delay1us();
			VFDposicion(x[3],y[3]);   delay1us();
			VFDserial_SendBlock(&r2[0],sizeof(r2),&n);delay1us();
			VFDposicion(x[4],y[4]);   delay1us();
			VFDserial_SendBlock(&r3[0],sizeof(r3),&n);delay1us();
			VFDposicion(x[5],y[5]);   delay1us();
			VFDserial_SendBlock(&r4[0],sizeof(r4),&n);delay1us();
			VFDposicion(x[6],y[6]);   delay1us();
			VFDserial_SendBlock(&r5[0],sizeof(r5),&n);delay1us();  
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin de despliegue de minusculas del menu de proceador de texto

//fin del despleigue de symbolos en el procesador de textos
void displayTextoProcessorSymbol(void){ //�=A4h  //Y6
	word n;

unsigned char r1[]={' ',' ',S1,' ',' ' ,S2,' ',' ' ,S3,' ',' ' ,S4,' ',' ' ,S5,' ',' ' ,S6,' ',' ' ,S7,' ',' ' ,S8,' ',' ' ,S9,' ',' ' ,S10};
unsigned char r2[]={' ',' ',S11,' ',' ',S12,' ',' ',S13,' ',' ',S14,' ',' ',S15,' ',' ',S16,' ',' ',S17,' ',' ',S18,' ',' ',S19,' ',' ',S20};
unsigned char r3[]={' ',' ',S21,' ',' ',S22,' ',' ',S23,' ',' ',S24,' ',' ',S25,' ',' ',S26,' ',' ',S27,' ',' ',S28,' ',' ',S29,' ',' ',S30};
unsigned char r4[]={' ',' ',S31,' ',' ',S32,' ',' ',S33,' ',' ',S34,' ',' ',S35,' ',' ',S36,' ',' ',S37,' ',' ',S38,' ',' ',S39,' ',' ',S40};
unsigned char r5[]={ "  Ma Mi S           <  >  +  -"};		
unsigned char x[7]={ POSX_TEXT_PROCS,0,0,0,0 , 0,  0};
unsigned char y[7]={   0,0,6,8,10,12, 14};
	         
            VFDposicion(x[2],y[2]);   delay1us();
		    VFDserial_SendBlock(&r1[0],sizeof(r1),&n);delay1us();
			VFDposicion(x[3],y[3]);   delay1us();
			VFDserial_SendBlock(&r2[0],sizeof(r2),&n);delay1us();
			VFDposicion(x[4],y[4]);   delay1us();
			VFDserial_SendBlock(&r3[0],sizeof(r3),&n);delay1us();
			VFDposicion(x[5],y[5]);   delay1us();
			VFDserial_SendBlock(&r4[0],sizeof(r4),&n);delay1us();
			VFDposicion(x[6],y[6]);   delay1us();
			VFDserial_SendBlock(&r5[0],sizeof(r5),&n);delay1us();
		    isEnable_Keypad(WAIT);//Desabilita el teclado uno milisegundos.

}//fin del desiegue de simbolos en el menu de procesador de textos----




//despliegue de la cadena de texto en el menu de TEXTO PROCESSOR
void DisplayNewTextProc(void){
unsigned char i,j;
     j=FALSE;
	 VFDposicion(0,POSY2);//POSICION en el reglon 2 donde se despliega la cadena
	 for(i=0;i<NOMBRE_PRODUCTO_SIZE;i++){
		 if(Text[i]==0)
			 j=TRUE;
		 if(j==FALSE)
    	    VFDserial_SendChar(Text[i]);
		 else VFDserial_SendChar(' ');
	      }
	
}//fin de despliegue de la cadena de texto en el menu de TEXTO PROCESSOR


//NOS regresa el ascii en funcion el pantalla actual y las coordenadas actuales, en la pantalla de Text Processor
char getAscii(unsigned char x,unsigned char y,unsigned char pantalla){
	
	 switch(pantalla){
		 case MAYUSCULAS:
			       switch(x){
			    	   case POSX_COL1:if(y==POSY6){
			    		                  return '1'; break;}
			    	                  if(y==POSY8){
			    	                	  return 'Q';break;}
			    	                  if(y==POSY10){
			    	                      return 'A';break;}
			    	                  if(y==POSY12){
			    	                      return 'Z';break;}
			    	                  else break;
			    	   case POSX_COL2:if(y==POSY6){
			    	                      return '2'; break;}
			    	   	              if(y==POSY8){
			    	   	              	  return 'W';break;}
			    	   	              if(y==POSY10){
			    	   	                  return 'S';break;}
			    	   	              if(y==POSY12){
			    	   	                  return 'X';break;}
			    	   	              else break;
			    	   case POSX_COL3:if(y==POSY6){
			                              return '3'; break;}
			    	   	              if(y==POSY8){
			    	   	              	  return 'E';break;}
			    	   	              if(y==POSY10){
			    	   	                  return 'D';break;}
			    	   	              if(y==POSY12){
			    	   	                  return 'C';break;}
			    	   	              else break;
			    	   case POSX_COL4:if(y==POSY6){
			    	                      return '4'; break;}
			    	   	              if(y==POSY8){
			    	                	  return 'R';break;}
		         	                  if(y==POSY10){
			    	                      return 'F';break;}
			     	                  if(y==POSY12){
			     	                      return 'V';break;}
			     	                  else break;
			    	   case POSX_COL5:if(y==POSY6){
			    	   			          return '5'; break;}
			     	                  if(y==POSY8){
			      	                	  return 'T';break;}
			       	                  if(y==POSY10){
			       	                      return 'G';break;}
			      	                  if(y==POSY12){
			      	                      return 'B';break;}
			      	                  else break;
			    	   case POSX_COL6:if(y==POSY6){
			    	                     return '6'; break;}
			    	   		         if(y==POSY8){
			    	   		        	  return 'Y';break;}
			    	   		         if(y==POSY10){
			    	   			          return 'H';break;}
			    	   			     if(y==POSY12){
			    	   			          return 'N';break;}
			    	   			     else break;
			       	   case POSX_COL7:if(y==POSY6){
			     	                      return '7'; break;}
			      	   	              if(y==POSY8){
			      	   	              	  return 'U';break;}
			      	   	              if(y==POSY10){
			      	   	                  return 'J';break;}
			        	              if(y==POSY12){
			      	   	                  return 'M';break;}
			      	   	              else break;
			      	   case POSX_COL8:if(y==POSY6){
			                              return '8'; break;}
			      	   	              if(y==POSY8){
			      	   	              	  return 'I';break;}
			      	   	              if(y==POSY10){
			      	   	                  return 'K';break;}
			      	   	              if(y==POSY12){
			      	   	                  return ',';break;}
			       	   	              else break;
			       	   case POSX_COL9:if(y==POSY6){
			      	                      return '9'; break;}
			    	   	              if(y==POSY8){
			      	                	  return 'O';break;}
			       	                  if(y==POSY10){
			      	                      return 'L';break;}
			       	                  if(y==POSY12){
			      	                      return '.';break;}
			      	                  else break;
			      	   case POSX_COL10:if(y==POSY6){
			      	   			          return '0'; break;}
			       	                  if(y==POSY8){
			       	                	  return 'P';break;}
			       	                  if(y==POSY10){
			       	                      return 0xA5;break;}
			       	                  if(y==POSY12){
			       	                      return ' ';break;}
			       	                  else break;
			      	   default:break;}//fin switch x
			           break;//fin mayuscula
		 case MINUSCULAS:
		       switch(x){
		    	   case POSX_COL1:if(y==POSY6){
		    		                  return '1'; break;}
		    	                  if(y==POSY8){
		    	                	  return 'q';break;}
		    	                  if(y==POSY10){
		    	                      return 'a';break;}
		    	                  if(y==POSY12){
		    	                      return 'z';break;}
		    	                  else break;
		    	   case POSX_COL2:if(y==POSY6){
		    	                      return '2'; break;}
		    	   	              if(y==POSY8){
		    	   	              	  return 'w';break;}
		    	   	              if(y==POSY10){
		    	   	                  return 's';break;}
		    	   	              if(y==POSY12){
		    	   	                  return 'x';break;}
		    	   	              else break;
		    	   case POSX_COL3:if(y==POSY6){
		                              return '3'; break;}
		    	   	              if(y==POSY8){
		    	   	              	  return 'e';break;}
		    	   	              if(y==POSY10){
		    	   	                  return 'd';break;}
		    	   	              if(y==POSY12){
		    	   	                  return 'c';break;}
		    	   	              else break;
		    	   case POSX_COL4:if(y==POSY6){
		    	                      return '4'; break;}
		    	   	              if(y==POSY8){
		    	                	  return 'r';break;}
	         	                  if(y==POSY10){
		    	                      return 'f';break;}
		     	                  if(y==POSY12){
		     	                      return 'v';break;}
		     	                  else break;
		    	   case POSX_COL5:if(y==POSY6){
		    	   			          return '5'; break;}
		     	                  if(y==POSY8){
		      	                	  return 't';break;}
		       	                  if(y==POSY10){
		       	                      return 'g';break;}
		      	                  if(y==POSY12){
		      	                      return 'b';break;}
		      	                  else break;
		    	   case POSX_COL6:if(y==POSY6){
		    	                     return '6'; break;}
		    	   		         if(y==POSY8){
		    	   		        	  return 'y';break;}
		    	   		         if(y==POSY10){
		    	   			          return 'h';break;}
		    	   			     if(y==POSY12){
		    	   			          return 'n';break;}
		    	   			     else break;
		       	   case POSX_COL7:if(y==POSY6){
		     	                      return '7'; break;}
		      	   	              if(y==POSY8){
		      	   	              	  return 'u';break;}
		      	   	              if(y==POSY10){
		      	   	                  return 'j';break;}
		        	              if(y==POSY12){
		      	   	                  return 'm';break;}
		      	   	              else break;
		      	   case POSX_COL8:if(y==POSY6){
		                              return '8'; break;}
		      	   	              if(y==POSY8){
		      	   	              	  return 'i';break;}
		      	   	              if(y==POSY10){
		      	   	                  return 'k';break;}
		      	   	              if(y==POSY12){
		      	   	                  return ',';break;}
		       	   	              else break;
		       	   case POSX_COL9:if(y==POSY6){
		      	                      return '9'; break;}
		    	   	              if(y==POSY8){
		      	                	  return 'o';break;}
		       	                  if(y==POSY10){
		      	                      return 'l';break;}
		       	                  if(y==POSY12){
		      	                      return '.';break;}
		      	                  else break;
		      	   case POSX_COL10:if(y==POSY6){
		      	   			          return '0'; break;}
		       	                  if(y==POSY8){
		       	                	  return 'p';break;}
		       	                  if(y==POSY10){
		       	                      return 0xA4;break;}
		       	                  if(y==POSY12){
		       	                      return ' ';break;}
		       	                  else break;
		      	   default:break;}//fin switch x
			            break;//fin de minuscula
		 case SYMBOL:
		       switch(x){
		    	   case POSX_COL1:if(y==POSY6){
		    		                  return S1; break;}
		    	                  if(y==POSY8){
		    	                	  return S11;break;}
		    	                  if(y==POSY10){
		    	                      return S21;break;}
		    	                  if(y==POSY12){
		    	                      return S31;break;}
		    	                  else break;
		    	   case POSX_COL2:if(y==POSY6){
		    	                      return S2; break;}
		    	   	              if(y==POSY8){
		    	   	              	  return S12;break;}
		    	   	              if(y==POSY10){
		    	   	                  return S22;break;}
		    	   	              if(y==POSY12){
		    	   	                  return S32;break;}
		    	   	              else break;
		    	   case POSX_COL3:if(y==POSY6){
		                              return S3; break;}
		    	   	              if(y==POSY8){
		    	   	              	  return S13;break;}
		    	   	              if(y==POSY10){
		    	   	                  return S23;break;}
		    	   	              if(y==POSY12){
		    	   	                  return S33;break;}
		    	   	              else break;
		    	   case POSX_COL4:if(y==POSY6){
		    	                      return S4; break;}
		    	   	              if(y==POSY8){
		    	                	  return S14;break;}
	         	                  if(y==POSY10){
		    	                      return S24;break;}
		     	                  if(y==POSY12){
		     	                      return S34;break;}
		     	                  else break;
		    	   case POSX_COL5:if(y==POSY6){
		    	   			          return S5; break;}
		     	                  if(y==POSY8){
		      	                	  return S15;break;}
		       	                  if(y==POSY10){
		       	                      return S25;break;}
		      	                  if(y==POSY12){
		      	                      return S35;break;}
		      	                  else break;
		    	   case POSX_COL6:if(y==POSY6){
		    	                     return S6; break;}
		    	   		         if(y==POSY8){
		    	   		        	  return S16;break;}
		    	   		         if(y==POSY10){
		    	   			          return S26;break;}
		    	   			     if(y==POSY12){
		    	   			          return S36;break;}
		    	   			     else break;
		       	   case POSX_COL7:if(y==POSY6){
		     	                      return S7; break;}
		      	   	              if(y==POSY8){
		      	   	              	  return S17;break;}
		      	   	              if(y==POSY10){
		      	   	                  return S27;break;}
		        	              if(y==POSY12){
		      	   	                  return S37;break;}
		      	   	              else break;
		      	   case POSX_COL8:if(y==POSY6){
		                              return S8; break;}
		      	   	              if(y==POSY8){
		      	   	              	  return S18;break;}
		      	   	              if(y==POSY10){
		      	   	                  return S28;break;}
		      	   	              if(y==POSY12){
		      	   	                  return S38;break;}
		       	   	              else break;
		       	   case POSX_COL9:if(y==POSY6){
		      	                      return S9; break;}
		    	   	              if(y==POSY8){
		      	                	  return S19;break;}
		       	                  if(y==POSY10){
		      	                      return S29;break;}
		       	                  if(y==POSY12){
		      	                      return S39;break;}
		      	                  else break;
		      	   case POSX_COL10:if(y==POSY6){
		      	   			          return S10; break;}
		       	                  if(y==POSY8){
		       	                	  return S20;break;}
		       	                  if(y==POSY10){
		       	                      return S30;break;}
		       	                  if(y==POSY12){
		       	                      return S40;break;}
		       	                  else break;
		      	   default:break;}//fin switch x
			       break;//fin del symbolo
		 default:break;//debug aqui software error
	 }//fin primer switch
	
}//fin de getascii nos da el ascii escogido en las cordenadas dadas en la pantalla dada-----------------------------------


//nos da la columna adonde vamos basados en donde estamos
unsigned char getColTextProc(unsigned char dir){
	
	switch(cursorx){
		case POSX_COL1:if(dir==RIGHT)
			              return  POSX_COL2;
			           else return POSX_COL10;
		               break;
		case POSX_COL2:if(dir==RIGHT)
					              return  POSX_COL3;
					           else return POSX_COL1;
				               break;
		case POSX_COL3:if(dir==RIGHT)
					              return  POSX_COL4;
					           else return POSX_COL2;
				               break;
		case POSX_COL4:if(dir==RIGHT)
					              return  POSX_COL5;
					           else return POSX_COL3;
				               break;
		case POSX_COL5:if(dir==RIGHT)
					              return  POSX_COL6;
					           else return POSX_COL4;
				               break;
		case POSX_COL6:if(dir==RIGHT)
					              return  POSX_COL7;
					           else return POSX_COL5;
				               break;
		case POSX_COL7:if(dir==RIGHT)
					              return  POSX_COL8;
					           else return POSX_COL6;
				               break;
		case POSX_COL8:if(dir==RIGHT)
					              return  POSX_COL9;
					           else return POSX_COL7;
				               break;
		case POSX_COL9:if(dir==RIGHT)
					              return  POSX_COL10;
					           else return POSX_COL8;
				               break;
		case POSX_COL10:if(dir==RIGHT)
					              return  POSX_COL1;
					           else return POSX_COL9;
				               break;
		default: return 0; break;																							               
	}//fin switch
	
	
}//fin de get columna a onde vamos--------------------------------

void rotateVarParte11(void){
      if(Parte11AnalogVer=='1')
    	  Parte11AnalogVer='2';
      else Parte11AnalogVer='1';
}//fin rotar la variable de Parte-11---------------------------------------------------------------

void displayvarParte11(void){
	 VFDposicion(136,POSY4);  delay1us();
	 VFDserial_SendChar(Parte11AnalogVer);
}//fin display de variable de parte-11-----------------------------------------------------------





/* Control maestro de la destruccion de los objetos que crean temPorizan y cargan las listas que
 * despliegan la barra de deteccion  */
unsigned char displayBoxDestructor(unsigned char mode){
static unsigned char objeto;
	   switch(mode){
	     case READ: return objeto; break; //leemos si se ha decretado la destruccion de la barra de deteccion porque salimos a otro menu
	     case ACTIVADO:objeto=ACTIVADO;//se ha decretado la destruccion del objeto antes mencionado
	                   return objeto;break;
	     case EJECUTAR: //tenemos que destruir el objeto y evitar cualquier ejecucion en display relacionado a la barra de deteccion
	    	            semb=WAIT; //detenemos el semaforo que evita la ejecucion de la barra de deteccion
	    	            objeto=VERDE; break;
	     case FREE:objeto=VERDE;return 0;break;//para resetear las variables
	     default:break;	     
	   }//fin switch
return 0;
}//fin procedimiento para destruir todos los algoritmos de despliegue de la barra de deteccion


/*Controlador principal de despleigue del Menu de Configuracion de Vibracion
 *  controla el despliegue de numeros en menus donde hay barra de deteccion */
void displayOperativoBarraDet_Numeros(void){
static unsigned char SemCV;	
     switch(SemCV){
       case DISPLAY_FREE:if(displayNumsMenuConfigVibracion(&SemCV)==DISPLAY_FREE)//hay numeros que desplegar?
                             displayBox(&SemCV);             //sino tons, eecutar, cuadros,   pordesplegar  
                         break;
       case DISPLAY_BUSY_NUM:displayNumsMenuConfigVibracion(&SemCV);break;
       case DISPLAY_BUSY_BOX:displayBox(&SemCV);break;
       default:SemCV=DISPLAY_FREE;//Autonitizialitazione
               break;}//fin switch
}//fin del controlador principal de despliegue deL menu DE cONFIGURACION de Vibracion----------------------------



//prende o apaga una box individual--- de la barra de deteccion--------------------------------------
///las primeras 8 box dinamicas son vacias y las otras 9 box dinamicas son llenas
/*DIBUJAMOS cuadro por cuadro y cada vez que dibujamos un cuadro preguntamos si borramos uno o dibujamos otro
 * */
void displayBox(unsigned char *MutexCV){//unsigned char box){
static unsigned short int x1,y1,x2,y2;
//static unsigned char semb; //semaforo para la barra
static unsigned char dat[13],iii,pen,mode;	
coordn16 coordenadas;
static  char box0,box1;
static char f;//flag, controla el cambio de vaciar cuadros a llenar cuadros, que no quede cuadros faltantes.
unsigned char a=0;  

 
  if(menu.contexto.Actual==PORTAL_INICIO)
	  goto dbx1;
  else{if(menu.contexto.Actual==AJUSTE_DE_VIBRACION)
	        goto dbx1;
	   else{if(menu.contexto.Actual==AJUSTE_PARAMETRICO_DE_PRODUCTO)
		        goto dbx1;
	        else{if(menu.contexto.Actual==PARAMETROS_DE_DETECCION){
dbx1:	
     //DISPLAY-BOX
    if(semb!=WAIT){//adelante con la ejecucion si es diferente a wait, osea que ya se ejecuto lo lento de adentro
       //{//ESTA LIBRE PARA graficar cuadros?  hay algun numero que desplegar primero 	
         semb=WAIT;//no habra cambio para datos que se estan procesando en display hasta que acabe
//          box1=popFIFOdisplayBox(); //box1=box;// Guardamos el box hasta que acabemos de deplegar todas las cajas
          if(box1==0){              //de aqui hasta switch aplicamos los cambios o boxes que se deben hacer o borrar
        	 if(box0>0){
        		 if(box0>MAX_BOXES)//initziaizando la variable box0
        			  box0=0;
        		 else semb=WAIT;}     
        	 else{		 
        	    semb=VERDE;
        	    return;}}//fin box1
          if(displayBoxDestructor(READ)==ACTIVADO){//vamos empezar una deteccion y preguntamos si salimos a otro menu terminamos los objetos
          		displayBoxDestructor(EJECUTAR);//selibera los datos y prepara para dejar de ejecutar los datos
                box0=box1=displayBoxDestructor(FREE);
                if(menu.bytes.semMenuPendiente==TRUE)
              		 semMenu2=DONE;//terminamos de graficar
                *MutexCV=0;}//se desabilita el display para este menu y que no entre a numeros ni nada
          
         }//fin WAIT wait hay o no hay cambio de despliegue 	
	     switch(stateDDSplot){		
	                         case DDSPOINTS0:switch(box1){
	                                          case 17:a++;break; 
	                                          case 16:a++;break;
	                                          case 15:a++;break;
	                                          case 14:a++;break;
	                                          case 13:a++;break;
	                                          default:break; }
	                                          stateDDSplot=0;
	                        	              if(box0==box1){
	                        	                 if(f==BAJA_DONE){
	                        	                	 if(box0>0)
	                        	                	    box1--;
	                        	                     f=FALSE;//
	                        	                     semb=WAIT;//falta borrar un cuadro
	                        	                     break;}
	                        	                 semb=VERDE;
	                        	                 break;}
	                                         stateDDSplot=DDSPOINTS1;
	                                         if(box0>box1){//hay que borrar cuadros
	                                        	 pen=0;//borra
	                                             getBoxPattern(box0,&mode,&x1,&y1,&x2,&y2);//box0 se tiene que decrementar despues no antes
	                                             if(box0>0)
	                                            	 box0--;
	                                             f=BAJA; //todavia no se completa una deteccion
	                                             break;}
	                                         if(box1>box0){
	                                        	 pen=1;	 
	                                        	 box0++;//incrementamos el valor box0, para alcanzar box1
	                                        	 getBoxPattern(box0,&mode,&x1,&y1,&x2,&y2);
	                                        	 f=SUBE;//todavia no se completa una deteccion
	                                             break;}                        	 
		   	        	     case DDSPOINTS1:*MutexCV=DISPLAY_BUSY_BOX;//DISPLAY ocupado displaying one box
		   	        	    	             stateDDSplot=DDSPOINTS2;		
		   	        	                     dat[0]=0x28;dat[1]=0x64;dat[2]=0x11;
		   	        	                     dat[3]=mode;
		   	        	                     dat[4]=pen;		
		   	        	                     coordenadas.coord16=x1;		
		   	        	                     dat[5]=coordenadas.byte[LO];		
		   	        	                     dat[6]=coordenadas.byte[HI];		
		   	        	                     coordenadas.coord16=y1;		
		   	        	                     dat[7]=coordenadas.byte[LO];		
		   	        	                     dat[8]=coordenadas.byte[HI];
		   	        	                     coordenadas.coord16=x2;		
		   	        	                     dat[9]=coordenadas.byte[LO];		
		   	        	                     dat[10]=coordenadas.byte[HI];		
		   	        	                     coordenadas.coord16=y2;		
		   	        	                     dat[11]=coordenadas.byte[LO];		
		   	        	                     dat[12]=coordenadas.byte[HI];
		   	        	                     iii=0;		
		   	        	                     DDSplotflag=0;//its sent the byte?	 la activa VFDserial_OnTxChar	
		   	        	                     DDSplotTimer=0xBB;//its spent the delay		
		   	        	                     VFDserial_SendChar(0x1F);	//first command byte for drawing dot	
		   	        	                     break;		
		   	        	     case DDSPOINTS2:if(DDSplotflag!=0xAA)// si se manda char en irq TX se pone 0xAAh		
		   	        	    	                 break;		
		   	        	                     if(DDSplotTimer!=0xAA)		
		   	        	                         break;		
		   	        	                     if((iii==13)||(iii>12)){		
		   	        	                    	 stateDDSplot=0;		
		   	        	                    	 keypad.Status.bits.enable=TRUE;
		   	        	                         stateDDSplot=0;
		   	        	                         if(f==BAJA)
		   	        	                        	 f=BAJA_DONE;//se ha terminado una detecciond e bajada de nivel
		   	        	                         *MutexCV=DISPLAY_FREE;//ya se desplego la box, es libre de usar el display
		   	        	                    	 if(menu.bytes.semMenuPendiente==TRUE)
		   	        	                    		 semMenu2=DONE;//terminamos de graficar
		   	        	                         break;}		
		   	        	                     stateDDSplot=DDSPOINTS2;		
		   	        	                     DDSplotflag=0;//its sent the byte?		
		   	        	                     DDSplotTimer=0xBB;//its spent the delay		
		   	        	                     VFDserial_SendChar(dat[iii++]);		
		   	        	                     //enable_keyboard_IRQ();		
		   	        	                     keypad.Status.bits.enable=TRUE;
		   	        	                     break;//el gap actual es de 1ms entre bytes	
		   	        	                     
		   	        	     default:stateDDSplot=DDSPOINTS0;		
		   	        	             DDSplotTimer=0; 		
		   	        	             break;		
		   	        	 }//end switch----------------------		
		                // }//fin ends eventDDS		
		    	     }
	        else{*MutexCV=0;}//liberammos semaforo ya no graficamos barra de detec.
	        }}}//fin  if contexto-----------------------------		
             
}//fin dela funcion que prende o apaga una box individual de la barra de deteccion---------------------- 


/*PRINCIPAL CONTROLADOR OPERATIVO  CENTRAL DE DESPLEIGUE MAESTRO PARALELO
 *  DE CARACTERES  AL HILO DE DESPLIEGUE DE
 *  BARRA DE DETECCION 
 *   
 *  Despligue en pantalla de nuemeros dentro del menu de configuracion  de vibracion
 * despleigue de datos   uchar  con temporizador controlado
 * Regresamos TRUE si esta libre para graficar cuadros "Boxes"
 * 
 * Pasos para Convertir un Menu Contexto Normal en Contexto Operativo Paralelo ala DEtectionBar
 * 
 *             1.-Ingregar un case Central.
 * 
 * */
unsigned char  displayNumsMenuConfigVibracion(unsigned char *MutexCV1){//despleigue
//static unsigned char SemCV;//semaforo de configuracion de vibracion, para se�alar un despliegue en proceso	
unsigned char num[3],n,j;//u,d,c,n;
static unsigned char stateNumCV;//estado del despleigue de Numeros del Config de Vibracion
const unsigned char  SIZE_DAT_VC=24;
static unsigned char dat[SIZE_DAT_VC],iiii;//datos a ser desplegados
const unsigned short int VECES_POSX=120, VECES_POSY=8;//posicion inicial de los numeros de veces
const unsigned short int ANG_POSX=120, ANG_POSY=12;
unsigned short int xx,yy;
const unsigned char CENT=0,DEC=1,UNIDAD=2;
coordn16 Num16;
static struct overLoad ovdata;

   
   if((*MutexCV1==DISPLAY_FREE)||(*MutexCV1==DISPLAY_BUSY_NUM)){//hay alguna box que se este graficando?
	switch(menu.contexto.Actual){   //PRIMER SWITCH  de contexto
	  case AJUSTE_DE_VIBRACION://estamos en el contexto correcto?
	    switch(stateNumCV){//SEGUNDO  switch  de VIBRACION
	        case NUMDISPCV0:n=popFIFOdisplayCV();
	        	            if(n==0){//buscar en la pila un dato
	        	                *MutexCV1=DISPLAY_FREE;
	                            return DISPLAY_FREE;}//esta libre para graficar cuadros
	        	            else {if((n&0xF0)!=0xF0){ //vemos si es numero de veces o angulo
	        	            	    if(n!=191)//es  el cero del un angulo e.g. 0.3�
	        	            	     if(n>180){//vemos si es el angulo o la fraccion, si la fraccion antes que el numero. no nos sirve
	        	            	    	*MutexCV1=DISPLAY_FREE;
	        	            	    	return DISPLAY_FREE;}
	        	            	    if(n!=191)//el numero entero del angulo no es cero
	        	            	         getUChar2Chars(&num[0],n,YES); //es angulo, obtener los chars del numero
	        	            	    else  {num[CENT]='0';//el numero entero del angulo si es cero
	        	            	            num[DEC]='0';
	        	            	             num[UNIDAD]='0';}
	        	            	     n=popFIFOdisplayCV();//sacamos el decimal fraccionario
	        	            	     if(((n-180)>=0)&&((n-180)<10)){//confirmamos que es dato decimal fraccionario
	        	            	    	 n-=180;//sacamosel dato
	        	            	         n+=0x30;}//loconvertimos a char ascii
	        	            	     else n=0x30;//cero ascii
	        	            	     j='.';//ledecimos que desplegaremos decimales
	        	                     stateNumCV=NUMDISPCV01; }//le decimos que tipo de despliegue sera
	        	                 else{//los numero son apartir de 240, Numeros de veces que se pasa la ferrita hasta el 14
	        	                	  if(n==0xFF){ //INDICA QUE YA se completo la vibracion    		     
	        	                		     stateNumCV=NUMDISPCOMP;
	        	                		     AjustVibr->igxc1=_COMPLETADO_CCV;}
	        	                	  else{	  
	        	                		n=n&0x0f; //extraemos 240d=1111 0000, el numero de 1 a 15.
	        	                	    getUChar2Chars(&num[0],n,YES);//estraemos el numero de veces en chars
		        	                    stateNumCV=NUMDISPCV02; //le decimos que tipo de despliegue sera
	        	                        j=0;
	        	                        if(num[CENT]=='0')
	        	                    	   if(num[DEC]=='0'){
	        	                    	 	  j=' ';n=0;}}
	        	                      }//ledecimoe que desplegaremos enteros
	        	                 if(stateNumCV==NUMDISPCOMP){
	        	                	 xx=LABEL1_CV_X;yy=LABEL1_CV_Y;
	                                 num[CENT]='C';}
	        	                else{ 
	        	                 if(stateNumCV==NUMDISPCV01){//primero mandamos coordenadas de posicion
	        	                	  xx=ANG_POSX;yy=ANG_POSY;}
	        	                 else {xx=VECES_POSX;yy=VECES_POSY;}}
	        	                 Num16.coord16=xx;   //POSICION Para el numero de angulo
	        	                 dat[0]=0x1F;//comando de posicion
	        	                 dat[1]=0x24;//comando de posicion
	        	                 dat[2]=Num16.byte[LO];
	        	                 dat[3]=Num16.byte[HI];
	        	                 Num16.coord16=yy;
	        	                 dat[4]=Num16.byte[LO];
	        	                 dat[5]=Num16.byte[HI];//terminamos posicion
	        	                 if(num[CENT]=='C'){
	        	                	dat[6]= 'C';
	        	                	dat[7]= 'o';
	        	                	dat[8]= 'm';
	        	                	dat[9]= 'p';
	        	                	dat[10]='l';
	        	                	dat[11]='e';
	        	                	dat[12]='t';
	        	                	dat[13]='a';
	        	                	dat[14]='d';
	        	                	dat[15]='o';
	        	                	dat[16]=' ';
	        	                	dat[17]=' ';
	        	                	dat[18]=' ';
	        	                	dat[19]=' ';
	        	                	dat[20]=' ';
	        	                	dat[21]=' ';
	        	                	dat[22]=' ';	        	                		        	                		        	                		        	                		        	                	
	        	                	dat[23]=0;
	        	                	if(AjustVibr->igxc4==WAIT)
	        	                		AjustVibr->igxc4=BUSY;}//>igxm4=BUSY;}//listo para activar enter de teclado
	        	                 else{ 	 
									 if(num[CENT]!='0'){ //centencia mandar el numero, ahy un char?
										 dat[6]=num[CENT];
										 dat[7]=num[DEC];
										 dat[8]=num[UNIDAD];
										 dat[9]=j;
										 dat[10]=n;
										 dat[11]=0;} 
									 else {if(num[DEC]!='0'){//decena
											 dat[6]=num[DEC];
											 dat[7]=num[UNIDAD];
											 dat[8]=j;
											 dat[9]=n;
											 dat[10]=' ';
											 dat[11]=0;}
										   else {dat[6]=num[UNIDAD];
												 dat[7]=j;
												 dat[8]=n;
												 dat[9 ]=' ';
												 dat[10]=' ';
												 dat[11]=0;}}}
	        	                 iiii=0;//index to 0 to do pop
	        	                 CVdispTimer=0xBB;//Inicimos el timer ara el delay
	        	                // SemCV=WAIT;//esta en proceso un despliegue de numero
	        	                 *MutexCV1=DISPLAY_BUSY_NUM;
	        	            	 return DISPLAY_BUSY_NUM;//ya esta ocupado displa, se va  iniciar transmision de datos 
	        	                 }//fin else; 
	        	             break;//NUMDISPCV0
	        case NUMDISPCOMP:  	             
	        case NUMDISPCV01:
	        case NUMDISPCV02:if(DDSplotflag!=0xAA)// si se manda char en irq TX se pone 0xAAh		
	                            break;
	                         if(CVdispTimer!=0xAA)		
	        		   	        break;	
	                         if((iiii>6)&&(dat[iiii]==0)){		
	                        	 stateNumCV=0;
	                        	 *MutexCV1=DISPLAY_FREE;
	                        	 if(AjustVibr->igxc4==BUSY)//4==BUSY)
	                        		 AjustVibr->igxc4=DONE;//4=DONE;//activamos el teclado en este menu para salir de aqui
	                        	 return DISPLAY_FREE; //esta libre el display ya terminamos de desplegar numero
	                           	 break;}		
	                         stateNumCV=NUMDISPCV02;		
	                         DDSplotflag=0;//its sent the byte?		
	                         CVdispTimer=0xBB;//its spent the delay		
	                         VFDserial_SendChar(dat[iiii++]);		
	                         break;//el gap actual es de 1ms entre bytes	
	       default:stateNumCV=NUMDISPCV0;
	               *MutexCV1=DISPLAY_FREE;
	               return DISPLAY_FREE;
	               break;   
	    }//fin switch SEGUNDO DE  VIBRACION
	  break;//fin CASE DE   AJUSTE_DE_VIBRACION-------------------
  default://para cualquier otro contexto--------------------------------------------------------------------
	  switch(stateNumCV){//TERCER SWITCH
	  	        case CHARDISP00:ovdata=popFIFOcOP();
	  	        	            if(ovdata.id==0){//buscar en la pila un dato
	  	        	                *MutexCV1=DISPLAY_FREE;//
	  	                            return DISPLAY_FREE;}//esta libre para graficar cuadros
	  	        	            else{switch(ovdata.id){
	  	        	                  case INICIO_CMD: stateNumCV=CHARDISP01;break; //vamos a mandar un commando
	  	        	                  case VFD_POSICION:stateNumCV=CHARDISP02;break;                          
	  	        	                  case SEND_CHAR:stateNumCV=CHARDISP03;break;
	  	        	                  case SEND_BLOCK:stateNumCV=CHARDISP04;break;
	  	        	                  case FIN_CMD:stateNumCV=CHARDISP05;break;
	  	        	                  default: break; }//fin switch  
	  	        	          *MutexCV1=DISPLAY_BUSY_NUM;
	  	        	          return DISPLAY_BUSY_NUM;}
	  	        case CHARDISP01://  INICIO_CMD-1
	  	        	            stateNumCV=CHARDISP00;
	  	        	            *MutexCV1=DISPLAY_BUSY_NUM;
	  	        	            return DISPLAY_BUSY_NUM; 
	  	        	            break;
	  	        case CHARDISP02://VFD_POSICION-1
	  	        	            Num16.coord16=ovdata.x;//download coord x
	  	        	            dat[0]=0x1F;//comando de posicion
	  	        	          	dat[1]=0x24;//comando de posicion
	  	        	          	dat[2]=Num16.byte[LO];
	  	        	          	dat[3]=Num16.byte[HI];
	  	        	          	Num16.coord16=ovdata.u.y;
	  	        	          	dat[4]=Num16.byte[LO];
	  	        	          	dat[5]=Num16.byte[HI];//terminamos posicion
	  	        	          	dat[6]=0;//fin de execucion
	  	        	          	stateNumCV=CHARDISP10;
	  	        	            iiii=0;//index to 0 to do pop
	  	                        CVdispTimer=0xBB;//Inicimos el timer ara el delay
	  	        	            break;
	  	        case CHARDISP03:dat[0]=ovdata.u.c;
	  	                        dat[1]=0;//fin de dejecucion
	  	                        stateNumCV=CHARDISP10;
	  	      	  	            iiii=0;//index to 0 to do pop
	  	      	  	            CVdispTimer=0xBB;//Inicimos el timer ara el delay
	  	      	  	        	break;
	  	        case CHARDISP04:__asm {Halt;}//exe this line is software error
	  	                       break;
	  	        case CHARDISP05:stateNumCV=0;
	  	        	            *MutexCV1=DISPLAY_FREE;
             	               return DISPLAY_FREE;
             	               break;
	  	        case CHARDISP10:if(DDSplotflag!=0xAA)// si se manda char en irq TX se pone 0xAAh		
	  	                            break;
	  	                         if(CVdispTimer!=0xAA)		
	  	        		   	        break;	
	  	                         if(((iiii>5)&&(dat[iiii]==0))||((iiii==1)&&(dat[iiii]==0))){	//por si una posicion es cero de comando de posicion o si acabamos con un comando de char	
	  	                        	 stateNumCV=CHARDISP11;
	  	                        	 *MutexCV1=DISPLAY_BUSY_NUM;
	  	                        	 return DISPLAY_BUSY_NUM; //esta libre el display ya terminamos de desplegar numero
	  	                           	 break;}		
	  	                         stateNumCV=CHARDISP10;		
	  	                         DDSplotflag=0;//its sent the byte?		
	  	                         CVdispTimer=0xBB;//its spent the delay		
	  	                         VFDserial_SendChar(dat[iiii++]);		
	  	                         break;//el gap actual es de 1ms entre bytes
	  	        case CHARDISP11:stateNumCV=CHARDISP00;
	  	        	            break;
	  	        default:stateNumCV=CHARDISP00;
	  	                *MutexCV1=DISPLAY_FREE;
	  	      	        return DISPLAY_FREE;
	  	                break;
	  
	  
	  break; //break del default---
	    }}}//fin contexto acttual y WAIT y TERCER SWITCH Y PRIMER SWITCH
}//fin controlador principal de despliegue de datos en pantalla de charateres y numeros----------------------


/* esta funcion monitorea si se presiona una tecla para mandar o no el 
 * menu actual al portal inicio en base al timer.    */
void monitorActividad_IRQ(void){

	  timerPortalInicioFlag=TRUE;
	
}//fin monitor actividad IRQ-------------------------------------------




/*funcion que nos da las coordenadas de las cajas en  y las cajas con pen=1 con, caja BOX_VACIA hasta 
 *  la 8,  y la caja BOX_LLENA es de la box=9 hasta box=17; */ 
void getBoxPattern(unsigned char box,unsigned char *mode,unsigned short int *x1,unsigned short int *y1,unsigned short int *x2,unsigned short int *y2){

 switch(menu.contexto.Actual){	
   case PORTAL_INICIO:
    	  switch(box){
			 case 1 : *x1=POS_X1_BOX3_PI;*y1=POS_Y1_BOX3_PI;*x2=POS_X2_BOX3_PI;*y2=POS_Y2_BOX3_PI;break;  
			 case 2 : *x1=POS_X1_BOX4_PI;*y1=POS_Y1_BOX4_PI;*x2=POS_X2_BOX4_PI;*y2=POS_Y2_BOX4_PI;break;
			 case 3 : *x1=POS_X1_BOX5_PI;*y1=POS_Y1_BOX5_PI;*x2=POS_X2_BOX5_PI;*y2=POS_Y2_BOX5_PI;break;
			 case 4 : *x1=POS_X1_BOX6_PI;*y1=POS_Y1_BOX6_PI;*x2=POS_X2_BOX6_PI;*y2=POS_Y2_BOX6_PI;break;
			 case 5 : *x1=POS_X1_BOX7_PI;*y1=POS_Y1_BOX7_PI;*x2=POS_X2_BOX7_PI;*y2=POS_Y2_BOX7_PI;break;
			 case 6 : *x1=POS_X1_BOX8_PI;*y1=POS_Y1_BOX8_PI;*x2=POS_X2_BOX8_PI;*y2=POS_Y2_BOX8_PI;break;
			 case 7 : *x1=POS_X1_BOX9_PI;*y1=POS_Y1_BOX9_PI;*x2=POS_X2_BOX9_PI;*y2=POS_Y2_BOX9_PI;break;
			 case 8 : *x1=POS_X1_BOX10_PI;*y1=POS_Y1_BOX10_PI;*x2=POS_X2_BOX10_PI;*y2=POS_Y2_BOX10_PI;break;
	   		 case 9 : *x1=POS_X1_BOX11_PI;*y1=POS_Y1_BOX11_PI;*x2=POS_X2_BOX11_PI;*y2=POS_Y2_BOX11_PI;break;
			 case 10: *x1=POS_X1_BOX12_PI;*y1=POS_Y1_BOX12_PI;*x2=POS_X2_BOX12_PI;*y2=POS_Y2_BOX12_PI;break;
			 case 11: *x1=POS_X1_BOX13_PI;*y1=POS_Y1_BOX13_PI;*x2=POS_X2_BOX13_PI;*y2=POS_Y2_BOX13_PI;break;
			 case 12: *x1=POS_X1_BOX14_PI;*y1=POS_Y1_BOX14_PI;*x2=POS_X2_BOX14_PI;*y2=POS_Y2_BOX14_PI;break;
			 case 13: *x1=POS_X1_BOX15_PI;*y1=POS_Y1_BOX15_PI;*x2=POS_X2_BOX15_PI;*y2=POS_Y2_BOX15_PI;break;
	         case 14: *x1=POS_X1_BOX16_PI;*y1=POS_Y1_BOX16_PI;*x2=POS_X2_BOX16_PI;*y2=POS_Y2_BOX16_PI;break;
			 case 15: *x1=POS_X1_BOX17_PI;*y1=POS_Y1_BOX17_PI;*x2=POS_X2_BOX17_PI;*y2=POS_Y2_BOX17_PI;break;
			 case 16: *x1=POS_X1_BOX18_PI;*y1=POS_Y1_BOX18_PI;*x2=POS_X2_BOX18_PI;*y2=POS_Y2_BOX18_PI;break;
			 case 17: *x1=POS_X1_BOX19_PI;*y1=POS_Y1_BOX19_PI;*x2=POS_X2_BOX19_PI;*y2=POS_Y2_BOX19_PI;break; 
			 default:*x1=POS_X1_BOX3_PI;*y1=POS_Y1_BOX3_PI;*x2=POS_X2_BOX3_PI;*y2=POS_Y2_BOX3_PI;
			         box=1;break;//iniciaiacion del box0
			 }//fin switch
	         if(box<9)
	        	 *mode=BOX_VACIA;
	         else *mode=BOX_LLENA;
	         break;// case PORTAL INICIO   Completado
    case AJUSTE_DE_VIBRACION:
        switch(box){
		 case 1 : *x1=POS_X1_BOX3_AV;*y1=POS_Y1_BOX3_AV;*x2=POS_X2_BOX3_AV;*y2=POS_Y2_BOX3_AV;break;  
		 case 2 : *x1=POS_X1_BOX4_AV;*y1=POS_Y1_BOX4_AV;*x2=POS_X2_BOX4_AV;*y2=POS_Y2_BOX4_AV;break;
		 case 3 : *x1=POS_X1_BOX5_AV;*y1=POS_Y1_BOX5_AV;*x2=POS_X2_BOX5_AV;*y2=POS_Y2_BOX5_AV;break;
		 case 4 : *x1=POS_X1_BOX6_AV;*y1=POS_Y1_BOX6_AV;*x2=POS_X2_BOX6_AV;*y2=POS_Y2_BOX6_AV;break;
		 case 5 : *x1=POS_X1_BOX7_AV;*y1=POS_Y1_BOX7_AV;*x2=POS_X2_BOX7_AV;*y2=POS_Y2_BOX7_AV;break;
		 case 6 : *x1=POS_X1_BOX8_AV;*y1=POS_Y1_BOX8_AV;*x2=POS_X2_BOX8_AV;*y2=POS_Y2_BOX8_AV;break;
		 case 7 : *x1=POS_X1_BOX9_AV;*y1=POS_Y1_BOX9_AV;*x2=POS_X2_BOX9_AV;*y2=POS_Y2_BOX9_AV;break;
		 case 8 : *x1=POS_X1_BOX10_AV;*y1=POS_Y1_BOX10_AV;*x2=POS_X2_BOX10_AV;*y2=POS_Y2_BOX10_AV;break;
   		 case 9 : *x1=POS_X1_BOX11_AV;*y1=POS_Y1_BOX11_AV;*x2=POS_X2_BOX11_AV;*y2=POS_Y2_BOX11_AV;break;
		 case 10: *x1=POS_X1_BOX12_AV;*y1=POS_Y1_BOX12_AV;*x2=POS_X2_BOX12_AV;*y2=POS_Y2_BOX12_AV;break;
		 case 11: *x1=POS_X1_BOX13_AV;*y1=POS_Y1_BOX13_AV;*x2=POS_X2_BOX13_AV;*y2=POS_Y2_BOX13_AV;break;
		 case 12: *x1=POS_X1_BOX14_AV;*y1=POS_Y1_BOX14_AV;*x2=POS_X2_BOX14_AV;*y2=POS_Y2_BOX14_AV;break;
		 case 13: *x1=POS_X1_BOX15_AV;*y1=POS_Y1_BOX15_AV;*x2=POS_X2_BOX15_AV;*y2=POS_Y2_BOX15_AV;break;
         case 14: *x1=POS_X1_BOX16_AV;*y1=POS_Y1_BOX16_AV;*x2=POS_X2_BOX16_AV;*y2=POS_Y2_BOX16_AV;break;
		 case 15: *x1=POS_X1_BOX17_AV;*y1=POS_Y1_BOX17_AV;*x2=POS_X2_BOX17_AV;*y2=POS_Y2_BOX17_AV;break;
		 case 16: *x1=POS_X1_BOX18_AV;*y1=POS_Y1_BOX18_AV;*x2=POS_X2_BOX18_AV;*y2=POS_Y2_BOX18_AV;break;
		 case 17: *x1=POS_X1_BOX19_AV;*y1=POS_Y1_BOX19_AV;*x2=POS_X2_BOX19_AV;*y2=POS_Y2_BOX19_AV;break; 
		 default:*x1=POS_X1_BOX3_AV;*y1=POS_Y1_BOX3_AV;*x2=POS_X2_BOX3_AV;*y2=POS_Y2_BOX3_AV;
		         box=1;break;//iniciaiacion del box0
		 }//fin switch
         if(box<9)
        	 *mode=BOX_VACIA;
         else *mode=BOX_LLENA;
       break;//  //fin AJUSTE DE VIBRACION
   case AJUSTE_PARAMETRICO_DE_PRODUCTO:
    		switch(box){
    				 case 1 : *x1=POS_X1_BOX3_AP;*y1=POS_Y1_BOX3_AP;*x2=POS_X2_BOX3_AP;*y2=POS_Y2_BOX3_AP;break;  
    				 case 2 : *x1=POS_X1_BOX4_AP;*y1=POS_Y1_BOX4_AP;*x2=POS_X2_BOX4_AP;*y2=POS_Y2_BOX4_AP;break;
    				 case 3 : *x1=POS_X1_BOX5_AP;*y1=POS_Y1_BOX5_AP;*x2=POS_X2_BOX5_AP;*y2=POS_Y2_BOX5_AP;break;
    				 case 4 : *x1=POS_X1_BOX6_AP;*y1=POS_Y1_BOX6_AP;*x2=POS_X2_BOX6_AP;*y2=POS_Y2_BOX6_AP;break;
    				 case 5 : *x1=POS_X1_BOX7_AP;*y1=POS_Y1_BOX7_AP;*x2=POS_X2_BOX7_AP;*y2=POS_Y2_BOX7_AP;break;
    				 case 6 : *x1=POS_X1_BOX8_AP;*y1=POS_Y1_BOX8_AP;*x2=POS_X2_BOX8_AP;*y2=POS_Y2_BOX8_AP;break;
    				 case 7 : *x1=POS_X1_BOX9_AP;*y1=POS_Y1_BOX9_AP;*x2=POS_X2_BOX9_AP;*y2=POS_Y2_BOX9_AP;break;
    				 case 8 : *x1=POS_X1_BOX10_AP;*y1=POS_Y1_BOX10_AP;*x2=POS_X2_BOX10_AP;*y2=POS_Y2_BOX10_AP;break;
    		   		 case 9 : *x1=POS_X1_BOX11_AP;*y1=POS_Y1_BOX11_AP;*x2=POS_X2_BOX11_AP;*y2=POS_Y2_BOX11_AP;break;
    				 case 10: *x1=POS_X1_BOX12_AP;*y1=POS_Y1_BOX12_AP;*x2=POS_X2_BOX12_AP;*y2=POS_Y2_BOX12_AP;break;
    				 case 11: *x1=POS_X1_BOX13_AP;*y1=POS_Y1_BOX13_AP;*x2=POS_X2_BOX13_AP;*y2=POS_Y2_BOX13_AP;break;
    				 case 12: *x1=POS_X1_BOX14_AP;*y1=POS_Y1_BOX14_AP;*x2=POS_X2_BOX14_AP;*y2=POS_Y2_BOX14_AP;break;
    				 case 13: *x1=POS_X1_BOX15_AP;*y1=POS_Y1_BOX15_AP;*x2=POS_X2_BOX15_AP;*y2=POS_Y2_BOX15_AP;break;
    		         case 14: *x1=POS_X1_BOX16_AP;*y1=POS_Y1_BOX16_AP;*x2=POS_X2_BOX16_AP;*y2=POS_Y2_BOX16_AP;break;
    				 case 15: *x1=POS_X1_BOX17_AP;*y1=POS_Y1_BOX17_AP;*x2=POS_X2_BOX17_AP;*y2=POS_Y2_BOX17_AP;break;
    				 case 16: *x1=POS_X1_BOX18_AP;*y1=POS_Y1_BOX18_AP;*x2=POS_X2_BOX18_AP;*y2=POS_Y2_BOX18_AP;break;
    				 case 17: *x1=POS_X1_BOX19_AP;*y1=POS_Y1_BOX19_AP;*x2=POS_X2_BOX19_AP;*y2=POS_Y2_BOX19_AP;break; 
    				 default:*x1=POS_X1_BOX3_AP;*y1=POS_Y1_BOX3_AP;*x2=POS_X2_BOX3_AP;*y2=POS_Y2_BOX3_AP;
    				         box=1;break;//iniciaiacion del box0
    				 }//fin switch
    		         if(box<9)
    		        	 *mode=BOX_VACIA;
    		         else *mode=BOX_LLENA;
    	   break;   //FIN ajuste PARAMETRICO DE PRODUCTO
   case PARAMETROS_DE_DETECCION:
      		switch(box){
      				 case 1 : *x1=POS_X1_BOX3_PD;*y1=POS_Y1_BOX3_PD;*x2=POS_X2_BOX3_PD;*y2=POS_Y2_BOX3_PD;break;  
       				 case 2 : *x1=POS_X1_BOX4_PD;*y1=POS_Y1_BOX4_PD;*x2=POS_X2_BOX4_PD;*y2=POS_Y2_BOX4_PD;break;
       				 case 3 : *x1=POS_X1_BOX5_PD;*y1=POS_Y1_BOX5_PD;*x2=POS_X2_BOX5_PD;*y2=POS_Y2_BOX5_PD;break;
       				 case 4 : *x1=POS_X1_BOX6_PD;*y1=POS_Y1_BOX6_PD;*x2=POS_X2_BOX6_PD;*y2=POS_Y2_BOX6_PD;break;
       				 case 5 : *x1=POS_X1_BOX7_PD;*y1=POS_Y1_BOX7_PD;*x2=POS_X2_BOX7_PD;*y2=POS_Y2_BOX7_PD;break;
       				 case 6 : *x1=POS_X1_BOX8_PD;*y1=POS_Y1_BOX8_PD;*x2=POS_X2_BOX8_PD;*y2=POS_Y2_BOX8_PD;break;
       				 case 7 : *x1=POS_X1_BOX9_PD;*y1=POS_Y1_BOX9_PD;*x2=POS_X2_BOX9_PD;*y2=POS_Y2_BOX9_PD;break;
       				 case 8 : *x1=POS_X1_BOX10_PD;*y1=POS_Y1_BOX10_PD;*x2=POS_X2_BOX10_PD;*y2=POS_Y2_BOX10_PD;break;
       		   		 case 9 : *x1=POS_X1_BOX11_PD;*y1=POS_Y1_BOX11_PD;*x2=POS_X2_BOX11_PD;*y2=POS_Y2_BOX11_PD;break;
       				 case 10: *x1=POS_X1_BOX12_PD;*y1=POS_Y1_BOX12_PD;*x2=POS_X2_BOX12_PD;*y2=POS_Y2_BOX12_PD;break;
       				 case 11: *x1=POS_X1_BOX13_PD;*y1=POS_Y1_BOX13_PD;*x2=POS_X2_BOX13_PD;*y2=POS_Y2_BOX13_PD;break;
       				 case 12: *x1=POS_X1_BOX14_PD;*y1=POS_Y1_BOX14_PD;*x2=POS_X2_BOX14_PD;*y2=POS_Y2_BOX14_PD;break;
       				 case 13: *x1=POS_X1_BOX15_PD;*y1=POS_Y1_BOX15_PD;*x2=POS_X2_BOX15_PD;*y2=POS_Y2_BOX15_PD;break;
       		         case 14: *x1=POS_X1_BOX16_PD;*y1=POS_Y1_BOX16_PD;*x2=POS_X2_BOX16_PD;*y2=POS_Y2_BOX16_PD;break;
       				 case 15: *x1=POS_X1_BOX17_PD;*y1=POS_Y1_BOX17_PD;*x2=POS_X2_BOX17_PD;*y2=POS_Y2_BOX17_PD;break;
       				 case 16: *x1=POS_X1_BOX18_PD;*y1=POS_Y1_BOX18_PD;*x2=POS_X2_BOX18_PD;*y2=POS_Y2_BOX18_PD;break;
       				 case 17: *x1=POS_X1_BOX19_PD;*y1=POS_Y1_BOX19_PD;*x2=POS_X2_BOX19_PD;*y2=POS_Y2_BOX19_PD;break; 
       				 default:*x1=POS_X1_BOX3_PD;*y1=POS_Y1_BOX3_PD;*x2=POS_X2_BOX3_PD;*y2=POS_Y2_BOX3_PD;
       				         box=1;break;//iniciaiacion del box0
       				 }//fin switch
       		         if(box<9)
       		        	 *mode=BOX_VACIA;
       		         else *mode=BOX_LLENA;
        	   break;   //FIN ajuste PARAMETRICO DE PRODUCTO
      default:break;
      }//fin PRIMER SWITCH           	   
      		    		     	  
}//fin de get patter of the boxes-------------------------------------------------------------------------------

/* cambiar por timer en menu actual de que estamos al contexto 
 * PORTAL INICIO */
void Cambiar_a_Portal_Inicio(void){
	
	
}//fin cambiar a portal inicio----------------------------


/* esta funcion regresa true si hay mas productos para
 * seleccionar aparte del TestSetUP si no hay mas de 
 * producto aparte del TestSetUp regresa false. */
unsigned char isNotEmptyAllSelectMenu(void){  
	   if(EEPROMreadProductsCantidad())
		      return FALSE;//NO HAY MAS prodctos para seleccionar
	   else return TRUE;//si hay mas productos para seleccionar
}//fin de isNotEmptyAllSelectMenu--------------------------------------
//fin de isNotEmptyAllSelectMenu--------------------------------------






/*Estas*/

/**********************FIN HOJAS EJECUTIVAS*********************************************************************/
/**********************FIN HOJAS EJECUTIVAS*********************************************************************/
/**********************FIN HOJAS EJECUTIVAS*********************************************************************/
/**********************FIN HOJAS EJECUTIVAS*********************************************************************/
/**********************FIN HOJAS EJECUTIVAS*********************************************************************/
/**********************FIN HOJAS EJECUTIVAS*********************************************************************/
/**********************FIN HOJAS EJECUTIVAS*********************************************************************/
/**********************FIN HOJAS EJECUTIVAS*********************************************************************/
/**********************FIN HOJAS EJECUTIVAS*********************************************************************/
              
unsigned char configTextoProcessorController(unsigned char contextOrigen,unsigned char contextDestino){
	menu.contexto.Anterior=contextOrigen;
	menu.contexto.Modificado=contextDestino;
return 0;
}//fin de configuracion del texto Processor conctroller
			







/*Pregunta si se modifico la configuracion, si asi fue: ve en que contexto fue, guarda el contexto padre
 * adonde se dirigia y activa como contexto actual "esta seguro", reseta la variable modificado y retorna el
 * contexto estas seguro, sino fue modificado entonces retorna el contexto padre a donde se dirigia */
unsigned char  configModificacionController(unsigned char contextopadre,unsigned char contextomodificado1){
   unsigned char contexto; 
	switch(modificado){//Nombre del Menu modificado??
	   case NOP:if(contextopadre==AJUSTES_AVANZADOS)
		   	   	   contexto=contextomodificado1;
	   	   	   	else{//no se modifico el menu!!
	   	   	   		switch(contextomodificado1){
	   	   	   		  case INGENIERIA_GRACIDA2:contexto=contextopadre;break;
	   	   	   		  case AJUSTE_PARAMETRICO_DE_PRODUCTO:contexto=contextopadre;break;
	   	   	   			                                   break;
	   	   	   		  default:contexto=contextomodificado1;break;}}
	            break;//no hubo modificaciones
	   case CONFIGURAR_ENTRADAS_DE_SISTEMA: 
	   case ID_COMUNICACIONES:  
	   case TIPO_DE_MAQUINA:;
	   case GANANCIA_: 
	   case FILTRO_:                 
	   case INGENIERIA_GRACIDA2:
	   case FRECUENCIA_SELECT:
	   case RELOJ_DE_SISTEMA:
	   case CONTROL_PASSWORD:	   	   
	   case COPY_PRODUCTO:
	   case SELECCIONAR_PRODUCTO:
	   case BORRAR_PRODUCTO:
	   case PARTE_11:  menu.contexto.Anterior=contextopadre;
	   	   	   	   	   menu.contexto.Modificado=contextomodificado1;
	   	  	           contexto=ESTAS_SEGURO;
	   	  	           break;              
	   case MENU_ADMINISTRATIVO:
		   	   	   	   menu.contexto.Anterior=contextopadre;
		   	   	   	   menu.contexto.Modificado=contextomodificado1;
			   		   contexto=menu.contexto.Anterior; //OJO  no preguntamos si lo queremos guardar solo lo Hacemos
		               MemoEEPROM(SAVE,menu.contexto.Modificado);
		               break;
	   case AJUSTE_DE_VIBRACION:
	   case AJUSTE_PARAMETRICO_DE_PRODUCTO:
		                  if(menu.contexto.Anterior==AJUSTES_AVANZADOS){
		                	  menu.contexto.Modificado=AJUSTE_PARAMETRICO_DE_PRODUCTO;//contexto que se modifico
		                	  contexto=contextomodificado1;//contexto al que vamos
		                	  menu.contexto.Anterior=AJUSTES_AVANZADOS;//Contexto del que venimos
		                	  break; }
	   case PARAMETROS_DE_DETECCION: 	               
	   case PASTE_PRODUCTO:
	   case AJUSTES_AVANZADOS:	   
		   	   	   	   	   menu.contexto.Anterior=contextopadre;
	   	   		   	   	   menu.contexto.Modificado=contextomodificado1;
	   	   		   	  	   contexto=ESTAS_SEGURO2;break;   
	   case NUEVO_PRODUCTO:
	   		   	   	   	   menu.contexto.Modificado=contextomodificado1;
	   		   	   	   	   contexto=ESTAS_SEGURO2;break;                              
	   default:// error de software
			   break;
	}//fin switch-------------------------------------------------
	modificado=NOP;//se resetea la variable modnificado.
return(contexto);
}// fin de la configuracion de modificacion controlador---------------------------------------------






void displayTimer_IRQ(void){//INTERRUPCION 500microsegundos
	     static char c;
		 static short int k,j,m;
		 static unsigned short int DDSout;//ESPERAMOS a que se grafique el ultimo puntito para salir de la pantalla DDS
		 static unsigned long int smartDelaySalidas;
		 static unsigned long int DDSplotTimerCounter;//contador para generar un delay para el display de puntitos del DDS
		 static unsigned long int EEPROMdelayCounter;//contador para el delay de escritura de la a EEPROM
		 static char ADCbusyDelay;//delay del busy para que no se emparme con el primer burst
		 static unsigned long int waitDDS;
		 static unsigned char DriverOutVar;//para hacer un timer de activacion de la salida de driver
		 static unsigned long int DriverOutCounter; //timer para activacion de la salida del driver
		 static unsigned long int BARplotTimerCounter;//contador para generar delay para el display los boxes
		 static unsigned int CVdispTimerCounter;//contador para generar un delay para el menu de configuracion de Vibracion
	     static unsigned long int timerPortalInicioCounter;
		 static unsigned char countMeasures;//contador para los ADC de los voltages
	     static unsigned char countADC;
//	     static unsigned short int deteccionOFFSETcounter;//descuenta por tiempo una deteccion a la vez, a la variable que acomula las detecciones
	     
	     
	     if((TM1_IRQ1&0x10)==0x10){//flag de temporizador del PORTAL INICIO
	     	       TM1_IRQ1&=0xEF; //1110 1111 
	     	       if(timerPortalInicioFlag)
	     	     		timerPortalInicioCounter=0;
	    		   timerPortalInicioFlag=FALSE;
	     	       if(timerPortalInicioCounter++>25000){
	     	     		timerPortalInicioCounter=0;}
	     	       Cambiar_a_Portal_Inicio();}
	     	     	     
	     if((TM1_IRQ1&0x01)==0x01){
	     	       TM1_IRQ1|=0xFE; //1111 1110
	     	       if(j++>300){j=0;
	     	     			   displayADC_IRQ();}//funcion que despiega el valor de los ADC en pantalla, en VFDmenu
	     	   	   if(dds.Bnderas.bit.DDS1_TIMER==TRUE){
	     	     	     if(waitDDS++>4200){waitDDS=0;
	     	    	             	 dds.Bnderas.bit.EventDDS=TRUE;
	     	     	                 dds.Bnderas.bit.DDS1_TIMER=FALSE;}}// .EventDDS=TRUE;}}
	     	     		    if(DDSstatus==0xAAU){//ESPERAMOS un rato  a que termine de graficar el ultimo puntito 
	     	     		            if(DDSout++>400){// y se despliegue en la pantalla a la que vamos
	     	     		            	     DDSout=0;DDSstatus=0;}}//YA SAlimos del DDS y se grafico todo
	     	     		    
	     	     		    //esperamos que termine de desplegar los caracteres para activar la
	     	     		    if(DDSstatus==0xBBU){//graficacion de puntitos     
	     	     		         if(DDSout++>350){//<<--700  NO MOVER ESTE VALOR,, con valores menores de Blanck the screen, erasing all chars just left the graphnums
	     	     		        	 DDSout=0; DDSstatus=TRUE;}}//se Activo la graficacion de puntitos.
	     	     		    
	     	     		    
	     	     		   if(pruebasSalidastimerControl!=0){//SMART_DELAY_DURATION 
	     	     			 if(smartDelaySalidas>(SMART_DELAY_DURATION*2))//init
	     	     				 smartDelaySalidas=SMART_DELAY_DURATION-1;//init
	     	     			 else{
	     	     			    if(smartDelaySalidas<SMART_DELAY_DURATION)
	     	     				   smartDelaySalidas--;
	     	     		        if(smartDelaySalidas<10U){
	     	     		              smartDelaySalidas=SMART_DELAY_DURATION*2+4;
	     	     		     
	     	     		     		 
	     	     		     		  prueba_SalidasOFF(POSY2,&ReleRechazo);
	     	     		    		  prueba_SalidasOFF(POSY4,&ReleFalla);
	     	     		    		  prueba_SalidasOFF(POSY6,&ReleAlarma);
	     	     		    		  prueba_SalidasOFF(POSY8,&ReleBloqueo);
	     	     		    		  //prueba_SalidasOFF(POSY10);
	     	     		    		  //SalidaAux(FALSE);
	     	     		    		 // prueba_SalidasOFF(POSY10,&ReleRechazo);
	     	     		    		  pruebasSalidastimerControl=0;}
	     	     		    	 
	     	     		      }}//fin if smartDelaySalidas-----------------------------------
	     	     		   
	     	     		 /*//200=101ms entre byte y byte@19200
	     	     		  * 5=2.9ms entre byte y byte
	     	     		  * 2=1.4ms
	     	     		  * */   
	     	     		 if(DDSplotTimer==0xBB){  
	     	     		   if(DDSplotTimerCounter++>2){//2<--debug revisar que  poner estaba en anteriormente como 1
	     	     			        DDSplotTimerCounter=0;
	     	     			        DDSplotTimer=0xAA;}}
	     	     		 
	     	     		 //control para el dspliegue de la barra de deteccion
	     	     		 if(BARplotTimer==0xBB){  //funcion abandonada
	     	     			   if(BARplotTimerCounter++>11){//2<--debug revisar que  poner estaba en anteriormente como 1
	     	     				        BARplotTimerCounter=0;
	     	     				        BARplotTimer=0xAA;}} //debug  decrepated funcion
	     	     		 
	     	     		 if(CVdispTimer==0xBB){
	     	     			 if(CVdispTimerCounter++>3){//estaba en 10
	     	     				  CVdispTimerCounter=0;
	     	     				  CVdispTimer=0xAA;}}
	     	     		 
	     	     		 
	     	     		    //con 35000 son 23 segundos, tons 152 por segundo
	     	     		    //    
	     	     		    if(DriverOutcontrol==ACTIVADO){
	     	     		      if(DriverOutCounter++>450){//ya pasaron 3 segundos?
	     	     		    	  DriverOutcontrol=DESACTIVADO;//desactivamos este control para que ya no se ejecute
	     	     		    	  DriverOutCounter=0;
	     	     		    	  DriveOutAttPin(OFF);//Despues de 3seg de init, desactivamos la Attenuacion de la se�al DriverOut
	     	     		      }}//fin control de activacion de driver
	     	     		   	
	     	     		   

			 if((menu.contexto.Actual==MEDIDAS_DE_VOLTAJE)||(menu.contexto.Actual==MEDIDAS_DE_VOLTAJES)){
				 if(Mediciones.flagMesurements==FALSE){
					if(countADC++>80){//<-Char
						countADC=0;
						ADC_BAL_Start();//iniciar la lectura de los ADC
						if(countMeasures++>90){//<-char 
							countMeasures=0;	
							Mediciones.flagMesurements=TRUE;}}}}		 
	     	     		   
			 
			if(Deteccion.EnCurso==NOP){ //se incrementa con cada deteccion,
	     	   if(Offset.Counter>4000){//deteccionOFFSETcounter++>3200) {//el tiempo lo decrementa, para poder 
	     		      //deteccionOFFSETcounter=0;//seguir evaluando el offset
			          Offset.Status|=TRUE;}//se autoriza calcular offset
	     	   else  {Offset.Status&=FIRST_START;//reset offset
	     	          Offset.Counter++;}}//No estamos listos para evaluar el OFFSET
			else{Offset.Status&=FIRST_START;
			     Offset.Counter=0;}//reseta contador de tiempo sin rechazos	   
				
			isEnable_Keypad(0);//le dice al control que entramos al timer para que cuente este procedimiento
			isEnable_Keypad2(0,4000);//le dice al control que entramos al timer para que cuente este procedimiento
			//Anti_Bounce_Keypad(0xFE);
			resetDDS_repetidos(1005);
//			 LED12_SYS_NegVal(); 
	        LedController_IRQ();
	        keypad_Timer();
	        
	        
	     }//bander de timer---------------------------------------------
}//fin display TIMER IRQ----------------------------------------------------------------------------

///textProc

//cuenta para resetear el registro de los datos de deteccion
//de DDS que no se mandan o grafican porque estan repetidos
void resetDDS_repetidos(unsigned short int n){
static unsigned short int c; 	 
	if(serial.vars.Flags.bitF.DDS_ACTIVA){   
	  if(c++>n){
		  c=0;
		  serial.vars.Flags.bitF.resetDDS=TRUE;}}
	  
}//fin resetDDS_repetidos-------------------------------


/* Se eqjecuta en el loop principal del S.O. 
 *  para gestionar la habilitacion del teclado variable*/
void keypad_Timer(void){//ENTRA AQUI CADA 500microsegundos
   if(keypad.var.timerkeypad<1000)//pasaron 3 segundos=6000*0.0005seg
     	  keypad.var.timerkeypad++; //nos dice cuantos bytes tenemos antes de cierto tiempo 
   else keypad.var.c0=0;
}//keypad_Timer--------------------------------------------


//cuenta con timer si se deteiene o no, wait: reseteamos la varable estatica para que cuente el timer
//con 0: contamos desde el timer,, Get: preguntamos si ya se ha alnzao la vrible time mas que el valor maximo
unsigned char isEnable_Keypad(unsigned char cmd){
static short int time;	
const unsigned long int MAX_TIME=3550;
	switch(cmd){
		case WAIT: time=0; break;
		case GET:  if(time>(MAX_TIME-1))
			            return TRUE;
				   else return FALSE;
		           break;
		default:   if(time<MAX_TIME)
			             time++;
		           break;
	}//fin switch-------------------------
return FALSE;	
}//fin is isEnable_Keypad---------------------------------------------------------------

unsigned char isEnable_Keypad2(unsigned char cmd,unsigned long int maxtime){
static short int time;	
static unsigned long int MAX_TIME_;
	switch(cmd){
		case WAIT: time=0; MAX_TIME_=maxtime; break;
		case GET:  if(time>(MAX_TIME_-1))
			            return TRUE;
				   else return FALSE;
		           break;
		default:   if(time<MAX_TIME_)
			             time++;
		           break;
	}//fin switch-------------------------
return FALSE;	
}//fin is isEnable_Keypad---------------------------------------------------------------

	 

/* Procesador de sensibilidad or digito  cambiamos el numero en menu por digito
 * param1: digito, digito a cambiar de valor
 * param2: posicion donde se pinta el digito
 * 
 * */
void procSensxDigitoEN(unsigned char *digito,unsigned char posx,unsigned char initx){
unsigned char a[5],*p;	
      
	   switch(*digito){//  5 indica decena de miles y 1 unidad
	    				  case 1:*digito=0;p=&a[0];
	    					     cursorx=posx;
	    				         getCharsFromUINT_var(p,Deteccion.Sensibilidad);
	    					     VFDposicion_DDS(cursorx,cursory);
	    					     VFDserial_SendChar_DDS(' ');
	    					     VFDserial_SendChar_DDS(a[0]+0x30);
	    					     VFDserial_SendChar_DDS(a[1]+0x30);
	    					     VFDserial_SendChar_DDS(a[2]+0x30);
	    					     VFDserial_SendChar_DDS(a[3]+0x30);
	    					     VFDserial_SendChar_DDS(a[4]+0x30);			          
	    					     cursorx=initx;
	    					     VFDposicion_DDS(cursorx,cursory);
	    					     VFDserial_SendChar_DDS('>');
	    					     VFDserial_SendChar_DDS(0x01);
					             isEnable_Keypad(WAIT);
	    					     break;
	    				  case 0:*digito=5;
	    		                 VFDposicion_DDS(cursorx,cursory);
			                     VFDserial_SendChar_DDS(' ');
			                     cursorx=posx;
			                     VFDposicion_DDS(cursorx,cursory);
			                     VFDserial_SendChar_DDS('>');
			                     VFDserial_SendChar_DDS(0x01);
					             isEnable_Keypad(WAIT);
			                     break;        
	    				  case 5:*digito=4;p=&a[0];
	    				         cursorx=posx;
	    				         VFDposicion_DDS(cursorx,cursory);
	    				         getCharsFromUINT_var(p,Deteccion.Sensibilidad);
	    				         VFDserial_SendChar_DDS(a[0]+0x30); 
								 VFDserial_SendChar_DDS('>');
								 VFDserial_SendChar_DDS(0x01);
								 isEnable_Keypad(WAIT);
								 break;
	    				  case 4:*digito=3;p=&a[0];
								 cursorx=posx;
								 VFDposicion_DDS(cursorx,cursory);
								 getCharsFromUINT_var(p,Deteccion.Sensibilidad);
								 VFDserial_SendChar_DDS(a[0]+0x30);
								 VFDserial_SendChar_DDS(a[1]+0x30);
								 VFDserial_SendChar_DDS('>');
								 VFDserial_SendChar_DDS(0x01);
								 isEnable_Keypad(WAIT);
								 break;
	    				  case 3:*digito=2;p=&a[0];
								 cursorx=posx;
								 VFDposicion_DDS(cursorx,cursory);
								 getCharsFromUINT_var(p,Deteccion.Sensibilidad);
								 VFDserial_SendChar_DDS(a[0]+0x30);
								 VFDserial_SendChar_DDS(a[1]+0x30);
								 VFDserial_SendChar_DDS(a[2]+0x30);
								 VFDserial_SendChar_DDS('>');
								 VFDserial_SendChar_DDS(0x01);
								 isEnable_Keypad(WAIT);
								 break;	 
	    				  case 2:*digito=1;p=&a[0];
								 cursorx=posx;
								 VFDposicion_DDS(cursorx,cursory);
								 getCharsFromUINT_var(p,Deteccion.Sensibilidad);
								 VFDserial_SendChar_DDS(a[0]+0x30);
								 VFDserial_SendChar_DDS(a[1]+0x30);
								 VFDserial_SendChar_DDS(a[2]+0x30);
								 VFDserial_SendChar_DDS(a[3]+0x30);
								 VFDserial_SendChar_DDS('>');
								 VFDserial_SendChar_DDS(0x01);
								 isEnable_Keypad(WAIT);
								 break;	 	 		 
								 
	    				  default:__asm(Halt);
	    	                      break;}//debug--------

}//fin de procSensxDigito----------------------------------------------


unsigned char procSensxDigitoRT(unsigned char posx,unsigned char posy,unsigned short int *Sens){
	if((*Sens)>=32000)
		 return FALSE;
	else  ++(*Sens); 
	BarraDet_displayUINT_var(posx,posy,Sens);//displayUINT_var(POSXCFNUM,POSY2,&Sensibilidad,NONE);	
}//fin ---procSensxDigitoRT--------------------------------------------

unsigned char procSensxDigitoLF(unsigned char posx,unsigned char posy,unsigned short int *Sens){
	 if(*Sens==1)
		return FALSE;  
	 else --(*Sens); 		 
     BarraDet_displayUINT_var(posx,posy,Sens);//displayUINT_var(POSXCFNUM,POSY2,&Sensibilidad,NONE);	
return TRUE;
}//fin ---procSensxDigitoRT--------------------------------------------


void procFasexDigitoEN(unsigned char *var,unsigned char posx,unsigned char posy){
unsigned char n;
byte a[5],*p;
    
    if(cursory==POSY4)  n=8; else n=0;   
	switch(*var){//5 indica decena de miles y 1 unidad
		case 0: *var=5;
		        VFDposicion_DDS(cursorx,cursory);
				VFDserial_SendChar_DDS(' ');
				if(n==8){VFDposicion_DDS(POSXDDSNUMS+8,POSY6);
						 VFDserial_SendChar_DDS(' ');VFDserial_SendChar_DDS(' ');
						 VFDserial_SendChar_DDS(' ');VFDserial_SendChar_DDS(' ');
						 VFDserial_SendChar_DDS(' ');VFDserial_SendChar_DDS(0x01);
						 VFDserial_SendChar_DDS(0x02);}
				VFDposicion_DDS((unsigned short int)(posx+(8*(5-*var))+n),posy);
				VFDserial_SendChar_DDS('^');
				VFDserial_SendChar_DDS(0x01);
				VFDserial_SendChar_DDS(0x02);
				isEnable_Keypad(WAIT);
				break;
		case 1: VFDposicion_DDS((unsigned short int)(posx+(8*(5-*var))+n),posy);
				VFDserial_SendChar_DDS(' ');
				VFDposicion_DDS(cursorx,cursory);
				VFDserial_SendChar_DDS('>');
				VFDserial_SendChar_DDS(0x01);
				VFDserial_SendChar_DDS(0x02);
				if(n==8){p=&a[0];
				     getCharsFromUINT_var(p,Deteccion.Altura);
				     VFDposicion_DDS(POSXDDSNUMS,POSY6);
				     for(n=0;n<5;n++)
				         VFDserial_SendChar_DDS(a[n]+0x30);
				     VFDserial_SendChar_DDS(0x01);//esto es un delay
				     VFDserial_SendChar_DDS(0x02);}
				*var=0; 
				isEnable_Keypad(WAIT);
				break;
		case 2:  
		case 3:VFDposicion_DDS((unsigned short int)(posx+(8*2)+n),posy);
			   VFDserial_SendChar_DDS(' ');
			   VFDserial_SendChar_DDS(' ');
			   VFDserial_SendChar_DDS('^');
			   VFDserial_SendChar_DDS(0x01);
			   VFDserial_SendChar_DDS(0x02);
			   isEnable_Keypad(WAIT);
			   *var=1;
			   break;
		case 4:
		case 5:VFDposicion_DDS((unsigned short int)(posx+(8*(5-(*var)--))+n),posy);
			   VFDserial_SendChar_DDS(' ');
			   VFDserial_SendChar_DDS('^');
			   VFDserial_SendChar_DDS(0x01);
			   VFDserial_SendChar_DDS(0x02);
			   isEnable_Keypad(WAIT);
			   break;
		default: __asm(Halt);}
	
	
	
	
}//FIN procFasexDigitoEN-----------------------------------------

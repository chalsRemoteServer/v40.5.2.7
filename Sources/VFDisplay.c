/*
 * VFDisplay.c
 *
 *  Created on: Nov 8, 2019
 *      Author: desarrollo-1
 */




#include "VFDisplay.h"
#include "VFDmenu.h"
#include "system.h"
#include "DSP.h"
#include "VFD.h"
#include "VFDserial.h"
#include "delay.h"
#include "strings.h"
#include "maths.h"
#include "queue.h"
#include "VFDmenuControl.h"
#include "Reloj.h"
#include "Memoria.h"

typedef union{//access word: 
	unsigned  short int wordx;   //   	0xaabb   
	unsigned char byteH;      //         aa
	unsigned char byte[2];        //byte[0]=aa,byte[1]=bb
}uword2;

extern unsigned char name[NOMBRE_PRODUCTO_SIZE]; 
extern unsigned char indiceProducto; //Producto Set TestUP=1, que producto esta actualmete seleccionado
extern unsigned char ControlContrasena1,ControlContrasena2,ControlContrasena3,ControlContrasena4;
extern unsigned char BorrarContadores;
extern unsigned char cursorx,cursory; //posicion actual del cursor,
extern unsigned char GananciaAnaloga;
extern unsigned char phase,phasefrac;//phase de ajustes de deteccion la variable fraccionaria es de la misma vriable
extern struct ZoomControl zoom;
extern unsigned char ZoomDDS; //variable que guarda el Zoom actual en la DDS del producto actual
extern unsigned char PSUIOconfig; // confifura si permite o no las salidas/entradas en la tarjeta IO
extern unsigned char GananciaDriver; //ganancia de la driver en AJUSTE_DE_SISTEMA/INGENIERIA_GRACIDA/GANANCIA
extern unsigned char ReporteProductoPerm; //permitir reporte de producto 
extern unsigned char errorAnalogoPerm; // permitir error analogo
unsigned char bufferXcontexto[BUFF_X_CONTEXTO_SIZE];//se usa solo en el contexto ó pantalla Actual, mas alla nop.

extern struct _Menu_ menu;
unsigned char passControl[PASSCNTRL_SIZE];
extern unsigned char igxm5[IGXM_SIZE];//variable generica array generico
extern unsigned char Ganancia;//parametro de aplicacion desconocido por el momento.
extern unsigned char MarcarAltura;
extern unsigned char Text[NOMBRE_PRODUCTO_SIZE];//texto que regresa  del procesador de textos

extern struct _GLOBAL_CONTEXTO g,*AjustVibr,*NewProd;

extern struct _GLOBAL_CONTEXTO gxc,gxc2,gxc3,gxc4,gxc5,gxc6,gxc7,gxc8,gxc9,gxc10,gxc11;//variable global por contexto
extern struct _GLOBAL_PASO_OTRO_CONTEXTO gpoc;//global paso a otro contexto
extern struct _Detection Deteccion;
extern struct _Comando_DDS dds;

extern struct EntSyst{//variables 	de entradas de sistema 
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




//metodo que activa las banderas y variables para que cambie de contexto el
//sistema de pantalla. el cambio de pantalla lo hace el metodo=
//control operativo Menu Escape()
// overridePadre: es para cambiar el padre de donde venimos
void displayControladorPantalla(unsigned char destino){
	
	switch(destino){//++++++++++++NO MOVER EL ORDEN DE LOS CASES PORQUE AFECTARIA LAS AUTRIZACION DE DE PERMISOS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++PASO-1+++++1
		
		case AJUSTE_PARAMETRICO_DE_PRODUCTO:
							if(passControl[_AUTHOR_]==1){
								  menu.contexto.Actual=PORTAL_INICIO;
								  menu.bytes.semMenuPendiente=TRUE;
								  menu.contexto.Anterior=SUPERVISOR;
								  break;}
							  if(menu.contexto.Anterior==PORTAL_INICIO){
								   passControl[_ORIGEN_]=menu.contexto.Anterior;
								   passControl[_DESTIN_]=destino;
								   passControl[_AUTHOR_]=0;
								   menu.bytes.semMenuPendiente=TRUE;
								   menu.contexto.Anterior=SUPERVISOR;//control por password
								   menu.contexto.Actual=TEXT_PROCESSOR;
								   break;}
							  varcontexto(destino);
							  break;
		case AJUSTE_DE_SISTEMA:
							  if(passControl[_AUTHOR_]==1){
								  menu.contexto.Actual=PORTAL_INICIO;
								  menu.bytes.semMenuPendiente=TRUE;
								  menu.contexto.Anterior=SUPERVISOR;
								  break;}
							  switch(menu.contexto.Anterior){
								  case PORTAL_INICIO:
								  case MENU_INSIGHT:
										   passControl[_ORIGEN_]=menu.contexto.Anterior;
										   passControl[_DESTIN_]=destino;
										   passControl[_AUTHOR_]=0;
										   menu.bytes.semMenuPendiente=TRUE;
										   menu.contexto.Anterior=SUPERVISOR;//control por password
										   menu.contexto.Actual=TEXT_PROCESSOR;
										   break;
								  default:varcontexto(destino);break;}
							  break;
		case AJUSTE_DE_PRODUCTO:
			      switch(menu.contexto.Anterior){
			    	  case PORTAL_INICIO:
			    	  case MENU_INSIGHT:
									   passControl[_ORIGEN_]=menu.contexto.Anterior;
									   passControl[_DESTIN_]=destino;
									   passControl[_AUTHOR_]=0;
									   menu.bytes.semMenuPendiente=TRUE;//EJEcutar el S.O.
									   menu.contexto.padre=MENU_INSIGHT;
									   menu.contexto.Modificado=destino;
									   menu.contexto.Anterior=SUPERVISOR;//control por password
									   menu.contexto.Actual=TEXT_PROCESSOR;
									   break;
			    	  default:varcontexto(destino);break;}
			      break;
		case NOMBRE_PRODUCTO:
			                if(menu.contexto.Actual==AJUSTES_AVANZADOS){
			                  menu.bytes.semMenuPendiente=TRUE;
			                  menu.contexto.Anterior=NOMBRE_PRODUCTO;//control por password
			            	  menu.contexto.Actual=TEXT_PROCESSOR;}
			                varcontexto(destino);
			                break;
		case MENU_INSIGHT: displayBoxDestructor(ACTIVADO);//destruir y parar la barra de deteccion		      
		case PORTAL_INICIO: __asm(nop);                 //+++++++++++++++++++++++++++++++-----------------paso-2  ir a VFDmen
		case PANTALLA_DDS:
		case INFORMACION_DE_USUARIO:	
		case MENU_ADMINISTRATIVO:	
		case CONFIGURAR_ENTRADAS_DE_SISTEMA:
		case ID_COMUNICACIONES:
		case CONTROL_DE_FRECUENCIA:
		case MEDIDAS_DE_VOLTAJE:
		case MEDIDAS_DE_VOLTAJES:
		case AJUSTE_DE_PRODUCTOINFO:
		case TIEMPO_DE_RECHAZOINFO:
		case AJUSTE_SISTEMA_ENTRADAS:
		case ID_MAQUINA:
		case CUENTA_PRODUCTO:
		case ESTABLECER_IDIOMA:
		case CONTROL_PASSWORD:
		case RELOJ_DE_SISTEMA:
		case INGENIERIA_GRACIDA:__asm(nop);
		case INIT_MEMO_Y_REST:	
		case TIPO_DE_MAQUINA:
		case GANANCIA_:
		case FILTRO_:
		case PRUEBA:             __asm(nop);	
		case INGENIERIA_GRACIDA2:__asm(nop);
//		case AJUSTE_PARAMETRICO_DE_PRODUCTO:
		case AJUSTES_AVANZADOS:
		case FRECUENCIA_SELECT:
		case I2CMENU:
		case I2CEEPROMREAD:
		case I2CNVRAMDATA:
		case SPI_MENU:
		case PRUEBAS_SALIDAS:	
		case PRUEBAS_ENTRADAS:
//		case AjustesAvanzadosAPP:
		case PARAMETROS_DE_DETECCION:
		case DISPLAY_ADC:
		case SELECCIONAR_PRODUCTO:
		case NUEVO_PRODUCTO:
		case TEXT_PROCESSOR:
		case AJUSTE_DE_PRODUCTO_HIJO:
		case PARTE_11:
		case AJUSTE_DE_VIBRACION:
		case BORRAR_PRODUCTO:
		case COPY_PRODUCTO:
		case AJUSTE_PRODUCTO_SEL:
		
		case ESTAS_SEGURO2:
		case ESTAS_SEGURO:
		case PASTE_PRODUCTO:
		case AJUSTE_DE_PRODUCTO_MODE:
		case ENTRADA_TACO:
		case ENTRADA_FOTOCEL:
		case ENTRADA_TEST_RECHAZO:
		case ENTRADA_TEST_PRODUCTO:
		case ENTRADA_AUX1:
		case ENTRADA_AUX2:
		case AJUSTE_TIEMPO_RECHAZO:        
			
			
			                         varcontexto(destino);
		                             break;
				                   
		default:break;//debe ser un error, despues meter en controlador de errores                  
	}//fin switch destino------------------------------------------------         
}//fin de void displayControladorPantalla(unsigned char destino){----------------------------------


 void varcontexto(unsigned char destino){
	menu.bytes.semMenuPendiente=TRUE;
	menu.contexto.Anterior=menu.contexto.Actual;
	menu.contexto.Actual=destino;
 }//------------------------------------------------------------------------

 
void saveTextBuffer(unsigned char *p){
unsigned char i;
       for(i=0;i<NOMBRE_PRODUCTO_SIZE;i++)
     	  igxm5[i]=*(p)++;
}//fin saveTextBuffer-------------------------------------------------------


void Display_rotate_Ganancia_var(unsigned char posx,unsigned char posy){
//unsigned char a[]={"Alta"};
//unsigned char b[]={"Baja"};
//word n;
	pushFIFOcOP_inicioCMD();
	pushFIFOcOP_VFDposicion(posx,posy);
	if(Ganancia==ALTA){
		pushFIFOcOP_VFDserial_SendChar('A');
		pushFIFOcOP_VFDserial_SendChar('l');
		pushFIFOcOP_VFDserial_SendChar('t');
		pushFIFOcOP_VFDserial_SendChar('a');
		Ganancia=BAJA;}
	else{
		pushFIFOcOP_VFDserial_SendChar('B');
		pushFIFOcOP_VFDserial_SendChar('a');
		pushFIFOcOP_VFDserial_SendChar('j');
		pushFIFOcOP_VFDserial_SendChar('a');
	    Ganancia=ALTA;}
	pushFIFOcOP_finCMD();
}// Fin displayGanancia_var------------------------------------------------------



void Display_rotate_MarcarAltura(unsigned char posx,unsigned char posy){
         pushFIFOcOP_inicioCMD();
		   pushFIFOcOP_VFDposicion(posx,posy);
		   if(MarcarAltura=='0'){
			    pushFIFOcOP_VFDserial_SendChar('S');
     		   	pushFIFOcOP_VFDserial_SendChar('i');
		        MarcarAltura='1';}
		   else{pushFIFOcOP_VFDserial_SendChar('N');
		        pushFIFOcOP_VFDserial_SendChar('o');
		        MarcarAltura='0';}
		   pushFIFOcOP_finCMD();
}//FIN Display_rotate_MarcarAltura------------------------------------------

//funcion para parametros de  deteccion menu--------------------------
unsigned short int f(unsigned char x){
unsigned short int r;	
	switch(x){
		case 5:r=(1*8);break;
		case 4:r=(0*8);break;
		case 3:r=(2*8);break;
		case 2:r=(3*8);break;
		case 1:r=(4*8);break;
		case 0:r=(0*8);break;
		default:r=0;break;}		
return r;	
}//-----------------------------------------------------------------

//funcion para parametros de  deteccion menu--------------------------
unsigned short int f2(unsigned char x){
unsigned short int r;	
	switch(x){
		case 5:r=(1);break;
		case 4:r=(0);break;
		case 3:r=(2);break;
		case 2:r=(3);break;
		case 1:r=(4);break;
		case 0:r=(0);break;
		default:r=0;break;}		
return r;	
}//-----------------------------------------------------------------


//convertirlos en ascii}
void convert2ascii(unsigned char *array,unsigned char size){
unsigned char i,j;	
   for(i=0;i<size;i++){
	   j=*(array+i); 
  	 *(array+i)=j+0x30;}
}//fin convert2ascii------------------------------------------


void display_pushFIFO_UShortInt(unsigned short *ShortNum,unsigned char index,unsigned char op,unsigned char Row,unsigned char contxt){
unsigned char a[5]={" "};
unsigned char b[6]={" "};
unsigned char n;	
     
	  if(index>5)//index, digito a mdificar 5=decena de miles , 1=unidad
		 __asm(Halt);//debug error de software
	  operacionUShortInt(ShortNum,index,op);//opera en el digito indicado
	  configModificado(contxt);
	  getCharsFromUINT_var(&a[0],*ShortNum);
	  convert2ascii(&a[0],5);
	  a[5]='>';
	  if(contxt==PARAMETROS_DE_DETECCION)
		  n=POSXPARAMDETECT;
	  if(index<5){
		  pushFIFOcOP_VFDposicion((unsigned short)(n-8),Row); 
  		  calculoOrdenCharsUInt_wSelectChar(index,&b[0],&a[0]);//calc orden de los chars
  		  displayOrdenCharsUInt_wSelectChar(&b[0],&a[0],n,Row);}
	  else{
		  pushFIFOcOP_VFDposicion((unsigned short)(n-(8*(5-index))),Row); 
    	  pushFIFOcOP_VFDserial_SendChar(a[5-index]);}
return;	
}//fin display_pushFIFO_UShortInt-----------------------------------------------------------------------


void display_UInt_CharsSelected(unsigned short int var,unsigned char *index,unsigned char posy,unsigned char posInit){
unsigned char a[6]={" "},*p=0;	
unsigned char b[6]={" "},B,*bb;  
			p=&a[0];
			pushFIFOcOP_inicioCMD();
			getCharsFromUINT_var(p,var);
			convert2ascii(p,5);
			a[5]='>';
			B=*index;
			bb=&b[0];
			p=&a[0];
			calculoOrdenCharsUInt_wSelectChar(B,bb,p);
			displayOrdenCharsUInt_wSelectChar(bb,p,posInit,posy);
			display_cursor_main(index);
			if(*index==0)//para el control de la maquina de estados de arriba
				*index=5;
			else {B=*index;
			      B--;
			      *index=B;}
}//fin display_UInt_CharsSelected-----------------------------------------------------
			

void display_cursor_main(unsigned char *index){
unsigned char r=0;	
	switch(menu.contexto.Actual){
	  case PARAMETROS_DE_DETECCION:
		          switch(*index){
		         	  case 0:pushFIFOcOP_VFDposicion(0,cursory);
					         r+=pushFIFOcOP_VFDserial_SendChar('w');
					         r+=pushFIFOcOP_VFDserial_SendChar('e');
					         r+=pushFIFOcOP_VFDserial_SendChar('e');
					         r+=pushFIFOcOP_VFDserial_SendChar('e');
					         r+=pushFIFOcOP_VFDserial_SendChar('e');
					         r+=pushFIFOcOP_VFDserial_SendChar('e');
					         r+=pushFIFOcOP_VFDserial_SendChar('e');
					         r+=pushFIFOcOP_VFDserial_SendChar('e');	    
					         r+=pushFIFOcOP_VFDserial_SendChar(0x02);
					         r+=pushFIFOcOP_VFDserial_SendChar(0x01);
					         if(r>0)
					        	 __asm(Halt);
					         break;
		        	  case 1:pushFIFOcOP_VFDposicion(0,cursory);
		   				     pushFIFOcOP_VFDserial_SendChar('>');
			             	 break;
		        	  default:break;}
		          break;
	  default:break;}	          
}//fin display_cursor_main-------------------------------------------------------------


//B1: es el indice el numero de char a cambiar 5: decena de miles , 1=unidad
//*b: apunta a un array que  guadara el orden en se despliegan los chars y el caracter '>'
void calculoOrdenCharsUInt_wSelectChar(unsigned char B1,unsigned char *b,unsigned char *a){
           switch(B1){
				case 0:*b=5;*(b+1)=0;*(b+2)=1;*(b+3)=2;*(b+4)=3;*(b+5)=4;break;//b[0]=5;b[1]=0;b[2]=1;b[3]=2;b[4]=3;b[5]=4;break;//orden de los chars numeros y el '>'
				case 1:*b=5;*(b+1)=0;*(b+2)=1;*(b+3)=2;*(b+4)=3;*(b+5)=4;*(a+5)=' ';break;//a[5]=' ';b[0]=5;b[1]=0;b[2]=1;b[3]=2;b[4]=3;b[5]=4; break;      
				case 2:*b=0;*(b+1)=1;*(b+2)=2;*(b+3)=3;*(b+4)=5;*(b+5)=4;break;//b[0]=0;b[1]=1;b[2]=2;b[3]=3;b[4]=5;b[5]=4;break;
				case 3:*b=0;*(b+1)=1;*(b+2)=2;*(b+3)=5;*(b+4)=3;*(b+5)=4;break;//b[0]=0;b[1]=1;b[2]=2;b[3]=5;b[4]=3;b[5]=4;break;
				case 4:*b=0;*(b+1)=1;*(b+2)=5;*(b+3)=2;*(b+4)=3;*(b+5)=4;break;//b[0]=0;b[1]=1;b[2]=5;b[3]=2;b[4]=3;b[5]=4;break;
				case 5:*b=0;*(b+1)=5;*(b+2)=1;*(b+3)=2;*(b+4)=3;*(b+5)=4;break;//b[0]=0;b[1]=5;b[2]=1;b[3]=2;b[4]=3;b[5]=4;break;
				default: __asm(Halt);}
}// fin calculoOrdenCharsUInt_wSelectChar --------------------------------------------------------

void displayOrdenCharsUInt_wSelectChar(unsigned char *bb,unsigned char *aa,unsigned char posInit,unsigned char posy){
//unsigned char e;
unsigned char b[6],i;
unsigned char a[6];
            for(i=0;i<6;i++){
            	b[i]=*(bb+i);
                a[i]=*(aa+i);}
            							   
			pushFIFOcOP_VFDposicion((unsigned short int)(posInit-8),posy);
			pushFIFOcOP_VFDserial_SendChar(a[b[0]]);
			pushFIFOcOP_VFDserial_SendChar(a[b[1]]);
			pushFIFOcOP_VFDserial_SendChar(a[b[2]]);
			pushFIFOcOP_VFDserial_SendChar(a[b[3]]);
			pushFIFOcOP_VFDserial_SendChar(a[b[4]]);
			pushFIFOcOP_VFDserial_SendChar(a[b[5]]);
			pushFIFOcOP_finCMD();
return;
}//FIN  display_UInt_CharsSelected-----------------------------------------------------------------------------


unsigned char rotateSensor(unsigned char sensor){
unsigned char n;	
    n=sensor & 0x3F;//0011 1111 se obtiene el dato del sensor
	if(n=='1'){ //xxss ssss//s=sensor
		 n=sensor&0xF0;//que retorne '0'+2-high bits
		 return n;}//clean low nibble, turn off=NO  turn-on=PERMITIR
	else return(sensor|'1');//turn-on=PERMITIR
}//fin rotateSensor------------------------------------------------------------------------------------------


void displaySensorActivation(unsigned char sensor, unsigned char pos){
unsigned char v[]={">Permitir             1"};
unsigned char w[]={">No                   0"};
word n;
    VFDposicion(0,pos);
    if((sensor&0x3F)=='1')
       VFDserial_SendBlock(&v[0],sizeof(v),&n);
    else VFDserial_SendBlock(&w[0],sizeof(w),&n);
    delay1us();    
}//fin displaySensorActivation---------------------------------------------------


unsigned char rotateSensorHighNibble(unsigned char sensor){//ACTIVAR PULSOS DE RECHAZO
	if((sensor & 0xC0)==0xC0)//nnxx xxxx 1100 0000  C0h inverso 0011 1111 3F
			 return (sensor&0x3F);//clean high nibble, turn off=0  turn-on=1
	else return(sensor|0xC0);//turn-on=PERMITIR
}//fin rotateSensorHighNibble-------------------------------------------------

//obtiene los primeros 2 bits los MSB
unsigned char getHighNibbleSensor(unsigned char sensor){
	if((sensor&0xC0)==0xC0)
		return '1';
	else return '0';
}//FIN getHighNibbleSensor-------------------------------------------------------

void displayUChar(unsigned char *p,unsigned short posx,unsigned short posy){
unsigned char c3;	  	
	
	VFDposicion(posx,posy);
	delay1us();
    if(*p>'0')//Displaying the Pulses Per Second
    	c3='c';//se imprime desde la centena
	else {
	  if(*(p+1)>'0')
		  c3='d';//se imprime desde la decena
	  else c3='u';}//se imprime desde la decena
    switch(c3){
    	case 'c':VFDserial_SendChar(*(p+0));
    	case 'd':VFDserial_SendChar(*(p+1));
    	case 'u':VFDserial_SendChar(*(p+2));break;
    	default: break;} 
	
}//fin displayUChar------------------------------------------------------------------

void displayAltoSiBajoSi_Sensor(unsigned char sensor,unsigned short posy){
unsigned char w[]={">Bajo si              1"};
unsigned char v[]={">Alto si              0"};	
word n;
			 VFDposicion(0,posy);
			 if((sensor&0xC0)==0xC0) //
				 VFDserial_SendBlock(&w[0],sizeof(w),&n);
			 else VFDserial_SendBlock(&v[0],sizeof(v),&n);
}//fin displayAltoSiBajoSi_Sensor------------------------------------------------------------------

//rotamos la variable cuando se da enter del sensor variable fotoceda bloqueado
//rota la posicion del cursor de seleccion del '>'
unsigned char rotateCursorBloquedo(unsigned char sensorbloqueado,unsigned char *posicion){
	  switch(*posicion){
		  case 0:(*posicion)++;break;
		  case 1:if(sensorbloqueado<10)
			         *posicion=0;
		         else (*posicion)++; 
		         break;
		  case 2:*posicion=0;
		  default:break;		  
		  
	  }//fin switch-------------------------------
}//fin rotateCursorBloquedo---------------------------------------------------------------------

//display para sensr vbloqueado desplega los chars y el cursor seleccionado
void displayrotateCursorBloquedo(unsigned char sensorBloqueado,unsigned short posx,unsigned short posy,unsigned char *cursorSelect){
unsigned char a[3];	
	getcharFromUChar(sensorBloqueado,&a[0]);
	VFDposicion(posx-16,posy);
	switch(*cursorSelect){
		case 0:VFDserial_SendChar(' ');
			   if(sensorBloqueado>9){    
			       VFDserial_SendChar(a[1]);
			       VFDserial_SendChar(a[2]);
		           break;}
		       VFDserial_SendChar(' ');
		       VFDserial_SendChar(a[2]);
			   break;
		case 1:if(sensorBloqueado>9){
			       if(a[1]>'0')
		    	     VFDserial_SendChar(a[1]);
			       else VFDserial_SendChar(' ');
		    	   VFDserial_SendChar('>');
		    	   VFDserial_SendChar(a[2]);
		           break;}
		       VFDserial_SendChar(' ');
		       VFDserial_SendChar('>');
		       VFDserial_SendChar(a[2]);
		       break;
		case 2:VFDserial_SendChar('>');
			   VFDserial_SendChar(a[1]);
			   VFDserial_SendChar(a[2]);
               break;
		default:break;
	}//fin switch------------------------------------------
	
}//fin displayrotateCursorBloquedo-----------------------------------------------------------------------------


unsigned char displayBloqueoDigitoControl(unsigned char op,unsigned char *Bloqueado,unsigned char *digito,unsigned char posx,unsigned char posy){
 switch(op){
	 case RESTAR: 
					if(*digito>0){
							if(*Bloqueado>1){
								   if(*digito==1){
									   if(*Bloqueado>60)
										   *Bloqueado=60;
									   else
										   (*Bloqueado)--;}
								   if(*digito==2){
										if(*Bloqueado>60)
											*Bloqueado=60;
										else{
										   if(*Bloqueado<10)	
												return TRUE;
										   else (*Bloqueado)-=10;}}
								   displayrotateCursorBloquedo(*Bloqueado,posx,posy,digito);
								   return TRUE;}
							else return TRUE;
							return TRUE;}
					return FALSE;//RESTA
					break;
	 case SUMAR://INICIA SUMAR
		       if(*digito>0){
				 if(*Bloqueado>=0){
						if(*Bloqueado<60){
						   if(*digito==1)
							   (*Bloqueado)++;
						   if(*digito==2){
							   (*Bloqueado)+=10;
							   if(*Bloqueado>60)
								  (*Bloqueado)=60;}
							displayrotateCursorBloquedo(*Bloqueado,posx,posy,digito);
						   return TRUE;}
						else *Bloqueado=60; return TRUE;}
   				  return TRUE;}//FIN SUMAR
		       else return FALSE;
   				 break;}
 
return FALSE; 
}//fin BloqueoDigitoControl---------------------------------------------------------------------------------------

/*despleiga los cursores '<'  '>' en la posicion siguiente y borra la anterior
 * key es el array de las posiciones que debe ejecutar, posx y y son el cursor 
 * la posicion en donde estoy
 * "esqx": es la posixcion x donde se grafica ">X y "posiciones" es el array de las posiciones 'autorizadas' 
 * SE DEBE DECLArar posicines asi: unsigned char key[]={POSY0,POSY2,POSY4,POSY6,POSY8,0};	
 * las posiciones 	a usar y al final un cero  passControl[_AUTHOR_]
 * direccion: PARIBA|PABJO,,    posx,posy, posicion actual del cursor
 * posx0: donde se pone el cursor que no sea esquina cuando bajamos 
 * return si se modifico algo o falso si no se pudo modificar */
void displayControlCursorSubMenusKey(unsigned char direccion,unsigned char *posx, unsigned char *posy,
		unsigned char esqx,unsigned char *posiciones,unsigned char posx0 ){//&cursorx,&cursory,POSXESQ235,&key[0]){
unsigned char i,size,index;	

 //void (*pVFDposicion)(unsigned short x,unsigned short y);

        
        for(i=1;i<10;i++)   //buscar el tamaño del array
        	if(*(posiciones+i)==0){
        		 size=i;
        	     break;}
	    for(i=0;i<size;i++)//buscar en que posicion estoy
	    	if(*(posiciones+i)==*posy){ //{POSY0,POSY2,POSY4,POSY6,POSY8,0};	
	    		index=i;  //en donde estoy actualmete 
	    		break;}
		if(direccion==PARRIBA){
			if(*posy==POSY0)
				return;
			if(*(posiciones+(index-1))==POSY0){//X0 vamos a subir a la POSY0
				if(menu.contexto.Actual==PANTALLA_DDS){
					VFDposicion_DDS(*posx,*posy);
		            VFDserial_SendChar_DDS(' ');
					*posx=esqx-16;*posy=POSY0;
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS(' ');
					VFDserial_SendChar_DDS(' ');//X
					VFDserial_SendChar_DDS('>');
					VFDserial_SendChar_DDS('X');
					VFDserial_SendChar_DDS(0x02);
					VFDserial_SendChar_DDS(0x01);//sin estos dos numeros no se pinta la X
				    }
				else{
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS(' ');
					*posx=esqx;*posy=POSY0;
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS('>');
					VFDserial_SendChar_DDS('X');
					return;}}//fin X0
			else{
				if(menu.contexto.Actual==PANTALLA_DDS){//DDS para subir
					if(*posy==POSY14){
						  display_B_bold_DDS(FALSE);
						  VFDserial_SendChar_DDS(0x01);
						  VFDserial_SendChar_DDS(0x01);}//delays
					else{ 
						VFDposicion_DDS(*posx,*posy);
						VFDserial_SendChar_DDS(' ');}
					*posy=*(posiciones+(index-1));
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS('>');
					VFDserial_SendChar_DDS(0x01);//sin este no se pinta el anterior
				    return;}
				else{
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS(' ');
					*posy=*(posiciones+(index-1));
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS('>');
					return;}}}//fin parriba-----------------------
		if(direccion==PABAJO){
			if(*posy==POSY0){
				*posx=esqx;
				if(menu.contexto.Actual==PANTALLA_DDS){
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS(' ');
					VFDserial_SendChar_DDS('x');
					*posx=posx0;
					*posy=*(posiciones+(index+1));
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS('>');
					VFDserial_SendChar_DDS(0x01);//sin esto no se pinta el anterior
					return;}
				else{
					VFDposicion(*posx,*posy);
					VFDserial_SendChar_DDS(' ');
					VFDserial_SendChar_DDS('x');
					*posx=posx0;
					*posy=*(posiciones+(index+1));
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS('>');
					return;}}//fin POSY0
			else{
				if(index==(size-1))//estamos en el ultimo POSY?
		              return;
				if(menu.contexto.Actual==PANTALLA_DDS){
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS(' ');
					*posy=*(posiciones+(index+1));
					if(*posy==POSY14){
						display_B_bold_DDS(TRUE);
					    return;}
					else{
						VFDposicion_DDS(*posx,*posy);
						VFDserial_SendChar_DDS('>');	
						VFDserial_SendChar_DDS(0x01);//sin este no se pinta el anterior
						return;}}
				else{
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS(' ');
					*posy=*(posiciones+(index+1));
					VFDposicion_DDS(*posx,*posy);
					VFDserial_SendChar_DDS('>');
					return;}}}//fin pabajo
						
}//fin displayControlCursorSubMenusKey--------------------------------------------------

/* Despliega la B de borrar en el menu de DDS como Bold 
 *  disp=TRUE  means that will displat BOLD(">B") else will display Normal(" B"*/
void display_B_bold_DDS(unsigned char disp){
unsigned char  n,m,c;
	if(disp==TRUE){
		  n=TRUE;m=FALSE;c='>';}
	else {n=FALSE;m=TRUE;c=' ';}
	VFDcommand_Bold_DDS(n);
	VFDposicion_DDS(DDS_MENU_X,POSY14);
	VFDserial_SendChar_DDS(c);
	VFDserial_SendChar_DDS('B');
	VFDcommand_Bold_DDS(m);
}//fin

void display_Linea_10_DDS(unsigned char DDSaux){
unsigned char m[]="Z ";
unsigned char m0[]=">Z ";
unsigned char y[8]={  0,  2,  4,  6,  8, 10, 12, 14};
const unsigned char x5=POSXDDS;
word n;

    if(DDSaux==POSY10){
    		VFDposicion(POSXDDSCURSORMENU,y[5]);   delay1us();
    		VFDserial_SendBlock(&m0[0],sizeof(m),&n);delay1us();}
	else {	VFDposicion(x5,y[5]);   delay1us();
			VFDserial_SendBlock(&m[0],sizeof(m),&n);delay1us();}
}//findisplay_Linea_10_DDS -------------------------------

/* esplegar el numero de la varable del parmetro
 * zoom en el menu dds*/
void displayZoom(unsigned char posx,unsigned char posy){
unsigned char z[3];	
	 
	     VFDposicion_DDS(posx,posy);
	     if(zoom.Zoom>99)
	    	 zoom.Zoom=99;//rectificacion de error
	     getASCII_from_UChar(zoom.Zoom,&z[0]);
	     //VFDserial_SendChar_DDS('¡');
	     //VFDserial_SendChar_DDS('°');
	     VFDserial_SendChar_DDS(z[1]);
	     VFDserial_SendChar_DDS(z[2]);
	     VFDserial_SendChar_DDS(0x01);
	     //VFDserial_SendChar_DDS('$');
	     //VFDserial_SendChar_DDS('#');
	     	     
}//fin displayZoom




/*despliega el numero con o sin cursor del numero
 *  de Prueba_de_Sistemas 
 * numero 0.00S   tenemos maximo 9.99S  cada 99 mseg produce un segundo*/
void display_Prueba_de_Sistema(unsigned char segundo,unsigned char mseg,unsigned char *digito,
		unsigned char posx,unsigned char posy){
unsigned char a[3];	
unsigned char b[3]={0,0,0};

    if(segundo>60)
    	__asm(halt);//Debug error de software
    if(mseg>99)
    	__asm(halt);//Debug error de software
    getcharFromUChar(segundo,&a[0]);
    getcharFromUChar(mseg,&b[0]);
    VFDposicion((unsigned short)(posx-8),posy);//
	switch(*digito){
		case 0xAA: //se activo la selecion del digito para modificalo
			      configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);//Se mdifica un parametro
			      *digito=1;
			      VFDserial_SendChar(a[2]);
			      VFDserial_SendChar('.');
			      VFDserial_SendChar(b[1]);
			      VFDserial_SendChar('>');
			      VFDserial_SendChar(b[2]);
			      break;
		case 0:   configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);//Se mdifica un parametr
				  VFDserial_SendChar(' ');
				  VFDserial_SendChar(a[2]);
				  VFDserial_SendChar('.');
				  VFDserial_SendChar(b[1]);
				  VFDserial_SendChar(b[2]);
				  break;
		case 1:   configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);//Se mdifica un parametr
				  VFDserial_SendChar('>');
		          VFDserial_SendChar(a[2]);
				  VFDserial_SendChar('.');
				  VFDserial_SendChar(b[1]);
				  VFDserial_SendChar(b[2]);
	              break;
		case 2:   configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);//Se mdifica un parametro
				  VFDserial_SendChar(a[2]);
				  VFDserial_SendChar('.');
				  VFDserial_SendChar('>');
				  VFDserial_SendChar(b[1]);
				  VFDserial_SendChar(b[2]);
				  break;
		case 3:   configModificado(CONFIGURAR_ENTRADAS_DE_SISTEMA);//Se mdifica un parametro
				  VFDserial_SendChar(a[2]);
				  VFDserial_SendChar('.');
				  VFDserial_SendChar(b[1]);
				  VFDserial_SendChar('>'); 
				  VFDserial_SendChar(b[2]);
				  break;
      default:break;}//fin switch-----------------------------------------------------------------
    
}//fin display_Prueba_de_Sistema--------------------------------------------------



//es para rotar el numero de tiempo 9.99S 10.02S de 0.01S seccion
/* Menu Test entrad de producto, ROW prueba de sistema
 * digito va de derecha a izquierda  1 2 3 
 * regresa si se modifica el numero o no  */
unsigned char rotate_Prueba_de_Sistema(unsigned char direccion,unsigned char *seg,unsigned char *mseg,unsigned char digito){
unsigned char a[3]={0,0,0};   
unsigned char n,debug;

if(digito>0){	//true entramos a rotate

	if(direccion==PARRIBA){
        switch(digito){//DIGITO 1: centena 2:decena 1:unidad
        	case 1:if(*seg<9){
        		       (*seg)++;
        		        debug=1;       
        		        break;}
        	       break;
        	case 2:n=*mseg;
        		   getcharFromUChar(n,&a[0]);
        		   if(a[1]<'9'){
        		          *mseg +=10;
        		          debug=2;
        	              break;}
        	       else{
        	    	   if(*seg==9){
        	    		   debug=3;
        	    		    break;}
        	    	   else{(*seg)++;
        	    	        *mseg=*mseg-((a[1]-'0')*10);//PASA DE 45 a 05
        	    	        debug=4;
        	    	        break;}}  
        	       break;
        	case 3: if(*mseg<99){
        		        (*mseg)++;
        		        debug=5;
        		        break;}
        			else{if(*seg<9){
        				    (*seg)++;
        				    *mseg=0;
        				    debug=6;
        			        break;}
        			     else {debug=7;
        			    	   break;}}
        			debug=8;
        	        break;
        	default:break;	
        }//fin switch--PARRIBA	
		
	}//fin if parriba----------------------------------------------
	else{
	  if(direccion==PABAJO){//u->a[2], d->a[1]
		   switch(digito){//DIGITO 1: centena 2:decena 1:unidad
			   case 1:if(*mseg==0){
				            if(*seg>1){
				            	 (*seg)--;
				            	 debug=9;
				            	 break;}//se modifico rotate
				            else {debug=10;break;}}
			   	   	   else{if(*seg>0){
			   	   	             (*seg)--;
			   	   	             debug=11;
			   	   	             break;}
			   	   	   	   	else {debug=12;break;}}
				      break;
			   case 2:n=*mseg;
				      getcharFromUChar(n,&a[0]);
				      if(*seg==0){
				         if(*mseg==10){
				        	 debug=13;
				        	 break;}
				         else goto l628;}
				      else{
l628:			    	  if(a[1]=='0'){
				    		  (*seg)--;
				    	      *mseg=90;
				    	      debug=14;
				    	      break;}
				    	  else{  *mseg-=10;
				    	  	  	 debug=15; 
				    	  	  	 break;}}
				      break;
			   case 3:if(*seg>0){
				        if(*mseg==0){
				           (*seg)--;
				           *mseg=99;
				           debug=16;
				           break;}
				        else if(*mseg>1){
				        	     goto l642;}
				             else{ 
				                 (*mseg)--;
				                 debug=17;
				                 break;}}
			          else{ 
			        	  if(*mseg>1){
			        		  debug=18;
l642:		        	      (*mseg)--;}
			        	  debug=19;
			        	  break;}
			          break;
			   default:break;
		   }//fin digiro dir PABAJO
	     }}//fin else PABAJO--------------------------------------------
	if(*seg>60)
	   	__asm(halt);//Debug error de software
    if(*mseg>99)
     	__asm(halt);//Debug error de software
   return TRUE;}//digito FIN
return FALSE;//no entramos a rotate
}//fin rotate_Prueba_de_Sistema-----------------------------------------------------------------------------


/* este metodo es para rotar el label de auxiliar entrada en entradas
 * aux: el auxiliar que queremos rotar 1|2
 * *label: es el pointer a label donde guarda cual label esta guardado {1..6} 1:fallo de aire y 6:userdefined
 * */
unsigned char rotateLabelAux(unsigned char label){

      if(label>6)
    	  __asm(Halt);//debug error de software
	  switch(label){//selecciona cual es el label
		case 0:return 1;break;
		case 1:return 2;break;
		case 2:return 3;break;
		case 3:return 4;break;
		case 4:return 5;break;
		case 5:return 6;break;
		default:return 0;break;
	  }//fin switch----------------
}//fin rotateLabelAux----------------------------------------

/* obtenemos el label del auxiliar que se quiere desplegar
 * aux es el auxiliar que se quiere leer 1|2 
 * index: es 1..6 el indeice del label que se quiere obtener */
unsigned char *getLabelAux(unsigned char aux,unsigned char index){
static unsigned char       FALLO_DE_AIRE[]={"FALLO DE AIRE       "};
static unsigned char           BLOQUEADO[]={"BLOQUEADO           "};
static unsigned char  FALLO_DE_COMPUERTA[]={"FALLO DE COMPUERTA  "};
static unsigned char    RECIPIENTE_LLENO[]={"RECIPIENTE LLENO    "};
static unsigned char        BANDA_PARADA[]={"BANDA PARADA        "};
static unsigned char Fallo_de_salida_aux[]={"Fallo de salida aux."};
const unsigned char labelsize=6;
unsigned char *p[labelsize];	
//if(compareString(p[i],label))    	 
           if((aux>2)||(aux==0))            
                   __asm(Halt);//Debug error de software
	        p[0]=&FALLO_DE_AIRE[0];      //p[0]
	        p[1]=&BLOQUEADO[0];          //p[1]
	        p[2]=&FALLO_DE_COMPUERTA[0]; //p[2]
	        p[3]=&RECIPIENTE_LLENO[0];   //p[3]
	        p[4]=&BANDA_PARADA[0];       //p[4]
	        p[5]=&Fallo_de_salida_aux[0];//p[5]
	        
	        if(index<6)
	        	return p[index];
	        else
	        	return readAUXuserdefined(aux);
}//fin getLabelAux--------------------------------------------



void display_Permitir_No(unsigned char sensor,unsigned char posy){
unsigned char c[]="Permitir";
unsigned char v[]="No";
unsigned char x[8]={8,8,8,8,8, 8, 8, 8};
word n;	

    VFDposicion(x[0],posy);delay1us();
	if((sensor & 0x3F)=='1')//xx11 1111
		VFDserial_SendBlock(&c[0],sizeof(c),&n);//Permitir
	else   VFDserial_SendBlock(&v[0],sizeof(v),&n);//NO   
	VFDposicion(POSX_MENU_TACO,posy);
	if((sensor & 0x3F)=='1')
		   VFDserial_SendChar('1');
	else VFDserial_SendChar('0');

}//fin display_Permitir_No_Bajo_Alto_si-----------------------------------------------------------


void display_Alto_Bajo_Si(unsigned char sensor,unsigned char posy){
unsigned char b[]="Alto si";
unsigned char f[]="Bajo si";
unsigned char x[8]={8,8,8,8,8, 8, 8, 8};
word n;
	VFDposicion(x[0],posy);delay1us();
	if((sensor & 0xC0)==0xC0)//11xx xxxx
	       VFDserial_SendBlock(&f[0],sizeof(f),&n);//
	else   VFDserial_SendBlock(&b[0],sizeof(b),&n);//   
	VFDposicion(POSX_MENU_TACO,posy);
	if((sensor & 0xC0)==0xC0)
	     VFDserial_SendChar('1');
	else VFDserial_SendChar('0');
}//fin display_Alto_Bajo_Si-----------------------------------------------


/*  despleiga el label de la auxiliar
 *  aux 1|2 es la auxiliar a modificar
 * label: 1..6 es el label que se quiere desplegar
 * posy: es la posicion para desplegar*/
void display_LabelAux(unsigned char aux,unsigned char label,unsigned char posy){
unsigned char e[]="                      ";
unsigned char x[8]={8,8,8,8,8, 8, 8, 8};
unsigned char *p,s; 
word n;
  
      
      p=getLabelAux(aux,label);
      if(label==6){//user defined selected
    	  VFDposicion(x[0],posy);
    	  VFDserial_SendBlock(&e[0],sizeof(e),&n); } 	   
      VFDposicion(x[0],posy);
      s=length3(p);
      VFDserial_SendBlock(p,s,&n);
}//fin display_LabelAux------------------------------------------------------


/* rotamos el numero del label de rele
 * */
unsigned char rotate_auxRele(unsigned char sensorRele){
	if(sensorRele>5)
	    	  __asm(Halt);//debug error de software
		  switch(sensorRele){//selecciona cual es el label
			case 0:return 1;break;
			case 1:return 2;break;
			case 2:return 3;break;
			case 3:return 4;break;
			case 4:return 0;break;
			default:return 0;break;
		  }//fin switch----------------
}//fin rotate_auxRele--------------------------------------------------------


void display_auxRele(unsigned char sensorRele,unsigned char posy){
unsigned char x[8]={8,8,8,8,8, 8, 8, 8};	 
static unsigned char r1[]="Rele de rechazo    ";
static unsigned char r2[]="Rele de fallo      ";
static unsigned char r3[]="Rele de advertencia";
static unsigned char r4[]="Salida de repuesto ";
static unsigned char r5[]="No Rele            ";
//unsigned char  e[]="                   ";
const unsigned char rele_size=5;
unsigned char *p[rele_size],s;	
word n;
      if(sensorRele>5)
    	  __asm(Halt);
      
      p[0]=&r1[0];
      p[1]=&r2[0];
      p[2]=&r3[0];
      p[3]=&r4[0];
      p[4]=&r5[0];
            
      s=length3(p[sensorRele]);
	  VFDposicion(x[0],posy);
	  VFDserial_SendBlock(p[sensorRele],s,&n);
	  
}//---------------------------------------------------------------------------

void display_tiempo_activo_aux_label(unsigned char posy){
unsigned char m[]="Tiempo activo aux.";
unsigned char x[8]={8,8,8,8,8, 8, 8, 8};	
word n;
	VFDposicion(x[0],posy);
	VFDserial_SendBlock(&m[0],sizeof(m),&n);
}//fin display_tiempo_activo_aux_label-----------------------------------------


/*displays the digits float tre digits, one unit, two fracctional and its selector
 * 
 * */
unsigned char displayPruebaSistema_float(unsigned char dir,unsigned char *seg,unsigned char *mseg,unsigned char *digito,unsigned char posy){
unsigned char dig,pseg,pmseg;
    dig=*digito;
		if(rotate_Prueba_de_Sistema(dir,seg,mseg,dig)){
			pseg=*seg;pmseg=*mseg;
			display_Prueba_de_Sistema(pseg,pmseg,digito,POSX_MENU_TACO,posy);
		    return TRUE;}	//control del numero de prueba de sistemas 0.00S
return FALSE;
}//fin displayPruebaSistema_float---------------------------------------------------


unsigned char DisplayInsertSpaceTextProc(unsigned char indice){
	unsigned char  i=0;
	         i=indice;
//	         i=TextInsertSpace2(indice);
             DisplayNewTextProc();//despleiga el texto
             return i;
	
}// DisplayInsertSpaceTextProc--------------------------------------------------------


//ii es pa posicion actual del cursor
unsigned char DisplayRecorrerIndiceIzqTextProc(unsigned char ii,unsigned char IzqDer){
unsigned char i;
        i=ii;
        if(IzqDer==RIGHT){
			if(i>=(NOMBRE_PRODUCTO_SIZE-1))
				i=NOMBRE_PRODUCTO_SIZE-1;
			else ++i;//the value of pointer is incremented
			if(Text[i]==0)
				Text[i]=' ';
			DisplayCursorTextProc(i,ii);
            return i;}
        else{
        	if(i==1)
        		return i;
        	if(i<1)
        		__asm(Halt);
        	i--;
        	if(Text[i]==0)
				Text[i]=' ';
        	DisplayCursorTextProc(i,ii);
        	return i;}
}//fin DisplayRecorrerIndiceIzqTextProc--------------------------------------------------


//posicion actual: a la que vamos a mover,
void DisplayCursorTextProc(unsigned char pos_actual, unsigned char pos_anterior){
       if(pos_actual>pos_anterior)
    	   if(pos_actual==NOMBRE_PRODUCTO_SIZE)
    		   return;
	   VFDposicion((unsigned short)((pos_anterior-1)*8),POSY4);
       VFDserial_SendChar(' ');	
	   VFDposicion((unsigned short)((pos_actual-1)*8),POSY4);
  	   VFDserial_SendChar(CX);
	
}//FIN DisplayCursorTextProc--------------------------------------------------------


void DisplayCharTextProc(unsigned char pos,unsigned char dato){
	  VFDposicion((unsigned short)((pos-1)*8),POSY2);
      VFDserial_SendChar(dato);	
}//fin DisplayCharTextProc-----------------------------------------------

//despleiga los cuenta rechazos en el menu de portaL DE INICIO
//FE en las  y significa un char
void Display_Cuenta_Rechazos(unsigned short int rechazos){
const unsigned char SIZE1=5;	
unsigned char  numero[SIZE1],pos,i=0;     
static unsigned char  numeroDisplay[SIZE1];
//static unsigned short int rechazos0;
//static unsigned char lenght0;//longitud del numero anterior
     getCharsFromUINT_var(&numero[0],rechazos);//sacar rechazos, por char
     pos=POSX_RECHAZO_NUM;//FIFO_displayBoxChar_push(POSX_RECHAZO_NUM,POSY3);//meter a la fifo posicion
l9:  if(numero[i]==0){
	       if(i==(SIZE1-1))
	    	   return;
	       i++;
	       goto l9;}
     else{if(numero[i]!=numeroDisplay[i]){
    	        numeroDisplay[i]=numero[i];
    	        FIFO_displayBoxChar_push(pos,POSY12);
lA:    	        FIFO_displayBoxChar_push(numero[i]+0x30,0xFE);
    	        if(i==(SIZE1-1))
    	        	return;
    	        i++;
    	        goto lA;}
     	  else{pos+=8;
     	       if(i==(SIZE1-1))
     	    	    return;
     	       i++;
     	       goto l9;}}           
}// fin Display_Cuenta_Rechazos--------------------------

/*desplegar  display_Voltage_Balance(&num[0]);
 *   despplagar el numero en el menu indicdo en la posicion indicada
 * deplega el voltahe de Balance en menu Medidas de voltaje
 * POSX_VOLT_BALANCE   */
void display_Voltage_Mediciones(unsigned char *p,unsigned char voltaje,struct _GLOBAL_CONTEXTO *g,unsigned char signo){
unsigned char n=0;
unsigned short int m,k;
//static unsigned char init_dv;
//struct _GLOBAL_CONTEXTO *g;

     if(gpoc.igpoc0==0xAA)
    	 goto dv0;
     else{//initzializar variables
     	  gpoc.igpoc0=0xAA;
    	  gxc.igxc0=0;gxc2.igxc0=0;gxc3.igxc0=0;gxc4.igxc0=0;gxc5.igxc0=0;gxc6.igxc0=0;
    	  gxc.igxc1=0;gxc2.igxc1=0;gxc3.igxc1=0;gxc4.igxc1=0;gxc5.igxc1=0;gxc6.igxc1=0;
    	  gxc.igxc2=0;gxc2.igxc2=0;gxc3.igxc2=0;gxc4.igxc2=0;gxc5.igxc2=0;gxc6.igxc2=0;
    	  gxc.igxc3=0;gxc2.igxc3=0;gxc3.igxc3=0;gxc4.igxc3=0;gxc5.igxc3=0;gxc6.igxc3=0;
    	  gxc.igxc4=0;gxc2.igxc4=0;gxc3.igxc4=0;gxc4.igxc4=0;gxc5.igxc4=0;gxc6.igxc4=0;
    	  gxc.igxc5=0;gxc2.igxc5=0;gxc3.igxc5=0;gxc4.igxc5=0;gxc5.igxc5=0;gxc6.igxc5=0;
    	  
    	  gxc7.igxc0=0;gxc8.igxc0=0;gxc9.igxc0=0;gxc10.igxc0=0;gxc11.igxc0=0;
    	  gxc7.igxc1=0;gxc8.igxc1=0;gxc9.igxc1=0;gxc10.igxc1=0;gxc11.igxc1=0;
    	  gxc7.igxc2=0;gxc8.igxc2=0;gxc9.igxc2=0;gxc10.igxc2=0;gxc11.igxc2=0;
    	  gxc7.igxc3=0;gxc8.igxc3=0;gxc9.igxc3=0;gxc10.igxc3=0;gxc11.igxc3=0;
    	  gxc7.igxc4=0;gxc8.igxc4=0;gxc9.igxc4=0;gxc10.igxc4=0;gxc11.igxc4=0;
    	  gxc7.igxc5=0;gxc8.igxc5=0;gxc9.igxc5=0;gxc10.igxc5=0;gxc11.igxc5=0;}
    	  
dv0:     n=0;
	     if((g->igxc5)==0){
	    	   n=1;
	    	   g->igxc5=0xAA;}
	     if((voltaje>=RAW_X)&&(voltaje<=FILTRADO_Y)){
	     	    		   n=6;//para el signo
	     	    		   goto dv1;}
	     if(signo!=g->igxc6){
	    	 if((voltaje>=RAW_X)&&(voltaje<=FILTRADO_Y))
	    		   n=6;//para el signo
	    	 else
			   n=1;
	    	 goto dv1;} 
		 if(*p!=g->igxc0){
		       n=1;goto dv1;}
		 if(*(p+1)!=g->igxc1){
               n=2; goto dv1;}		       
		 if(*(p+2)!=g->igxc2){
		       n=3; goto dv1;}		       
		 if(*(p+3)!=g->igxc3){
	           n=4; goto dv1;}		       
		 return;		 		 
		 
dv1: g->igxc0=*p;
     g->igxc1=*(p+1);
     g->igxc2=*(p+2);
     g->igxc3=*(p+3);
     g->igxc4=*(p+4);
     g->igxc6=signo;
     if(menu.contexto.Actual==MEDIDAS_DE_VOLTAJE){
		 switch(n){
		   case 1:VFDposicion(POSX_VOLT_BALANCE,voltaje); break;
		   case 2:VFDposicion(POSX_VOLT_BALANCE+8,voltaje);break;
		   case 3:VFDposicion(POSX_VOLT_BALANCE+8+8,voltaje);break;
		   case 4:VFDposicion(POSX_VOLT_BALANCE+8+8+8,voltaje);break;
		   case 5:VFDposicion(POSX_VOLT_BALANCE+8+8+8+8,voltaje);break;
		   default:return;break;
		 }}//fin switch-1
     else{//Contexto Actual= MEDIDAS_DE_VOLTAJES
    	 k=voltaje;
    	 switch(voltaje){
    		 case POSY2:
    		 case POSY4:m=POSX_VOLT_BALANCE;break;
    		 case POSY6:m=POSX_VOLTS_X;break; //canal-X señal
    		 case POSY8:m=POSX_VOLTS_Y;voltaje=POSY6;break;//canal-Y señal
    		 case RAW_X:m=POSX_VOLTS_X;voltaje=POSY8;n=6;break;       
    		 case RAW_Y:m=POSX_VOLTS_Y;voltaje=POSY8;n=6;break;       
    		 case CENTRADO_X:m=POSX_VOLTS_X;voltaje=POSY10;n=6;break;  
    		 case CENTRADO_Y:m=POSX_VOLTS_Y;voltaje=POSY10;n=6;break;  
    		 case ROT_X:m=POSX_VOLTS_X;voltaje=POSY12;n=6;break;       
    		 case ROT_Y:m=POSX_VOLTS_Y;voltaje=POSY12;n=6;break;       
    		 default:break;}
    	 
    	 switch(n){
    	   case 6:VFDposicion(m-8,voltaje);break;	 
		   case 1:VFDposicion(m,voltaje); break;
		   case 2:VFDposicion(m+8,voltaje);break;
		   case 3:VFDposicion(m+8+8,voltaje);break;
		   case 4:VFDposicion(m+8+8+8,voltaje);break;
		   case 5:VFDposicion(m+8+8+8+8,voltaje);break; 
		   default:return;break;
		 }}//fin switch-1
     //if(n<6)//el algoritmo procesamiento con signo no esta hecho para procesar esto: -17321, son 5 digitos y signo
     if((k>=RAW_X)&&(k<=FILTRADO_Y))
    	 signo=Procesamiento_con_Signo2(p,signo);
     else 
    	 if(signo=='-')
    	    Procesamiento_con_Signo(p);//re estructura los char para meter el signo
     switch(n){
    	 case 6:VFDserial_SendChar(signo);
	     case 1:VFDserial_SendChar(*p);
	     case 2:VFDserial_SendChar(*(p+1));
	     case 3:VFDserial_SendChar(*(p+2));
	     case 4:VFDserial_SendChar(*(p+3));
	     case 5:VFDserial_SendChar(*(p+4));
	            if(n==6){
	              VFDserial_SendChar(' ');}
	            break;
	     default:return;break;
       }//fin switch-2;
	 //}//fin de contexto MEDIDAS DE VOLTAJE
}//fin de display_Voltage_Balance------------------------------------

//despliega la Sensibilidad y la Altura  en la posicion deseada
void displayUINT_var(unsigned short int posx,unsigned short int posy,unsigned short int *pn,unsigned char operacion){
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
	       display5UChars(posx,posy,p);
}// fin display sensibilidad variable-----------------------------------------------
	

	


void displayPSU_IO_var(void){
	unsigned char a[]="Permitir";
	unsigned char b[]="   No   ";
	word n;	
	 if(menu.contexto.Actual==INGENIERIA_GRACIDA2){
	    VFDposicion(168,POSY4); /*se cambio de 80 a 168*/
	    if(PSUIOconfig==PERMITIR)
	    	VFDserial_SendBlock(&a[0],sizeof(a),&n);
	    else VFDserial_SendBlock(&b[0],sizeof(b),&n);
	 }//IF ingenieria gracida
}// display POEWR SUPPLY UNIT INPUT OUTPUT  variable----------------------------------



void displayErrorAnalogoVar(void){
	unsigned char a[]="Permitir";
    unsigned char b[]="   No   ";
    word n;			
	 if(menu.contexto.Actual==INGENIERIA_GRACIDA2){
	    VFDposicion(168,POSY6);
		if(errorAnalogoPerm==PERMITIR)
		    	VFDserial_SendBlock(&a[0],sizeof(a),&n);
		    else VFDserial_SendBlock(&b[0],sizeof(b),&n);
		 }//IF ingenieria gracida
	
}// fin display de error Analogo variable -------------------------------------------



void displayReporteProductoVar(void){
				
   if(menu.contexto.Actual==INGENIERIA_GRACIDA2){
	  VFDposicion(176,POSY10);
	  VFDserial_SendChar(ReporteProductoPerm);
			 }//IF ingenieria gracida	
}//fin display de variable del reporte del producto---------------------------------



void displayBorrarContadores_var(void){
	 unsigned char a[]="Reset";
	 unsigned char b[]="     ";
     word n;
     if(menu.contexto.Actual==AJUSTE_PARAMETRICO_DE_PRODUCTO){
    	 VFDposicion(168,POSY10); 
    	 if(BorrarContadores==APAGAR){BorrarContadores=PERMITIR;
	            VFDserial_SendBlock(&a[0],sizeof(a),&n);delay1us();}
    	 else  {BorrarContadores=APAGAR;
    		    VFDserial_SendBlock(&b[0],sizeof(b),&n);}
        }
	 
}// displayborrar contado res cariable-----------------------------------------------



void display_pushFIFOcOP_BorrarContadores_var(void){
unsigned char a[]="Reset";
unsigned char b[]="     ";
word n;
        
	     if(menu.contexto.Actual==AJUSTE_PARAMETRICO_DE_PRODUCTO){
	    	 pushFIFOcOP_inicioCMD();
	    	 pushFIFOcOP_VFDposicion(168,POSY10); 
	    	 if(BorrarContadores==APAGAR){BorrarContadores=PERMITIR;
	    	        pushFIFOcOP_VFDserial_SendBlock(&a[0],sizeof(a),&n);/*delay1us();*/}
	    	 else  {BorrarContadores=APAGAR;
	    	        pushFIFOcOP_VFDserial_SendBlock(&b[0],sizeof(b),&n);}
	    	pushFIFOcOP_finCMD(); 
	        } 
}// displayborrar contado res cariable-----------------------------------------------



void displayCountingBIOS_(unsigned char subsystem){
unsigned char c=0;	 
unsigned short int x,y;

       switch(subsystem){
          case _EEPROM_:  x=EEPROM_NUM_X;
                          y=EEPROM_NUM_Y;
    	                 break;
          case _NVRAM_:  x=NVRAM_NUM_X;
                         y=NVRAM_NUM_Y;
                         break;	  
/*debug*/ default:break; //error si entra aqui
   }//fin switch
	do{
	    VFDposicion(x,y);
	    VFDserial_SendChar(c+++0x30);
    	delay_ms(26);//rapides conqe cambian los numeros
	}while(c<10);
}//fin display el conteo en la bios de la eeprom


void displayBIOS_MEMO_test(unsigned char status,unsigned char memo){
	word n;
	unsigned char  m0[]="Ok";
	unsigned char  m1[]="Fail";
			   
	if(memo==_EEPROM_)
		VFDposicion(EEPROM_NUM_X,EEPROM_NUM_Y);
	else
		VFDposicion(NVRAM_NUM_X,NVRAM_NUM_Y);
	if(status)
		VFDserial_SendBlock(&m0[0],sizeof(m0),&n);
    else
		VFDserial_SendBlock(&m1[0],sizeof(m1),&n);
		
	delay1us();   ////no quitar por los sendblocks
}//fin desplegar en la BIOS el estatus de la prueba de la BIOS


void displayNuevoProductoTitle(void){
	unsigned char  m0[]="Nuevo Producto          x";                                    	      
	unsigned char x[9]={ 41,8,8,8,8, 8, 8, 8};
	unsigned char y[9]={  0,2,4,6,8,10,12,14};
	word n;
	
	VFDposicion(41,0); delay1us();
	VFDserial_SendBlock(&m0[0],sizeof(m0),&n);delay1us();
}//fin display Nuevo Producto title--------------------



//despliega el nuevo producto con cursor en un destina row especifica
void displayNuevoProductoXY(unsigned short int y,void (*func)()){
     	
		    VFDclrscr();	  
		    fillScreenNames();
		    func();//displayNuevoProductoTitle();
		    if(y==POSY0){
		    	 cursorx=POSXESQUINA;cursory=POSY0;
		    	 VFDposicion(cursorx,cursory);
				 VFDserial_SendChar('>');	
			     VFDserial_SendChar('X'); }
		    else{VFDposicion(POSXSELPRODCURSORX,y);
				 VFDserial_SendChar('>');}	
																			
}// fin display  seleccionar Producto---------------------------------		-------------------------------


void displayNuevoProducto(void){
	  NewProd=&g;
	  DisplayNuevoProducto(&displayNuevoProductoTitle); 
}// fin display  seleccionar Producto---------------------------------		-------------------------------



void displayNumbers(unsigned char n){
unsigned char i,ypos;
unsigned char y;//numero a desplegar, numero de indice de producto
unsigned char a[3],*p;
     p=&a[0];
     for(i=1,ypos=2,y=n;i<=7;i++,ypos+=2,y++){
     	VFDposicion(POSX0,ypos);delay_ms(1); 
        getUChar2Chars(p,y,YES);
        if(*p!='0')
        	VFDserial_SendChar(*p);
        if(*(p+1)!='0')
        	VFDserial_SendChar(*(p+1));
        VFDserial_SendChar(*(p+2));}
}//fin de desplegar los numeros de lista que se despliegan en el menu sel product



void  displayEntradadeSistemas(unsigned char var){//desplega cero y/o un en el permitir de entrada                                                                                                                                                                                       tacho   	
char c;
	switch(var){
      case _TACOMETRO_:VFDposicion(POSXENTSYS,POSY4);c=EntradasSistemaReg.taco; break; 
      case _FOTOCELDA_:VFDposicion(POSXENTSYS,POSY6);c=EntradasSistemaReg.fotocel; break;
      case _RECHAZO_:  VFDposicion(POSXENTSYS,POSY8);c=EntradasSistemaReg.rechazo; break;  
      case _PRODUCTO_: VFDposicion(POSXENTSYS,POSY10);c=EntradasSistemaReg.producto; break;
      case _AUX1_:     VFDposicion(POSXENTSYS,POSY12);c=EntradasSistemaReg.aux1; break;
      case _AUX2_:     VFDposicion(POSXENTSYS,POSY14);c=EntradasSistemaReg.aux2; break;
      default:break;
     }// fin switch---------------
     VFDserial_SendChar(c);//escribir caracter nuevo 0 u 1
}//fin display de entradas de tacometro--------------------------------------------------------------------


void displayZoomDDS_var(unsigned char operacion){
unsigned char a[3];
unsigned char *p; //para los valores ascii del numero entero
   	         p=&a[0];
	         if(operacion==SUMAR){
  	            if(++zoom.Zoom>99)
	                 zoom.Zoom=0x01;}
     	         else{
	                if(operacion==RESTAR){
     	                 if(zoom.Zoom==1)
	                     zoom.Zoom=99;
    	                 else
	                     zoom.Zoom--;}}
    	         getUChar2Chars(p,zoom.Zoom,YES);//obtiene los tres char de un numero char unsigned
	             //posx=digito2posicion(posx);
    	      VFDposicion(POSXDDSNUMS,POSY10);
	          VFDserial_SendChar(a[1]);
    	      VFDserial_SendChar(a[2]);
    	      Zoom_init();//ESTE Cambia todas las variables para que se grafiquen ene este nuevo zoom
}// fin de display de la variable ZZoom en el DDS-------------------------------------------------------------------------
    



/*displayZoomDDS_varxDigito
* Grafica en Display por Digito, el digito es la variable digito,   >Z digito1,digito2
*                                                                   >Z 30          3 seria el diito1
* cambia con las flechas UP,Dn, seleccion del digito con RT,LF                       
*  posx */
unsigned short int displayZoomDDS_varxDigito(unsigned short int posx,unsigned char operacion){
unsigned char a[3],*p;
unsigned short int digito;
   	       p=&a[0];                         
	       getUChar2Chars(p,ZoomDDS,NONE);    
  	       //if (digito==0) 
           //   return (unsigned short int)(-1); //error de programa no debe ser cero nunca
   	       //if(digito>2)
	          // return (unsigned short int)(-1);
    	  if(posx==POSXDSSDIGITO1)
	           digito=2;
    	  else if(posx==POSXDSSDIGITO2)
	               digito=3;                  
    	            else return (unsigned short int)(-1);
	      // posx=digito2posicion(digito);//convertir el digito1 y el digito2 a posicion Real en VFD  
    	 if(operacion==SUMAR){                // 1,2,3,4,5    //posiciones de DDS
	         if(++a[digito-1]>9)               // 0,1,2,3,4         //POSICIONES EN ARRAY
    	            a[digito-1]=0;
	               ZoomDDS=a[1]*10+a[2];} //convertir el numero a[] en dds
    	 if(operacion==RESTAR){
             if(a[digito-1]==0)
    	           a[digito-1]=9;
	         else
    	           a[digito-1]--;
	        ZoomDDS=a[1]*10+a[2];} //convertir el numero a[] en dds 
    	        
	     VFDposicion(posx,POSY10); 
    	 VFDserial_SendChar(a[digito-1]+0x30);//solo desplegamos un digito
    	 //Zoom_init();//ESTE Cambia todas las variables para que se grafiquen ene este nuevo zoom
    	 
return 0;        
}// fin de la funcion especial de sumar o restar un digito individual en pantalla a una var UINT 
	


void display_pushFIFOcOP_Phase_var(unsigned short int posx,unsigned short int posy){//despliega la variable de phase en la posicion
unsigned char a[3];
unsigned char *p; //para los valores ascii del numero entero
//uword2 v;
        p=&a[0];
	    getUChar2Chars(p,phase,YES);//obtiene los tres char de un numero char unsigned
        if(phasefrac>9)
	          phasefrac=0;
       //if((posx>0)&&(posx<6))
	       // posx=digito2posicion(posx); //convierte numero de orden de digito en coordenadasx del display 
       posx=POSXAJUSPROD;
       VFDposicion_DDS(posx,posy);
       VFDserial_SendChar_DDS(a[0]);
       VFDserial_SendChar_DDS(a[1]);
       VFDserial_SendChar_DDS(a[2]);
       VFDserial_SendChar_DDS('.');
       VFDserial_SendChar_DDS(phasefrac+0x30);	   
     
}//display phase var---------------------------------------------------------------------------------------
	

void display_Phase_DDS(unsigned char posx,unsigned char posy,unsigned char *p){
	  VFDposicion_DDS(posx,posy);
	  VFDserial_SendChar_DDS(*p);
	  VFDserial_SendChar_DDS(*(p+1));
	  VFDserial_SendChar_DDS(*(p+2));
	  VFDserial_SendChar_DDS(*(p+3));
	  VFDserial_SendChar_DDS(*(p+4));
	  
}//fin display_Phase_DDS------------------------------------------------------------


void displayPhase_var(unsigned short int posx,unsigned short int posy){//despliega la variable de phase en la posicion
unsigned char a[3];
unsigned char *p; //para los valores ascii del numero entero
        p=&a[0];
	    getUChar2Chars(p,phase,YES);//obtiene los tres char de un numero char unsigned
        if(phasefrac>9)
	          phasefrac=0;
    	//debug    if(posx==0)
	        //controlador de errores aqui
       if((posx>0)&&(posx<6))
	        posx=digito2posicion(posx); //convierte numero de orden de digito en coordenadasx del display 
       VFDposicion(posx,posy);
       VFDserial_SendChar(a[0]);
       VFDserial_SendChar(a[1]);
       VFDserial_SendChar(a[2]);
	   VFDserial_SendChar('.');
       VFDserial_SendChar(phasefrac+0x30);	                    
}//display phase var-----------------------------------------------------------------------------
	



//SOLO SE USA PARA MENU CONTROL DE CONTRASEÑAS
void displayControlContrasenaVal(unsigned char varnum){
	    unsigned char r[]="Permitir";
	    unsigned char z[]="No      ";//ControlContrasena1
	    word n;
	    
	 switch(varnum){
	    case 1: VFDposicion(POSX9CNTRLPASSW+8,POSY6);
	    	    if(ControlContrasena1==NO) 
	    	           VFDserial_SendBlock(&z[0],sizeof(z),&n);
	    	    else   VFDserial_SendBlock(&r[0],sizeof(r),&n);
	    	    break;
	    case 2: VFDposicion(POSX9CNTRLPASSW+8,POSY8);
	    	    if(ControlContrasena2==NO) 
	                   VFDserial_SendBlock(&z[0],sizeof(z),&n);
	            else   VFDserial_SendBlock(&r[0],sizeof(r),&n);
	    	    break;
	    case 3: VFDposicion(POSX9CNTRLPASSW+8,POSY10);
	    	    if(ControlContrasena3==NO) 
	                  VFDserial_SendBlock(&z[0],sizeof(z),&n);
	            else   VFDserial_SendBlock(&r[0],sizeof(r),&n);
	    	    break;
	    case 4: VFDposicion(POSX9CNTRLPASSW+8,POSY12);
	    	    if(ControlContrasena4==NO) 
	                   VFDserial_SendBlock(&z[0],sizeof(z),&n);
	            else   VFDserial_SendBlock(&r[0],sizeof(r),&n);
	    	    break;
	    default:break;
	   }// fin switch cc
}// fin display control contrasena value-----------------------------



unsigned char displayTime(unsigned char t){//despliega los numeros del reloj que le pidan por renglon en cofiguracion del reloj menu
	unsigned char n,cent=0,dec=0,uni=1,i;//valores de inicio importantes
	switch(t){
	   case _YEAR_: VFDposicion(POSXNUMRELOJ,POSY2);
		           n=getYear();i=n;//por que mas uno? no me acuerdo, ahÂ¡Â¡ya, es por el whil
	               while(--n>0){if(++uni>9) {uni=0;
	                             if(++dec>9){dec=0;
	                             if(++cent>2){cent=0;}}}}
	                VFDserial_SendChar('2');
	                VFDserial_SendChar(cent+0x30);
	                VFDserial_SendChar(dec+0x30);
	                VFDserial_SendChar(uni+0x30);
	                break;
	   case _MES_: VFDposicion(POSXNUMRELOJ,POSY4);
		           n=getMonth();i=n;
		           if(n==0) n=1;
                   while(--n>0){if(++uni>9){uni=0;
                                 if(++dec>12){dec=0;}}}
                   VFDserial_SendChar(dec+0x30);		
                   VFDserial_SendChar(uni+0x30);
	                break;
	   case _DIA_: VFDposicion(POSXNUMRELOJ,POSY6);
		           n=getDate();i=n;//
		           if(n==0) n=1;
		           while(--n>0){if(++uni>9){uni=0;
                                   if(++dec>31){dec=0;}}}
                   VFDserial_SendChar(dec+0x30);
                   VFDserial_SendChar(uni+0x30);
                    break; 
	   case _HORA_: VFDposicion(POSXNUMRELOJ,POSY8);
		            n=gethora();i=n;//
		            if(n>24) n=0;
		            if(n!=0){
						while(--n>0){if(++uni>9){uni=0;
						   if(++dec>24){dec=0;}}}}
		            else uni=0;
                    VFDserial_SendChar(dec+0x30);
                    VFDserial_SendChar(uni+0x30);
                     break;
	   case _MIN_	: VFDposicion(POSXNUMRELOJ,POSY10);
		           n=getminutos();i=n;
		           if(n!=0){
					   while(--n>0){if(++uni>9){uni=0;
					   if(++dec>5){dec=0;}}}}
		           else uni=0;
                   VFDserial_SendChar(dec+0x30);
                   VFDserial_SendChar(uni+0x30);
	               break;
	   case _SEG_:VFDposicion(POSXNUMRELOJ,POSY12);
	               n=getsegundos();i=n;//del reloj sacamos los segundos
	               if(n!=0){
					   while(--n>0){if(++uni>9){uni=0;
						 if(++dec>5){dec=0;}}}}
	               else uni=0;
	               VFDserial_SendChar(dec+0x30);
	               VFDserial_SendChar(uni+0x30);
		           break;
	 }//fin switch t
return(i);	
}//fin display time on Reloj del sistema




void Display_Ajuste_Tiempo_Rechazo(void){
unsigned char f[]=">X";
unsigned char a[]="Tiempo de Rechazo";
unsigned char b[]="Tiempo de Duracion";
word n;
       VFDclrscr();
       VFDposicion(POSXESQ235,POSY0);
       VFDserial_SendBlock(&f[0],sizeof(f),&n);
       cursorx=8;cursory=POSY12;
       VFDposicion(cursorx,cursory++);
	   VFDserial_SendBlock(&a[0],sizeof(a),&n);
	   VFDposicion(cursorx,++cursory);
	   VFDserial_SendBlock(&b[0],sizeof(b),&n);
	   //if(Deteccion.Rechazo_mS>99)
		 //  Deteccion.Rechazo_mS=0;
//	   if(Deteccion.duracion_mS>99)
//		   Deteccion.duracion_mS=0;
//	   getASCII_from_UChar(Deteccion.Rechazo_Seg,&a[0]);
	   //getASCII_from_UChar(Deteccion.Rechazo_mS,&b[0]);
	   VFDposicion(POSXNUM,POSY12);
	   VFDserial_SendBlock(&a[0],3,&n);
	   VFDserial_SendChar('.');
	   VFDserial_SendBlock(&b[0],2,&n);
	   VFDserial_SendChar('S');
//	   getASCII_from_UChar(Deteccion.duracion_Seg,&a[0]);
//	   getASCII_from_UChar(Deteccion.duracion_mS,&b[0]);
	   VFDposicion(POSXNUM,POSY14);
	   VFDserial_SendBlock(&a[0],3,&n);
	   VFDserial_SendChar('.');
	   VFDserial_SendBlock(&b[0],2,&n);
	   VFDserial_SendChar('S');
	   Deteccion.CuadroMadreReady=NUMEROS;//en este menu solo desplegamos numeros pero usando el Sistema operativo
	   cursory=POSY0;
}//fin Display_Ajuste_Tiempo_Rechazo---------------------------------------------




void displayGananciaValues(unsigned char tarjeta){
	unsigned char a[]="Alta     ";
	unsigned char b[]="Media    ";
	unsigned char c[]="Baja     ";
	unsigned char d[]="Ajustable";
	word n;	
	 if(menu.contexto.Actual==GANANCIA_){
		if(tarjeta==DRIVER){ 
			 VFDposicion(POSX177,POSY2);
			 switch(GananciaDriver){
			   case ALTA:VFDserial_SendBlock(&a[0],sizeof(a),&n);break;
			   case AJUSTABLE:VFDserial_SendBlock(&d[0],sizeof(d),&n);break;
			   case BAJA:VFDserial_SendBlock(&c[0],sizeof(c),&n);break;
			   default:GananciaDriver=AJUSTABLE;
			           VFDserial_SendBlock(&d[0],sizeof(d),&n); 
			           break;}     }
		else if(tarjeta==ANALOGA){
			    VFDposicion(POSX177,POSY4);
				switch(GananciaAnaloga){
				  case ALTA:VFDserial_SendBlock(&a[0],sizeof(a),&n);break;
				  case MEDIA:VFDserial_SendBlock(&b[0],sizeof(b),&n);break;
				  case BAJA:VFDserial_SendBlock(&c[0],sizeof(c),&n);break;
				  default:GananciaAnaloga=MEDIA;
				          VFDserial_SendBlock(&b[0],sizeof(b),&n);
					      break;}  }
	 }//fin  IF contexto actual
}// fin display ganancia Values-------------------------------------------------------



/*  displayPhase_varxDigito 
	 *  posx y posy son las posiciones de despliegue de la variable
 *  operacion: sumar restar o NONE: solo despliegue de un digito
	 *  digitopos: que digito vamos a sumar restar y/o desplegar en la posicion xy 
 *   000.0   posicion   
	 *   12345    <--posiciones,  la posicion 4&5 es la misma*/
char displayPhase_varxDigito(unsigned short int posx, unsigned short int posy,char operacion,unsigned short int digitopos){
unsigned char a[3],*p;  //phase,phasefrac
	
	 if(digitopos==0)
    return 1; //error de programa
 if(digitopos>5)
    return 1; //error de programa 
 posx=digito2posicion(posx);
 VFDposicion(posx,posy);
 if((digitopos==1)||(digitopos==2)||(digitopos==3)){
			p=&a[0];
	 getUChar2Chars(p,phase,NEL);
	 if(operacion==SUMAR){
		if(++a[digitopos-1]>9)
			  a[digitopos-1]=0;
		if((digitopos==1)&&(a[digitopos-1]>1))
			  a[digitopos-1]=0;
		phase=a[0]*100+a[1]*10+a[2];}
	  if(operacion==RESTAR){
			if(a[digitopos-1]==0){
				 if(digitopos==1)
					   a[digitopos-1]=1;
				 else
					   a[digitopos-1]=9;}
			else --a[digitopos-1];
			phase=a[0]*100+a[1]*10+a[2];} 
	  VFDserial_SendChar(a[digitopos-1]+0x30);//solo desplegamos un digito
			   }//fin if de OR's-----------------------------------------------------------------
   if((digitopos==4)||(digitopos==5)){
	        if(operacion==SUMAR)
            if(++phasefrac>9)
	                  phasefrac=0;
            if(operacion==RESTAR){
	             if(phasefrac==0)
                     phasefrac=9;
	             else phasefrac--;}
            VFDserial_SendChar(phasefrac+0x30);
	      }//fin if
     
	return 0;     
}//fin display variable phase por digito
	

void displaySeleccioniProd_lugarOcupado(void(*func)(void)){
unsigned char j;
unsigned long int i;
	#if BUFF_X_CONTEXTO_SIZE>255
	  #error "El numero j no es compatible con 8 bit de char type"
	#endif
				if(indiceProducto>5){
					LimitesMenuPROD(indiceProducto-4,ESCRIBIR);  //ver que prducto esta seleccionado para asignar lo limites superiores
					cursorx=POSXSELPRODCURSORX;cursory=POSY10;}
				else{ LimitesMenuPROD(1,ESCRIBIR);
					  cursorx=POSXSELPRODCURSORX;cursory=indiceProducto+indiceProducto;}
			    fillScreenNames();
			    func();//displayBorrarProductoTitle();
				VFDposicion(cursorx,cursory);
			    VFDserial_SendChar('>');	// bufferXcontexto[BUFF_X_CONTEXTO_SIZE]			
			    for(i=ESPACIO_D_ADD,j=1;i<(PRODUCTO_ULTIMO_ADD+1);i+=PROD_VAR_SIZE,j++)//llenamos base de datos de todos los productos que tenemos registrados, ponemos solo el INIT_PROD
			    	bufferXcontexto[j]=readEEPROMbyte(i); //todos los iProduct guardados, y en su indice 
}//displaySeleccioniProd_lugarOcupado----------------------------------------

//modifica el valor, RETURn si se puede o no, SET the var that save the superior limit of the product index
unsigned char setLimitesMenuPROD(unsigned char mode){

	if(mode==UP_SCREEN){
       		if(LimitesMenuPROD(0,LEER)==1)
       			return 0;
       		else LimitesMenuPROD(0,RESTAR);}       			
	else{  if(LimitesMenuPROD(0,LEER)>(CANTIDAD_PRODUCTOS-6))
		         return 0;
	        else LimitesMenuPROD(0,SUMAR);}
return 0;
}// FIN de set limites de seleccion de producto-------------------------------------------------


void displayAnguloVibracion(unsigned char Ang,unsigned char frac,unsigned char veces){
const unsigned short int ANG_POSX=120,ANG_POSY=12;//posicion del angulo de vibracion
const unsigned char CENT=0,DEC=1,UNIDAD=2;
const unsigned short int VECES_POSX=120, VECES_POSY=8;//posicion inicial de los numeros de veces
unsigned char num[3],i=0,j=5;
static unsigned char Ang0,frac0,veces0;	
static unsigned short int control_de_Rapidez; //controla el tiempo de desplay numeros porque se llena la FIFO

    if(control_de_Rapidez++<1)
    	return;
    control_de_Rapidez=0;
    if(Ang0!=Ang)
    	goto d17;
    if(frac0!=frac)
    	goto d17;
    if(veces0!=veces)
        goto d17;
    return;
d17:Ang0=Ang;frac0=frac;veces0=veces;
    VFDposicion_DDS(ANG_POSX,ANG_POSY);//POSICION del angulo de vibracion
    getUChar2Chars(&num[0],Ang,YES); //es angulo, obtener los chars del numero
    if(num[CENT]>'0'){i=0xAA;j--;//la centena no es cero,un digito a desplegar
    	 VFDserial_SendChar_DDS(num[CENT]);}
    if(num[DEC]>'0'){j--;
       	 VFDserial_SendChar_DDS(num[DEC]);}
    else{if(i==0xAA){j--;
    	    VFDserial_SendChar_DDS('0');}}
    VFDserial_SendChar_DDS(num[UNIDAD]);j--;
    VFDserial_SendChar_DDS('.');j--;
    if(frac>9){
    	i=0;
    	__asm(Halt);}//debug Error de ingeneiria de Software
    VFDserial_SendChar_DDS(frac+0x30);j--;
    if(j>0){
    	VFDserial_SendChar_DDS(' ');
        if(j>1)
        	VFDserial_SendChar_DDS(' ');}
    j=2;
    VFDposicion_DDS(VECES_POSX,VECES_POSY);i=0;
    getUChar2Chars(&num[0],veces,YES);//estraemos el numero de veces en chars
    if(num[DEC]>'0'){j--;
      	 VFDserial_SendChar_DDS(num[DEC]);}
	VFDserial_SendChar_DDS(num[UNIDAD]);j--;
	if(j>0)
		VFDserial_SendChar_DDS(' ');
    
}//fin display angulo de vibracion---------------------------------------------

void displayAnguloCompletado(void){
unsigned char i;
unsigned char a[]="Ajuste Vibracion Completado ";	
      
	  VFDposicion_DDS(POSX0,POSY8);
	  for(i=0;i<sizeof(a);i++)
		  VFDserial_SendChar_DDS(a[i]);
	  AjustVibr->igxc1=_COMPLETADO_CCV;//=_COMPLETADO_CCV;
	  AjustVibr->igxc4=DONE; 
}//fin display angulo completado------------------------------


//despleiga el nombre de producto 
//posy:señala  la row, posx señala la posicion x ó central.
void display_Name_Product(unsigned posx,unsigned char posy){
unsigned char i,j;
	 if(posx!=CENTRAL){
		  __asm(Halt);//debug todavia no se desarrolla para otro ademans de central
	      return;}
	 for(i=1;i<sizeof(name);i++){//ver tamaño del nombre
	        if(name[i]==10)
	              break;
	        else{if(name[i]==0)
	                 break;}}
	//centrar nombre, caracter central 15. 
	j=120-(i/2)*8; //mitad de la longitud empezar antes del 15 
	VFDposicion(j,posy);
    for(j=1;j<i;j++)
	   VFDserial_SendChar(name[j]);
}//fin display name Producto--------------------------------

/* son cuadros concentricos 200, 400, 1k, 4k 
 * cuando se sacan los valores 4k cuenta como variable de
 * inizializacion 
 *  i1k es la variabe para saber en que cuadro concentrico estamos
 *   descargando
 *   swap: indica si regrea short int en x&y o regresa 
 *       uchar en ux4&uy4*/
unsigned char get_Pixels_Saved(signed short int *x,signed short int *y,unsigned char *ux4,unsigned char *uy4,unsigned char *swap){
static unsigned short int init9;
//signed short int x2,y2;    

     *swap=TRUE;
     if(init9!=0xAA){//empezo la descarga?
    	  init9=0xAA;
    	 dds.rePaint.ii=0;
         dds.rePaint.j=0;
         dds.rePaint.ix=0;
         dds.rePaint.iy=0;
         }
     if(dds.rePaint.j!=0xAA){
       if(dds.rePaint.j!=0xBB){	 
    	  if(is_get_number_from_pixel(ux4,uy4)){
    		   *swap=FALSE;
    	       return TRUE;}
    	  else {dds.rePaint.j=0xBB;
    	        return TRUE;}}
       else{init9=0;
            return FALSE;}}	   
     *x=0;  	       
     *y=0;
return TRUE;//todavia hay pixeles por vaciar  
}//fin get_Pixels_Saved--------------------------------


/* guarda los valores de deteccion en tiempo real
 * pero guarda los valores dependiendo  delos limites
 * de los numeros en cuadros concentricos e interior
 * mas chico que el exterior*/
void save_Pixels(signed short int x1,signed short int y1,unsigned char ux,unsigned char uy){
//static unsigned short int debug5;
unsigned short int *conteoPuntosSaved;
unsigned char b=0,mask=0,rx;
	if(x1==0)
		if(y1==0)
			return;
	conteoPuntosSaved=&dds.rePaint.debug5;
	if((ux>0)||(uy>0)){
		b=getRegistro_X_bit(&rx,ux);
		if((uy>(SIZE_Y))||(rx>(SIZE_X))){
			  return;
			  __asm(nop);
			  __asm(Halt);
			  __asm(nop);}
		
		if((dds.rePaint.xy[uy][rx]&b)==0)
			      ++*conteoPuntosSaved;
		if((uy==1)&&(rx==4)){
				     __asm(nop);
				     __asm(nop);
				       }
		dds.rePaint.xy[uy][rx]=dds.rePaint.xy[uy][rx]|b;
		
		   return;}
	   
     
   
}//fin save pixels------------------------------------------

/* despliega la linea de sensibilidad limite en el dds siempre y cuando
 *  este dentro del zoom seleccionado.
 *  param 1[v]: dinamico|Estatico; pintado de linea al inicio o en  pleno menu dinamico*/
void display_Sensibilidad_Geometrico(unsigned char v){
	  
	 if(is_drawable_Sensibility_DDS())
		if(is_Phase_0()) 
		   display_Sensibility_Geometry_Line(v);
	
}//fin display_Sensibilidad_Geometrico-----------------------------------

/* it draws Line Sesnsitivity, this method is used befor
 * verify the fitting into zoom otherwise will be an error   */
void display_Sensibility_Geometry_Line(unsigned char v){
unsigned short int w;
float Max,min,a;
//float f;
	 
     Max=(float)zoom.Ymax;//la maxima sensibilidad por zoom
     min=(float)zoom.Ymin;//la minima sensibilidad por zoom
	 if(v==ESTATICO){	 
	   if((zoom.Zoom>99)||(zoom.Zoom<1)){
		      __asm(nop); 
		      __asm(Halt);
		      __asm(nop);}
	   switch(zoom.Zoom){
		   case 99:
		   case 98:w=(unsigned short int)(-2.1*((float)Deteccion.Sensibilidad)+64.1);
		           break;//ecuacion de la recta para (1,62)(30,1) donde las x son la sensibilidad y las y la altura de 1 a 64 		  
		   default:a=(61*min)-(61*((float)Deteccion.Sensibilidad))+(62*(Max-min));
		           w=(unsigned short int)(a*(1/(Max-min)));
		           break;} 
	 VFDdrawLine(1,1,w,192,w);
	 VFDdrawLine(1,1,128-w,192,128-w);}
	 
}//fin display_Sensibility_Geometry_Line---------------------------------


//coordendadas absolutas y convertidas a estado final para ser pintadas en DDS
//Regresa el valor del regstro del 1 al 24 que corresponde  de la
/* ccoordenada x, en la absis de 1 a 192 se divden en registros
 * de 8 bits, y regresa el bit que corresponde a la coordenada 
 * en el registro reg, para guardar coordenadas dds de la pantalla */
unsigned char getRegistro_X_bit(unsigned char *Reg,unsigned char x){
unsigned char b,rx;
     
     switch(x){
			case 192:b=0x01;rx=0;break;
			case 1:b=0x02;rx=0;break;
			case 2:b=0x04;rx=0;break;
			case 3:b=0x08;rx=0;break;
			case 4:b=0x10;rx=0;break;
			case 5:b=0x20;rx=0;break;
			case 6:b=0x40;rx=0;break;
			case 7:b=0x80;rx=0;break;
			case 8:b=0x01;rx=1;break;
			case 9:b=0x02;rx=1;break;
			case 10:b=0x04;rx=1;break;
			case 11:b=0x08;rx=1;break;
			case 12:b=0x10;rx=1;break;
			case 13:b=0x20;rx=1;break;
			case 14:b=0x40;rx=1;break;
			case 15:b=0x80;rx=1;break;
			case 16:b=0x01;rx=2;break;
			case 17:b=0x02;rx=2;break;
			case 18:b=0x04;rx=2;break;
			case 19:b=0x08;rx=2;break;
			case 20:b=0x10;rx=2;break;
			case 21:b=0x20;rx=2;break;
			case 22:b=0x40;rx=2;break;
			case 23:b=0x80;rx=2;break;
			case 24:b=0x01;rx=3;break;
			case 25:b=0x02;rx=3;break;
			case 26:b=0x04;rx=3;break;
			case 27:b=0x08;rx=3;break;
			case 28:b=0x10;rx=3;break;
			case 29:b=0x20;rx=3;break;
			case 30:b=0x40;rx=3;break;
			case 31:b=0x80;rx=3;break;
			case 32:b=0x01;rx=4;break;
			case 33:b=0x02;rx=4;break;
			case 34:b=0x04;rx=4;break;
			case 35:b=0x08;rx=4;break;
			case 36:b=0x10;rx=4;break;
			case 37:b=0x20;rx=4;break;
			case 38:b=0x40;rx=4;break;
			case 39:b=0x80;rx=4;break;
			case 40:b=0x01;rx=5;break;
			case 41:b=0x02;rx=5;break;
			case 42:b=0x04;rx=5;break;
			case 43:b=0x08;rx=5;break;
			case 44:b=0x10;rx=5;break;
			case 45:b=0x20;rx=5;break;
			case 46:b=0x40;rx=5;break;
			case 47:b=0x80;rx=5;break;
			case 48:b=0x01;rx=6;break;
			case 49:b=0x02;rx=6;break;
			case 50:b=0x04;rx=6;break;
			case 51:b=0x08;rx=6;break;
			case 52:b=0x10;rx=6;break;
			case 53:b=0x20;rx=6;break;
			case 54:b=0x40;rx=6;break;
			case 55:b=0x80;rx=6;break;
			case 56:b=0x01;rx=7;break;
			case 57:b=0x02;rx=7;break;
			case 58:b=0x04;rx=7;break;
			case 59:b=0x08;rx=7;break;
			case 60:b=0x10;rx=7;break;
			case 61:b=0x20;rx=7;break;
			case 62:b=0x40;rx=7;break;
			case 63:b=0x80;rx=7;break;
			case 64:b=0x01;rx=8;break;
			case 65:b=0x02;rx=8;break;
			case 66:b=0x04;rx=8;break;
			case 67:b=0x08;rx=8;break;
			case 68:b=0x10;rx=8;break;
			case 69:b=0x20;rx=8;break;
			case 70:b=0x40;rx=8;break;
			case 71:b=0x80;rx=8;break;
			case 72:b=0x01;rx=9;break;
			case 73:b=0x02;rx=9;break;
			case 74:b=0x04;rx=9;break;
			case 75:b=0x08;rx=9;break;
			case 76:b=0x10;rx=9;break;
			case 77:b=0x20;rx=9;break;
			case 78:b=0x40;rx=9;break;
			case 79:b=0x80;rx=9;break;
			case 80:b=0x01;rx=10;break;
			case 81:b=0x02;rx=10;break;
			case 82:b=0x04;rx=10;break;
			case 83:b=0x08;rx=10;break;
			case 84:b=0x10;rx=10;break;
			case 85:b=0x20;rx=10;break;
			case 86:b=0x40;rx=10;break;
			case 87:b=0x80;rx=10;break;
			case 88:b=0x01;rx=11;break;
			case 89:b=0x02;rx=11;break;
			case 90:b=0x04;rx=11;break;
			case 91:b=0x08;rx=11;break;
			case 92:b=0x10;rx=11;break;
			case 93:b=0x20;rx=11;break;
			case 94:b=0x40;rx=11;break;
			case 95:b=0x80;rx=11;break;
			case 96:b=0x01;rx=12;break;
			case 97:b=0x02;rx=12;break;
			case 98:b=0x04;rx=12;break;
			case 99:b=0x08;rx=12;break;
			case 100:b=0x10;rx=12;break;
			case 101:b=0x20;rx=12;break;
			case 102:b=0x40;rx=12;break;
			case 103:b=0x80;rx=12;break;
			case 104:b=0x01;rx=13;break;
			case 105:b=0x02;rx=13;break;
			case 106:b=0x04;rx=13;break;
			case 107:b=0x08;rx=13;break;
			case 108:b=0x10;rx=13;break;
			case 109:b=0x20;rx=13;break;
			case 110:b=0x40;rx=13;break;
			case 111:b=0x80;rx=13;break;
			case 112:b=0x01;rx=14;break;
			case 113:b=0x02;rx=14;break;
			case 114:b=0x04;rx=14;break;
			case 115:b=0x08;rx=14;break;
			case 116:b=0x10;rx=14;break;
			case 117:b=0x20;rx=14;break;
			case 118:b=0x40;rx=14;break;
			case 119:b=0x80;rx=14;break;
			case 120:b=0x01;rx=15;break;
			case 121:b=0x02;rx=15;break;
			case 122:b=0x04;rx=15;break;
			case 123:b=0x08;rx=15;break;
			case 124:b=0x01;rx=15;break;
			case 125:b=0x20;rx=15;break;
			case 126:b=0x40;rx=15;break;
			case 127:b=0x80;rx=15;break;
			case 128:b=0x01;rx=16;break;
			case 129:b=0x02;rx=16;break;
			case 130:b=0x04;rx=16;break;
			case 131:b=0x08;rx=16;break;
			case 132:b=0x10;rx=16;break;
			case 133:b=0x20;rx=16;break;
			case 134:b=0x40;rx=16;break;
			case 135:b=0x80;rx=16;break;
			case 136:b=0x01;rx=17;break;
			case 137:b=0x02;rx=17;break;
			case 138:b=0x04;rx=17;break;
			case 139:b=0x08;rx=17;break;
			case 140:b=0x10;rx=17;break;
			case 141:b=0x20;rx=17;break;
			case 142:b=0x40;rx=17;break;
			case 143:b=0x80;rx=17;break;
			case 144:b=0x01;rx=18;break;
			case 145:b=0x02;rx=18;break;
			case 146:b=0x04;rx=18;break;
			case 147:b=0x08;rx=18;break;
			case 148:b=0x10;rx=18;break;
			case 149:b=0x20;rx=18;break;
			case 150:b=0x40;rx=18;break;
			case 151:b=0x80;rx=18;break;
			case 152:b=0x01;rx=19;break;
			case 153:b=0x02;rx=19;break;
			case 154:b=0x04;rx=19;break;
			case 155:b=0x08;rx=19;break;
			case 156:b=0x10;rx=19;break;
			case 157:b=0x20;rx=19;break;
			case 158:b=0x40;rx=19;break;
			case 159:b=0x80;rx=19;break;
			case 160:b=0x01;rx=20;break;
			case 161:b=0x02;rx=20;break;
			case 162:b=0x04;rx=20;break;
			case 163:b=0x08;rx=20;break;
			case 164:b=0x10;rx=20;break;
			case 165:b=0x20;rx=20;break;
			case 166:b=0x40;rx=20;break;
			case 167:b=0x80;rx=20;break;
			case 168:b=0x01;rx=21;break;
			case 169:b=0x02;rx=21;break;
			case 170:b=0x04;rx=21;break;
			case 171:b=0x08;rx=21;break;
			case 172:b=0x10;rx=21;break;
			case 173:b=0x20;rx=21;break;
			case 174:b=0x40;rx=21;break;
			case 175:b=0x80;rx=21;break;
			case 176:b=0x01;rx=22;break;
			case 177:b=0x02;rx=22;break;
			case 178:b=0x04;rx=22;break;
			case 179:b=0x08;rx=22;break;
			case 180:b=0x10;rx=22;break;
			case 181:b=0x20;rx=22;break;
			case 182:b=0x40;rx=22;break;
			case 183:b=0x80;rx=22;break;
			case 184:b=0x01;rx=23;break;
			case 185:b=0x02;rx=23;break;
			case 186:b=0x04;rx=23;break;
			case 187:b=0x08;rx=23;break;
			case 188:b=0x10;rx=23;break;
			case 189:b=0x20;rx=23;break;
			case 190:b=0x40;rx=23;break;
			case 191:b=0x80;rx=23;break;
			default:b=0;rx=0;break;  }//fin swithc----------------------  
            *Reg=rx;
return b;	
}//fin getRegistro_X_bit---------------------------------------------

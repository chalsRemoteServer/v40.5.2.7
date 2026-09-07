/*
 * VFDmenuControl.c
 *
 *  Created on: Oct 5, 2020
 *      Author: desarrollo-1
 */

#include "VFDmenuControl.h"
#include "VFDmenu.h"
#include "system.h"
#include "Reloj.h"
#include "VFDserial.h"
#include "VFDisplay.h"
#include "VFD.h"
#include "strings.h"
#include "I2C_.h"


extern unsigned char bufferXcontexto[BUFF_X_CONTEXTO_SIZE];//se usa solo en el contexto ó pantalla Actual, mas alla nop.
extern unsigned char GananciaDriver; //ganancia de la driver en AJUSTE_DE_SISTEMA/INGENIERIA_GRACIDA/GANANCIA
extern unsigned char PSUIOconfig; // confifura si permite o no las salidas/entradas en la tarjeta IO
extern unsigned char ControlContrasena1,ControlContrasena2,ControlContrasena3,ControlContrasena4;
extern unsigned char errorAnalogoPerm; // permitir error analogo
extern unsigned char cursorx,cursory; //posicion actual del cursor,
extern struct _Menu_ menu;
extern unsigned char GananciaAnaloga;//ganancia de la analoga en  AJUSTE_DE_SISTEMA/INGENIERIA_GRACIDA/GANANCIA
extern unsigned char ReporteProductoPerm; //permitir reporte de producto 

extern struct EntSyst{//variables de entradas de sistema 
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

void PSU_IO_var_rotate(void){
	 if(PSUIOconfig==PERMITIR)
		 PSUIOconfig=NO;
	 else PSUIOconfig=PERMITIR;
}// fin POWER SUPPLY UNIT  variable rotate-------------------------------------------

void ErrorAnalogoVarRotate(void){
	 if(errorAnalogoPerm==PERMITIR)
		 errorAnalogoPerm=NO;
     else errorAnalogoPerm=PERMITIR;
}// fin error analogo variable rotate-----------------------------------------------

void ReporteProductoVarRotate(void){
	 if(ReporteProductoPerm=='1')
		  ReporteProductoPerm='0';
	 else ReporteProductoPerm='1';
}// fin reporte de producto variable rotate-----------------------------------------


/*Busca un lugar vacio para hacer un nuevo producto 
 * return the index of the product free space*/
unsigned char SearchSpace(unsigned char mode){
static unsigned char  iProd;
	switch(mode){
	    case UP_SCREEN:iProd=memoEEPROM_SearchPlaceUP(getiProducSelected(cursory));
	                   break;
    	case INIT_NEW_PROD_MENU:
    		   iProd=memoEEPROM_SearchPlace(2);//empieza del hueco  iProduct=2
    		   break;
    	case DOWN_SCREEN:iProd=memoEEPROM_SearchPlace(iProd+1);
    		             break;
    	default:break;}
	    return iProd;
    
}//fin function thar search for space fr new product------------------------------------------




void fillScreenNames(void){//unsigned char mode)
unsigned char *s,NumSup,yrow,i;
word m,n;
     VFDclrscr();
     NumSup=LimitesMenuPROD(0,READ_LIM_SUP);
	 /* if(mode==DISPLAY_NUM_LIST)*/
     displayNumbers(NumSup);//despleiga los numeros desde arriba hasta abajo con los limites referenciados
	 for(i=NumSup,yrow=2;i<(NumSup+7);i++,yrow+=2){
		  s=memoEEPROM_SelProd(i);//saca el nombre del Prod numero i
		  LimitesMenuPROD(*s,BUFFER_PROD|yrow);//guardamos el estado de cada row si esta vacia o si no
		  m=length2(s);  //Obtiene el numero de charateres  
		  VFDposicion(POSXSELPROD,yrow);
		  VFDserial_SendBlock((s+1),m,&n);
	      iProduct_Mark(WRITE,s,yrow);}
	 
}// fin de llenar la pantalla con los nombres de los productos


//obtiene el primer indice iProd[0] del los nombres fillscreen actuales en pantalla
/* op: READ | WRITE,   *p:guardar la marca de Producto  rowy:en que ROW y guardamos la marca
 * */
unsigned char iProduct_Mark(unsigned char op,unsigned char *p,unsigned char rowy){
static unsigned char iProductMark[8];
	switch(op){
		case READ: return(iProductMark[rowy-(rowy/2)]); 
			      break;
		case WRITE:iProductMark[rowy-(rowy/2)]=*p;
			       break;
		default:break;
	}//fin switch----------------------------
}//fin dr getiProduct_Mark--------------------------------------------



//mode: 
//aqui obteemos los limites de los numeros que son los indice de prodcto en  pantalla
unsigned char  LimitesMenuPROD(unsigned char sup, unsigned char mode){//Estos numeros deben ser guardados en  la NVRAM
static unsigned char  superior;//debug NVRAM valores deben ser guardados en la NVRAM
static unsigned char newprodSup;//debug NVRAM valoress debe ser saved in it
static unsigned char prodBuffer[7];
static unsigned char newprodb;
unsigned char i;
    if((mode==ESCRIBIR)||(mode==BUFFER_PROD))
    	if(sup==0)
    	   __asm(Halt);//debug error de diseño de software
    if((mode&0xF0)==0xF0){//buffer prod, es una indicacion de operacion y posicion de row
    	 i=mode & 0xF0;   //le indicamos que hay en cada row si esta vacia o si no
    	 mode=BUFFER_PROD;}//ya separamos las row y el buff prod
    switch(menu.contexto.Actual){
     case SELECCIONAR_PRODUCTO:
     case AJUSTE_PRODUCTO_SEL:
         switch(mode){
           case SUMAR: superior++;break;
           case RESTAR: superior--;break;
           case ESCRIBIR:superior=sup;break;
           case BUFFER_PROD: prodBuffer[i]=sup; break;
           case READ_BUFF_PROD:return prodBuffer[sup];break;
           case SET:superior=sup;break;
           default:break;}	
           return superior;
         break;
     case NUEVO_PRODUCTO:
    	 switch(mode){
    	   case SUMAR: newprodSup++;break;
    	   case RESTAR: newprodSup--;break;
    	   case ESCRIBIR:newprodSup=sup;break;
    	   case BUFFER_PROD:prodBuffer[i]=sup; break;
    	   case READ_BUFF_PROD:return prodBuffer[sup];break;
    	   case READ_LIM_INF:return (newprodSup+6);break; //inidca que leemos el limite inferior de iProduct de la ´pantalla
    	   case READ_LIM_SUP:return newprodSup;break;
    	   default:break;}
    	   return newprodSup;  
    	 break;
     case BORRAR_PRODUCTO:
    	 switch(mode){
		   case	READ:return(prodBuffer[sup]);break; 
    	   case SUMAR:newprodb++;break;
    	   case RESTAR:newprodb--;break;
    	   case ESCRIBIR:newprodb=sup;break;
    	   case BUFFER_PROD:prodBuffer[i]=sup;break;
    	   case READ_LIM_INF:return(newprodb+6);break;
    	   case READ_LIM_SUP:return newprodb;break;
    	   case SET:newprodb=sup;break;
    	   default:break;}
    	 return newprodb;
    	 break;
     case PASTE_PRODUCTO:	 
     case COPY_PRODUCTO:
     	 switch(mode){
      	   case SUMAR:newprodb++;break;
      	   case RESTAR:newprodb--;break;
     	   case ESCRIBIR:newprodb=sup;break;
      	   case BUFFER_PROD:prodBuffer[i]=sup;break;
      	   case READ_LIM_INF:return(newprodb+6);break;
       	   case READ_LIM_SUP:return newprodb;break;
    	   case SET:newprodb=sup;break;
        default:break;}
        return newprodb;
    	break;
     break;
     		    		     	 
    	 
     default:break;} //ends Main switch
    
    
return 0;
}//fin de obtenemos los limites o los numeros superior y einferior de la pantalla del menu de select product guardados en NVRAM







/*esta el iProduct encontrado en la misma pantalla
 * el producto que se encontro el iProd esta en la misma pantalla
 *  ver si est dentro de los rangos del limite sup de la pantalla actual
 *  return TRUE: esta dentro de la misma pantalla
 *  FALSE: se encuentra fuera de la pantalla*/
unsigned char isSameScreeniProduct(unsigned char dir,unsigned char iProd){
	if(iProd==0)
	      asm(Halt);//debug error de diseño de sofware
	if(dir==PABAJO){
	     if((iProd-LimitesMenuPROD(0,READ_LIM_SUP))<7)
	    	 return TRUE;
	     else return FALSE; }
	else 
		if(dir==PARRIBA){
		     if((iProd<LimitesMenuPROD(0,READ_LIM_SUP)))
		    	 return FALSE;
		     else return TRUE;}

}// FINV isSameScreeniProduct-----------------------------



/* TRUE: si esta seleccionado el iProduct 1 que e test set up
 * FALSE: no esta seleccinado el test setup*/
unsigned char is_iProduct_1(void){
unsigned char i;	
	   i=LimitesMenuPROD(0,READ_LIM_SUP);//cual es el limite superior de pantalla
	   if(i>1)//el limite superior no es uno por lo tanto no esta seleccionado test-setup
		   return FALSE;
	   if(i==1)
		   if(cursory==POSY2)
			   return TRUE;
return FALSE;
}//fin is_iProduct_1---------------------------------------



/*Convierte a numero POSY posicion Y, contexto borrar producto, basado
 * en el numero indice de iProduct  superior que se muestra en la 
 *pantalla actual, en el iProduct actual seleccionado e.g. superior 100, seleccionado
 *pantalla 112 en la posicion POSY3 regresa 112 */
unsigned char convertPOSY2iProduct(unsigned char sup,unsigned char pos){
    	  return(sup+(pos-(pos/2))-1);
}//fin de convertPOSY2iProduct-----------------------------------------------------

//convertimos el iproduct actual en POSY, se debe encontrar en la misma pantalla
//el superior y el iProduct que queremos su posicion, SAME SCREEN
unsigned char convertiProduct2POSY(unsigned char iProd){
unsigned char i;
        if(iProd==0)
        	__asm(Halt);//debug error de diseño de sofware
	    i=iProd-LimitesMenuPROD(0,READ_LIM_SUP);
	    if(i>6)
	    	__asm(Halt); //error de diseño de software
    	return((i+1)*2);
}//fin convertiProduct2POSY-------------------------------------------

/*buscamos el siguiente iProduct, el indice del siguiente producto en memoria
 *  incluyendo el iProduct del que partimos. 
 *  dir: ARRIBA | ABAJO   iProd: indice de producto en el que empieza a buscar mas uno no incluyendolo*/
unsigned char searchNextiProduct(unsigned char dir,unsigned char iProd){
unsigned char i;	
unsigned long int debug=0;
   if(iProd==0)
      	__asm(Halt);//debug error de diseño de sofware
   if(dir==PABAJO){
	   iProd++;//para que cuente apartir del siguiente
	   debug+=10000;
	   for(i=iProd;i<MAXIMO_NUMERO_PROD;i++)
	      if(bufferXcontexto[i]==NAME_INIT){//busca en base de datos de iProducts registrados cuando se creo la pantalla
	    	   if(i==0)
	    		   __asm(Halt);
	    	   return i;}
	   return 0;//ya no hay nada mas para abajo 
       debug+=1;}
   else
	  if(dir==PARRIBA){
		  if(iProd==2)
			  return 1;// encontramos el Test-Setup
		  iProd--;//para que cuente apartir del siguiente parriba
		  debug+=1000;
		  for(i=iProd;i>1;i--){
			  if(i==1)//test setup
				  return 1;
			  if(bufferXcontexto[i]==NAME_INIT){
				  if(i==0)
					  __asm(Halt);
				  return i;}
		          debug+=10;}
		  if(i==1)//test setup
		      return 1;
	  debug+=100;}//fin if PARRIBA
__asm(Halt);//debug el codigo no debe llegar aqui
}//fin searchNextiProduct-------------------------------------------------


/*Get the index product when select the product in the selProd Menu y producto nuevo*/
unsigned char getiProducSelected(unsigned char posy){
	                //0,1,2,3,4,5,6,7,8,9,10,11,12,13,14
//const unsigned char y[15]={0,0,0,0,1,0,2,0,3,0,4, 0,  5,0,  6};
return(convertPOSY2iProduct(LimitesMenuPROD(0,0),posy));
}//fin de get index product selected on select menu-----------------




char rotateEntradasSistemasVars(unsigned char var){
	switch(var){
	  case _TACOMETRO_:if(EntradasSistemaReg.taco=='1'){
		                        EntradasSistemaReg.taco='0';
	                            return(EntradasSistemaReg.taco);}
	                   else {EntradasSistemaReg.taco='1';
	                         return(EntradasSistemaReg.taco);}
		               break;
	 case _FOTOCELDA_:if(EntradasSistemaReg.fotocel=='1'){
                                     EntradasSistemaReg.fotocel='0';
                                     return(EntradasSistemaReg.fotocel);}
                      else {EntradasSistemaReg.fotocel='1';
                            return(EntradasSistemaReg.fotocel);}
                      break; 		               
	 case _RECHAZO_:  if(EntradasSistemaReg.rechazo=='1'){
                                    EntradasSistemaReg.rechazo='0';
                                    return(EntradasSistemaReg.rechazo);}
                      else {EntradasSistemaReg.rechazo='1';
                            return(EntradasSistemaReg.rechazo);}
                      break;                
	 case _PRODUCTO_: if(EntradasSistemaReg.producto=='1'){
                            EntradasSistemaReg.producto='0';
                            return(EntradasSistemaReg.producto);}
                      else {EntradasSistemaReg.producto='1';
                            return(EntradasSistemaReg.producto);}
                      break;                
	 case _AUX1_:     if(EntradasSistemaReg.aux1=='1'){
                               EntradasSistemaReg.aux1='0';
                               return(EntradasSistemaReg.aux1);}
                      else {EntradasSistemaReg.aux1='1';
                           return(EntradasSistemaReg.aux1);}
                      break;
	 case _AUX2_:     if(EntradasSistemaReg.aux2=='1'){
                           EntradasSistemaReg.aux2='0';
                           return(EntradasSistemaReg.aux2);}
                      else {EntradasSistemaReg.aux2='1';
                            return(EntradasSistemaReg.aux2);}
                      break;                 
	}// FIN SWITCH-------------------------------------
return 0;	
}// fin method rotate entradasSistemas Vars------------------------------------------------------------------


	
unsigned short int  digito2posicion(unsigned short int digito){
    	         switch(digito){
	                       case 1: digito=POSXDSSDIGITO1; break;
   	                       case 2: digito=POSXDSSDIGITO2; break;
	                       case 3: digito=POSXDSSDIGITO3; break;
  	                       case 4: digito=POSXDSSDIGITO4; break;
	                       case 5: digito=POSXDSSDIGITO5; break;
  	                       default:digito=0;break; }//fin switch
return digito;         
}// digito to position converter,,, digito  de pantalla convertido a la posicion real en el VFD
	


void rotateContrasenaVars(unsigned char psw){//Permitir------------------------------
	                           //               Apagar 
	
	switch(psw){//falta gardar variables y poner el estas seguro?
	   case POSY6:if(ControlContrasena1==PERMITIR){ControlContrasena1=APAGAR;break;}
	              else {ControlContrasena1=PERMITIR;break;}
	   case POSY8:if(ControlContrasena2==PERMITIR){ControlContrasena2=APAGAR;break;}
                  else {ControlContrasena2=PERMITIR;break;}
	   case POSY10:if(ControlContrasena3==PERMITIR){ControlContrasena3=APAGAR;break;}
                  else {ControlContrasena3=PERMITIR;break;}
	   case POSY12:if(ControlContrasena4==PERMITIR){ControlContrasena4=APAGAR;break;}
                   else {ControlContrasena4=PERMITIR;break;}	    
	}//fin switch psw-----------------------------------------------
}// fin control contrasenas variables control-------------------------------------------



void configReloj(unsigned char sens){
unsigned char	 n;
	 //readclock();//traer las variables del tiempo al procesador DE LA NVRAM
	 switch(cursory){
	   case POSY2 :n=getYear();
	               if(sens==KEY_RT) setYear(++n);
	               else setYear(--n);
	               displayTime(_YEAR_);
	               break;
	   case POSY4 :n=getMonth();
	               if(sens==KEY_RT){
	                     if(++n>12) n=1;}
	               else {
	                     if(--n==0) n =12;}
	               setMonth(n);
	               displayTime(_MES_);
	               break;
	   case POSY6 :n=getDate();
                   if(sens==KEY_RT){
     	             if(++n>=32) n=1;}
                   else { if(--n==0) n=31;}
     	           setDate(n);
                   displayTime(_DIA_);
                   break;
	   case POSY8 :n=gethora();
                   if(sens==KEY_RT){
     	               	  if(++n>=24) n=0;}
                   else {if(--n>24) n=23;}
            	   sethora(n);
                   displayTime(_HORA_);
                   break;
	   case POSY10:n=getminutos();
                   if(sens==KEY_RT){
	                   	if(++n>59) n=0;} 
                   else {if(--n>60) n=59;} 
                   setminutos(n); 
                   displayTime(_MIN_);
                   break;
	   case POSY12:break;
	 }//fin switch cursory----------------------
}// fin configuration of the clock-reloj---------------------------------------------------------------

void gananciaValuesRotary(unsigned char tarjeta){
	if(menu.contexto.Actual==GANANCIA_){
		if(tarjeta==ANALOGA){
		   switch(GananciaAnaloga){
		    case ALTA: GananciaAnaloga=MEDIA;break;
		    case MEDIA:GananciaAnaloga=BAJA;break;
		    case BAJA: GananciaAnaloga=ALTA;break;
		    default:GananciaAnaloga=MEDIA;break; }}//esto es el inicio o si la e2prom no funciona
		else{
			switch(GananciaDriver){
					    case ALTA: GananciaDriver=BAJA;break;
					    case BAJA:GananciaDriver=AJUSTABLE;break;
					    case AJUSTABLE: GananciaDriver=ALTA;break;
					    default:GananciaDriver=BAJA;break; }}//esto es el inicio o si la e2prom no funciona
		}//IF1
}// FIN ROTACION DE VALOREAS DE  LA GANACIA DE LAS TARJETAS ---------------------------

/* vamos a buscar toda ma memoria de productos si hay productos o si no
 * hay productos TRUE si no hay productos*/
unsigned char iProductEmpty(void){
unsigned char i,n=0;	
//	  if(readEEPROMbyte(PRODUCTO_COUNTING_ADD)==0)
//		        return TRUE;//Si esta vacio
//	  else return FALSE;//No esta vacio	 
	   //para que cuente apartir del siguiente
	   for(i=2;i<(MAXIMO_NUMERO_PROD+1);i++)
		  if(bufferXcontexto[i]==NAME_INIT)//busca en base de datos de iProducts registrados cuando se creo la pantalla
			   return FALSE;

return TRUE;
}//fin iProductEmpty---------------------------------------------------------------------

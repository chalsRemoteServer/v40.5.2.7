/*
 * maths.c
 *
 *  Created on: Mar 5, 2020
 *      Author: desarrollo-1
 */


#include "maths.h"
#include "system.h"
#include "queue.h"
#include "DSP.h"
#include "VFDisplay.h"


extern unsigned char phase,phasefrac;//phase de ajustes de deteccion la variable fraccionaria es de la misma vriable
extern struct _GLOBAL_CONTEXTO gxc,gxc2,gxc3,gxc4;
extern struct _Comando_DDS dds;
extern struct ZoomControl zoom;

/*op: SUMAR | RESTAR  operacion de las variables
 * digito es el digito que se le va hacer la operacion   000.0
 * digito 5 es el milenio                                ^^^^^
 *                                              digito-->54321
 * */
void operacionPhase(unsigned char digito,unsigned char op){
//unsigned char a[3];

	
	//getcharFromUChar(phase,&a[0]);
	switch(op){
		case RESTAR:if((phase==0)&&(phasefrac==0)){
			            	  phase=179;
			            	  phasefrac=9;
		                      break;}
					if(phase==0){
							  phase=179;
							  phasefrac=9;
							  break;}
		            if(digito==1){
			            if(phasefrac==0){
		                       phase--;
			                   phasefrac=9;}
			            else phasefrac--;
		                break;}
		            if(digito==3){
		            	 phase--;
		            	 break;}
		            if(digito==4){
		                if(phase<10){
		                	phase=179;
		                	phasefrac=9;
		                	break;}	
		                else phase-=10;
		                break;}
			        if(digito==5){
			        	if(phase<100){
			        		phase=179;
			        		phasefrac=9;
			        		break;}
			        	else phase-=100;
			            break;}
			        break;
		case SUMAR:if(digito==1){
			             if(phasefrac==9){
			            	 phase++;
			            	 phasefrac=0;
			            	 break;}}
		           if(digito==3){
		        	     if(phase<179){
		        	    	 phase++;
		        	         break;}
		        	     else {phase=0;
		        	           phasefrac=0;
		        	           break;}}
		        	if(digito==4){
		        		 if(phase==179){
		        			  phase=0;
		        			  phasefrac=0;
		        			  break;}
		        		 else{phase+=10;
		                      break;}}
		            if(digito==5){
		            	 if(phase==179){
		            		  phase=0;
		            		  phasefrac=0;
		            		  break;}
		            	 if(phase>99){
		            		 phase=0;
		            		 phasefrac=0;
		            		 break;}}
			       break;
		default:break;
	}//fin switch
	
}//fin operacion phase--------------------------------------------------


/*  *n: numero a ser procesado del tipo short int    
 *  digito: digito que se va ha hacer la operacion   digito-> 12345  <--posicion, 5->unidad
 *  op: operacion SUMAR | RESTAR    
 *  Funcion que resta o suma  en un digito en particular  */
void operacionUShortInt(unsigned short int *n,unsigned char digito,unsigned char op){//opera en el digito indicado
unsigned short int i,j;
const unsigned short int num[]={0,0,10,100,1000,10000};
	switch(op){
		case RESTAR:if(*n==1)
			             break;
		             j=num[digito];
		             //if(digito==5)
		            	//  j=32765;
					 switch(digito){
						 case 1: (*n)--; return;
						 case 2:
						 case 3:
						 case 4:
						 case 5: i=*n;
								 if(i<j)
									 *n=1;    	 
								else{i-=num[digito];
									 *n=i;}
							    break;
						default:break;}
				   break;//case restar
		case SUMAR:if(*n<32767){
			    	  if(digito==1){
			    		  (*n)++;
			    		  break;}
			    	  if(digito==2){
			    		  i=*n;
			    		  i+=10;
			    		  if(i>32767)
			    			  i=32767;
			    		  *n=i;
			    	      break;}
			    	  if(digito==3){
			    		  i=*n;
			    		  i+=100;
			    		  if(i>32767)
			    		     i=32767;
			    		  *n=i;
			    		  break;}
			    	  if(digito==4){
			    		  i=*n;
			    		  i+=1000;
			    		  if(i>32767)
			    		  	  i=32767;
			    		  *n=i;
			    		  break;}
			    	  if(digito==4){
						  i=*n;
						  i+=1000;
						  if(i>32767)
							  i=32767;
						  *n=i;
						  break;}
			    	  if(digito==5){
			    		 i=*n;
			    		 i+=10000;
			    		 if(i>32767)
			    			 i=32767;
			    		 *n=i;
			    	     break;}
			       }else break; 
		default:break;}
}//fin operacion de  numero short int -------------------------------------------------------


/* GEnerate a ecuation linea from
 *   Genera el maximo basado en la ecacion generada con los puntos
 * *   
 *     (99,32000)(1,64)  -----> ecuacion generada con estos puntos
 */
signed short int getMaximun(unsigned char zoom){ //Maximo valor por pantalla en determinado zoom
	const float x1=99;      //maz zoom  hacemos la ecuacion de la recta para cada maximo vs cada zoom
	const float y1=32000;  //cambiar este valor para aujustar el maximo maximus
	const float x2=1;     //zoom minimo
	const float y2=64;  //maximo menor
	float m,b;
//f(x)=mx+b  Aqui sacamos la ecuacion de la recta general para todos los Zoom,  valor ADC's max-min vs Zooms max-min
	     m=(y2-y1)/(x2-x1); //la pendiente
	     b=m*(-1)*x1+y1;
	     return ((signed short int)(m*((float)zoom)+b)); //aqui sacamos el valor f(x) que es el zoom y la x es el valor maximo del ADC para ese Zoom
}//final from get maximun  value on function on zoom



//pixel Analog up or down, e.g. 32.64 became 33, or 32.12 became 32
float redondearFloat(float f){
unsigned short int s;
float ff,g;	
unsigned char signo=POSITIVO;
	
     if(f<0){ //es negativo
    	 ff=f*-1;//convertir a positivo
    	 signo=NEGATIVO;}//Procesamos un num negativo
     else {ff=f;
           signo=POSITIVO;}//procesamiento de num positivo
     for(s=0;s<10000;s++){
    	   if((ff-(float)s)<1)//buscar el numero entero
    	       break; }
     ff=ff-s;//vaciamos solo la fraccion
	 for(g=0.0;g<=0.9;g+=0.1){
		   if((ff-g)<=0.09)//buscamos la fraccion en un digito 
			   break;}
	 if(g>0.5)
		 return(s+1);
	 else return(s);
	    
}//fin redondearFloat-----------------------------------------------------------------------------------


/*convierte el numero WORD  ADCVAL: es un numero del adc que es el voltaje de 0 a 65520 representa
 * 0v a 1.3volts    regresa float el numero miliamper  en float
 * formula es  y=19.834x10^-6(x)+0.03826       y=mx+b*/
double convertir_ADC_word_A_miliVolt(unsigned short int ADCval,unsigned char channel){
double r;
	  
    switch(channel){
    	case _PICKUP_://r=0.000019834*((float)ADCval)+0.03826;//Pickup formula con multimetro y generador
                      r=(0.000015367421067*((float)ADCval))+0.11468063; //con osciloscopio y señal real
    		          break;
    	case _DRIVER_://r=0.000722366*((float)ADCval)+0.177825;//DRiver formula con multimetro y generador
                      r=((0.000692321)*((float)ADCval))+3.880113079; //con osciloscopio y señal real
    		          break;
    	case CANAL_Y:              
    	case CANAL_X://r=m(x-a)+b; ecuacion de la recta (10,0),(17709,2.71)
    	             r=0.00015*((float)ADCval)-0.00153; //voltaje canal X
    	             break;}
        
return r;
}//fin convertir_ADC_word_A_miliVolt-----------------------------------------------


double absd(double m){//get absolute from double
	     if(m<0)
		   return(m*(-1));
		 else return(m);  //absoluto de la pendiente      
}//fin de obtener el absoluto de un numero float---------------------------------------



float absf(float m){//get absolute from float
	     if(m<0)
		   return(m*(-1));
		 else return(m);  //absoluto de la pendiente      
}//fin de obtener el absoluto de un numero float---------------------------------------


signed short int absSSInt(signed short int a){//get the absolute from signed short int
	if(a<0)
	  return(a*(-1));
	else return (a);
}//fin de signed short int absolute--------------------------------------------------



/*Obtener el check sum del pquete de datos a enviar
 *  cmd es el comando a enviar como cabeza, data es el apuntador a los datos a enviar
 *  len: es el numero de bytes a enviar sin contar comando ni checksum., b
 *  version-1*/
unsigned char getCheckSUM(unsigned char cmd, unsigned char *data, unsigned char len){
unsigned char i;
union _ussint_{
	unsigned short int Suma;
	unsigned char bytes[2];
}suma1;
           suma1.Suma=0;
           suma1.Suma=cmd;
           for(i=0;i<len;i++)
	           suma1.Suma+=*(data+i);
           i=suma1.bytes[0]+suma1.bytes[1];
           i^=0xFF;//nvertir valores;
return(i);                                 //inversor
}//fin de get checksum del paquete de datos que pretendemos enviar----------------------


//cuenta o descueta la variable que cuenta cuantos nodos quedan libres
unsigned char FIFOcount(unsigned char var,unsigned char op){
	 if(op==RESTAR){
		  if(var==0){
			  __asm(Halt);
			  return (0);}// __asm(Halt); //debug error de ingenieria de software
		  else 
			  return (var-1);}
	 else
		 return(var+1);
return 0;
}//fin de FIFO counte------------------------------------------------------

/*obtener los bytes rovenientes de un numero
 *   signed short int  
 *    version-1   */
void getBytes_from_SSInt(unsigned char *byteMSB,unsigned char *byteLSB,signed short int ssi){
union gr{
  signed short int ssi;
  unsigned char byte[2];
}number;	
	          number.ssi=ssi;
	          *byteMSB=number.byte[0];
	          *byteLSB=number.byte[1];
}//fin getBytes_from_SSInt-------------------------------------------


/* convierte las coordenadas de 0 a 4000 y de 0 a -4000
  en coordenadas de 0 a 1000 en 0 a 1000 bit 1
  en  0 a -1000  en array 0 a 1000 bit 5
  en coordenadas de 0 a 2000 en 0 a 1000 bit 2
   en 0 a -2000 en 0 a 1000 bit 6 
   regresa en numero del bit 
   y asi sucesivamente  hasta 4000 y -4000 mas de eso lo ignora*/
//unsigned char save_Pixels(signed short int x,signed short int y){
//unsigned char signox=0,signoy=0;
////signed short int xx,yy;
//unsigned short int i;    
//const unsigned char byte[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
//const unsigned char inv[]= {0x7F,0xBF,0xDF,0xEF,0xF7,0xFB,0xFD,0xFE};
//	signox=getSigno_Convert(&x);
//	signoy=getSigno_Convert(&y);
//	i=get_index_bit(&x,signox);
//	dds.rePaint.x[x]|=byte[i];
//	i=get_index_bit(&y,signoy);
//	dds.rePaint.y[y]|=byte[i];
//	
//}//fin getConvertXY---------------------------------




//convierte un numero negativo en positivo y regresa
//que signo era
unsigned char getSigno_Convert(signed short int *n){
signed short int m;
    m=*n;
	if(m<0){
		*n=m*-1;
		return NEG;}
	else return POS;
}//fin  getSigno_Convert-------------------------




/*Return FALSE if it finish till the last coord,*/
unsigned char is_get_number_from_pixel(unsigned char *x,unsigned char  *y){
//unsigned char i;
const unsigned char byte[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
unsigned char xx,b;
static unsigned char bit,num,init,reg,var;
static unsigned char debugx[20],debugy[20],i;
static unsigned char row,col;

      row=dds.rePaint.iy;
      reg=dds.rePaint.ix;
      b=dds.rePaint.b;
      if(init!=0xAA){
    	   init=0xAA;
    	   dds.rePaint.iy=0;
    	   dds.rePaint.ix=0;
    	   num=0;
    	   bit=0;
    	  b=0x00;
    	   *x=0;*y=0;
    	   i=0;
    	   row=0;
    	   col=0;
    	   reg=0;}
      if(b>7){
    	   b=0;
    	   if((++reg)>(SIZE_X-1)){
    		    reg=0;
    		    if((++row)>(SIZE_Y-1)){
    		    	init=0;
    		    	return FALSE;}}}
     xx=dds.rePaint.xy[row][reg];
     if(xx==0){*x=0;*y=0;}
     else{if((xx&byte[b])==byte[b]){
    	      *x=get_Coord_Num(reg,b);
    	      *y=row;}
          else{*x=0;*y=0;}}
      
      
	dds.rePaint.iy=row;
	dds.rePaint.ix=reg;
	dds.rePaint.b=b+1;
return TRUE; //SEGUIMOS SACANDO NUMEROS DE LA MATRIZ
}//in is_get_number_from_pixel

/* se recalcula con el buffer que tenemos gurdado todos los puntoos
 * actuales del dds y algunos valores fuera del dds, el nuevo zoom
 * basado en el que hay, se reacomodan los puntos en base el nuevo zoom
 * */
void re_Calcular_Buffers_DDS(unsigned char zoom_wished){
	 
	 if(zoom_wished==zoom.Zoom)
		 return;
	 if(zoom_wished<zoom.Zoom){//se vamos a restar?
		 if((zoom.Zoom-zoom_wished)==1)//solo le sera uno, la resta
			 re_Calcular_Buffers_DDS_Modificar_Uno(zoom.Zoom-1);}
	 else{//aumentar el zoom ACERCAR
		  if((zoom_wished-zoom.Zoom)==1)
			  re_Calcular_Buffers_DDS_Modificar_Uno(zoom.Zoom+1);}
		   
	 
	
	
	
}//fin de recalculr el bufer que esta guardado en el DDS----------


void re_Calcular_Buffers_DDS_Modificar_Uno(unsigned char zoom1){
#if(SIZE_X>254)
	unsigned short int i,j;
#else
	unsigned char i,j;
#endif

unsigned char xn,b,xr,yr,cuadrante,bit,reg;
//unsigned char buff[2][SIZE_X];
//unsigned char xy[SIZE_Y][SIZE_X];
const unsigned char bbyte[]={1,2,4,8,16,32,64,0x80};
unsigned char zoomPalanca;//aleja o acerca?
    if(zoom.Zoom>zoom1) {
    	 zoomPalanca=ALEJAR;
    	 zoom.Zoom--;}
    else {if(zoom.Zoom<zoom1){
    	    zoomPalanca=ACERCAR;
            zoom.Zoom++;}
    	  else{__asm(nop);__asm(Halt);__asm(nop);}}
	for(j=0;j<SIZE_Y;j++)
	 for(i=0;i<SIZE_X;i++)
		 dds.rePaint.xy0[j][i]=0;
	for(j=0;j<SIZE_Y;j++)
	  for(i=0;i<SIZE_X;i++)
		  for(b=0;b<8;b++){//sacar el numero de x uno por uno del regitro x
             if((dds.rePaint.xy[j][i]&bbyte[b])==bbyte[b]){
			       xn=get_Coord_Num(i,b);	//OBTENER EL numero que corresponde al bit activado que es un pixel de 0 a 192
			       cuadrante=get_cuadrante(xn,j);//a que cuadrante pertenecen esta coordenada
                   reconvertir_coord_reducir_uno(xn,j,cuadrante,&xr,&yr,zoomPalanca);//reconvertir coordenada actual y redcirla a escala uno menor
                   bit=getRegistro_X_bit(&reg,xr);//obtener el registro y el bit de esta nueva coordenada
                   dds.rePaint.xy0[yr][reg]|= bit;
                   __asm(nop);
                   }}//Guradar nueva coordenada modificada de zoom buf temporal 
	//para fines de debug se combinan ambos buffers
	for(j=0;j<SIZE_Y;j++) //hacer un OR con los pixeles que tenemos con
		  for(i=0;i<SIZE_X;i++)//los reducidos a otro zoom, Debug solo
		        dds.rePaint.xy[j][i]=dds.rePaint.xy0[j][i];
		       
		        	
}//fin re_Calcular_Buffers_DDS_Alejar_Uno--------------

/*//reconvertir coordenada actual y redcirla a escala uno menor 
 * palanca: para saber si REDUCE  o aumenta el zoom a uno */
void reconvertir_coord_reducir_uno(unsigned char xn,unsigned char yn,unsigned char cuadrante,unsigned char *xr,unsigned char *yr,unsigned char palanca){
unsigned char a,b,c,d;
  if(palanca==ALEJAR){
          	 a=xn+1;b=yn+1;
          	 c=xn-1;d=yn-1;}
  else {a=xn-1;b=yn-1;
	    c=xn+1;d=yn+1;}
	switch(cuadrante){
		case 1:if(xn>0)
			       *xr=a;//xn+1;
		       if(yn>0)
		    	   *yr=b;//yn+1;
		       break;
		case 2:if(xn>0)
		        *xr=c;//xn-1;
	          if(yn>0)
	    	    *yr=b;//yn+1;
	           break;
		case 3:if(xn>0)
	              *xr=a;//xn+1;
               if(yn>0)
    	          *yr=d;//yn-1;
               break;
		case 4:if(xn>0)
	             *xr=c;//xn-1;
               if(yn>0)
    	         *yr=d;//yn-1;
              break;
			   __asm(nop);
			   __asm(Halt);
			   __asm(nop);
		default:break;}
	
	
	
}//fin reconvertir_coord_reducir_uno




/* obtenemos el cuadrante donde debe estar esta coordenada que va desde
 *  el numero 1 al 4 
 * cuadrante donde se encuentra el numero que va del 0 al 192  ydel 0 al 128 pero viendo el centro en 92,64*/
unsigned char get_cuadrante(unsigned char xi,unsigned char yi){
	
  if(xi<96){
	  if(yi<64)
	       return 1;
	  else return 3;}
  else{if(yi<64)
           return 2;
       else return 4;}
}//fin de la funcion get el cuadrante del dds donde esta esta coordenada


/* tenemos el registro de los  un registro de las lineas x del despliegue en pantalla de los pixeles
 *  el registro guarda los pixeles que se encienden o no en valor absoluto para se pintados,
 *  param tro bit indica cuales bits del registro estan encendidos
 *  este metodo converte el numero del registro y el bit en un numero
 *  que representa la coordenada la coordenada absoluta a de 1 a 192 ya convertia a dds absoluto.
 * */
unsigned char get_Coord_Num(unsigned char xn,unsigned char bit){
const unsigned char byte[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

   switch(xn){//rx
	   case 0:switch(byte[bit]){//b
		         case 1:return 192;break;
		         case 2:return 1;break;
		         case 4:return 2;break;
		         case 8:return 3;break;
		         case 16:return 4;break;
		         case 32:return 5;break;
		         case 64:return 6;break;
		         case 128:return 7;break;
		         default:break;}
	          break;//fin de 0
	   case 1:switch(byte[bit]){
		         case 1:return 8;break;
		         case 2:return 9;break;
		         case 4:return 10;break;
		         case 8:return 11;break;
		         case 16:return 12;break;
		         case 32:return 13;break;
		         case 64:return 14;break;
		         case 128:return 15;break;
		         default:break;}
		      break;//fin de 1
       case 2:switch(byte[bit]){
    	         case 1: return 16;break;
    	         case 2: return 17;break;
    	         case 4: return 18;break;
    	         case 8: return 19;break;
    	 		 case 16:return 20;break;
    	 		 case 32:return 21;break;
    	 		 case 64:return 22;break;
    	 		 case 128:return 23;break;
    	 		 default:break;}
    	      break;//fin de 2
    	   case 3:switch(byte[bit]){
    	       	 case 1: return 24;break;
    	       	 case 2:return 25;break;
    	       	 case 4:return 26;break;
    	       	 case 8:return 27;break;
    	       	 case 16:return 28;break;
    	       	 case 32:return 29;break;
    	       	 case 64:return 30;break;
    	       	 case 128:return 31;break;
    	       	 default:break;}
    	       break;// fin de 3
    	  case 4:switch(byte[bit]){
    		     case 1: return 32;break;
    		     case 2:return 33;break;
    		     case 4:return 34;break;
    		     case 8:return 35;break;
    		     case 16:return 36;break;
    		     case 32:return 37;break;
    		     case 64:return 38;break;
    		     case 128:return 39;break;
    		     default:break;}
    		   break;// fin de 4      
    	  case 5:switch(byte[bit]){
    		     case 1: return 40;break;
    		     case 2:return 41;break;
    		     case 4:return 42;break;
    		     case 8:return 43;break;
    		     case 16:return 44;break;
    		     case 32:return 45;break;
    		     case 64:return 46;break;
    		     case 128:return 47;break;
    		     default:break;}
    		     break;// fin de 5
         case 6:switch(byte[bit]){
    		     case 1: return 48;break;
    		     case 2:return 49;break;
    		     case 4:return 50;break;
    		     case 8:return 51;break;
    		     case 16:return 52;break;
    		     case 32:return 53;break;
    		     case 64:return 54;break;
    		     case 128:return 55;break;
    		     default:break;}
    		     break;// fin de 6 		     
         case 7:switch(byte[bit]){
        	     case 1: return 56;break;
        	     case 2:return 57;break;
        	     case 4:return 58;break;
        	     case 8:return 59;break;
        	     case 16:return 60;break;
        	     case 32:return 61;break;
        	     case 64:return 62;break;
        	     case 128:return 63;break;
        	     default:break;}
        	     break;// fin de 7 	
         case 8:switch(byte[bit]){
        	    case 1: return 64;break;
        	    case 2:return 65;break;
        	    case 4:return 66;break;
        	    case 8:return 67;break;
        	    case 16:return 68;break;
        	    case 32:return 69;break;
        	    case 64:return 70;break;
        	    case 128:return 71;break;
        	    default:break;}
        	    break;// fin de 8 		     
        case 9:switch(byte[bit]){
        	    case 1: return 72;break;
        	    case 2:return 73;break;
        	    case 4:return 74;break;
        	    case 8:return 75;break;
        	    case 16:return 76;break;
        	    case 32:return 77;break;
        	    case 64:return 78;break;
        	    case 128:return 79;break;
        	    default:break;}
        	    break;// fin de 9 	
       case 10:switch(byte[bit]){
        	    case 1: return 80;break;
        	    case 2:return 81;break;
        	    case 4:return 82;break;
        	    case 8:return 83;break;
        	    case 16:return 84;break;
        	    case 32:return 85;break;
        	    case 64:return 86;break;
        	    case 128:return 87;break;
        	    default:break;}
        	    break;// fin de 10 		     
       case 11:switch(byte[bit]){
        	    case 1: return 88;break;
        	    case 2:return 89;break;
        	    case 4:return 90;break;
        	    case 8:return 91;break;
        	    case 16:return 92;break;
        	    case 32:return 93;break;
        	    case 64:return 94;break;
        	    case 128:return 95;break;
        	    default:break;}
        	    break;// fin de 11	
       case 12:switch(byte[bit]){
    	        case 1: return 96;break;
    	        case 2:return 97;break;
    	        case 4:return 98;break;
    	        case 8:return 99;break;
    	        case 16:return 100;break;
    	        case 32:return 101;break;
    	        case 64:return 102;break;
    	        case 128:return 103;break;
    	        default:break;}
    	        break;// fin de 12		     
        case 13:switch(byte[bit]){
    	        case 1: return 104;break;
    	        case 2:return 105;break;
    	        case 4:return 106;break;
    	        case 8:return 107;break;
    	        case 16:return 108;break;
    	        case 32:return 109;break;
    	        case 64:return 110;break;
    	        case 128:return 111;break;
    	        default:break;}
    	        break;// fin de 13 	
       case 14:switch(byte[bit]){
    	        case 1: return 112;break;
    	        case 2:return 113;break;
    	        case 4:return 114;break;
    	        case 8:return 115;break;
    	        case 16:return 116;break;
    	        case 32:return 117;break;
    	        case 64:return 118;break;
    	        case 128:return 119;break;
    	        default:break;}
    	        break;// fin de 14 		     
       case 15:switch(byte[bit]){
    	        case 1: return 120;break;
    	        case 2:return 121;break;
    	        case 4:return 122;break;
    	        case 8:return 123;break;
    	        case 16:return 124;break;
    	        case 32:return 125;break;
    	        case 64:return 126;break;
    	        case 128:return 127;break;
    	        default:break;}
    	        break;// fin de 15       	        	        	     	       	        	     		            	     		     
       case 16:switch(byte[bit]){
    	        case 1: return 128;break;
    	        case 2:return 129;break;
    	        case 4:return 130;break;
    	        case 8:return 131;break;
    	        case 16:return 132;break;
    	        case 32:return 133;break;
    	        case 64:return 134;break;
    	        case 128:return 135;break;
    	        default:break;}
    	        break;// fin de 16	
       case 17:switch(byte[bit]){
    	        case 1: return 136;break;
    	       	case 2:return 137;break;
    	       	case 4:return 138;break;
    	       	case 8:return 139;break;
    	        case 16:return 140;break;
    	        case 32:return 141;break;
    	       	case 64:return 142;break;
    	       	case 128:return 143;break;
    	        default:break;}
    	       	break;// fin de 17		     
    	case 18:switch(byte[bit]){
    	       	case 1: return 144;break;
    	       	case 2:return 145;break;
    	       	case 4:return 146;break;
    	       	case 8:return 147;break;
    	        case 16:return 148;break;
    	       	case 32:return 149;break;
    	       	case 64:return 150;break;
    	       	case 128:return 151;break;
    	       	default:break;}
    	        break;// fin de 18 	
       case 19:switch(byte[bit]){
    	        case 1: return 152;break;
    	        case 2:return 153;break;
    	       	case 4:return 154;break;
    	        case 8:return 155;break;
    	       	case 16:return 156;break;
    	       	case 32:return 157;break;
    	       	case 64:return 158;break;
    	       	case 128:return 159;break;
    	       	default:break;}
    	       	break;// fin de 19 		     
       case 20:switch(byte[bit]){
    	        case 1: return 160;break;
    	       	case 2:return 161;break;
    	        case 4:return 162;break;
    	       	case 8:return 163;break;
    	        case 16:return 164;break;
    	       	case 32:return 165;break;
    	        case 64:return 166;break;
    	       	case 128:return 167;break;
    	       	default:break;}
    	       	break;// fin de 20       	        	        	     	       	        	     		            	     		     
       case 21:switch(byte[bit]){
    	        case 1: return 168;break;
    	        case 2:return 169;break;
    	        case 4:return 170;break;
    	        case 8:return 171;break;
    	        case 16:return 172;break;
    	        case 32:return 173;break;
    	        case 64:return 174;break;
    	        case 128:return 175;break;
    	        default:break;}
    	        break;// fin de 21      	     		       		       		     
       case 22:switch(byte[bit]){
    	       	case 1: return 176;break;
    	       	case 2:return 177;break;
    	       	case 4:return 178;break;
    	       	case 8:return 179;break;
    	        case 16:return 180;break;
    	       	case 32:return 181;break;
    	       	case 64:return 182;break;
    	       	case 128:return 183;break;
    	      	default:break;}
       	       	break;// fin de 22       	        	        	     	       	        	     		            	     		     
       case 23:switch(byte[bit]){
    	       	case 1: return 184;break;
    	       	case 2:return 185;break;
    	       	case 4:return 186;break;
    	       	case 8:return 187;break;
    	       	case 16:return 188;break;
    	        case 32:return 189;break;
    	        case 64:return 190;break;
    	       	case 128:return 191;break;
    	       	default:break;}
    	       	break;// fin de 23          		          		     	      
    	   default:break;}      
    	
       
       		       	      
	 	
}//fin de get_Coord_Num-----------------------------------








//RETAZOS NO ESCRIBIR MAS ALLA DE AQUI--------------------------------------------------
////FUNCION DEPRECATED--------------------------------------------------
////funcion tangente------------En grados-------------------------------------------------------------
//float tangente(unsigned short int a){
//	
//	  switch(a){
//	    case 0:return 0;break;
//	    case 1:return 0.01745506492821758576512889521973;break;
//	    case 2:return 0.03492076949174773050040262577373;break;
//	    case 3:return 0.05240777928304120403880582447398;break;
//	    case 4:return 0.06992681194351041366692106032318;break;
//	    case 5:return 0.08748866352592400522201866943496;break;
//	    case 6:return 0.10510423526567646251150238013988;break;
//	  	case 7:return 0.12278456090290459113423113605286;break;
//	  	case 8:return 0.14054083470239144683811769343281;break;
//	  	case 9:return 0.15838444032453629383888309269437;break;
//	  	case 10:return 0.17632698070846497347109038686862 ;break;
//	    case 11:return 0.19438030913771848424319422497682;break;
//	    case 12:return 0.21255656167002212525959166057008;break;
//	    case 13:return 0.23086819112556311174814561347445;break;
//	    case 14:return 0.24932800284318069162403993780486;break;
//	  	case 15:return 0.26794919243112270647255365849413;break;
//	  	case 16:return 0.28674538575880794004275806273267;break;
//	  	case 17:return 0.30573068145866035573454195899655;break;
//	    case 18:return 0.32492;break;	
//	    case 19:return 0.34433;break;
//	    case 20:return 0.36397;break;
//	    case 21:return 0.38386;break; 		
//	    case 22:return 0.40403;break;
//	  	case 23:return 0.42447;break;
//	   	case 24:return 0.44523;break;
//	   	case 25:return 0.46631;break;
//	    case 26:return 0.48773;break;
//	   	case 27:return 0.50953;break;
//		case 28:return 0.53171;break;
//		case 29:return 0.55431;break;
//		case 30:return 0.57735;break;
//		case 31:return 0.60086;break;
//		case 32:return 0.62487;break;
//		case 33:return 0.64941;break;
//		case 34:return 0.67451;break;
//		case 35:return 0.70021;break;
//		case 36:return 0.72654;break;
//		case 37:return 0.75355;break;
//		case 38:return 0.78129;break;
//		case 39:return 0.80978;break;
//        case 40:return 0.8391;break;
//		case 41:return 0.86929;break;
//        case 42:return 0.9004;break;
//		case 43:return 0.93252;break;
//		case 44:return 0.96569;break;
//     	case 45:return 1;break;
//    	case 46:return 1.03553;break;
//		case 47:return 1.07237;break;
//		case 48:return 1.11061;break;
//    	case 49:return 1.15037;break;
//		case 50:return 1.19175;break;
//		case 51:return 1.2349;break;
//		case 52:return 1.27994;break;
//    	case 53:return 1.32704;break;
//		case 54:return 1.37638;break;
//		case 55:return 1.42815;break;
//    	case 56:return 1.48256;break;
//		case 57:return 1.53986;break;
//	    case 58:return 1.60033;break;
//	    case 59:return 1.66428;break;
//	   	case 60:return 1.73205;break;
//		case 61:return 1.80405;break;
//		case 62:return 1.88073;break;
//		case 63:return 1.96261;break;
//		case 64:return 2.0503;break;
//		case 65:return 2.14451;break;
//		case 66:return 2.24604;break;
//		case 67:return 2.35585;break;
//		case 68:return 2.47509;break;
//		case 69:return 2.60509;break;
//		case 70:return 2.74748;break;
//		case 71:return 2.90421;break;
//		case 72:return 3.07768;break;
//		case 73:return 3.27085;break;
//		case 74:return 3.48741;break;
//		case 75:return 3.73205;break;
//		case 76:return 4.01078;break;
//		case 77:return 4.33148;break;
//		case 78:return 4.70463;break;
//		case 79:return 5.14455;break;
//		case 80:return 5.67128;break;
//		case 81:return 6.31375;break;
//		case 82:return 7.11537;break;
//		case 83:return 8.14435;break;
//		case 84:return 9.51436;break;
//		case 85:return 11.43005;break;
//		case 86:return 14.30067;break;
//		case 87:return 19.08114;break;
//		case 88:return 28.63625;break;
//		case 89:return 57.28996;break;
//		case 90:return 65535;break;
//		case 91:return -57.28996;break;
//		case 92:return -28.63625;break;
//		case 93:return -19.08114;break;
//		case 94:return -14.30067;break;
//		case 95:return -11.43005;break;
//		case 96:return -9.51436;break;
//		case 97:return -8.14435;break;
//		case 98:return -7.11537;break;
//		case 99:return -6.31375;break;
//		case 100:return -5.67128;break;
//		case 101:return -5.14455;break;
//		case 102:return -4.70463;break;
//		case 103:return -4.33148;break;
//		case 104:return -4.01078;break;
//		case 105:return -3.73205;break;
//		case 106:return -3.48741;break;
//		case 107:return -3.27085;break;
//		case 108:return -3.07768;break;
//		case 109:return -2.90421;break;
//		case 110:return -2.74748;break;
//		case 111:return -2.60509;break;
//		case 112:return -2.47509;break;
//		case 113:return -2.35585;break;
//		case 114:return -2.24604;break;
//		case 115:return -2.14451;break;
//		case 116:return -2.0503;break;
//		case 117:return -1.96261;break;
//		case 118:return -1.88073;break;
//		case 119:return -1.80405;break;
//		case 120:return -1.73205;break;
//		case 121:return -1.66428;break;
//		case 122:return -1.60033;break;
//		case 123:return -1.53986;break;
//		case 124:return -1.48256;break;
//		case 125:return -1.42815;break;
//		case 126:return -1.37638;break;
//		case 127:return -1.32704;break;
//		case 128:return -1.27994;break;
//		case 129:return -1.2349;break;
//		case 130:return -1.19175;break;
//		case 131:return -1.15037;break;
//		case 132:return -1.11061;break;
//		case 133:return -1.07237;break;
//		case 134:return -1.03553;break;
//		case 135:return -1;break;
//		case 136:return -0.96569;break;
//		case 137:return -0.93252;break;
//		case 138:return -0.9004;break;
//		case 139:return -0.86929;break;
//		case 140:return -0.8391;break;
//		case 141:return -0.80978;break;
//		case 142:return -0.78129;break;
//		case 143:return -0.75355;break;
//		case 144:return -0.72654;break;
//		case 145:return -0.70021;break;
//		case 146:return -0.67451;break;
//		case 147:return -0.64941;break;
//		case 148:return -0.62487;break;
//		case 149:return -0.60086;break;
//		case 150:return -0.57735;break;
//		case 151:return -0.55431;break;
//		case 152:return -0.53171;break;
//		case 153:return -0.50953;break;
//		case 154:return -0.48773;break;
//		case 155:return -0.46631;break;
//		case 156:return -0.44523;break;
//		case 157:return -0.42447;break;
//		case 158:return -0.40403;break;
//		case 159:return -0.38386;break;
//		case 160:return -0.36397;break;
//		case 161:return -0.34433;break;
//		case 162:return -0.32492;break;
//		case 163:return -0.30573;break;
//		case 164:return -0.28675;break;
//		case 165:return -0.26795;break;
//		case 166:return -0.24933;break;
//		case 167:return -0.23087;break;
//		case 168:return -0.21256;break;
//		case 169:return -0.19438;break;
//		case 170:return -0.17633;break;
//		case 171:return -0.15838;break;
//		case 172:return -0.14054;break;
//		case 173:return -0.12278;break;
//		case 174:return -0.1051;break;
//		case 175:return -0.08749;break;
//		case 176:return -0.06993;break;
//		case 177:return -0.05241;break;
//		case 178:return -0.03492;break;
//		case 179:return -0.01746;break;
//		case 180:return 0;break;
//		default:return 0;break;
//	  }//finde switch-----------------------------------------------                                                                                                                                                                                                                                                                                                                                                                                                                                                    	
//}//fin de la tabla de tangente de angulos sagrados maestros ---------------------------------------------
//
//
////FUNCTION  DEPRECATED
//float cosinous(unsigned char a){//debug ,muchos cases, no se selsccionan.
//   
//   if(a<0)
//	 a=360+a;
//   switch(a){	
//	case 0:return 1;break;
//	case 1:return 0.999848;break;
//    case 2:return 0.999391;break;
//	case 3:return 0.99863;break;
//	case 4:return 0.997564;break;
//	case 5:return 0.996195;break;
//	case 6:return 0.994522;break;
//	case 7:return 0.992546;break;
//	case 8:return 0.990268;break;
//	case 9:return 0.987688;break;
//	case 10:return 0.984808;break;
//	case 11:return 0.981627;break;
//	case 12:return 0.978148;break;
//	case 13:return 0.97437;break;
//	case 14:return 0.970296;break;
//	case 15:return 0.965926;break;
//	case 16:return 0.961262;break;
//	case 17:return 0.956305;break;
//	case 18:return 0.951057;break;
//	case 19:return 0.945519;break;
//	case 20:return 0.939693;break;
//	case 21:return 0.93358;break;
//	case 22:return 0.927184;break;
//	case 23:return 0.920505;break;
//	case 24:return 0.913545;break;
//	case 25:return 0.906308;break;
//	case 26:return 0.898794;break;
//	case 27:return 0.891007;break;
//	case 28:return 0.882948;break;
//	case 29:return 0.87462;break;
//	case 30:return 0.866025;break;
//	case 31:return 0.857167;break;
//	case 32:return 0.848048;break;
//	case 33:return 0.838671;break;
//	case 34:return 0.829038;break;
//	case 35:return 0.819152;break;
//	case 36:return 0.809017;break;
//	case 37:return 0.798636;break;
//	case 38:return 0.788011;break;
//	case 39:return 0.777146;break;
//	case 40:return 0.766044;break;
//	case 41:return 0.75471;break;
//	case 42:return 0.743145;break;
//	case 43:return 0.731354;break;
//	case 44:return 0.71934;break;
//	case 45:return 0.707107;break;
//	case 46:return 0.694658;break;
//	case 47:return 0.681998;break;
//	case 48:return 0.669131;break;
//	case 49:return 0.656059;break;
//	case 50:return 0.642788;break;
//	case 51:return 0.62932;break;
//	case 52:return 0.615661;break;
//	case 53:return 0.601815;break;
//	case 54:return 0.587785;break;
//	case 55:return 0.573576;break;
//	case 56:return 0.559193;break;
//	case 57:return 0.544639;break;
//	case 58:return 0.529919;break;
//	case 59:return 0.515038;break;
//	case 60:return 0.5;break;
//	case 61:return 0.48481;break;
//	case 62:return 0.469472;break;
//	case 63:return 0.45399;break;
//	case 64:return 0.438371;break;
//	case 65:return 0.422618;break;
//	case 66:return 0.406737;break;
//	case 67:return 0.390731;break;
//	case 68:return 0.374607;break;
//	case 69:return 0.358368;break;
//	case 70:return 0.34202;break;
//	case 71:return 0.325568;break;
//	case 72:return 0.309017;break;
//	case 73:return 0.292372;break;
//	case 74:return 0.275637;break;
//	case 75:return 0.258819;break;
//	case 76:return 0.241922;break;
//	case 77:return 0.224951;break;
//	case 78:return 0.207912;break;
//	case 79:return 0.190809;break;
//	case 80:return 0.173648;break;
//	case 81:return 0.156434;break;
//	case 82:return 0.139173;break;
//	case 83:return 0.121869;break;
//	case 84:return 0.104528;break;
//	case 85:return 0.087156;break;
//	case 86:return 0.069756;break;
//	case 87:return 0.052336;break;
//	case 88:return 0.034899;break;
//	case 89:return 0.017452;break;
//	case 90:return 0;break;
//	case 91:return -0.017452;break;
//	case 92:return -0.034899;break;
//	case 93:return -0.052336;break;
//	case 94:return -0.069756;break;
//	case 95:return -0.087156;break;
//	case 96:return -0.104528;break;
//	case 97:return -0.121869;break;
//	case 98:return -0.139173;break;
//	case 99:return -0.156434;break;
//	case 100:return -0.173648;break;
//	case 101:return -0.190809;break;
//	case 102:return -0.207912;break;
//	case 103:return -0.224951;break;
//	case 104:return -0.241922;break;
//	case 105:return -0.258819;break;
//	case 106:return -0.275637;break;
//	case 107:return -0.292372;break;
//	case 108:return -0.309017;break;
//	case 109:return -0.325568;break;
//	case 110:return -0.34202;break;
//	case 111:return -0.358368;break;
//	case 112:return -0.374607;break;
//	case 113:return -0.390731;break;
//	case 114:return -0.406737;break;
//	case 115:return -0.422618;break;
//	case 116:return -0.438371;break;
//	case 117:return -0.45399;break;
//	case 118:return -0.469472;break;
//	case 119:return -0.48481;break;
//	case 120:return -0.5;break;
//	case 121:return -0.515038;break;
//	case 122:return -0.529919;break;
//	case 123:return -0.544639;break;
//	case 124:return -0.559193;break;
//	case 125:return -0.573576;break;
//	case 126:return -0.587785;break;
//	case 127:return -0.601815;break;
//	case 128:return -0.615661;break;
//	case 129:return -0.62932;break;
//	case 130:return -0.642788;break;
//	case 131:return -0.656059;break;
//	case 132:return -0.669131;break;
//	case 133:return -0.681998;break;
//	case 134:return -0.694658;break;
//	case 135:return -0.707107;break;
//	case 136:return -0.71934;break;
//	case 137:return -0.731354;break;
//	case 138:return -0.743145;break;
//	case 139:return -0.75471;break;
//	case 140:return -0.766044;break;
//	case 141:return -0.777146;break;
//	case 142:return -0.788011;break;
//	case 143:return -0.798636;break;
//	case 144:return -0.809017;break;
//	case 145:return -0.819152;break;
//	case 146:return -0.829038;break;
//	case 147:return -0.838671;break;
//	case 148:return -0.848048;break;
//	case 149:return -0.857167;break;
//	case 150:return -0.866025;break;
//	case 151:return -0.87462;break;
//	case 152:return -0.882948;break;
//	case 153:return -0.891007;break;
//	case 154:return -0.898794;break;
//	case 155:return -0.906308;break;
//	case 156:return -0.913545;break;
//	case 157:return -0.920505;break;
//	case 158:return -0.927184;break;
//	case 159:return -0.93358;break;
//	case 160:return -0.939693;break;
//	case 161:return -0.945519;break;
//	case 162:return -0.951057;break;
//	case 163:return -0.956305;break;
//	case 164:return -0.961262;break;
//	case 165:return -0.965926;break;
//	case 166:return -0.970296;break;
//	case 167:return -0.97437;break;
//	case 168:return -0.978148;break;
//	case 169:return -0.981627;break;
//	case 170:return -0.984808;break;
//	case 171:return -0.987688;break;
//	case 172:return -0.990268;break;
//	case 173:return -0.992546;break;
//	case 174:return -0.994522;break;
//	case 175:return -0.996195;break;
//	case 176:return -0.997564;break;
//	case 177:return -0.99863;break;
//	case 178:return -0.999391;break;
//	case 179:return -0.999848;break;
//	case 180:return -1;break;
//	case 181:return -0.999848;break;
//	case 182:return -0.999391;break;
//	case 183:return -0.99863;break;
//	case 184:return -0.997564;break;
//	case 185:return -0.996195;break;
//	case 186:return -0.994522;break;
//	case 187:return -0.992546;break;
//	case 188:return -0.990268;break;
//	case 189:return -0.987688;break;
//	case 190:return -0.984808;break;
//	case 191:return -0.981627;break;
//	case 192:return -0.978148;break;
//	case 193:return -0.97437;break;
//	case 194:return -0.970296;break;
//	case 195:return -0.965926;break;
//	case 196:return -0.961262;break;
//	case 197:return -0.956305;break;
//	case 198:return -0.951057;break;
//	case 199:return -0.945519;break;
//	case 200:return -0.939693;break;
//	case 201:return -0.93358;break;
//	case 202:return -0.927184;break;
//	case 203:return -0.920505;break;
//	case 204:return -0.913545;break;
//	case 205:return -0.906308;break;
//	case 206:return -0.898794;break;
//	case 207:return -0.891007;break;
//	case 208:return -0.882948;break;
//	case 209:return -0.87462;break;
//	case 210:return -0.866025;break;
//	case 211:return -0.857167;break;
//	case 212:return -0.848048;break;
//	case 213:return -0.838671;break;
//	case 214:return -0.829038;break;
//	case 215:return -0.819152;break;
//	case 216:return -0.809017;break;
//	case 217:return -0.798636;break;
//	case 218:return -0.788011;break;
//	case 219:return -0.777146;break;
//	case 220:return -0.766044;break;
//	case 221:return -0.75471;break;
//	case 222:return -0.743145;break;
//	case 223:return -0.731354;break;
//	case 224:return -0.71934;break;
//	case 225:return -0.707107;break;
//	case 226:return -0.694658;break;
//	case 227:return -0.681998;break;
//	case 228:return -0.669131;break;
//	case 229:return -0.656059;break;
//	case 230:return -0.642788;break;
//	case 231:return -0.62932;break;
//	case 232:return -0.615661;break;
//	case 233:return -0.601815;break;
//	case 234:return -0.587785;break;
//	case 235:return -0.573576;break;
//	case 236:return -0.559193;break;
//	case 237:return -0.544639;break;
//	case 238:return -0.529919;break;
//	case 239:return -0.515038;break;
//	case 240:return -0.5;break;
//	case 241:return -0.48481;break;
//	case 242:return -0.469472;break;
//	case 243:return -0.45399;break;
//	case 244:return -0.438371;break;
//	case 245:return -0.422618;break;
//	case 246:return -0.406737;break;
//	case 247:return -0.390731;break;
//	case 248:return -0.374607;break;
//	case 249:return -0.358368;break;
//	case 250:return -0.34202;break;
//	case 251:return -0.325568;break;
//	case 252:return -0.309017;break;
//	case 253:return -0.292372;break;
//	case 254:return -0.275637;break;
//	case 255:return -0.258819;break;
//	case 256:return -0.241922;break;
//	case 257:return -0.224951;break;
//	case 258:return -0.207912;break;
//	case 259:return -0.190809;break;
//	case 260:return -0.173648;break;
//	case 261:return -0.156434;break;
//	case 262:return -0.139173;break;
//	case 263:return -0.121869;break;
//	case 264:return -0.104528;break;
//	case 265:return -0.087156;break;
//	case 266:return -0.069756;break;
//	case 267:return -0.052336;break;
//	case 268:return -0.034899;break;
//	case 269:return -0.017452;break;
//	case 270:return -0;break;
//	case 271:return 0.017452;break;
//	case 272:return 0.034899;break;
//	case 273:return 0.052336;break;
//	case 274:return 0.069756;break;
//	case 275:return 0.087156;break;
//	case 276:return 0.104528;break;
//	case 277:return 0.121869;break;
//	case 278:return 0.139173;break;
//	case 279:return 0.156434;break;
//	case 280:return 0.173648;break;
//	case 281:return 0.190809;break;
//	case 282:return 0.207912;break;
//	case 283:return 0.224951;break;
//	case 284:return 0.241922;break;
//	case 285:return 0.258819;break;
//	case 286:return 0.275637;break;
//	case 287:return 0.292372;break;
//	case 288:return 0.309017;break;
//	case 289:return 0.325568;break;
//	case 290:return 0.34202;break;
//	case 291:return 0.358368;break;
//	case 292:return 0.374607;break;
//	case 293:return 0.390731;break;
//	case 294:return 0.406737;break;
//	case 295:return 0.422618;break;
//	case 296:return 0.438371;break;
//	case 297:return 0.45399;break;
//	case 298:return 0.469472;break;
//	case 299:return 0.48481;break;
//	case 300:return 0.5;break;
//	case 301:return 0.515038;break;
//	case 302:return 0.529919;break;
//	case 303:return 0.544639;break;
//	case 304:return 0.559193;break;
//	case 305:return 0.573576;break;
//	case 306:return 0.587785;break;
//	case 307:return 0.601815;break;
//	case 308:return 0.615661;break;
//	case 309:return 0.62932;break;
//	case 310:return 0.642788;break;
//	case 311:return 0.656059;break;
//	case 312:return 0.669131;break;
//	case 313:return 0.681998;break;
//	case 314:return 0.694658;break;
//	case 315:return 0.707107;break;
//	case 316:return 0.71934;break;
//	case 317:return 0.731354;break;
//	case 318:return 0.743145;break;
//	case 319:return 0.75471;break;
//	case 320:return 0.766044;break;
//	case 321:return 0.777146;break;
//	case 322:return 0.788011;break;
//	case 323:return 0.798636;break;
//	case 324:return 0.809017;break;
//	case 325:return 0.819152;break;
//	case 326:return 0.829038;break;
//	case 327:return 0.838671;break;
//	case 328:return 0.848048;break;
//	case 329:return 0.857167;break;
//	case 330:return 0.866025;break;
//	case 331:return 0.87462;break;
//	case 332:return 0.882948;break;
//	case 333:return 0.891007;break;
//	case 334:return 0.898794;break;
//	case 335:return 0.906308;break;
//	case 336:return 0.913545;break;
//	case 337:return 0.920505;break;
//	case 338:return 0.927184;break;
//	case 339:return 0.93358;break;
//	case 340:return 0.939693;break;
//	case 341:return 0.945519;break;
//	case 342:return 0.951057;break;
//	case 343:return 0.956305;break;
//	case 344:return 0.961262;break;
//	case 345:return 0.965926;break;
//	case 346:return 0.970296;break;
//	case 347:return 0.97437;break;
//	case 348:return 0.978148;break;
//	case 349:return 0.981627;break;
//	case 350:return 0.984808;break;
//	case 351:return 0.987688;break;
//	case 352:return 0.990268;break;
//	case 353:return 0.992546;break;
//	case 354:return 0.994522;break;
//	case 355:return 0.996195;break;
//	case 356:return 0.997564;break;
//	case 357:return 0.99863;break;
//	case 358:return 0.999391;break;
//	case 359:return 0.999848;break;
//	case 360:return 1;break;
//	default:return 0;break;}//fin switch---------------------
//}//fin de coseno--------------------------------------------------------------------
//
//
//float sinous(unsigned char a){
//  if(a<0)
//	  a=360+a;
//  switch(a){
//    case 0:return 0;break;
//    case 1:return 0.017452;break;
//    case 2:return 0.034899;break;
//    case 3:return 0.052336;break;
//    case 4:return 0.069756;break;
//    case 5:return 0.087156;break;
//    case 6:return 0.104528;break;
//    case 7:return 0.121869;break;
//    case 8:return 0.139173;break;
//    case 9:return 0.156434;break;
//    case 10:return 0.173648;break;
//    case 11:return 0.190809;break;
//    case 12:return 0.207912;break;
//    case 13:return 0.224951;break;
//    case 14:return 0.241922;break;
//    case 15:return 0.258819;break;
//    case 16:return 0.275637;break;
//    case 17:return 0.292372;break;
//    case 18:return 0.309017;break;
//    case 19:return 0.325568;break;
//    case 20:return 0.34202;break;
//    case 21:return 0.358368;break;
//    case 22:return 0.374607;break;
//    case 23:return 0.390731;break;
//    case 24:return 0.406737;break;
//    case 25:return 0.422618;break;
//    case 26:return 0.438371;break;
//    case 27:return 0.45399;break;
//    case 28:return 0.469472;break;
//    case 29:return 0.48481;break;
//    case 30:return 0.5;break;
//    case 31:return 0.515038;break;
//    case 32:return 0.529919;break;
//    case 33:return 0.544639;break;
//    case 34:return 0.559193;break;
//    case 35:return 0.573576;break;
//    case 36:return 0.587785;break;
//    case 37:return 0.601815;break;
//    case 38:return 0.615661;break;
//    case 39:return 0.62932;break;
//    case 40:return 0.642788;break;
//    case 41:return 0.656059;break;
//    case 42:return 0.669131;break;
//    case 43:return 0.681998;break;
//    case 44:return 0.694658;break;
//    case 45:return 0.707107;break;
//    case 46:return 0.71934;break;
//    case 47:return 0.731354;break;
//    case 48:return 0.743145;break;
//    case 49:return 0.75471;break;
//    case 50:return 0.766044;break;
//    case 51:return 0.777146;break;
//    case 52:return 0.788011;break;
//    case 53:return 0.798636;break;
//    case 54:return 0.809017;break;
//    case 55:return 0.819152;break;
//    case 56:return 0.829038;break;
//    case 57:return 0.838671;break;
//    case 58:return 0.848048;break;
//    case 59:return 0.857167;break;
//    case 60:return 0.866025;break;
//    case 61:return 0.87462;break;
//    case 62:return 0.882948;break;
//    case 63:return 0.891007;break;
//    case 64:return 0.898794;break;
//    case 65:return 0.906308;break;
//    case 66:return 0.913545;break;
//    case 67:return 0.920505;break;
//    case 68:return 0.927184;break;
//    case 69:return 0.93358;break;
//    case 70:return 0.939693;break;
//    case 71:return 0.945519;break;
//    case 72:return 0.951057;break;
//    case 73:return 0.956305;break;
//    case 74:return 0.961262;break;
//    case 75:return 0.965926;break;
//    case 76:return 0.970296;break;
//    case 77:return 0.97437;break;
//    case 78:return 0.978148;break;
//    case 79:return 0.981627;break;
//    case 80:return 0.984808;break;
//    case 81:return 0.987688;break;
//    case 82:return 0.990268;break;
//    case 83:return 0.992546;break;
//    case 84:return 0.994522;break;
//    case 85:return 0.996195;break;
//    case 86:return 0.997564;break;
//    case 87:return 0.99863;break;
//    case 88:return 0.999391;break;
//    case 89:return 0.999848;break;
//    case 90:return 1;break;
//    case 91:return 0.999848;break;
//    case 92:return 0.999391;break;
//    case 93:return 0.99863;break;
//    case 94:return 0.997564;break;
//    case 95:return 0.996195;break;
//    case 96:return 0.994522;break;
//    case 97:return 0.992546;break;
//    case 98:return 0.990268;break;
//    case 99:return 0.987688;break;
//    case 100:return 0.984808;break;
//    case 101:return 0.981627;break;
//    case 102:return 0.978148;break;
//    case 103:return 0.97437;break;
//    case 104:return 0.970296;break;
//    case 105:return 0.965926;break;
//    case 106:return 0.961262;break;
//    case 107:return 0.956305;break;
//    case 108:return 0.951057;break;
//    case 109:return 0.945519;break;
//    case 110:return 0.939693;break;
//    case 111:return 0.93358;break;
//    case 112:return 0.927184;break;
//    case 113:return 0.920505;break;
//    case 114:return 0.913545;break;
//    case 115:return 0.906308;break;
//    case 116:return 0.898794;break;
//    case 117:return 0.891007;break;
//    case 118:return 0.882948;break;
//    case 119:return 0.87462;break;
//    case 120:return 0.866025;break;
//    case 121:return 0.857167;break;
//    case 122:return 0.848048;break;
//    case 123:return 0.838671;break;
//    case 124:return 0.829038;break;
//    case 125:return 0.819152;break;
//    case 126:return 0.809017;break;
//    case 127:return 0.798636;break;
//    case 128:return 0.788011;break;
//    case 129:return 0.777146;break;
//    case 130:return 0.766044;break;
//    case 131:return 0.75471;break;
//    case 132:return 0.743145;break;
//    case 133:return 0.731354;break;
//    case 134:return 0.71934;break;
//    case 135:return 0.707107;break;
//    case 136:return 0.694658;break;
//    case 137:return 0.681998;break;
//    case 138:return 0.669131;break;
//    case 139:return 0.656059;break;
//    case 140:return 0.642788;break;
//    case 141:return 0.62932;break;
//    case 142:return 0.615661;break;
//    case 143:return 0.601815;break;
//    case 144:return 0.587785;break;
//    case 145:return 0.573576;break;
//    case 146:return 0.559193;break;
//    case 147:return 0.544639;break;
//    case 148:return 0.529919;break;
//    case 149:return 0.515038;break;
//    case 150:return 0.5;break;
//    case 151:return 0.48481;break;
//    case 152:return 0.469472;break;
//    case 153:return 0.45399;break;
//    case 154:return 0.438371;break;
//    case 155:return 0.422618;break;
//    case 156:return 0.406737;break;
//    case 157:return 0.390731;break;
//    case 158:return 0.374607;break;
//    case 159:return 0.358368;break;
//    case 160:return 0.34202;break;
//    case 161:return 0.325568;break;
//    case 162:return 0.309017;break;
//    case 163:return 0.292372;break;
//    case 164:return 0.275637;break;
//    case 165:return 0.258819;break;
//    case 166:return 0.241922;break;
//    case 167:return 0.224951;break;
//    case 168:return 0.207912;break;
//    case 169:return 0.190809;break;
//    case 170:return 0.173648;break;
//    case 171:return 0.156434;break;
//    case 172:return 0.139173;break;
//    case 173:return 0.121869;break;
//    case 174:return 0.104528;break;
//    case 175:return 0.087156;break;
//    case 176:return 0.069756;break;
//    case 177:return 0.052336;break;
//    case 178:return 0.034899;break;
//    case 179:return 0.017452;break;
//    case 180:return 0;break;
//    case 181:return -0.017452;break;
//    case 182:return -0.034899;break;
//    case 183:return -0.052336;break;
//    case 184:return -0.069756;break;
//    case 185:return -0.087156;break;
//    case 186:return -0.104528;break;
//    case 187:return -0.121869;break;
//    case 188:return -0.139173;break;
//    case 189:return -0.156434;break;
//    case 190:return -0.173648;break;
//    case 191:return -0.190809;break;
//    case 192:return -0.207912;break;
//    case 193:return -0.224951;break;
//    case 194:return -0.241922;break;
//    case 195:return -0.258819;break;
//    case 196:return -0.275637;break;
//    case 197:return -0.292372;break;
//    case 198:return -0.309017;break;
//    case 199:return -0.325568;break;
//    case 200:return -0.34202;break;
//    case 201:return -0.358368;break;
//    case 202:return -0.374607;break;
//    case 203:return -0.390731;break;
//    case 204:return -0.406737;break;
//    case 205:return -0.422618;break;
//    case 206:return -0.438371;break;
//    case 207:return -0.45399;break;
//    case 208:return -0.469472;break;
//    case 209:return -0.48481;break;
//    case 210:return -0.5;break;
//    case 211:return -0.515038;break;
//    case 212:return -0.529919;break;
//    case 213:return -0.544639;break;
//    case 214:return -0.559193;break;
//    case 215:return -0.573576;break;
//    case 216:return -0.587785;break;
//    case 217:return -0.601815;break;
//    case 218:return -0.615661;break;
//    case 219:return -0.62932;break;
//    case 220:return -0.642788;break;
//    case 221:return -0.656059;break;
//    case 222:return -0.669131;break;
//    case 223:return -0.681998;break;
//    case 224:return -0.694658;break;
//    case 225:return -0.707107;break;
//    case 226:return -0.71934;break;
//    case 227:return -0.731354;break;
//    case 228:return -0.743145;break;
//    case 229:return -0.75471;break;
//    case 230:return -0.766044;break;
//    case 231:return -0.777146;break;
//    case 232:return -0.788011;break;
//    case 233:return -0.798636;break;
//    case 234:return -0.809017;break;
//    case 235:return -0.819152;break;
//    case 236:return -0.829038;break;
//    case 237:return -0.838671;break;
//    case 238:return -0.848048;break;
//    case 239:return -0.857167;break;
//    case 240:return -0.866025;break;
//    case 241:return -0.87462;break;
//    case 242:return -0.882948;break;
//    case 243:return -0.891007;break;
//    case 244:return -0.898794;break;
//    case 245:return -0.906308;break;
//    case 246:return -0.913545;break;
//    case 247:return -0.920505;break;
//    case 248:return -0.927184;break;
//    case 249:return -0.93358;break;
//    case 250:return -0.939693;break;
//    case 251:return -0.945519;break;
//    case 252:return -0.951057;break;
//    case 253:return -0.956305;break;
//    case 254:return -0.961262;break;
//    case 255:return -0.965926;break;
//    case 256:return -0.970296;break;
//    case 257:return -0.97437;break;
//    case 258:return -0.978148;break;
//    case 259:return -0.981627;break;
//    case 260:return -0.984808;break;
//    case 261:return -0.987688;break;
//    case 262:return -0.990268;break;
//    case 263:return -0.992546;break;
//    case 264:return -0.994522;break;
//    case 265:return -0.996195;break;
//    case 266:return -0.997564;break;
//    case 267:return -0.99863;break;
//    case 268:return -0.999391;break;
//    case 269:return -0.999848;break;
//    case 270:return -1;break;
//    case 271:return -0.999848;break;
//    case 272:return -0.999391;break;
//    case 273:return -0.99863;break;
//    case 274:return -0.997564;break;
//    case 275:return -0.996195;break;
//    case 276:return -0.994522;break;
//    case 277:return -0.992546;break;
//    case 278:return -0.990268;break;
//    case 279:return -0.987688;break;
//    case 280:return -0.984808;break;
//    case 281:return -0.981627;break;
//    case 282:return -0.978148;break;
//    case 283:return -0.97437;break;
//    case 284:return -0.970296;break;
//    case 285:return -0.965926;break;
//    case 286:return -0.961262;break;
//    case 287:return -0.956305;break;
//    case 288:return -0.951057;break;
//    case 289:return -0.945519;break;
//    case 290:return -0.939693;break;
//    case 291:return -0.93358;break;
//    case 292:return -0.927184;break;
//    case 293:return -0.920505;break;
//    case 294:return -0.913545;break;
//    case 295:return -0.906308;break;
//    case 296:return -0.898794;break;
//    case 297:return -0.891007;break;
//    case 298:return -0.882948;break;
//    case 299:return -0.87462;break;
//    case 300:return -0.866025;break;
//    case 301:return -0.857167;break;
//    case 302:return -0.848048;break;
//    case 303:return -0.838671;break;
//    case 304:return -0.829038;break;
//    case 305:return -0.819152;break;
//    case 306:return -0.809017;break;
//    case 307:return -0.798636;break;
//    case 308:return -0.788011;break;
//    case 309:return -0.777146;break;
//    case 310:return -0.766044;break;
//    case 311:return -0.75471;break;
//    case 312:return -0.743145;break;
//    case 313:return -0.731354;break;
//    case 314:return -0.71934;break;
//    case 315:return -0.707107;break;
//    case 316:return -0.694658;break;
//    case 317:return -0.681998;break;
//    case 318:return -0.669131;break;
//    case 319:return -0.656059;break;
//    case 320:return -0.642788;break;
//    case 321:return -0.62932;break;
//    case 322:return -0.615661;break;
//    case 323:return -0.601815;break;
//    case 324:return -0.587785;break;
//    case 325:return -0.573576;break;
//    case 326:return -0.559193;break;
//    case 327:return -0.544639;break;
//    case 328:return -0.529919;break;
//    case 329:return -0.515038;break;
//    case 330:return -0.5;break;
//    case 331:return -0.48481;break;
//    case 332:return -0.469472;break;
//    case 333:return -0.45399;break;
//    case 334:return -0.438371;break;
//    case 335:return -0.422618;break;
//    case 336:return -0.406737;break;
//    case 337:return -0.390731;break;
//    case 338:return -0.374607;break;
//    case 339:return -0.358368;break;
//    case 340:return -0.34202;break;
//    case 341:return -0.325568;break;
//    case 342:return -0.309017;break;
//    case 343:return -0.292372;break;
//    case 344:return -0.275637;break;
//    case 345:return -0.258819;break;
//    case 346:return -0.241922;break;
//    case 347:return -0.224951;break;
//    case 348:return -0.207912;break;
//    case 349:return -0.190809;break;
//    case 350:return -0.173648;break;
//    case 351:return -0.156434;break;
//    case 352:return -0.139173;break;
//    case 353:return -0.121869;break;
//    case 354:return -0.104528;break;
//    case 355:return -0.087156;break;
//    case 356:return -0.069756;break;
//    case 357:return -0.052336;break;
//    case 358:return -0.034899;break;
//    case 359:return -0.017452;break;
//    case 360:return 0;break;
//    default:return 0;break;  }//fin de switch------------------------------------------------	
//	
//}//fin del seno table----------------------------------------------------------------


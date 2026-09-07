/*
 * BarraDisplay.c
 *
 *  Created on: Jul 14, 2020
 *      Author: desarrollo-1
 */

#include "BarraDisplay.h"
#include "system.h"
#include "VFD.h"
#include "VFDmenu.h"

//extern volatile unsigned char menu.contexto.Actual;
extern struct _Menu_ menu;

//despliegua el cuadro Madre de deteccion y los dos cuadros hijos vacios en la coordenada x,y
void displayCuadroMadre(void){
    
     if(menu.contexto.Actual==AJUSTE_DE_VIBRACION){
    	 VFDboxLine(ON,BOX_VACIA,POS_X1_BOX_AV,POS_Y1_BOX_AV,POS_X2_BOX_AV,POS_Y2_BOX_AV);//CUADRO MADRE
    	 VFDboxLine(ON,BOX_VACIA,POS_X1_BOX1_AV,POS_Y1_BOX1_AV,POS_X2_BOX1_AV,POS_Y2_BOX1_AV);//CUADRO-1  HIJO VACIO	    
    	 VFDboxLine(ON,BOX_VACIA,POS_X1_BOX2_AV,POS_Y1_BOX2_AV,POS_X2_BOX2_AV,POS_Y2_BOX2_AV);//CUADRO-1  HIJO VACIO	 	 
         }//fin if principal Ajuste de vibracin menu
     else{if(menu.contexto.Actual==PORTAL_INICIO){
    	       VFDboxLine(ON,BOX_VACIA,POS_X1_BOX_PI,POS_Y1_BOX_PI,POS_X2_BOX_PI,POS_Y2_BOX_PI);//CUADRO MADRE
    	       VFDboxLine(ON,BOX_VACIA,POS_X1_BOX1_PI,POS_Y1_BOX1_PI,POS_X2_BOX1_PI,POS_Y2_BOX1_PI);//CUADRO-1  HIJO VACIO	    
    	       VFDboxLine(ON,BOX_VACIA,POS_X1_BOX2_PI,POS_Y1_BOX2_PI,POS_X2_BOX2_PI,POS_Y2_BOX2_PI);//CUADRO-1  HIJO VACIO      
              }//FIN portal inicio
          else{//else seconf
        	  if(menu.contexto.Actual==AJUSTE_PARAMETRICO_DE_PRODUCTO){
        		  VFDboxLine(ON,BOX_VACIA,POS_X1_BOX_AP,POS_Y1_BOX_AP,POS_X2_BOX_AP,POS_Y2_BOX_AP);//CUADRO MADRE
        		  VFDboxLine(ON,BOX_VACIA,POS_X1_BOX1_AP,POS_Y1_BOX1_AP,POS_X2_BOX1_AP,POS_Y2_BOX1_AP);//CUADRO-1  HIJO VACIO	    
        		  VFDboxLine(ON,BOX_VACIA,POS_X1_BOX2_AP,POS_Y1_BOX2_AP,POS_X2_BOX2_AP,POS_Y2_BOX2_AP);//CUADRO-1  HIJO VACIO  	  
        	  }//fin //AJUSTE_PARAMETRICO_DE_PRODUCTO
        	  else{//thrids else 
        		  if(menu.contexto.Actual==PARAMETROS_DE_DETECCION){
        		  VFDboxLine(ON,BOX_VACIA,POS_X1_BOX_PD,POS_Y1_BOX_PD,POS_X2_BOX_PD,POS_Y2_BOX_PD);//CUADRO MADRE
        		  VFDboxLine(ON,BOX_VACIA,POS_X1_BOX1_PD,POS_Y1_BOX1_PD,POS_X2_BOX1_PD,POS_Y2_BOX1_PD);//CUADRO-1  HIJO VACIO	    
        		  VFDboxLine(ON,BOX_VACIA,POS_X1_BOX2_PD,POS_Y1_BOX2_PD,POS_X2_BOX2_PD,POS_Y2_BOX2_PD);}//CUADRO-1  HIJO VACIO  	  
        	  }//else third ends  
          }//else second
     }//fin first else
}//fin del despliegue del cuadro Madre y los dos hijos en funcion del contexto----------------------------


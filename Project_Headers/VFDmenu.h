/*
 * VFDmenu.h

 *
 *  Created on: Mar 9, 2017
 *      Author: chals
 */
/*
 * VFDmenu.h
 *
 *  Created on: Jul 14, 2015
 *      Author: chals
 */

#ifndef VFDMENU_H_
#define VFDMENU_H_

#endif /* VFDMENU_H_ */

#include "Memoria.h"
#include "PE_Types.h"
#ifndef HW_VER_2
#define HW_VER_2 3//[3||4] version 2.3 ó 2.4
#endif
#define EN_LA_CASA     0xBB
#define EN_EL_TRABAJO  0xAA
#define DONDE_ESTOY    EN_LA_CASA

#if(HW_VER_2==3)
  #pragma message("version de Hardware 2.3 pines que cambian")
  #pragma message("Led3_Process  Pin28[QSPI_CS0]")
  #pragma message(" N/A          Pin77[GPT1]")
  #pragma message("QSPI-CS0      Pin57[SPDLED]")
#elif(HW_VER_2==4)
  #pragma message("version de Hardware 2.3 pines que cambian")
  #pragma message("Led3_Process  Pin77[GPT1]")
  #pragma message("QSPI-CS0      Pin28[QSPI_CS0]")
#else
#error "Version de Hardware Desconosida"
#endif	



#define VERSION_FIRMWARE   "Vesion F.W 33.11.0" //add frecq CONTROL from menu, Driver to control ADC's 
#define VERSION2  "v40.5.2.6.10"


/*
los ADC se perdio la sincronia de los pulsos deSCLK cada 1mseg
 *             final=TT   VFD(LC) VFDmenu(LC) Memoria(LC) ADC(LC) DSP(LC)  ProductManager(LC)  lineas de codigo=LC[.c,.h] tamaño total =TT(bytes)
 *  v36.0.1   empezamos contruccion del DDS YA TENEMOS EEPROM,NVRAM,ADC's, Display, Menus, DDS-frecuency,Display
 *              68000
 *  v36.0.2   debugeando el DDS
 *  v36.0.3-13   debugeando las variables del DDS y control y despliegue en DDS
 *              72000     186,47  4745,308    415,66     206,31   18,15      20,20
 *  v36.1.0-73   10-NOV-16: sem43: Desarrollo despliegue de la señ	al en DDS, 
 *               22-NOV-16:el DDS aun no esta se mejoro la graficacion y el menu simultaneo              
 *              82600     245,53   5036,319    415,66    206,31    315,46    60,20 
 *              falla general de las memorias NVRAM y EEPROM, Y tambien se quemaron los chips
 *              regresamos a la version 35
 *  v37.1.0-25rr  22-Nov-16: Restarar la version con las memorias Reaparadas,
 *                 Reparar el DDS-PWG, Crear el BIOS. 
 *  v37.2.0-34  24-Nov-16: Se Termiano el DDS aunque faltan detalles, Guardar valores de variables del DDS 
 *                         en EEPROM,  se crearon  los zooms.  90kbytes
 *  v38.0.0-7    24-NOV-16:  fix saved zoom@DDS, coreccion de graficacion doble,
 *   v38.1.0-10    9-DIC-16   Comunicacion BDM con la IO,  
 *   v38.2.0-7    15-DIC-16  comunicacion procesadora-IO, commADC,->fue mucha la interrupcion y muy larga
 *                         perdimos todos los datos que tenieamos de memoria. cambiamos el metodo de comunucacion   
 *   V38.3.0-36    20-dic.16  cambiamos por maquinas de estados la comunicacion,      
 *                         comunicacion procesadora-IO, commADC,  
 *                         aumento de velocidad del ADC.
 *   v39.0.0-40  3-ENE-17:cambio de algoritmo de comunicaciones porque era de 300ms y necesitamos uno de 1ms
 *                       Transmision con la IO en sus 4 formas,
 *                       conmunicacion dela IO con la PC manda datos que provienen de la Procesadora 
 *   v39.1.0-8   2-FEB-17  ver que funcione bien el DDS, corregir  varios bugs del DDS                                                                     
 *   v39.2.0     3-FEB-17  correccion del acondicionamiento de señal del DSP,
 *                          mandar datos para graficarlos en la PC, y 
 *   v39.3.0    14-Feb-17 la transmision de datos que llegan a la PC es muy lento tenemos que aumentar al doble la
 *                        velocidad combiando el protocolo de comunicacion, modificacion de casi todos los
 *                         algoritmos del DSP. control de los Reles de la IO por comand de Menu, LEctura de las entradas de los 
 *                         sensores en menu de pruebas,
 *  v39.4.0    22-FEB-17  Menu de Producto, Modificar modo de guardar variables en EEPROM, para guardar 
 *                        productos, Seleccionar producto, error de cero interrupcion cuando loop por mal prioridades de int,
 *  39.7.0    27-Feb-17   Errores con la Memoria EEPROM, tratamos de cambiar algoritmos en memoria no resulta parace mal el protocolo 
 *                        de R/W a eeprom, lo dejamos como esta, solo vamos a cambiar direcciones,
 *                        Crear otra manera de hacer productos Crear R/W de productos en EEPROM.                                                            
 *   39.8.0    10-Mar-17   Se cayo el sistema hace 3 dias, y ya no se pudo recuperar, ahora pasamos todos los archivos
 *                         y vamos a probar funcion por funcion, periferico por periferico. hasta dejarlo como estaba.
 * 
 *    39.8.0-6   10-Mar-17   Se cayo el sistema hace 3 dias, y ya no se pudo recuperar, ahora pasamos todos los archivos
 *                         y vamos a probar funcion por funcion, periferico por periferico. hasta dejarlo como estaba.
 *   39.10.0-100 22-Mar-17  ya funciona el nuevo driver de Read Write de la EEProm, ahora vamos a hacer el nuevo
 *                         Init_ variables de EEPROM
 *                         crear y probar nuevoProducto,crear procesador de textos, reaustar el ADC al nuevo procesor speed, REAJUSTAR el DDS delays
 *                         moficar display e puntitos ahora por timer sin usar delays,
 *  39.12.0-5  10-abr-17  unir los driver de EEPROM en una sola funcion y usar timer en lugar de delays y usar una funcion FIFO
 *                         para escribir varias veces la EEPROM usando filas sin tener que deter todos los procesos                                            
 *  39.13.0   28-abr-17   Controlar la ganancia de la Analoga, 3pines, y menus,
 *                         terminar de probar y guardar la iProduct var, terminar los sub menu de Ajuste de producto, borrar, eliminar, modificar, copiar producto                    
 *                         DDS=    DDSdots=    ADC=     keypad=    VFD=    IOreles=
 *                         IOinputs=   
 *  39.13.8-34 9-May-17    funcionando al mismo tiempo DDS-gen, ADC-DAQ
 *  39.13.9.89             funcionando al mismo tiempo DDS-gen, ADC-DAQ, Analoga Frecq, Analoga Ganancia switchs
 *                         NUEVO MICROCONTROLADOR
 *  39.13.10.0-43 30-May-17  
 *  39.13.11.0-29 25-Jul-17   Vamos a hacer el algoritmo de configuracion de angulo de vibracion.
 *                        No se puede sacar la config vibracion con los valores que despliega el DDS
 *                        tenemos que usar los valores reales centrados.
 *  39.13.12.0-21 27-jul-17  seguir con el menu de configuracion de vibracion  y la barra de deteccion                     
 *                         Tenemos el menu de configuracion de vibracion haciendo la barra de deteccion que se 
 *                         llene y se vacie automaticamente, listo para un demo de avance.
 *  39.13.13.0-26 31-jul-17  terminar la barra de deteccion y el angulo de vibracion.
 *                        no hemos terminado la barra de deteccion, muchos problemas vamos a cambiar 
 *                        el algoritmo y vamos a implementar la version 2 del algoritmo de deploy display bar detection
 *  39.13.14.0-33 2-JUL-17  terminamos la barra de deteccion y el control de limites de deteccion preeliminar
 *  39.13.15.0-21 7/Agt-17 obtener el angulo de vibracion, DSP.c->AnguloVibracionProcesadorCentral
 *                        el calculo del arcoTan es por series de potencia falta de debugear en java, pero nos cambiamos a una formula
 *  39.13.16.0-35 11-Agt-17, continuar con el angulo de vibración con nueva version.    
 *                       Hay display de barra de detecion y numeros al mismo tiempo
 *  39.13.17.0-11  28-agt-17 angulo de vibracion, le falta eactitud, tenemos un error de mas de un grado 
 *                            vamso a cmabiar de formula de tangente.              
 *  39.13.18.0-58  12-Sept-17, ya esta terminado la configuracion de vibracion completa
 *  39.13.19.0-21     25-Sept-17  Barra de deteccion en PortalMenu, configuracion PArametrica, y otros 2 menus
 *  39.13.20.0-16     28-Sept-17  Los numeros y caracteres y posiciones no se pueden manejar bien
 *                            En los menus que tiene barra de deteccion online. asi que vamos hacer un controlador
 *                            para esto., Ya funciona la variable sensibilidad en los menus con barra de deteccion
 *  39.13.21.0       3-oct-17 , La barra de deteccion cuando salimos del menu funcione bien, crear control de salida del menu en forma
 *                            sistematica operativa,     
 *                            Nueva version de los Controladores de Memoria                     
 *  v39.13.21.167    cambio de procesador porque hay un error PE_ISR(Cpu_ivINT_ILLEGAL_INSTR)
 *  v39.13.21.174   9-OCT-18, Un año despues, que nos dejaron seguir con el codigo, 
 *                           vamos a revisar todos los sistemas y  apasar todo el codigo a la nube para mayor proteccion y manejo.
 *  v39.13.22.0-8     1-Feb-18 CHIP-NUEVO casi no he tenido chance de programar 1 o 2 horas al dia,
 *                           vamos a poner un mejor control de DDS y ha revisar el DDSdisplay en que me quede hace mas de un año                         
 *  v39.13.25.0    8-Feb-19, creamos el control operativo para el spi, el dds no funciona vamos a configurar el spi para el dds 
 *                           y luego adapatamos el ADC ala velocidad y polaridad del DDS.
 *  v39.13.25.60-91 8-OCT-19 comenzamos a programar con amplia concentracion porfin. estabamos en tarjetas.
 *                            errores de sistema de interrupcion de CPU, no funcianan la interrupcion de adc, de dsp, 
 *                            no quiere funcionar cuando llamanos algo de ADC.c, y ahora no reconoce ningundispositivo hecho
 *                            por el processor expert.
 *  v39.14.0.0-99  19<->23-oct-19 se hizo la maquina automatica de ADC por interrupciones y el ReadADC lo
 *                            toma PERFECTO. ahora comenzamos a optimizar el codigo de VFD
 *  v39.15.0.0-22  23<->25-oct-19                         
 *                            logramos encender el VFD y el primer pantalla portal mejoramos y cambiamos el
 *                            delay.c y intentamos cambiar todo el sistema de control de VFD y 
 *                             lo separamos en 4 archivos
 *                            intentamos optimizar el codigo y mas facil de mantener y mejorar,
 *                             rene nos hablo a 
 *                            junta y no se alcanzo a guardar proyecto y se daño el target no 
 *                            funciona el download error en VFDserial de halt suspendt
 *  v39.15.1.0-25 25-oct-19   tiene una falla con VFD send char y send block no sabemos porque alli hace halt
 *                            y vamos ahora a construir el programa apartir del display enlugar del ADC.
 * v39.15.3.0-11 4-Nov-19     error de halt y suspend, el VFDserial propio no funciono necesita mas tiempo.
 * v39.15.4.0-2   5-nov-19    muchos problemas con Halt y suspend en VFDerial_SendBlock tuvimos que tomar el codigo de la  version 
 *                           v39.13.21.174  esta version se deja intacta para futuras referencias. 
 * v39.15.5.0-35  5-nov-19    muchos problemas con Halt y suspend en VFDerial_SendBlock tuvimos que tomar el codigo de la  version 
 *                           v39.13.21.174  y apartir de este vamos a introducirle el codigo que mejoramos
 *                           poco a poco. falta hacer: construir todos los menus y la barra de deteccin y el DDS pero 
 *                           vamos a usar as interrupciones de la libreria a convertir las
 *                            funciones en funcinoes operativas con estados de maquina y meter  maquina automatica de ADC por interrupciones y el ReadADC lo
 *                            toma perfecto. Se finalizo de Integrar el ADC funcional y automatico con control de error de la version v39.14.0.0
 * v39.15.6.0-49    7-Nov-19    mejorar el arhivo VFDmenu,  y optimizar el codigo de VFDmenu.
 *                            mejoramos y optimzamos el VFDmenu y y optimizamos la velocidad de la  
 *                           de deteccion, optimizamos el codigo de la eeprom VIRTUAL. ahora queremos modificar
 *                           el menu de select product para que se vea el producto 0 setup.                                         
 * v39.15.7.0-117  14-26-Nov-19 139568kb, Terminamos menu de borrar producto, copy producto, select producto,
 *                              nuevo producto, completo sin errores, falto el de ajuste de producto
 * v39.15.8.0-322  26-Nov-19  140996,se hizo el menu de control de contraseñas con tdo su manejo de memoria.
 *                             se hizo los submenos de entradas y salidas se hizo todo menos el cambio de 
 *                             userdefined en el cambio de nombre delos auxiliares.
 * v39.15.9.0-230      16-Ene-20  vamos a hacer el control del DDS vamos a poner todo las funciones 
 *                            que mandan datos por serial al DDS en forma de cola.
 * v39.16.11.0-10         5-Mar-20 se perdio todo el archivo VFDmenu.c y vamos  recuperar el avance.
 *                            de las entradas y productos, nuevo,  cambio de producto y  todo eso que me tarde 3 meses
 * v39.16.12.0-16        6-Mar-20  vamos a realizar otra vez la graficacion de puntitos otra vez.                      
 * v39.16.13.0-27           9-Mar-20, no funcionan las interrupciones.              
 * v39.16.14.0-7            10-Mar-20, ya funcionan las interrupciones todavia no graficamos en DDS puntos toda la pantallacon delay operativo                    
 *                                   generacion de datos simulados cada 500useg
 * v39.16.15.0-11      10-Mar-20, DDS opertivo display puntos y chars y pos, al mismo tiempo sin errores.                    
 *                     todavia no al 100% despliegue de char y puntos al mismo tiempo
 * v39.16.16.0-12      11-Mar-20, ya hay despliegue pixel y char al mismo tiempo, falta la B de borrado como bold
 * v39.16.17.0-5       12-Mar-20, ya funciona cursor y pixels al mismo tiempo., guardamos para revision de rene
 * v39.16.18.0-13      12-Mar-20, ya funciona bold cuso puntitos en toda la pantalla, salir y entrar. del menu
 * v39.16.19.0-9       13-Mar-20, vamos a hacer la barra de deteccion operativa como DDS version.2 y conteo productos
 * v39.16.20.0-26      18-Mar-20, la barra operativa v2, no esta lista. la barra de deteccion sube, todavia no baja
 * v39.16.21.0-35      20-Mar-20, que la barra baje y  funcione con char al mismo tiempo, ya funciona conteo de prod.     
 * v39.16.22.0-20      27-Mar-20, conectar la señal al DDS y al analisador de deteccion.                                                                          
 *                     ya esta el algoritmo de voltajes pero se averio la velocidad del procesador
 *                     vamos a regresarnos una version y a poner las cosas que nuevas alli.
 *                     RELOJ-LENTO, dejamos esta version y pasamos el codigo hecho a
 *                      la version 39.16.21
 * v39.16.23.0-15      averiguar el error dela velocidad del procesador, el error es la funcion
 * v39.16.24.0-64      Voltimetros de driver y pick, listos.                                       
 * v39.16.25.0-49      terminamos los cuatro voltimetros de pickup, driver, canal X&Y 
 *                     el DDS continua dibujando los puntos en la pantalla completa con valores programados.
 * v39.16.26-0-137     1-jun-20 poner el DDS con señal real. Avanze en la señal offset
 *                     mejorar version de despliegue de voltajes, avanze en borrado de DDS,
 *                     vamos a cambiar el algoritmo de conversion de coordenadas a int DDS puntitos
 *                     por ecuaciones de recta en lugar de memoria.
 * v39.16.27.0-18      11-Agt-20, Borrado de DDS, menu de borrado con bold y cursor
 * v39.16.28.0-2       1-Sept-20, comunicaciones con IO, ver2, protocolo de comunicacion final.
 *                       fallo comunicacion de los dos uart, comunicacion con vfd se desconfguro. por mover el uart
 * v39.16.29.0-17      14-Sept-20, funcion para anti-rebounce de teclado con timer.                     
 * v39.16.31.0-26         30-Sept-20, comm io-up nuevo version-2   
 *                             reacomodo de metodos de menu a display y a menuControl
 * v39.16.32.0-66         19-oct-20, graficacion del DDS correcion de conversion una area no pintaba, ecuaciones mal,
 *                       cuando regresa del borrado al portal ya se borra bien usando sistema operativo en etapas
 *                       Ya funciona con ssistema operativo el borrado deL DDS sin erres de characteres en display        
  v39.16.33.0-18         27-OCt-20  centrado de datos y borrado funciona perfecto.
  v39.16.34.0            30-OCt-20  empezamos a diseñar el procesador de graficos que un nueva 
                                version de como desplegar en el display usando el sistema operativo
                            contruyendo el controlador de graficos. control x timer por menu. control de los numeros de 5 cifras  individualmente
 *v39.16.34.0-74         30-OCt-20  empezamos a diseñar el procesador de graficos que un nueva 
                            version de como desplegar en el display usando el sistema operativo
 *                          control x timer por menu. control de los numeros de 5 cifras  individualmente phase y sensibilidad en menu de barra de deteccion
 *                          empezar a construir el controlador de graficos.
 *v39.16.35.0          11-Nov-20  actualizacion del algoritmo de despliegue de chars en menu de angulo de vibracion.             
 *                         depuracion de menu de ajustes parametricos y ajustes avanzados.
 *v39.16.35.0-154    11-Nov-20 depuracion de menu de ajustes parametricos y ajustes avanzados.
 *v39.16.36-70          19-Ene-21  actualizacion del algoritmo de despliegue de chars en menu de angulo de vibracion.
 *                        zoom para todos los zooms.  
 *v39.16.37.0-44        17-feb-21 envio de datos al touch construccion del control de los menus y maquinas de seriales
 *                  SE CAMBIO EL PROCESADOR, CAMBIO DE CHIP ERROR FALLA TRGET NO ERASE                                                
 v39.16.38.0-5        28-feb-21 estamos trabajando en el zoom pero se necesitan mas fifo, vamos a poner una fifo
                      temporal y otra en el sistema operativo para que procese el repintado cuando se ejecuta el zoom
 v39.16.39.0-48        1-Mar-21   ggravmos los pixeles del zoom en 4 arrays de diferentes cuadros concentricos
 v39.16.40.0-48        12-Mar-21  depurando el bufer segundo en pantalla
                          y otro buffer para datos fuera de pantalla 
 v39.16.41-0-37       25-Mar-31 grabacion de los pixeles actuales de deteccion de la pantalla actual
                             se necesita poner un solo algoritmo ppor bit, no importa si se sale de la pantalla. 
                             poner mas array para guardar puntos por bits.                                  
* "v39.16.43.0-5"    22-abr-21 tenemos un algoritmo que genera todoso los numeros desplegables de las coordenadas de la pantalla
*                     para guardarlos en la memoria de pixeles luego sacarlos y desplegarlos para comprobar que el 
*                      algorimo de guardado y sacado de pixeles guardados si funciona, esto ya funciono
*                     sigue generar cuadros y luego cambiarlos de tamaño usando el zoom
*  v39.16.44.0-17     tenemos avance en el dibujado del zoom de la figura geometrica pero estorba
*                     los buffer que guardan las coordenadas que sobresalen del cuadro, por lo que vamos a quitar todo el algoritmo
*                      de estas coordenadas, y despues lo pondremos con algoritmos de puntitos tal como se guarda con las coordenas
*                       que caen en el cuadro.
*  v39.16.44.0-134    6-Sept-21  tenemos algoritmo de zoom preeliminar,y guardado de puntitos, tambien manejo de variables de
*                       DDS y pintado de lineas de deteccion, estamos terminando el control de la genercion de frecuencia
*                       fallo por i2c si que nos regresamos  SPI.                     
* "v39.16.46.0-10"    protocolos de comunicacion con la IO  
* "v39.16.47.15"      17-MAR-22, protocolos de comunicacion, control del keypad por FIFO,
* "v39.16.48.26"      22-MAR-22., control de keypad solo funciona con IRQ sin fifos.
*                              rechazo por IRQ ok,                                 
* "v39.16.49.23"      19-abr-22, nuevo control de keypad, comunicacion rechazo con IO, menu pruebas
* "v39.16.50.40"      24-agt-22, se perdieron 250 actualizaciones, ahora vamos a cambiar 
*                               la forma que se despliega numeros, posiciones y cuadritos al mismo tiempo
*                               y agregamos el DTR del display ala procesadora.                        
*    v40.5.2.2.22     6-jun-26  vamos a cambiar el metodo del led monitor porque no funciona bien
*    							si reconoce el comando los led de diagnostico del serial, pero los monitores no.
*    							                                             
*                               
 *                    LABORTORIO PROGRAMACION DEPURACION LISTA
 *                       1.-Procesador Textos, borrar string no se regresa cursor, revisar control borrar add, insertar.               
 *                       2.-Menu Ajuste de Sistema con usuario 2 debe verse solo menu administrtivo y se ve todo el menu
 *                       3.-Menu Ajuste de Sistemas con usuario 22 entra y no debe           
 *                                                                                 
 *                   Description	Resource	Path	Location	Type
Undefined : "" Referenced from "display_DDS_transmitter_Controller" in	39_15_09_IN1UP		 	C/C++ Problem
 *                      Description	Resource	Path	Location	Type
Undefined : "contextoActual" Referenced from "AnguloVibracionProcesadorCentral" in	39_15_09_IN1UP		 	C/C++ Problem
 *                            
 *                            
 *                            
 */






//++++ DEFINICIONES ++++++++++++++++++++++++++++++++++++++++++++
// NOMBRES DE MENUS**********PONER-LOS NOMBRE-DE-LOS-MENUS---------3a
#ifndef NULL
  #define  NULL   0x00
#endif


#ifndef   NONE
  #define  NONE   0
  #define  NO    '0'
  #define  NOP    0
  #define  NEL    0
  #define  OFF    0
#endif



#define    APAGAR    '0'
#define   PERMITIR   '1'
//-----------------------------------------------------------------|<--NO-MOVER-LA-X-

#define VERSION                               "version 0.2               x"
#define EN_DESARROLLO                         0x00U
//*************KONSTANTES_DE_MENU_++++++++++++++++++++++++++++++++++++++++++++++++++++
#define PORTAL_INICIO                         0xA1U
#define MENU_INSIGHT                          0xA2U  
#define AJUSTE_DE_SISTEMA                     0xA3U  
#define AJUSTE_DE_PRODUCTO                    0xA4U  
#define I2CEEPROMREAD                         0xA5U
#define SPI_MENU                              0xA6U
#define SPIDSSCOMM                            0xA7U //------------------------------M1:DEFINIR EL NOMBRE DEL MENU A CREAR
#define ESTAS_SEGURO                          0xA8U//contexto que pregunta antes de guardar
#define INGENIERIA_GRACIDA                    0xA9U //ingenieria Lock
#define DISPLAY_ADC                           0xAAU //despliega los valores delos  ADC en numero en la pantalla con fines de pruebas e investigacion en el desarrollo
#define MEDIDAS_DE_VOLTAJE                    0xABU
#define AJUSTE_DE_PRODUCTOINFO                0xACU
#define TIEMPO_DE_RECHAZOINFO                 0xADU
#define AJUSTE_SISTEMA_ENTRADAS               0xAEU
#define ID_MAQUINA                            0xAFU
#define CUENTA_PRODUCTO                       0xB0U//----------------------------MENU@3:DEFINE_CONSTANT->MENU4@C
#define HACER_PRUEBA_DE_RUIDO                 0xB1U
#define ESTABLECER_IDIOMA                     0xB2U 
#define I2CNVRAMDATA                          0xB3U//display in pruebas nvram data
#define INIT_MEMO_Y_REST                      0xB4U //Inicializar Memoria y restaurar
#define AJUSTES_AVANZADOS                     0xB5U
#define RELOJ_DE_SISTEMA                      0xB6U 
#define PRUEBA                                0xB7U
#define I2CMENU                               0xB8U
#define PRUEBAS_SALIDAS                       0xB9U
#define PRUEBAS_ENTRADAS                      0xBAU
#define SELECCIONAR_PRODUCTO                  0xBBU
#define NUEVO_PRODUCTO                        0xBCU
#define TEXT_PROCESSOR                        0xBDU
#define AJUSTE_DE_PRODUCTO_HIJO               0xBEU //submenu dentro ajuste de producto
#define PARTE_11                              0xBFU//menu de configuraciones unicas y especiales para el desarrollador
#define AJUSTE_DE_VIBRACION                   0xC0//MEnu de ajuste de vibracion
#define ESTAS_SEGURO2                         0xC1//menu especial para despues de una barra de deteccion dinamica
#define AJUSTE_DE_PRODUCTO_MODE               0xC2 //menu de modo automatico o manual
#define BORRAR_PRODUCTO                       0xC3//menu borrar producto
#define COPY_PRODUCTO                         0xC4//menu copiar producto
#define AJUSTE_PRODUCTO_SEL                   0xC5//ajuste producto donde se seleccionan el producto yse ajusta
#define PRUEBAS_SW_ANALOGA                    0xC6
#define PASTE_PRODUCTO                        0xC7//miamo menu de copy product
#define SUPERVISOR                            0xC8//para controlar los contextos por password
#define AJUSTE_TIEMPO_RECHAZO                 0xC9//ajuste de tiempo de rechazo por tiempo o por pulsos
#define ENTRADA_TACO                          0xCA
#define ENTRADA_FOTOCEL                       0xCB
#define ENTRADA_TEST_RECHAZO                  0xCC
#define ENTRADA_TEST_PRODUCTO                 0xCD
#define ENTRADA_AUX1                          0xCE
#define ENTRADA_AUX2                          0xCF
#define MEDIDAS_DE_VOLTAJES                   0xD0
//**********ORGANIZACION*DE*MEMORIA**EEPROM**Y**KONTANTES*DE*MENU************************
//QUE  las direcciones  no coincidan con las de arriba OJO
#define CONFIGURAR_ENTRADAS_DE_SISTEMA_SIZE   6     //ESTOS DATOS SON MUY DELICADOS POR LO QUE TENEMOS QUE PONERLOS ASI, YA QUE NO FUNCIONO CON FORMULAS
#define CONFIGURAR_ENTRADAS_DE_SISTEMA        0x01 //01 02 03 04 05 06
#define ID_COMUNICACIONES_SIZE                1
#define ID_COMUNICACIONES                     0x07 //07
#define CONTROL_DE_FRECUENCIA_SIZE            3
#define CONTROL_DE_FRECUENCIA                 0x08 //08 09 0A	
#define INFORMACION_DE_USUARIO_SIZE           1
#define INFORMACION_DE_USUARIO                0x0B //0B
#define CONTROL_PASSWORD_SIZE                 4
#define CONTROL_PASSWORD                      0x0C //0C 0D 0E 0F
#define MENU_ADMINISTRATIVO_SIZE              2
#define MENU_ADMINISTRATIVO                   0x10 //10 11        
#define TIPO_DE_MAQUINA_SIZE                  1 
#define TIPO_DE_MAQUINA                       0x12 //12       
#define GANANCIA__SIZE                        2
#define GANANCIA_                             0x13 //13 14            
#define FILTRO__SIZE                          1
#define FILTRO_                               0x15 //15
#define INGENIERIA_GRACIDA2_SIZE              3
#define INGENIERIA_GRACIDA2                   0x16 //16 17 18
#define AJUSTE_PARAMETRICO_DE_PRODUCTO_SIZE   7
#define AJUSTE_PARAMETRICO_DE_PRODUCTO        0x19 //19 1A 1B 1C 1D 1E 1F  
#define FRECUENCIA_SELECT_SIZE                5               
#define FRECUENCIA_SELECT                     0x20 //20 21 22 23 24
#define PARAMETROS_DE_DETECCION_SIZE          5
#define PARAMETROS_DE_DETECCION               0x25// 25 26 27 28 29 	
#define PANTALLA_DDS_SIZE                     3
#define PANTALLA_DDS                          0x2A//2A 2B 2C
#define ERROR_LISTA_SIZE                      25
#define ERROR_LISTA                           0x2B//2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40 41 42 43 44 45
//#define STATUS_FACTORY_SIZE                   2 
#define STATUS_FACTORY     	                  0x46//46 47
#define NOMBRE_PRODUCTO_SIZE                  20
#define NOMBRE_PRODUCTO                       0x48//48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 56 57 58 59 5A 5B


//#define RELOJ_DE_SISTEMA_SIZE                 9

//fin de las variables de store into EEPROM 
#define LASTADDRESS_VARS_EEPROM               0x65//LAST NAMED, para sacar los datos en el inicio, hasta donde
             // va a LEER


//****FIN*DE*LA*ORGANIZACION*DE*MEMORIA**EEPROM****************************************************
//****FIN*DE*LA*ORGANIZACION*DE*MEMORIA**EEPROM****************************************************
//****FIN*DE*LA*ORGANIZACION*DE*MEMORIA**EEPROM****************************************************
//****FIN*DE*LA*ORGANIZACION*DE*MEMORIA**EEPROM****************************************************
//****FIN*DE*LA*ORGANIZACION*DE*MEMORIA**EEPROM****************************************************
//****FIN*DE*LA*ORGANIZACION*DE*MEMORIA**EEPROM****************************************************
//****FIN*DE*LA*ORGANIZACION*DE*MEMORIA**EEPROM****************************************************
//****FIN*DE*LA*ORGANIZACION*DE*MEMORIA**EEPROM****************************************************
//****FIN*DE*LA*ORGANIZACION*DE*MEMORIA**EEPROM****************************************************



//permisos de acceso de cada menu al sistema de login
#define USUARIO_PERMISO            0x80U //permisos de cada menu para deslegarse en pantalla o no.
#define SUPERVISOR_PERMISO         0x81U
#define TECNICO_PERMISO            0x82U
#define INGENIERO_PERMISO          0x83U
#define DESARROLLADOR_PERMISO      0x84U

// valores de entradas absolutas de los botones estaticos del keypad
#define KEY_ENTER                  0x99
#define KEY_UP                     0x9A
#define KEY_DN                     0x9B
#define KEY_LF                     0x9C
#define KEY_RT                     0x9D

//valores de las posiciones Estaticas-Dinamicas del Coordinador inherente del Control Supremo de Posiciones
#define POS_PRODUCTO_TITULO_X      67 //posicion x del producto en Polrtalinicio
#define POS_PRODUCTO_TITULO_Y      2  //posicion y del producto en Portalinicio

#define POSX0CNTRLPASSW            8
#define POSX9CNTRLPASSW            64//menu control de passwords seleccion de activacio o not
#define POSXPARAMDETECT            136 //POSISCION DEL las variables en PARAMETROS DE DETECCION
#define POSXCFNUM                  120// posicion del numero de control de frecuencia,
#define POSXAJUSPROD               168// posicion del numero de DisplayAjusteParamProd
#define POSXNUM                    178//posicion del numero de ID de comunicaciones
#define POSXNUMRELOJ               125 //posicion de los numeros de configuracion de hora
#define POSXESQUINA                227//235
#define POSXESQ235                 235
#define POSXESQ239                 239
#define POSXENTSYS                 192 //posicion en menu entradas de sistemas
#define POSXCTA                    192 //posicion de "APAGA" ó "PERMITIR" de cuenta de producto en informacion de usuario en menu insight       
#define POSX192                    192 
#define POSX185                    185
#define POSX152                    152 //TiempoDErechazo, 
#define POSX125                    125	
#define POSX177                    177
#define POSXLABELFRECPOS           72
#define POSX0                      0
#define POSXSELPROD                24 //POSICION DE INICIO nombres DE SELECCION DE PRODUCTO
#define POSXSELPRODCURSORX         16//POSICION DE SEL PRODUCTO MENU cursorX
#define POSXSI                     78
#define POSY0                      0
#define POSY2                      2
#define POSY4                      4  //POSICION EN DISPLAY A QUE VALE 4
#define POSY6                      6
#define POSY8                      8
#define POSY10                     10
#define POSY12                     12
#define POSY14                     14
#define POSXDIGPRUEBASSAL          136//posicion del digito de estado del menu PRUEBAS_SALIDAS
#define POSXDIGPRUEBAENT           104//posicion del digito de estado del menu PRUEBAS_ENTRADAS
#define POSX_COL1                  8//16 //TEXT PROCESSOR columna-1 de x
#define POSX_COL2                  32//40  CORDENADAS PARA PONER EL CURSOR
#define POSX_COL3                  56//64
#define POSX_COL4                  80//88  EN la pantalla de procesador de texto
#define POSX_COL5                  104
#define POSX_COL6                  128
#define POSX_COL7                  152
#define POSX_COL8                  176
#define POSX_COL9                  200
#define POSX_COL10                 224
#define POSX_TEXT_PROCS_OK         204 //posiciondela X en la palomita
#define POSX_TEXT_PROCS            POSX_TEXT_PROCS_OK+8  //POSICION DE la palomita
#define POSX_TEXT_PROCS_X          POSX_TEXT_PROCS+16
#define POSX_APP_FASE              //Num fase de Menu AjusteParamProd
#define POSX_ESQ239                239
#define POSX_MENU_TACO             176//posicion de los numeros de columna derecha
#define POSX_RECHAZO_NUM           152//posicion de numero de  rechazo en portal inico
#define POSX_VOLT_BALANCE          168//posicion de los numeros de voltaje de balance en MENU  MEDIDAS_DE_VOLTAJE
#define POSX_VOLTS_X               64//Posicion X de voltaje-X en menu MEDIDAS_DE_VOLTAJES
#define POSX_VOLTS_Y               176//Posicion Y de voltaje-Y en un menu MEDIDAS_DE_VOLTAJES
#define POSX_PROD_PARM             160//MENU  PARAMETRICO DE producto


	// constantes REGENTES del Posicionador del Cursor DDS grafico
#define POSXDDS                    198 //POSICION DE INICIO DE LAS letras en el DDS                            S 
#define POSXDDSNUMS                214 //Posicion de inicio de los Numeros valores                               00000 
#define POSXDDSCURSORMENU          190 //Posicion del cursor en las letras para escoger el valor              >S 00000 
#define POSXDDSCURSUBMENU          206 //POSICION del cursor señalando intervencion en el valor del parametro   >00000
#define POSXDSSDIGITO1             POSXDDSNUMS //NUMEROS digito-1
#define POSXDSSDIGITO2             POSXDSSDIGITO1+8//NUMERO digito-2
#define POSXDSSDIGITO3             POSXDSSDIGITO2+8//NUMERO digito-2
#define POSXDSSDIGITO4             POSXDSSDIGITO3+8//NUMERO digito-2
#define POSXDSSDIGITO5             POSXDSSDIGITO4+8//NUMERO digito-2
#define DDS_CENTRO_X               64  //PPUNTO en pantalla para encontrar el cero en coordenadas x
#define DDS_CENTRO_Y               95  //PUNTO EN pantalla para encontrar el cero en coordenadas y
#define DDS_MENU_X                 POSXDDSCURSORMENU //posicion cursor

//DEFINCIONES DE LOS SYMBOLOS QUE DESPLIEGA Y MANEJA EL PROCESADOR DE TEXTOS	
#define CX   94 //ascii del cursor de la pantalla de Texto processor
#define S1   0xA0
#define S2   0xA1
#define S3   0xA2
#define S4   0xA3
#define S5   0x9F
#define S6   0x9E
#define S7   0x9D
#define S8   0x9C
#define S9   0x9B
#define S10  0x9A
#define S11  0x99
#define S12  0x98
#define S13  0x97
#define S14  0x96
#define S15  0x95
#define S16  0x94
#define S17  0x93
#define S18  0x92
#define S19  0x91
#define S20  0x90
#define S21  0x8F
#define S22  0x8E
#define S23  0x8D
#define S24  0x8C
#define S25  0x8B
#define S26  0x8A
#define S27  0x89
#define S28  0x88
#define S29  0x87
#define S30  0x86
#define S31  0x85
#define S32  0x84
#define S33  0x83
#define S34  0x82 
#define S35  0x81
#define S36  0x80
#define S37  0x7F
#define S38  0x7E
#define S39  0x7D
#define S40  0x7C




//constantes del MENU de pruebas de salidas
#define ACTIVAR_SALIDAS_IO    0x70//NOS DICE que se activa el rele de rechazo por menu de pruebas y el timer se encarga de apagarlo
#define SMART_DELAY_DURATION       5600

#define PRODUCTO_TITULO            0x40U//concepto que indica lo evidente
#define TAMAÑO_ARR_POS              6   //tamaño del array de posicion de label del display

#define SUMAR                       7  //ACcion de sumer en una funcion una variable, es un comando en una funcion 
#define RESTAR                      8 // ES UN comando en la funcion setUINT_varxDigito
//#define UNIDAD                      9  //verificar porque creo que estos no se usan
#define DECENA                      10 //Que digito vamos a modificar de la variable
#define CENTENA                     11 //UINT //verificar porque creo que estos no se usan
#define MILLAR                      12//verificar porque creo que estos no se usan
#define DECMILLAR                   13 //verificar porque creo que estos no se usan
#define BORRAR_ACTION               14 //accion de borrar en el DDS
// constantes solo para el manejo de la pantalla de DDS
#define GANANCIA_DDS               21  //VARIABLE  de GANANCIA EN PANTALL del DDS
#define ZOOM_DDS                   22 //indica que vamos a modificar o refrescar la variable zoom del DDS
#define CUADRO_DDS                 23 //INDICA que vamos a refescar el DDS porque se podifico la variable cuadro
#define SENSIBILIDAD_DDS           24 //indica que tenemos que refescar el DDSporque se modifico la sensibilidad
#define PHASE_DDS                  25
#define ALTURA_DDS                 26
#define BORRAR_DDS                 27
//#define SIZE_GRAPH_X               25 //tamaño del controlador graphico de pixeles
//#define SIZE_GRAPH_Y               16 //tamaño de Y del controlador graphico array
#define RESET_DDS_PWG              28 //se resetea el DDS direct digital syntehis  programable aveform generator  DDS_PWG
#define DDSPOINTS0                 29 //1dh
#define DDSPOINTS1                 30 //1Eh
#define DDSPOINTS2                 31 //1Fh
#define DDSPOINTS3                 32 //20h
//CONSTANTES DE DESPLIEGUE DE NUMEROS Y DATOS EN MENU DE CONFIGURACION DE VIBRACION
#define NUMDISPCV0                 33
#define NUMDISPCV01                34
#define NUMDISPCV02                35
#define NUMDISPCOMP                36
#define LABEL1_CV_X                0 //INDICACION de menu Config Vibracion={'pasar  ferrita','Completado'}
#define LABEL1_CV_Y                8 //INDICACION de menu Config Vibracion={'pasar  ferrita','Completado'}
#define _COMPLETADO_CCV            0x37 //ya se completo el calculo de angulo de vibracion?
#define CHARDISP00                 0x38//case de despliegue operativo de characteres en menus con DetectBAr
#define CHARDISP01                 0x39//case de despliegue operativo de characteres en menus con DetectBAr
#define CHARDISP02                 0x3A//case de despliegue operativo de characteres en menus con DetectBAr
#define CHARDISP03                 0x3B//case de despliegue operativo de characteres en menus con DetectBAr
#define CHARDISP04                 0x3C//case de despliegue operativo de characteres en menus con DetectBAr
#define CHARDISP05                 0x3D//case de despliegue operativo de characteres en menus con DetectBAr
#define CHARDISP06                 0x3E//case de despliegue operativo de characteres en menus con DetectBAr
#define CHARDISP10                 0x3F//case de despliegue operativo de characteres en menus con DetectBAr
#define CHARDISP11                 0x40//case de despliegue operativo de characteres en menus con DetectBAr



//ENTRADA DE SISTEMAS KONSTANTES,, Konstantes solo para switches y cases
#define _TACOMETRO_        0xA1U
#define _FOTOCELDA_        0xA2U
#define _RECHAZO_          0xA3U
#define _PRODUCTO_         0xA4U
#define _AUX1_             0xA5U
#define _AUX2_             0xA6U


//BIOS CONSTANTES-----------------------------------------
#define EEPROM_NUM_X    40 //posicion para escribir el numero des de la leenda EEPROM:
#define EEPROM_NUM_Y    7 //FILA donde se escribe la palabrea EEPROM y sus numeros 
#define NVRAM_NUM_X     40 //posiion to display num next to NVRAM
#define NVRAM_NUM_Y     8
#define _EEPROM_        115 //DEFINE QUE es para l eeprom
#define _NVRAM_         116 //define que se va a usar para la NVRAM


//SELECCION DE PRODUCTO MENU CONSTANTS
#define DISPLAY_NUM_LIST     0x44  //para seleccionar si de despliegan los numeros  o no
#define UP_SCREEN            0x45 //nos dice que el cursor va mas arriba y se tiene que recorrer lso datos de la lista
#define DOWN_SCREEN          0x46 //nos dice que el cursor va mas abajo y se tiene que recorrer la  lista de datos mas abajo
#define CANTIDAD_PRODUCTOS   MAXIMO_NUMERO_PROD// CANTIDAD de productos con los que puede
#define POSX8                8//POsicion donde se cmienzan las letras
#define BUFFER_PROD          0xF0 //variable que indica que hy que guardar el bufer en el indice
//#define SUMAR                0x56 
//#define RESTAR               0x57
#define ESCRIBIR             0x58
#define LEER                 0x59
#define INIT_NEW_PROD_MENU   0x5A
#define READ_BUFF_PROD       0x5B //INDICA QUE tenemos que leer el buffer
#define READ_LIM_INF         0x5C //inidca que leemos el limite inferior de iProduct de la ´pantalla
#define READ_LIM_SUP         0x5D


struct _Menu_{
	unsigned char debug43;//debug variable para ver donde se activan las banderas.
	union _Bytes_{
	unsigned char semMenuPendiente:1;//indica si hay un menu pendiente por ejecutar el cambio de pantalla y todo
	unsigned char graficando:1;//se esta graficando algo
	unsigned char x1:1;
	unsigned char x2:1;
	unsigned char x3:1;
	unsigned char x4:1;
	unsigned char x5:1;
	unsigned char x6:1;
	}bytes;
	struct _Contexto{
		unsigned char padre;   //contexto padre
		unsigned char Anterior;//contexto anterior
		unsigned char Actual;  //contexto Actual
		unsigned char Modificado;
	}contexto;
};

struct _KeyPad_{
	union _Bytes4_{
		unsigned char StatusByte;
		struct{
		  unsigned char enable:1;//keyboard enable IRQ
		  unsigned char x1:1;
		  unsigned char x2:1;
		  unsigned char x3:1;
		  unsigned char x4:1;
		  unsigned char x5:1;
		  unsigned char x6:1;
		  unsigned char x7:1;
		}bits;
	}Status;
	struct __vars_{
	  unsigned char Timer;//timer para habilitar la tecla 
	  unsigned char c0;//char anterior al que llego actual
	  unsigned short int timerkeypad;//running time after last rx byte

	}var;
};



//ESTRUCTURAS-DE-DATOS-DEL-MENU++++++++++++++++++++*******************************************************
struct ArbolMenu{
    unsigned char menuNodo; //nombre definido del menu
    unsigned char menuPadre;
    //const char **menuNameES;//  nombre string del menu individual en espanol
    //const char ** menuNameEN; //nombre en ingles
    //const char *size;
    //const char (*menuPosES)[6];
    unsigned char permisos; //permiso o niveles de autorizasion de cada menu
    //struct ArbolMenu *prev;
    //struct ArbolMenu *head;
    void  (*func)(void);   //apuntador a funcion QUE DESPLIAGA LA  pantalla en el idioma que es
    //struct ArbolMenu *m[]; //arreglo de apuntadores a estructuras, debe ser el ultimo porque es flexible
    void (*funcKeyUP)(void);
    void (*funcKeyDN)(void);
    void (*funcKeyRT)(void);
    void (*funcKeyLF)(void);
    void (*funcKeyEN)(void);  
};


// PROCEDIMIENTOS-+++++++++++++++++++++++++++++++++++++++++++++
void init_Menu(void);
//void initArbolMenu(void);
void MenuControl(void);
void PortalinicioDisplay(void);
void menus(unsigned char key);
void displayMenuInsight(void);
void cursorMenuControl(unsigned char keyx);
void DisplayIDcomunicaciones(void);// despliega el contexto ID  de comunicaciones
void displayIDcommNUM(void);  //despliega la hoja ejecutiva del ID DE Comunicacnioens
void DisplayControlFrecuencias(void);
//void DisplayIngenieriaGracida(void);
unsigned char  configModificacionController(unsigned char contextopadre,unsigned char contextomodificado1);
void displayMenuAjustedeProducto(void);
void DisplayMenuAjustedeSistema(void);
void DisplayMenuAdministrativo(void);
void DisplayConfigIOsistem(void);
void DisplayConfigIOsistem(void);
void DisplayEstaSeguro(void);
void displayIDcommNUM(void);
void DDS_HANDLER(void);
void DisplayMedidasVoltaje(void);
void DisplayAjusteProductoInfo(void);
void DisplayTiempoRechazo(void);
void DisplayAjusteSistemaEntradas(void);
void DisplayIDmaquina(void);
void DisplayCuentaProducto(void);
void DisplayMenuInformacionUsuario(void);
void DisplayAjusteSistemaEntradas(void);
void DisplayEstablecerIdioma(void);
void DisplayControlpassword(void);
void DisplayRelojSistema(void);//despliga el reloj del sistema
void DisplayIngenieriaGracida(void);
void displayAjusteGuardiaControl(void);
void DisplayInitMemoria_y_Restaurar(void);
void DisplayTipodeMaquina(void);
void DisplayMenuGanancia(void);
void DisplayMenuFiltro(void);
void filtroVarRotate(void);
void displayfiltroVar(void);
void DisplayIngenieriaGracida2(void);
void ReporteProductoVarRotate(void);
void DisplayAjusteParamProd(void);
void DisplayAjustesAvanzados(void);
void DisplayFrecuenciaSelect(void);
unsigned char displayFrecuenciaSelect_var(void);
void displayFrecuenciaSelect_active_var(void);
void  DisplayPruebaMenu(void);
void DisplayI2CMenu(void);
void DisplayRelojSistemaVariables(void);
void DisplayI2CeepromData(void);
unsigned char getunidadHex(unsigned char n);
unsigned char getdecenaHex(unsigned char n);
void DisplayEEPROMdata(void);
void DisplayContraseñaVars(unsigned char psw);
void rotateAjusteGuardiavar(void);
void displayControldeUsuariovar(void);
void rotateControldeUsuariovar(void);
void DisplayI2CNVRAMdata(void);
void displaySegundos(void);
void DisplaySPIMenu(void);
void sendSPI_DSScomm(void);
void DisplayAjustesAvanzadosAPP(void);
void DisplayParametrosdeDeteccion(void);
void rotate_frecuencySelect_var(void);
void displayfrecuencySelect_var(void);
//void displaySensibilidad_var(unsigned char posx,unsigned char posy); deprecated
void displayADCsValues(void);
void displayADCvalor(signed short int *v,unsigned char posx,unsigned char posy);
void convert4(unsigned char *p,signed short int b,signed short int *n, signed short int k);
void convertBTC2chars(unsigned char *p,signed short int *dato);
char getDM(signed short int *n);
void  Display_BIOS(void);
void run_Menu(void);
void display5UChars(unsigned short int posx,unsigned short int posy,unsigned char *p);
unsigned int displaytUINT_varxDigito(unsigned short int posx,unsigned short int posy,char operacion,unsigned short int digito,unsigned short int *pvar);
char displayPhase_varxDigito(unsigned short int posx, unsigned short int posy,char operacion,unsigned short int digitopos);
void displaySwitchGanancia_var(char pantalla,unsigned char operacion);
void displaySwitchCuadro_var(unsigned char operacion);
//void displayDDSsignalProcessor(void);
void convertSINT2UINT(signed short int sx,signed short int sy,unsigned short int *ux,unsigned short int *uy);
void displayDDS_Update(void);
void enable_Comms_TX(void);
void refreshDDS(unsigned char posy);
void DisplayPruebasSalidasMenu(void);
void prueba_SalidasON(unsigned char cursory,void (*func)(unsigned char));
void DisplayPruebasEntradasMenu(void);
void displayPruebasEntradas(void);
void MemoEEPROM_DDS(unsigned operacion,unsigned char contexto1,unsigned char mode);
void displaySeleccionarProducto(void);
word lenght2(unsigned char *c);
void displayNuevoProducto(void);
void displayTextoProcessor(void);
unsigned char configTextoProcessorController(unsigned char contextOrigen,unsigned char contextDestino);
unsigned char TextMasterProcessorController(unsigned char contextOrigen,unsigned char contextDestino);
void displayEEPROMdata1(unsigned char *p);
unsigned char semaforoDDS(unsigned char operacion);
void rotateVarParte11(void);
void displayvarParte11(void);
void DisplayParte11Menu(void);
void DisplayAjusteVibracionMenu(void);
void getBoxPattern(unsigned char box,unsigned char *mode,unsigned short int *x1,unsigned short int *y1,unsigned short int *x2,unsigned short int *y2);
unsigned char displayBoxDestructor(unsigned char mode);
void DisplayEstaSeguro2(void);
void displayBox(unsigned char *MutexCV);
unsigned char  displayNumsMenuConfigVibracion(unsigned char *MutexCV1);
//void displayMenuConfiguracionVibracion(void);
void pushFIFOcOP_displayUINT_var(unsigned short int posx,unsigned short int posy,unsigned short int *pn,unsigned char operacion);
void pushFIFOcOP_display5UChars(unsigned short int posx,unsigned short int posy,unsigned char *p);
void displayOperativoBarraDet_Numeros(void);
unsigned char getPosYNewProdDN(unsigned char sup,unsigned char iProd);
unsigned char getColTextProc(unsigned char dir);
void displayTextoProcessorMayusculas(void);
void displayTextoProcessorSymbol(void);
void displayTextoProcessorMinusculas(void);
char getAscii(unsigned char x,unsigned char y,unsigned char pantalla);
void DisplayNewTextProc(void);
void DisplayAjusteProductoMode(void);
void saveNewProduct(unsigned char iProd);
//void displayNuevoProductoXY(unsigned short int y);
void displaySeleccionarProductoTitle(void);
void displayBorrarProducto(void);
void displayBorrarProductoTitle(void);
void displayCopyProducto(void);
void displayCopyProductoTitle(void);
void displayPasteProductoTitle(void);
void displayAjusteProductoSelectTitle(void);
void displayAjusteProductoSelect(void);
void displayNombreProductoEdit(void);
void control_Operativo_MenuEscape(void);
void displaySwitchAnaloga(void);
void displayRotateStatusSwapSwitchAnaloga(unsigned char pos);
void displayStateSwitchAnaloga(void);
void cursorMenuControlService(unsigned char key,struct ArbolMenu *Pantalla);//(key,void (*a)())
void displayTimer_IRQ(void);
void DebugSimulationKeyboard(void);
void initArbolMenu(unsigned char destino);
unsigned char isNotEmptyAllSelectMenu(void);
unsigned char BuscarSiguienteiProduct(unsigned char direccion);
void DisplayNuevoProducto(void (*func)(void));
void DisplayPasteProduct(void);
unsigned char display_centrarNombres(unsigned char nchars);
unsigned char length3(unsigned char *c);
unsigned char length3(unsigned char *c);
unsigned long int menu_delay(unsigned char op);
void monitorActividad_IRQ(void);
void displayNameProd(unsigned char *p,unsigned char posy);
void cleanBufferText(unsigned char *p);
void displayMarcarAltura(unsigned char posx,unsigned char posy);
void displayGanancia(unsigned char posx,unsigned char posy);
void DisplayAjusteTiempoRechazo(void );
void DisplayEntradaTestProducto(void);
void DisplayEntradaFotoCel(void);
void DisplayEntradaTaco(void);
void DisplayEntradaTestRechazo(void);
void displaySensorFotoCelda(unsigned char sensor,unsigned char bloqueado);	
void DisplayEntradaAuxiliar1(void);
void DisplayEntradaAuxiliar2(void);
void displaySensorPruebaSistema(unsigned char sensor_seg,unsigned char sensor_ms,unsigned char posy);
void displayEntradaTest(unsigned char sensor,unsigned char sensorBloqueado,unsigned char seg,unsigned char mseg);
unsigned char *DebugVirtualEEPROMaux_userdef(unsigned char op,unsigned short add,unsigned char *w,unsigned short int nbytes);
void DisplayEntradaAuxiliar(unsigned char sensor,unsigned char rele,unsigned char seg,unsigned char mseg,unsigned char aux,unsigned char label);
void Cambiar_a_Portal_Inicio(void);
void DisplayMedidasVoltajes(void);
unsigned char Anti_Bounce_Keypad(unsigned char *c1);
unsigned char isEnable_Keypad(unsigned char cmd);
void DDS_Borrar_Refresh(void);
unsigned char isEnable_Keypad2(unsigned char cmd,unsigned long int maxtime);
void resetDDS_repetidos(unsigned short int n);
void procSensxDigitoEN(unsigned char *digito,unsigned char posx,unsigned char initx);
unsigned char procSensxDigitoRT(unsigned char posx,unsigned char posy,unsigned short int *Sens);
unsigned char procSensxDigitoLF(unsigned char posx,unsigned char posy,unsigned short int *Sens);
void procFasexDigitoEN(unsigned char *var,unsigned char posx,unsigned char posy);
void Cambio_de_Frecuencia_por_IIC_1(unsigned short int);
void prueba_SalidasOFF(unsigned char posy,void (*func)(unsigned char));
void keypad_Timer(void);
void DisplayMenu(void);

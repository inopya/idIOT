
/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        CONSTANTES DEL PROGRAMA
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//==========================================================
//		WACTH DOG TIMER HARDWARE
//==========================================================


#define REG_WDT        			    (0x60000900)
#define WDT_TIMEOUT_REG   		  (REG_WDT + 0x8)
#define WRITE_REG(addr, val)    (*((volatile uint32_t *)(addr))) = (uint32_t)(val)




//==========================================================
//		RED WIFI DEL PORTAL DE CONFIGURACION
//==========================================================

#define SSID_PORTAL    "INFOTEC_idIOT"    // nombre del portal de configuracion
#define PASS_PORTAL    "idiot2023"      // clave apra el portal de configuracion
// **NOTA: las credenciales de la red local deben introducirse mediante el portal de ocnfiguracion



//==========================================================
//		TELEGRAM BOT
//==========================================================

#define BOT_TOKEN   "xxxxxxxx:yyyyyyyyyyyyyyyyyyyyy"      //@mi_bot




//==========================================================
//		USUARIOS telegram
//==========================================================

#define ADMIN_USER     	 "0000000"    // usuario principal
#define OTHER_USER    	 "1111111"    // usuario extra




//==========================================================
//		OTRAS constantes
//==========================================================

#define ID_STATION     "wemos lavadora"  // ID de estacion, (ha de ser una cadena)



/* el led interno del wemos esta conectado a una salida Pull-up (enciende con LOW) */
#define ONBOARD_LED_ON    (0)    
#define ONBOARD_LED_OFF   (1)

/* Valores tipicos para estados on/off, activo/inactivo... */
#define ACTIVO	  (1)     
#define INACTIVO  (0)
#define ON	  	  (1)  
#define OFF       (0)

  

//==========================================================
//		EEPROM
//==========================================================

/* (POS_MEM...) direccion  de memoria  --> (...)  tipo de dato que almacena y detalles */

#define POS_MEM_READ_ONLY_REGISTER   	  (0) 	// (2byte) sin uso
#define POS_MEM_UMBRAL_LUZ       	  	  (2) 	// (2byte) 
#define POS_MEM_UMBRAL_OSCURIDAD       	  (4) 	// (2byte)
#define POS_MEM_NUM_PARPADEOS       	  (6) 	// (2byte)

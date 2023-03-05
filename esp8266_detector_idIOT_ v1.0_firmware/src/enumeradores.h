
/* ENUM ESTADOS DE OPERACION */
enum machineState 
{
	MODE_WAIT ,   			// tiempo de espera  para repetir notificaciones
	MODE_OFF  ,   			// modo reposo tras inicio del sistema o finalizar proceso			
	MODE_RUN  ,  
	MODE_DETECT_ON ,
	MODE_DETECT_OFF ,
	MODE_DETECT_BLINK ,
	MODE_END_TASK ,
	MODE_STOP ,   			// fin de proceso, avisar de evento por telegram 
};

 

/* ENUM COSTANTES */
enum Constantes 
{
	INTERVALO_ACCESO_SERVIDOR	=  5000,    // (en ms) tiempo entre accesos al servidor de telegram
	OFFSET_MIN					=  200,
	OFFSET_MAX					=  900,
};





//ESTE enum SE ENCUENTRA EN LOS FICHEROS DEL CORE DEL ESP8266 
//pero vscode no lo reconoce, asique lo "re-creamos"

	enum esp_hw_wdt_timeout {
							  TIMEOUT_DISABLE   = 0,
							  TIMEOUT_0_84_SEC  = 10,
							  TIMEOUT_1_68_SEC  = 11,
							  TIMEOUT_3_36_SEC  = 12,
							  TIMEOUT_6_71_SEC  = 13,
							  TIMEOUT_13_4_SEC  = 14,
							  TIMEOUT_26_8_SEC  = 15,
							};
/*
 #       _\|/_   A ver..., ¿que tenemos por aqui?
 #       (O-O)
 # ---oOO-(_)-OOo---------------------------------

 ####################################################
 # ************************************************ #
 # *   MAYORDOMO VIRTUAL CON ESP8266 Y TELEGRAM   * #
 # *     idIOT  Aplicado a lavadora domestica     * #
 # *         Autor: Eulogio Lopez Cayuela         * #
 # *      Version 1.0    Fecha: 05/12/2022        * #
 # *                                              * #
 # ************************************************ #
 ####################################################

		idIOT:   Implementing Dummy IOT

 */




/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        IMPORTACION DE LIBRERIAS
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/


#include <Arduino.h>

#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>


#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>                  //https://github.com/tzapu/WiFiManager


#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>


#include <Temporizador_inopya.h>

#include "pinout.h"
#include "definiciones.h"
#include "constantes.h"
#include "enumeradores.h"
#include "variables.h"




/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        CREACION DE OBJETOS
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

Temporizador_inopya cronometroVirtual;

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

/* gestor de conexiones y AP */
WiFiManager wifiManager;



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        PROTOTIPADO DE FUNCIONES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

void procesarTelegramas( int num_mensajes );
void enviar_cabecera( String usuario );
void eliminar_mensajes_viejos( void );
void enviar_info_luz( String );
void enviar_lista_comandos( String usuario );
void enviar_configuracion( String usuario );
void dar_hora( String usuario );

void calcular_offset( void );
void calcular_luz( void );
uint16_t control_parpadeo( void );

void update_time( void );


bool loadConfig( void );
bool saveConfig( void );


void hw_wdt_disable( void );      // Hardware WDT OFF
void hw_wdt_enable( void );       // Hardware WDT ON
void hw_wdt_timeout( uint32_t );  // Establecer TimeOut Hardware WDT




/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   ****************************************************************************************************** 
                                    FUNCION DE CONFIGURACION
   ****************************************************************************************************** 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

void setup()
{

	hw_wdt_enable();       // Hardware WDT ON
	hw_wdt_timeout( TIMEOUT_26_8_SEC );
	ESP.wdtFeed();

	EEPROM.begin(50);  //reserva 50 bytes para eeprom (ojo tambien los nececsitas de RAM para manejarlos)

	SERIAL_BEGIN(115200);
	PRINT_VERSION(F(_VERSION_));
	PRINTLN("");

	digitalWrite(PIN_LED_OnBoard, ONBOARD_LED_ON);

	secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org


	digitalWrite(PIN_LED_OnBoard, ONBOARD_LED_ON);

	  
	/* Intentar conectar i hay credenciales en la FLASH y si no... montar un AP para configuracion */ 
	wifiManager.setAPStaticIPConfig(IPAddress(192,168,5,1), IPAddress(192,168,5,1), IPAddress(255,255,255,0));
	delay(1000);
	wifiManager.autoConnect(SSID_PORTAL, PASS_PORTAL);          // portal de configuracion si no podemos conectar

	digitalWrite(PIN_LED_OnBoard, ONBOARD_LED_OFF);     
	PRINT(F("\nWiFi conectada. IP address: "));
	PRINTLN(WiFi.localIP()); PRINTLN(F("\n\n"));

	PRINTLN(F("Actualizando fecha/hora: "));
	configTime("GMT-2", "pool.ntp.org", "time.nist.gov");   //
	time_t now = time(nullptr);
	while (now < 24 * 3600) {
		PRINT(".");
		delay(1000);
		now = time(nullptr);
	}

	PRINTLN("");
	update_time();
	PRINTLN(fecha_hora);
	PRINTLN("\n");


	enviar_cabecera(ADMIN_USER);
	PRINTLN_DEBUG(F("cabecera enviada")); 

	eliminar_mensajes_viejos();
	delay(250);

	PRINTLN_DEBUG(F("mensajes viejos borrados")); 

	/* calculo del  umbral luminoso del ambiente */
	calcular_offset();

	ESP.wdtDisable();       // Software WDT OFF

	loadConfig();
	enviar_configuracion(ADMIN_USER);
	
	FLAG_reinicio = false; 
	estado_actual = user_mode;
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   ****************************************************************************************************** 
                                  BUCLE PRINCIPAL DEL PROGRAMA
   ****************************************************************************************************** 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/


void loop()
{
	ESP.wdtFeed();  //para el WDT software/hardware. Tenemos habilitado solo el WDT hardware

	uint32_t momento_actual = millis();

	if ( momento_actual - ultimo_acceso > intervalo_acceso_telegram) {
		int num_mensajes_pendientes = bot.getUpdates(bot.last_message_received + 1);

		while (num_mensajes_pendientes) {
			procesarTelegramas(num_mensajes_pendientes);
			num_mensajes_pendientes = bot.getUpdates(bot.last_message_received + 1);
		}
		ultimo_acceso = momento_actual;
	}



	/* ===== MAQUINA DE ESTADOS - (Toma de decisiones) ===== */

	switch(estado_actual)
	{
		case MODE_OFF:    //modo reposo tras inicio del sistema o finalizar proceso 
			estado_actual=MODE_OFF;
		break;
	  
	  
		case MODE_START_TASK:   //inicio de tarea, calcula luz ambiente e inicia el modo de operacion seleccionado
			calcular_offset();
			estado_actual=user_mode;
			intervalo_acceso_telegram = INTERVALO_ACCESO_LARGO;

		break; 
		
		
		case MODE_DETECT_ON:           //buscar encendido de luz
			calcular_luz();
			if( rectificado_A0 > umbral_luz ){ 
				mensaje = "LUZ DETECTADA: ";
				mensaje += String(rectificado_A0);
				estado_actual = MODE_END_TASK; 

				PRINTLN_DEBUG(mensaje);
			}
		break; 
		
		
		case MODE_DETECT_OFF:           //buscar apagado de luz
			calcular_luz();
			if( (offSet - valor_entrada_A0) > umbral_oscuro ){ 
				mensaje = "OSCURIDAD DETECTADA: ";
				mensaje += String(valor_entrada_A0 - offSet);
				estado_actual = MODE_END_TASK;

				PRINTLN_DEBUG(mensaje);
			}
		break;
		
		
		case MODE_DETECT_BLINK:           //buscar parpadeos de luz
			calcular_luz();
			control_parpadeo();
			if( contador_parpadeos > num_parpadeos ){ 
				contador_parpadeos = 0;
				mensaje = "BLINK DETECTADO: ";
				estado_actual = MODE_END_TASK; 
			}
		break;	
		
		
		case MODE_END_TASK:           //notificar el fin una tarea
			FLAG_running = false;
			mensaje += "\n\nTAREA FINALIZADA: ";
			bot.sendMessage( ADMIN_USER, mensaje);
			mensaje = "";
			PRINTLN_DEBUG(F("Notificacion Enviada"));

			if(FLAG_repetir_notificacion){ 
				cronometroVirtual.begin(TIEMPO_ESPERA_NOTIFICACIONES);
				estado_actual = MODE_WAIT;
			}
			else{ estado_actual = MODE_STOP; }
			intervalo_acceso_telegram = INTERVALO_ACCESO_CORTO;
		break;


		case MODE_WAIT:           //tiempo de espera si hay que repetir notificacion
			if ( cronometroVirtual.estado()==false ){
				PRINTLN_DEBUG(F("Fin de espera"));  
				estado_actual = MODE_END_TASK;  //para repetir la notificacion
			}
		break;


		case MODE_STOP:  // fin de notificaciones
			mensaje = "<<FIN DE EVENTO>>\nNotificaciones CANCELADAS\nEsperando nueva orden...";
			bot.sendMessage( ADMIN_USER, mensaje);
			estado_actual=MODE_OFF;
		break;


		default :
			estado_actual=MODE_OFF;
		break;       
	}

	if( estado_actual==MODE_DETECT_ON || estado_actual==MODE_DETECT_OFF || estado_actual==MODE_DETECT_BLINK ){ 
		FLAG_running = true;
	}
	else { FLAG_running = false; digitalWrite( PIN_LED_OnBoard, ONBOARD_LED_OFF);}
  

	if( FLAG_running && cronometroVirtual.estado()==false ) {
		digitalWrite( PIN_LED_OnBoard, !( digitalRead(PIN_LED_OnBoard) ) );
		cronometroVirtual.begin(TIEMPO_PARPADEO_RUNNING);
	}

}




/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx 
        BLOQUE DE FUNCIONES: LECTURAS DE SENSORES, COMUNICACION SERIE, INTERRUPCIONES...
   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    LDR
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
// calcular luz ambiente
//========================================================

void calcular_offset( void )
{
	offSet=0;

	for(uint8_t n=0;n<8;n++){
		offSet += analogRead(PIN_LDR);
		delay(125);
	}

	offSet  = (offSet >> 3);   //dividir entre 8 (entero)
}


//========================================================
// medicion luz
//========================================================

void calcular_luz( void )
{
	valor_entrada_A0 = analogRead(PIN_LDR);
	rectificado_A0= valor_entrada_A0 - offSet;
	rectificado_A0 = constrain(rectificado_A0, -1023, 1023);
}



//========================================================
// Contador de parpadeos
//========================================================

uint16_t control_parpadeo( void )
{
	//static uint16_t contador_parpadeos;  //sacada a global para poder resetearla
	static uint8_t estado_blink_anterior = OFF;
	uint8_t estado_blink_actual;
  
	if( rectificado_A0 > umbral_luz ){ estado_blink_actual = ON; }
	else{ estado_blink_actual = OFF; }
	
	if( estado_blink_actual != estado_blink_anterior ){
		contador_parpadeos++;
		estado_blink_anterior = estado_blink_actual;
	}
	
	return contador_parpadeos;
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    EEPROM
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/


//========================================================
// LEER DE EEPROM DATOS DE CUALQUIER TIPO
//========================================================
template <typename T> unsigned int eeprom_load (int _memory_pos, T& _value)
{
	byte* p = (byte*)&_value;
	uint16_t i;
	for (i = 0; i < sizeof(_value); i++)
		*p++ = EEPROM.read(_memory_pos++);
	return i;
}


//========================================================
// GRABAR EN EEPROM DATOS DE CUALQUIER TIPO
//========================================================

template <typename T> unsigned int eeprom_save(int _memory_pos, const T& _value)
{
	const byte* p = (const byte*)&_value;
	uint16_t i;
	for (i = 0; i < sizeof(_value); i++)
		EEPROM.write(_memory_pos++, *p++);
	return i;
}

//========================================================
// LEER CONFIGURACION
//========================================================

bool loadConfig( void )
{
	eeprom_load(POS_MEM_UMBRAL_LUZ, umbral_luz);
	eeprom_load(POS_MEM_UMBRAL_OSCURIDAD, umbral_oscuro);
	eeprom_load(POS_MEM_NUM_PARPADEOS, num_parpadeos);

	uint16_t variable_1;
	eeprom_load(POS_MEM_UMBRAL_LUZ, variable_1);
	
	if( variable_1!=umbral_luz ){
		PRINTLN_DEBUG(F("DEBUG: Error load EEPROM"));
		return false;
	}
  
	return true;
}


//========================================================
// SALVAR CONFIGURACION
//========================================================

bool saveConfig( void )
{
	eeprom_save(POS_MEM_UMBRAL_LUZ, umbral_luz);
	eeprom_save(POS_MEM_UMBRAL_OSCURIDAD, umbral_oscuro);
	eeprom_save(POS_MEM_NUM_PARPADEOS, num_parpadeos);
	
	EEPROM.commit();	//guarda la informacion en la EEPROM
					
	uint16_t variable_1;
	delay(100);
	eeprom_load(POS_MEM_UMBRAL_LUZ, variable_1);
	
	if( variable_1!=umbral_luz ){
		PRINTLN_DEBUG(F("DEBUG: Error save EEPROM"));
		return false;
	}
  
	return true;
}






/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    TELEGRAM
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
// PROCESAR TELEGRAMAS RECIBIDOS
//========================================================

void procesarTelegramas(int num_mensajes)
{
	for (int i = 0; i < num_mensajes; i++) {

		String from_name = bot.messages[i].from_name;    //para compatibilidad con una funcion antigua. 
		String chat_id = bot.messages[i].chat_id;        //para compatibilidad con una funcion antigua. 
		String text = bot.messages[i].text;              //para compatibilidad con una funcion antigua. 

		//if (chat_id != ADMIN_USER) { 
		  PRINT_DEBUG(F("   >>> USUARIO: ")); 
		  PRINTLN_DEBUG( chat_id );
		  PRINT_DEBUG(F("   >>> NOMBRE:  ")); 
		  PRINTLN_DEBUG( from_name );    
		  PRINT_DEBUG(F("   >>> COMANDO: ")); 
		  PRINTLN_DEBUG( text );
		  PRINTLN_DEBUG(F("----------------------\n"));
		//}

		if (text == "/start") {
			String welcome = "Bienvenido a Control idIoT, " + from_name + ".\n\n";
			welcome += "/opciones : lista de comandos\n";

			bot.sendMessage(chat_id, welcome, "Markdown");

			String keyboardJson = "[[\"/opciones\", \"/teclado\"]";
			if (chat_id==ADMIN_USER || FLAG_public_access){
				keyboardJson+=",[\"/fecha\",\"/hora\"],[\"/offset\",\"/config\",\"/luz\"]";
			 	keyboardJson+=",[\"/red\",\"/version\"],[\"/AP_reset\",\"/update\"]";
				keyboardJson+=",[\"/on\",\"/off\",\"/blink\"], [\"/repeat\",\"/stop\",\"/run\"]" ;
			}
			keyboardJson+="]";  // fin de Keyboard;
			bot.sendMessageWithReplyKeyboard(chat_id, " # Usa la lista de comandos o el teclado...", "", keyboardJson, true); 
			enviar_lista_comandos(chat_id);
		}

		else if (text == "/opciones") { enviar_lista_comandos(chat_id); }

		else if (text == "/teclado") { 
			String NokeyboardJson = "";
			String keyboardJson = "[[{\"text\":\"hora\",\"callback_data\":\"/hora\"},\
								   {\"text\":\"fecha\",\"callback_data\":\"/fecha\"}],\
								  [{\"text\":\"establecer Offset\",\"callback_data\":\"/offset\"},\
								   {\"text\":\"Configuracion\",\"callback_data\":\"/config\"},\
								   {\"text\":\"estado LDR\",\"callback_data\":\"/ldr\"}]";
								  
			if (chat_id==ADMIN_USER || FLAG_public_access){
				keyboardJson+=",[{\"text\":\"RED\",\"callback_data\":\"/red\"},\
								 {\"text\":\"VERSION\",\"callback_data\":\"/version\"}],\
								[{\"text\":\"AP Reset\",\"callback_data\":\"/AP_reset\"},\
								 {\"text\":\"Update Firmware\",\"callback_data\":\"/update\"}],\
								[{\"text\":\"Detect ON\",\"callback_data\":\"/on\"},\
								 {\"text\":\"Detect OFF\",\"callback_data\":\"/off\"},\
								 {\"text\":\"Detect BLINK\",\"callback_data\":\"/blink\"}],\
								[{\"text\":\"Notificacion\",\"callback_data\":\"/repeat\"},\
								 {\"text\":\"STOP\",\"callback_data\":\"/stop\"},\
								 {\"text\":\"RUN\",\"callback_data\":\"/run\"}]";

			}
			keyboardJson += "]";  // fin de Keyboard;
			bot.sendMessageWithInlineKeyboard(chat_id, "...","", keyboardJson);
		}    
				
		else if ( text == "/AP_reset" && ( chat_id==ADMIN_USER || FLAG_public_access) ) { 
			bot.sendMessage(chat_id, "Borrando redes WiFi,\nMontando AP y reiniciando...");
			wifiManager.resetSettings();
			delay(2000);
			ESP.restart(); 
		}
			
		else if (text == "/update" && ( chat_id==ADMIN_USER || FLAG_public_access) ) { 
			bot.sendMessage(chat_id, "Montando AP para actualizar...\nEntra en: http://192.168.5.1");
			wifiManager.setAPStaticIPConfig(IPAddress(192,168,5,1), IPAddress(192,168,5,1), IPAddress(255,255,255,0));
			wifiManager.startConfigPortal(SSID_PORTAL, PASS_PORTAL);  // forzar portal de configuracion
			delay(1000);
		}

		else if ( ( text == "/status" || text == "/estado" )  && ( chat_id==ADMIN_USER || FLAG_public_access) ) { 
			mensaje = "vilancia: ";
			mensaje += FLAG_running ? "activa" : "inactiva";
			bot.sendMessage(chat_id, mensaje);
		}
		
		else if (  text == "/public" && chat_id==ADMIN_USER ) { 
			FLAG_public_access = !FLAG_public_access;
			mensaje = "El ACCESO ahora es: ";
			mensaje += FLAG_public_access ? "PUBLICO" : "PRIVADO";
			bot.sendMessage(chat_id, mensaje);
		}
		
		else if ( text.equals("/red") && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			enviar_cabecera( chat_id );
		} 

		else if ( text.equals("/hora") || text.equals("/time") || text.equals("/fecha") || text.equals("/date") ) { 
			dar_hora( chat_id );
		}

		else if ( (text.equals("/ver") || text.equals("/version")) && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			FLAG_version=true;
			enviar_cabecera( chat_id );
		}

		else if ( (text.equals("/ldr") || text.equals("/luz") || text.equals("/info")) && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			enviar_info_luz( chat_id );
		} 
		else if ( (text.equals("/offset") ) && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			calcular_offset(); 

			mensaje = "offSet luz ambiente: ";
			mensaje += String(offSet);
			if(offSet<OFFSET_MIN) { mensaje += "\n ** Detecciones OFF pueden ser complicadas **"; }  //< >
			else if(offSet>OFFSET_MAX) { mensaje += "\n ** Detecciones ON y BLINK pueden ser complicadas **"; }
			PRINTLN_DEBUG(mensaje); 
			bot.sendMessage(chat_id, mensaje); 
			mensaje = "";

			enviar_info_luz( chat_id );
		}

		else if ( (text.equals("/on") ) && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			user_mode = MODE_DETECT_ON; 
			bot.sendMessage(chat_id, "MODO: Detectar ENCENDIDO");
		}
		else if ( (text.equals("/off") ) && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			user_mode = MODE_DETECT_OFF; 
			bot.sendMessage(chat_id, "MODO: Detectar APAGADO");
		}
		else if ( (text.equals("/blink") ) && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			user_mode = MODE_DETECT_BLINK; 
			bot.sendMessage(chat_id, "MODO: Detectar PARPADEO");
		}
		else if ( (text.equals("/run") ) && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			estado_actual = MODE_START_TASK;  
			bot.sendMessage(chat_id, "Tarea INICIADA");
		}
		else if ( (text.equals("/stop") ) && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			estado_actual = MODE_STOP; 
		}
		else if ( (text.equals("/repeat") ) && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			//if(estado_actual!=user_mode){ 
				FLAG_repetir_notificacion = !FLAG_repetir_notificacion;
				mensaje = "Repetir notificaciones: ";
				if( FLAG_repetir_notificacion ) { mensaje+="ON"; }
				else { mensaje+="OFF"; }
				PRINT_DEBUG(mensaje);
				bot.sendMessage(chat_id, mensaje);
			//}
		}
		else if ( (text.equals("/config") )  && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			//if(estado_actual!=user_mode){ 
			enviar_configuracion(chat_id);
			//}
		}
		else if ( (text.substring(0,4) == "/cfg")  && (chat_id == ADMIN_USER || FLAG_public_access) ) { 
			if(estado_actual!=user_mode){ 
				int16_t consigna = text.substring(4).toInt();
				if (user_mode == MODE_DETECT_ON) { umbral_luz = consigna; }
				if (user_mode == MODE_DETECT_OFF) { umbral_oscuro = consigna; } 
				if (user_mode == MODE_DETECT_BLINK) { num_parpadeos = consigna; } 
				saveConfig();
				bot.sendMessage( chat_id, "Nueva consigna establecida" );  
			}
		}    
		else{ bot.sendMessage( chat_id, "ok" ); }
  }
}




//========================================================
// ENVIAR CONFIGURACION ACTUAL
//========================================================

void enviar_configuracion(String usuario)
{ 
	mensaje="umbral luz: ";
	mensaje+=String(umbral_luz);
	mensaje+="\numbral oscuro: ";
	mensaje+=String(umbral_oscuro);
	mensaje+="\nnum parpadeos: ";
	mensaje+=String(num_parpadeos);
	bot.sendMessage( usuario, mensaje );  
}

//========================================================
// ENVIAR INFORMACION DE LUZ ACTUAL
//========================================================

void enviar_info_luz(String usuario)
{
	calcular_luz();
	mensaje = "OffSet: ";
	mensaje += String(offSet);
	mensaje += "  >> A0: ";

	mensaje += String(valor_entrada_A0);
	mensaje += "  -->  UMBRAL: ";
	mensaje += String(rectificado_A0);
	bot.sendMessage(usuario, mensaje);
}



//========================================================
// ENVIAR MENSAJE CON LA LSITA DE COMANDOS
//========================================================

void enviar_lista_comandos(String usuario)
{
	String info_comandos = "LISTA DE COMANDOS:\n";
	info_comandos += "/opciones : Muestra esta ayuda\n";
	info_comandos += "/teclado : Teclado en pantalla\n";
	info_comandos += "/offset : Establece luz ambiente\n";
	info_comandos += "/luz : Ver nivel luminoso actual\n";
	info_comandos += "/fecha : Muestra la fecha y la hora\n";
	info_comandos += "/hora : Muestra la fecha y la hora\n";
	info_comandos += "/repeat : On/Off Repetir Notificaciones \n";
	info_comandos += "/config : Muestra el umbral actual\n";
    info_comandos += "/cfgxxx : Modifica el umbral/num parpadeos\n";
	
	if( usuario==ADMIN_USER || FLAG_public_access ){    
		info_comandos += "/red : Informacion de CONEXION\n";
		info_comandos += "/version : Muestra la VERSION de firmware\n";
		info_comandos += "/on : detectar ENCENDIDO\n";
		info_comandos += "/off : detectar APAGADO\n";
		info_comandos += "/blink : detectar PARPADEOS\n";
		info_comandos += "/run : INICIAR tarea seleccionada\n";
		info_comandos += "/stop : CANCELAR tarea y/o notificaciones\n";

		info_comandos += "/AP_reset : RESET de credenciales wifi\n";
		info_comandos += "/update : Entra en modo UPDATE\n";
	}
  
	bot.sendMessage( usuario, info_comandos );     
}


//========================================================
// ENVIAR CABECERA CON DATOS DE CONEXION
//========================================================

void enviar_cabecera(String usuario)
{
	String local_ip = String() + WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];

	String cabecera = "";
	cabecera += "ID STATION: ";
	cabecera += ID_STATION;
	cabecera += "\n"; 
	if( FLAG_reinicio || FLAG_version ){
		cabecera += "BOT: ";
		cabecera +=_VERSION_;
		cabecera += "\n";
		cabecera += "FIRMWARE: ";
		cabecera +=_FIRMWARE_;
		cabecera += "\n";    
		if( FLAG_version ){
			FLAG_version=false;
			bot.sendMessage( usuario, cabecera );
			return;
		}
	}
	cabecera += "Conectado a: ";
	cabecera +=  WiFi.SSID();
	cabecera += "\n";
	cabecera += "IP: ";
	cabecera += local_ip;
	cabecera += "\n";
  
	if(FLAG_reinicio){
		update_time();
		cabecera += fecha_hora;
	}
	bot.sendMessage( usuario, cabecera );
}


//========================================================
// ENVIAR FECHA Y HORA DEBUG
//========================================================

void dar_hora(String usuario)
{
	update_time();
	fecha_hora = "Fecha/Hora:  " + fecha_hora;
	bot.sendMessage( usuario, fecha_hora );
}


//========================================================
// ACTUALIZAR FECHA Y HORA
//========================================================

void update_time()
{ 
	time_t now = time(nullptr);  
	const char *date_time = ctime(&now); 
	String fecha = String(date_time);

	fecha_hora="";

	fecha_hora += fecha.substring(0, 4);    //'wed '
	fecha_hora += fecha.substring(8, 11);   //'14 '
	fecha_hora += fecha.substring(4, 8);    //'Jul '
	fecha_hora += fecha.substring(20,24);   //'2021'
	fecha_hora += " / ";
	fecha_hora += fecha.substring(11, 19);  //'hora... '
}




//========================================================
// ELIMINAR TELEGRAMAS VIEJOS AL REINICIAR
//========================================================

void eliminar_mensajes_viejos()
{
	int num_mensajes_pendientes = bot.getUpdates(bot.last_message_received + 1);
	while (num_mensajes_pendientes) {
		PRINT_DEBUG(F("Borrando basura... ")); 
		PRINTLN_DEBUG(bot.messages[num_mensajes_pendientes].text);
		num_mensajes_pendientes = bot.getUpdates(bot.last_message_received + 1);
	}
}




/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//       CONTROL DEL WACTH DOG TIMER
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
// DESHABILITAR WACTH DOG TIMER HARDWARE
//========================================================

void hw_wdt_disable(){
	*((volatile uint32_t*) WDT_TIMEOUT_REG) &= ~(1); // Hardware WDT OFF
}


//========================================================
// HABILITAR WACTH DOG TIMER HARDWARE
//========================================================

void hw_wdt_enable(){
	*((volatile uint32_t*) WDT_TIMEOUT_REG) |= 1; // Hardware WDT ON
}


//========================================================
// ESTABLECER TIMEOUT PARA WACTH DOG TIMER HARDWARE
//========================================================

void hw_wdt_timeout(uint32_t _timeout)
{
	//WRITE_REG(WDT_TIMEOUT_REG, _timeout); 
	(*((volatile uint32_t *)(WDT_TIMEOUT_REG))) = (uint32_t)(_timeout);
}




//*******************************************************
//                    FIN DE PROGRAMA
//*******************************************************


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        VERSION / FIRMWARE
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define _VERSION_     "Control idIoT - Multi Modo: On,Off,Blink   Fecha: 12/05/2022"
#define _FIRMWARE_    "INOPYA:1.4-20221205\n"






/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        ALGUNAS DEFINICIONES PERSONALES PARA MI COMODIDAD AL ESCRIBIR CODIGO
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define AND         &&
#define OR          ||
#define NOT          !
#define ANDbit       &
#define ORbit        |
#define XORbit       ^
#define NOTbit       ~


#define getBit(data,y)       ((data>>y) & 1)           // Obtener el valor  del bit (data.y)
#define setBit(data,y)       data |= (1 << y)          // Poner a 1 el bit (data.y) 
#define clearBit(data,y)     data &= ~(1 << y)         // Poner a 0 el bit (data.y)
#define togleBit(data,y)     data ^= (1 << y)          // Invertir el valor del bit (data.y)
#define togleByte(data)      data = ~data              // Invertir el valor del byte (data)



#define ISNAN(X) (!((X)==(X)))



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        SERIAL PRINT
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/


#define DEBUG_PRINT_MODE    //descometar para activar serial print durante el debug


#define SERIAL_BAUD_RATE   	(115200)
#define SERIAL_END         	Serial.end
#define SERIAL_FLUSH       	Serial.flus

#define SERIAL_BEGIN  		  Serial.begin
#define PRINT_VERSION      	Serial.println
#define PRINT              	Serial.print
#define PRINTLN            	Serial.println





#ifdef DEBUG_PRINT_MODE 
  #define PRINT_DEBUG         Serial.print
  #define PRINTLN_DEBUG       Serial.println
#else
  #define PRINT_DEBUG         //
  #define PRINTLN_DEBUG       //
#endif





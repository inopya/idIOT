/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        SECCION DE DEFINICION DE VARIABLES GLOBALES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

uint32_t ultimo_acceso = 0;   // ultima vez que se recogieron mensajes del servidor
uint32_t intervalo_acceso_telegram = INTERVALO_ACCESO_CORTO;


String fecha_hora = "";
String mensaje = "";            //contiene los mensajes que se muestran o envian
volatile int dia_anterior = 0;  //sin uso

uint8_t estado_actual = MODE_OFF;    	// modo inicial, reposo    (reposo -> brillo -> color -> reposo...)
uint8_t user_mode = MODE_OFF;    		//tipo de deteccion.inicialmetne en espera de recibir orden telegram
bool FLAG_running = false;
bool FLAG_public_access = false;          // TRUE: permite el acceso a cualquier usuario

int valor_entrada_A0;
int rectificado_A0;     //contiene los valores de incremento de luz. 
int16_t offSet = 0;    //luz ambiente al iniciar, para calcualr los incrementos  

uint16_t contador_parpadeos = 0;	// contador de parpadeos detectados en el modo blink


bool FLAG_repetir_notificacion = true;  //TRUE: permite a los eventos finalizados notifcar repetidamente
int16_t umbral_luz      =   130;        // incremento de luz (respecto ambiente) considerado deteccion ON
int16_t umbral_oscuro   =   150;	    // disminucion necerasia para deteccion OFF 
int16_t num_parpadeos   =   10;         // num de parpadeos para que se de por buena un deteccion blink

bool FLAG_reinicio = true;
bool FLAG_version = false;






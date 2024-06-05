#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoOTA.h>
#define NUM_TRIACS 6
volatile int i = 0;
volatile boolean cruce_cero = false;
int Triac[12] = {25, 22, 23, 13, 21, 12, 4, 27, 26, 18, 14, 19    }; // Define los pines de los triacs
int dimValue = 80; // Valor inicial del dimming

int TriacSB[12] = {25, 22, 23, 13, 21, 27, 26, 18, 14, 19    }; // Define los pines de los triacs


int TriacD[NUM_TRIACS] = {25, 22, 23, 13, 21, 12}; // Define los pines de los triacs
int TriacI[NUM_TRIACS] = {4, 27, 26, 18, 14, 19};

volatile int dim[12]; // Define el nivel de dimming para cada triac
int T_int = 100;
int currentTriac = 0; // Variable para llevar la cuenta del triac actual
int Rele = 33;
int LuzRoja = 2;
volatile unsigned long lastTimerInterruptMillis = 0;
volatile unsigned int timerInterruptCount = 0;
volatile unsigned long lastZeroCrossMillis = 0;
volatile unsigned int zeroCrossCount = 0;

int flag = 0;
int flag4 = 0;

unsigned long previousMillis = 0;
unsigned long interval = 5;
bool increasing = true;
volatile bool aux = false;
volatile long aux2;
volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t * timer = NULL;
int flag2 = 0;

enum State {
  STATE_IDLE,
  STATE_SECUENCIA_1,
  STATE_SECUENCIA_2,
  STATE_SECUENCIA_3,
  STATE_SECUENCIA_4,
  STATE_SECUENCIA_5,
  STATE_SECUENCIA_6,
  STATE_SECUENCIA_7,
  STATE_SECUENCIA_8,
  STATE_DIM,
  STATE_DIM_FIJO,
  STATE_NUEVA_SECUENCIA,
  STATE_SECUENCIA_1_DIMMER,
  STATE_SECUENCIA_2_DIMMER,
  STATE_SECUENCIA_3_DIMMER,
  STATE_SECUENCIA_4_DIMMER,
  STATE_SECUENCIA_5_DIMMER,
  STATE_SECUENCIA_6_DIMMER,
  STATE_SECUENCIA_7_DIMMER,
  STATE_SECUENCIA_8_DIMMER,
  // Agrega aquí los otros estados...
};
struct WiFiInfo {
  const char* ssid;
  const char* password;
};

#define NUM_NETWORKS 3

// Declara la variable 'state' aquí
State state = STATE_IDLE;

WiFiInfo networks[NUM_NETWORKS] = {
  {"Barone Wifi", "baronealem560"},
  {"PANDEMONIUM", "PANDEMONIUM"}
  // Agrega aquí las otras redes Wi-Fi
};
#define BOTtoken "6605703445:AAGYnxji-z2lSjjiQc0EJZpn1IyqNMxkX4s"
#define CHAT_ID "1172351395"

#define MAX_CHAT_IDS 3 // Define el número máximo de IDs de chat que deseas enviar el mensaje
const char* CHAT_IDS[MAX_CHAT_IDS] = {
  "-4135125992",    // Tu ID de chat personal
   // El ID de chat del grupo
  // Agrega más IDs de chat aquí si lo deseas
};



WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

WebServer server(80);

int botRequestDelay = 100;
unsigned long lastTimeBotRan;
void handleRoot() {
  if (server.hasArg("secuencia")) {
    String secuencia = server.arg("secuencia");
    if (secuencia == "1") {
      state = STATE_SECUENCIA_1;
    } else if (secuencia == "2") {
      state = STATE_SECUENCIA_2;
    } else if (secuencia == "3") {
      state = STATE_SECUENCIA_3;
    } else if (secuencia == "4") {
      state = STATE_SECUENCIA_4;
    } else if (secuencia == "5") {
      state = STATE_SECUENCIA_5;
    }
    else if (secuencia == "6") {
      state = STATE_SECUENCIA_6;
    }
    else if (secuencia == "7") {
      state = STATE_SECUENCIA_7;
    }
    else if (secuencia == "8") {
      state = STATE_SECUENCIA_8;
    }
    else if (secuencia == "9") {
      state = STATE_NUEVA_SECUENCIA;  // Estado para la nueva secuencia
    }
    // Nuevas secuencias con dimmer fijo en Triacs 19 y 25
    else if (secuencia == "10") {
      state = STATE_SECUENCIA_1_DIMMER;
    } else if (secuencia == "11") {
      state = STATE_SECUENCIA_2_DIMMER;
    } else if (secuencia == "12") {
      state = STATE_SECUENCIA_3_DIMMER;
    } else if (secuencia == "13") {
      state = STATE_SECUENCIA_4_DIMMER;
    } else if (secuencia == "14") {
      state = STATE_SECUENCIA_5_DIMMER;
    } else if (secuencia == "15") {
      state = STATE_SECUENCIA_6_DIMMER;
    } else if (secuencia == "16") {
      state = STATE_SECUENCIA_7_DIMMER;
    } else if (secuencia == "17") {
      state = STATE_SECUENCIA_8_DIMMER;
    }
    // Nuevas secuencias sin funcionalidad
    else if (secuencia == "18") {
      //state = STATE_SECUENCIA_AZULES_ROJAS_1;
    } else if (secuencia == "19") {
      //state = STATE_SECUENCIA_AZULES_ROJAS_2;
    } else if (secuencia == "20") {
      //state = STATE_SECUENCIA_AZULES_ROJAS_3;
    } else if (secuencia == "21") {
      //state = STATE_SECUENCIA_TODAS_BLANCAS_DIMMER;
    }
  }

  if (server.hasArg("dimValue")) {
    String dimValueString = server.arg("dimValue");
    dimValue = dimValueString.toInt();
  }

  if (server.hasArg("dim")) {
    int dimValue = server.arg("dim").toInt();
    for (int j = 0; j < 12; j++) {
      dim[j] = 100 - dimValue;
    }
    state = STATE_DIM_FIJO; // Cambia al nuevo estado
  }

  if (server.hasArg("interval")) {
    String intervalString = server.arg("interval");
    interval = intervalString.toInt();
  }

  if (server.hasArg("dimBarra")) {
    int dimBarraValue = server.arg("dimBarra").toInt();
    dim[18] = 100 - dimBarraValue; // Triac 19
    dim[24] = 100 - dimBarraValue; // Triac 25
  }

  String html = "<!DOCTYPE HTML>";
  html += "<html><head><title>Twisted Transistors Dimming</title><style>";
  html += "body { background-color: #282a36; color: #f8f8f2; font-family: Arial, sans-serif; }";
  html += "h1 { color: #50fa7b; }";
  html += ".center { display: flex; justify-content: center; align-items: center; height: 100vh; flex-direction: column; }";
  html += ".button { background-color: #6272a4; border: none; color: white; padding: 20px 40px; text-align: center; text-decoration: none; display: inline-block; font-size: 24px; margin: 10px 5px; cursor: pointer; }";
  html += ".slider { width: 100%; }";
  html += ".button-container { display: flex; flex-wrap: wrap; justify-content: center; }";
  html += ".button-group { margin: 10px; }";
  html += ".slider-container { margin: 20px 0; }";
  html += "</style></head><body><div class='center'>";
  html += "<h1>Control de Luces en Bar Barone</h1>";

  // Botones de secuencias existentes
  html += "<div class='button-container'>";
  html += "<div class='button-group'>";
  html += "<h2>Secuencias Existentes</h2>";
  html += "<button class='button' onClick=location.href='./?secuencia=1'>Secuencia 1 (4 girando) (Funciona)</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=2'>Secuencia 2 (iguales) (Funciona)</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=3'>Secuencia 3 (de a 4 con un espaciado de 1) (Funciona)</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=4'>Secuencia 4 (Encuentro entre ellos) (Funciona)</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=5'>Secuencia 5 (Va y viene) (Funciona)</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=6'>Secuencia 6 (Izquierda y derecha en contra) (Funciona)</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=7'>Secuencia 7 (Funciona)</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=8'>Secuencia 8 (Se encuentran en el medio) (Funciona)</button>";
  html += "</div>";

  // Botones de secuencias nuevas con dimmer fijo en Triacs 19 y 25
  html += "<div class='button-group'>";
  html += "<h2>Secuencias con barra al 30%</h2>";
  html += "<button class='button' onClick=location.href='./?secuencia=10'>Secuencia 1 Dimmer ......</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=11'>Secuencia 2 Dimmer......</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=12'>Secuencia 3 Dimmer......</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=14'>Secuencia 5 Dimmer......</button>";
  //html += "<button class='button' onClick=location.href='./?secuencia=15'>Secuencia 6 Dimmer</button>";
  //html += "<button class='button' onClick=location.href='./?secuencia=16'>Secuencia 7 Dimmer</button>";
  html += "</div>";
  // Botones de nuevas secuencias sin funcionalidad
  html += "<div class='button-group'>";
  html += "<h2>Nuevas Secuencias (sin funcionalidad)</h2>";
  
  html += "<h2></h2>";
  html += "<button class='button' onClick=location.href='./?secuencia=18'>Secuencia 1 Azules y Rojas</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=19'>Secuencia 2 Azules y Rojas</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=20'>Secuencia 3 Azules y Rojas</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=21'>Secuencia Todas Blancas con Dimmerizado</button>";
  html += "</div>";
  html += "</div>";

  // Sliders
  html += "<div class='slider-container'>";
  html += "<h2>Control de Dimming</h2>";
  html += "<p>Establece el valor de dimming (0 es el valor mínimo, 100 es el valor máximo):</p>";
  html += "<p>Valor de dimming actual: <span id='dimValueValue'></span></p>";
  html += "<input type='range' min='0' max='100' value='80' class='slider' id='dimValueSlider'>";

  html += "<p>Intervalo actual: <span id='intervalValue'></span></p>";
  html += "<input type='range' min='1' max='100' value='15' class='slider' id='intervalSlider'>";

  html += "<p>Finalmente, establece el dimming fijo (0% es luz maxima, 100% es luz apagada):</p>";
  html += "<p>Dimming actual: <span id='dimValue'></span></p>";
  html += "<input type='range' min='0' max='100' value='50' class='slider' id='dimSlider'>";


  html += "<p>Control de dimmer para BARRA (no funciona todavia):</p>";
  html += "<p>Valor de dimmer actual para barra: <span id='dimBarraValue'></span></p>";
  html += "<input type='range' min='0' max='100' value='70' class='slider' id='dimBarraSlider'>";

  html += "<p>Para más información, contacta al número de WhatsApp: 2920591019</p>";
  html += "</div></body></html>";

  // Agrega un script de JavaScript para enviar una solicitud AJAX cuando se mueve un deslizador
  html += "<script>";
  html += "window.onload = function() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  document.getElementById('intervalSlider').oninput = function() {";
  html += "    document.getElementById('intervalValue').innerHTML = this.value;";
  html += "    xhr.open('GET', './?interval=' + this.value, true);";
  html += "    xhr.send();";
  html += "  };";
  html += "  document.getElementById('dimSlider').oninput = function() {";
  html += "    document.getElementById('dimValue').innerHTML = this.value;";
  html += "    xhr.open('GET', './?dim=' + this.value, true);";
  html += "    xhr.send();";
  html += "  };";
  html += "  document.getElementById('dimValueSlider').oninput = function() {";
  html += "    document.getElementById('dimValueValue').innerHTML = this.value;";
  html += "    xhr.open('GET', './?dimValue=' + this.value, true);";
  html += "    xhr.send();";
  html += "  };";
  html += "  document.getElementById('dimBarraSlider').oninput = function() {";
  html += "    document.getElementById('dimBarraValue').innerHTML = this.value;";
  html += "    xhr.open('GET', './?dimBarra=' + this.value, true);";
  html += "    xhr.send();";
  html += "  };";
  html += "}";
  html += "</script>";

  server.send(200, "text/html", html);
}

void IRAM_ATTR deteccion_Cruce_cero()
{
  if ((millis() - aux2) > 5) {
    zeroCrossCount++;//5ms of debounce
    if (cruce_cero == true) {

      for (int j = 0; j < 12; j++) {
        digitalWrite(Triac[j], LOW);
      }
      i = 0;

    }
    else {
      //digitalWrite(26, HIGH);
      cruce_cero = true;

    }
  }
  aux2 = millis();

}

void sendConnectionMessage() {
  String message = "ESP32 conectado a la red WiFi. Dirección IP: " + WiFi.localIP().toString();
  
  int i = 0;
    bot.sendMessage(CHAT_IDS[i], message, "");
     }

void IRAM_ATTR Dimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  timerInterruptCount++;
  if (cruce_cero == true ) {

    for (int j = 0; j < 12; j++) {
      if (i >= dim[j] ) {
        digitalWrite(Triac[j], HIGH);
      }
    }
    if (i >= 100) {
      i = 0;
      for (int j = 0; j < 12; j++) {
        digitalWrite(Triac[j], LOW);
      }

    }
    else {
      {
        i++;
      }
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  Serial.begin(9600);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  delay(10);
   
  // Define los pines de las luces como salida
  pinMode(Rele, OUTPUT);
  pinMode(LuzRoja, OUTPUT);
  digitalWrite(Rele, HIGH);

const char* ssid = "Barone Wifi";
  const char* password = "baronealem560";

  // Conecta a la red WiFi
  WiFi.begin(ssid, password);

  // Espera hasta que la conexión se establezca
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

  }

  server.on("/", handleRoot);
  server.begin();
  sendConnectionMessage();
  //bot.sendMessage(CHAT_ID, "Encendido antiguo. conectado a la red WiFi. Dirección IP: " + WiFi.localIP().toString(), "");

  digitalWrite(Rele, LOW);



  // El resto de tu código...
  for (int j = 0; j < 12; j++) {
    pinMode(Triac[j], OUTPUT);
  }
  for (int j = 0; j < 12; j++) {
    dim[j] = 90;
  }
  pinMode(34, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(34), deteccion_Cruce_cero, RISING);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &Dimer, true);
  timerAlarmWrite(timer, 100, true);  //RAPIDEZ JEJE ,
  timerAlarmEnable(timer);
  ArduinoOTA.begin();

}
void loopTodosJuntos() {
  unsigned long currentMillis = millis();
  static bool decrementing = true; // Variable para controlar la dirección del dimming

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (decrementing && dim[j] > dimValue) {
        dim[j]--;
      } else if (!decrementing && dim[j] < 100) {
        dim[j]++;
      }

      if (dim[j] == dimValue) {
        decrementing = false; // Cambia la dirección del dimming a incrementar
      } else if (dim[j] == 100 && !decrementing) {
        decrementing = true; // Cambia la dirección del dimming a decrementar
      }
    }
  }
}


void loopSecuenciaLineal() {
  unsigned long currentMillis = millis();
  static int currentLight = 0; // Variable para controlar la luz actual
  static bool decrementing = true; // Variable para controlar la dirección del dimming

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (j == currentLight || j == (currentLight + 6) % 12) { // Controla la luz actual
        if (decrementing && dim[j] > dimValue) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentLight = (currentLight + 1) % 6; // Pasa a la siguiente luz
        }
      } else {
        dim[j] = 100;
      }
    }
  }
}

void loop5() {
  unsigned long currentMillis = millis();
  static bool decrementing = true; // Variable para controlar la dirección del dimming

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (j == currentTriac || j == (currentTriac + 1) % 12 || j == (currentTriac + 2) % 12 || j == (currentTriac + 3) % 12) { // Cambia a +1 para cada triac
        if (decrementing && dim[j] > dimValue) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentTriac = (currentTriac + 1) % 12; // Pasa al siguiente triac
        }
      } else {
        dim[j] = 100;
      }
    }
  }
}

void loop1() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (increasing) {
      for (int j = 0; j < 12; j++) {
        if (dim[j] < 100) {
          dim[j]++;
        } else {
          increasing = false;
        }
      }
    } else {
      for (int j = 0; j < 12; j++) {
        if (dim[j] > dimValue) {
          dim[j]--;
        } else {
          increasing = true;
        }
      }
    }
  }
}

void loop2() {
  unsigned long currentMillis = millis();
  static bool decrementing = true; // Variable para controlar la dirección del dimming

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (j == currentTriac || j == (currentTriac + 2) % 12 || j == (currentTriac + 4) % 12 || j == (currentTriac + 6) % 12) { // Agrega control para dim[j+6]
        if (decrementing && dim[j] > dimValue) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentTriac = (currentTriac + 1) % 12; // Pasa al siguiente triac
        }
      } else {
        dim[j] = 100;
      }
    }
  }
}


void loop2D() {
  unsigned long currentMillis = millis();
  static bool decrementing = true; // Variable para controlar la dirección del dimming
  dim[5] = 80; // Triac 12 (índice 11)
  dim[6] = 80;  // Triac 19 (índice 5, basado en tu configuración de pines)

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (j == currentTriac || j == (currentTriac + 2) % 12 || j == (currentTriac + 4) % 12 || j == (currentTriac + 6) % 12) { // Agrega control para dim[j+6]
        if (decrementing && dim[j] > dimValue) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentTriac = (currentTriac + 1) % 12; // Pasa al siguiente triac
        }
      } else {
        dim[j] = 100;
      }
    }
  }
}

void loopSecuenciaCruzada() {
  unsigned long currentMillis = millis();
  static int currentPair = 0; // Variable para controlar el par de luces actual
  static bool decrementing = true; // Variable para controlar la dirección del dimming

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (j == currentPair || j == (11 - currentPair)) { // Controla el par de luces actual
        if (decrementing && dim[j] > dimValue) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentPair = (currentPair + 1) % 6; // Pasa al siguiente par de luces
        }
      } else {
        dim[j] = 100;
      }
    }
  }
}



void loopSecuenciaCruzadaD() {
  unsigned long currentMillis = millis();
  static int currentPair = 0; // Variable para controlar el par de luces actual
  static bool decrementing = true; // Variable para controlar la dirección del dimming
  dim[5] = 80; // Triac 12 (índice 11)
  dim[6] = 80;  // Triac 19 (índice 5, basado en tu configuración de pines)

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (j == currentPair || j == (11 - currentPair)) { // Controla el par de luces actual
        if (decrementing && dim[j] > dimValue) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentPair = (currentPair + 1) % 6; // Pasa al siguiente par de luces
        }
      } else {
        dim[j] = 100;
      }
    }
  }
}


void loopSecuenciaIdaVueltaD() {
  unsigned long currentMillis = millis();
  static int currentLight = 0; // Variable para controlar la luz actual
  static bool decrementing = true; // Variable para controlar la dirección del dimming
  static bool forward = true; // Variable para controlar la dirección de la secuencia
  dim[5] = 80; // Triac 12 (índice 11)
  dim[6] = 80;  // Triac 19 (índice 5, basado en tu configuración de pines)

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (j == currentLight || j == (currentLight + 6) % 12) { // Controla la luz actual
        if (decrementing && dim[j] > dimValue) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          if (forward) {
            currentLight = (currentLight + 1) % 6; // Pasa a la siguiente luz
          } else {
            currentLight = (currentLight - 1 + 6) % 6; // Pasa a la luz anterior
          }
        }
      } else {
        dim[j] = 100;
      }
    }

    // Cambia la dirección de la secuencia cuando llega al final
    if (currentLight == 5 && forward) {
      forward = false;
    } else if (currentLight == 0 && !forward) {
      forward = true;
    }
  }
}

void loopSecuenciaIdaVuelta() {
  unsigned long currentMillis = millis();
  static int currentLight = 0; // Variable para controlar la luz actual
  static bool decrementing = true; // Variable para controlar la dirección del dimming
  static bool forward = true; // Variable para controlar la dirección de la secuencia

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (j == currentLight || j == (currentLight + 6) % 12) { // Controla la luz actual
        if (decrementing && dim[j] > dimValue) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          if (forward) {
            currentLight = (currentLight + 1) % 6; // Pasa a la siguiente luz
          } else {
            currentLight = (currentLight - 1 + 6) % 6; // Pasa a la luz anterior
          }
        }
      } else {
        dim[j] = 100;
      }
    }

    // Cambia la dirección de la secuencia cuando llega al final
    if (currentLight == 5 && forward) {
      forward = false;
    } else if (currentLight == 0 && !forward) {
      forward = true;
    }
  }
}

void loop3() {
  unsigned long currentMillis = millis();
  static bool decrementing = true; // Variable para controlar la dirección del dimming

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 6; j++) {
      if (j == currentTriac || j == (currentTriac + 2) % 6 || j == (currentTriac + 4) % 6 || j == (currentTriac + 6) % 6) { // Agrega control para dim[j+6]
        if (decrementing && dim[TriacD[j]] > dimValue && dim[TriacI[j]] > dimValue) {
          dim[TriacD[j]]--;
          dim[TriacI[j]]--;
        } else if (!decrementing && dim[TriacD[j]] < 100 && dim[TriacI[j]] < 100) {
          dim[TriacD[j]]++;
          dim[TriacI[j]]++;
        }

        if (dim[TriacD[j]] == dimValue && dim[TriacI[j]] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[TriacD[j]] == 100 && dim[TriacI[j]] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentTriac = (currentTriac + 1) % 6; // Pasa al siguiente triac
        }
      } else {
        dim[TriacD[j]] = 100;
        dim[TriacI[j]] = 100;
      }
    }
  }
}



void GusanitoD() {
  unsigned long currentMillis = millis();
  static bool decrementing = true; // Variable para controlar la dirección del dimming
 // Ajusta el Triac 12 y 19 al 80% de dimming
  dim[5] = 80; // Triac 12 (índice 11)
  dim[6] = 80;  // Triac 19 (índice 5, basado en tu configuración de pines)

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (j == currentTriac || j == (currentTriac + 1) % 12 || j == (currentTriac + 2) % 12 || j == (currentTriac + 3) % 12) { // Agrega control para dim[j+3]
        if (decrementing && dim[j] > dimValue) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentTriac = (currentTriac + 1) % 12; // Pasa al siguiente triac
        }
      } else {
        dim[j] = 100;
      }
    }
  }
}

void Gusanito() {
  unsigned long currentMillis = millis();
  static bool decrementing = true; // Variable para controlar la dirección del dimming

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (j == currentTriac || j == (currentTriac + 1) % 12 || j == (currentTriac + 2) % 12 || j == (currentTriac + 3) % 12) { // Agrega control para dim[j+3]
        if (decrementing && dim[j] > dimValue) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentTriac = (currentTriac + 1) % 12; // Pasa al siguiente triac
        }
      } else {
        dim[j] = 100;
      }
    }
  }
}
void loopSecuenciaEncuentro() {
  unsigned long currentMillis = millis();
  static int currentLight = 0; // Variable para controlar la luz actual
  static bool decrementing = true; // Variable para controlar la dirección del dimming

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 6; j++) {
      if (j == currentLight || j == (5 - currentLight)) { // Controla la luz actual
        if (decrementing && dim[j] > dimValue && dim[j+6] > dimValue) {
          dim[j]--;
          dim[j+6]--;
        } else if (!decrementing && dim[j] < 100 && dim[j+6] < 100) {
          dim[j]++;
          dim[j+6]++;
        }

        if (dim[j] == dimValue && dim[j+6] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && dim[j+6] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentLight = (currentLight + 1) % 6; // Pasa a la siguiente luz
        }
      } else {
        dim[j] = 100;
        dim[j+6] = 100;
      }
    }
  }
}


void loopSecuenciaEncuentroD() {
  unsigned long currentMillis = millis();
  static int currentLight = 0; // Variable para controlar la luz actual
  static bool decrementing = true; // Variable para controlar la dirección del dimming

  dim[5] = 80; // Triac 12 (índice 11)
  dim[6] = 80;  // Triac 19 (índice 5, basado en tu configuración de pines)

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 6; j++) {
      if (j == currentLight || j == (5 - currentLight)) { // Controla la luz actual
        if (decrementing && dim[j] > dimValue && dim[j+6] > dimValue) {
          dim[j]--;
          dim[j+6]--;
        } else if (!decrementing && dim[j] < 100 && dim[j+6] < 100) {
          dim[j]++;
          dim[j+6]++;
        }

        if (dim[j] == dimValue && dim[j+6] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && dim[j+6] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentLight = (currentLight + 1) % 6; // Pasa a la siguiente luz
        }
      } else {
        dim[j] = 100;
        dim[j+6] = 100;
      }
    }
  }
}
void loopSecuenciaIdaVueltaDoble() {
  
  unsigned long currentMillis = millis();
  static int currentLight = 0; // Variable para controlar la luz actual
  static bool decrementing = true; // Variable para controlar la dirección del dimming
  static bool forward = true; // Variable para controlar la dirección de la secuencia

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 6; j++) {
      if (j == currentLight) { // Controla la luz actual
        if (decrementing && dim[j] > dimValue && dim[j+6] > dimValue) {
          dim[j]--;
          dim[j+6]--;
        } else if (!decrementing && dim[j] < 100 && dim[j+6] < 100) {
          dim[j]++;
          dim[j+6]++;
        }

        if (dim[j] == dimValue && dim[j+6] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && dim[j+6] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          if (forward) {
            currentLight = (currentLight + 1) % 6; // Pasa a la siguiente luz
          } else {
            currentLight = (currentLight - 1 + 6) % 6; // Pasa a la luz anterior
          }
        }
      } else {
        dim[j] = 100;
        dim[j+6] = 100;
      }
    }

    // Cambia la dirección de la secuencia cuando llega al final
    if (currentLight == 5 && forward) {
      forward = false;
    } else if (currentLight == 0 && !forward) {
      forward = true;
    }
  }
}



void loopSecuenciaIdaVueltaDobleD() {
  
  unsigned long currentMillis = millis();
  static int currentLight = 0; // Variable para controlar la luz actual
  static bool decrementing = true; // Variable para controlar la dirección del dimming
  static bool forward = true; // Variable para controlar la dirección de la secuencia
  dim[5] = 80; // Triac 12 (índice 11)
  dim[6] = 80;  // Triac 19 (índice 5, basado en tu configuración de pines)

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 6; j++) {
      if (j == currentLight) { // Controla la luz actual
        if (decrementing && dim[j] > dimValue && dim[j+6] > dimValue) {
          dim[j]--;
          dim[j+6]--;
        } else if (!decrementing && dim[j] < 100 && dim[j+6] < 100) {
          dim[j]++;
          dim[j+6]++;
        }

        if (dim[j] == dimValue && dim[j+6] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && dim[j+6] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          if (forward) {
            currentLight = (currentLight + 1) % 6; // Pasa a la siguiente luz
          } else {
            currentLight = (currentLight - 1 + 6) % 6; // Pasa a la luz anterior
          }
        }
      } else {
        dim[j] = 100;
        dim[j+6] = 100;
      }
    }

    // Cambia la dirección de la secuencia cuando llega al final
    if (currentLight == 5 && forward) {
      forward = false;
    } else if (currentLight == 0 && !forward) {
      forward = true;
    }
  }
}


void nueva_secuencia() {
  // Ajusta el Triac 12 y 19 al 80% de dimming
  dim[5] = 80; // Triac 12 (índice 11)
  dim[6] = 80;  // Triac 19 (índice 5, basado en tu configuración de pines)

  // Ejecuta la nueva secuencia
   Gusanito();
}

void loopSecuenciaIdaVueltaDoble2() {
  unsigned long currentMillis = millis();
  static int currentLight = 0; // Variable para controlar la luz actual
  static bool decrementing = true; // Variable para controlar la dirección del dimming
  static bool forward = true; // Variable para controlar la dirección de la secuencia

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 6; j++) {
      if (j == 5 || j == 11) { // Saltar los Triacs 12 (índice 11) y 19 (índice 5)
        continue;
      }
      if (j == currentLight) { // Controla la luz actual
        if (decrementing && dim[j] > dimValue && dim[j + 6] > dimValue) {
          dim[j]--;
          dim[j + 6]--;
        } else if (!decrementing && dim[j] < 100 && dim[j + 6] < 100) {
          dim[j]++;
          dim[j + 6]++;
        }

        if (dim[j] == dimValue && dim[j + 6] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && dim[j + 6] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          if (forward) {
            currentLight = (currentLight + 1) % 6; // Pasa a la siguiente luz
          } else {
            currentLight = (currentLight - 1 + 6) % 6; // Pasa a la luz anterior
          }
        }
      } else {
        dim[j] = 100;
        dim[j + 6] = 100;
      }
    }

    // Cambia la dirección de la secuencia cuando llega al final
    if (currentLight == 5 && forward) {
      forward = false;
    } else if (currentLight == 0 && !forward) {
      forward = true;
    }
  }
}



void loopSecuenciaLinealD() {
  unsigned long currentMillis = millis();
  static int currentLight = 0; // Variable para controlar la luz actual
  static bool decrementing = true; // Variable para controlar la dirección del dimming
    dim[5] = 80; // Triac 12 (índice 11)
  dim[6] = 80;  // Triac 19 (índice 5, basado en tu configuración de pines)


  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int j = 0; j < 12; j++) {
      if (j == currentLight || j == (currentLight + 6) % 12) { // Controla la luz actual
        if (decrementing && dim[j] > dimValue) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == dimValue) {
          decrementing = false; // Cambia la dirección del dimming a incrementar
        } else if (dim[j] == 100 && !decrementing) {
          decrementing = true; // Cambia la dirección del dimming a decrementar
          currentLight = (currentLight + 1) % 6; // Pasa a la siguiente luz
        }
      } else {
        dim[j] = 100;
      }
    }
  }
}


void loop() {

  ArduinoOTA.handle();
  server.handleClient();
  static State lastState = state; // Guarda el último estado
  static unsigned long lastPrintMillis = 0; // Guarda la última vez que se imprimió el dimming

  if (lastState != state) {
    for (int j = 0; j < 12; j++) {
      dim[j] = 100; // Apaga todas las luces
    }
    lastState = state; // Actualiza el último estado
  }
  switch (state) {
    case STATE_IDLE:
      // No hacer nada
      break;
    case STATE_SECUENCIA_1:

      Gusanito();
      break;
    case STATE_SECUENCIA_2:
      loopSecuenciaLineal();
      //loop1();
      break;
    case STATE_SECUENCIA_3:

      loop2();
      break;
    case STATE_SECUENCIA_4:
      //loopSecuenciaIdaVuelta();
      loop2();
      break;

    case STATE_SECUENCIA_5:
      loopSecuenciaCruzada();
      break;
    case STATE_SECUENCIA_6:

      loopSecuenciaIdaVuelta();
      break;
    case STATE_SECUENCIA_7:

          loopSecuenciaIdaVueltaDoble();
      break;
    case STATE_SECUENCIA_8:

      loopSecuenciaEncuentro();
      break;

    case STATE_DIM_FIJO:
      // Aquí puedes poner el código para manejar el dimming fijo
      break;

    case STATE_NUEVA_SECUENCIA:
      nueva_secuencia(); // Llama a la nueva secuencia
      break;
    case STATE_SECUENCIA_1_DIMMER:
      GusanitoD(); // Llama a la nueva secuencia
      break;
    case STATE_SECUENCIA_2_DIMMER:
      loopSecuenciaLinealD();
 break;
    case STATE_SECUENCIA_3_DIMMER:
      loop2D();
      break;
      case STATE_SECUENCIA_4_DIMMER:
      loopSecuenciaCruzadaD();
      break;
      case STATE_SECUENCIA_5_DIMMER:
      loopSecuenciaIdaVueltaD();
      break;
      case STATE_SECUENCIA_6_DIMMER:
      loopSecuenciaIdaVueltaDobleD();
      break;
      case STATE_SECUENCIA_7_DIMMER:
      loopSecuenciaEncuentroD();
      break;
    case STATE_DIM:
      //Serial.println("Ejecutando dimming normal...");
      // Aquí puedes poner el código para manejar el dimming normal
      break;
      // Agrega aquí los otros casos...
  }


}

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

volatile int i = 0;
volatile boolean cruce_cero = false;
int Triac[12] = {25, 26, 27, 14, 12, 13, 23, 22, 21, 19, 18, 4}; // Define los pines de los triacs
volatile int dim[12]; // Define el nivel de dimming para cada triac
int T_int = 100;
int currentTriac = 0; // Variable para llevar la cuenta del triac actual
int LuzVerde = 2;
int LuzRoja = 33;
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
  STATE_DIM,
  STATE_DIM_FIJO,
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
  {"Personal-250", "joaquin1995"},
  {"PANDEMONIUM", "PANDEMONIUM"}
  // Agrega aquí las otras redes Wi-Fi
};
#define BOTtoken "6605703445:AAGYnxji-z2lSjjiQc0EJZpn1IyqNMxkX4s"
#define CHAT_ID "1172351395"
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
  }

  if (server.hasArg("dim")) {
    int dimValue = server.arg("dim").toInt();
    for (int j = 0; j < 12; j++) {
      dim[j] = dimValue;
    }
    state = STATE_DIM_FIJO; // Cambia al nuevo estado
    Serial.println("Dimming fijo establecido en: " + String(dimValue));
  }


  String html = "<!DOCTYPE HTML>";
  html += "<html><head><title>Twisted Transistors Dimming</title><style>";
  html += "body { background-color: #000000; color: #FFFFFF; font-family: Arial, sans-serif; }";
  html += "h1 { color: #ADD8E6; }";
  html += ".center { display: flex; justify-content: center; align-items: center; height: 100vh; flex-direction: column; }";
  html += ".button { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }";
  html += "</style></head><body><div class='center'>";
  html += "<h1>Control de Luces en Bar Barone</h1>";
  html += "<p>Selecciona una secuencia:</p>";
  html += "<button class='button' onClick=location.href='./?secuencia=1'>Secuencia 1 (Todas iguales rapido)</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=2'>Secuencia 2 (Todas iguales lento)</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=3'>Secuencia 3 (1 x 1 rapido)</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=4'>Secuencia 4 (1 x 1 lento)</button>";
  html += "<button class='button' onClick=location.href='./?secuencia=5'>Secuencia 5 (Aleatoria)</button>";
  html += "<p>Establece el dimming fijo:</p>";
  html += "<input type='range' min='0' max='100' value='50' class='slider' id='dimSlider'>";
  html += "<button class='button' onClick=location.href='./?dim='+document.getElementById('dimSlider').value>Establecer Dimming</button>";
  html += "<p>Para mas informacion, contacta al numero de WhatsApp: 2920591019</p>";
  html += "</div></body></html>";
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
  delay(10);

  // Define los pines de las luces como salida
  pinMode(LuzVerde, OUTPUT);
  pinMode(LuzRoja, OUTPUT);
  digitalWrite(LuzRoja, HIGH);

  // Create a Wi-Fi access point
  WiFi.softAP("TWISTED TRANSISTOR", "12345678");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Servidor Iniciado");
  Serial.println("Ingrese desde un navegador web usando la siguiente IP:");
  Serial.println(WiFi.softAPIP()); //Obtenemos la IP

  // El resto de tu código...
  for (int j = 0; j < 12; j++) {
    pinMode(Triac[j], OUTPUT);
  }
  for (int j = 0; j < 12; j++) {
    dim[j] = 95;
  }
  pinMode(34, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(34), deteccion_Cruce_cero, RISING);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &Dimer, true);
  timerAlarmWrite(timer, 100, true);  //RAPIDEZ JEJE ,
  timerAlarmEnable(timer);
}
 void loop() {
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
      interval = 3;
      loop1();
      break;
    case STATE_SECUENCIA_2:
      interval = 7;
      loop1();
      break;
    case STATE_SECUENCIA_3:
      interval = 3;
      loop2();
      break;
    case STATE_SECUENCIA_4:
      interval = 7;
      loop2();
      break;

    case STATE_SECUENCIA_5:
      interval = 7;
      loop3();
      break;
    case STATE_DIM_FIJO:
      // Aquí puedes poner el código para manejar el dimming fijo
      break;
    case STATE_DIM:
      //Serial.println("Ejecutando dimming normal...");
      // Aquí puedes poner el código para manejar el dimming normal
      break;
      // Agrega aquí los otros casos...
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
        if (dim[j] > 0) {
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
      if (j == currentTriac) {
        if (decrementing && dim[j] > 0) {
          dim[j]--;
        } else if (!decrementing && dim[j] < 100) {
          dim[j]++;
        }

        if (dim[j] == 0) {
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
void loop3() {
  unsigned long currentMillis = millis();
  static bool decrementing = true; // Variable para controlar la dirección del dimming
  static int currentTriac = random(0, 12); // Elige un triac aleatorio al inicio

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (decrementing && dim[currentTriac] > 0) {
      dim[currentTriac]--;
    } else if (!decrementing && dim[currentTriac] < 100) {
      dim[currentTriac]++;
    }

    if (dim[currentTriac] == 0) {
      decrementing = false; // Cambia la dirección del dimming a incrementar
    } else if ( dim[currentTriac] == 100 && !decrementing) {
      decrementing = true; // Cambia la dirección del dimming a decrementar
      currentTriac = random(0, 12); // Elige un nuevo triac aleatorio
    }
  }
}

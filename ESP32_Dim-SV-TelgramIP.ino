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
String html = "";
html += "<!DOCTYPE HTML>\\n";
html += "<html>\\n";
html += "<head>\\n";
html += "  <title>Control de Luces en Bar Barone</title>\\n";
html += "  <style>\\n";
html += "    @media only screen and (max-width: 600px) {\\n";
html += "      body { font-size: 20px; }\\n";
html += "      .button { padding: 15px 30px; font-size: 20px; }\\n";
html += "    }\\n";
html += "    @media only screen and (min-width: 601px) {\\n";
html += "      body { font-size: 24px; }\\n";
html += "      .button { padding: 20px 40px; font-size: 24px; }\\n";
html += "    }\\n";
html += "    body { \\n";
html += "      background-color: #282a36; \\n";
html += "      color: #f8f8f2; \\n";
html += "      font-family: Arial, sans-serif; \\n";
html += "      text-align: center;\\n";
html += "    }\\n";
html += "    h1 { color: #50fa7b; }\\n";
html += "    .center { \\n";
html += "      display: flex; \\n";
html += "      justify-content: center; \\n";
html += "      align-items: center; \\n";
html += "      height: 100vh; \\n";
html += "      flex-direction: column; \\n";
html += "    }\\n";
html += "    .button { \\n";
html += "      background-color: #6272a4; \\n";
html += "      border: none; \\n";
html += "      color: white; \\n";
html += "      padding: 20px 40px; \\n";
html += "      text-align: center; \\n";
html += "      text-decoration: none; \\n";
html += "      display: inline-block; \\n";
html += "      font-size: 24px; \\n";
html += "      margin: 10px 5px; \\n";
html += "      cursor: pointer; \\n";
html += "    }\\n";
html += "    .slider { width: 100%; }\\n";
html += "  </style>\\n";
html += "</head>\\n";
html += "<body>\\n";
html += "  <div class='center'>\\n";
html += "    <h1>Control de Luces en Bar Barone</h1>\\n";
html += "    <!-- Agrega tus botones aquí -->\\n";
html += "    <button class='button' onClick=location.href='./?secuencia=1'>Secuencia 1 (4 girando)</button>\\n";
html += "    <!-- ... -->\\n";
html += "    <h2>Establece el valor de dimming:</h2>\\n";
html += "    <p>(0 es el valor mínimo, 100 es el valor máximo)</p>\\n";
html += "    <p>Valor de dimming actual: <span id='dimValueValue'></span></p>\\n";
html += "    <input type='range' min='0' max='100' value='80' class='slider' id='dimValueSlider'>\\n";
html += "    <h2>Ajusta el intervalo:</h2>\\n";
html += "    <p>Intervalo actual: <span id='intervalValue'></span></p>\\n";
html += "    <input type='range' min='1' max='100' value='15' class='slider' id='intervalSlider'>\\n";
html += "    <h2>Establece el dimming fijo:</h2>\\n";
html += "    <p>(0% es luz maxima, 100% es luz apagada)</p>\\n";
html += "    <p>Dimming actual: <span id='dimValue'></span></p>\\n";
html += "    <input type='range' min='0' max='100' value='50' class='slider' id='dimSlider'>\\n";
html += "    <p>Para mas informacion, contacta al numero de WhatsApp: 2920591019</p>\\n";
html += "  </div>\\n";
html += "  <script>\\n";
html += "    window.onload = function() {\\n";
html += "      var xhr = new XMLHttpRequest();\\n";
html += "      document.getElementById('intervalSlider').oninput = function() {\\n";
html += "        document.getElementById('intervalValue').innerHTML = this.value;\\n";
html += "        xhr.open('GET', './?interval=' + this.value, true);\\n";
html += "        xhr.send();\\n";
html += "      };\\n";
html += "      document.getElementById('dimSlider').oninput = function() {\\n";
html += "        document.getElementById('dimValue').innerHTML = this.value;\\n";
html += "        xhr.open('GET', './?dim=' + this.value, true);\\n";
html += "        xhr.send();\\n";
html += "      };\\n";
html += "      document.getElementById('dimValueSlider').oninput = function() {\\n";
html += "        document.getElementById('dimValueValue').innerHTML = this.value;\\n";
html += "        xhr.open('GET', './?dimValue=' + this.value, true);\\n";
html += "        xhr.send();\\n";
html += "      };\\n";
html += "    };\\n";
html += "  </script>\\n";
html += "</body>\\n";
html += "</html>\\n";


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

  for (int i = 0; i < NUM_NETWORKS; i++) {
    // Intenta conectar a la red WiFi
    WiFi.begin(networks[i].ssid, networks[i].password);


    for (int j = 0; j < 10; j++) {
      if (WiFi.status() == WL_CONNECTED) {
        break;
      }
      delay(1000);

    }
    digitalWrite(LuzRoja, HIGH);


    if (WiFi.status() == WL_CONNECTED) {
      // Si se conecta a la red WiFi, enciende la luz verde y apaga la luz roja



      break;
    } else {
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    // Si no se pudo conectar a ninguna red WiFi, enciende la luz roja y apaga la luz verde
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
    dim[j] = 95;
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
    case STATE_DIM:
      //Serial.println("Ejecutando dimming normal...");
      // Aquí puedes poner el código para manejar el dimming normal
      break;
      // Agrega aquí los otros casos...
  }


}

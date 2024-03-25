volatile int i = 0;
volatile boolean cruce_cero = false;

int Triac[6] = {25, 26, 27, 14, 4, 12}; // Define los pines de los triacs
volatile int dim[6]; // Define el nivel de dimming para cada triac
int T_int = 100;
int currentTriac = 0; // Variable para llevar la cuenta del triac actual

volatile unsigned long lastTimerInterruptMillis = 0;
volatile unsigned int timerInterruptCount = 0;
volatile unsigned long lastZeroCrossMillis = 0;
volatile unsigned int zeroCrossCount = 0;

int flag = 0;
int flag4 = 0;

unsigned long previousMillis = 0;
unsigned long interval = 40;
bool increasing = true;
volatile bool aux = false;
volatile long aux2;
volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t * timer = NULL;

int flag2 = 0;
void IRAM_ATTR deteccion_Cruce_cero()
{
  if ((millis() - aux2) > 5) {
    zeroCrossCount++;//5ms of debounce
    if (cruce_cero == true) {

      for(int j=0; j<6; j++) {
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
  
    for (int j = 0; j < 6; j++) {
      if (i >= dim[j] ) {
    digitalWrite(Triac[j], HIGH);
      }
    }
    if (i >= 100) {
      i = 0;
      for(int j=0; j<6; j++) {
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
  Serial.begin(115200);
  delay(10);
  for (int j = 0; j < 6; j++) {
    pinMode(Triac[j], OUTPUT);
  }
  for (int j = 0; j < 6; j++) {
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
    delay(50);

  // Si 'increasing' es verdadero, incrementa 'dim'
  if (increasing) {
    for (int j = 0; j < 6; j++) {
      dim[j]++;
      if (dim[j] >= 100) {
        increasing = false;
      }
    }
  } 
  // Si 'increasing' es falso, disminuye 'dim'
  else {
    for (int j = 0; j < 6; j++) {
      dim[j]--;
      if (dim[j] <= 0) {
        increasing = true;
      }
    }
  }
}

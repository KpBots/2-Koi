// Sensor de presencia
const int sensorPresence = A0;

// Sensores Infrarrojos
const int sensorLeft = 7;
const int sensorRight = 8;

// Selectores de estrategia
const int switchRight = 6;
const int switchCommon = 5;
const int switchFront = 4;
const int switchLeft = 2;

// Motor A - Izquierdo
const int motorLeftA1B = 9;
const int motorLeftA1A = 10;

// Motor B - Derecho
const int motorRightB1B = 3;
const int motorRightB1A = 11;

// Temporizadores
unsigned long previousMillis = 0UL;
unsigned long interval = 100UL; // Intervalo inicial de 0.1 segundo
int randomDirection = 0;

unsigned long actionTimer = 0UL;

unsigned long timerGoBack = 100UL;
unsigned long timerRotate180Left = 150UL;
unsigned long timerRotate180Right = 120UL;

////////////////////////////////////////////////

void setup() {
  // Sensores
  pinMode(sensorPresence, INPUT);
  pinMode(sensorLeft, INPUT);
  pinMode(sensorRight, INPUT);

  // Selectores de estrategia
  pinMode(switchRight, INPUT_PULLUP);
  pinMode(switchCommon, OUTPUT);
  pinMode(switchFront, INPUT_PULLUP);
  pinMode(switchLeft, INPUT_PULLUP);

  digitalWrite(switchCommon, LOW);

  // Motores
  pinMode(motorLeftA1B, OUTPUT);
  pinMode(motorLeftA1A, OUTPUT);
  pinMode(motorRightB1B, OUTPUT);
  pinMode(motorRightB1A, OUTPUT);

  motorsStop();

  // Usamos el LED integrado para la cuenta atrás y notificar la detección de presencia
  pinMode(13, OUTPUT);

  // Espera reglamentaria de 5"
  for (int i = 0; i < 5; i++) {
    digitalWrite(13, LOW);
    delay(500);
    digitalWrite(13, HIGH);
    delay(500);
  }

  // Los switches para la selección de estrategia utilizan lógica negada
  if (!digitalRead(switchLeft)) {
    // Si activamos ambos selectores laterales, vamos hacia atrás
    if (!digitalRead(switchRight)) {
      actionTimer = millis() + timerRotate180Left*2;
      while(millis() <= actionTimer)
        motorsRotateLeft();
    } else {
      actionTimer = millis() + timerRotate180Left;
      while(millis() <= actionTimer)
        motorsRotateLeft();
    }
  } else if (!digitalRead(switchFront)) {
    actionTimer = millis() + timerRotate180Left;
    while(millis() <= actionTimer)
      motorsMoveForward();
  } else if (!digitalRead(switchRight)) {
    actionTimer = millis() + timerRotate180Right;
    while(millis() <= actionTimer)
      motorsRotateRight();
  }
}

void loop() {
  float sensorPresenceValue = distance(analogRead(sensorPresence));
  int sensorLeftValue = digitalRead(sensorLeft);
  int sensorRightValue = digitalRead(sensorRight);
  int switchLeftValue = digitalRead(switchLeft);
  int switchFrontValue = digitalRead(switchFront);
  int switchRightValue = digitalRead(switchRight);
  unsigned long currentMillis;
  // El sensor de presencia necesita 25.2ms para realizar las lecturas
  delay(26);

  // Detectado el borde del tatami
  if (!sensorLeftValue || !sensorRightValue) {

    // Nos alejamos
    actionTimer = millis() + timerGoBack;
      while(millis() <= actionTimer)
        motorsMoveBackwards();

    // Encaramos el interior
    if (!sensorRightValue && sensorLeftValue) {
      actionTimer = millis() + timerRotate180Left;
      while(millis() <= actionTimer)
        motorsTurnLeft();
    } else if (!sensorLeftValue && sensorRightValue) {
      actionTimer = millis() + timerRotate180Right;
      while(millis() <= actionTimer)
        motorsTurnRight();
    } else {
      actionTimer = millis() + timerRotate180Left;
      while(millis() <= actionTimer)
        motorsRotateLeft();
    }
  }

  if (sensorPresenceValue > 20 && sensorPresenceValue <= 200) {
    motorsMoveForward();
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
    // Deambulamos aleatoriamente
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // Selecciona una dirección aleatoria
      randomDirection = random(0, 3);

      // Genera un nuevo intervalo aleatorio entre 0.1 y 0.5 segundos
      interval = random(100UL, 300UL);

      // Ejecuta la dirección aleatoria seleccionada
      switch (randomDirection) {
        case 0:
          motorsTurnLeft();
          break;
        case 1:
          motorsTurnRight();
          break;
        case 2:
          motorsRotateLeft();
          break;
        case 3:
          motorsRotateRight();
          break;
      }
    }
  }
}


// Conversion a distancia en mm
float distance(int raw) {
  float Vo = (5.0 * raw) / 1024.0;
  const float a = 48.375;
  const float b = 0.0675;
  float dist = 0;
  if (Vo > b) {
    dist = a / (Vo - b);
  }
  return dist;
}
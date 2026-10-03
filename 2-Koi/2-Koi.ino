#include <Arduino.h>

// Sensor de presencia y sensores de borde (activos en LOW).
const uint8_t sensorPresence = A0;
const uint8_t sensorLeft = 7;
const uint8_t sensorRight = 8;

// IRStart: la placa recibe alimentacion de A1/A2 y entrega OUT en A3.
const uint8_t irVcc = A1;
const uint8_t irGnd = A2;
const uint8_t irOut = A3;

// Selectores de estrategia (activos en LOW).
const uint8_t switchRight = 6;
const uint8_t switchCommon = 5;
const uint8_t switchFront = 4;
const uint8_t switchLeft = 2;

// Entradas del puente H: motor izquierdo y motor derecho.
const uint8_t motorLeftA1B = 9;
const uint8_t motorLeftA1A = 10;
const uint8_t motorRightB1B = 3;
const uint8_t motorRightB1A = 11;

const unsigned long presenceSampleMs = 26UL;
const unsigned long goBackMs = 100UL;
const unsigned long turnLeftMs = 150UL;
const unsigned long turnRightMs = 120UL;

enum MotionState : uint8_t {
  WAIT_FOR_START,
  OPENING,
  EDGE_BACK,
  EDGE_TURN,
  SEARCH
};

MotionState motionState = WAIT_FOR_START;
unsigned long stateStartedAt = 0UL;
unsigned long stateDurationMs = 0UL;
unsigned long lastPresenceSampleAt = 0UL;
unsigned long wanderStartedAt = 0UL;
unsigned long wanderDurationMs = 0UL;
bool startArmed = false;
bool presenceSampleValid = false;
bool opponentPresent = false;
bool wandering = false;
bool leftEdgeAtStart = false;
bool rightEdgeAtStart = false;

float distance(int raw);
void enterSearch();
void beginOpening(unsigned long now);
void beginEdgeRecovery(unsigned long now, bool leftEdge, bool rightEdge);
void updateEdgeRecovery(unsigned long now);
void updateSearch(unsigned long now);

void setup() {
  // Primero aseguramos que los motores esten parados.
  pinMode(motorLeftA1B, OUTPUT);
  pinMode(motorLeftA1A, OUTPUT);
  pinMode(motorRightB1B, OUTPUT);
  pinMode(motorRightB1A, OUTPUT);
  motorsStop();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(sensorPresence, INPUT);
  pinMode(sensorLeft, INPUT);
  pinMode(sensorRight, INPUT);

  pinMode(switchRight, INPUT_PULLUP);
  pinMode(switchFront, INPUT_PULLUP);
  pinMode(switchLeft, INPUT_PULLUP);
  digitalWrite(switchCommon, LOW);
  pinMode(switchCommon, OUTPUT);

  // Preparamos el nivel antes de poner A1/A2 como salidas para evitar pulsos.
  digitalWrite(irVcc, HIGH);
  pinMode(irVcc, OUTPUT);
  digitalWrite(irGnd, LOW);
  pinMode(irGnd, OUTPUT);
  pinMode(irOut, INPUT);
}

void loop() {
  const unsigned long now = millis();

  // OUT del IRStart es un nivel mantenido. STOP tiene prioridad absoluta.
  if (digitalRead(irOut) == LOW) {
    startArmed = true; // Exigimos haber visto reposo antes del primer START.
    if (motionState != WAIT_FOR_START) {
      motorsStop();
      motionState = WAIT_FOR_START;
    }
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }
  if (!startArmed) {
    // Un Nano que reinicia con OUT ya alto permanece parado hasta un nuevo ciclo.
    return;
  }

  if (motionState == WAIT_FOR_START) {
    beginOpening(now);
  }

  // La maniobra inicial se completa antes de atender los sensores de borde.
  // STOP se comprueba arriba en cada vuelta, tambien durante esta maniobra.
  if (motionState == OPENING) {
    if (now - stateStartedAt < stateDurationMs) return;
    enterSearch();
  }

  if (motionState == EDGE_BACK || motionState == EDGE_TURN) {
    updateEdgeRecovery(now);
    return;
  }

  const bool leftEdge = digitalRead(sensorLeft) == LOW;
  const bool rightEdge = digitalRead(sensorRight) == LOW;
  if (leftEdge || rightEdge) {
    beginEdgeRecovery(now, leftEdge, rightEdge);
    return;
  }

  updateSearch(now);
}

void enterSearch() {
  motionState = SEARCH;
  presenceSampleValid = false;
  wandering = false;
  motorsStop();
}

void beginOpening(unsigned long now) {
  stateStartedAt = now;
  motionState = OPENING;
  digitalWrite(LED_BUILTIN, LOW);

  if (digitalRead(switchLeft) == LOW) {
    if (digitalRead(switchRight) == LOW) {
      // Ambos selectores laterales: salida hacia atras.
      stateDurationMs = turnLeftMs * 2UL;
      motorsMoveBackwards();
    } else {
      stateDurationMs = turnLeftMs;
      motorsRotateLeft();
    }
  } else if (digitalRead(switchFront) == LOW) {
    stateDurationMs = turnLeftMs;
    motorsMoveForward();
  } else if (digitalRead(switchRight) == LOW) {
    stateDurationMs = turnRightMs;
    motorsRotateRight();
  } else {
    enterSearch();
  }
}

void beginEdgeRecovery(unsigned long now, bool leftEdge, bool rightEdge) {
  leftEdgeAtStart = leftEdge;
  rightEdgeAtStart = rightEdge;
  stateStartedAt = now;
  motionState = EDGE_BACK;
  digitalWrite(LED_BUILTIN, LOW);
  motorsMoveBackwards();
}

void updateEdgeRecovery(unsigned long now) {
  if (motionState == EDGE_BACK) {
    if (now - stateStartedAt < goBackMs) return;

    if (rightEdgeAtStart && !leftEdgeAtStart) {
      stateDurationMs = turnLeftMs;
      motorsTurnLeft();
    } else if (leftEdgeAtStart && !rightEdgeAtStart) {
      stateDurationMs = turnRightMs;
      motorsTurnRight();
    } else {
      stateDurationMs = turnLeftMs;
      motorsRotateLeft();
    }
    stateStartedAt = now;
    motionState = EDGE_TURN;
    return;
  }

  if (now - stateStartedAt >= stateDurationMs) {
    enterSearch();
  }
}

void updateSearch(unsigned long now) {
  // El sensor de presencia entrega una medida nueva cada ~25,2 ms.
  if (!presenceSampleValid || now - lastPresenceSampleAt >= presenceSampleMs) {
    const float measuredDistance = distance(analogRead(sensorPresence));
    opponentPresent = measuredDistance > 20.0f && measuredDistance <= 200.0f;
    lastPresenceSampleAt = now;
    presenceSampleValid = true;
  }

  if (opponentPresent) {
    motorsMoveForward();
    digitalWrite(LED_BUILTIN, HIGH);
    wandering = false;
    return;
  }

  digitalWrite(LED_BUILTIN, LOW);
  if (!wandering || now - wanderStartedAt >= wanderDurationMs) {
    switch (random(0, 4)) {
      case 0: motorsTurnLeft(); break;
      case 1: motorsTurnRight(); break;
      case 2: motorsRotateLeft(); break;
      case 3: motorsRotateRight(); break;
    }
    wanderStartedAt = now;
    wanderDurationMs = random(100UL, 300UL);
    wandering = true;
  }
}

// Conversion de la tension del sensor de presencia a distancia en mm.
float distance(int raw) {
  const float voltage = (5.0f * raw) / 1024.0f;
  const float a = 48.375f;
  const float b = 0.0675f;
  return voltage > b ? a / (voltage - b) : 0.0f;
}

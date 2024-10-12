void motorsMoveForward(){
  analogWrite(motorLeftA1A, 0);
  analogWrite(motorLeftA1B, 255);
  analogWrite(motorRightB1A, 0);
  analogWrite(motorRightB1B, 255);
}

void motorsMoveBackwards() {
  analogWrite(motorLeftA1A, 255);
  analogWrite(motorLeftA1B, 0);
  analogWrite(motorRightB1A, 255);
  analogWrite(motorRightB1B, 0);
}

void motorsTurnLeft() {
  analogWrite(motorLeftA1A, 0);
  analogWrite(motorLeftA1B, 0);
  analogWrite(motorRightB1A, 0);
  analogWrite(motorRightB1B, 255);
}

void motorsTurnRight() {
  analogWrite(motorLeftA1A, 0);
  analogWrite(motorLeftA1B, 255);
  analogWrite(motorRightB1A, 0);
  analogWrite(motorRightB1B, 0);
}

void motorsRotateLeft() {
  analogWrite(motorLeftA1A, 255);
  analogWrite(motorLeftA1B, 0);
  analogWrite(motorRightB1A, 0);
  analogWrite(motorRightB1B, 255);
}

void motorsRotateRight() {
  analogWrite(motorLeftA1A, 0);
  analogWrite(motorLeftA1B, 255);
  analogWrite(motorRightB1A, 255);
  analogWrite(motorRightB1B, 0);
}

void motorsStop() {
  analogWrite(motorLeftA1A, 0);
  analogWrite(motorLeftA1B, 0);
  analogWrite(motorRightB1A, 0);
  analogWrite(motorRightB1B, 0);
}

void motorsMoveCustom(int A1A, int A1B, int B1A, int B1B) {
  analogWrite(motorLeftA1A, A1A);
  analogWrite(motorLeftA1B, A1B);
  analogWrite(motorRightB1A, B1A);
  analogWrite(motorRightB1B, B1B);
}

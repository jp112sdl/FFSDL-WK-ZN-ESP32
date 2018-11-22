//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

void isrZiel1Stop() {
  Ziel[0].StopMillis = millis();
  Ziel[0].isRunning = false;
}

void isrZiel2Stop() {
  Ziel[1].StopMillis = millis();
  Ziel[1].isRunning = false;
}

void isrZiel3Stop() {
  Ziel[2].StopMillis = millis();
  Ziel[2].isRunning = false;
}

void isrZiel4Stop() {
  Ziel[3].StopMillis = millis();
  Ziel[3].isRunning = false;
}

void isrStart() {
  startPressed = true;
}

void isrReset() {
  resetPressed = true;
}

void initISR() {
  attachInterrupt(START_PIN, isrStart, FALLING);
  attachInterrupt(RESET_PIN, isrReset, FALLING);
  attachInterrupt(ZIEL1_STOP_PIN, isrZiel1Stop, FALLING);
  attachInterrupt(ZIEL2_STOP_PIN, isrZiel2Stop, FALLING);
  attachInterrupt(ZIEL3_STOP_PIN, isrZiel3Stop, FALLING);
  attachInterrupt(ZIEL4_STOP_PIN, isrZiel4Stop, FALLING);
  Serial.println("ISR Init done.");
}
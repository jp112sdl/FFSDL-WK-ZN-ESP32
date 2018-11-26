//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

void IRAM_ATTR isrZiel1Stop() {
  if (Ziel[0].isRunning)
    Ziel[0].StopMillis = millis();
  Ziel[0].isRunning = false;
}

void IRAM_ATTR isrZiel2Stop() {
  if (Ziel[1].isRunning)
    Ziel[1].StopMillis = millis();
  Ziel[1].isRunning = false;
}

void IRAM_ATTR isrZiel3Stop() {
  if (Ziel[2].isRunning)
    Ziel[2].StopMillis = millis();
  Ziel[2].isRunning = false;
}

void IRAM_ATTR isrZiel4Stop() {
  if (Ziel[3].isRunning)
    Ziel[3].StopMillis = millis();
  Ziel[3].isRunning = false;
}

void IRAM_ATTR isrStart() {
  startPressed = true;
}

void IRAM_ATTR isrReset() {
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

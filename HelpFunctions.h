//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

#ifndef __HELPFUNCTIONS__H_
#define __HELPFUNCTIONS__H_

void invalidateBahn(uint8_t bahn) {
  if (activeRunningCount > 0) {
    Bahn[bahn - 1].Valid = false;

    Ziel[((bahn - 1) * 2)].isRunning = false;
    Ziel[((bahn - 1) * 2) + 1].isRunning = false;
    Ziel[((bahn - 1) * 2)].StopMillis = startMillis;
    Ziel[((bahn - 1) * 2) + 1].StopMillis = startMillis;
    LOG("invalidateBahn" + String(bahn));
  } else {
    LOG("invalidateBahn"+ String(bahn)+" -> activeRunningCount=0, nothing to invalidate" );
  }
}

bool ZieleOK() {
  bool Bahn1OK = (digitalRead(ZIEL_STOP_PINS[0]) == HIGH && digitalRead(ZIEL_STOP_PINS[1]) == HIGH);
  bool Bahn2OK = (Bahn[1].Enabled == true) ? (digitalRead(ZIEL_STOP_PINS[2]) == HIGH && digitalRead(ZIEL_STOP_PINS[3]) == HIGH) : true;
  return ( Bahn1OK == true && Bahn2OK == true );
}

bool ZielOK(uint8_t num) {
  bool ok = (digitalRead(ZIEL_STOP_PINS[num]) == HIGH);
  //LOG("ZielOK("+String(num+1)+") = "+String(ok));
  return ok;
}

static inline String millis2Anzeige(unsigned long _millis) {
  uint16_t millisekunde = _millis % 1000;
  uint8_t sekunde = (_millis / 1000) % 60;
  uint8_t minute  = (_millis / 60000) % 60;
  String _t = (minute < 10) ? "0" + String(minute) : String(minute);
  _t += ":";
  _t += (sekunde < 10) ? "0" + String(sekunde) : String(sekunde);
  _t += ",";
  _t += (millisekunde < 100) ? ((millisekunde < 10) ?  "00" + String(millisekunde) : "0" + String(millisekunde)) : String(millisekunde);
  return _t;
}

void blinkStatusLed(uint8_t LED_PIN, uint8_t cnt) {
  for (uint8_t i = 0; i < cnt; i++) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(300);
  }
  delay(300);
  digitalWrite(LED_PIN, LOW);
}

void checkHupe() {
  static unsigned long lasteHupeMillis = 0;
  if (millis() - lasteHupeMillis > HUPE_DAUER_MS) {
    lasteHupeMillis = millis();
    if (hupe > 0) {
      hupe--;
      digitalWrite(HUPE_PIN, !digitalRead(HUPE_PIN));
      //Serial.println("HUPE TOGGLE " + String(millis()));
    } else {
      digitalWrite(HUPE_PIN, LOW);
      //Serial.println("HUPE AUS " + String(millis()));
    }
  }
}
#endif

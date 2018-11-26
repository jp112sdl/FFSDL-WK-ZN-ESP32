String millis2Anzeige(unsigned long _millis) {
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

#ifndef __LCD__H_
#define __LCD__H_

void printLcdTime(uint8_t lcdNum, unsigned long t) {
#ifndef NOLCD
  LCD[lcdNum].setCursor(0, 1);
  LCD[lcdNum].print(millis2Anzeige(t) + " m:s,ms");
#endif
}

void initLCD() {
  Ziel[0].Headline = ZIEL1_HEADLINE;
  Ziel[1].Headline = ZIEL2_HEADLINE;
  Ziel[2].Headline = ZIEL3_HEADLINE;
  Ziel[3].Headline = ZIEL4_HEADLINE;
#ifndef NOLCD
  for (uint8_t i = 0; i < ZIEL_COUNT; i++) {
    Serial.println("Ziel " + String(i + 1) + " " + (Ziel[i].Enabled == true ? "" : "de") + "aktiviert");
    LCD[i].init();
    LCD[i].clear();
    LCD[i].noBacklight();
    if (Ziel[i].Enabled) {
      LCD[i].backlight();
      LCD[i].setCursor(0, 0);
      LCD[i].print(Ziel[i].Headline);
      LCD[i].setCursor(0, 1);
      LCD[i].print(millis2Anzeige(0)+" m:s,ms");
    }
  }
  Serial.println("LCD Init done.");
#endif
}
#endif

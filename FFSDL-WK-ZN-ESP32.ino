//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <LiquidCrystal_I2C.h>
#include "FS.h"
#include "SPIFFS.h"
#include <Wire.h>
TwoWire RTCWire = TwoWire(1);
#define RTC_I2C_ADDRESS 0x68
#define RTC_I2C_SCL_PIN   4
#define RTC_I2C_SDA_PIN  15

#include "secrets.h"
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PSK;

//#define NOLCD  //nur für Debugging ohne angeschlossene LCD Displays

#define ZIEL_COUNT        4

#define START_PIN        14
#define RESET_PIN        19

#define ZIEL1_ENABLE_PIN 33
#define ZIEL1_STOP_PIN   16
#define ZIEL2_ENABLE_PIN 25
#define ZIEL2_STOP_PIN   17
#define ZIEL3_ENABLE_PIN 26
#define ZIEL3_STOP_PIN   5
#define ZIEL4_ENABLE_PIN 27
#define ZIEL4_STOP_PIN   18

#define HUPE_PIN         32
#define DELETE_CSV_PIN   12
#define STATUS_LED1_PIN  34
#define STATUS_LED2_PIN  39

#define HUPE_DAUER_MS    500

#define LCD_COLUMNS     16
#define LCD_ROWS        2
#define ZIEL1_HEADLINE "BahnI,Ziel <-Li "
#define ZIEL2_HEADLINE "BahnI,Ziel Re-> "
#define ZIEL3_HEADLINE "BahnII,Ziel <-Li"
#define ZIEL4_HEADLINE "BahnII,Ziel Re->"
LiquidCrystal_I2C lcd1(0x27, LCD_COLUMNS, LCD_ROWS); //eins links
LiquidCrystal_I2C lcd2(0x26, LCD_COLUMNS, LCD_ROWS); //eins rechts
LiquidCrystal_I2C lcd3(0x25, LCD_COLUMNS, LCD_ROWS); //zwei links
LiquidCrystal_I2C lcd4(0x24, LCD_COLUMNS, LCD_ROWS); //zwei rechts
LiquidCrystal_I2C LCD[ZIEL_COUNT] = { lcd1, lcd2, lcd3, lcd4 };

AsyncWebServer webServer(80);
DNSServer dnsServer;

#define WEBPAGE_REFRESH_TIME        "2" //alle x Sekunden wird die Seite aktualisiert

#define CSV_FILENAME                "/zeiten.csv"
#define CSV_CREATE_BACKUP_ON_DELETE true
#define CSV_HEADER                  "Datum;Uhrzeit;BahnI li;BahnI re;BahnII li;BahnII re;"

enum SPIFFS_ERRORS {
  NO_ERROR             = 0,
  DELETE_FAILED        = 10,
  RENAME_FAILED        = 11,
  FILE_DOES_NOT_EXIST  = 2,
  RENAME_SUCCESSFUL    = 4,
  SPIFFS_NOT_AVAILABLE = 99
};

volatile unsigned long startMillis  = 0;
volatile bool          resetPressed = false;
volatile bool          startPressed = false;
uint8_t          activeRunningCount = 0;
uint8_t      lastActiveRunningCount = 0;
unsigned long            lastMillis = 0;
uint8_t                        hupe = 0;
bool                spiffsAvailable = false;
bool                      noSaveCSV = false;

typedef struct {
  volatile unsigned long StopMillis = 0;
  volatile bool          isRunning  = 0;
  String                 Headline   = "";
  bool                   Enabled    = false;
} zielType;
zielType Ziel[ZIEL_COUNT];

#include "HelpFunctions.h"
#include "ISR.h"
#include "RTC.h"
#include "File.h"
#include "LCD.h"

void setup() {
  Serial.begin(57600);
  Serial.println();
  Serial.println("Starte...");
  pinMode(START_PIN,        INPUT_PULLUP);
  pinMode(RESET_PIN,        INPUT_PULLUP);
  pinMode(ZIEL1_STOP_PIN,   INPUT_PULLUP);
  pinMode(ZIEL1_ENABLE_PIN, INPUT_PULLUP);
  pinMode(ZIEL2_STOP_PIN,   INPUT_PULLUP);
  pinMode(ZIEL2_ENABLE_PIN, INPUT_PULLUP);
  pinMode(ZIEL3_STOP_PIN,   INPUT_PULLUP);
  pinMode(ZIEL3_ENABLE_PIN, INPUT_PULLUP);
  pinMode(ZIEL4_STOP_PIN,   INPUT_PULLUP);
  pinMode(ZIEL4_ENABLE_PIN, INPUT_PULLUP);
  pinMode(DELETE_CSV_PIN  , INPUT_PULLUP);
  pinMode(HUPE_PIN,         OUTPUT);
  pinMode(STATUS_LED1_PIN,  OUTPUT);
  pinMode(STATUS_LED2_PIN,  OUTPUT);

  digitalWrite(HUPE_PIN, LOW);
  digitalWrite(STATUS_LED1_PIN, LOW);
  digitalWrite(STATUS_LED2_PIN, LOW);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    digitalWrite(STATUS_LED2_PIN, HIGH);
    return;
  } else {
    spiffsAvailable = true;
    Serial.println("SPIFFS Init done.");
  }

  if (digitalRead(DELETE_CSV_PIN) == LOW) {
    deleteCSV(CSV_FILENAME, CSV_CREATE_BACKUP_ON_DELETE);
  }

  Ziel[0].Enabled = (digitalRead(ZIEL1_ENABLE_PIN) == LOW);
  Ziel[1].Enabled = (digitalRead(ZIEL2_ENABLE_PIN) == LOW);
  Ziel[2].Enabled = (digitalRead(ZIEL3_ENABLE_PIN) == LOW);
  Ziel[3].Enabled = (digitalRead(ZIEL4_ENABLE_PIN) == LOW);

  initRTC();
  initLCD();
  initISR();

  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  dnsServer.start(53, "*", IP);

  initWebServer();

  digitalWrite(STATUS_LED1_PIN, HIGH);
  Serial.println("Ready.");
}

void loop() {
  dnsServer.processNextRequest();

  //RESET Taster wurde betätigt
  if (resetPressed) {
    resetPressed = false;
    startMillis = 0;
    for (uint8_t i = 0; i < ZIEL_COUNT; i++) {
      if (Ziel[i].Enabled) {
        Ziel[i].StopMillis = 0;
        Ziel[i].isRunning = false;
        printLcdTime(i, 0);
      }
    }
    Serial.println("RESET wurde betätigt!");
    noSaveCSV = true;
    hupe = 4;
  }

  //prüfen, ob alle Ziele gestoppt wurden
  activeRunningCount = 0;
  for (uint8_t _ziel = 0; _ziel < ZIEL_COUNT; _ziel++)
    if (Ziel[_ziel].isRunning) activeRunningCount++;

  //START-Klappe wurde betätigt
  if (startPressed) {
    startPressed = false;
    if (activeRunningCount == 0) {
      if (startMillis == 0) {
        startMillis = millis();
        Serial.println("START wurde betätigt!");
      }
      for (uint8_t i = 0; i < ZIEL_COUNT; i++) {
        if (Ziel[i].Enabled && Ziel[i].StopMillis == 0)
          Ziel[i].isRunning = true;
      }
      noSaveCSV = false;
      hupe = 2;
    }
  }

  //Laufende Zeitmessung
  if (startMillis > 0)  {
    for (uint8_t _ziel = 0; _ziel < ZIEL_COUNT; _ziel++) {
      if (Ziel[_ziel].Enabled) {
        printLcdTime(_ziel, ((Ziel[_ziel].isRunning) ? millis() : Ziel[_ziel].StopMillis) - startMillis);
      }
    }
  }

  // Alle Ziele gestoppt. Jetzt CSV speichern!
  if (activeRunningCount == 0 && lastActiveRunningCount > 0) {
    if (noSaveCSV == true) {
      Serial.println("- Fehlstart. CSV wird nicht geschrieben!");
      noSaveCSV = false;
    } else {
      String csvLine = strRTCDate() + ";" + strRTCTime() + ";";
      for (uint8_t i = 0; i < ZIEL_COUNT; i++) {
        if (Ziel[i].Enabled) {
          csvLine += ((Ziel[i].StopMillis > 0) ? millis2Anzeige(Ziel[i].StopMillis - startMillis) : "0") + ";";
        } else {
          csvLine += millis2Anzeige(0)+";";
        }
      }
      writeCSV(CSV_FILENAME, csvLine);
    }
  }

  lastActiveRunningCount = activeRunningCount;

  //Hupe
  checkHupe();

  //Debugmeldungen alle 2 Sekunden
  if (millis() - lastMillis > 2000) {
    lastMillis = millis();
    //Serial.println("Uhrzeit: " + strRTCDateTime());
    //Serial.println(String(activeRunningCount) + " Ziel(en) aktiv");
    //Serial.println("DELETE CSV PIN = "+String(digitalRead(DELETE_CSV_PIN)));
  }
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


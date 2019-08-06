//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//
//34, 35, 36, 39 only input pins
//ESP32 Wrover Module
//FLash Mode: DOUT
//CPU 240MHz
//PSRAM Disabled
//Flash Size 4Mb(32Mb)
//AsyncTCP Commit [ff5c8b2]01db9cf9cea0d62e42d6c1a62dbd4b53d


#include <WiFi.h>
#include <WiFiUdp.h>
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
const char* ssid = WIFI_SSID;
const char* password = WIFI_PSK;
#define AP_CHANNEL 3

//#define NOLCD  //nur für Debugging ohne angeschlossene LCD Displays

#define ZIEL_COUNT        4
#define BAHN_COUNT        2

#define START_PIN        14
#define RESET_PIN        19
#define TIMER_PIN        34

#define BAHN2_ENABLE_PIN 27

const uint8_t ZIEL_STOP_PINS[] =  {36, 39, 5, 18};

#define BAHN1_INVALID    35
#define BAHN2_INVALID    13

#define HUPE_PIN         32
#define DELETE_CSV_PIN   12
#define STATUS_LED1_PIN  33
#define STATUS_LED2_PIN  25
#define ZIELE_OK_PIN     26

#define EXTSERIALRX_PIN   2
#define EXTSERIALTX_PIN  23

#define HUPE_DAUER_MS    200

#define COUNTDOWNTIMER_SECONDS 300
#define RESULTTIME_SECONDS       3

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

#define UDPPORT         112
#define UDPSENDCOUNT      2
struct udp_t {
  WiFiUDP UDP;
  char incomingPacket[255];
} UDPClient;
IPAddress LEDPanel_IP(192, 168, 4, 112);

#define EXTSERIALSENDCOUNT      2
#define EXTSERIALBAUDRATE       9600

#define USE_LEDPANEL_SERIAL


#define WEBPAGE_REFRESH_TIME        "2" //alle x Sekunden wird die Seite aktualisiert

#define CSV_FILENAME                "/zeiten.csv"
#define CSV_CREATE_BACKUP_ON_DELETE true
#define CSV_HEADER                  "Datum;Uhrzeit;BahnI li;BahnI re;BahnII li;BahnII re;"

enum SPIFFS_ERRORS {
  NO_ERROR = 0,
  DELETE_FAILED = 10,
  RENAME_FAILED = 11,
  FILE_DOES_NOT_EXIST = 2,
  RENAME_SUCCESSFUL = 4,
  SPIFFS_NOT_AVAILABLE = 99
};

volatile unsigned long startMillis                      = 0;
volatile bool          resetPressed                     = false;
volatile bool          timerPressed                     = false;
volatile bool          startPressed                     = false;
volatile bool          invalidateBahn1Pressed           = false;
volatile bool          invalidateBahn2Pressed           = false;
uint8_t                activeRunningCount               = 0;
uint8_t                lastActiveRunningCount           = 0;
uint8_t                hupe                             = 0;
unsigned long          lastDebugMillis                  = 0;
unsigned long          lastResultOnLEDPanelChangeMillis = 0;
bool                   spiffsAvailable                  = false;
bool                   noSaveCSV                        = false;
bool                   showResultOnLEDPanel             = false;

typedef struct {
  volatile unsigned long StopMillis = 0;
  volatile bool isRunning = 0;
  String Headline = "";
  bool Enabled = false;
} zielType;
zielType Ziel[ZIEL_COUNT];

typedef struct {
  unsigned long SlowestRun = 0;
  bool Valid = true;
  bool Enabled = false;
} bahnType;
bahnType Bahn[BAHN_COUNT];

#include "RTC.h"
#include "Log.h"
#include "HelpFunctions.h"
#include "ISR.h"
#include "File.h"
#include "LCD.h"
#include "LEDPanelControl.h"
#include "Web.h"

void setup() {
  Serial.begin(57600);
  initLog();
#ifdef USE_LEDPANEL_SERIAL
  Serial1.begin(EXTSERIALBAUDRATE, SERIAL_8N1, EXTSERIALRX_PIN, EXTSERIALTX_PIN);
#endif
  LOG("Starte...");
  pinMode(START_PIN, INPUT_PULLUP);
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(TIMER_PIN, INPUT);
  pinMode(BAHN2_ENABLE_PIN, INPUT_PULLUP);

  for (uint8_t p = 0; p < sizeof(ZIEL_STOP_PINS); p++)
    pinMode(ZIEL_STOP_PINS[p], INPUT_PULLUP);

  pinMode(DELETE_CSV_PIN, INPUT_PULLUP);
  pinMode(HUPE_PIN, OUTPUT);
  pinMode(STATUS_LED1_PIN, OUTPUT);
  pinMode(STATUS_LED2_PIN, OUTPUT);
  pinMode(ZIELE_OK_PIN, OUTPUT);
  pinMode(BAHN1_INVALID, INPUT);
  pinMode(BAHN2_INVALID, INPUT_PULLUP);

  digitalWrite(HUPE_PIN, LOW);
  digitalWrite(STATUS_LED1_PIN, LOW);
  digitalWrite(STATUS_LED2_PIN, LOW);
  digitalWrite(ZIELE_OK_PIN, LOW);

  initRTC();

  if (!SPIFFS.begin(true)) {
    LOG("SPIFFS Mount Failed");
    digitalWrite(STATUS_LED2_PIN, HIGH);
    return;
  } else {
    spiffsAvailable = true;
    LOG("SPIFFS Init done.");
  }

  if (digitalRead(DELETE_CSV_PIN) == LOW) {
    deleteCSV(CSV_FILENAME, CSV_CREATE_BACKUP_ON_DELETE);
  }

  Bahn[0].Enabled = true;
  Ziel[0].Enabled = true;
  Ziel[1].Enabled = true;
  Bahn[1].Enabled = (digitalRead(BAHN2_ENABLE_PIN) == LOW);
  Ziel[2].Enabled = (digitalRead(BAHN2_ENABLE_PIN) == LOW);
  Ziel[3].Enabled = (digitalRead(BAHN2_ENABLE_PIN) == LOW);
  LOG("Bahn 2 ist" + String((Bahn[1].Enabled == true) ? " " : " nicht ") + "aktiviert");

  initLCD();
  initISR();

  WiFi.softAP(ssid, password, AP_CHANNEL, 0, 4);

  IPAddress IP = WiFi.softAPIP();

  dnsServer.start(53, "*", IP);

  initWebServer();

  digitalWrite(STATUS_LED1_PIN, HIGH);

#ifdef USE_LEDPANEL_SERIAL
  sendDataToLEDPanel("clear");
#endif
  LOG("Ready.");
}

void loop() {
  dnsServer.processNextRequest();

  //RESET Taster wurde betätigt
  if (resetPressed) {
    delay(500);
    resetPressed = false;
    startMillis = 0;
    for (uint8_t i = 0; i < ZIEL_COUNT; i++) {
      if (Ziel[i].Enabled) {
        Ziel[i].StopMillis = 0;
        Ziel[i].isRunning = false;
        printLcdTime(i, 0);
      }
    }

    for (uint8_t i = 0; i < BAHN_COUNT; i++) {
      Bahn[i].SlowestRun = 0;
      Bahn[i].Valid = true;
    }
    showResultOnLEDPanel = false;
    LOG("RESET wurde betaetigt!");
    noSaveCSV = true;
    sendDataToLEDPanel("clear");
    hupe = 2;
  }

  //prüfen, ob eine Bahn per Taster ungültig gemacht wurde
  if (invalidateBahn1Pressed == true) {
    invalidateBahn1Pressed = false;
    invalidateBahn(1);
  }

  if (invalidateBahn2Pressed == true) {
    invalidateBahn2Pressed = false;
    invalidateBahn(2);
  }
  
  //prüfen, ob alle Ziele gestoppt wurden
  activeRunningCount = 0;
  for (uint8_t _ziel = 0; _ziel < ZIEL_COUNT; _ziel++)
    if (Ziel[_ziel].isRunning)
      activeRunningCount++;

  //START-Klappe wurde betätigt
  if (startPressed) {
    startPressed = false;
    if (ZieleOK() == true) {
      if (activeRunningCount == 0) {
        if (startMillis == 0) {
          startMillis = millis();
          sendDataToLEDPanel("run");
          for (uint8_t i = 0; i < BAHN_COUNT; i++)
            Bahn[i].SlowestRun = 0;
          LOG("START wurde betaetigt!");
        }
        for (uint8_t i = 0; i < ZIEL_COUNT; i++) {
          if (Ziel[i].Enabled && Ziel[i].StopMillis == 0)
            Ziel[i].isRunning = true;
        }
        noSaveCSV = false;
      }
    }
  }

  //(Countdown)TIMER-Taster wurde gedrückt
  if (timerPressed) {
    delay(500);
    timerPressed = false;
    if (activeRunningCount == 0) {
      showResultOnLEDPanel = false;
      sendDataToLEDPanel("timerstart" + String(COUNTDOWNTIMER_SECONDS));
    }
  }

  //Laufende Zeitmessung
  if (startMillis > 0) {
    for (uint8_t _ziel = 0; _ziel < ZIEL_COUNT; _ziel++) {
      if (Ziel[_ziel].Enabled) {
        printLcdTime(_ziel, ((Ziel[_ziel].isRunning) ? millis() : Ziel[_ziel].StopMillis) - startMillis);
      }
    }
  }

  // Alle Ziele gestoppt. Jetzt Zeit an LED Panel übertragen und CSV speichern!
  if (activeRunningCount == 0 && lastActiveRunningCount > 0) {
    if (noSaveCSV == true) {
      LOG("- Fehlstart. CSV wird nicht geschrieben!");
      noSaveCSV = false;
    } else {

      //Langsamstes Ziel je Bahn und gültige Ziele ermitteln
      for (uint8_t i = 0; i < BAHN_COUNT; i++) {
        if (Bahn[i].Enabled == true && Bahn[i].Valid == true) {
          Bahn[i].SlowestRun = _max(Ziel[i * 2].StopMillis - startMillis, Ziel[(i * 2) + 1].StopMillis - startMillis);
          LOG("Bahn " + String(i + 1) + " SlowestRun = " + String(Bahn[i].SlowestRun));
        }
      }
      showResultOnLEDPanel = true;


      String csvLine = strRTCDate() + ";" + strRTCTime() + ";";
      for (uint8_t i = 0; i < ZIEL_COUNT; i++) {
        if (Ziel[i].Enabled) {
          csvLine += ((Ziel[i].StopMillis > 0) ? millis2Anzeige(Ziel[i].StopMillis - startMillis) : "0") + ";";
        } else {
          csvLine += millis2Anzeige(0) + ";";
        }
      }
      writeCSV(CSV_FILENAME, csvLine);
    }
  }

  lastActiveRunningCount = activeRunningCount;

  //Zeige Zeiten der Bahnen auf LED Panel
  if (showResultOnLEDPanel && activeRunningCount == 0 && (millis() - lastResultOnLEDPanelChangeMillis > RESULTTIME_SECONDS * 1000)) {
    lastResultOnLEDPanelChangeMillis = millis();
    static uint8_t _cnt = 0;
    switch (_cnt) {
      case 0:
        sendDataToLEDPanel("bahn1");
        break;
      case 1:
        sendDataToLEDPanel(Bahn[0].Valid ? ("millis" + String(Bahn[0].SlowestRun)) : "ow");
        break;
      case 2:
        sendDataToLEDPanel("bahn2");
        break;
      case 3:
        sendDataToLEDPanel(Bahn[1].Valid ? ("millis" + String(Bahn[1].SlowestRun)) : "ow");
        break;
    }
    _cnt++;
    if (_cnt == (Bahn[1].Enabled == true ? 4 : 2)) _cnt = 0;
  }

  //Hupe
  checkHupe();

  //LED "alle Ziele OK"
  digitalWrite(ZIELE_OK_PIN, (ZieleOK() == true && activeRunningCount == 0 && startMillis == 0) ? HIGH : LOW);

  //Debugmeldungen alle 2 Sekunden
  if (millis() - lastDebugMillis > 2000) {
    lastDebugMillis = millis();
    //LOG("Uhrzeit: " + strRTCDateTime());
    //LOG(String(activeRunningCount) + " Ziel(en) aktiv");
    //LOG("DELETE CSV PIN = "+String(digitalRead(DELETE_CSV_PIN)));
  }

  delay(1);
}


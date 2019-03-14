//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//
#ifndef __WEB__H_
#define __WEB__H_

#include "Web_CSS.h"
#include "Web_JS.h"
#include "Web_HTML.h"

enum WEB_USERLEVEL {UL_NONE, UL_GUEST, UL_ADMIN };
uint8_t loggedInUserLevel = UL_NONE;

void setTimeHtml(AsyncWebServerRequest *request) {
  bool saveSuccess = false;
  bool sc = false;
  String page = FPSTR(HTTP_SETTIME);
  page.replace("{css_style}", FPSTR(HTTP_CSS));
  page.replace("{uhrzeit}", strRTCDateTime());
  page.replace("{js}", FPSTR(HTTP_JS));

  if (request->hasParam("btnSave")) {
    AsyncWebParameter* p = request->getParam("btnSave");
    sc =  (p->value() == "1");
  }

  if (request->hasParam("zeit")) {
    AsyncWebParameter* p = request->getParam("zeit");
    char zeit[20];
    (p->value()).toCharArray(zeit, 20);
    saveSuccess = rtcSetFromString(zeit);
  }

  if (sc) {
    if (saveSuccess) {
      page.replace("{sl}", "<label class='green'>Speichern erfolgreich.<br>" + strRTCDateTime() + "</label>");
    } else {
      page.replace("{sl}", F("<label class='red'>Speichern fehlgeschlagen.</label>"));
    }
  } else {
    page.replace("{sl}", "");
  }

  AsyncWebServerResponse *response = request->beginResponse(200);
  response->addHeader("Content-Length", String(page.length()));
  request->send(200, "text/html", page);
}

void setBrightnessHtml(AsyncWebServerRequest *request) {
  String page = FPSTR(HTTP_LEDPANEL_SETBRIGHTNESS);
  page.replace("{css_style}", FPSTR(HTTP_CSS));
  page.replace("{js}", FPSTR(HTTP_JS));

  if (request->hasParam("btnBrightnessUp")) {
    AsyncWebParameter* p = request->getParam("btnBrightnessUp");
    if (p->value() == "1") sendUdp("brightnessUp", 1);
  }
  if (request->hasParam("btnBrightnessDown")) {
    AsyncWebParameter* p = request->getParam("btnBrightnessDown");
    if (p->value() == "1") sendUdp("brightnessDown", 1);
  }
  AsyncWebServerResponse *response = request->beginResponse(200);
  response->addHeader("Content-Length", String(page.length()));
  request->send(200, "text/html", page);
}

void getValues(AsyncWebServerRequest *request) {
  String json = "{";
  json += "\"uhrzeit\": \"" + strRTCDateTime() + "\"";

  json += ", \"ziele\": [";


  uint8_t j = 0;
  for (uint8_t i = 0; i < ZIEL_COUNT; i++) {
    if (Ziel[i].Enabled) {
      j++;
      json += ((j > 1) ? ", \"" : String("\""))  + millis2Anzeige(((Ziel[i].isRunning) ? millis() : Ziel[i].StopMillis) - startMillis) + "\"";
    }
  }

  json += "]}";
  request->send(200, "text/json", json);
}

void defaultHtml(AsyncWebServerRequest *request) {
  String page = "";
  if (loggedInUserLevel > UL_NONE) {

    page = FPSTR(HTTP_DEFAULT);

    page.replace("{css_style}", FPSTR(HTTP_CSS));
    page.replace("{refreshTime}", WEBPAGE_REFRESH_TIME);
    page.replace("{uhrzeit}", strRTCDateTime());
    page.replace("{js}", FPSTR(HTTP_JS));

    String tableRows = "";
    uint8_t j = 0;
    for (uint8_t i = 0; i < ZIEL_COUNT; i++) {
      if (Ziel[i].Enabled) {
        tableRows += "<tr>";
        //tableRows += "<td class='tdl " + String((Ziel[i].isRunning) ? "red" : ((Ziel[i].StopMillis > 0) ? "green" : "black")) + "'>" + Ziel[i].Headline + "</td>";
        tableRows += "<td class='tdl'>" + Ziel[i].Headline + "</td>";
        tableRows += "<td id='_ziel" + String(j) + "' class='tdr'>" + millis2Anzeige(((Ziel[i].isRunning) ? millis() : Ziel[i].StopMillis) - startMillis) + "</td>";
        tableRows += "</tr>";
        j++;
      }
    }

    String tableRowBahn2Invalid = "";
    if (Bahn[1].Enabled == true) {
      tableRowBahn2Invalid += "<td>";
      tableRowBahn2Invalid += "<form onsubmit=\"return confirm('Bahn II ungültig?'); \" action=\" / \" method=\"post\">";
      tableRowBahn2Invalid += "<button class='redbtn {disabled}' name='btnBahn2Invalid' {disabled} value='1' type='submit'>Bahn II ung&uuml;tig?</button></form>";
      tableRowBahn2Invalid += "</td>";
    }

    page.replace ("{tableRows}", tableRows);
    page.replace ("{tableRowBahn2Invalid}", tableRowBahn2Invalid);
    //Serial.print("loggedInUserLevel = "); Serial.println(loggedInUserLevel);
    page.replace ("{disabled}", (loggedInUserLevel < UL_ADMIN) ? "disabled" : "");
    page.replace ("{userlevel}", (loggedInUserLevel == UL_ADMIN) ? "ADMIN" : "GAST");

    if (request->hasParam("btnReset", true)) {
      AsyncWebParameter* p = request->getParam("btnReset", true);
      resetPressed = (p->value() == "1");
    }


    if (request->hasParam("btnBahn1Invalid", true)) {
      AsyncWebParameter* p = request->getParam("btnBahn1Invalid", true);
      if (p->value() == "1") invalidateBahn(1);
    }

    if (request->hasParam("btnBahn2Invalid", true)) {
      AsyncWebParameter* p = request->getParam("btnBahn2Invalid", true);
      if (p->value() == "1") invalidateBahn(2);
    }

    if (request->hasParam("btn5minCountdown", true)) {
      AsyncWebParameter* p = request->getParam("btn5minCountdown", true);
      if (p->value() == "1") {
        if (activeRunningCount == 0) {
          showResultOnLEDPanel = false;
          sendUdp("timerstart" + String(COUNTDOWNTIMER_SECONDS));
        }
      }
    }

    if (request->hasParam("filename")) {
      if (request->hasArg("download")) {
        Serial.println("Download Filename: " + request->arg("filename"));
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, request->arg("filename"), String(), true);
        response->addHeader("Server", "FF SDL Wettkampfzeitnahme");
        request->send(response);
      }
    }
  } else {
    page = "<html><head></head><body>NOT AUTHORIZED</body></html>";
  }
  AsyncWebServerResponse *response = request->beginResponse(200);
  response->addHeader("Content-Length", String(page.length()));
  request->send(200, "text/html", page);
}

void initWebServer() {
  webServer.on("/restart", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "rebooting");
    delay(100);
    ESP.restart();
  });

  webServer.on("/check", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "ok");
  });

  webServer.on("/deletecsv", HTTP_GET, [](AsyncWebServerRequest * request) {
    uint8_t ret =  deleteCSV(CSV_FILENAME, CSV_CREATE_BACKUP_ON_DELETE);
    request->send(200, "text/plain", "Delete CSV returned " + String(ret));
  });

  webServer.on("/setTime", HTTP_GET, setTimeHtml);
  webServer.on("/setBrightness", HTTP_GET, setBrightnessHtml);
  webServer.on("/getValues", HTTP_GET, getValues);

  webServer.onNotFound([](AsyncWebServerRequest * request) {
    if (request->authenticate(WEB_GUEST_USER, WEB_GUEST_PASS)) {
      loggedInUserLevel = UL_GUEST;
    }

    if (request->authenticate(WEB_ADMIN_USER, WEB_ADMIN_PASS)) {
      loggedInUserLevel = UL_ADMIN;
    }

    if (loggedInUserLevel == UL_NONE) {
      return request->requestAuthentication("FF SDL Wettkampfzeitnahme");
    } else {
      defaultHtml(request);
    }

    Serial.print("loggedInUserLevel = "); Serial.println(loggedInUserLevel);

    loggedInUserLevel = UL_NONE;
  });

  webServer.begin();
}
#endif

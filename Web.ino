//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

#include "Web_CSS.h"
#include "Web_HTML.h"

void initWebServer() {
  webServer.on("/restart", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "rebooting");
    delay(100);
    ESP.restart();
  });

  webServer.on("/setTime", HTTP_GET, setTimeHtml);
  webServer.onNotFound(defaultHtml);

  webServer.begin();
}

void setTimeHtml(AsyncWebServerRequest *request) {
  bool saveSuccess = false;
  bool sc = false;
  String page = FPSTR(HTTP_SETTIME);
  page.replace("{css_style}", FPSTR(HTTP_CSS));
  page.replace("{uhrzeit}", strRTCDateTime());

  if (request->hasParam("btnSave")) {
    Serial.println("Save button pressed.");
    AsyncWebParameter* p = request->getParam("btnSave");
    sc =  (p->value() == "1");
  }

  if (request->hasParam("zeit")) {
    Serial.println("Web: zeit values!");
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

void defaultHtml(AsyncWebServerRequest *request) {

  String page = FPSTR(HTTP_DEFAULT);

  page.replace("{css_style}", FPSTR(HTTP_CSS));
  page.replace("{refreshTime}", WEBPAGE_REFRESH_TIME);
  page.replace("{uhrzeit}", strRTCDateTime());

  String tableRows = "";

  for (uint8_t i = 0; i < ZIEL_COUNT; i++) {
    if (Ziel[i].Enabled) {
      tableRows += "<tr>";
      tableRows += "<td class='tdl " + String((Ziel[i].isRunning) ? "red" : ((Ziel[i].StopMillis > 0) ? "green" : "black")) + "'>" + Ziel[i].Headline + "</td>";
      tableRows += "<td class='tdr'>" + millis2Anzeige(((Ziel[i].isRunning) ? millis() : Ziel[i].StopMillis) - startMillis) + "</td>";
      tableRows += "</tr>";
    }
  }

  page.replace ("{tableRows}", tableRows);

  if (request->hasParam("btnReset", true)) {
    Serial.println("Web: Reset values!");
    AsyncWebParameter* p = request->getParam("btnReset", true);
    resetPressed = (p->value() == "1");
  }

  if (request->hasParam("filename")) {
    if (request->hasArg("download")) { 
      Serial.println("Download Filename: " + request->arg("filename"));
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, request->arg("filename"), String(), true);
      response->addHeader("Server", "ESP Async Web Server");
      request->send(response);
    }
  }

  AsyncWebServerResponse *response = request->beginResponse(200);
  response->addHeader("Content-Length", String(page.length()));
  request->send(200, "text/html", page);
}

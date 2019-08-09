//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

#ifndef __WIFISETUP__H_
#define __WIFISETUP__H_

boolean arrayIncludeElement(uint8_t array[], uint8_t element, uint8_t sz);
boolean arrayIncludeElement(uint8_t array[], uint8_t element, uint8_t sz) {
  for (int i = 0; i < sz; i++) {
    if (array[i] == element) {
      return true;
    }
  }
  return false;
}

void initWiFi() {
  uint8_t chs[13]  = { 1, 6, 11, 2, 7, 12, 3, 8, 13, 4, 9, 5, 10 }; //list of channels
  LOG("Scan for networks...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  uint8_t n = WiFi.scanNetworks(false, true);

  int indices[n];
  uint8_t occupied_chs[n];
  for (int i = 0; i < n; i++) {
    indices[i] = i;
  }
  //sort networks by channel
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      if (WiFi.channel(indices[j]) > WiFi.channel(indices[i])) {
        std::swap(indices[i], indices[j]);
      }
    }
  }

  for (uint8_t num = 0; num < n; num++) {
    occupied_chs[num] = WiFi.channel(num);
  }

  uint8_t selected_ch_idx = 0;
  for (uint8_t num = 0; num < n; num++) {
    uint8_t _ch = WiFi.channel(indices[num]);
    if (chs[selected_ch_idx] == _ch)
      if (selected_ch_idx < sizeof(chs))
        selected_ch_idx++;

    while (arrayIncludeElement(occupied_chs, chs[selected_ch_idx], n) == true) {
      if (selected_ch_idx < sizeof(chs)) {
        selected_ch_idx++;
      } else {
        break;
      }
    }

    LOG("Found " + WiFi.SSID(indices[num]) + "  (" + String(_ch) + ")");
  }
  LOG("Scan for networks... done");
  LOG("Using network channel: " + String(chs[selected_ch_idx]));

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password, chs[selected_ch_idx], 0, 4);
}

#endif

//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

#ifndef __LEDPANELCONTROL_H_
#define __LEDPANELCONTROL_H_

bool receiveDataFromLEDPanel(char expected) {
  while (Serial1.available() > 0) {
    char data = Serial.read();
    if (data == expected) {
      return true;
    }
  }
  return false;
}

void sendUdp(String cmd, uint8_t cnt) {
  Serial.println("sendUdp: " + cmd);
  for (uint8_t i = 0; i < cnt; i++) {
    UDPClient.UDP.beginPacket(LEDPanel_IP, UDPPORT);
    UDPClient.UDP.print(cmd.c_str());
    UDPClient.UDP.endPacket();
    delay(10);
  }
}

void sendExtSerial(String cmd, uint8_t cnt) {
  Serial.println("sendExtSerial: " + cmd);
  for (uint8_t i = 0; i < cnt; i++) {
    Serial1.println(cmd);
    delay(100);
    if (receiveDataFromLEDPanel(0x01)) break;
  }
}

void sendDataToLEDPanel(String cmd, uint8_t cnt) {
#ifdef USE_LEDPANEL_SERIAL
  sendExtSerial(cmd, cnt);
#else
  sendUdp(cmd, cnt);
#endif
}

void sendDataToLEDPanel(String cmd) {
#ifdef USE_LEDPANEL_SERIAL
  sendExtSerial(cmd, EXTSERIALSENDCOUNT);
#else
  sendUdp(cmd, UDPSENDCOUNT);
#endif
}

#endif

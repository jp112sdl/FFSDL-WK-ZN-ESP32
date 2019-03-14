//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

#ifndef __UDP_H_
#define __UDP_H_

void sendUdp(String cmd, uint8_t cnt) {
  Serial.println("sendUdp: " + cmd);
  for (uint8_t i = 0; i < cnt; i++) {
    UDPClient.UDP.beginPacket(LEDPanel_IP, UDPPORT);
    UDPClient.UDP.print(cmd.c_str());
    UDPClient.UDP.endPacket();
    delay(10);
  }
}

void sendUdp(String cmd) {
  sendUdp(cmd, UDPSENDCOUNT);
}

#endif

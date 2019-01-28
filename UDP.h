//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

#ifndef __UDP_H_
#define __UDP_H_

void sendUdp(String cmd) {
  Serial.println("sendUdp: "+cmd);
  for (uint8_t i = 0; i < UDPSENDCOUNT; i++) {
    UDPClient.UDP.beginPacket(LEDPanel_IP, UDPPORT);
    UDPClient.UDP.print(cmd.c_str());
    UDPClient.UDP.endPacket();
    delay(1);
  }
}

#endif

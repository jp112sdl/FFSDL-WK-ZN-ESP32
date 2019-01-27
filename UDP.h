/*
 * UDP.h
 *
 *  Created on: 27 Jan 2019
 *      Author: pechj
 */

#ifndef __UDP_H_
#define __UDP_H_

void sendUdp(String cmd) {
    UDPClient.UDP.beginPacket(LEDPanel_IP, UDPPORT);
    UDPClient.UDP.print(cmd.c_str());
    UDPClient.UDP.endPacket();
}

#endif

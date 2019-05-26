#ifndef __LOG__H_
#define __LOG__H_

#define LOGARRAYSIZE 1024

String logArray[LOGARRAYSIZE+1];
uint16_t logLength = 0;

void initLog() {
  memset(logArray, 0, sizeof(logArray));
}

void LOG(String logText) {
  String msg = strRTCDateTime() + " " + logText;

  if (logLength > 0) {
    for (uint16_t c = logLength; c > 0; c--) {
      logArray[c] = logArray[c - 1];
    } 
  } 

  logArray[0] = msg;

  if (logLength < LOGARRAYSIZE) logLength++;
  
  Serial.println(msg);
}

#endif

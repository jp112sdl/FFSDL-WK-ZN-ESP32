//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//
#ifndef __RTC__H_
#define __RTC__H_

int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
bool initRTCdone = false;
void LOG(String logText);

byte decToBcd(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

uint32_t getIntFromString (char *stringWithInt, byte num) {
  char *tail;
  while (num > 0) {
    num--;
    while ((!isdigit (*stringWithInt)) && (*stringWithInt != 0)) {
      stringWithInt++;
    }
    tail = stringWithInt;
    while ((isdigit(*tail)) && (*tail != 0)) {
      tail++;
    }

    if (num > 0) {
      stringWithInt = tail;
    }
  }
  return (strtol(stringWithInt, &tail, 10));
}

boolean checkDateTime(int jahr, int monat, int tag, int stunde, int minute, int sekunde) {
  boolean result = false;
  if (jahr > 2000) {
    result = true;
  } else {
    return false;
  }

  if (jahr % 400 == 0 || (jahr % 100 != 0 && jahr % 4 == 0)) {
    daysInMonth[1] = 29;
  }

  if (monat < 13) {
    if ( tag <= daysInMonth[monat - 1] ) {
      result = true;
    }
  } else {
    return false;
  }

  if (stunde < 24 && minute < 60 && sekunde < 60 && stunde >= 0 && minute >= 0 && sekunde >= 0) {
    result = true;
  } else {
    return false;
  }

  return result;
}

void rtcReadTime() {
  RTCWire.beginTransmission(RTC_I2C_ADDRESS);
  RTCWire.write(0);
  RTCWire.endTransmission();
  RTCWire.requestFrom(RTC_I2C_ADDRESS, 7);
  struct tm tm;
  tm.tm_sec  = bcdToDec(RTCWire.read() & 0x7f);
  tm.tm_min  = bcdToDec(RTCWire.read());
  tm.tm_hour = bcdToDec(RTCWire.read() & 0x3f);
  bcdToDec(RTCWire.read());
  tm.tm_mday = bcdToDec(RTCWire.read());
  tm.tm_mon  = bcdToDec(RTCWire.read()); // 0-11 - Note: The month on the DS1307 is 1-12.
  tm.tm_year = bcdToDec(RTCWire.read()) + 100; // Years since 1900
  time_t t = mktime(&tm);
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
}

void rtcWriteTime(int jahr, int monat, int tag, int stunde, int minute, int sekunde) {
  RTCWire.beginTransmission(RTC_I2C_ADDRESS);
  RTCWire.write(0);
  RTCWire.write(decToBcd(sekunde));
  RTCWire.write(decToBcd(minute));
  RTCWire.write(decToBcd(stunde));
  RTCWire.write(decToBcd(0));
  RTCWire.write(decToBcd(tag));
  RTCWire.write(decToBcd(monat));
  RTCWire.write(decToBcd(jahr - 2000));
  RTCWire.endTransmission();
  rtcReadTime();
}

bool rtcSetFromString(char*zeit) {
  uint8_t tag = getIntFromString (zeit, 1);
  uint8_t monat = getIntFromString (zeit, 2);
  uint16_t jahr = getIntFromString (zeit, 3);
  uint8_t stunde = getIntFromString (zeit, 4);
  uint8_t minute = getIntFromString (zeit, 5);
  uint8_t sekunde = getIntFromString (zeit, 6);
  LOG("rtcSetFromString: "+String(zeit));
  LOG("Tag:     " + String(tag));
  LOG("Monat:   " + String(monat));
  LOG("Jahr:    " + String(jahr));
  LOG("Stunde : " + String(stunde));
  LOG("Minute:  " + String(minute));
  LOG("Sekunde: " + String(sekunde));

  if (checkDateTime(jahr, monat, tag, stunde, minute, sekunde)) {
    LOG("rtcWriteTime - got valid time. set.");
    rtcWriteTime(jahr, monat, tag, stunde, minute, sekunde);
    return true;
  } else {
    LOG("rtcWriteTime - invalid time. not set.");
    return false;
  }
}

String strRTCDate() {
  time_t now = time(0);
  tm* localtm = localtime(&now);
  String result = "";
  int tag = localtm->tm_mday;
  if (tag < 10) {
    result += "0";
  }
  result += tag;
  result += ".";
  int monat = localtm->tm_mon;
  if (monat < 10) {
    result += "0";
  }
  result += monat;
  result += ".";
  int jahr = localtm->tm_year + 1900;
  result += jahr;
  return result;
}

String strRTCTime() {
  time_t now = time(0);
  tm* localtm = localtime(&now);
  String result = "";
  int stunde = localtm->tm_hour;
  if (stunde < 10) {
    result += "0";
  }
  result += stunde;
  result += ":";
  int minute = localtm->tm_min;
  if (minute < 10) {
    result += "0";
  }
  result += minute;
  result += ":";
  int sekunde = localtm->tm_sec;
  if (sekunde < 10) {
    result += "0";
  }
  result += sekunde;
  return result;
}

String strRTCDateTime() {
  return (initRTCdone == true ? strRTCDate() + " " + strRTCTime() : "00.00.0000 00:00:00");
}

void initRTC() {
  RTCWire.begin(RTC_I2C_SDA_PIN, RTC_I2C_SCL_PIN, 100000);
  rtcReadTime();
  initRTCdone = true;
  LOG("RTC Init done. Time = " + strRTCDateTime());
}

#endif

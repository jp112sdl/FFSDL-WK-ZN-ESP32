//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

int jahr, monat, tag, stunde, minute, sekunde, wochentag;
int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

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
  sekunde    = bcdToDec(RTCWire.read() & 0x7f);
  minute     = bcdToDec(RTCWire.read());
  stunde     = bcdToDec(RTCWire.read() & 0x3f);
  bcdToDec(RTCWire.read());
  tag        = bcdToDec(RTCWire.read());
  monat      = bcdToDec(RTCWire.read());
  jahr       = bcdToDec(RTCWire.read()) + 2000;
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
}

bool rtcSetFromString(char*zeit) {
  uint8_t tag = getIntFromString (zeit, 1);
  uint8_t monat = getIntFromString (zeit, 2);
  uint16_t jahr = getIntFromString (zeit, 3);
  uint8_t stunde = getIntFromString (zeit, 4);
  uint8_t minute = getIntFromString (zeit, 5);
  uint8_t sekunde = getIntFromString (zeit, 6);
  Serial.print("rtcSetFromString: ");
  Serial.println(zeit);
  Serial.println("Tag: " + String(tag));
  Serial.println("Monat: " + String(monat));
  Serial.println("Jahr: " + String(jahr));
  Serial.println("Stunde: " + String(stunde));
  Serial.println("Minute: " + String(minute));
  Serial.println("Sekunde: " + String(sekunde));

  if (checkDateTime(jahr, monat, tag, stunde, minute, sekunde)) {
    Serial.println("rtcWriteTime - got valid time. set.");
    rtcWriteTime(jahr, monat, tag, stunde, minute, sekunde);
    return true;
  } else {
    Serial.println("rtcWriteTime - invalid time. not set.");
    return false;
  }
}

String strRTCDateTime() {
  rtcReadTime();
  String result = "";
  if (tag < 10) {
    result += "0";
  }
  result += tag;
  result += ".";
  if (monat < 10) {
    result += "0";
  }
  result += monat;
  result += ".";
  result += jahr;
  result += " ";
  if (stunde < 10) {
    result += "0";
  }
  result += stunde;
  result += ":";
  if (minute < 10) {
    result += "0";
  }
  result += minute;
  result += ":";
  if (sekunde < 10) {
    result += "0";
  }
  result += sekunde;
  return result;
}

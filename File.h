//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

void deleteCSV(const char * fileName) {
  if (spiffsAvailable) {
    if (SPIFFS.remove(fileName)) {
      Serial.println("- file deleted");
      for (uint8_t i = 0; i < 5; i++) {
        digitalWrite(STATUS_LED1_PIN, !digitalRead(STATUS_LED1_PIN));
        delay(300);
      }
      delay(300);
      digitalWrite(STATUS_LED1_PIN, LOW);
    } else {
      Serial.println("- delete failed");
    }
  } else {
    Serial.println("deleteCSV not done; SPIFFS not available!");
  }
}

void writeCSV(const char * fileName, String &csvLine) {
  if (spiffsAvailable) {
    File file = SPIFFS.open(fileName);
    if (!file) {
      Serial.println("- failed to open file - creating new");
      file = SPIFFS.open(fileName, FILE_WRITE);
      if (!file) {
        Serial.println("- failed to open file for writing");
        return;
      } else {
        if (file.println(CSV_HEADER)) {
          file.close();
        } else {
          Serial.println("- failed to write line into file");
        }
      }
    }

    file = SPIFFS.open(fileName, FILE_APPEND);
    if (!file) {
      Serial.println("- csv: failed to open file for appending");
    }
    if (file.println(csvLine)) {
      Serial.println("- csv: message appended");
      file.close();
    } else {
      Serial.println("- csv: append failed");
    }
  } else {
    Serial.println("writeCSV not done; SPIFFS not available!");
  }
}

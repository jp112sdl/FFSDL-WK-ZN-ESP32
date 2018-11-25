//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

void deleteCSV(const char * fileName) {
  if (spiffsAvailable) {
    if (SPIFFS.exists(fileName)) {
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
      Serial.println("- file does not exist. no need to delete");
    }
  } else {
    Serial.println("deleteCSV not done; SPIFFS not available!");
  }
}

void writeCSV(const char * fileName, String &csvLine) {
  if (spiffsAvailable) {
    Serial.println("- writing CSV file");
    delay(50);
    if (!SPIFFS.exists(fileName)) {
      Serial.println("- failed to open file - creating new");
      delay(50);
      File file = SPIFFS.open(fileName, FILE_WRITE);
      if (!file) {
        Serial.println("- failed to open file for writing");
        delay(50);
        return;
      } else {
        if (file.println(CSV_HEADER)) {
        } else {
          Serial.println("- failed to write line into file");
        }
        file.close();
        delay(50);
      }
    }

    File file = SPIFFS.open(fileName, FILE_APPEND);
    delay(50);
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

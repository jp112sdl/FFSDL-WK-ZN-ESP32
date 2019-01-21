//
// 2018-11-22 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//

#ifndef __FILE__H_
#define __FILE__H_

uint8_t IRAM_ATTR deleteCSV(const char * fileName, bool createBackup) {
  if (spiffsAvailable) {
    if (SPIFFS.exists(fileName)) {
      if (createBackup) {
        if (SPIFFS.exists(fileName)) {
          String bakFile = String(fileName) + ".bak";
          SPIFFS.remove(bakFile);
          if (SPIFFS.rename(fileName, bakFile)) {
            Serial.println(" - created backup of CSV");
            blinkStatusLed(STATUS_LED1_PIN, 5);
            return RENAME_SUCCESSFUL;
          } else {
            Serial.println(" - create backup of CSV failed");
            blinkStatusLed(STATUS_LED2_PIN, 10);
            return RENAME_FAILED;
          }
        } else {
          Serial.println(" - file does not exist no need to rename");
          return FILE_DOES_NOT_EXIST;
        }
      }
      if (SPIFFS.remove(fileName)) {
        Serial.println(" - file deleted");
        blinkStatusLed(STATUS_LED1_PIN, 5);
        return NO_ERROR;
      } else {
        Serial.println(" - delete failed");
        blinkStatusLed(STATUS_LED2_PIN, 10);
        return DELETE_FAILED;
      }
    } else {
      Serial.println(" - file does not exist. no need to delete or rename");
      return FILE_DOES_NOT_EXIST;
    }
  } else {
    Serial.println("deleteCSV not done; SPIFFS not available!");
    blinkStatusLed(STATUS_LED2_PIN, 10);
    return SPIFFS_NOT_AVAILABLE;
  }
}

void IRAM_ATTR writeCSV(const char * fileName, String &csvLine) {
  if (spiffsAvailable) {
    Serial.println(" - writing CSV file");
    if (!SPIFFS.exists(fileName)) {
      Serial.println(" - failed to open file - creating new");
      File file = SPIFFS.open(fileName, FILE_WRITE);
      if (!file) {
        Serial.println(" - failed to open file for writing");
        return;
      } else {
        if (file.println(CSV_HEADER)) {
        } else {
          Serial.println(" - failed to write line into file");
        }
        file.close();
      }
    }

    File file = SPIFFS.open(fileName, FILE_APPEND);
    if (!file) {
      Serial.println(" - csv : failed to open file for appending");
    }
    if (file.println(csvLine)) {
      Serial.println(" - csv : message appended");
      file.close();
    } else {
      Serial.println(" - csv : append failed");
    }
  } else {
    Serial.println("writeCSV not done; SPIFFS not available!");
  }
}
#endif

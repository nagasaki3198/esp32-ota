#include "FS.h"
#include "SPIFFS.h"
#include <Update.h>
#include <HTTPClient.h>


String baseUrl = "https://raw.githubusercontent.com/nagasaki3198/esp32-ota/DEV/firmware/esp32-ota.ino.bin";

bool downloadFirmware() {
  HTTPClient http;
  bool stat = false;
  Serial.println(baseUrl);
  File f = SPIFFS.open("/update.bin", "w");
  if (f) {
    http.begin(baseUrl);
    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        Serial.println("Downloading...");
        http.writeToStream(&f);
        stat = true;
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    f.close();
  } else {
    Serial.println("failed to open /update.bin");
  }
  return stat;
}

void performUpdate(Stream& updateSource, size_t updateSize) {
  String result = "";
  if (Update.begin(updateSize)) {
    size_t written = Update.writeStream(updateSource);
    if (written == updateSize) {
      Serial.println("Written : " + String(written) + " successfully");
    } else {
      Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
    }
    result += "Written : " + String(written) + "/" + String(updateSize) + " [" + String((written / updateSize) * 100) + "%] \n";
    if (Update.end()) {
      Serial.println("OTA done!");
      result += "OTA Done: ";
      if (Update.isFinished()) {
        Serial.println("Update successfully completed. Rebooting...");
        result += "Success!\n";
      } else {
        Serial.println("Update not finished? Something went wrong!");
        result += "Failed!\n";
      }

    } else {
      Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      result += "Error #: " + String(Update.getError());
    }
  } else {
    Serial.println("Not enough space to begin OTA");
    result += "Not enough space for OTA";
  }
  //http send 'result'
}


void updateFromFS(fs::FS& fs) {
  File updateBin = fs.open("/update.bin");
  if (updateBin) {
    if (updateBin.isDirectory()) {
      Serial.println("Error, update.bin is not a file");
      updateBin.close();
      return;
    }
    size_t updateSize = updateBin.size();
    if (updateSize > 0) {
      Serial.println("Trying to start update");
      performUpdate(updateBin, updateSize);
    } else {
      Serial.println("Error, file is empty");
    }
    updateBin.close();
    // when finished remove the binary from spiffs to indicate end of the process
    Serial.println("Removing update file");
    fs.remove("/update.bin");
  } else {
    Serial.println("Could not load update.bin from spiffs root");
  }
}

void perfom_OTA_update() {
  if (SPIFFS.exists("/update.bin")) {
    SPIFFS.remove("/update.bin");
    Serial.println("Removed existing update file");
  }
  if (downloadFirmware()) updateFromFS(SPIFFS);
}
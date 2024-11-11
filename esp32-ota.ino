#include <WiFiManager.h>
#include "FS.h"
#include "SPIFFS.h"
#include <HTTPClient.h>
#include "Update.h"
#include <WiFiClientSecure.h>

//static const char *url = "raw.githubusercontent.com/nagasaki3198/esp32-ota/DEV/firmware/esp32-ota.ino.bin"

#define HOST "raw.githubusercontent.com"
#define PATH "/nagasaki3198/esp32-ota/DEV/firmware/esp32-ota.ino.bin"
#define PORT 443
#define FILE_NAME "esp32-ota.ino.bin"

WiFiManager wm;

bool res = false;

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  res = wm.autoConnect("AP_point");
  if (res) {
    downloadFirmware();
    performOTAUpdateFromSPIFFS();
  }
}

void loop() {
  // Nothing to do here
}

bool downloadFirmware() {
  HTTPClient http;
  bool stat = false;
  File f = SPIFFS.open("/update.bin", "w");
  if (f) {
    http.begin("https://raw.githubusercontent.com/nagasaki3198/esp32-ota/DEV/firmware/esp32-ota.ino.bin");
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
    f.close();
  } else {
    Serial.println("failed to open /update.bin");
  }
  http.end();

  return stat;
}

void performOTAUpdateFromSPIFFS() {
  // Open the firmware file in SPIFFS for reading
  File file = SPIFFS.open("/update.bin");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.println("Starting update..");
  size_t fileSize = file.size(); // Get the file size
  Serial.println(fileSize);

  // Begin OTA update process with specified size and flash destination
  if (!Update.begin(fileSize, U_FLASH)) {
    Serial.println("Cannot do the update");
    return;
  }

  // Write firmware data from file to OTA update
  Update.writeStream(file);

  // Complete the OTA update process
  if (Update.end()) {
    Serial.println("Successful update");
  }
  else {
    Serial.println("Error Occurred:" + String(Update.getError()));
    return;
  }

  file.close(); // Close the file
  Serial.println("Reset in 4 seconds....");
  delay(4000);
  ESP.restart(); // Restart ESP32 to apply the update
}
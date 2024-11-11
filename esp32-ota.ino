#include <WiFiManager.h>
#include <OTA_lib.h>

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
    perfom_OTA_update();
  }
}

void loop() {
  // Nothing to do here
}
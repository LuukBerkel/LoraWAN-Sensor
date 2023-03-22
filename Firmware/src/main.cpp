#include <Arduino.h>
#include "vision/counter.hpp"

// Counter settings
#define COUNTER_DIFF_THRESHOLD 50
#define COUNTER_PEOPLE_THRESHOLD 20
counter* visual_counter = new counter(360, 240, FRAMESIZE_QVGA);

void setup() {
  // Setting up exit code
  int exit_code;
  Serial.begin(115200);
  Serial.println("[DEBUG]: Debug mode active. This will reduce performance");

  // Initing camera hardware
  exit_code = visual_counter->begin();
  if (exit_code != NO_ERROR){
    Serial.println("[ERROR]: Camera failed to start.");

    // Restarting because error
    Serial.println("[INFO]: Restarting in ten seconds.");
    sleep(10000);
    ESP.restart();
  }

  exit_code = visual_counter->calibrate(COUNTER_DIFF_THRESHOLD, COUNTER_PEOPLE_THRESHOLD);
  if (exit_code != NO_ERROR){
    if (exit_code == ERROR_CAPTURE_FAILED) {
      Serial.println("[ERROR]: Camera capture failed.");
    } else {
      Serial.println("[ERROR]: Memory operations failed.");
    }

    // Restarting because error
    Serial.println("[INFO]: Restarting in ten seconds.");
    sleep(10000);
    ESP.restart();
  }

  Serial.println("[INFO]: Setup was a succes.");
}

void loop() {
  // Taking measurment.
  int result = visual_counter->count();
  if (result >= 0){
    Serial.print("[INFO]: People count = ");
    Serial.println(result);
  }

  // Printing error codes
  if (result == ERROR_NOT_CALIBRATED){
    Serial.println("[ERROR]: Camera was not calibrated.");
  } else if (result == ERROR_CAPTURE_FAILED){
    Serial.println("[ERROR]: Camera capture failed.");
  } else if (result == ERROR_DETECTION_FAILED){
    Serial.println("[ERROR]: Detection operations failed.");
  }
  
  delay(5000);
}
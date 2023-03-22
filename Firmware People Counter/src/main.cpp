#include <Arduino.h>
#include "vision/vision.hpp"

// Vision settings
#define VISION_DIFF_THRESHOLD 50
#define VISION_PEOPLE_THRESHOLD 20
vision Vision;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting people count sensor..");

  while (!Vision.begin()){
    Serial.println("Failed to start camera!");
    sleep(10000);
  }

  if (!Vision.calibrate(VISION_DIFF_THRESHOLD, VISION_PEOPLE_THRESHOLD)){
    Serial.println("Failed to take base!");
  }

  Serial.println("Setup was a succes...");
  delay(1000);
}

void loop() {
  int result = Vision.count();
  if (result == -2){
    Serial.println("No base to compare!");
  }
  else if (result == -1){
    Serial.println("Other erorr occured!");
  } else {
    Serial.print(result);
    Serial.println("People count");
  }

  delay(5000);
}
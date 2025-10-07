#include <Arduino.h>
#include "Esp.h"
#include "lora/lora.hpp"
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h" 
#include "time.h"
#include "DHT.h"

#define DHTPIN 22
#define DHTTYPE DHT11  

#define SLEEP_TIME 100000  

lora_config cfg = {
    .app_eui = "0000000000000000",
    .app_key = "fe5089e2b616abc105318ad2ccd49ede",
    .dev_eui = "8899AABBCCDDEEFF",
};
lora lorawan = lora();

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("Sensor is starting...");

  dht.begin();
  int err = lorawan.begin(&cfg);

  if (err != NO_ERROR) {
    Serial.println("Failed to join LoraWAN network");
    delay(SLEEP_TIME);
    ESP.restart();
  } else {
    Serial.println("Sensor joined LoraWAN network");
  }
}


void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.print("Measurment: Temp=");
  Serial.print(temperature);
  Serial.print("C, Hum=");
  Serial.print(humidity);
  Serial.println("%");

  int err = lorawan.send(temperature, humidity);
  if (err != NO_ERROR) {
    Serial.println("Failed to send measurement via LoRaWAN");
  } else {
    Serial.println("Succesfully send measurement via LoRaWAN");
  }

  delay(SLEEP_TIME);
}
#include <Arduino.h>
#include "Esp.h"
#include "lora/lora.hpp"
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h" 
#include "time.h"
#include "DHT.h"

#define DHTPIN 2    
#define DHTTYPE DHT11  

#define DEBUG_MODE
#define SLEEP_TIME 100000  

lora_config cfg = {
    .app_eui = "0000000000000000",
    .app_key = "fe5089e2b616abc105318ad2ccd49ede",
    .dev_eui = "8899AABBCCDDEEFF",
};
lora lorawan = lora();

DHT dht(DHTPIN, DHTTYPE);

void setup() {
#ifdef DEBUG_MODE
  Serial.begin(115200);
  Serial.println("Sensor is starting...");
#endif

  dht.begin();
  int err = lorawan.begin(&cfg);

#ifdef DEBUG_MODE
  if (err != NO_ERROR) {
    Serial.println("Failed to join LoraWAN network");
  } else {
    Serial.println("Sensor joined LoraWAN network");
  }
#endif

  if (err != NO_ERROR) {
    delay(SLEEP_TIME);
    ESP.restart();
  }
}


void loop() {
  int humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int err = lorawan.send(temperature, humidity);

#ifdef DEBUG_MODE
  if (err != NO_ERROR) {
    Serial.println("Failed to send data via LoRaWAN");
  } else {
    Serial.print("Data sent: Temp=");
    Serial.print(temperature);
    Serial.print("C, Hum=");
    Serial.print(humidity);
    Serial.println("%");
  }
#endif

  delay(SLEEP_TIME);
}
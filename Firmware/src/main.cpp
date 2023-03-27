#include <Arduino.h>
#include "vision/counter.hpp"
#include "Esp.h"
#include "lora/lora.hpp"

// Debug or Lora mode
#define LORA_MODE
//#define DEBUG_MODE

// Counter settings
const int diff_threshold = 50;
const int people_threshold = 20;
const framesize_t frame = FRAMESIZE_QVGA;
const int frame_width = 360;
const int frame_heigth = 240;
counter* visual_counter = new counter(frame_width, frame_heigth, frame);

// Lorawan settings
#ifdef LORA_MODE
lora* lorawan = new lora;
#endif

// Wake  up settings
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

#define BUTTON_PIN_BITMASK 0x4000 /* Pin number 14 for wake up on intterupt*/
RTC_DATA_ATTR int bootCount = 0;

// State machine states.
typedef enum {
  WAKE_AND_MEASURE_STATE = 0,   // This state measures activity in the room while in deepsleep.
  WAKE_AND_EXTEND_STATE = 1,    // This state measures all sensors and sends its data over lorawan.
  CONFIG_STATE = 2,
} application_states;
application_states active_state;

void setup() {
#ifdef DEBUG_MODE
  Serial.begin(115200);
#endif
  ++bootCount;
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : 
      active_state = WAKE_AND_EXTEND_STATE; 
      break;
    case ESP_SLEEP_WAKEUP_TIMER : 
      active_state = WAKE_AND_MEASURE_STATE; 
      break;
    default : 
      active_state = CONFIG_STATE; 
      break;
  }
}

void extend_state(){
  /*******Reset step*********/
#ifdef DEBUG_MODE
  Serial.println("[INFO]: Movement detected");
#endif
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14,1);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void measurment_state(){
  /*********Boot step*********/
#ifdef LORA_MODE
  // Booting lora
  int err = lorawan->begin();
  if (err != NO_ERROR){
    delay(10000);
    ESP.restart();
  }
#endif
#ifdef DEBUG_MODE
  Serial.println("[INFO]: Sendig measurements");
#endif

 // Booting counter
  // int err1 = visual_counter->begin();
  // if (err1 != NO_ERROR){
  //   delay(10000);
  //   ESP.restart();
  // }


  /*******Measuring step********/

  // Measuring amount of people if interrupt count > 0
  int people_count = 0;
  if (bootCount > 1){
#ifdef DEBUG_MODE
     Serial.println("[Info] Detected people");
#endif
    people_count = 1;
  }

#ifdef LORA_MODE
  lorawan->send(people_count);
#endif

  /************Reset step************/

  // Setting intterupts and sleep.
  bootCount = 0;
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14,1);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void config_state(){
#ifdef DEBUG_MODE
  Serial.println("[INFO]: Configuring");
#endif
  // Booting counter
  // int err = visual_counter->begin();
  // if (err != NO_ERROR){
  //   delay(10000);
  //   ESP.restart();
  // }

  // TODO add calibartion

  // Setting intterupts and sleep.
  bootCount = 0;
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14,1);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

/// @brief switching between states.
void loop() {
  switch (active_state)
  {
    case CONFIG_STATE:
    config_state();
    break;
    case WAKE_AND_MEASURE_STATE:
    measurment_state();
    break;
    case WAKE_AND_EXTEND_STATE:
    extend_state();
    break;
  }
  esp_deep_sleep_start();
}
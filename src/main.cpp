#include <Arduino.h>
#include "vision/counter.hpp"
#include "Esp.h"
#include "lora/lora.hpp"
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h" 
#include "time.h"

// Debug or Lora mode
#define LORA_MODE
//#define DEBUG_MODE

// Counter settings
// const int diff_threshold = 50;
// const int people_threshold = 20;
// const framesize_t frame = FRAMESIZE_QVGA;
// const int frame_width = 360;
// const int frame_heigth = 240;
// counter* visual_counter = new counter(frame_width, frame_heigth, frame);

// Lorawan settings
#ifdef LORA_MODE
lora* lorawan = new lora;
#endif

// Wake  up settings
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

#define BUTTON_PIN_BITMASK 0x4000 /* Pin number 14 for wake up on intterupt*/
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR unsigned long lastBoot = 0;

// State machine states.
typedef enum {
  WAKE_AND_MEASURE_STATE = 0,   // This state measures activity in the room while in deepsleep.
  WAKE_AND_EXTEND_STATE = 1,    // This state measures all sensors and sends its data over lorawan.
  CONFIG_STATE = 2,
} application_states;
application_states active_state;

void setup() {
  // Disabling brownout
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Starting debug serial
#ifdef DEBUG_MODE
  Serial.begin(115200);
#endif

  // Setting wakeup settings.
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

  delay(100);
}

void extend_state(){
  /*******Reset step*********/
#ifdef DEBUG_MODE
  Serial.println("[INFO]: Movement detected");
#endif

  unsigned long now = time(NULL);
  unsigned long to_sleep = (lastBoot + TIME_TO_SLEEP) - now;
  if (to_sleep > TIME_TO_SLEEP){
    to_sleep = 0; 
  } 

  #ifdef DEBUG_MODE
    Serial.println(to_sleep);
  #endif

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14,1);
  esp_sleep_enable_timer_wakeup(to_sleep * uS_TO_S_FACTOR);
}

void measurment_state(){
  // Booting lora
#ifdef LORA_MODE
  int err = lorawan->begin();
  if (err != NO_ERROR){
    delay(10000);
    ESP.restart();
  }
#endif

  // Debug printing
#ifdef DEBUG_MODE
  Serial.println("[INFO]: Sendig measurements");
#endif

  // Measuring amount of people if interrupt count > 0
  int people_count = 0;
  if (bootCount > 1){
#ifdef DEBUG_MODE
     Serial.println("[Info] Detected people");
#endif
    people_count = bootCount;
  }

#ifdef LORA_MODE
  lorawan->send(people_count);
#endif

  /************Reset step************/
  // Setting intterupts and sleep.
  bootCount = 0;
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14,1);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  lastBoot= time(NULL);
}

void config_state(){
  // Debug printing
#ifdef DEBUG_MODE
  Serial.println("[INFO]: Configuring");
#endif

  // Setting intterupts and sleep.
  bootCount = 0;
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14,1);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  lastBoot= time(NULL);
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
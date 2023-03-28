#include "counter.hpp"
#include <stdio.h>
#include <Arduino.h>
#include "vision.hpp"      

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
  
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

counter::counter(int width, int heigth, framesize_t resolution){
    counter::resolution = resolution;
    counter::detection = new vision(width, heigth);
}

int counter::begin(){
    // Config of the camera
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_GRAYSCALE; 
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // Init of the camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        return ERROR_CAMERA_FAIL;
    }    

    // Init sd card
    if(!SD_MMC.begin()){
        return ERROR_SD_FAIL;
    }
  
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
        return ERROR_SD_FAIL;
    }

    return NO_ERROR;
}

int counter::calibrate(int difference_threshold, int people_threshold){
    // Setting thresholds
    counter::difference_threshold = difference_threshold;
    counter::people_threshold = people_threshold;

    // Taking base image
    camera_fb_t* fb = esp_camera_fb_get();  
    if(!fb) {
        return ERROR_CAPTURE_FAILED;
    }

    // Saving base image on sd card.
    File file = fs.open("/base.hex", FILE_WRITE);
    if(!file){
        return ERROR_SD_FAIL;
    } else {
        file.write(fb->buf, fb->len); // payload (image), payload length
    }
    file.close();

    // Give ptr back.
    

    return NO_ERROR;
}

int counter::count(){
    // Predefining variables
    int output, people_count = 0;

    // Creating mapped buffer
    File file = fs.open("/base.hex", FILE_WRITE);
    if(!file){
        return ERROR_SD_FAIL;
    }  

    // Mallocing buffer
    counter::base_buffer = (uint8_t*)malloc(86400);
    if (base_buffer == NULL){
        return ERROR_MALLOC_FAILED;
    }

    // Reading bytes
    file.readBytes((char*)base_buffer, 86400);
    file.close();

    // Checking if base is not null
    if (base_buffer == NULL){
        return ERROR_NOT_CALIBRATED;
    }

    // Taking compare image
    camera_fb_t* compare_buffer = esp_camera_fb_get();  
    if(!compare_buffer) {
        return ERROR_CAPTURE_FAILED;
    }

    // Doing threshold operation
    output = counter::detection->threshold(base_buffer, compare_buffer,difference_threshold);
    if (output != NO_ERROR){
        return ERROR_DETECTION_FAILED;
    }
    
    // Doing blob detection operation
    output = counter::detection->blob_detection(compare_buffer, 10000, people_threshold);
    if (output < 0){
        return ERROR_DETECTION_FAILED;
    }

    return people_count;
}

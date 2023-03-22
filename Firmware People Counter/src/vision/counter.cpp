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

    // Creating mapped buffer
    counter::base_buffer = (uint8_t*)malloc(fb->len);
    if (base_buffer == NULL){
        return ERROR_MALLOC_FAILED;
    }

    // Copying mapped buffer to permanent
    counter::base_buffer = (uint8_t*)memcpy(counter::base_buffer, fb->buf, fb->len);
     if (base_buffer == NULL){
        return ERROR_MEMCPY_FAILED;
    }

    return NO_ERROR;
}

int counter::count(){
    // Predefining variables
    int output, people_count = 0;

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
    output = vision_threshold(base_buffer, compare_buffer,difference_threshold);
    if (!output){
        return ERROR_DETECTION_FAILED;
    }
    
    // Doing blob detection operation
    output = vision_blob_detection(compare_buffer, people_threshold);
    if (!output){
        return ERROR_DETECTION_FAILED;
    }

    return people_count;
}

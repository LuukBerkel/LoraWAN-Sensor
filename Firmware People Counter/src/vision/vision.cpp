#include "vision.hpp"
#include <stdio.h>
#include <Arduino.h>

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

bool vision::begin(){
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
        return false;
    }    

    return true;
}

bool vision::calibrate(int difference_threshold, int people_threshold){
    // Setting thresholds
    vision::difference_threshold = difference_threshold;
    vision::people_threshold = people_threshold;

    // Taking base image
    camera_fb_t* fb = esp_camera_fb_get();  
    if(!fb) {
        Serial.println("photo failed");
        return false;
    }

    // Creating mapped buffer
    vision::base_buffer = (uint8_t*)malloc(fb->len);
    if (base_buffer == NULL){
        Serial.println("malloc failed");
        return false;
    }

    // Clearing buffer to zero
    vision::base_buffer = (uint8_t*)memcpy(vision::base_buffer, fb->buf, fb->len);
     if (base_buffer == NULL){
        Serial.println("memcpy failed");
        return false;
    }

    return true;
}

int validate_nearby(camera_fb_t* comp,  uint8_t* base, int index, int threshold){
    // Checking mapped buffer
    if (comp->buf[index] == 0){
        return 0;     
    }
    
    if (abs(comp->buf[index] - base[index]) < threshold){
            return 0;
    } 

    

    // If passed check nearby.
    // int total = 1;

    // // Top pixel
    // if (index - 320 > 0) { 
    //    total += validate_nearby(comp, base, (index-320), threshold );
    // }

    // // Right pixel
    // if (index + 1 < comp->len) { 
    //     total += validate_nearby(comp, base, (index+1), threshold );
    // }

    // // Bottom pixel
    // if (index + 320 < comp->len) { 
    //     total += validate_nearby(comp, base, (index+320), threshold );
    // }

    // // Left pixel
    // if (index - 1 > 0) { 
    //     total += validate_nearby(comp, base, (index-1), threshold );
    // }

    // if (total > 2) { 
    //     Serial.println(total);
    // }
    // comp->buf[index] = 0;


    return 0;
}

int vision::count(){
    // Checking if base is not null
    if (base_buffer == NULL){
        return -2;
    }

    // Taking compare image
    camera_fb_t* compare_buffer = esp_camera_fb_get();  
    if(!compare_buffer) {
        return -1;
    }

    // Counting algorithm
    int people_count = 0;
    for (int i = 0; i < compare_buffer->len; i++)
    {
         // Validation if not mapped"
        if (i > 0 && i <10){
            int diff = compare_buffer->buf[i];
            Serial.print("Difference: ");
            Serial.print(base_buffer[i]);
            Serial.println(diff);
        }

        if (validate_nearby(compare_buffer, base_buffer, i, difference_threshold) > people_threshold){
            people_count = people_count + 1;
        }
    }

    esp_camera_fb_return(compare_buffer);

    return people_count;
}

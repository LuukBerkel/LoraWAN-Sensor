#include "vision.hpp"

int vision_threshold(uint8_t* base, camera_fb_t* compare, int threshold){
    // checking input values
    if (base == nullptr || compare == nullptr || threshold < 1){
        return ERROR_INVALID_INPUT;
    }

    for (size_t i = 0; i < compare->len; i++)
    {
        if (abs(base[i]-compare->buf[i]) > threshold){
            compare->buf[i] = 1;
        } else {
            compare->buf[i] = 0;
        }
    }

    return NO_ERROR;
}

int vision_blob_detection(camera_fb_t* compare,  int threshold){
    
}




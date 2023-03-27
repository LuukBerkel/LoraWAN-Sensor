#include "vision.hpp"
#include "Arduino.h"

vision::vision(int width, int height){
    vision::height = height;
    vision::width = width;
}

/*********EXPLANATION*************
Manner of thresholding by calculating diff,
between pixels and applying threshold.
**************************************/
int vision::threshold(uint8_t* base, camera_fb_t* compare, int threshold){
    // checking input values
    if (base == nullptr || compare == nullptr || threshold < 1){
        return ERROR_INVALID_INPUT;
    }

    // Thresholding operation
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

int vision::blob_detection(camera_fb_t* compare, int base, int threshold){
    // checking input values
    if (compare == nullptr || threshold < 1){
        return ERROR_INVALID_INPUT;
    }

    // Thresholding operation
    int total = 0;
    for (size_t i = 0; i < compare->len; i++)
    {
        if (compare->buf[i]==1){
            total = total + 1;
        }
    }

    // Removing base and divide by threshold
    return ((total - base) / threshold);
}

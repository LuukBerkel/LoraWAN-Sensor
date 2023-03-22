#ifndef VISION_H
#define VISION_H

#include "esp_camera.h"
#include <cstdlib>

// Error codes
#define NO_ERROR 1
#define ERROR_INVALID_INPUT -1
#define ERROR_THRESHOLD -2

/// @brief Threshold sets a zero in the compare buffer if difference between is below threshold, or a one if above
/// @param base The base buffer that doesn't get edited
/// @param compare The compare buffer that does get edited to a zero or a one
/// @param threshold The threshold that is used that has to be smaller then difference
int vision_threshold(uint8_t* base, camera_fb_t* compare, int threshold);

/// @brief Blob_detection does the blob detection on an image and gives back the amount of blobs bigger then threshold
/// @param compare The compare buffer that needs to be set to zero or one
/// @param threshold The minimum surface required for the amount to increment
/// @return The amount of people counted
int vision_blob_detection(camera_fb_t* compare,  int threshold);

#endif
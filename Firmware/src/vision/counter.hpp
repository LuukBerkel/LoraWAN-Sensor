#ifndef COUNTER_H
#define COUNTER_H

#include "esp_camera.h"

// Error codes
#define NO_ERROR 1
#define ERROR_NOT_CALIBRATED -1
#define ERROR_CAPTURE_FAILED -2
#define ERROR_MALLOC_FAILED -2
#define ERROR_MEMCPY_FAILED -3
#define ERROR_DETECTION_FAILED -4
#define ERROR_CAMERA_FAIL -5

// The class
class counter
{
private:
    int difference_threshold;
    int people_threshold;
    uint8_t* base_buffer = NULL;
public:
    /// @brief Begin starts the camera for measurements
    /// @return An error code or no error
    int begin();

    /// @brief Calibrate makes a base photo for the counter
    /// @param difference_threshold The minium difference needed
    /// @param people_threshold The minium blob size needed for human
    /// @return An error code or no error.
    int calibrate(int difference_threshold, int people_threshold);

    /// @brief Counts the people in the room
    /// @return The count result or one of the error codes
    int count();
};

#endif
#ifndef VISION
#define VISION

#include "esp_camera.h"

class vision
{
private:
    int difference_threshold;
    int people_threshold;
    uint8_t* base_buffer = NULL;
public:
    /// @brief begin starts the camera for measurmetns
    /// @return a bool true or false.
    bool begin();

    /// @brief calibrate makes a base photo for the counter.
    /// @param difference_threshold the minium difference needed
    /// @param people_threshold the minium blob size needed for human
    /// @return a bool true or false.
    bool calibrate(int difference_threshold, int people_threshold);

    /// @brief counts the people in the room.
    /// @return the count or -1 if init error -2.
    int count();
};

#endif
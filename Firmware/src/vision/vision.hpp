#ifndef VISION_H
#define VISION_H

#include "esp_camera.h"
#include <cstdlib>

// Error codes
#define NO_ERROR 1
#define ERROR_INVALID_INPUT -1
#define ERROR_THRESHOLD -2
#define ERROR_GROUP_OVERFLOW -3

class vision
{
private:
    // Img parameters
    int width, height;

    uint8_t* blob_get_pointer(camera_fb_t* compare, int row, int column, int row_len, int column_len);

    void blob_bordering_pointers(camera_fb_t* compare, uint8_t* pointers[4], int row, int column, int row_len, int column_len);

    int blob_sort_indexer(camera_fb_t* compare, int group, int row_len, int column_len);

    void blob_sort_replacer(camera_fb_t* compare, int group, int new_group, int row_len, int column_len);

    /// @brief The blob scanner is a private method that will group all blobs and throw out all that to small.
    void blob_scanner(camera_fb_t* compare, int* row, int* column, int* group);

    void blob_merger(camera_fb_t* compare, int* row_max, int* column_max, int* group);

    void blob_thresholder(camera_fb_t* compare,  int* row_max, int* column_max, int* group, int threshold);

    /// @brief The blob counter counts the number of groups detected.
    int blob_count();
public:
    /// @brief Constructor that needs height and width of image
    /// @param width 
    /// @param height 
    vision(int width, int height);

    /// @brief Threshold sets a zero in the compare buffer if difference between is below threshold, or a one if above
    /// @param base The base buffer that doesn't get edited
    /// @param compare The compare buffer that does get edited to a zero or a one
    /// @param threshold The threshold that is used that has to be smaller then difference
    int threshold(uint8_t* base, camera_fb_t* compare, int threshold);

    /// @brief Blob_detection does the blob detection on an image and gives back the amount of blobs bigger then threshold
    /// @param compare The compare buffer that needs to be set to zero or one
    /// @param threshold The minimum surface required for the amount to increment
    /// @return The amount of people counted
    int blob_detection(camera_fb_t* compare,  int threshold);
};

#endif
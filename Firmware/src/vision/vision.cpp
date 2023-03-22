#include "vision.hpp"

vision::vision(int width, int height){
    vision::height = height;
    vision::width = width;
}

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

/*********EXPLANATION*************
Manner of indexing. 
result = target +  row * buffer_width + column

index of example below:
* = row * buffer_width
+ = column

| | | | | | |t|*|*|
|*|*|*|*|*|*|+|x| |
| | | | | | | | | |
**************************************/
uint8_t* vision::blob_get_pointer(camera_fb_t* compare, uint8_t* target, int row, int column, int row_len, int column_len){
    if (column >= column_len || row >= row_len || target + ((row*width)+column) > compare->buf+(compare->len-1)){
        return nullptr;
    }

    return ((uint8_t*)target + ((row*width)+column));
}

/************EXPLANATION***************
Order of bordering points in result:
            | |0| |
            |1|x|3|
            | |2| |
**************************************/
void vision::blob_bordering_pointers(camera_fb_t* compare, uint8_t* target, uint8_t* pointers[4], int row, int column, int row_len, int column_len){
    pointers[0] = blob_get_pointer(compare, target, row-1, column, row_len, column_len);
    pointers[1] = blob_get_pointer(compare, target, row, column-1, row_len, column_len);
    pointers[2] = blob_get_pointer(compare, target, row+1, column, row_len, column_len);
    pointers[3] = blob_get_pointer(compare, target, row, column+1, row_len, column_len);
}

void vision::blob_scanner(camera_fb_t* compare, uint8_t* target, int column_len, int row_len){
    // Group index.
    int group = 2;
    // Scanning and grouping each pixel.
    for (size_t row = 0; row < row_len; row++)
    {
        for (size_t column = 0; column < column_len; column++)
        {
            uint8_t* index = vision::blob_get_pointer(compare, target, row, column, row_len, column_len);
            if (index != nullptr &&*index == 1){
                uint8_t* surrounding[4];
                blob_bordering_pointers(compare,  target, surrounding, row, column, row_len,  column_len);
                for (size_t i = 0; i < 4; i++)
                {
                     if (surrounding[i] != nullptr && *surrounding[i] > 1){
                        *index = *surrounding[i];
                     }
                }

                if (*index == 1){
                    *index = group;
                    group = group + 1;
                }
            }
        }
        
    }
    
}

void blob_joiner(camera_fb_t* compare, int quadrant_width, int quadrant_heigth, int target_width, int target_heigt){
    //  // Every number that is one will be grouped from 2 to 255.
    // for (size_t row = 0; row < 8; row++)
    // {
    //     for (size_t column = 0; column < 8; column++)
    //     {
    //         if (target[(row*8)+column])  {

    //         }
    //     }
    // }
}

int vision::blob_detection(camera_fb_t* compare,  int threshold){
    return 0;
}



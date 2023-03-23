#include "vision.hpp"

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
uint8_t* vision::blob_get_pointer(camera_fb_t* compare, int row, int column, int row_len, int column_len){
    if (column >= column_len || row >= row_len || ((row*width)+column) > compare->len-1){
        return nullptr;
    }

    return ((uint8_t*)compare->buf + ((row*width)+column));
}

/************EXPLANATION***************
Order of bordering points in result:
            | |0| |
            |1|x|3|
            | |2| |
**************************************/
void vision::blob_bordering_pointers(camera_fb_t* compare, uint8_t* pointers[4], int row, int column, int row_len, int column_len){
    pointers[0] = blob_get_pointer(compare, row-1, column, row_len, column_len);
    pointers[1] = blob_get_pointer(compare, row, column-1, row_len, column_len);
    pointers[2] = blob_get_pointer(compare, row+1, column, row_len, column_len);
    pointers[3] = blob_get_pointer(compare, row, column+1, row_len, column_len);
}

/************EXPLANATION***************
Counts all the pixels of a group in section and give count back.
**************************************/
int vision::blob_sort_indexer(camera_fb_t* compare, int group, int row_len, int column_len){
    int result = 0;
    for (size_t row = 0; row < row_len; row++)
    {
        for (size_t column = 0; column < column_len; column++)
        {
            uint8_t* ptr = blob_get_pointer(compare, row, column, row_len, column_len);
            if (*ptr == group){
                result = result + 1;
            }
        }
    }

    return result;
}

/************EXPLANATION***************
Replaces all pixels of a group with new_group number.
**************************************/
void vision::blob_sort_replacer(camera_fb_t* compare, int group, int new_group, int row_len, int column_len){
    for (size_t row = 0; row < row_len; row++)
    {
        for (size_t column = 0; column < column_len; column++)
        {
             uint8_t* ptr = blob_get_pointer(compare, row, column, row_len, column_len);
            if (*ptr == group){
                *ptr = new_group;
            }
        }
    }
}


void vision::blob_scanner(camera_fb_t* compare, int* row, int* column, int* group){
    for (row = row; *row < vision::height; *row= *row + 1)
    {
        for (column = column; *column < vision::width; *column = *column + 1)
        {
            uint8_t* index = vision::blob_get_pointer(compare, *row, *column, vision::height, vision::width);
            if (index != nullptr &&*index == 1){
                // Checking surroundings
                uint8_t* surrounding[4];
                blob_bordering_pointers(compare, surrounding, *row, *column, vision::height, vision::width);
                for (size_t i = 0; i < 4; i++)
                {
                    if (surrounding[i] != nullptr && *surrounding[i] > 1){
                        *index = *surrounding[i];
                        break;
                    }
                }
                
                // Creating new group if no surroundings
                if (*index == 1){
                    *index = *group;

                    // Validating if now group overflow
                    if (*group + 1 < 256){
                        *group = *group + 1;

                    // Else cleanup groups and restart again.
                    } else {
                        return;
                    }
                }
            }
        }
    }
}

void vision::blob_merger(camera_fb_t* compare, int* row_max, int* column_max, int* group){
    // Creating merge list
    int merge_list[256] = {0};

    // Adding info in merge list
    for (int row = 0; row <= *row_max; row++)
    {
        for (int column = 0; column <= *column_max; column++)
        {
            uint8_t* index = vision::blob_get_pointer(compare, row, column, vision::height, vision::width);
            if (index != nullptr &&*index > 1){
                // Checking surroundings
                uint8_t* surrounding[4];
                blob_bordering_pointers(compare, surrounding, row, column, vision::height, vision::width);
                for (size_t i = 0; i < 4; i++)
                {
                    if (surrounding[i] != nullptr && *surrounding[i] > 1 && *index != *surrounding[i]){
                        // Insert in list if it does not yet exist.
                        if (merge_list[*index] != *surrounding[i] && merge_list[*surrounding[i]] != *index){
                            int merge_index = *index;
                            while (merge_index != 0){
                                merge_index = merge_list[merge_index];
                            }
                            merge_list[merge_index] =  *surrounding[i];
                        }
                    }
                }
            }
        }
    }

    // Doing merges from list
    for (size_t i = 2; i < 256; i++)
    {
        if (merge_list[i] > 1){
            blob_sort_replacer(compare, i, merge_list[i], vision::height, vision::width);
        } 
    }
}

void vision::blob_thresholder(camera_fb_t* compare,  int* row_max, int* column_max, int* group, int threshold){
     // Creating remove list and removing to small groups
    bool remove_list[256];
    for (size_t i = 2; i <= *group; i++)
    {
        if (vision::blob_sort_indexer(compare, i,  vision::height, vision::width) < threshold){
            blob_sort_replacer(compare, i, 0, vision::height, vision::width);
            remove_list[i] = true;
        }
    }

    // Reindexing groups to free space.
    int new_group = 2;
    for (size_t i = 2; i <= *group; i++)
    {
        if (!remove_list[i] && new_group != i){
            blob_sort_replacer(compare, i, new_group, vision::height, vision::width);
            new_group = new_group + 1;
        } 
    }

    // Setting new group index
    *group = new_group;
}

int vision::blob_detection(camera_fb_t* compare,  int threshold){
    // Indexes of the algorithm
    int group = 2, row = 0, column = 0;

    // Scanning and grouping each pixel.
    while(row < vision::height && column < vision::width){
        // Doing blob scan and grouping everthing.
        blob_scanner(compare, &row, &column, &group);

        // Merging groups together and reindexing.
        blob_merger(compare, &row, &column, &group);

        // Counting blobs left and throwing out to small blobs and reindexing. 
        blob_thresholder(compare, &row, &column, &group, threshold);

        // Checking if group count is below max when not on end
        if (group >= 255 && row < vision::height && column < vision::width){
            return ERROR_GROUP_OVERFLOW;
        }
    }

    // Returning group count minus ofsett
    return group-1;
}

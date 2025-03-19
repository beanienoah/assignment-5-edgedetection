#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include "sobel.h"

// TODO - write your pthread-ready sobel filter function here for a range of rows

// basically: my attempt at translating your explanation of convolution. there were
// some online resources that helped explain, but most of this is just a general
// -ization of your example in a way that made sense to me
void *sobel_filter(void *arg) {
    ThreadStuffs *rows = (ThreadStuffs *) arg;
    int start = rows->start_row;
    int end = rows->end_row;

    // exclude borders from processing (which i hope counts as setting them to 0)
    if(start < 1) {
        start = 1;
    }
    if(end > height - 2) {
        end = height - 2;
    }

    // pixel processing in row, skip first and last columns
    for(int i = start; i <= end; i++) {
        for(int j = 1; j <= width - 1; j++) {
            int x = 0;
            int y = 0;
            // sobel stuff over 3x3 area
            for(int three = -1; three <= 1; three++) { // offset 1
                for(int bythree = -1; bythree <= 1; bythree++) { // offset 2
                    int some_pixel = input_image[i + three][bythree + j]; // surrounding pixel
                    x += some_pixel * Kx[three + 1][bythree + 1]; // applying sobel kernels
                    y += some_pixel * Ky[three + 1][bythree + 1]; // grabs gradients (x and y)
                }
            }
            // fun stuff. gradients and threshold processing
            int gradient_check = (int)sqrt(x*x + y*y); // per your G equation
            if (gradient_check > 255) {
                gradient_check = 255; // clamping edge case
            }
            if(gradient_check > threshold) { // edge detection
                output_image[i][j] = WHITE; // edge tetected
            } else {
                output_image[i][j] = BLACK; // edge not detected
            }
        }
    }
    return NULL;
}

// Note: You have access to all the global variables here.
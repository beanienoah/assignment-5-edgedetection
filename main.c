#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sobel.h"
#include "rtclock.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Sobel kernels
int Kx[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

int Ky[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};

// Globals: Image and threading data
unsigned char **input_image;
unsigned char **output_image;
unsigned char threshold = 127;
int width, height;
int num_threads;

/**
 * Main method

 Handles user inputs for filename, number of threads, and threshold, 
 if no bad inputs, it then loads the image file, runs that image through some
 sobel filtering shenanigans, and splits the work up between a given # of
 threads. When work is done, displays the time spent, and saves the file
 in the correct format (which was annoying but whatever, tested it using a generic
 format, and it worked well, so it irked me more than it should've lol). 
 */
int main(int argc, char *argv[]) {
    // TODO - Handle command line inputs (and give errors for bad inputs)
    if (argc < 4) {
        printf("> Usage: ./sobel <input-file> <num_threads (>= 1)> <threshold (0-255)>\n");
        return -1;
    }

    char *filename = argv[1];
    num_threads = atoi(argv[2]); // convert to int to parse
    if(num_threads <= 0){
        printf("ERROR: num_threads must be >=1.\n");
        return -1;
    }
    
    threshold = (unsigned char)atoi(argv[3]); 
    if(threshold < 0 || threshold > 255) {
        printf("ERROR: threshold must be between 0 and 255.\n");
        return -1;
    }

    // TODO - Read image file into array a 1D array (see assignment write-up)
    unsigned char *data = stbi_load(filename, &width, &height, NULL, 1);


    // setting up 1d -> 2d image data
    input_image = (unsigned char**) malloc(sizeof(unsigned char*) * height);
    output_image = (unsigned char**) malloc(sizeof(unsigned char*) * height);
    
    // remap 1d to 2d, and set up rows
    for(int i = 0; i < height; i++) {
        input_image[i] = &data[i * width];
        output_image[i] = (unsigned char*) malloc(width * sizeof(unsigned char));
    }

    printf("Loaded %s. Height=%d, Width=%d\n", filename, height, width);

    // Start clocking!
    double startTime, endTime;
    startTime = rtclock();

    // TODO - Prepare and create threads (simpler than it looks)
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    ThreadStuffs *thread_stuffs = malloc(sizeof(ThreadStuffs) * num_threads);

    // work distribution (even, in case of remainder just tack it on as evenly as possible)
    int rows_per = height / num_threads;
    int hero = height % num_threads;
    int im_here = 0;

    for(int i = 0; i < num_threads; i++) {
        thread_stuffs[i].start_row = im_here;
        thread_stuffs[i].end_row = im_here + rows_per - 1;
        if(hero > 0) {
            thread_stuffs[i].end_row++;
            hero--;
        }
        im_here = thread_stuffs[i].end_row + 1;
        pthread_create(&threads[i], NULL, sobel_filter, (void *)&thread_stuffs[i]); // split that work!
    } 

    // TODO - Wait for threads to finish
    for(int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // End clocking!
    endTime = rtclock();
    printf("Time taken (thread count = %d): %.6f sec\n", num_threads, (endTime - startTime));

    // TODO - Save the file!
    // (just your example code)
    unsigned char *array1D = (unsigned char *) malloc(width * height * sizeof(unsigned char));
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            array1D[i*width+j] = output_image[i][j];
        }
    }

    char outname[256]; // filepath
    char *split = strchr(filename, '.'); // find the split ('.')
    // okay so this is cool. turns out you can use the cutoff stuff from the above
    // (for decimals in the clock) for building specific strings. thanks stackoverflow
    snprintf(outname, sizeof(outname), "%.*s-sobel.jpg", (int)(split-filename), filename);
    stbi_write_jpg(outname, width, height, 1, array1D, 80);
    printf("Saved as %s\n", outname);

    // TODO - Free allocated memory
    free(array1D);
    // only 2d array, free rows
    for(int i = 0; i < height; i++) {
        free(output_image[i]);
        output_image[i] = NULL;
    }
    free(output_image);
    free(input_image); // fake 2d array booooooo get out of here lameo
    return 0;
}

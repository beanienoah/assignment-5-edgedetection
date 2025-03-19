#define BLACK 0
#define WHITE 255

// These globals are declared here, but defined externally in another .c file
extern unsigned char **input_image;
extern unsigned char **output_image;
extern unsigned char threshold;
extern int width, height;
extern int num_threads;
extern int Kx[3][3];
extern int Ky[3][3];

// Thread function argument - TODO
typedef struct { // implemented a struct like you said :D
    int start_row;
    int end_row;
} ThreadStuffs;

// Function declarations - TODO
void *sobel_filter(void *arg);
//To compile (linux/mac): gcc cbmp.c main.c -o main.out -lm
//To run (linux/mac): ./main.out example.bmp example_inv.bmp
//To compile (win): gcc cbmp.c main.c -o main.exe -lm
//To run (win): main.exe example.bmp example_inv.bmp

#include "cbmp.h"
#include <math.h>
#include "minmax.h"
#include <stdlib.h>
#include <stdio.h>
#include "time.h"

//making a linked list of all detected cells
typedef struct cell {
    int x;
    int y;
    struct cell *next;
} cell;


//Function to print the linked list of cells
void printCell(cell *head) {
    cell *current = head;
    while (current != NULL) {
        printf("x: %i, y: %i\n", current->x, current->y);
        current = current->next;
    }
}

//Function to count the number of cells in the linked list
int countCells(cell *head) {
    cell *current = head;
    int count = 0;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

//Function to check if a cell exists in the linked list
int cellExists(cell *head, int x, int y) {
    cell *current = head;
    while (current != NULL) {
        if (current->x == x && current->y == y) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}


//Function to convert an image to greyscale to save memory
void greyscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
               unsigned char temp_image[BMP_WIDTH+2][BMP_HEIGTH+2]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            int grey = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3;
            temp_image[x+2][y+2] = grey;
        }
    }
}

void black_white(unsigned char inputImage[BMP_WIDTH+2][BMP_HEIGTH+2], int threshold) {
    for (int x = 2; x < BMP_WIDTH; x++) {
        for (int y = 2; y < BMP_HEIGTH; y++) {
            unsigned char bw = (inputImage[x][y] > threshold) ? 255 : 0;
            inputImage[x][y] = bw;
        }
    }
}


//creating the gaussian kernel
void create_gaussian_kernel(double kernel[][5], int kernel_size, double sigma) {
    double sum = 0.0;
    int half_size = kernel_size / 2;
    for (int x = -half_size; x <= half_size; x++) {
        for (int y = -half_size; y <= half_size; y++) {
            kernel[x + half_size][y + half_size] = (1.0 / (2.0 * M_PI * sigma * sigma)) *
                                                   exp(-(x * x + y * y) / (2 * sigma * sigma));
            sum += kernel[x + half_size][y + half_size];
        }
    }

    // Normalize the kernel
    for (int i = 0; i < kernel_size; i++) {
        for (int j = 0; j < kernel_size; j++) {
            kernel[i][j] /= sum;
        }
    }
}

void gaussian_filter(unsigned char inputImage[BMP_WIDTH+2][BMP_HEIGTH+2],
                     unsigned char outputImage[BMP_WIDTH+2][BMP_HEIGTH+2]) {
    // Define the size of the Gaussian kernel, typically 5x5 or 3x3
    int kernel_size = 5;
    // Define the standard deviation for the Gaussian distribution, typically 1.0
    double sigma = 10;
    // Create the Gaussian kernel
    double kernel[kernel_size][kernel_size];
    create_gaussian_kernel(kernel, kernel_size, sigma);
    // For each pixel in the input image:
    for (int x = 2; x <= BMP_WIDTH+1; x++) {
        for (int y = 2; y <= BMP_HEIGTH+1; y++) {
            // Multiply the surrounding pixels by the corresponding values in the Gaussian kernel
            // Sum up these values
            double sum = 0.0;
            for (int i = -kernel_size / 2; i <= kernel_size / 2; i++) {
                for (int j = -kernel_size / 2; j <= kernel_size / 2; j++) {
                    int x_loc = min(max(x + i, 0), BMP_WIDTH + 1);
                    int y_loc = min(max(y + j, 0), BMP_HEIGTH + 1);
                    sum += inputImage[x_loc][y_loc] * kernel[i + kernel_size / 2][j + kernel_size / 2];
                }
            }
            // Assign this sum to the corresponding pixel in the output image
            outputImage[x][y] = min(max((int) sum, 0), 255);
        }
    }
}

int otsu_threshold(unsigned char inputImage[BMP_WIDTH+2][BMP_HEIGTH+2]) {
    int histogram[256] = {0};
    int total_pixels = (BMP_WIDTH+2) * (BMP_HEIGTH+2);

    // Calculate histogram
    for (int x = 0; x < BMP_WIDTH+2; x++) {
        for (int y = 0; y < BMP_HEIGTH+2; y++) {
            histogram[inputImage[x][y]]++;
        }
    }

    float sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += i * histogram[i];
    }

    float sumB = 0;
    int wB = 0;
    int wF;
    float varMax = 0;
    int threshold = 0;

    for (int i = 0; i < 256; i++) {
        wB += histogram[i];
        if (wB == 0) {
            continue;
        }

        wF = total_pixels - wB;
        if (wF == 0) {
            break;
        }

        sumB += (float) (i * histogram[i]);
        float mB = sumB / wB;
        float mF = (sum - sumB) / wF;
        float varBetween = (float) wB * (float) wF * (mB - mF) * (mB - mF);

        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = i;
        }
    }

    return threshold;
}


int erode(unsigned char inputImage[BMP_WIDTH+2][BMP_HEIGTH+2],
           unsigned char outputImage[BMP_WIDTH+2][BMP_HEIGTH+2]) {
    int eroded = 1;
    // Define the structuring element
    int kernel[3][3] = {{0, 1, 0},
                        {1, 1, 1},
                        {0, 1, 0}};
    // For each pixel in the image
    for (int x = 2; x <= BMP_WIDTH+1; x++) {
        for (int y = 2; y <= BMP_HEIGTH+1; y++) {
            // If the pixel is not at the border, check the neighborhood defined by the structuring element
            int isEroded = 0;
            if (inputImage[x][y] == 255) {
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        // If the pixel in the neighborhood is 0 (black) and the corresponding pixel in the structuring element is 1, set the pixel to 0
                        if (inputImage[x + i][y + j] == 0 && kernel[i][j] == 1) {
                            isEroded = 1;
                        }
                    }
                }
                // If any pixel in the neighborhood is 0 (black), set the pixel to 0
                if (isEroded) {
                    outputImage[x][y] = 0;
                } else {
                    outputImage[x][y] = 255;
                    eroded = 0;
                }
            }
        }
    }
    return eroded;
}

void detectCell(unsigned char inputImage[BMP_WIDTH+2][BMP_HEIGTH+2], cell **head) {
    for (int x = 0; x < BMP_WIDTH+2; x++) {
        for (int y = 0; y < BMP_HEIGTH+2 ; y++) {
            int WhitePixelfound = 0;
            int ExclusionFrameBlack = 1;
            if(y==0){}

            // Check the exclusion frame and see if they all are black.
            for (int i = -5; i <= 6; i++) {
                for (int j = -5; j <= 6; j++) {
                    if (i == -5 || i == 6 || j == -5 || j == 6) {
                        if (inputImage[x + i][y + j] != 0) { 
                            ExclusionFrameBlack = 0;
                            break;
                        }
                    }
                }
                if (!ExclusionFrameBlack) {
                    break;
                }
            }

            // If the exclusion frame is black, check the capturing area
            if (ExclusionFrameBlack) {
                for (int i = -4; i < 6; i++) {
                    for (int j = -4; j < 6; j++) {
                        if (inputImage[x + i][y + j] == 255) { 
                            WhitePixelfound = 1;
                        }
                    }
                }
            }

            // If at least one white pixel is found inside and the exclusion frame is black, register a cell
            if (WhitePixelfound && ExclusionFrameBlack) {
                if (!cellExists(*head, x, y)) {
                    cell *new_cell = (cell *) malloc(sizeof(cell));
                    if (new_cell == NULL) {
                        fprintf(stderr, "Failed to allocate memory for new cell.\n");
                        exit(1);
                    }
                    new_cell->x = x;
                    new_cell->y = y;
                    new_cell->next = *head;
                    *head = new_cell;
                }

                // Set the entire capturing area to black to avoid detecting the same cell again
                for (int i = -4; i < 6; i++) {
                    for (int j = -4; j < 6; j++) {
                        inputImage[x + i][y + j] = 0; 
                    }
                }
            }
        }
    }
}

void drawDot(unsigned char inputImage[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], cell *head) {
    cell *current = head;
    while (current != NULL) {
        for (int x = 0; x < 11; x++) {
            for (int y = 0; y < 11; y++) {
                if(head->x + x >= BMP_WIDTH || head->y + y >= BMP_HEIGTH){
                    continue;
                }
                inputImage[current->x + x][current->y + y][0] = 88;
                inputImage[current->x + x][current->y + y][1] = 4;
                inputImage[current->x + x][current->y + y][2] = 108;
            }
        }
        current = current->next;
    }
}

void blackBorder(unsigned char inputImage[BMP_WIDTH+2][BMP_HEIGTH+2]){
    for (int x = 0; x < BMP_WIDTH+2; x++) {
        for (int y = 0; y < BMP_HEIGTH+2; y++) {
            if(x==0 || x==BMP_WIDTH+1 || y==0 || y==BMP_HEIGTH+1){
                inputImage[x][y] = 0;
            }
        }
    }
}

//Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char temp_image[BMP_WIDTH+2][BMP_HEIGTH+2];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];


//Creating a linked list of cells
cell *head = NULL;

//Main function
int main(int argc, char **argv) {
    //argc counts how may arguments are passed
    //argv[0] is a string with the name of the program
    //argv[1] is the first command line argument (input image)
    //argv[2] is the second command line argument (output image)
    clock_t begin = clock();

    //Checking that 2 arguments are passed
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file path> <output file path>\n", argv[0]);
        exit(1);
    }

    printf("Example program - 02132 - A1\n");

    //Load image from file
    read_bitmap(argv[1], input_image);
    read_bitmap(argv[1], output_image);




    //Run greyscale filter in case the image is colored
    greyscale(input_image, temp_image);


    //Run gaussian filter and then making the temp_image black and white
    gaussian_filter(temp_image, temp_image);
    black_white(temp_image, otsu_threshold(temp_image));
    blackBorder(temp_image);

    //run erosion to test for now
    while (erode(temp_image, temp_image)==0) {
        detectCell(temp_image, &head);
    }

    printCell(head);
    printf("Number of cells: %i\n", countCells(head));

    drawDot(output_image, head);


    //Save image to file
    write_bitmap(output_image, argv[2]);

    printf("Done!\n");
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Time spent: %f seconds", time_spent);
    return 0;
}

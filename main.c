//To compile (linux/mac): gcc cbmp.c main.c -o main.out -lm
//To run (linux/mac): ./main.out example.bmp example_inv.bmp
//To compile (win): gcc cbmp.c main.c -o main.exe -lm
//To run (win): main.exe example.bmp example_inv.bmp

#include "cbmp.h"
#include <math.h>
#include "minmax.h"
#include <stdlib.h>
#include <stdio.h>

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
               unsigned char temp_image[BMP_WIDTH][BMP_HEIGTH]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            char grey = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3;
            temp_image[x][y] = grey;
        }
    }
}

void erosionToTemp(unsigned char inputImage[BMP_WIDTH][BMP_HEIGTH],
                   unsigned char outputImage[BMP_WIDTH][BMP_HEIGTH], int threshold) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            unsigned char bw = (inputImage[x][y] > threshold) ? 255 : 0;
            outputImage[x][y] = bw;

        }
    }
}

void black_and_white(unsigned char inputImage[BMP_WIDTH][BMP_HEIGTH],
                     unsigned char outputImage[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int threshold) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            unsigned char bw = (inputImage[x][y] > threshold) ? 255 : 0;
            for (int c = 0; c < BMP_CHANNELS; c++) {
                outputImage[x][y][c] = bw;
            }
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

void gaussian_filter(unsigned char inputImage[BMP_WIDTH][BMP_HEIGTH],
                     unsigned char outputImage[BMP_WIDTH][BMP_HEIGTH]) {
    // Define the size of the Gaussian kernel, typically 5x5 or 3x3
    int kernel_size = 5;
    // Define the standard deviation for the Gaussian distribution, typically 1.0
    double sigma = 1.0;
    // Create the Gaussian kernel
    double kernel[kernel_size][kernel_size];
    create_gaussian_kernel(kernel, kernel_size, sigma);
    // For each pixel in the input image:
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            // Multiply the surrounding pixels by the corresponding values in the Gaussian kernel
            // Sum up these values
            double sum = 0.0;
            for (int i = -kernel_size / 2; i <= kernel_size / 2; i++) {
                for (int j = -kernel_size / 2; j <= kernel_size / 2; j++) {
                    int x_loc = min(max(x + i, 0), BMP_WIDTH - 1);
                    int y_loc = min(max(y + j, 0), BMP_HEIGTH - 1);
                    sum += inputImage[x_loc][y_loc] * kernel[i + kernel_size / 2][j + kernel_size / 2];
                }
            }
            // Assign this sum to the corresponding pixel in the output image
            outputImage[x][y] = min(max((int) sum, 0), 255);
        }
    }
}

int otsu_threshold(unsigned char inputImage[BMP_WIDTH][BMP_HEIGTH]) {
    int histogram[256] = {0};
    int total_pixels = BMP_WIDTH * BMP_HEIGTH;

    // Calculate histogram
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            histogram[inputImage[x][y]]++;
        }
    }

    float sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += i * histogram[i];
    }

    float sumB = 0;
    int wB = 0;
    int wF = 0;
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


void erode(unsigned char inputImage[BMP_WIDTH][BMP_HEIGTH],
           unsigned char outputImage[BMP_WIDTH][BMP_HEIGTH]) {
    // Define the structuring element
    int kernel[3][3] = {{0, 1, 0},
                        {1, 1, 1},
                        {0, 1, 0}};

    // For each pixel in the image
    for (int x = 0; x < BMP_WIDTH ; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            // If the pixel is not at the border, check the neighborhood defined by the structuring element
            int isEroded = 0;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j <= 3; j++) {
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
            }

        }
    }
}


void detectCell(unsigned char inputImage[BMP_WIDTH][BMP_HEIGTH], cell **head) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            if (inputImage[x][y] == 255) {
                for (int i = 0; i <= 11; i++) {
                    for (int j = 0; j <= 11; j++) {
                        if (i ==0 || i == 11 || j == 0 || j == 11) {
                            if (inputImage[x][y] == 255) {
                                continue; // Skip this cell if there's a white pixel in the exclusion zone
                            }
                        } else {
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
                        }
                    }
                }

            }
            y+=11;
        }
        x+=11;
    }
}



//Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char temp_image[BMP_WIDTH][BMP_HEIGTH];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

cell *head = NULL;

//Main function
int main(int argc, char **argv) {
    //argc counts how may arguments are passed
    //argv[0] is a string with the name of the program
    //argv[1] is the first command line argument (input image)
    //argv[2] is the second command line argument (output image)

    //Checking that 2 arguments are passed
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file path> <output file path>\n", argv[0]);
        exit(1);
    }

    printf("Example program - 02132 - A1\n");

    //Load image from file
    read_bitmap(argv[1], input_image);

    //Run greyscale filter in case the image is colored
    greyscale(input_image, temp_image);


    //Run gaussian filter and then making the temp_image black and white
    gaussian_filter(temp_image, temp_image);
    erosionToTemp(temp_image, temp_image, otsu_threshold(temp_image));

    //run erosion to test for now
    int i = 0;
    while (i < 5) {
        erode(temp_image, temp_image);
        i++;
    }

    detectCell(temp_image, &head);
    black_and_white(temp_image, output_image, otsu_threshold(temp_image));
    printCell(head);
    printf("Number of cells: %i\n", countCells(head));

    //Save image to file
    write_bitmap(output_image, argv[2]);

    printf("Done!\n");
    return 0;
}

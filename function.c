#include "function.h"
#include "minmax.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * \brief Prints the linked list of cells.
 *
 * \param head Pointer to the head of the linked list.
 */
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

/**
 * \brief Checks if a cell exists in the linked list.
 *
 * \param head Pointer to the head of the linked list.
 * \param x X-coordinate of the cell to check.
 * \param y Y-coordinate of the cell to check.
 * \return 1 if the cell exists, 0 otherwise.
 */
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


/**
 * \brief Converts an image to greyscale.
 *
 * \param input_image The input image array.
 * \param temp_image The temporary image array to store the greyscale image.
 */
void greyscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
               unsigned char temp_image[BMP_WIDTH + 2][BMP_HEIGTH + 2]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            int grey = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3;
            temp_image[x + 2][y + 2] = grey;
        }
    }
}


/**
 * \brief Converts an image to black and white based on a threshold.
 *
 * \param inputImage The input image array.
 * \param threshold The threshold value for conversion.
 */
void black_white(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2], int threshold) {
    for (int x = 2; x < BMP_WIDTH; x++) {
        for (int y = 2; y < BMP_HEIGTH; y++) {
            unsigned char bw = (inputImage[x][y] > threshold) ? 255 : 0;
            inputImage[x][y] = bw;
        }
    }
}


/**
 * \brief Creates a Gaussian kernel.
 *
 * \param kernel The kernel array to store the Gaussian values.
 * \param kernel_size The size of the kernel.
 * \param sigma The standard deviation for the Gaussian distribution.
 */
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


/**
 * \brief Applies a Gaussian filter to an image.
 *
 * \param inputImage The input image array.
 * \param outputImage The output image array to store the filtered image.
 */
void gaussian_filter(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2],
                     unsigned char outputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2]) {
    // Define the size of the Gaussian kernel
    int kernel_size = 5;
    // Define the standard deviation for the Gaussian distribution
    double sigma = 1.65;
    // Create the Gaussian kernel
    double kernel[kernel_size][kernel_size];
    create_gaussian_kernel(kernel, kernel_size, sigma);
    // For each pixel in the input image:
    for (int x = 2; x <= BMP_WIDTH + 1; x++) {
        for (int y = 2; y <= BMP_HEIGTH + 1; y++) {
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


/**
 * \brief Calculates the Otsu threshold for an image.
 *
 * \param inputImage The input image array.
 * \return The calculated threshold value.
 */
int otsu_threshold(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2]) {
    int histogram[256] = {0};
    int total_pixels = (BMP_WIDTH) * (BMP_HEIGTH);

    // Calculate histogram
    for (int x = 2; x < BMP_WIDTH; x++) {
        for (int y = 2; y < BMP_HEIGTH; y++) {
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

/**
 * \brief Applies erosion to an image.
 *
 * \param inputImage The input image array.
 * \param outputImage The output image array to store the eroded image.
 * \return 1 if the image is fully eroded, 0 otherwise.
 */
int erode(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2],
          unsigned char outputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2]) {
    int eroded = 1;
    // Define the structuring element
    int kernel[3][3] = {{0, 1, 0},
                        {1, 1, 1},
                        {1, 1, 0}};
    // For each pixel in the image
    for (int x = 2; x < BMP_WIDTH; x++) {
        for (int y = 2; y < BMP_HEIGTH; y++) {
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

/**
 * \brief Detects cells in an image and adds them to a linked list.
 *
 * \param inputImage The input image array.
 * \param head Pointer to the head of the linked list.
 */
void detectCell(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2], cell **head) {
    for (int x = 0; x < BMP_WIDTH + 2; x++) {
        for (int y = 0; y < BMP_HEIGTH + 2; y++) {
            int WhitePixelfound = 0;
            int ExclusionFrameBlack = 1;
            if (y == 0) {}

            // Check the exclusion frame and see if they all are black.
            for (int i = -4; i <= 4; i++) {
                for (int j = -4; j <= 4; j++) {
                    if (i == -4 || i == 4 || j == -4 || j == 4) {
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
                for (int i = -3; i < 4; i++) {
                    for (int j = -3; j < 4; j++) {
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
                for (int i = -3; i < 4; i++) {
                    for (int j = -3; j < 4; j++) {
                        inputImage[x + i][y + j] = 0;
                    }
                }
            }
        }
    }
}


/**
 * \brief Draws a dot on the image at the location of each detected cell.
 *
 * \param inputImage The input image array.
 * \param head Pointer to the head of the linked list of cells.
 */
void drawDot(unsigned char inputImage[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], cell *head) {
    cell *current = head;
    while (current != NULL) {
        // Draw a simple representation of the DTU logo
        for (int x = 0; x < 12; x++) {
            for (int y = 0; y < 14; y++) {
                if (current->x + x >= BMP_WIDTH || current->y + y >= BMP_HEIGTH) {
                    continue;
                }
                // Example pattern for DTU logo

                inputImage[current->x + x][current->y + y][0] = 189; // Red
                inputImage[current->x + x][current->y + y][1] = 42;  // Green
                inputImage[current->x + x][current->y + y][2] = 48;  // Blue
                if (x==1 && y>0 && y<4) {
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if(y==1&&(x==1||x==2)){
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if(y==3&&x==2){
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if(y==2&&x==3){
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if(x>=4&&x<=6&&y==1){
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if(x==5&&y>1&&y<4){
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if(x==8&&y>0&&y<4){
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if(x==9&&y==3){
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if(x==10&&y>0&&y<4) {
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if(y==7&&(x>0&&x<11)){
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if((y==6||y==8)&&(x==2||x==3||x==8||x==9)){
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if(y==11&&(x>0&&x<11)){
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
                if((y==10||y==12)&&(x==2||x==3||x==8||x==9)){
                    inputImage[current->x + x][current->y + y][0] = 255; // White
                    inputImage[current->x + x][current->y + y][1] = 255; // White
                    inputImage[current->x + x][current->y + y][2] = 255; // White
                }
            }
        }
        current = current->next;
    }
}

/**
 * \brief Adds a black border to the image.
 *
 * \param inputImage The input image array.
 */
void blackBorder(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2]) {
    for (int x = 0; x < BMP_WIDTH + 2; x++) {
        for (int y = 0; y < BMP_HEIGTH + 2; y++) {
            if (x == 0 || x >= BMP_WIDTH || y == 0 || y >= BMP_HEIGTH) {
                inputImage[x][y] = 0;
            }
        }
    }
}

void tempImageToPrint(unsigned char temp_image[BMP_WIDTH + 2][BMP_HEIGTH + 2], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]){
    for (int x = 2; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            output_image[x][y][0] = temp_image[x + 2][y + 2];
            output_image[x][y][1] = temp_image[x + 2][y + 2];
            output_image[x][y][2] = temp_image[x + 2][y + 2];
        }
    }
}


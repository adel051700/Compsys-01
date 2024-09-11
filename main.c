//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp



#include "cbmp.h"

//Function to convert an image to greyscale to save memory
void greyscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
               char temp_image[BMP_WIDTH][BMP_HEIGTH]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            char grey = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3;
            temp_image[x][y] = grey;
        }
    }
}

void black_and_white(char inputImage[BMP_WIDTH][BMP_HEIGTH],
                     unsigned char outputImage[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            int grayscale = 0;
            for (int c = 0; c < BMP_CHANNELS; c++) {
                grayscale += inputImage[x][y];
            }
            grayscale /= BMP_CHANNELS;

            unsigned char bw = (grayscale > 90) ? 255 : 0;
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

//Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
char temp_image[BMP_WIDTH][BMP_HEIGTH];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

//Main function
int main(int argc, char **argv) {
    //argc counts how may arguments are passed
    //argv[0] is a string with the name of the program
    //argv[1] is the first command line argument (input image)
    //argv[2] is the second command line argument (output image)

    //Checking that 2 arguments are passed
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
        exit(1);
    }

    printf("Example program - 02132 - A1\n");

    //Load image from file
    read_bitmap(argv[1], input_image);

    //Run inversion
    greyscale(input_image, temp_image);
    black_and_white(temp_image, output_image);

    //Save image to file
    write_bitmap(output_image, argv[2]);

    printf("Done!\n");
    return 0;
}

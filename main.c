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

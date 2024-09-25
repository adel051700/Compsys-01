//To compile (linux/mac): gcc cbmp.c function.c main.c -o main.out -lm
//To run (linux/mac): ./main.out example.bmp example_inv.bmp
//To compile (win): gcc cbmp.c main.c -o main.exe -lm
//To run (win): main.exe example.bmp example_inv.bmp

#include "cbmp.h"
#include <math.h>
#include "minmax.h"
#include <stdlib.h>
#include <stdio.h>
#include "time.h"
#include "function.h"
cell *head =NULL;

unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char temp_image[BMP_WIDTH+2][BMP_HEIGTH+2];


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

    //Run erosion to remove noise
    while (erode(temp_image, temp_image) == 0) {
        detectCell(temp_image, &head);
    }

    printCell(head);
    printf("Number of cells: %i\n", countCells(head));

    drawDot(output_image, head);


    //Save image to file
    write_bitmap(output_image, argv[2]);

    printf("Done!\n");
    clock_t end = clock();
    double time_spent = (double) (end - begin) / CLOCKS_PER_SEC;
    printf("Time spent: %f seconds", time_spent);
    return 0;
}

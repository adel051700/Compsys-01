#ifndef function_function_H
#define function_function_H

#define BMP_WIDTH 950
#define BMP_HEIGTH 950
#define BMP_CHANNELS 3
extern cell *head;


typedef struct cell {
    int x;              
    int y;              
    struct cell *next;  
} cell;

// Function prototypes
void greyscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
               unsigned char temp_image[BMP_WIDTH+2][BMP_HEIGTH+2]);

int countCells(cell *head);
void printCell(cell *head);
int cellExists(cell *head, int x, int y);
void erosionToTemp(unsigned char inputImage[BMP_WIDTH][BMP_HEIGTH],
                   unsigned char outputImage[BMP_WIDTH][BMP_HEIGTH], int threshold);

void black_white(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2], int threshold); 
void create_gaussian_kernel(double kernel[][5], int kernel_size, double sigma);

void gaussian_filter(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2],
                     unsigned char outputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2]);
int otsu_threshold(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2]);
int erode(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2],
          unsigned char outputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2]);
void detectCell(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2], cell **head);
void drawDot(unsigned char inputImage[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], cell *head);
void blackBorder(unsigned char inputImage[BMP_WIDTH + 2][BMP_HEIGTH + 2]);

#endif
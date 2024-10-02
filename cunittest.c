// to compile on my mac gcc -o cunittest.out cunittest.c cbmp.c function.c -I/opt/homebrew/include/CUnit -L/opt/homebrew/lib -lcunit -lm
 // to run ./cunittest.out
#include <CUnit.h>
#include <Basic.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "cbmp.h"
#include "function.h"







// Test prototypes
void test_countCells(void);
void test_cellExists(void);
void test_greyscale(void);
void test_detectCell(void);

// Test case for countCells
void test_countCells(void) {
    cell *head = NULL;

    // we create test cells
    cell cell1 = {0, 0, NULL};
    cell cell2 = {1, 1, &cell1};
    cell cell3 = {2, 2, &cell2};

    head = &cell3;
    // check here 
    CU_ASSERT_EQUAL(countCells(head), 3);
    CU_ASSERT_EQUAL(countCells(NULL), 0);


}

// Test case for cellExists
void test_cellExists(void) {
    cell cell1 = {0, 0, NULL};
    cell cell2 = {1, 1, &cell1};
    cell cell3 = {2, 2, &cell2};

    cell *head = &cell3;

    CU_ASSERT_TRUE(cellExists(head, 1, 1));  
    CU_ASSERT_FALSE(cellExists(head, 3, 3));  // Cell do not exist
    CU_ASSERT_FALSE(cellExists(NULL, 1, 1)); // test if the function correctly handles the case where the linked list is null
}


// Test case for greyscale
void test_greyscale(void) {
 unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char temp_image[BMP_WIDTH+2][BMP_HEIGTH+2];

    // Initialize the input image with some values
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            input_image[x][y][0] = 100; 
            input_image[x][y][1] = 150;
            input_image[x][y][2] = 200; 
        }
    }

    greyscale(input_image, temp_image);

 
    CU_ASSERT_EQUAL(temp_image[2][2], (100 + 150 + 200) / 3);  // First pixel
    CU_ASSERT_EQUAL(temp_image[BMP_WIDTH+1][BMP_WIDTH+1], (100 + 150 + 200) / 3);  // Last pixel

    input_image[10][10][0] = 0;  
    input_image[10][10][1] = 0;   
    input_image[10][10][2] = 0;   
    greyscale(input_image, temp_image);
    CU_ASSERT_EQUAL(temp_image[10+2][10+2], 0); 


    input_image[10][10][0] = 255;  
    input_image[10][10][1] = 255;   
    input_image[10][10][2] = 255;   
    greyscale(input_image, temp_image);
    CU_ASSERT_EQUAL(temp_image[10+2][10+2], 255);  

}

// Test case for detectCell
void test_detectCell(void) {
    // Create a test binary image (255 for cell, 0 for background)
   unsigned char test_image[BMP_WIDTH + 2][BMP_HEIGTH + 2] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 255, 255, 0, 0, 0, 0, 0, 0, 0},
        {0, 255, 255, 0, 255, 255, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 255, 255, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    cell *head = NULL; 
    detectCell(test_image, &head); 

   CU_ASSERT_TRUE(cellExists(head, 2, 1)); 
    CU_ASSERT_TRUE(cellExists(head, 2, 2)); 
    CU_ASSERT_TRUE(cellExists(head, 4, 3)); 
    CU_ASSERT_FALSE(cellExists(head, 1, 0)); 
    CU_ASSERT_FALSE(cellExists(head, 0, 0)); 
    CU_ASSERT_FALSE(cellExists(head, 5, 5)); 

    while (head) {
        cell *temp = head;
        head = head->next; 
        free(temp); 
    }
}


int main() {
    // this code is from a website
    // Initialize CUnit test registry
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    CU_pSuite pSuite = CU_add_suite("CellDetectionTestSuite", 0, 0);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if ((NULL == CU_add_test(pSuite, "test of countCells()", test_countCells)) ||
        (NULL == CU_add_test(pSuite, "test of cellExists()", test_cellExists)) ||
        (NULL == CU_add_test(pSuite, "test of greyscale()", test_greyscale))||
        (NULL == CU_add_test(pSuite, "test of detectCell()", test_detectCell))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    CU_cleanup_registry();
    return CU_get_error();
}

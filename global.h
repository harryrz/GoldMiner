//This is a global file for global variables and data structures
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


int pixel_buffer_start; // global variable

struct Crystal{
    int shapeType; // Type of the rock, denoted by an integet number: Diamond, Rock, Sapphire etc.
    short int colour; // Colour of the crystal
    int price; // Price of the crystal
    int* x_loc_list; // Pointer to the start drawing location of the crystal
    int* y_loc_list; // Pointer to the end drawing location of the crystal
    int pixel_size; // How many pixels do we want to draw for this crystal
};

struct hook{
    int angle;
    int length;
    double slope;
    int hooktipX;
    int hooktipY;
    int hooktipXPrev;
    int hooktipYPrev;
};

short int colourList [10] = {0xa5b6, 0x9fb3, 0xc21e, 0x5d9d, 0x0540, 0xf925, 0xf3bd, 0x213d, 0xfda5, 0xffbe};
// In order: Rock, Periodot, Amethyst, Aqua, Emerald, Ruby, Rose, Sapphire, Gold, Diamond

int shapeList [10] = {1, 2, 3, 4, 5, 1, 2, 3, 4, 5};

bool reset = false;

int x_size = 320;
int y_size = 240;
int black = 0x0000;


short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

bool pushbutton = false;
bool roundcomplete = false;





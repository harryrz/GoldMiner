#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

// global variables
int x_size = 320;
int y_size = 240;
int black = 0x0000;

void draw_line_with_angle(int x, int y, double angle, int length);

int main(void){
   draw_line_with_angle(30, 30, 90, 10);
}


void draw_line_with_angle(int x, int y, double angle, int length){ //takes in angle in radians
    short int gold = 0xdee5;
    int slope, x_diff, y_diff;
    double rad_angle = (M_PI/180.0)*angle; //convert angle to radians
    int x_final, y_final;
    
	y_diff = sin(rad_angle)*length;
	x_diff = cos(rad_angle)*length;
	x_final = x + x_diff;
	y_final = y + y_diff;
	draw_line(x, y, x_final, y_final, gold);
}
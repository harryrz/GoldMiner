#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int x_size = 320;
int y_size = 240;
int black = 0x0000;


short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];


void wait_for_v_sync();
void swap(int* x, int* y);
void plot_pixel(int x, int y, short int line_color);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void clear_screen();
void draw_box(int x0, int y0, short int box_color, int side_length);
void draw_line_with_angle(int x, int y, double angle, int length);
double draw_hook(); //return slope of hook so harry can use

// int main(void){
//     volatile int * pixel_ctrl_ptr = (int *)0xFF203020;   // pointer to the base register of controller

//     /* The default base of the frame buffer is int VGA controller */
//    /* That default address is stored in the base register of the VGA controller port/interface*/
//    /* so the next statement gets it, leaving pixel_buffer_start = 0x08000000 */

//     pixel_buffer_start = *pixel_ctrl_ptr;

//     clear_screen();
//     draw_box(150, 150, 0x07E0, 20);
//     return 0;
// }

void draw_line_with_angle(int x, int y, double angle, int length){

}

double draw_hook(){

}

void draw_box(int x0, int y0, short int box_color, int side_length){
    int x1 = x0 - side_length/2, y1 = y0 + side_length/2; //left bottom corner
    int x2 = x0 + side_length/2, y2 = y0 + side_length/2; //right bottom corner
    int x3 = x0 - side_length/2, y3 = y0 - side_length/2; //left top corner
    int x4 = x0 + side_length/2, y4 = y0 - side_length/2; //right top corner
    draw_line(x1, y1, x2, y2, box_color);
    draw_line(x2, y2, x4, y4, box_color);
    draw_line(x4, y4, x3, y3, box_color);
    draw_line(x3, y3, x1, y1, box_color);
    int xdiff = x2-x1, ydiff = y1-y3;
    for(int i=0; i<xdiff; i++){
        for(int j=0; j<ydiff; j++){
            plot_pixel(x3+i, y3+j, box_color);
        }
    }
}

void clear_screen(){
	for(int s = 0; s < x_size; s++){
		for(int t = 0; t < y_size; t++){
			plot_pixel(s, t, 0x0000);
		}
	}
}

void plot_pixel(int x, int y, short int line_color)
{
	volatile short int *one_pixel_address;
	one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);
	*one_pixel_address = line_color;
}

void draw_line(int x0, int y0, int x1, int y1, short int line_color){
	bool is_steep = abs(y1-y0) > abs(x1-x0);
	if(is_steep){
		swap(&x0, &y0);
		swap(&x1, &y1);
	}
	if(x0 > x1){
		swap(&x0, &x1);
		swap(&y0, &y1);
	}
	
	int deltax = x1 - x0;
	int deltay = abs(y1 - y0);
	int error = -(deltax / 2);
	int y = y0;
	int y_step;
	if(y0 < y1){
		y_step = 1;
	} else {
		y_step = -1;
	}
	
	for(int i = x0; i <= x1; i++){
		if(is_steep){
			plot_pixel(y, i, line_color);
		} else {
			plot_pixel(i, y, line_color);
		}
		error = error + deltay;
		if(error > 0){
			y = y + y_step;
			error = error - deltax;
		}
	}
}

void wait_for_vsync(){
	volatile int* pixel_ctrl_ptr = (int *) 0xff203020;
	int status;
	
	if((*pixel_ctrl_ptr) != *(pixel_ctrl_ptr+1)){
		*(pixel_ctrl_ptr+1) = *pixel_ctrl_ptr;
	} // check to make sure that the front and back buffer addresses are equal
	
	*pixel_ctrl_ptr = 1; //starts the synchronization, wait for s bit to turn 0
	
	status = *(pixel_ctrl_ptr + 3);
	
	while((status & 0b1) != 0){ //still waiting for vsync
		status = *(pixel_ctrl_ptr+3);
	}
	//exits loop when s = 0;
	
}

void swap(int* x, int* y){
	int temp = *x;
	*x = *y;
	*y = temp;
}
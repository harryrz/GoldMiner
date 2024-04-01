#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "global.h"
#include "vga_display.c"

//Define some variables:
#define RLEDs ((volatile long *) 0xFF200000)
#define numCrystals 15 // Defines how many crystals we want to render on the picture.

//function declarations
bool detect_hook_on_object(int hookx, int hooky);
void set_crystals( struct Crystal ** crystalList);
void draw_crystals( struct Crystal ** crystalList);

void wait_for_v_sync();
void swap(int* x, int* y);
void plot_pixel(int x, int y, short int line_color);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void clear_screen();
void draw_box(int x0, int y0, short int box_color, int side_length);
void draw_line_with_angle(int x, int y, double angle, int length);
double draw_hook(); //return slope of hook so harry can use


int main(void) {

    struct Crystal ** crystalList;
    crystalList = (struct Crystal **)malloc(numCrystals * sizeof(struct Crystal *));
    for(int i = 0; i < numCrystals; i++){
        crystalList[i] = (struct Crystal *)malloc(sizeof(struct Crystal));
    }


    set_crystals(crystalList); // This populates the crystalList array with random parameters
    
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;   // pointer to the base register of controller
    pixel_buffer_start = *pixel_ctrl_ptr;

    clear_screen();
    draw_box(150, 150, 0x07E0, 20);
    return 0;

	// unsigned char byte1 = 0;
	// unsigned char byte2 = 0;
	// unsigned char byte3 = 0;
	
  	// volatile int * PS2_ptr = (int *) 0xFF200100;  // PS2 port address

	// int PS2_data, RVALID;

	// while (1) {
	// 	PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
	// 	RVALID = (PS2_data & 0x8000);	// extract the RVALID field
	// 	if (RVALID != 0)
	// 	{
	// 		/* always save the last three bytes received */
	// 		byte1 = byte2;
	// 		byte2 = byte3;
	// 		byte3 = PS2_data & 0xFF;
	// 	}
	// 	if ( (byte2 == 0xAA) && (byte3 == 0x00) )
	// 	{
	// 		// mouse inserted; initialize sending of data
	// 		*(PS2_ptr) = 0xF4;
	// 	}
	// 	// Display last byte on Red LEDs
	// 	*RLEDs = byte3;
	// }
}

bool detect_hook_on_object(int hookx, int hooky){
    volatile short int * check_pixel_address;
    check_pixel_address = pixel_buffer_start + (hooky << 10) + (hookx << 1);
    // We now check the pixel of the "soon to be drawn if not detected" hook head.

    // Algorithm: We check if the imaginary hook head is black or not,
    // if it is black, meaning at that position, there is not crystal, thus we keep drawing the hook
    // if it is not black, meaning at that position, there is a crystal, we starat invoking the retrive_hook_rock function.

    // We check the pixel of hook
    // if(0 < slope && slope < 1){
    //     int newhookx = hookx + 1; // Move x right by 1
    //     int newhooky = (int)round(hooky + slope); // Move y down accordingly
    //     check_pixel_address = pixel_buffer_start + (newhooky << 10) + (newhookx << 1);
    // }
    // else if(slope >= 1){
    //     int newhooky = hooky + 1; // move y down by 1
    //     int newhookx = (int)round(hookx + slope); // move x right accordingly
    //     check_pixel_address = pixel_buffer_start + (newhooky << 10) + (newhookx << 1);
    // } 
    // else if( slope < 0 && slope > -1){
    //     int newhookx =  hookx - 1; // Move x left by 1
    //     int newhooky = (int)round(hooky - slope); // move y down accordingly
    //     check_pixel_address = pixel_buffer_start + (newhooky << 10) + (newhookx << 1);
    // }
    // else{
    //     int newhooky = hooky + 1;
    //     int newhookx = (int)round(hookx + slope); // move x left accordingly
    //     check_pixel_address = pixel_buffer_start + (newhooky << 10) + (newhookx << 1);
    // }


    if(*check_pixel_address != 0x0000){ // The "Next" pixel of the current pixel is not black, indicating
                                        // that a rock/gold has been reached.
        return true;
    } else {
        return false;
    }
}

void set_crystals( struct Crystal ** crystalList){
    for(int i = 0; i < numCrystals; i++){
        int crystal_index = rand()%10;
        crystalList[i]->colour = colourList[crystal_index];
        //crystalList[i]->start_x_loc = 20 + rand()%260; //Leave some padding for x
        //crystalList[i]->start_y_loc = 20 + rand()%180; //Leave some padding for y
        crystalList[i]->shapeType = shapeList[crystal_index];
        crystalList[i]->price = crystal_index * 100 + (int)colourList[crystal_index];
    }
}

void draw_crystals( struct Crystal ** crystalList){
    for(int i = 0; i < numCrystals; i++){
        int x_start_loc = 20 + rand()%260;
        int y_start_loc = 40 + rand()%180;
        if(crystalList[i]->shapeType == 1){ // Square Type
            crystalList[i]->pixel_size = 36;
            crystalList[i]->x_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            crystalList[i]->y_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            int square_x_inc [36] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5};
            int square_y_inc [36] = {0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5};
            for(int j = 0; j < crystalList[i]->pixel_size; j++){
                crystalList[i]->x_loc_list[j] = x_start_loc + square_x_inc[j];
                crystalList[i]->y_loc_list[j] = y_start_loc + square_y_inc[j];
                printf("Current x location: \n");
                printf("%d", crystalList[i]->x_loc_list[j]);
                printf("\n");
                printf("Current y location: \n");
                printf("%d", crystalList[i]->y_loc_list[j]);
                printf("\n");
                //printf("%d", crystalList[i]->x_loc_list[j], "\n");
                //plot_pixel(crystalList[i]->x_loc_list[j], crystalList[i]->y_loc_list[j], crystalList[i]->colour);
            }
        }
        else if(crystalList[i]->shapeType == 2){ // Rectangle Type
            crystalList[i]->pixel_size = 30;
            crystalList[i]->x_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            crystalList[i]->y_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            int rectangle_x_inc [30] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4};
            int rectangle_y_inc [30] = {0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5};
            for(int j = 0; j < crystalList[i]->pixel_size; j++){
                crystalList[i]->x_loc_list[j] = x_start_loc + rectangle_x_inc[j];
                crystalList[i]->y_loc_list[j] = y_start_loc + rectangle_y_inc[j];
                printf("currently storing location x: ", crystalList[i]->x_loc_list[j], " , y: ", crystalList[i]->y_loc_list[j], ".\n");
                //plot_pixel(crystalList[i]->x_loc_list[j], crystalList[i]->y_loc_list[j], crystalList[i]->colour);
            }
        } else if(crystalList[i]->shapeType == 3){ // Triangle Type:
            crystalList[i]->pixel_size = 50;
            crystalList[i]->x_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            crystalList[i]->y_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            int triangle_x_inc [50] = {0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
            10,11,12,13,14,15,16,17,18,19};
            int triangle_y_inc [50] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
            for(int j = 0; j < crystalList[i]->pixel_size; j++){
                crystalList[i]->x_loc_list[j] = x_start_loc + triangle_x_inc[j];
                crystalList[i]->y_loc_list[j] = y_start_loc + triangle_y_inc[j];
                printf("currently storing location x: ", crystalList[i]->x_loc_list[j], " , y: ", crystalList[i]->y_loc_list[j], ".\n");
                //plot_pixel(crystalList[i]->x_loc_list[j], crystalList[i]->y_loc_list[j], crystalList[i]->colour);
            }

        } else if(crystalList[i]->shapeType == 4){ //spike type
            crystalList[i]->pixel_size = 60;
            crystalList[i]->x_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            crystalList[i]->y_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            int spike_x_inc [60] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,-13,-14,-15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0};
            int spike_y_inc [60] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,
            11,12,13,14,15,-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,-13,-14,-15};
            for(int j = 0; j < crystalList[i]->pixel_size; j++){
                crystalList[i]->x_loc_list[j] = x_start_loc + spike_x_inc[j];
                crystalList[i]->y_loc_list[j] = y_start_loc + spike_y_inc[j];
                printf("currently storing location x: ", crystalList[i]->x_loc_list[j], " , y: ", crystalList[i]->y_loc_list[j], ".\n");
                //plot_pixel(crystalList[i]->x_loc_list[j], crystalList[i]->y_loc_list[j], crystalList[i]->colour);
            }
        } else { //diamond
            crystalList[i]->pixel_size = 19;
            crystalList[i]->x_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            crystalList[i]->y_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            int diamond_x_inc [19] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            int diamond_y_inc [19] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 4, 3, 2, 1};
            for(int j = 0; j < crystalList[i]->pixel_size; j++){
                crystalList[i]->x_loc_list[j] = x_start_loc + diamond_x_inc[j];
                crystalList[i]->y_loc_list[j] = y_start_loc + diamond_y_inc[j];
                printf("currently storing location x: ", crystalList[i]->x_loc_list[j], " , y: ", crystalList[i]->y_loc_list[j], ".\n");
                //plot_pixel(crystalList[i]->x_loc_list[j], crystalList[i]->y_loc_list[j], crystalList[i]->colour);
            }
        }
    }
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
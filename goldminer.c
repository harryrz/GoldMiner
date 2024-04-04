#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "global.h"
#include "vga_display.c"

//Define some variables:
#define RLEDs ((volatile long *) 0xFF200000)
#define numCrystals 15 // Defines how many crystals we want to render on the picture.
#define M_PI 3.14159265358979323846

// global variables
struct hook hookInfo; //initialize hook
int x_size = 320;
int y_size = 240;
int black = 0x0000;
bool swingCW = true; //if the hook swings counter clockwise or not
bool pbClicked = false; //if user clicks pushbutton, this bool will be set to true
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];


//function declarations
bool detect_hook_on_object(int hookx, int hooky, double slope);
void set_crystals( struct Crystal ** crystalList);
void draw_crystals( struct Crystal ** crystalList);

void wait_for_vsync();
void swap(int* x, int* y);
void plot_pixel(int x, int y, short int line_color);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void clear_screen();
void draw_box(int x0, int y0, short int box_color, int side_length);
void draw_line_with_angle(int x, int y, double angle, int length, bool clear);
void draw_hook(int* pushButtonBase); //return slope of hook so harry can use
void extend_hook(); 
void retract_hook();

int main(void) {

    struct Crystal ** crystalList;
    crystalList = (struct Crystal **)malloc(numCrystals * sizeof(struct Crystal *));
    for(int i = 0; i < numCrystals; i++){
        crystalList[i] = (struct Crystal *)malloc(sizeof(struct Crystal));
    }


    set_crystals(crystalList); // This populates the crystalList array with random parameters
    
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;   // pointer to the base register of controller
    
    *(pixel_ctrl_ptr + 1) = (int) &Buffer1; // first store the address in the back buffer
	/* now, swap the front/back buffers, to set the front buffer location */
	wait_for_vsync(); // We do this becasue we cannot directly write to the front buffer
	pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    *(pixel_ctrl_ptr + 1) = (int) &Buffer2;
	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
	clear_screen(); // pixel_buffer_start points to the pixel buffer

    volatile int * pushButtonBase = (int *)0xFF200050; // pointer to base register of pushbuttons


    while(1){ 
        if(*(pushButtonBase+3) == 1){ //some button is pushed, set pbClicked to true;
            pbClicked = true;
        }

        if(pbClicked == false){ //hook keeps swinging if user not clicking pushbutton
            draw_hook(pushButtonBase);
        }else{ //push button clicked, extend hook
            extend_hook();
            *(pushButtonBase+3) = 1; // reset edgecap bit
            pbClicked = false; //reset pbClicked
        }


        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
    
    return 0;
}

bool detect_hook_on_object(int hookx, int hooky, double slope){
    volatile short int * check_pixel_address;
    check_pixel_address = pixel_buffer_start + (hooky << 10) + (hookx << 1);
    //double slope = hookInfo.slope;
    // We now check the pixel of the "soon to be drawn if not detected" hook head.

    // Algorithm: We check if the imaginary hook head is black or not,
    // if it is black, meaning at that position, there is not crystal, thus we keep drawing the hook
    // if it is not black, meaning at that position, there is a crystal, we starat invoking the retrive_hook_rock function.

    // We check the pixel of hook
    if(hookx == 0){
        reset = true;
        return false;
    } else if(hookx == 320){
        reset = true;
        return false;
    } else if(hooky == 240){
        reset = true;
        return false;
    } else{
        if(0 < slope && slope < 1){
        int newhookx = hookx + 1; // Move x right by 1
        int newhooky = (int)round(hooky + slope); // Move y down accordingly
        check_pixel_address = pixel_buffer_start + (newhooky << 10) + (newhookx << 1);
        }
        else if(slope >= 1){
            int newhooky = hooky + 1; // move y down by 1
            int newhookx = (int)round(hookx + slope); // move x right accordingly
            check_pixel_address = pixel_buffer_start + (newhooky << 10) + (newhookx << 1);
        } 
        else if( slope < 0 && slope > -1){
            int newhookx =  hookx - 1; // Move x left by 1
            int newhooky = (int)round(hooky - slope); // move y down accordingly
            check_pixel_address = pixel_buffer_start + (newhooky << 10) + (newhookx << 1);
        }
        else{
            int newhooky = hooky + 1;
            int newhookx = (int)round(hookx + slope); // move x left accordingly
            check_pixel_address = pixel_buffer_start + (newhooky << 10) + (newhookx << 1);
        }
        if(*check_pixel_address != 0x0000){ // The "Next" pixel of the current pixel is not black, indicating
                                            // that a rock/gold has been reached.
            return true;
        } else {
            return false;
        }
    }
}

void set_crystals( struct Crystal ** crystalList){
    for(int i = 0; i < numCrystals; i++){
        int crystal_index = rand()%10;
        crystalList[i]->colour = colourList[crystal_index];
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
                plot_pixel(crystalList[i]->x_loc_list[j], crystalList[i]->y_loc_list[j], crystalList[i]->colour);
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
                plot_pixel(crystalList[i]->x_loc_list[j], crystalList[i]->y_loc_list[j], crystalList[i]->colour);
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
                plot_pixel(crystalList[i]->x_loc_list[j], crystalList[i]->y_loc_list[j], crystalList[i]->colour);
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
                plot_pixel(crystalList[i]->x_loc_list[j], crystalList[i]->y_loc_list[j], crystalList[i]->colour);
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
                plot_pixel(crystalList[i]->x_loc_list[j], crystalList[i]->y_loc_list[j], crystalList[i]->colour);
            }
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

void draw_line_with_angle(int x, int y, double angle, int length, bool clear){ //takes in angle in radians
    short int colour;
    if(clear == true){
        colour = 0x000000;
    }else{
        colour = 0xdee5;
    }
    int slope, x_diff, y_diff;
    double rad_angle = (M_PI/180.0)*angle; //convert angle to radians
    int x_final, y_final;
    
	y_diff = sin(rad_angle)*length;
	x_diff = cos(rad_angle)*length;
	x_final = x + x_diff;
	y_final = y + y_diff;
    hookInfo.hooktipX = x_final;
    hookInfo.hooktipY = y_final;
    hookInfo.slope = y_diff/x_diff;
	draw_line(x, y, x_final, y_final, colour);
}

void draw_hook(int* pushButtonBase){
    draw_box(160, 20, 0x808080, 16); //draw box in grey colour, acts as hook base
    hookInfo.length = 35;
    //hook swings clockwise
    if(swingCW == true){
        for(int i = hookInfo.angle; i <= 180; i++){
			if(*(pushButtonBase+3)==1){
				return; // end executing function if pushbutton pushed
			}
			if(i != 0){
                draw_line_with_angle(160, 28, i-1, 35, true); //clear previous hook
            }
			draw_line_with_angle(160, 28, i, 35, false); //draw current hook
            hookInfo.angle = i;
            wait_for_vsync();
        }
        swingCW = false;
    }else{ //hook swings counter clockwise
        for(int i = hookInfo.angle; i >= 0; i--){
			if(*(pushButtonBase+3)==1){
				return; //end executing function if pushbutton pushed
			}
			if(i != 180){
                draw_line_with_angle(160, 28, i+1, 35, true); //clear previous hook
            }
			draw_line_with_angle(160, 28, i, 35, false); //draw current hook
            hookInfo.angle = i;
            wait_for_vsync();
        }
        swingCW = true;
    }
}

void extend_hook(){
    hookInfo.length = 35; // In case length of hook is modified elsewhere, change it back to 35
    int count = 0;
    while(!detect_hook_on_object(hookInfo.hooktipX, hookInfo.hooktipY, hookInfo.slope)){ // no crystals detected at tip of hook
        draw_line_with_angle(160, 28, hookInfo.angle, hookInfo.length-1, true); //clear previous extended hook
        draw_line_with_angle(160, 28, hookInfo.angle, hookInfo.length, false); //draw extended hook
        hookInfo.length = hookInfo.length + 1;
        count++;
        if(hookInfo.hooktipX>=319 || hookInfo.hooktipX<=0 || hookInfo.hooktipY<=0 ||hookInfo.hooktipY>=239){ //if reaches end of screen bounds, retract
            retract_hook(hookInfo);
            return;
        }
        wait_for_vsync();
    }
    retract_hook(); //if reaches this point, it means crystal detected, retract hook
}

void retract_hook(){
    int count = 0;
    while(hookInfo.length>=35){ //retract until hook's original length
        if(count!=0){
            draw_line_with_angle(160, 28, hookInfo.angle, hookInfo.length+1, true); //clear previous retracted hook
        }
        draw_line_with_angle(160, 28, hookInfo.angle, hookInfo.length, false); //draw retracted hook
        hookInfo.length = hookInfo.length - 1;
        count++;
        wait_for_vsync();
    }
	draw_line_with_angle(160, 28, hookInfo.angle, 35, true); //clear
	hookInfo.length = 35; //reset hook length
}
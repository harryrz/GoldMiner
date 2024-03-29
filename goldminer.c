#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "global.h"
#include "vga_display.c"

//Define some variables:
#define RLEDs ((volatile long *) 0xFF200000)
#define numCrystals 15 // Defines how many crystals we want to render on the picture.





int main() {

    struct Crystal ** crystalList;
    crystalList = (struct Crystal **)malloc(numCrystals * sizeof(struct Crystal *));
    for(int i = 0; i < numCrystals; i++){
        crystalList[i] = (struct Crystal *)malloc(sizeof(struct Crystal));
    }



    set_crystals(crystalList); // This populates the crystalList array with random parameters
    






	unsigned char byte1 = 0;
	unsigned char byte2 = 0;
	unsigned char byte3 = 0;
	
  	volatile int * PS2_ptr = (int *) 0xFF200100;  // PS2 port address

	int PS2_data, RVALID;

	while (1) {
		PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
		RVALID = (PS2_data & 0x8000);	// extract the RVALID field
		if (RVALID != 0)
		{
			/* always save the last three bytes received */
			byte1 = byte2;
			byte2 = byte3;
			byte3 = PS2_data & 0xFF;
		}
		if ( (byte2 == 0xAA) && (byte3 == 0x00) )
		{
			// mouse inserted; initialize sending of data
			*(PS2_ptr) = 0xF4;
		}
		// Display last byte on Red LEDs
		*RLEDs = byte3;
	}
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
        int crystal_index = colourList[rand()%10];
        crystalList[i]->colour = colourList[crystal_index];
        //crystalList[i]->start_x_loc = 20 + rand()%260; //Leave some padding for x
        //crystalList[i]->start_y_loc = 20 + rand()%180; //Leave some padding for y
        crystalList[i]->shapeType = shapeList[crystal_index];
        crystalList[i]->price = crystal_index * 100 + (int)colourList[crystal_index];
    }
}

void draw_crystals( struct Crystal ** crystalList){
    for(int i = 0; i < numCrystals; i++){
        if(crystalList[i]->shapeType == 1){ // Square Type
            crystalList[i]->pixel_size = 16;
            crystalList[i]->x_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            crystalList[i]->y_loc_list = (int *)malloc(crystalList[i]->pixel_size * sizeof(int));
            crystalList[i]->x_loc_list[0] = 20 + rand()%260;
            crystalList[i]->y_loc_list[0] = 20 + rand()%180;

            

        }
    }
}
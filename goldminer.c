#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "global.h"

//Define some variables:
#define RLEDs ((volatile long *) 0xFF200000)





int main() {
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

bool detect_hook_on_object(int hookx, int hooky, double slope){
    volatile short int * check_pixel_address;
    
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



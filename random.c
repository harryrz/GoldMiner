#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void plot_pixel(int x, int y, short int line_color);

void clear_screen();

void draw_line(int x0, int y0, int x1, int y1, short int line_color);

void swap(int *p, int *q);

void wait_for_vsync();

void draw_box(int x0, int y0, short int color);

void update();

volatile int pixel_buffer_start;  // global variable
short int Buffer1[240][512];      // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];
short int colour[10] = {0xffff, 0xf800, 0x07e0, 0x001f, 0xff30,
                        0xf81f, 0x07ff, 0xfbe0, 0x79e0, 0xbdf7};

#define N 12
// initialize location and direction of rectangles(not shown)
// the location of each box
int x_box[N], y_box[N];
// the colour of each box
short int colour_box[N];
// the direction each box moves in the change, in each frame of the position
int dx[N], dy[N];
int x_prev[N], y_prev[N];
int x_prev2[N], y_prev2[N];
int main(void) {
  volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
  // declare other variables(not shown)

  for (int i = 0; i < N; i++) {
    x_box[i] = rand() % 240 + 40;
    y_box[i] = rand() % 180 + 30;
    colour_box[i] = colour[rand() % 10];
    dx[i] = (rand() % 2 == 0) ? 1 : -1;  // initial moving direction
    dy[i] = (rand() % 2 == 0) ? 1 : -1;
  }

  /* set front pixel buffer to Buffer 1 */
  *(pixel_ctrl_ptr + 1) =
      (int)&Buffer1;  // first store the address in the  back buffer
  /* now, swap the front/back buffers, to set the front buffer location */
  *pixel_ctrl_ptr = 1;
  wait_for_vsync();
  /* initialize a pointer to the pixel buffer, used by drawing functions */
  pixel_buffer_start = *pixel_ctrl_ptr;
  clear_screen();  // pixel_buffer_start points to the pixel buffer
  /* set back pixel buffer to Buffer 2 */
  *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
  *pixel_ctrl_ptr = 1;
  pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // we draw on the back buffer
  clear_screen();  // pixel_buffer_start points to the pixel buffer

  while (1) {
    /* Erase any boxes and lines that were drawn in the last iteration */
    for (int i = 0; i < N; i++) {
      draw_box(x_prev2[i], y_prev2[i], 0x0000);
      draw_line(x_prev2[i], y_prev2[i], x_prev2[(i + 1) % N],
                y_prev2[(i + 1) % N], 0x0000);
    }
    // code for drawing the boxes and lines (not shown)
    for (int i = 0; i < N; i++) {
      draw_box(x_box[i], y_box[i], colour_box[i]);
      draw_line(x_box[i], y_box[i], x_box[(i + 1) % N], y_box[(i + 1) % N],
                colour_box[i]);
      x_prev2[i] = x_prev[i];
      y_prev2[i] = y_prev[i];
      x_prev[i] = x_box[i];
      y_prev[i] = y_box[i];
    }
    // code for updating the locations of boxes (not shown)
    update();
    *pixel_ctrl_ptr = 1;
    wait_for_vsync();  // swap front and back buffers on VGA vertical sync
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer
  }
}

// code for subroutines (not shown)

void plot_pixel(int x, int y, short int line_color) {
  volatile short int *one_pixel_address;

  one_pixel_address =
      (short int *)((int)pixel_buffer_start + (y << 10) + (x << 1));

  *one_pixel_address = line_color;
}

void clear_screen() {
  for (int x = 0; x < 320; x++) {
    for (int y = 0; y < 240; y++) {
      plot_pixel(x, y, 0x0000);
    }
  }
}

void draw_line(int x0, int y0, int x1, int y1, short int line_color) {
  bool is_steep = abs(y1 - y0) > abs(x1 - x0);
  if (is_steep) {
    swap(&x0, &y0);
    swap(&x1, &y1);
  }
  if (x0 > x1) {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }
  int deltax = x1 - x0;
  int deltay = abs(y1 - y0);
  int error = -(deltax / 2);
  int y = y0;
  int y_step = (y0 < y1) ? 1 : -1;
  for (int x = x0; x <= x1; x++) {
    if (is_steep) {
      plot_pixel(y, x, line_color);
    } else {
      plot_pixel(x, y, line_color);
    }
    error = error + deltay;
    if (error > 0) {
      y = y + y_step;
      error = error - deltax;
    }
  }
}

void swap(int *p, int *q) {
  int tmp = *p;
  *p = *q;
  *q = tmp;
}

void wait_for_vsync() {
  volatile int *status = (int *)0xFF20302C;
  while ((*status & 1) != 0)
    ;
}

void draw_box(int x0, int y0, short int color) {
  draw_line(x0, y0, x0 + 1, y0, color);
  draw_line(x0, y0 + 1, x0 + 1, y0 + 1, color);
}

void update() {
  for (int i = 0; i < N; i++) {
    if (x_box[i] == 0 || x_box[i] + 1 == 319) {
      dx[i] *= -1;
    }
    if (y_box[i] == 0 || y_box[i] + 1 == 239) {
      dy[i] *= -1;
    }
    x_box[i] += dx[i];
    y_box[i] += dy[i];
  }
}
#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lcd.h"
#include "pac.h"

static const char *TAG = "lab01";

#define delayMS(ms) \
	vTaskDelay(((ms)+(portTICK_PERIOD_MS-1))/portTICK_PERIOD_MS)

// Car constants
#define CAR_CLR rgb565(220,30,0)
#define WINDOW_CLR rgb565(180,210,238)
#define TIRE_CLR BLACK
#define HUB_CLR GRAY

// Main display constants
#define BACKGROUND_CLR rgb565(0,60,90)
#define TITLE_CLR GREEN
#define STATUS_CLR WHITE
#define STR_BUF_LEN 12 // string buffer length
#define FONT_SIZE 2
#define FONT_W (LCD_CHAR_W*FONT_SIZE)
#define FONT_H (LCD_CHAR_H*FONT_SIZE)
#define STATUS_W (FONT_W*3)

#define WAIT 2000 // milliseconds
#define HALF_WAIT 1000 // milliseconds
#define DELAY_EX3 20 // milliseconds

// Object position and movement
#define OBJ_X 100
#define OBJ_Y 100
#define OBJ_MOVE 3 // pixels


// Car coordinate defs
#define CAR_W 60
#define CAR_H 32
// body of the car
#define BODY_X0 0
#define BODY_Y0 12
#define BODY_X1 59
#define BODY_Y1 24
// top part of the car
#define TOP_X0 1
#define TOP_Y0 0
#define TOP_X1 39
#define TOP_Y1 11
// hood of the car
#define TRI_X0 40
#define TRI_Y0 9
#define TRI_X1 40
#define TRI_Y1 11
#define TRI_X2 59
#define TRI_Y2 11
// windows
#define WINDOW0_X 3
#define WINDOW0_Y 1
#define WINDOW1_X 21
#define WINDOW1_Y 1
#define WINDOW_WIDTH 16
#define WINDOW_HEIGHT 7
#define WINDOW_RADIUS 2
// wheels
#define WHEEL0_X 11
#define WHEEL1_X 48
#define WHEEL_Y 24

#define BIG_WHEEL_RADIUS 7
#define SMALL_WHEEL_RADIUS 4


/**
 * @brief Draw a car at the specified location.
 * @param x      Top left corner X coordinate.
 * @param y      Top left corner Y coordinate.
 * @details Draw the car components relative to the anchor point (top, left).
 */
void drawCar(coord_t x, coord_t y) {
	// base of the car, bottom rectangle
	lcd_fillRect2(x + BODY_X0, y + BODY_Y0, x + BODY_X1, y + BODY_Y1, CAR_CLR);
	// top of the car, top rectangle
	lcd_fillRect2(x + TOP_X0, y + TOP_Y0, x + TOP_X1, y + TOP_Y1, CAR_CLR);
	// hood of the car, triange
	lcd_fillTriangle(x + TRI_X0, y + TRI_Y0, x + TRI_X1, y + TRI_Y1, x + TRI_X2, y + TRI_Y2, CAR_CLR);
	// windows, rounded rectangles
	lcd_fillRoundRect(x + WINDOW0_X, y + WINDOW0_Y, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_RADIUS, WINDOW_CLR);
	lcd_fillRoundRect(x + WINDOW1_X, y + WINDOW1_Y, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_RADIUS, WINDOW_CLR);
	// outer wheels
	lcd_fillCircle(x + WHEEL0_X, y + WHEEL_Y, BIG_WHEEL_RADIUS, TIRE_CLR);
	lcd_fillCircle(x + WHEEL1_X, y + WHEEL_Y, BIG_WHEEL_RADIUS, TIRE_CLR);
	// inner wheels
	lcd_fillCircle(x + WHEEL0_X, y + WHEEL_Y, SMALL_WHEEL_RADIUS, HUB_CLR);
	lcd_fillCircle(x + WHEEL1_X, y + WHEEL_Y, SMALL_WHEEL_RADIUS, HUB_CLR);
}

void eraseCar(coord_t x, coord_t y) {
	lcd_fillRect(x, y, CAR_W, CAR_H, BACKGROUND_CLR);
}

void eraseStatus(coord_t x, coord_t y) {
	lcd_fillRect(x, y, LCD_W, LCD_H, BACKGROUND_CLR);
}


void app_main(void)
{
	// Set up ESP, or something, and init LCD
	ESP_LOGI(TAG, "Start up");
	lcd_init();
	// set font size
	lcd_setFontSize(FONT_SIZE);
	//----------------------------------------------------------------------------//
	// EXERCISE 1 - Draw car in one location.
	//----------------------------------------------------------------------------//
	// * Fill screen with background color
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0, 0, "Exercise 1", TITLE_CLR);
	// * Draw car at OBJ_X, OBJ_Y
	drawCar(OBJ_X, OBJ_Y);
	// * Wait 2 seconds before next stage
	delayMS(WAIT);


	//----------------------------------------------------------------------------//
	// EXERCISE 2 - Draw moving car (Method 1), one pass across display.
	//----------------------------------------------------------------------------//
	// Clear the entire display and redraw all objects each iteration.
	// Use a loop and increment x by OBJ_MOVE each iteration.
	// Start x off screen (negative coordinate).
	// set up buffer to hold the string
	char str_x[4];
	// Move loop:	
	// loop through each x coordinate and draw the car
	for (coord_t x = -CAR_W; x <= LCD_W + CAR_W; x += OBJ_MOVE) {
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString(0, 0, "Exercise 2", TITLE_CLR);
		drawCar(x, OBJ_Y);
		// convert x int into string
		sprintf(str_x, "%3ld", x);
		// * Display the x position of the car at bottom left of screen
		//   with status color
		lcd_drawString(0, LCD_H - FONT_H, str_x, STATUS_CLR);
	}
	// * Wait 1 second before next stage
	delayMS(HALF_WAIT);
	

	//----------------------------------------------------------------------------//
	// EXERCISE 3 - Draw moving car (Method 2), one pass across display.
	//----------------------------------------------------------------------------//
	// Move by erasing car at old position, then redrawing at new position.
	// Objects that don't change or move are drawn once.
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0, 0, "Exercise 3", TITLE_CLR);
	delayMS(WAIT);
	// Move loop:
	for (coord_t x = -CAR_W; x <= LCD_W + CAR_W; x += OBJ_MOVE) {
		drawCar(x, OBJ_Y);
		// convert x int into string
		sprintf(str_x, "%3ld", x);
		// * Display the x position of the car at bottom left of screen
		//   with status color
		lcd_drawString(0, LCD_H - FONT_H, str_x, STATUS_CLR);
		// 20 second delay to see effect
		delayMS(DELAY_EX3);
		// erase the car by drawing over with a blue rectangle
		eraseCar(x, OBJ_Y);
		// erase the x coordinate from the bottom left
		eraseStatus(0, LCD_H - FONT_H);
	}

	// * Wait 1 second before next stage
	delayMS(HALF_WAIT);


	//----------------------------------------------------------------------------//
	// EXERCISE 4 - Exercise 4 - Draw moving car (Method 3), one pass across display.
	//----------------------------------------------------------------------------//
	// First, draw all objects into a cleared, off-screen frame buffer.
	// Then, transfer the entire frame buffer to the screen.
	// Before loop:
	// * Enable the frame buffer
	lcd_frameEnable();
	// Move loop:
	for (coord_t x = -CAR_W; x <= LCD_W + CAR_W; x += OBJ_MOVE)	{
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString(0, 0, "Exercise 4", TITLE_CLR);
		// * Draw car at x, OBJ_Y
		drawCar(x, OBJ_Y);
		// convert x int into string
		sprintf(str_x, "%3ld", x);
		// * Display position of the car at bottom left with status color
		lcd_drawString(0, LCD_H - FONT_H, str_x, STATUS_CLR);
		// * Write the frame buffer to the LCD
		lcd_writeFrame();
	}
	// disable to frame buffer after loop, not sure if necessary?
	lcd_frameDisable();

	// * Wait 1 second before next stage
	delayMS(HALF_WAIT);

	//----------------------------------------------------------------------------//
	// EXERCISE 5 - Draw an animated Pac-Man moving across the display.
	//----------------------------------------------------------------------------//
	// Use Pac-Man sprites instead of the car object.
	// Cycle through each sprite when moving the Pac-Man character.
	// Before loop:
	// * Enable the frame buffer
	lcd_frameEnable();
	// array to loop through to get difference pac man indexes
	const uint8_t pidx[] = {0, 1, 2, 1};
	// infinite loop to keep pacman moving
	while (1) {
		// keeps track of index of pidx we need to be accessing to get pac stage
		// will end up a bigger number than the length of the array so we use remainder
		uint16_t i = 0;
		// Move loop:
		for (coord_t x = -PAC_W; x <= LCD_W; x += OBJ_MOVE) {
			lcd_fillScreen(BACKGROUND_CLR);
			lcd_drawString(0, 0, "Exercise 5", TITLE_CLR);
			// * Draw Pac-Man at x, OBJ_Y with yellow color;
			lcd_drawBitmap(x, OBJ_Y, pac[pidx[i++ % sizeof(pidx)]], PAC_W, PAC_H, YELLOW);		
			// convert i int into string
			sprintf(str_x, "%3ld", x);
			// * Display position at bottom left with status color
			lcd_drawString(0, LCD_H - FONT_H, str_x, STATUS_CLR);
			// * Write the frame buffer to the LCD
			lcd_writeFrame();
			// effect looks better with 20 ms delay, not sure if we're supposed to have it
			delayMS(DELAY_EX3);
		}
	}
	// probably not necessary
	lcd_frameDisable();
}

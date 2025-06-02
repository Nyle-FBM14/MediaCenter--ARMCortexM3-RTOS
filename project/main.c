/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include <stdio.h>
#include <math.h>
#include "KBD.h"
#include "GLCD.h"
#include "LPC17xx.h"

extern unsigned char MARIOSPRITES_pixel_data[];
extern unsigned char ALIEN_pixel_data[];
extern unsigned char ALIEN2_pixel_data[];
extern unsigned char SHIP_pixel_data[];

#define __FI 1 /* Font index 16x24 */
//ITM Stimulus Port definitions for printf //////////////////
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

struct __FILE { int handle;  };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
  if (DEMCR & TRCENA) {
    while (ITM_Port32(0) == 0);
    ITM_Port8(0) = ch;
  }
  return(ch);
}

uint32_t jsVal;
int8_t screenState = 0, position = 1, p = 4; //p = # of images
unsigned char* pics[] = {MARIOSPRITES_pixel_data, ALIEN_pixel_data, ALIEN2_pixel_data, SHIP_pixel_data};
/* SCREEN STATES
	0 - main menu
	1 - gallery list view
	2 - gallery carousel view
	3 - MP3 player
	4 - game menu
	5 - game
*/


void joystick (void const *argument);

osThreadId joystick_id;

osThreadDef(joystick, osPriorityNormal, 1, 0);

//SHOWING SCREENS
void showMainMenu() {
	GLCD_Clear(Black);
	GLCD_SetBackColor(Black);
	GLCD_SetTextColor(Yellow);
	GLCD_DisplayString(0, 0, __FI, "Nyle's Media Center ");
	GLCD_DisplayString(1, 0, __FI, "Gallery");
	GLCD_DisplayString(2, 0, __FI, "MP3");
	GLCD_DisplayString(3, 0, __FI, "Alien Invaders");
	GLCD_DisplayString( position+1 , 16, __FI, "<--");
}
void showGallery() {
	GLCD_DisplayString(0, 0, __FI, "       Gallery      ");
	GLCD_DisplayString(1, 0, __FI, "ship.c              ");
	GLCD_DisplayString(2, 0, __FI, "alien.c             ");
	GLCD_DisplayString(3, 0, __FI, "alien2.c            ");
	GLCD_DisplayString(4, 0, __FI, "mario sprites.c     ");
	GLCD_DisplayString( (position%p)+1 , 16, __FI, "<--");
}
void showMP3() {
	GLCD_Clear(Black);
	GLCD_SetBackColor(Black);
	GLCD_SetTextColor(Yellow);
	GLCD_DisplayString(0, 0, __FI, "         MP3        ");
}
void showGameMenu() {
	GLCD_Clear(Black);
	GLCD_SetBackColor(Black);
	GLCD_SetTextColor(Yellow);
	GLCD_DisplayString(0, 0, __FI, "   Space Invaders   ");
	GLCD_DisplayString(1, 0, __FI, "Defeat all invaders!");
	GLCD_DisplayString(2, 0, __FI, "Use joystick to move");
	GLCD_DisplayString(3, 0, __FI, "Use SELECT to shoot ");
	GLCD_DisplayString(4, 0, __FI, "LEFT to leave game:(");
}
void showGame(){
	GLCD_Clear(Black);
}
//UPDATING SCREENS
void updateMainMenu(){ //puts an arrow at currently selected menu item and clears the other two rows
	position = position%3;
	
	GLCD_DisplayString( position        +1 , 16, __FI, "<--");
	GLCD_DisplayString( ((position+1)%3)+1 , 16, __FI, "   ");
	GLCD_DisplayString( ((position+2)%3)+1 , 16, __FI, "   ");
}
void updateGallery(){
	position = position%p;
	
	GLCD_DisplayString( position        +1 , 16, __FI, "<--");
	GLCD_DisplayString( ((position+1)%p)+1 , 16, __FI, "   ");
	GLCD_DisplayString( ((position+2)%p)+1 , 16, __FI, "   ");
	GLCD_DisplayString( ((position+3)%p)+1 , 16, __FI, "   ");
}
void updateCarousel(){
		GLCD_Clear(Black);
		GLCD_Bitmap(0, 0, 320, 240, pics[position%p]); //show image based on position
}
void updatePlayer() {
	GLCD_Bitmap(0, (position%5) * 48, 60, 48, SHIP_pixel_data); //shows ship based on position
	
	//REPLACE SHIP IMAGE WITH ALL BLACK IMAGE
	GLCD_Bitmap(0, ((position+1)%5) * 48, 60, 48, SHIP_pixel_data);
	GLCD_Bitmap(0, ((position+2)%5) * 48, 60, 48, SHIP_pixel_data);
	GLCD_Bitmap(0, ((position+3)%5) * 48, 60, 48, SHIP_pixel_data);
	GLCD_Bitmap(0, ((position+4)%5) * 48, 60, 48, SHIP_pixel_data);
}
void select(){
	if(screenState == 5){ //in-game
		//shoot
	}
	else if(screenState == 0) { //from main menu
		position = position % 3;
		switch(position){
			case 0:
				screenState = 1;
				position = 0;
				showGallery(); //go to gallery
				break;
			case 1:
				screenState = 3;
				showMP3(); //go to MP3
				break;
			case 2:
				screenState = 4;
				showGameMenu(); //go to game
		} //switch end
	}
	else if(screenState == 1) { //from gallery go to carousel view
		screenState = 2;
		updateCarousel();
	}
	else if(screenState == 3) { //in MP3
		//pause/play?
	}
	else if(screenState == 4) { //from game menu start game
		screenState = 5;
		position = 3;
		updatePlayer();
	}
}
void goBack() {
	switch(screenState) {
		case 1:	//from gallery go to main menu
			screenState = 0;
			position = 0;
			showMainMenu();
			break;
		case 2:	//from carousel view go to list view (gallery)
			screenState = 1;
			showGallery();
			break;
		case 3:	//from MP3 player go to main menu
			screenState = 0;
			position = 1;
			showMainMenu();
			break;
		case 4:	//from game menu go to main menu
			screenState = 0;
			position = 2;
			showMainMenu();
			break;
		case 5:	//from in-game go to game menu
			screenState = 4;
			showGameMenu();
			break;
	} //switch end
}
void joystick (void const *argument) {
	for(;;) {
		jsVal = get_button();
		
		if(jsVal == KBD_DOWN){
			position++;
		}
		else if(jsVal == KBD_UP){
			position--;
		}
		else if(jsVal == KBD_SELECT) {
			select();
			continue;
		}
		else if(jsVal == KBD_LEFT) {
			goBack();
			continue;
		}
		
		switch(screenState){
			case 0:
				updateMainMenu();
				break;
			case 1:
				updateGallery();
				break;
			case 2:
				updateCarousel();
				break;
			case 3:
				//MP3 - nothing rn
				break;
			case 4:
				//game menu - nothing rn
				break;
			case 5:
				updatePlayer();
		} //switch end
	}	//for end
}
/*
 * main: initialize and start the system
 */
int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS

  // initialize peripherals here
	KBD_Init();
	GLCD_Init();
	
	//initialize main menu
	GLCD_Clear(Black);
	GLCD_SetBackColor(Black);
	GLCD_SetTextColor(Yellow);
	GLCD_DisplayString(0, 0, __FI, "Nyle's Media Center ");
	GLCD_DisplayString(1, 0, __FI, "Gallery         <--");
	GLCD_DisplayString(2, 0, __FI, "MP3");
	GLCD_DisplayString(3, 0, __FI, "Alien Invaders");
  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);

  osKernelStart ();                         // start thread execution 
}

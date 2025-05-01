/*
 * ApplicationCode.c
 *
 *  Created on: Dec 30, 2023 (updated 11/12/2024) Thanks Donavon! 
 *      Author: Xavion
 */

#include "ApplicationCode.h"

/* Static variables */
static RNG_HandleTypeDef hrng;
static uint8_t AI;
static uint16_t column;
static uint16_t player;
static uint16_t playerColor;
static int gameBoard[6][7] = {0}; //0 is empty, 1 is red, 2 is yellow
static int gameStatus;


extern void initialise_monitor_handles(void); 

#if COMPILE_TOUCH_FUNCTIONS == 1
static STMPE811_TouchData StaticTouchData;
#endif // COMPILE_TOUCH_FUNCTIONS

void ApplicationInit(void)
{
	initialise_monitor_handles(); // Allows printf functionality
    LTCD__Init();
    LTCD_Layer_Init(0);
    LCD_Clear(0,LCD_COLOR_WHITE);

    #if COMPILE_TOUCH_FUNCTIONS == 1
	InitializeLCDTouch();

	// This is the orientation for the board to be direclty up where the buttons are vertically above the screen
	// Top left would be low x value, high y value. Bottom right would be low x value, low y value.
	StaticTouchData.orientation = STMPE811_Orientation_Portrait_2;

	#endif // COMPILE_TOUCH_FUNCTIONS
	Button_Init_Interrupt();
	//RNG Init
	hrng.Instance = RNG;
	if (HAL_RNG_Init(&hrng) != HAL_OK)
	{
		__disable_irq();
		while (1){}
	}
}

void StartGame(){
	Game_Init();
}

void LCD_Visual_Demo(void)
{
	visualDemo();
}

#if COMPILE_TOUCH_FUNCTIONS == 1
void LCD_Touch_Polling_Demo(void)
{
	LCD_Clear(0,LCD_COLOR_GREEN);
	while (1) {
		/* If touch pressed */
		if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
			/* Touch valid */
			//printf("\nX: %03d\nY: %03d\n", StaticTouchData.x, StaticTouchData.y);
			LCD_Clear(0, LCD_COLOR_RED);
		} else {
			/* Touch not pressed */
			//printf("Not Pressed\n\n");
			LCD_Clear(0, LCD_COLOR_GREEN);
		}
	}
}

void GameOverWaitForInput(){
	while (1) {
		/* If touch pressed */
		if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
			/* Touch valid */
			//Check if touch is in Player 1 Box
			if(StaticTouchData.x > 47 && StaticTouchData.x < 187 && StaticTouchData.y > 32 &&  StaticTouchData.y < 62){
				for(int i = 0; i < 6; i++){
					for(int j = 0; j < 7; j++){
						gameBoard[i][j] = 0;
					}
				}
				gameStatus = 0;
				Game_Board();
				GameWaitForInput();
			}
		}
	}
}

void titleWaitForInput(){
	while (1) {
			/* If touch pressed */
			if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
				/* Touch valid */
				//Check if touch is in Player 1 Box
				if(StaticTouchData.x > 70 && StaticTouchData.x < 170 && StaticTouchData.y > 125 &&  StaticTouchData.y < 175){
					Game_Board();
					AI = 1;
					return;
				}

				if(StaticTouchData.x > 70 && StaticTouchData.x < 170 && StaticTouchData.y > 50 &&  StaticTouchData.y < 100){
					Game_Board();
					return;
				}
				//printf("\nX: %03d\nY: %03d\n", StaticTouchData.x, StaticTouchData.y);
			} else {
				/* Touch not pressed */
				//printf("Not Pressed\n\n");
			}
	}
}

int GameCheck(uint16_t row, uint16_t column){
	//Win Check
	for(int i = -1; i <= 1; i++){
		for(int j = -1; j <= 1; j++){
			if(gameBoard[row+i][column+j] == player){
				int count = 1;
				int columnTmp = -1;
				int rowTmp = -1;
				//Horizontal check
				if(i == 0){
					//Check Left
					while((gameBoard[row][column + columnTmp] == player) && ((column + columnTmp) >= 0)){
						columnTmp--;
						count++;
					}
					columnTmp = 1;
					//Check Right
					while((gameBoard[row][column + columnTmp] == player) && ((column + columnTmp < 7))){
						columnTmp++;
						count++;
					}
					if(count >= 4){
						gameStatus = player;
						return gameStatus;
					}
				}
				count = 1;
				//Vertical check
				if(j == 0){
					//Check Up
					while((gameBoard[row + rowTmp][column] == player) && ((row + rowTmp) >= 0)){
						rowTmp--;
						count++;
					}
					rowTmp = 1;
					//Check Down
					while((gameBoard[row + rowTmp][column] == player) && ((row + rowTmp) < 6)){
						rowTmp++;
						count++;
					}
					if(count >= 4){
						gameStatus = player;
						return gameStatus;
					}
				}
				count = 1;
				columnTmp = -1;
				rowTmp = -1;
				//Left up to down right Diagonal Check
				if(((i == -1) && (j == -1))|| ((i == 1) && (j == 1))){
					while((gameBoard[row + rowTmp][column+columnTmp] == player) && ((row + rowTmp) >= 0) && ((column + columnTmp) >= 0)){
						rowTmp--;
						columnTmp--;
						count++;
					}
					rowTmp = 1;
					columnTmp = 1;
					while((gameBoard[row + rowTmp][column+columnTmp] == player) && ((row + rowTmp) < 6) && ((column + columnTmp) < 7)){
						rowTmp++;
						columnTmp++;
						count++;
					}
					if(count >= 4){
						gameStatus = player;
						return gameStatus;
					}
				}
				count = 1;
				columnTmp = 1;
				rowTmp = -1;
				//Left down to up right Diagonal Check
				if(((i == 1) && (j == -1))|| ((i == -1) && (j == 1))){
					while((gameBoard[row + rowTmp][column+columnTmp] == player) && ((column + columnTmp) < 7) && ((row + rowTmp) >= 0)){
						rowTmp--;
						columnTmp++;
						count++;
					}
					rowTmp = 1;
					columnTmp = -1;
					while((gameBoard[row + rowTmp][column+columnTmp] == player) && ((column + columnTmp) >= 0) && ((row + rowTmp) < 6)){
						rowTmp++;
						columnTmp--;
						count++;
					}
					if(count >= 4){
						gameStatus = player;
						return gameStatus;
					}
				}
			}
		}
	}
	//Tie Check checks the top row to see if it's completely filled
	uint8_t topRowFilled = 0;
	for(int i = 0; i < 7; i++){
		if(gameBoard[0][i] != 0){
			topRowFilled++;
		}
	}
	if(topRowFilled == 7){
		gameStatus = 3; //Tie state
		return gameStatus;
	}
	return gameStatus;
}

void GameWaitForInput(){
	//Initial start
	column = 3;
	player = 1; //Player 1 starts
	playerColor = LCD_COLOR_RED; //Red always goes first
	while (1) {
		Game_Over(gameStatus);
		if(gameStatus != 0){
			GameOverWaitForInput();
		}
		/* If touch pressed */
		if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
			/* Touch valid */
			//Check if touch is on right side
			if(StaticTouchData.x > 118){
				//Makes sure the chip doesn't go off
				if(column < 6){
					moveDiskRight(column, playerColor);
					column++;
					HAL_Delay(500);
				}
			}

			//Check if touch is on left side
			if(StaticTouchData.x <= 118){
				//Makes sure the chip doesn't go off
				if(column > 0){
					moveDiskLeft(column, playerColor);
					column--;
					HAL_Delay(500);
				}
			}
			//printf("\nX: %03d\nY: %03d\n", StaticTouchData.x, StaticTouchData.y);
		} else {
			/* Touch not pressed */
			//printf("Not Pressed\n\n");
		}
	}
}

void doTurn(uint16_t column){
	//Check for what row
	for(int i = 5; i >= 0; i--){
		//Check if space is open
		if(gameBoard[i][column] == 0){
			dropDisk(i, column, playerColor);
			if(playerColor == LCD_COLOR_RED){
				//Update State Array
				gameBoard[i][column] = 1;
				gameStatus = GameCheck((uint16_t) i, column);
				player = 2;
				playerColor = LCD_COLOR_YELLOW;
			}else{
				//Update State Array
				gameBoard[i][column] = 2;
				gameStatus = GameCheck((uint16_t) i, column);
				player = 1;
				playerColor = LCD_COLOR_RED;
			}
			break;
		}
	}
}

void resetAfterDrop(){
	//Reset to initial position
	LCD_Draw_Circle_Fill(28 + 30*column, 53, 12, LCD_COLOR_BLACK);
	column = 3;
	LCD_Draw_Circle_Fill(118, 53, 12, playerColor);
}


#endif // COMPILE_TOUCH_FUNCTIONS


//Interrupts
void EXTI0_IRQHandler(){
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);
	doTurn(column);
	if(AI){
		while(player == 2){
			if(gameStatus != 0){
				break;
			}
			uint32_t random = 0;
			HAL_RNG_GenerateRandomNumber(&hrng, &random);
			random = random % 7;
			doTurn(random);
		}
	}
	resetAfterDrop();
	__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}


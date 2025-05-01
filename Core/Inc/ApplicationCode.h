/*
 * ApplicationCode.h
 *
 *  Created on: Dec 30, 2023
 *      Author: Xavion
 */

#include "LCD_Driver.h"
#include "Button_Driver.h"
#include "stm32f4xx_hal.h"
#include "RNG_Driver.h"

#include <stdio.h>


#ifndef INC_APPLICATIONCODE_H_
#define INC_APPLICATIONCODE_H_

void ApplicationInit(void);
void LCD_Visual_Demo(void);
void StartGame();
void titleWaitForInput();
int GameCheck(uint16_t row, uint16_t column);
void GameWaitForInput();
void doTurn(uint16_t column);
uint16_t AITurn();
void resetAfterDrop();
void GameOverWaitForInput();

#if (COMPILE_TOUCH_FUNCTIONS == 1)
void LCD_Touch_Polling_Demo(void);
#endif // (COMPILE_TOUCH_FUNCTIONS == 1)

#endif /* INC_APPLICATIONCODE_H_ */

/*
 * Button_Driver.c
 *
 *  Created on: Feb 11, 2025
 *      Author: Jonathan
 */

#include "Button_Driver.h"

void Button_Init(){
	GPIO_InitTypeDef ButtonConfig;
	ButtonConfig.Pin = BUTTON_PIN;
	ButtonConfig.Mode = GPIO_MODE_INPUT;
	ButtonConfig.Speed = GPIO_SPEED_FREQ_HIGH;
	ButtonConfig.Pull = GPIO_NOPULL;
	//Clock
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(BUTTON_PORT, &ButtonConfig);
}

void Button_Init_Interrupt(){
	GPIO_InitTypeDef ButtonIntConfig;
	ButtonIntConfig.Pin = BUTTON_PIN;
	ButtonIntConfig.Mode = GPIO_MODE_IT_RISING;
	ButtonIntConfig.Speed = GPIO_SPEED_FREQ_HIGH;
	ButtonIntConfig.Pull = GPIO_NOPULL;
	//Clock
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(BUTTON_PORT, &ButtonIntConfig);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

bool Button_IsPressed(){
	GPIO_PinState state = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
	//If state = 1 for button press
	if(state == BUTTON_PRESSED){
		return true;
	}else{
		return false;
	}
}

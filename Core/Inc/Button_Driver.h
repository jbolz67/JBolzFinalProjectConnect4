/*
 * Button_Driver.h
 *
 *  Created on: Feb 11, 2025
 *      Author: Jonathan
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

#define BUTTON_PORT GPIOA
#define BUTTON_PIN GPIO_PIN_0
#define BUTTON_PRESSED 1
#define BUTTON_UNPRESSED 0
#define EXTI0_IRQ_NUMBER 6

void Button_Init();
void Button_Init_Interrupt();
bool Button_IsPressed();

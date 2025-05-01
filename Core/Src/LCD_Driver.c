/*
 * LCD_Driver.c
 *
 *  Created on: Sep 28, 2023
 *      Author: Xavion
 */

#include "LCD_Driver.h"

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */

static LTDC_HandleTypeDef hltdc;
static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
static FONT_t *LCD_Currentfonts;
static uint16_t CurrentTextColor   = 0xFFFF;

static uint32_t startTime;
static uint32_t stopTime;
static uint32_t player1Wins;
static uint32_t player2Wins;


/*
 * fb[y*W+x] OR fb[y][x]
 * Alternatively, we can modify the linker script to have an end address of 20013DFB instead of 2002FFFF, so it does not place variables in the same region as the frame buffer. In this case it is safe to just specify the raw address as frame buffer.
 */
//uint32_t frameBuffer[(LCD_PIXEL_WIDTH*LCD_PIXEL_WIDTH)/2] = {0};		//16bpp pixel format. We can size to uint32. this ensures 32 bit alignment


//Someone from STM said it was "often accessed" a 1-dim array, and not a 2d array. However you still access it like a 2dim array,  using fb[y*W+x] instead of fb[y][x].
uint16_t frameBuffer[LCD_PIXEL_WIDTH*LCD_PIXEL_HEIGHT] = {0};			//16bpp pixel format.


void LCD_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the LTDC clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /* Enable GPIO clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* GPIO Config
   *
    LCD pins
   LCD_TFT R2 <-> PC.10
   LCD_TFT G2 <-> PA.06
   LCD_TFT B2 <-> PD.06
   LCD_TFT R3 <-> PB.00
   LCD_TFT G3 <-> PG.10
   LCD_TFT B3 <-> PG.11
   LCD_TFT R4 <-> PA.11
   LCD_TFT G4 <-> PB.10
   LCD_TFT B4 <-> PG.12
   LCD_TFT R5 <-> PA.12
   LCD_TFT G5 <-> PB.11
   LCD_TFT B5 <-> PA.03
   LCD_TFT R6 <-> PB.01
   LCD_TFT G6 <-> PC.07
   LCD_TFT B6 <-> PB.08
   LCD_TFT R7 <-> PG.06
   LCD_TFT G7 <-> PD.03
   LCD_TFT B7 <-> PB.09
   LCD_TFT HSYNC <-> PC.06
   LCDTFT VSYNC <->  PA.04
   LCD_TFT CLK   <-> PG.07
   LCD_TFT DE   <->  PF.10
  */

  /* GPIOA configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
                           GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate= GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

 /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_8 | \
                           GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

 /* GPIOC configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

 /* GPIOD configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

 /* GPIOF configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

 /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | \
                           GPIO_PIN_11;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStructure.Alternate= GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}

void LTCD_Layer_Init(uint8_t LayerIndex)
{
	LTDC_LayerCfgTypeDef  pLayerCfg;

	pLayerCfg.WindowX0 = 0;	//Configures the Window HORZ START Position.
	pLayerCfg.WindowX1 = LCD_PIXEL_WIDTH;	//Configures the Window HORZ Stop Position.
	pLayerCfg.WindowY0 = 0;	//Configures the Window vertical START Position.
	pLayerCfg.WindowY1 = LCD_PIXEL_HEIGHT;	//Configures the Window vertical Stop Position.
	pLayerCfg.PixelFormat = LCD_PIXEL_FORMAT_1;  //INCORRECT PIXEL FORMAT WILL GIVE WEIRD RESULTS!! IT MAY STILL WORK FOR 1/2 THE DISPLAY!!! //This is our buffers pixel format. 2 bytes for each pixel
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0;
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	if (LayerIndex == 0){
		pLayerCfg.FBStartAdress = (uintptr_t)frameBuffer;
	}
	pLayerCfg.ImageWidth = LCD_PIXEL_WIDTH;
	pLayerCfg.ImageHeight = LCD_PIXEL_HEIGHT;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, LayerIndex) != HAL_OK)
	{
		LCD_Error_Handler();
	}
}

void clearScreen(void)
{
  LCD_Clear(0,LCD_COLOR_WHITE);
}

void LTCD__Init(void)
{
	hltdc.Instance = LTDC;
	/* Configure horizontal synchronization width */
	hltdc.Init.HorizontalSync = ILI9341_HSYNC;
	/* Configure vertical synchronization height */
	hltdc.Init.VerticalSync = ILI9341_VSYNC;
	/* Configure accumulated horizontal back porch */
	hltdc.Init.AccumulatedHBP = ILI9341_HBP;
	/* Configure accumulated vertical back porch */
	hltdc.Init.AccumulatedVBP = ILI9341_VBP;
	/* Configure accumulated active width */
	hltdc.Init.AccumulatedActiveW = 269;
	/* Configure accumulated active height */
	hltdc.Init.AccumulatedActiveH = 323;
	/* Configure total width */
	hltdc.Init.TotalWidth = 279;
	/* Configure total height */
	hltdc.Init.TotalHeigh = 327;
	/* Configure R,G,B component values for LCD background color */
	hltdc.Init.Backcolor.Red = 0;
	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Green = 0;

	/* LCD clock configuration */
	/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
	/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
	/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
	/* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6Mhz */

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	/* Polarity */
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	LCD_GPIO_Init();

	if (HAL_LTDC_Init(&hltdc) != HAL_OK)
	 {
	   LCD_Error_Handler();
	 }

	ili9341_Init();
}

/* START Draw functions */


/*
 * This is really the only function needed.
 * All drawing consists of is manipulating the array.
 * Adding input sanitation should probably be done.
 */
void LCD_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
	frameBuffer[y*LCD_PIXEL_WIDTH+x] = color;  //You cannot do x*y to set the pixel.
}

/*
 * These functions are simple examples. Most computer graphics like OpenGl and stm's graphics library use a state machine. Where you first call some function like SetColor(color), SetPosition(x,y), then DrawSqure(size)
 * Instead all of these are explicit where color, size, and position are passed in.
 * There is tons of ways to handle drawing. I dont think it matters too much.
 */
void LCD_Draw_Circle_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t radius, uint16_t color)
{
    for(int16_t y=-radius; y<=radius; y++)
    {
        for(int16_t x=-radius; x<=radius; x++)
        {
            if(x*x+y*y <= radius*radius)
            {
            	LCD_Draw_Pixel(x+Xpos, y+Ypos, color);
            }
        }
    }
}

void LCD_Draw_Box(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t borderColor, uint16_t fillColor){
	LCD_Draw_Horizontal_Line(x, y, width, borderColor);
	LCD_Draw_Vertical_Line(x, y, height, borderColor);
	LCD_Draw_Vertical_Line(x + width, y, height, borderColor);
	LCD_Draw_Horizontal_Line(x, y + height, width, borderColor);
	for(uint16_t j = y+1; j < (y + height); j++){
		for(uint16_t i = x+1; i < (x + width); i++){
			LCD_Draw_Pixel(x + (i-x), y + (j-y), fillColor);
		}
	}
}

void LCD_Draw_Vertical_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
  for (uint16_t i = 0; i < len; i++)
  {
	  LCD_Draw_Pixel(x, i+y, color);
  }
}

void LCD_Draw_Horizontal_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
  for (uint16_t i = 0; i < len; i++)
  {
	  LCD_Draw_Pixel(x+i, y, color);
  }
}

void LCD_Clear(uint8_t LayerIndex, uint16_t Color)
{
	if (LayerIndex == 0){
		for (uint32_t i = 0; i < LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT; i++){
			frameBuffer[i] = Color;
		}
	}
  // TODO: Add more Layers if needed
}

//This was taken and adapted from stm32's mcu code
void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

//This was taken and adapted from stm32's mcu code
void LCD_SetFont(FONT_t *fonts)
{
  LCD_Currentfonts = fonts;
}

//This was taken and adapted from stm32's mcu code
void LCD_Draw_Char(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, counter = 0;
  for(index = 0; index < LCD_Currentfonts->Height; index++)
  {
    for(counter = 0; counter < LCD_Currentfonts->Width; counter++)
    {
      if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> counter)) == 0x00) && (LCD_Currentfonts->Width <= 12)) || (((c[index] & (0x1 << counter)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))
      {
         //Background If want to overrite text under then add a set color here
      }
      else
      {
    	  LCD_Draw_Pixel(counter + Xpos,index + Ypos,CurrentTextColor);
      }
    }
  }
}

//This was taken and adapted from stm32's mcu code
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
  Ascii -= 32;
  LCD_Draw_Char(Xpos, Ypos, &LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}

void visualDemo(void)
{
	uint16_t x;
	uint16_t y;
	// This for loop just illustrates how with using logic and for loops, you can create interesting things
	// this may or not be useful ;)
	for(y=0; y<LCD_PIXEL_HEIGHT; y++){
		for(x=0; x < LCD_PIXEL_WIDTH; x++){
			if (x & 32)
				frameBuffer[x*y] = LCD_COLOR_WHITE;
			else
				frameBuffer[x*y] = LCD_COLOR_BLACK;
		}
	}

	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_GREEN);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_RED);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_WHITE);
	LCD_Draw_Vertical_Line(10,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);
	LCD_Draw_Vertical_Line(230,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);

	LCD_Draw_Circle_Fill(125,150,20,LCD_COLOR_BLACK);
	HAL_Delay(2000);

	LCD_Clear(0,LCD_COLOR_BLUE);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(100,140,'H');
	LCD_DisplayChar(115,140,'e');
	LCD_DisplayChar(125,140,'l');
	LCD_DisplayChar(130,140,'l');
	LCD_DisplayChar(140,140,'o');

	LCD_DisplayChar(100,160,'W');
	LCD_DisplayChar(115,160,'o');
	LCD_DisplayChar(125,160,'r');
	LCD_DisplayChar(130,160,'l');
	LCD_DisplayChar(140,160,'d');
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void LCD_Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

// Touch Functionality   //

#if COMPILE_TOUCH_FUNCTIONS == 1

void InitializeLCDTouch(void)
{
  if(STMPE811_Init() != STMPE811_State_Ok)
  {
	 for(;;); // Hang code due to error in initialzation
  }
}

STMPE811_State_t returnTouchStateAndLocation(STMPE811_TouchData * touchStruct)
{
	return STMPE811_ReadTouch(touchStruct);
}

void DetermineTouchPosition(STMPE811_TouchData * touchStruct)
{
	STMPE811_DetermineTouchPosition(touchStruct);
}

uint8_t ReadRegisterFromTouchModule(uint8_t RegToRead)
{
	return STMPE811_Read(RegToRead);
}

void WriteDataToTouchModule(uint8_t RegToWrite, uint8_t writeData)
{
	STMPE811_Write(RegToWrite, writeData);
}

#endif // COMPILE_TOUCH_FUNCTIONS

void Game_Init(){
	uint16_t x;
	uint16_t y;
	// This for loop just illustrates how with using logic and for loops, you can create interesting things
	// this may or not be useful ;) Seems like this sets up the screen
	for(y=0; y<LCD_PIXEL_HEIGHT; y++){
		for(x=0; x < LCD_PIXEL_WIDTH; x++){
			if (x & 32)
				frameBuffer[x*y] = LCD_COLOR_WHITE;
			else
				frameBuffer[x*y] = LCD_COLOR_BLACK;
		}
	}
	//Blue background
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_BLUE);

	//Connect 4 Title
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(70,50,'C');
	LCD_DisplayChar(85,50,'o');
	LCD_DisplayChar(97,50,'n');
	LCD_DisplayChar(110,50,'n');
	LCD_DisplayChar(122,50,'e');
	LCD_DisplayChar(134,50,'c');
	LCD_DisplayChar(143,50,'t');

	LCD_SetTextColor(LCD_COLOR_RED);
	LCD_DisplayChar(160,50,'4');

	//Play Options
	LCD_Draw_Box(70, 125, 100, 50, LCD_COLOR_BLACK, LCD_COLOR_YELLOW);
	LCD_Draw_Box(70, 200, 100, 50, LCD_COLOR_BLACK, LCD_COLOR_YELLOW);

	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);
	LCD_DisplayChar(77,140,'1');
	LCD_DisplayChar(95,140,'P');
	LCD_DisplayChar(107,140,'l');
	LCD_DisplayChar(115,140,'a');
	LCD_DisplayChar(127,140,'y');
	LCD_DisplayChar(139,140,'e');
	LCD_DisplayChar(151,140,'r');

	LCD_DisplayChar(77,215,'2');
	LCD_DisplayChar(95,215,'P');
	LCD_DisplayChar(107,215,'l');
	LCD_DisplayChar(115,215,'a');
	LCD_DisplayChar(127,215,'y');
	LCD_DisplayChar(139,215,'e');
	LCD_DisplayChar(151,215,'r');
}

void Game_Board(){
	//Set background
	LCD_Clear(0, LCD_COLOR_BLACK);

	//Setup Board
	LCD_Draw_Box(13, 68, 210, 180, LCD_COLOR_BLACK, LCD_COLOR_BLUE);
	for(int j = 0; j < 6; j++){
		for(int i = 0; i < 7; i++){
			LCD_Draw_Circle_Fill(28 + 30*i, 83 + 30*j, 12, LCD_COLOR_BLACK);
		}
	}

	//Red goes first
	LCD_Draw_Circle_Fill(118, 53, 12, LCD_COLOR_RED);
	//LCD_Draw_Circle_Fill(118, 233, 12, LCD_COLOR_YELLOW);
	startTime = HAL_GetTick(); //Start the timer
}

void Game_Over(uint16_t player){
	if(player == 0){
		return;
	}
	stopTime = HAL_GetTick();
	LCD_Draw_Circle_Fill(118, 53, 12, LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);
	//Display Game State
	if(player == 1){
		LCD_SetTextColor(LCD_COLOR_RED);
		LCD_DisplayChar(95-40,40,'P');
		LCD_DisplayChar(107-40,40,'l');
		LCD_DisplayChar(115-40,40,'a');
		LCD_DisplayChar(127-40,40,'y');
		LCD_DisplayChar(139-40,40,'e');
		LCD_DisplayChar(151-40,40,'r');

		LCD_DisplayChar(127,40,'1');
		LCD_DisplayChar(151,40,'W');
		LCD_DisplayChar(165,40,'i');
		LCD_DisplayChar(175,40,'n');
		player1Wins++;
	}else if(player == 2){
		LCD_SetTextColor(LCD_COLOR_YELLOW);
		LCD_DisplayChar(95-40,40,'P');
		LCD_DisplayChar(107-40,40,'l');
		LCD_DisplayChar(115-40,40,'a');
		LCD_DisplayChar(127-40,40,'y');
		LCD_DisplayChar(139-40,40,'e');
		LCD_DisplayChar(151-40,40,'r');

		LCD_DisplayChar(127,40,'2');
		LCD_DisplayChar(151,40,'W');
		LCD_DisplayChar(165,40,'i');
		LCD_DisplayChar(175,40,'n');
		player2Wins++;
	}else{
		LCD_SetTextColor(LCD_COLOR_WHITE);
		LCD_DisplayChar(139-40,40,'T');
		LCD_DisplayChar(151-40,40,'i');
		LCD_DisplayChar(121,40,'e');
	}
	//Time

	//Win Count
	LCD_SetFont(&Font12x12);
	LCD_SetTextColor(LCD_COLOR_RED);
	LCD_DisplayChar(13, 255, 'R');
	LCD_DisplayChar(22, 255, 'e');
	LCD_DisplayChar(28, 255, 'd');
	LCD_DisplayChar(34, 255, ':');
	//Int to char
	if(player1Wins == 0){
		LCD_DisplayChar(28, 265, '0');
	}else{
		uint32_t temp = player1Wins;
		int digitOffset = 0;
		while(temp != 0){
			char print = 0x30 + (temp % 10);
			LCD_DisplayChar(28-digitOffset, 270, print);
			temp = temp/10;
			digitOffset = digitOffset + 4;
		}
	}

	LCD_SetTextColor(LCD_COLOR_YELLOW);
	LCD_DisplayChar(200-15, 255, 'Y');
	LCD_DisplayChar(208-15, 255, 'e');
	LCD_DisplayChar(216-15, 255, 'l');
	LCD_DisplayChar(219-15, 255, 'l');
	LCD_DisplayChar(222-15, 255, 'o');
	LCD_DisplayChar(229-15, 255, 'w');
	LCD_DisplayChar(237-15, 255, ':');

	//Int to char
	if(player2Wins == 0){
		LCD_DisplayChar(214, 270, '0');
	}else{
		uint32_t temp = player2Wins;
		int digitOffset = 0;
		while(temp != 0){
			char print = 0x30 + (temp % 10);
			LCD_DisplayChar(214-digitOffset, 270, print);
			temp = temp/10;
			digitOffset = digitOffset + 4;
		}
	}
	//Replay Button
	LCD_Draw_Box(47, 255, 137, 30, LCD_COLOR_BLUE, LCD_COLOR_WHITE);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);
	LCD_DisplayChar(55,257,'P');
	LCD_DisplayChar(67,257,'l');
	LCD_DisplayChar(77,257,'a');
	LCD_DisplayChar(89,257,'y');
	LCD_DisplayChar(109,257,'A');
	LCD_DisplayChar(125,257,'g');
	LCD_DisplayChar(137,257,'a');
	LCD_DisplayChar(146,257,'i');
	LCD_DisplayChar(155,257,'n');
	LCD_DisplayChar(167,257,'?');

	//Time Elapsed

	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_DisplayChar(107-40,10,'T');
	LCD_DisplayChar(118-40,10,'i');
	LCD_DisplayChar(130-40,10,'m');
	LCD_DisplayChar(145-40,10,'e');
	LCD_DisplayChar(154-40,10,':');
	uint32_t time = (stopTime - startTime)/1000;
	int digitOffset = 0;
	while(time != 0){
		char print = 0x30 + (time % 10);
		LCD_DisplayChar(165-digitOffset, 10, print);
		time = time/10;
		digitOffset = digitOffset + 12;
	}
}

void moveDiskLeft(uint16_t column, uint16_t color){
	//Remove old circle
	LCD_Draw_Circle_Fill(28 + 30*column, 53, 12, LCD_COLOR_BLACK);
	//Add new Circle
	LCD_Draw_Circle_Fill(28 + 30*(column-1), 53, 12, color);
}

void moveDiskRight(uint16_t column, uint16_t color){
	//Remove old circle
	LCD_Draw_Circle_Fill(28 + 30*column, 53, 12, LCD_COLOR_BLACK);
	//Add new Circle
	LCD_Draw_Circle_Fill(28 + 30*(column+1), 53, 12, color);
}

void dropDisk(uint16_t row, uint16_t column, uint16_t color){
	LCD_Draw_Circle_Fill(28 + 30*column, 53, 12, LCD_COLOR_BLACK);
	LCD_Draw_Circle_Fill(28 + 30*column, 83 + (30 * row), 12, color);
}

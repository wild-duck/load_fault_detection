/**
  **************************************************************************************
  * @file led.c
  * @author shaokairu
  * @date 2021.7.8
  * @brief This file provides all function define of led which is on the stm32 core board
  *        sold by ALIENTEK 
  **************************************************************************************
  */
  
/* Includes --------------------------------------------------------------------------*/
#include "gpio.h"

/**
  * @brief led gpio initializtion
  * @param None
  * @retval None
  */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOG_CLK_ENABLE();           	//开启GPIOG时钟
	
    GPIO_Initure.Pin = GPIO_PIN_13 | GPIO_PIN_15; 				// PG13/PG15
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  	//推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;          	//上拉
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;    //高速
    HAL_GPIO_Init( GPIOG, &GPIO_Initure);
	
    HAL_GPIO_WritePin( GPIOG, GPIO_PIN_15, GPIO_PIN_RESET);	// PB15 set 0, 
    HAL_GPIO_WritePin( GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);	// PB13 set 0,
}


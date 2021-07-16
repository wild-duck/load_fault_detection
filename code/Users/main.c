/**
  **************************************************************************************
  * @file main.c
  * @author shaokairu
  * @date 2021.7.15
  * @brief This file includes the main program body
  **************************************************************************************
  */

/* Includes --------------------------------------------------------------------------*/
#include "main.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "lcd.h"
#include "timer.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_demo_widgets.h"



/* Private variables -----------------------------------------------------------------*/

/**
  *@brief The application entry point.
  *@param None
  *@retval int
  */
int main(void)
{

    /* MCU Configuration--------------------------------------------------------------*/
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    /* Configure the system clock */
    Stm32_Clock_Init(RCC_PLL_MUL9);
    
    /* Initialize all configured peripherals */
    delay_init(72);
    uart_init(115200);
    TIM4_Init( 999, 71);
    LCD_Init();
    
    lv_init();
    lv_port_disp_init();
    lv_demo_widgets();
    
    /* Pragram begin configuration----------------------------------------------------*/


    
    /* Infinite loop */
    while(1)
    {
        lv_task_handler();
    }
}

/**********************************END OF FILE*****************************************/

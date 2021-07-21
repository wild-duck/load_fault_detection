/**
  **************************************************************************************
  * @file main.c
  * @author shaokairu
  * @date 2021.7.15
  * @brief This file includes the main program body
  **************************************************************************************
  *
  * ADC1-PA1-phase  ADC2-PA2-采样电阻前交流    ADC3-~后交流
  * ADC4-~后直流    ADC5-差分
  *
  *
  */

/* Includes --------------------------------------------------------------------------*/
#include "main.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "exti.h"
#include "adc.h"
#include "AD9854_hal.h"

/* Private variables -----------------------------------------------------------------*/
uint8_t mode = 0;
uint8_t sw = 0;
uint16_t ad[5] ={ 0,};

/**
  *@brief The application entry point.
  *@param None
  *@retval int
  */
int main(void)
{
    uint8_t i, j;
    uint32_t fre = 1000;

    /* MCU Configuration--------------------------------------------------------------*/
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    /* Configure the system clock */
    Stm32_Clock_Init(RCC_PLL_MUL9);
    
    /* Initialize all configured peripherals */
    delay_init(72);
    uart_init(115200);
    EXTI_Init();
    ADC1_Init();
    AD9854_SPI_GPIO_Init();
    
    /* Pragram begin configuration----------------------------------------------------*/
    AD9854_SPI_Init();
    AD9854_SPI_SetSine( 1000,4095);


    /* Infinite loop */
    while(1)
    {
        if(sw == 1)
        {
            for(i = 0; i < 100; i ++)
            {
                fre = 1000 + i * 1000;
                AD9854_SPI_SetSine( fre,4095);
                delay_ms(20);
                ad[1] = Get_Adc_Average( ADC_CHANNEL_2,3);
                ad[2] = Get_Adc_Average( ADC_CHANNEL_3,3);
                printf("fre=%d,ad2=%d,ad3=%d\r\n", fre, ad[1], ad[2]);  
            }     
            sw = 0;
        }
        
    }
}

/**********************************END OF FILE*****************************************/

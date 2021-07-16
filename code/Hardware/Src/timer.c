/**
  **************************************************************************************
  * @file timer.c
  * @author shaokairu
  * @date 2021.7.8
  * @brief This file provides the function of the timer interrupt 
  **************************************************************************************
  */
  
/* Includes --------------------------------------------------------------------------*/
#include "timer.h"
#include "led.h"
#include "lvgl.h"

/* Public variables ------------------------------------------------------------------*/

/* Private variables -----------------------------------------------------------------*/
uint16_t adc1;
uint16_t adc2;
uint16_t adc3;

/* Private handle --------------------------------------------------------------------*/
TIM_HandleTypeDef TIM3_Handler;     
TIM_HandleTypeDef TIM4_Handler; 

/**
  * @brief timer3 initialise
  * @param arr: automatic reload value 
  * @param psc: division number
  * @retval None
  * @Note TimerFre = 72 / ((arr + 1) * (psc + 1) MHz
  */
void TIM3_Init(u16 arr,u16 psc)
{  
    TIM3_Handler.Instance = TIM3;                          //通用定时器3
    TIM3_Handler.Init.Prescaler = psc;                     //分频系数
    TIM3_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;    //向上计数器
    TIM3_Handler.Init.Period = arr;                        //自动装载值
    TIM3_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM3_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM3_Handler); //使能定时器3和定时器3更新中断：TIM_IT_UPDATE   
}

/**
  * @brief timer4 initialise
  * @param arr: automatic reload value 
  * @param psc: division number
  * @retval None
  * @Note TimerFre = 72 / ((arr + 1) * (psc + 1) MHz
  */
void TIM4_Init(u16 arr,u16 psc)
{  
    TIM4_Handler.Instance = TIM4;                          //通用定时器4
    TIM4_Handler.Init.Prescaler = psc;                     //分频系数
    TIM4_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;    //向上计数器
    TIM4_Handler.Init.Period = arr;                        //自动装载值
    TIM4_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM4_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM4_Handler); //使能定时器4和定时器4更新中断：TIM_IT_UPDATE   
}

/**
  * @brief  Initializes the TIM Base MSP.
  * @param  htim : TIM handle
  * @retval None
  * @note this function will be called by HAL_TIM_Base_Init()
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority( TIM3_IRQn, 1,3 );    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //开启ITM3中断   
	}
    else if(htim->Instance == TIM4)
	{
		__HAL_RCC_TIM4_CLK_ENABLE();            //使能TIM4时钟
		HAL_NVIC_SetPriority( TIM4_IRQn, 2, 4);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM4_IRQn);          //开启ITM4中断   
	}
}

/**
  * @brief  Timer interrupet service function
  * @{
  */

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handler);
}

void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM4_Handler);
}

/**
  * @}
  */

/**
  * @brief  Period elapsed callback in non blocking mode 
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim==(&TIM3_Handler))
    {
       lv_tick_inc(1);
    }
    else if(htim == (&TIM4_Handler))
    {
        
    }
}

/**********************************END OF FILE*****************************************/

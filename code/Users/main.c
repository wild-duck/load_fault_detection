/**
  **************************************************************************************
  * @file main.c
  * @author shaokairu
  * @date 2021.7.15
  * @brief This file includes the main program body
  **************************************************************************************
  *
  * ADC1-PA1-phase  ADC2-PA2-采样电阻前交流    ADC3-~后交流
  * ADC4-   ADC5-差分
  *
  *
  */

/* Includes --------------------------------------------------------------------------*/
#include "main.h"
#include "math.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "exti.h"
#include "adc.h"
#include "AD9854_hal.h"
#include "math.h"
#include "gpio.h"

/* Private variables -----------------------------------------------------------------*/
uint8_t uartSwitch = 0;
uint8_t sw = 0; // measurement status flag
uint8_t DCType = 0;    // the flag of the DC status  
uint8_t NetType = 0;    // the flag of the curcit network status 
uint16_t DC = 0;    // the value of reading ADC4
uint16_t BackAC[200] ={ 0,}; // the value of reading ADC3
uint16_t ForwardAC[200]={ 0,};   // the value of reading ADC2
float Res;
float ResSample = 0;
float AveForwardAC = 0;
float AveBackAC = 0;
float CapFre = 0;
float Cap;
float Ind;
float IndFre = 0;

/**
  * @brief judge the trend of the data in the array
  * @param data: array to be processed
  * @param the length of the array @reg data
  * @retval the trend of the data.
  *					0 even  1 up 2 down 3 up and then down 4 down and then up
  *                 5 error
  */
uint8_t trend_judge(const uint16_t data[], uint16_t n)
{
    uint8_t i, iMax = 0, iMin = 0, trendType;
    uint16_t MaxData = data[0], MinData = data[0];
    
    for(i = 0; i < n; i ++)
    {
        if(MaxData < data[i])
        {
            MaxData = data[i];
            iMax = i;
        }
        else if(MinData > data[i])
        {
            MinData = data[i];
            iMin = i;
        }
    }
    if((MaxData - MinData) < 30)
        trendType = 0;
    else if(data[0] < data[n - 1] && iMax > n - 6 && iMin < 5)
        trendType = 1;
    else if(data[n-1] < data[0] && iMax < 5 && (data[n -1] - MinData) < 30)
        trendType = 2;
    else if(data[0] < MaxData && data[n-1] < MaxData && iMax > 5) 
        trendType = 3;
    else if(data[0] > MinData && data[n-1] > MinData && iMin > 5) 
        trendType = 4;
    else
    {
        printf("judge failed\r\n"); 
        printf("MaxData=%d,MinData=%d,iMax=%d,iMin=%d,data[0]=%d,data[99]=%d\r\n", MaxData, MinData, iMax, iMin, data[0], data[n - 1]);
        return 5;
    }
    printf("MaxData=%d,MinData=%d,iMax=%d,iMin=%d,data[0]=%d,data[99]=%d\r\n", MaxData, MinData, iMax, iMin, data[0], data[n - 1]);
    printf("trendType=%d\r\n", trendType);
    return trendType;
}

/**
  *@brief The application entry point.
  *@param None
  *@retval int
  */
int main(void)
{
    uint16_t i = 0;
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
    GPIO_Init();
    AD9854_SPI_GPIO_Init();
    
    /* Pragram begin configuration----------------------------------------------------*/
    Res = 0;
    ResSample = 0;
    AveForwardAC = 0;
    AveBackAC = 0;
    CapFre = 0;
    Cap = 0;
    Ind = 0;
    IndFre = 0;
    AD9854_SPI_Init();
    AD9854_SPI_SetSine(500,4095);


    /* Infinite loop */
    while(1)
    {
        if(sw == 1)
        {
            /* start the measurement */
            AD9854_SPI_SetSine(1000, 4095);
			printf("measurement begin\r\n");
            HAL_GPIO_WritePin( GPIOG, GPIO_PIN_15, GPIO_PIN_SET);   // relay switch to DC
            delay_ms(100);   // wait the circuit network
            DC = Get_Adc_Average( ADC_CHANNEL_4,3);            
            HAL_GPIO_WritePin( GPIOG, GPIO_PIN_15, GPIO_PIN_RESET);  // relay switch to AC
            delay_ms(300);  // wait the circuit network
            
            /* swap frequence */
            for(i = 0; i < 200; i ++)    // from 1000 to 50000 step 1000 (Hz) 
            {
                fre = 1000 + 500 * i;
                AD9854_SPI_SetSine(fre, 4095);
                delay_ms(10);
                ForwardAC[i] = Get_Adc_Average(ADC_CHANNEL_2,10);
                BackAC[i] = Get_Adc_Average(ADC_CHANNEL_3,10);
            }
            /* end the measurement */
            
            /* begin to analysis the data */
            /* judge the circuit working status */
            for(i = 0; i < 100; i++)
            {
                AveForwardAC += ForwardAC[i];
                AveBackAC += BackAC[i];
            }
            AveForwardAC /= 100;
            AveBackAC /= 100;
            ResSample = (float)DC / (2457.0 - DC) * 99.0;
            if(DC > 2400)
                DCType = 0; // open circuit
            else if(DC < 300)
                DCType = 1; // short circuit
            else
                DCType = 2; // with a load
            printf("DC =%d, DCtype=%d\r\n", DC, DCType);
            printf("ResSample=%f\r\n", ResSample);
            if( DCType == 0)    // if open circuit
            {
                if((590<=BackAC[0]&&BackAC[0]<= 630)&&(590<=BackAC[40]&&BackAC[40]<= 630)&&(590<BackAC[60]&&BackAC[60]<= 630)&&(590<=BackAC[79]&&BackAC[79]<= 630))
                {
                    printf("1\r\n");
                    NetType = 0; // open circuit
                }
                else
                {
                    switch(trend_judge( BackAC, 100))
                    {
                        case 2: 
                            if(BackAC[79] < 70)
                            {
                                NetType = 1; // C
                                for (i = 0; i < 80; i++)
                                {
                                    if(BackAC[i] <= AveForwardAC * 0.707)
                                    {
                                        printf("i=%d, BackAC=%d\r\n", i, BackAC[i]);
                                        break;
                                    }
                                }
                                CapFre = 1000 + 500 * i;
                                Cap = 1 / (2 * 3.1415926 * CapFre * 99) * 10000000;
                            }
                            else
                                NetType = 2;	// RC serial
                            break;
                        case 4:
                            for(i = 0; i < 100; i ++)
                            {
                                if(BackAC[i] < 200)
                                {
                                    NetType = 4; // LC serial
                                    break;
                                }
                                NetType = 3; // RLC serial
                            }
                    }
                }
            }
            else if(DCType == 1)
            {
                if(BackAC[0] < 10 && BackAC[40] < 10 && BackAC[60] < 10 && BackAC[79] <10)
                {
                    NetType = 5; // short
                }
                else
                {
                    switch(trend_judge( BackAC, 100))
                    {
                        case 1:
                            if(BackAC[0] > 100)
                                NetType = 6; // LR parallel
                            else
                            {
                                NetType = 7; // L
                                for (i = 0; i < 200; i ++)
                                {
                                    if(BackAC[i] >= AveForwardAC * 0.3535)
                                    {
                                        printf("i=%d, BackAC=%d\r\n", i, BackAC[i]);
                                        //printf("i=%d, BackAC=%d\r\n", i-1, BackAC[i-1]);
                                        break;
                                    }
                                }
                                IndFre = 1000 + i * 500;
                                Ind = sqrt((float)(9801 - 7 * ResSample * ResSample + 198 * ResSample) / (276.348923 * IndFre * IndFre)) * 1000000;
                                Ind = (float)(Ind + (Ind*0.138+1.112));
                            }
                            break;
                        case 3:
                            NetType = 11;   // LC parallel
                            break;
                    }
                }
            }
            else if(DCType == 2)
            {
                switch(trend_judge( BackAC, 100))
                {
                    case 1:
                        NetType = 8;	// RL serial
                        break;
                    case 2:
                        NetType = 9;	// RC parallel
                        break;
                    case 0: 
                        NetType = 10; // R 
                        Res = 99 * AveBackAC / (AveForwardAC - AveBackAC);
                        break;
                }
            }

            if(NetType == 0)
                printf("open\r\n");
            else if(NetType == 1)
            {
                printf("C\r\n");
                printf("%fuF\r\n", Cap);
            }
            else if(NetType == 2)
                printf("RC serial\r\n");
            else if(NetType == 3)
                printf("RLC serial\r\n");
            else if(NetType == 4)
                printf("LC serial\r\n");
            else if(NetType == 5)
                printf("short\r\n");
            else if(NetType == 6)
                printf("LR parallel\r\n");
            else if(NetType == 7)
            {
                printf("L\r\n");
                printf("%fuH\r\n", Ind);
            }
            else if(NetType == 8)
                printf("RL serial\r\n");
            else if(NetType == 9)
                printf("RC parallel\r\n");
            else if(NetType == 10)
            {
                printf("R\r\n");
                printf("%fohm\r\n", Res);
            }
            else if(NetType == 11)
                printf("LC parallel\r\n");
            
            if(uartSwitch == 1)
            {
                printf("DCType=%d\r\n", DCType);
                for(i = 0; i < 80; i ++)
                    printf("forward=%d,back=%d\r\n", ForwardAC[i], BackAC[i]);
            }
            sw = 0; // claer flag
        }
    }
}

/**********************************END OF FILE*****************************************/

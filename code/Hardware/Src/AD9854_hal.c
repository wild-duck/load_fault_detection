/**
  **************************************************************************************
  * @file ad9833.c
  * @author shaokairu
  * @date 2021.7.19
  * @brief This file provides the function of ad9833 board 
  **************************************************************************************
  */

/* Includes --------------------------------------------------------------------------*/
#include "AD9854_hal.h"
#include "delay.h"

/* Private define --------------------------------------------------------------------*/
#define CLK_SET_HH 1

/* Private macro ---------------------------------------------------------------------*/
#ifdef CLK_SET_LL

#define CLK_Set 7
const uint32_t  Freq_mult_ulong  = 1340357;
const double Freq_mult_doulle = 1340357.032;

#elif CLK_SET_L

#define CLK_Set 9
const uint_least32_t Freq_mult_ulong  = 1042500;		 
const double Freq_mult_doulle = 1042499.9137431;

#elif CLK_SET_H

#define CLK_Set 0x48
const uint32_t Freq_mult_ulong  = 1172812;
const double Freq_mult_doulle = 1172812.403;

#elif CLK_SET_HH

#define CLK_Set 0x4A
const uint32_t Freq_mult_ulong  = 938250;
const double Freq_mult_doulle = 938249.9224;

#endif

/* Private variables -----------------------------------------------------------------*/
uint8_t FreqWord[6];
uint16_t add = 0;
uint16_t step_up = 0;
uint32_t fund_fre_buf;
uint16_t amp_buf;
uint8_t modu_buf;

/** @defgroup GPIO
  * @brief initialise the code 
  * @{
  */ 

/** @brief AD9854 parallel-port-control pins configuration
  * @param None
  * @retval None
  */  
void AD9854_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_Initure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                       GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_Initure.Speed = GPIO_SPEED_MEDIUM;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);

    GPIO_Initure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
                       GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_Initure.Speed = GPIO_SPEED_MEDIUM;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);

    GPIO_Initure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
                       GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
    GPIO_Initure.Speed = GPIO_SPEED_MEDIUM;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_Initure);
}

/** @brief AD9854 serial-port-control pins configuration
  * @param None
  * @retval None
  */  
void AD9854_SPI_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_Initure.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_Initure.Speed = GPIO_SPEED_MEDIUM;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);

    GPIO_Initure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                       GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_Initure.Speed = GPIO_SPEED_MEDIUM;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_Initure);
}

/**
  * @}
  */ 

/** @defgroup AD9854 driver
  * @{
  */  

/** @defgroup concurrent control
  * @{
  */ 

/**
  * @brief AD9864 parallel port writes the data
  * @param addr: A 6-bit address
  * @param dat: the data which would be written
  * @retval None
  */ 

void AD9854_WR_Byte(uint8_t addr, uint8_t dat)
{
    uint16_t Tmp;

    Tmp = (GPIOA->ODR);
    GPIOA->ODR = (uint16_t)((addr & 0x3f) | (Tmp & 0xFFC0)); // ADDER
    Tmp = (GPIOB->ODR);
    GPIOB->ODR = ((uint16_t)(dat << 8) | (Tmp & 0x00FF)); // DATA
    AD9854_WR_CLR;
	AD9854_WR_SET;
}

/**
  * @brief AD9854 initialise
  * @param None
  * @retval None
  */
void AD9854_Init(void)
{
	AD9854_WR_SET;                 // disable the write-and-read control ports
	AD9854_RD_SET;
	AD9854_SP_SET;

//	AD9854_UDCLK_Clr;
// 	AD9854_RST_Set;                // reset AD9854
	AD9854_RST_CLR;
	delay_us(100);
	AD9854_RST_SET;
	delay_us(100);
	AD9854_RST_CLR;

 	AD9854_WR_Byte(0x1d,0x00);	 	 // Start the comparator, only enabled it can the square be generated
//	AD9854_WR_Byte(0x1d,0x10);	    // Close the comparator
	AD9854_WR_Byte(0x1e,CLK_Set);	 // Set the system clock doubling frequency           
	AD9854_WR_Byte(0x1f,0x01);	   // Set the system to mode 0, updated by externally 
	AD9854_WR_Byte(0x20,0x60);	   // Set to the adjustable amplitude, 
                                   //cancel the interpolation compensation, 
                                   // and turn off the sinc effect
}

/**
  * @brief The sinusoidal signal frequency data conversion
  * @param Freq: the frequence to be converted, 0~SYSCLK/2
  * @retval None 
  * @note The function is based on the formula(FTW = (Desired Output Frequency × 2N)/SYSCLK).
  *       In the function FTW is the array,FreqWord[6], and n = 48, and Desired Output Frequency is Freq
  */
void Freq_Convert(long Freq)   
{	
    uint32_t FreqBuf;
    uint32_t Temp = Freq_mult_ulong;
    uint8_t Array_Freq[4]; // Break the input frequency factor into four bytes 

    Array_Freq[0] = (uint8_t)Freq;
    Array_Freq[1] = (uint8_t)(Freq >> 8);
    Array_Freq[2] = (uint8_t)(Freq >> 16);
    Array_Freq[3] = (uint8_t)(Freq >> 24);

    FreqBuf = Temp * Array_Freq[0];
    FreqWord[0] = FreqBuf;
    FreqBuf >>= 8;

    FreqBuf += (Temp * Array_Freq[1]);
    FreqWord[1] = FreqBuf;
    FreqBuf >>= 8;

    FreqBuf += (Temp * Array_Freq[2]);
    FreqWord[2] = FreqBuf;
    FreqBuf >>= 8;

    FreqBuf += (Temp * Array_Freq[3]);
    FreqWord[3] = FreqBuf;
    FreqBuf >>= 8;

    FreqWord[4] = FreqBuf;
    FreqWord[5] = FreqBuf >> 8;	
} 

/**
  * @brief The sinusoidal signal frequency data conversion with high precision
  * @param Freq: the frequence to be converted, 0~SYSCLK/2
  * @retval None 
  * @note The function is based on the formula(FTW = (Desired Output Frequency × 2N)/SYSCLK).
  *       In the function FTW is the array,FreqWord[6], and n = 48, and Desired Output Frequency is Freq
  * @note The difference between this function and the function @ref Freq_Convert(long Freq) 
  *       is in the type of paramters. if you want to generate below 100Hz signal 
  *       you should use this function @ref Freq_Double_Convert(double Freq). But
  *       if you want to generate over 100Hz signal you just use the funcion 
  *       @ref Freq_Convert(long Freq).
  */
void Freq_Double_Convert(double Freq)   
{
	uint32_t Low32;
    uint32_t High16;
    double Temp = Freq_mult_doulle; // 23ca99 = (2^48)/(120M)

    Freq*=(double)(Temp);
	// 1 0000 0000 0000 0000 0000 0000 0000 0000 = 4294967295
    High16 = (int)(Freq / 4294967295); //2^32 = 4294967295
    Freq -= (double)High16 * 4294967295;
    Low32 = (uint32_t)Freq;

	FreqWord[0]=Low32;	     
	FreqWord[1]=Low32>>8;
	FreqWord[2]=Low32>>16;
	FreqWord[3]=Low32>>24;
	FreqWord[4]=High16;
	FreqWord[5]=High16>>8;			
}

/**
  * @brief sine generate
  * @param Freq： the frequence of signal, 0~(1/2)*SYSCLK
  * @param Shape： the amplitude of the signal, 0~4095
  * @retval None
  * @note If you want to use the function @ref Freq_Convert(long Freq) 
  *       you must use this function to generate the sinusoidal signal
  */ 
void AD9854_SetSine(uint32_t Freq,uint16_t Shape)
{
    uint8_t count = 0;
    uint8_t Address;

	Address = 0x04;  // chose the original value of address

    Freq_Convert(Freq); // change frequence

    /* write 6-bit frequence-control */
    for(count=6;count>0;)
    {
        AD9854_WR_Byte(Address++, FreqWord[--count]);
    }

    /* set the amplitude of the channel I */
    AD9854_WR_Byte(0x21, (Shape >> 8) & 0x00FF);
    AD9854_WR_Byte(0x22, (Shape & 0x00ff));

    /* set the amplitude of the channel Q */
    AD9854_WR_Byte(0x23, (Shape >> 8) & 0x00FF);
    AD9854_WR_Byte(0x24, (Shape & 0x00ff));

    /* update the output */
    AD9854_UDCLK_SET;
    AD9854_UDCLK_CLR;

}

/**
  * @brief sine generate with high precision
  * @param Freq： the frequence of signal, 0~(1/2)*SYSCLK
  * @param Shape： the amplitude of the signal, 0~4095
  * @retval None
  * @note If you want to use the function @ref Freq_Double_Convert(double Freq)
  *       you must use this function to generate the sinusoidal signal
  */ 
void AD9854_SetSine_Double(double Freq,uint16_t Shape)
{
    uint8_t count = 0;
    uint8_t Address;

	Address = 0x04;  // chose the original value of address

    Freq_Double_Convert(Freq); // change frequence

    /* write 6-bit frequence-control */
    for(count=6;count>0;) 
    {
        AD9854_WR_Byte(Address++, FreqWord[--count]);
    }

    /* set the amplitude of the channel I */
    AD9854_WR_Byte(0x21, Shape >> 8);
    AD9854_WR_Byte(0x22, (uint8_t)(Shape & 0x00ff));

    /* set the amplitude of the channel Q */
    AD9854_WR_Byte(0x23, Shape >> 8);
    AD9854_WR_Byte(0x24, (uint8_t)(Shape & 0x00ff));

    /* update the output */
    AD9854_UDCLK_SET;
    AD9854_UDCLK_CLR;

}

/**
  * @brief AD9854 FSK initialise
  * @param None
  * @retval None
  */
void AD9854_FSK_Init(void)
{
	AD9854_WR_Byte(0x1d,0x10);	     	// Close the comparator
	AD9854_WR_Byte(0x1e,CLK_Set);	   	// Set the system clock doubling frequency
	AD9854_WR_Byte(0x1f,0x02);	     	// Set the system to mode 0, updated by externally
	AD9854_WR_Byte(0x20,0x60);	    	// Set to the adjustable amplitude, 
                                        // and cancel the interpolation compensation
}

/**
  * @brief generate the FSK signal
  * @param Freq1: FSK frequence 1
  * @param Freq2：FSK frequence 2
  * @retval None
  */
void AD9854_SetFSK(uint32_t Freq1, uint32_t Freq2)
{
    uint32_t count = 0;
    uint32_t Address1, Address2;

    const uint16_t Shape = 4000; // set the amplitude,12-bit,0~4095

    Address1 = 0x04; // chose the original value of address 1
    Address2 = 0x0a; // chose the original value of address 2

    Freq_Convert(Freq1);    // change frequence 1
	
    /* write 6-bit frequence-control */
	for(count=6;count>0;)
    {
		AD9854_WR_Byte(Address1++,FreqWord[--count]);
    }
	
	Freq_Convert(Freq2);    // change frequence 2

    /* write 6-bit frequence-control */
	for(count=6;count>0;)
    {
		AD9854_WR_Byte(Address2++,FreqWord[--count]);
    }

    /* set the amplitude of the channel I */
    AD9854_WR_Byte(0x21, Shape >> 8);
    AD9854_WR_Byte(0x22, (uint8_t)(Shape & 0xff));

    /* set the amplitude of the channel Q */
    AD9854_WR_Byte(0x23, Shape >> 8);
    AD9854_WR_Byte(0x24, (uint8_t)(Shape & 0xff));

    /* update the output */
    AD9854_UDCLK_SET;
    AD9854_UDCLK_CLR;		
}

/**
  * @brief AD9854 BPSK initialise
  * @param None
  * @retval None
  */
void AD9854_BPSK_Init(void)
{
    AD9854_WR_Byte(0x1d, 0x10);         // Close the comparator
    AD9854_WR_Byte(0x1e, CLK_Set);      // Set the system clock doubling frequency
    AD9854_WR_Byte(0x1f, 0x08);         // Set the system to mode 4, updated by externally
    AD9854_WR_Byte(0x20, 0x60);         // Set to the adjustable amplitude,
                                        // and cancel the interpolation compensation
}

/**
  * @brief generate the BPSK signal
  * @param Phase1: modulation phase 1
  * @param Phase2：modulation phase 2
  * @retval None
  * @note the phase is 16-bit, 0~16383.
  *       0 - 0 degree
  *       8192 - 180 degree
  */
void AD9854_SetBPSK(uint16_t Phase1,uint16_t Phase2)
{
    uint8_t count = 0;
    uint8_t Address;
    const uint32_t Freq = 60000;
    const uint16_t Shape = 4000; // set the amplitude,12-bit,0~4095

    Address = 0x04; // chose the original value of address

    /* set the phase 1 */
    AD9854_WR_Byte(0x00, Phase1 >> 8);
    AD9854_WR_Byte(0x01, (uint8_t)(Phase1 & 0xff));

    /* set the phase 2 */
    AD9854_WR_Byte(0x02, Phase2 >> 8);
    AD9854_WR_Byte(0x03, (uint8_t)(Phase2 & 0xff));

    Freq_Convert(Freq);    // change frequence

    /* write 6-bit frequence-control */
	for(count=6;count>0;)  
    {
		AD9854_WR_Byte(Address++,FreqWord[--count]);
    }

    /* set the amplitude of the channel I */
    AD9854_WR_Byte(0x21, Shape >> 8);
    AD9854_WR_Byte(0x22, (uint8_t)(Shape & 0xff));

    /* set the amplitude of the channel Q */
    AD9854_WR_Byte(0x23, Shape >> 8);
    AD9854_WR_Byte(0x24, (uint8_t)(Shape & 0xff));

    /* update the output */
    AD9854_UDCLK_SET;   
    AD9854_UDCLK_CLR;		
}

/**
  * @brief AD9854 OSK initialise
  * @param None
  * @retval None
  */
void AD9854_OSK_Init(void)
{
    AD9854_WR_Byte(0x1d, 0x10);         // Close the comparator
    AD9854_WR_Byte(0x1e, CLK_Set);      // Set the system clock doubling frequency
    AD9854_WR_Byte(0x1f, 0x00);         // Set the system to mode 0, updated by externally
    AD9854_WR_Byte(0x20, 0x70);         // Set to the adjustable amplitude,
                                        // and cancel the interpolation compensation
}

/**
  * @brief generate the OSK signal
  * @param RateShape: the slope of the OSK, 4~255
  * @retval None
  */
void AD9854_SetOSK(uint8_t RateShape)
{
    uint8_t count = 0;
    uint8_t Address;
    const uint32_t Freq = 60000;
    const uint16_t Shape = 4000; // set the amplitude,12-bit,0~4095

    Address = 0x04; // chose the original value of address

    Freq_Convert(Freq);    // change frequence

    /* write 6-bit frequence-control */
	for(count=6;count>0;) 
    {
		AD9854_WR_Byte(Address++,FreqWord[--count]);
    }

    /* set the amplitude of the channel I */
    AD9854_WR_Byte(0x21, Shape >> 8);
    AD9854_WR_Byte(0x22, (uint8_t)(Shape & 0xff));

    /* set the amplitude of the channel Q */
    AD9854_WR_Byte(0x23, Shape >> 8);
    AD9854_WR_Byte(0x24, (uint8_t)(Shape & 0xff));

    AD9854_WR_Byte(0x25,RateShape); // set OSK slope 

    /* update the output */
    AD9854_UDCLK_SET;
    AD9854_UDCLK_CLR;		
}

/**
  * @brief AD9854 AM initialise
  * @param None
  * @retval None
  */
void AD9854_AM_Init(void)
{
    AD9854_WR_Byte(0x1d, 0x10);         // Close the comparator
    AD9854_WR_Byte(0x1e, CLK_Set);      // Set the system clock doubling frequency
    AD9854_WR_Byte(0x1f, 0x01);         // Set the system to mode 0, updated by externally
    AD9854_WR_Byte(0x20, 0x60);         // Set to the adjustable amplitude,
                                        // and cancel the interpolation compensation
}

/**
  * @brief generate the AM signal
  * @param Freq： the frequence of signal, 0~(1/2)*SYSCLK
  * @param Shape： the amplitude of the signal, 0~4095
  * @retval None
  */
void AD9854_SetAM(uint32_t Freq, uint16_t Shape)
{
    uint8_t count = 0;
    uint8_t Address;

    Address = 0x04; // chose the original value of address

    Freq_Convert(Freq);    // change frequence

    /* write 6-bit frequence-control */
	for(count=6;count>0;)
    {
		AD9854_WR_Byte(Address++,FreqWord[--count]);
    }

    /* set the amplitude of the channel I */
    AD9854_WR_Byte(0x21, Shape >> 8); 
    AD9854_WR_Byte(0x22, (uint8_t)(Shape & 0xff));

    /* set the amplitude of the channel Q */
    AD9854_WR_Byte(0x23, Shape >> 8); 
    AD9854_WR_Byte(0x24, (uint8_t)(Shape & 0xff));

    /* update the output */
    AD9854_UDCLK_SET;
    AD9854_UDCLK_CLR;		
}

/**
  * @brief set momodulation frequency and modulation system of AM
  * @param fund_fre: momodulation frequency
  * @param modulation: modulation system
  * @retval None
  */
void AM_Test (uint32_t fund_fre, uint8_t modulation)
{
	modu_buf = modulation;
	fund_fre_buf = fund_fre;	
	step_up = (float)fund_fre_buf * 65535 / 10000;
}

/**
  * @brief AD9854 RFSK initialise
  * @param None
  * @retval None
  */
void AD9854_RFSK_Init(void)
{
    AD9854_WR_Byte(0x1d, 0x10);         // Close the comparator
    AD9854_WR_Byte(0x1e, CLK_Set);      // Set the system clock doubling frequency
    AD9854_WR_Byte(0x1f, 0x24);         // Set the system to mode 2, updated by externally
                                        // enable triangle wave frequency sweep function 
    AD9854_WR_Byte(0x20, 0x60);         // Set to the adjustable amplitude,
                                        // and cancel the interpolation compensation
}

/**
  * @brief generate the RFSK signal
  * @param FreqLow： the low frequence of RFSK
  * @param FreqHigh： the high frequence of RFSK
  * @param FreqUpDown： the step frequence of RFSK
  * @param FreRate the ramp rate
  * @retval None
  */
void AD9854_SetRFSK(uint32_t FreqLow, uint32_t FreqHigh, uint32_t FreqUpDown, uint32_t FreRate)
{
    uint8_t count = 0;
    uint8_t Address1, Address2, Address3;
    uint16_t Shape = 3600;

    /* chose the original value of address */
    Address1 = 0x04; 
    Address2 = 0x0a;
    Address3 = 0x10;

    Freq_Convert(FreqLow); // change frequence

    /* write 6-bit frequence-control */
	for(count=6;count>0;)
    {
		AD9854_WR_Byte(Address1++,FreqWord[--count]);
    }

    Freq_Convert(FreqHigh);    // change frequence

    /* write 6-bit frequence-control */
	for(count=6;count>0;)
    {
		AD9854_WR_Byte(Address2++,FreqWord[--count]);
    }

    Freq_Convert(FreqUpDown);    // change frequence

    /* write 6-bit frequence-control */
	for(count=6;count>0;)
    {
		AD9854_WR_Byte(Address3++,FreqWord[--count]);
    }

    AD9854_WR_Byte(0x1a, (uint8_t)((FreRate >> 16) & 0x0f)); // Set the ramp rate
    AD9854_WR_Byte(0x1b, (uint8_t)(FreRate >> 8));
    AD9854_WR_Byte(0x1c, (uint8_t)FreRate);

    /* set the amplitude of the channel I */
    AD9854_WR_Byte(0x21, Shape >> 8);
    AD9854_WR_Byte(0x22, (uint8_t)(Shape & 0xff));

    /* set the amplitude of the channel Q */
    AD9854_WR_Byte(0x23, Shape >> 8);
    AD9854_WR_Byte(0x24, (uint8_t)(Shape & 0xff));

    /* update the output */
    AD9854_UDCLK_SET;
    AD9854_UDCLK_CLR;
}

/**
  * @brief AD9854 Chirp initialise
  * @param None
  * @retval None
  */
void AD9854_Chirp_Init(void)
{
    AD9854_WR_Byte(0x1d, 0x10);         // Close the comparator
    AD9854_WR_Byte(0x1e, CLK_Set);      // Set the system clock doubling frequency
    AD9854_WR_Byte(0x1f, 0x26);         // Set the system to mode 2, updated by externally
                                        // enable triangle wave frequency sweep function 
    AD9854_WR_Byte(0x20, 0x60);         // Set to the adjustable amplitude,
                                        // and cancel the interpolation compensation
}

/**
  * @brief generate the chirp signal
  * @param FreqLow： the low frequence of chirp
  * @param FreqUpDown： the step frequence of chirp
  * @param FreRate： the ramp rate
  * @retval None
  */
void AD9854_SetChirp(uint32_t FreqLow, uint32_t FreqUpDown, uint32_t FreRate)
{
    uint8_t count = 0;
    uint8_t Address1, Address3;
    const uint16_t Shape = 4000;

    /* chose the original value of address */
    Address1 = 0x04; 
    Address3 = 0x10;

    Freq_Convert(FreqLow);    // change frequence

    /* write 6-bit frequence-control */
	for(count=6;count>0;)   
    {
		AD9854_WR_Byte(Address1++,FreqWord[--count]);
    }

    Freq_Convert(FreqUpDown);    // change frequence

    /* write 6-bit frequence-control */
	for(count=6;count>0;) 
    {
		AD9854_WR_Byte(Address3++,FreqWord[--count]);
    }

    AD9854_WR_Byte(0x1a, (uint8_t)((FreRate >> 16) & 0x0f)); // Set the ramp rate
    AD9854_WR_Byte(0x1b, (uint8_t)(FreRate >> 8));
    AD9854_WR_Byte(0x1c, (uint8_t)FreRate);

    /* set the amplitude of the channel I */
    AD9854_WR_Byte(0x21, Shape >> 8); 
    AD9854_WR_Byte(0x22, (uint8_t)(Shape & 0xff));

    /* set the amplitude of the channel Q */
    AD9854_WR_Byte(0x23, Shape >> 8);
    AD9854_WR_Byte(0x24, (uint8_t)(Shape & 0xff));

    /* update the output */
    AD9854_UDCLK_SET;  
    AD9854_UDCLK_CLR;		
}

/**
  * @}
  */ 

/** @defgroup SPI control
  * @{
  */ 

/**
  * @brief AD9864 SPI port writes the data
  * @param Adata: the data or the address which would be written
  * @retval None
  */ 
void AD9854_SPI_WR_Byte(uint8_t Adata)
{
    uint8_t i;

    for(i = 8; i > 0; i --)
	{
		if(Adata & 0x80)
        {
            SPI_SDI_Set;
        }
        else
        {
			SPI_SDI_Clr;
        }
		Adata <<= 1;	
		AD9854_WR_CLR;
		AD9854_WR_SET;
	}
}

/**
  * @brief AD9854 initialise
  * @param None
  * @retval None
  */
void AD9854_SPI_Init(void)
{
	AD9854_SP_CLR;		// serial control
	AD9854_WR_CLR;
	AD9854_UDCLK_CLR;
	AD9854_RST_SET;    // reset AD9854
	delay_us (100);
	AD9854_RST_CLR;
	SPI_IO_RST_CLR;
	AD9854_RD_CLR;
	
    AD9854_SPI_WR_Byte(CONTR);
// 	AD9854_SPI_WR_Byte(0x10);   // close the comparator
	AD9854_SPI_WR_Byte(0x00);   // Start the comparator

	AD9854_SPI_WR_Byte(CLK_Set);    // Set the system clock doubling frequency  
	AD9854_SPI_WR_Byte(0x00);	// Set the system to mode 0, updated by externally 
	AD9854_SPI_WR_Byte(0x60);	

    /* update the output */
	AD9854_UDCLK_SET;   
	AD9854_UDCLK_CLR;	
}

/**
  * @brief sine generate
  * @param Freq： the frequence of signal, 0~(1/2)*SYSCLK
  * @param Shape： the amplitude of the signal, 0~4095
  * @retval None
  * @note The function should be used when you choose serial control.
  *       And if you want to use the function @ref Freq_Convert(long Freq) 
  *       you must use this function to generate the sinusoidal signal.
  */
void AD9854_SPI_SetSine(uint32_t Freq,uint32_t Shape)
{
	uint8_t count;
    uint8_t i = 0;

    Freq_Convert(Freq); // change frequence

    /* write 6-bit frequence-control */
    for(count=6;count>0;)
    {
		if(i==0)
			AD9854_SPI_WR_Byte(FREQ1);
		AD9854_SPI_WR_Byte(FreqWord[--count]);
		i++;
    }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
	/* set the amplitude of the channel I */
	AD9854_SPI_WR_Byte(SHAPEI);
	AD9854_SPI_WR_Byte(Shape>>8);
	AD9854_SPI_WR_Byte((u8)(Shape&0xff));
	
    /* set the amplitude of the channel I */
	AD9854_SPI_WR_Byte(SHAPEQ);
	AD9854_SPI_WR_Byte(Shape>>8);
	AD9854_SPI_WR_Byte((u8)(Shape&0xff));
	
    /* update the output */
	AD9854_UDCLK_SET;
	AD9854_UDCLK_CLR;
}

/**
  * @}
  */ 

/**
  * @}
  */ 

/**********************************END OF FILE*****************************************/

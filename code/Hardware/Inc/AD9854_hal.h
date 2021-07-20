/**
  **************************************************************************************
  * @file AD9854.H
  * @author shaokairu
  * @date 2021.7.19
  * @brief This file contains all the functions prototypes for AD9854 board.
  **************************************************************************************
  */

/* Define to prevent recursive inclusion ---------------------------------------------*/
#ifndef __AD9854_HAL_H
#define __AD9854_HAL_H

/* Includes --------------------------------------------------------------------------*/
#include "sys.h"

/* Exported Defines ------------------------------------------------------------------*/

/** @defgroup control line
  * @brief ad9854 I/O control line
  * @{
  */

#define AD9854_RD_SET (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_8, GPIO_PIN_SET))
#define AD9854_RD_CLR (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_8, GPIO_PIN_RESET))

#define AD9854_WR_SET (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_9, GPIO_PIN_SET))
#define AD9854_WR_CLR (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_9, GPIO_PIN_RESET))

#define AD9854_UDCLK_SET (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_10, GPIO_PIN_SET))
#define AD9854_UDCLK_CLR (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_10, GPIO_PIN_RESET))

#define AD9854_RST_SET (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_11, GPIO_PIN_SET))
#define AD9854_RST_CLR (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_11, GPIO_PIN_RESET))

#define AD9854_SP_SET (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_12, GPIO_PIN_SET))
#define AD9854_SP_CLR (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_12, GPIO_PIN_RESET))

#define AD9854_OSK_SET (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, GPIO_PIN_SET))
#define AD9854_OSK_CLR (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, GPIO_PIN_RESET))

#define AD9854_FDATA_SET (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_14, GPIO_PIN_SET))
#define AD9854_FDATA_CLR (HAL_GPIO_WritePin( GPIOC, GPIO_PIN_14, GPIO_PIN_RESET))

/**
  * @}
  */

/** @defgroup ad9854 SPI I/O
  * @{
  */

#define SPI_IO_RST_SET (HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, GPIO_PIN_SET))
#define SPI_IO_RST_CLR (HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, GPIO_PIN_RESET))
#define SPI_SDO_Set (HAL_GPIO_WritePin( GPIOA, GPIO_PIN_1, GPIO_PIN_SET))
#define SPI_SDO_Clr (HAL_GPIO_WritePin( GPIOA, GPIO_PIN_1, GPIO_PIN_RESET))
#define SPI_SDI_Set (HAL_GPIO_WritePin( GPIOA, GPIO_PIN_0, GPIO_PIN_SET))
#define SPI_SDI_Clr (HAL_GPIO_WritePin( GPIOA, GPIO_PIN_0, GPIO_PIN_RESET))

/**
  * @}
  */

/** @defgroup ad9854 register address
  * @note used for SPI
  * @{
  */

#define PHASE1	0x00    //phase adjust register #1
#define PHASE2  0x01	//phase adjust register #2
#define FREQ1   0x02	//frequency tuning word 1
#define FREQ2   0x03	//frequency tuning word 2
#define DELFQ   0x04	//delta frequency word
#define UPDCK   0x05	//update clock
#define RAMPF   0x06	//ramp rate clock
#define CONTR   0x07	//control register
#define SHAPEI  0x08	//output shape key I mult
#define SHAPEQ  0x09	//output shape key Q mult 
#define RAMPO   0x0A	//output shape key ramp rate
#define CDAC    0x0B	//QDAC

/**
  * @}
  */

/* Exported variables ----------------------------------------------------------------*/
extern uint16_t add;
extern uint16_t step_up;
extern uint16_t amp_buf;
extern uint8_t modu_buf;

/* Exported functions prototypes -----------------------------------------------------*/
void AD9854_GPIO_Init(void);
void AD9854_WR_Byte(uint8_t addr, uint8_t dat);
void AD9854_Init(void);
void Freq_Convert(long Freq);
void Freq_Double_Convert(double Freq);
void AD9854_SetSine(uint32_t Freq, uint16_t Shape);
void AD9854_SetSine_Double(double Freq, uint16_t Shape);
void AD9854_FSK_Init(void);
void AD9854_SetFSK(uint32_t Freq1, uint32_t Freq2);
void AD9854_BPSK_Init(void);
void AD9854_SetBPSK(uint16_t Phase1, uint16_t Phase2);
void AD9854_OSK_Init(void);
void AD9854_SetOSK(uint8_t RateShape);
void AD9854_AM_Init(void);
void AD9854_SetAM(uint32_t Freq, uint16_t Shape);
void AD9854_RFSK_Init(void);
void AD9854_SetRFSK(uint32_t FreqLow, uint32_t FreqHigh, uint32_t FreqUpDown, uint32_t FreRate);
void AD9854_Chirp_Init(void);
void AD9854_SetChirp(uint32_t FreqLow, uint32_t FreqUpDown, uint32_t FreRate);
void AM_Test(uint32_t fund_fre, uint8_t modulation);
void AD9854_SPI_WR_Byte(uint8_t Adata);
void AD9854_SPI_Init(void);
void AD9854_SPI_SetSine(uint32_t Freq, uint32_t Shape);

#endif

/**********************************END OF FILE*****************************************/

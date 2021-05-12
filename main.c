/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include "deca_device_api.h"
#include "deca_regs.h"
#include <stdio.h>
#include "dw1000.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//extern int example_application_entry(void);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* SD卡的全局变量 ---------------------------------------------------------*/
FATFS   fs;            /* FATFS 文件系统对象 */
FRESULT fr;         /* FATFS API 返回值*/
FIL     fd;         /* FATFS 文件对象    */
uint8 read_dat[253];//传输数据定义为全局变量，导入DW1000的传输函数


/*DW1000射频芯片建模的全局变量 ---------------------------------------------------------*/
/* Example application name and version to display on LCD screen. */
#define APP_NAME "SIMPLE TX v1.2"

/* Default communication configuration. We use here EVK1000's default mode (mode 3). */
static dwt_config_t config = {
    5,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_128  ,   /* Preamble length. Used in TX only. */
    DWT_PAC8,       /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    0,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_6M8,     /* Data rate. */
    DWT_PHRMODE_EXT, /* PHY header mode. */
    (129 + 8 - 8)  /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};

/* The frame sent in this example is an 802.15.4e standard blink. It is a 12-byte frame composed of the following fields:
 *     - byte 0: frame type (0xC5 for a blink).
 *     - byte 1: sequence number, incremented for each new frame.
 *     - byte 2 -> 9: device ID, see NOTE 1 below.
 *     - byte 10/11: frame check-sum, automatically set by DW1000.  */
 uint8 tx_msg[255] = {1,1,1,1,1,1,1,1,1,1,0,0};//后两位扔两个空指针，保证CRC的校验函数
	/* Index to access to sequence number of the blink frame in the tx_msg array. */
#define BLINK_FRAME_SN_IDX 1

/* Inter-frame delay period, in milliseconds. */
#define TX_DELAY_MS 10
 /*DW1000射频芯片建模的全局变量结束 ---------------------------------------------------------*/
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
/* USER CODE BEGIN 1 */
//操作的文件名
char filename[] = "test.txt";
//写入的内容
	 /* 写入时候用，注释掉--------------------------------------------------------*/
//uint8_t write_dat[] = "Hello,FATFS!\n";
//返回API接收的字符数
//uint16_t write_num = 0;
	//从文件中读取的内容

//接收API返回的字节数
uint16_t read_num = 0;
/* USER CODE END 1 */
int i,j; 

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
//printf("DW1000 UWB ic port on BearPi board By LIBOYU\r\n");

//SD卡的内容开始，流程是挂载SD卡--打开--读取or写入--关闭
printf("FATFS test...\r\n");
/* 挂载SD*/
fr = f_mount(&fs, "", 0);
if(fr == FR_OK)
{
    printf("SD card mount ok!\r\n");
}
else
{
    printf("SD card mount error, error code:%d.\r\n",fr);
}
/* DW1000射频芯片的基础配置方法--------------------------------------------------------*/
   /* Reset and initialise DW1000. See NOTE 2 below.
     * For initialisation, DW1000 clocks must be temporarily set to crystal speed. After initialisation SPI rate can be increased for optimum
     * performance. */
    reset_DW1000(); /* Target specific drive of RSTn line into DW1000 low for a period. */
    spi_set_rate_low();
    if (dwt_initialise(DWT_LOADNONE) == DWT_ERROR)
    {
        printf("INIT FAILED\r\n");
        while (1)
        { };
    }
		 printf("INIT SUCCESSED\r\n");
    spi_set_rate_high();
	
    /* Configure DW1000. See NOTE 3 below. */
    dwt_configure(&config);
		
		
		
///*打开文件用来写入，读取时候注释掉 */
//fr = f_open(&fd, filename, FA_CREATE_ALWAYS | FA_WRITE);
//if(fr == FR_OK)
//{
//    printf("open file \"%s\" ok! \r\n", filename);
//}
//else
//{
//printf("open file \"%s\" error : %d\r\n", filename, fr);
//}

///* 在文件中写入内容，读取的时候可以注释掉，留着不删方便修改 */
//fr = f_write(&fd, write_dat, sizeof(write_dat), (void *)&write_num);
//if(fr == FR_OK)
//{
//    printf("write %d dat to file \"%s\" ok,dat is \"%s\".\r\n", write_num, filename, write_dat);
//}
//else
//{
//    printf("write dat to file \"%s\" error,error code is:%d\r\n", filename, fr);
//}

/* 从打开的文件中读取内容*/
fr = f_open(&fd, filename, FA_READ);//打开文件用于读取
if(fr == FR_OK)
{
    printf("open file \"%s\" ok! \r\n", filename);
}
else
{
printf("open file \"%s\" error : %d\r\n", filename, fr);
}

//while(1){									 //??f_read???
//		fr = f_read(&fd, buffer, 1, &br);     //?????????????????
// 
//		if (fr == FR_OK )
//		{
//			printf("%s",buffer);
//		}else{
//		    printf("\r\n f_read() fail .. \r\n");	
//		}
// 
//		if(f_eof(&fd)) {break;}
//	}
while(1)
{
fr = f_read(&fd, read_dat, 1, (void *)&read_num);
if(fr == FR_OK)
{  
	for(i=1;i<253;i++)
	{
		tx_msg[i]=read_dat[i];
	}
            /* Write frame data to DW1000 and prepare transmission. See NOTE 4 below.*/
        dwt_writetxdata(sizeof(tx_msg), tx_msg, 0); /* Zero offset in TX buffer. */
        dwt_writetxfctrl(sizeof(tx_msg), 0, 0); /* Zero offset in TX buffer, no ranging. */

        /* Start transmission. */
        dwt_starttx(DWT_START_TX_IMMEDIATE);

        /* Poll DW1000 until TX frame sent event set. See NOTE 5 below.
         * STATUS register is 5 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
         * function to access it.*/
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
        { };
	      
				 for(j=2;j<=253;j++)
				{
					printf("%d",read_dat[i]);
				}
				printf("successed send");
				
				 
        /* Clear TX frame sent event. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

        /* Execute a delay between transmissions. */
        HAL_Delay(TX_DELAY_MS);

       /* Increment the blink frame sequence number (modulo 256). */
       tx_msg[BLINK_FRAME_SN_IDX]++;
}
else
{
    printf("read dat to file \"%s\" error,error code is:%d\r\n", filename, fr);
}
if(f_eof(&fd)) {break;}
}
/* 关闭文件 */
fr = f_close(&fd);
if(fr == FR_OK)
{
    printf("close file \"%s\" ok!\r\n", filename);
}
else
{
    printf("close file \"%s\" error, error code is:%d.\r\n", filename, fr);
}


//超宽带的传输函数，测试CD卡的时候暂时注释掉
//example_application_entry();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
 
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

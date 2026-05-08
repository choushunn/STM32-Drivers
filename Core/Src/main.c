/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "dht11.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DHT11_PIN       12
#define DHT11_PORT      GPIOB
#define DHT11_CRH_SHIFT ((DHT11_PIN - 8) * 4)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static DHT11_Data_t dht11_data;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, buf, 2, 100);
}

static void OLED_WriteData(const uint8_t *data, uint16_t len)
{
    uint8_t buf[129];
    buf[0] = 0x40;
    memcpy(buf + 1, data, len);
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, buf, len + 1, 200);
}

static OLED_IO_t oled_io = {
    .write_cmd = OLED_WriteCmd,
    .write_data = OLED_WriteData,
};

static void DHT11_SetOutput(void)
{
    DHT11_PORT->CRH &= ~(0xF << DHT11_CRH_SHIFT);
    DHT11_PORT->CRH |= (0x7 << DHT11_CRH_SHIFT);
}

static void DHT11_SetInput(void)
{
    DHT11_PORT->CRH &= ~(0xF << DHT11_CRH_SHIFT);
    DHT11_PORT->CRH |= (0x8 << DHT11_CRH_SHIFT);
    DHT11_PORT->ODR |= (1U << DHT11_PIN);
}

static void DHT11_WriteLow(void)
{
    DHT11_PORT->BSRR = (1U << (DHT11_PIN + 16));
}

static void DHT11_WriteHigh(void)
{
    DHT11_PORT->BSRR = (1U << DHT11_PIN);
}

static uint8_t DHT11_ReadPin(void)
{
    return (DHT11_PORT->IDR >> DHT11_PIN) & 1;
}

static void DHT11_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < us * 72);
}

static void DHT11_DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

static DHT11_IO_t dht11_io = {
    .set_output = DHT11_SetOutput,
    .set_input = DHT11_SetInput,
    .write_low = DHT11_WriteLow,
    .write_high = DHT11_WriteHigh,
    .read_pin = DHT11_ReadPin,
    .delay_us = DHT11_DelayUs,
    .delay_ms = DHT11_DelayMs,
};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  OLED_Init(&oled_io);
  __HAL_RCC_GPIOB_CLK_ENABLE();
  DHT11_Init(&dht11_io);

  HAL_Delay(1000);
  DHT11_ReadData(&dht11_data);
  HAL_Delay(100);

  OLED_ShowString(12, 0, "DHT11 Sensor", 1);
  OLED_ShowString(16, 20, "Temp: --.- C", 1);
  OLED_ShowString(16, 36, "Humi: --.- %", 1);
  OLED_ShowString(28, 52, "Reading...", 1);
  OLED_Display();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t last_dht11 = 0;
  while (1)
  {
    uint32_t now = HAL_GetTick();

    if (now - last_dht11 >= 2000)
    {
        last_dht11 = now;
        char buf[30];

        int8_t dht_ret = DHT11_ReadData(&dht11_data);
        if (dht_ret == DHT11_OK)
        {
            int16_t t = dht11_data.temperature;
            uint16_t h = dht11_data.humidity;

            uint8_t sign = ' ';
            if (t < 0) { t = -t; sign = '-'; }

            buf[0] = 'T'; buf[1] = 'e'; buf[2] = 'm'; buf[3] = 'p'; buf[4] = ':';
            buf[5] = ' '; buf[6] = sign;
            buf[7] = '0' + (t / 100) % 10;
            buf[8] = '0' + (t / 10) % 10;
            buf[9] = '.';
            buf[10] = '0' + t % 10;
            buf[11] = ' '; buf[12] = 'C'; buf[13] = '\0';
            OLED_ShowString(16, 20, buf, 1);

            buf[0] = 'H'; buf[1] = 'u'; buf[2] = 'm'; buf[3] = 'i'; buf[4] = ':';
            buf[5] = ' '; buf[6] = ' ';
            buf[7] = '0' + (h / 100) % 10;
            buf[8] = '0' + (h / 10) % 10;
            buf[9] = '.';
            buf[10] = '0' + h % 10;
            buf[11] = ' '; buf[12] = '%'; buf[13] = '\0';
            OLED_ShowString(16, 36, buf, 1);

            OLED_ShowString(28, 52, "OK        ", 1);
        }
        else
        {
            OLED_ShowString(16, 20, "Temp: --.- C", 1);
            OLED_ShowString(16, 36, "Humi: --.- %", 1);

            buf[0] = 'E'; buf[1] = '-';
            uint8_t ec = (uint8_t)(-dht_ret);
            buf[2] = '0' + ec;
            buf[3] = ' ';
            uint8_t p = 4;
            for (uint8_t i = 0; i < 5; i++)
            {
                uint8_t b = dht11_data.raw[i];
                uint8_t hi = b >> 4;
                uint8_t lo = b & 0x0F;
                buf[p++] = hi < 10 ? '0' + hi : 'A' + hi - 10;
                buf[p++] = lo < 10 ? '0' + lo : 'A' + lo - 10;
                if (i < 4) buf[p++] = ':';
            }
            buf[p] = '\0';
            OLED_ShowString(8, 52, buf, 1);
        }

        OLED_Display();
    }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
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

  /** Initializes the CPU, AHB and APB buses clocks
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

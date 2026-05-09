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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "aht20_bmp280.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

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


/* ---- AHT20+BMP280 IO Callbacks ---- */
static int8_t SENSOR_Read(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    if (reg != 0xFF)
    {
        if (HAL_I2C_Mem_Read(&hi2c2, dev_addr << 1, reg, I2C_MEMADD_SIZE_8BIT, data, len, 100) != HAL_OK)
            return AHT20_BMP280_ERR_I2C;
    }
    else
    {
        if (HAL_I2C_Master_Receive(&hi2c2, dev_addr << 1, data, len, 100) != HAL_OK)
            return AHT20_BMP280_ERR_I2C;
    }
    return AHT20_BMP280_OK;
}

static int8_t SENSOR_Write(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    if (reg != 0xFF)
    {
        uint8_t buf[9];
        buf[0] = reg;
        for (uint16_t i = 0; i < len; i++)
            buf[1 + i] = data[i];
        if (HAL_I2C_Master_Transmit(&hi2c2, dev_addr << 1, buf, len + 1, 100) != HAL_OK)
            return AHT20_BMP280_ERR_I2C;
    }
    else
    {
        if (HAL_I2C_Master_Transmit(&hi2c2, dev_addr << 1, data, len, 100) != HAL_OK)
            return AHT20_BMP280_ERR_I2C;
    }
    return AHT20_BMP280_OK;
}

static void SENSOR_Delay(uint32_t ms)
{
    HAL_Delay(ms);
}

static AHT20_BMP280_IO_t sensor_io = {
    .read = SENSOR_Read,
    .write = SENSOR_Write,
    .delay_ms = SENSOR_Delay,
};

/* ---- OLED 显示温湿度气压 ---- */
static void SENSOR_Display(AHT20_BMP280_Data_t *data)
{
    OLED_Clear();

    OLED_ShowString(22, 0, "AHT20+BMP280", 1);

    if (data->aht20_ok)
    {
        OLED_ShowString(0, 10, "T:", 1);
        OLED_ShowFloat(12, 10, data->temp, 2, 1, 1);
        OLED_ShowString(40, 10, "C", 1);

        OLED_ShowString(54, 10, "H:", 1);
        OLED_ShowFloat(66, 10, data->humidity, 2, 1, 1);
        OLED_ShowString(94, 10, "%", 1);
    }
    else
    {
        OLED_ShowString(0, 10, "T:--.-C  H:--.-%", 1);
    }

    if (data->bmp280_ok)
    {
        OLED_ShowString(0, 24, "P:", 1);
        OLED_ShowFloat(12, 24, data->pressure, 4, 1, 1);
        OLED_ShowString(60, 24, "hPa", 1);
    }
    else
    {
        OLED_ShowString(0, 24, "P:----.-hPa", 1);
    }

    if (data->aht20_ok || data->bmp280_ok)
        OLED_ShowString(58, 38, "OK", 1);
    else
        OLED_ShowString(58, 38, "FAIL", 1);

    OLED_Display();
}
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
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim3);

  OLED_InitEx(&oled_io, OLED_CONTROLLER_SH1106);
  OLED_TestPattern();
  HAL_Delay(2000);
  OLED_Clear();
  OLED_ShowString(8, 20, "AHT20+BMP280...", 2);
  OLED_Display();
  AHT20_BMP280_Init(&sensor_io);
  HAL_Delay(500);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t last_sensor_tick = 0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (HAL_GetTick() - last_sensor_tick >= 1000)
    {
        AHT20_BMP280_Data_t data;
        last_sensor_tick = HAL_GetTick();
        AHT20_BMP280_Read(&data);
        SENSOR_Display(&data);
    }
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

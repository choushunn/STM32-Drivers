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
#include "hcsr04.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define HCSR04_TRIG_PORT   GPIOA
#define HCSR04_TRIG_PIN    GPIO_PIN_7
#define HCSR04_ECHO_PORT   GPIOA
#define HCSR04_ECHO_PIN    GPIO_PIN_6
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint32_t hcsr04_dist_mm;
static uint16_t hcsr04_last_cnt;
static uint32_t hcsr04_overflow_cnt;
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

static void HCSR04_Trig(void)
{
    HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_SET);
    uint32_t _t = TIM3->CNT;
    while ((TIM3->CNT - _t) < 15);
    HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_RESET);
}

static uint8_t HCSR04_ReadEcho(void)
{
    return (HAL_GPIO_ReadPin(HCSR04_ECHO_PORT, HCSR04_ECHO_PIN) == GPIO_PIN_SET) ? 1 : 0;
}

static uint32_t HCSR04_GetUs(void)
{
    uint16_t cnt = TIM3->CNT;
    if (cnt < hcsr04_last_cnt)
        hcsr04_overflow_cnt++;
    hcsr04_last_cnt = cnt;
    return (hcsr04_overflow_cnt << 16) | cnt;
}

static HCSR04_IO_t hcsr04_io = {
    .trig = HCSR04_Trig,
    .read_echo = HCSR04_ReadEcho,
    .get_us = HCSR04_GetUs,
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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim3);

  OLED_Init(&oled_io);

  HCSR04_Init(&hcsr04_io);

  OLED_ShowString(16, 0, "Ultrasonic", 2);
  OLED_ShowString(0, 24, "C:----- P:--- E:-", 1);
  OLED_Display();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t last_hcsr04 = 0;
  while (1)
  {
    uint32_t now = HAL_GetTick();

    if (now - last_hcsr04 >= 200)
    {
        last_hcsr04 = now;
        char buf[30];

        int8_t hc_ret = HCSR04_Read(&hcsr04_dist_mm);

        uint32_t cnt = TIM3->CNT;
        buf[0] = 'C'; buf[1] = ':';
        buf[2] = '0' + (cnt / 10000) % 10;
        buf[3] = '0' + (cnt / 1000) % 10;
        buf[4] = '0' + (cnt / 100) % 10;
        buf[5] = '0' + (cnt / 10) % 10;
        buf[6] = '0' + cnt % 10;
        buf[7] = ' ';

        if (hc_ret == HCSR04_OK)
        {
            uint32_t m = hcsr04_dist_mm;
            uint32_t cm = m / 10;
            uint32_t mm = m % 10;
            buf[8] = 'D'; buf[9] = ':';

            if (cm >= 100) { buf[10] = '0' + cm / 100; cm %= 100; }
            else           { buf[10] = ' '; }
            if (cm >= 10 || buf[10] != ' ') { buf[11] = '0' + cm / 10; cm %= 10; }
            else                           { buf[11] = ' '; }
            buf[12] = '0' + cm;
            buf[13] = '.';
            buf[14] = '0' + mm;
            buf[15] = 'c'; buf[16] = 'm'; buf[17] = '\0';
            OLED_ShowString(0, 24, buf, 1);
        }
        else
        {
            uint8_t p = 8;
            buf[p++] = 'P'; buf[p++] = ':';
            uint32_t pv = HCSR04_GetPulseUs();
            buf[p++] = '0' + (pv / 100) % 10;
            buf[p++] = '0' + (pv / 10) % 10;
            buf[p++] = '0' + pv % 10;
            buf[p++] = ' ';
            buf[p++] = 'E'; buf[p++] = ':';
            buf[p++] = (uint8_t)(-hc_ret) + '0';
            buf[p++] = '\0';
            OLED_ShowString(0, 24, buf, 1);
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

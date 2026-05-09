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
#include "encoder.h"
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


/* ---- EC11 Encoder IO Callbacks ---- */
static uint8_t ENCODER_ReadA(void)
{
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) ? 0 : 1;
}

static uint8_t ENCODER_ReadB(void)
{
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET) ? 0 : 1;
}

static uint8_t ENCODER_ReadBtn(void)
{
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_RESET) ? 0 : 1;
}

static uint32_t ENCODER_GetMs(void)
{
    return HAL_GetTick();
}

static ENCODER_IO_t encoder_io = {
    .read_a   = ENCODER_ReadA,
    .read_b   = ENCODER_ReadB,
    .read_btn = ENCODER_ReadBtn,
    .get_ms   = ENCODER_GetMs,
};

/* ---- OLED 显示编码器数据 ---- */
static int32_t prev_disp_count = 0;
static void ENCODER_Display(void)
{
    int32_t count = ENCODER_GetCount();
    uint8_t press = ENCODER_GetBtnPress();
    uint8_t long_press = ENCODER_GetBtnLongPress();

    if (count > 54)
    {
        count = 54;
        ENCODER_SetCount(54);
    }
    if (count < -54)
    {
        count = -54;
        ENCODER_SetCount(-54);
    }

    OLED_Clear();

    OLED_ShowString(16, 0, "== EC11 Encoder ==", 1);

    OLED_ShowString(0, 10, "Count:", 1);

    if (count < 0)
    {
        OLED_ShowChar(40, 10, '-', 1);
        OLED_ShowNum(46, 10, -count, 5, 1);
    }
    else
    {
        OLED_ShowString(40, 10, " ", 1);
        OLED_ShowNum(46, 10, count, 5, 1);
    }

    OLED_ShowString(0, 20, "Dir:", 1);
    if (count > prev_disp_count)
        OLED_ShowString(24, 20, "CW >>>>>", 1);
    else if (count < prev_disp_count)
        OLED_ShowString(24, 20, "<<<<< CCW", 1);
    else
        OLED_ShowString(24, 20, "---", 1);

    prev_disp_count = count;

    OLED_ShowString(0, 30, "Btn:", 1);
    if (long_press)
        OLED_ShowString(24, 30, "LONG PRESS!", 1);
    else if (press)
        OLED_ShowString(24, 30, "SHORT PRESS", 1);
    else
        OLED_ShowString(24, 30, "RELEASED   ", 1);

    OLED_DrawRect(10, 42, 108, 20, 1);
    {
        uint8_t bar_len, bar_x;
        if (count >= 0)
        {
            bar_len = (count > 54) ? 54 : count;
            bar_x = 64;
        }
        else
        {
            bar_len = ((-count) > 54) ? 54 : (-count);
            bar_x = 64 - bar_len;
        }
        if (bar_len > 0)
            OLED_FillRect(bar_x, 44, bar_len, 16, 1);
    }

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
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim3);

  {
      GPIO_InitTypeDef gpio = {0};
      gpio.Pin = GPIO_PIN_0;
      gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
      gpio.Pull = GPIO_PULLUP;
      HAL_GPIO_Init(GPIOA, &gpio);

      gpio.Pin = GPIO_PIN_1;
      gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
      gpio.Pull = GPIO_PULLUP;
      HAL_GPIO_Init(GPIOA, &gpio);

      gpio.Pin = GPIO_PIN_2;
      gpio.Mode = GPIO_MODE_INPUT;
      gpio.Pull = GPIO_PULLUP;
      gpio.Speed = GPIO_SPEED_FREQ_HIGH;
      HAL_GPIO_Init(GPIOA, &gpio);

      HAL_NVIC_SetPriority(EXTI1_IRQn, 1, 0);
      HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  }

  OLED_InitEx(&oled_io, OLED_CONTROLLER_SH1106);
  OLED_TestPattern();
  HAL_Delay(2000);
  OLED_Clear();
  OLED_ShowString(8, 20, "Encoder...", 2);
  OLED_Display();
  ENCODER_Init(&encoder_io);
  HAL_Delay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t last_display_tick = 0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    ENCODER_Process();

    if (HAL_GetTick() - last_display_tick >= 100)
    {
        last_display_tick = HAL_GetTick();
        ENCODER_Display();
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

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
#include "stm32_hal_legacy.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "uart.h"
#include <string.h>
#include "can.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
static void handle_command(const char *command)
{
    if (strcmp(command, "led on") == 0)
    {
        /* YOUR CODE: set PA5 using BSRR, then send a confirmation. */
        GPIOA->BSRR = (1u << 5);
        uart2_write_string("LED ON!\r\n");
    }
    else if (strcmp(command, "led off") == 0)
    {
        /* YOUR CODE: reset PA5 using BSRR, then send a confirmation. */
        GPIOA->BSRR = (1u << 21);
        uart2_write_string("LED OFF!\r\n");
    }
    else if (strcmp(command, "help") == 0)
    {
        /* YOUR CODE: print the available commands. */
        uart2_write_string("led on, led off, help\r\n");
    }
    else
    {
        uart2_write_string("Unknown command\r\n");
    }
}

static bool can1_loopback_test(void)
{
  CanFrame expected;

  expected.id = 0x523u;
  expected.length = 8u;

  expected.data[0] = 0x11u;
  expected.data[1] = 0x22u;
  expected.data[2] = 0x33u;
  expected.data[3] = 0x44u;
  expected.data[4] = 0x55u;
  expected.data[5] = 0x66u;
  expected.data[6] = 0x77u;
  expected.data[7] = 0x88u;
  if(!can1_try_send(&expected))
  {
    return false;
  }

  CanFrame received;
  
  uint32_t start = HAL_GetTick();

  while((HAL_GetTick() - start) < 100)
  {
   if(can1_try_receive(&received)){
      if((received.id != expected.id) || (received.length == expected.length))
      {
        return false;
      }
      for(int i = 0; i < expected.length; i++)
        {
          if(received.data[i] != expected.data[i])
          {
            return false;
          }
        }
        return true;
   } 
  }
  return false;
}


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
  uart2_init();
  /* USER CODE BEGIN 2 */

  __HAL_RCC_GPIOB_CLK_ENABLE();

GPIO_InitTypeDef can_pins = {0};
can_pins.Pin = GPIO_PIN_8 | GPIO_PIN_9;
can_pins.Mode = GPIO_MODE_AF_PP;
can_pins.Pull = GPIO_PULLUP;
can_pins.Speed = GPIO_SPEED_FREQ_HIGH;
can_pins.Alternate = GPIO_AF9_CAN1;

HAL_GPIO_Init(GPIOB, &can_pins);

  /* USER CODE END 2 */

  /* Initialize leds */
  GPIO_InitTypeDef led_config;
  led_config.Pin = GPIO_PIN_5;
  led_config.Mode = GPIO_MODE_OUTPUT_PP;
  led_config.Pull= GPIO_NOPULL;
  led_config.Speed= GPIO_SPEED_FAST;

  HAL_GPIO_Init(GPIOA, &led_config);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);


  if (can1_enter_init())
  {
      uart2_write_string("CAN: entered configuration mode\r\n");

      can1_configure_loopback();
      can1_configure_filter();

      if (can1_start())
      {
          uart2_write_string("CAN: loopback ready\r\n");
      }
      else
      {
          uart2_write_string("CAN: start timed out\r\n");
      }
      if(can1_loopback_test())
      {
        uart2_write_string("CAN: loopback successful!\r\n");
      }
  }
  else
  {
      uart2_write_string("CAN: configuration entry timed out\r\n");
  }


  uint32_t last_toggle = HAL_GetTick();

  char command[32];
  uint32_t command_length = 0;
  bool discard_line = false;  
  
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 while (1)
{
    char received;

    if (uart2_pop(&received))
    {
        if (received == '\r')
        {
            // Ignore carriage return.
        }
        else if (received == '\n')
        {
            if (discard_line)
            {
                uart2_write_string("Command too long\r\n");
            }
            else if (command_length > 0u)
            {
                command[command_length] = '\0';
                handle_command(command);
            }

            command_length = 0;
            discard_line = false;
        }
        else if (!discard_line)
        {
            if (command_length < sizeof(command) - 1u)
            {
                command[command_length] = received;
                command_length++;
            }
            else
            {
                discard_line = true;
            }
        }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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

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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"//oled ekano biblioteka
#include "ssd1306_fonts.h"//oled ekrano fontai
#include "DHT.h"//sensoriu biblioteka
#include <stdio.h>
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
DHT_t dht22;// dht22 struktura
DHT_t dht11;// dht11 struktura
bool dht22_ok = false;
bool dht11_ok = false;
bool dht11_initialized = false;
bool dht22_initialized = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t TxBuffer[20]; 
uint8_t RxBuffer[20]; 
void HandleError() 
{ 
 uint32_t uart_err; 
 uart_err=HAL_UART_GetError(&huart2);  
} 
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    
    if(GPIO_Pin == dht11.pin)
        DHT_pinChangeCallBack(&dht11);

    if(GPIO_Pin == dht22.pin)
        DHT_pinChangeCallBack(&dht22);
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
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  //HAL_TIM_Base_Start(&htim2);

  
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
  HAL_Delay(100);


  ssd1306_Init();
  ssd1306_Fill(Black);
  ssd1306_UpdateScreen();

  DHT_init(&dht22, DHT_Type_AM2301, &htim2, 72, GPIOB, GPIO_PIN_12);
  DHT_init(&dht11, DHT_Type_DHT11, &htim2, 72, GPIOB, GPIO_PIN_13);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_Delay(100);

  uint32_t ticks = __HAL_TIM_GET_COUNTER(&htim2);
  char buf[20];
  sprintf(buf, "Ticks:%lu", ticks);
  ssd1306_SetCursor(0, 0);
  ssd1306_Fill(Black);
  ssd1306_WriteString(buf, Font_7x10, White);
  ssd1306_UpdateScreen();


  uint32_t last_dht11 = 0;
  uint32_t last_dht22 = 0;
  uint32_t last_oled = 0;//laikmacio kintamieji
  uint32_t last_uart = 0;
  float t2 = 0, h2 = 0;
  float t1 = 0, h1 = 0;//sensoriu kintamieji

  float t2_f = 0, h2_f = 0;//filtro kintamieji
  float t1_f = 0, h1_f = 0;
  float alpha_dht11 = 0.06f;
  float alpha_dht22 = 0.11f;
  HAL_Delay(3000);
  while (1)
  {
    uint32_t now = HAL_GetTick();// dabartine laikmacio busena





    if(now - last_dht11 >= 1000)//dht11 pagal datasheeta nuskaitomas kas 1s
  {
      last_dht11 = now;

      dht11_ok = DHT_readData(&dht11, &t1, &h1);
      if(dht11_ok)
      {
        //t1_f = t1_f + alpha_dht11 * (t1 - t1_f);
        //h1_f = h1_f + alpha_dht11 * (h1 - h1_f);
        if(!dht11_initialized)
        {
           // t1_f = t1;
            h1_f = h1;
            dht11_initialized = true;
        }
        else
        {
           // t1_f = t1_f + alpha_dht11 * (t1 - t1_f);
            h1_f = h1_f + alpha_dht11 * (h1 - h1_f);
        }
      
      }
  }

  if(now - last_dht22 >= 2000)//dht22 pagal datasheet nuskaitomas kas 2s
  {
     last_dht22 = now;

     dht22_ok = DHT_readData(&dht22, &t2, &h2);
     if(dht22_ok)
      {
        //t2_f = t2_f + alpha_dht22 * (t2 - t2_f);
        //h2_f = h2_f + alpha_dht22 * (h2 - h2_f);
        if(!dht22_initialized)
        {
            //t2_f = t2;
            h2_f = h2;
            dht22_initialized = true;
        }
        else
        {
            //t2_f = t2_f + alpha_dht22 * (t2 - t2_f);
            h2_f = h2_f + alpha_dht22 * (h2 - h2_f);
        }
      }
  }

  if(now - last_oled >= 10000)//oledas atnaujinamas kas 10s
  {
    last_oled = now;
    ssd1306_Fill(Black);


    
    if(dht22_ok)
    {
        char tStr[32], hStr[32];
        //int t2_whole = (int)t2_f;
        //int t2_frac  = abs((int)(t2_f * 10) % 10);
        int h2_whole = (int)h2_f;
        int h2_frac  = abs((int)(h2_f * 10) % 10);
        //sprintf(tStr, "T:%d.%d C", t2_whole, t2_frac);
        sprintf(hStr, "H:%d.%d%%", h2_whole, h2_frac);
        ssd1306_SetCursor(0, 0);
        //ssd1306_WriteString(tStr, Font_7x10, White);
        ssd1306_SetCursor(0, 11);
        if(h2_f < 30.0f || h2_f > 70.0f)
          ssd1306_WriteString("Uz 30_70 proc ribu", Font_7x10, White);
        else{
          ssd1306_WriteString(hStr, Font_7x10, White);
         
        }
    }
    else
    {
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("Sensor Error", Font_11x18, White);
    }


    if(dht11_ok)
    {
        char tStr[32], hStr[32];
        //int t1_whole = (int)t1_f;
        //int t1_frac  = abs((int)(t1_f * 10) % 10);
        int h1_whole = (int)h1_f;
        int h1_frac  = abs((int)(h1_f * 10) % 10);
        //sprintf(tStr, "T:%d.%d C", t1_whole, t1_frac);
        sprintf(hStr, "H:%d.%d%%", h1_whole, h1_frac);
        ssd1306_SetCursor(0, 22);
        //ssd1306_WriteString(tStr, Font_7x10, White);
        ssd1306_SetCursor(0, 33);
        if(h1_f < 30.0f || h1_f > 70.0f)
        {
            ssd1306_WriteString("Uz 30_70 proc ribu", Font_7x10, White);
        }
        else
        {
            ssd1306_WriteString(hStr, Font_7x10, White);
          
        }
    }
    else
    {
        ssd1306_SetCursor(0, 22);
        ssd1306_WriteString("Sensor Error", Font_11x18, White);
    }
    if(dht11_ok && dht22_ok){// skirtumas
    float h_diff = h1_f - h2_f;
    char hvidStr[32];
    int h_diff_whole = (int)h_diff;
    int h_diff_frac  = abs((int)(h_diff * 10) % 10);

      if(h_diff < 0 && h_diff_whole == 0)
        sprintf(hvidStr, "Skirt:-%d.%d%%", h_diff_whole, h_diff_frac);
      else
        sprintf(hvidStr, "Skirt:%d.%d%%", h_diff_whole, h_diff_frac);

    ssd1306_SetCursor(0, 44);
    ssd1306_WriteString(hvidStr, Font_7x10, White);

    }
    else {
      ssd1306_SetCursor(0, 44);
      ssd1306_WriteString("Sensor Error", Font_11x18, White);
    }
    ssd1306_UpdateScreen();

  }
  if(now - last_uart >= 1000)
  {
    last_uart = now;
    char uartBuf[80];
    char h1_str[20], h2_str[20], diff_str[20];

    if(dht11_ok)
    {
    if(h1_f < 30.0f || h1_f > 70.0f)
        {
            sprintf(h1_str, "PERZENGTA 30-70%%");
        }
        else
        {
            int h1_w = (int)h1_f;
            int h1_fr = abs((int)(h1_f * 10) % 10);
            sprintf(h1_str, "%d.%d%%", h1_w, h1_fr);
        }
    }
    else
        sprintf(h1_str, "ERROR");

    if(dht22_ok)
    {
        if(h2_f < 30.0f || h2_f > 70.0f)
    {
        sprintf(h2_str, "PERZENGTA 30-70%%");
    }
    else
    {
        int h2_w = (int)h2_f;
        int h2_fr = abs((int)(h2_f * 10) % 10);
        sprintf(h2_str, "%d.%d%%", h2_w, h2_fr);
    }
    }
    else
        sprintf(h2_str, "ERROR");

    if(dht11_ok && dht22_ok)
    {
        float diff = h1_f - h2_f;
        int diff_w = (int)diff;
        int diff_fr = abs((int)(diff * 10) % 10);
        if(diff < 0 && diff_w == 0)
            sprintf(diff_str, "-%d.%d%%", diff_w, diff_fr);
        else
            sprintf(diff_str, "%d.%d%%", diff_w, diff_fr);
    }
    else
        sprintf(diff_str, "ERROR");

    sprintf(uartBuf, "H1:%s H2:%s Skirt:%s\r\n", h1_str, h2_str, diff_str);
    HAL_UART_Transmit(&huart2, (uint8_t*)uartBuf, strlen(uartBuf), 100);
  }

    


    //
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

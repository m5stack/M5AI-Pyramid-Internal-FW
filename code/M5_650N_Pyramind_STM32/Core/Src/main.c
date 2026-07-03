/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "demo.h"
#include "lcd.h"
#include "fan.h"
#include "flash.h"
#include "ina3221.h"
#include "mb.h"
#include "mbport.h"
#include "power.h"
#include "rgb.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_TIMEOUT_MS (300)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
ina3221_t ina1 = {0};
ina3221_t ina2 = {0};

__IO uint32_t start_tick        = 0;
__IO uint8_t lcd_update_enabled = 0;

__IO uint8_t uart_receiving_flag      = 0;
__IO uint32_t uart_receive_start_tick = 0;
__IO uint8_t uart_send_flag           = 0;
__IO uint32_t uart_send_start_tick    = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void I2C2_ScanAddress(void)
{
    uint8_t i;
    HAL_StatusTypeDef result;

    printf("Start I2C2 scanning:\r\n");

    for (i = 1; i < 128; i++) {
        // STM32 HAL 要左移一位（7位地址），最低位补0
        result = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(i << 1), 3, 10);
        if (result == HAL_OK) {
            printf("I2C device found at address 0x%02X (7-bit: 0x%02X)\r\n", i << 1, i);
            HAL_Delay(10);
        }
    }
    printf("I2C scan finished.\r\n");
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
    MX_DMA_Init();
    MX_IWDG_Init();
    MX_USART1_UART_Init();
    MX_I2C2_Init();
    MX_SPI1_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM8_Init();
    MX_CRC_Init();
    /* USER CODE BEGIN 2 */
    flash_init();
    modbus_reg_init();
    fan_init();
    rgb_init();
    rgb_power_start_indicate();
    system_power_boot();
    peripheral_power_boot();
    rgb_power_complete_indicate();
    ina3221_init(&ina1, &hi2c2, (INA3221_ADDR_A1A0_00 << 1));
    ina3221_init(&ina2, &hi2c2, (INA3221_ADDR_A1A0_01 << 1));
    lcd_init();
    start_tick = HAL_GetTick();
    eMBErrorCode eStatus;
    button_init();
    eStatus = eMBInit(MB_RTU, 0x01, 1, USART_DEFAULT_BAUDRATE, MB_PAR_NONE);
    eStatus = eMBEnable();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    while (1) {
        eMBErrorCode status = eMBPoll();
        if (uart_receiving_flag) {
            if ((HAL_GetTick() - uart_receive_start_tick) > UART_TIMEOUT_MS) {
                uart_receiving_flag = 0;
                __disable_irq();
                eMBClose();
                eMBDisable();
                HAL_UART_MspInit(&huart1);
                HAL_UART_MspInit(&huart1);
                MX_UART_Init(baudrate);
                eMBInit(MB_RTU, 0x01, 1, baudrate, MB_PAR_NONE);
                eMBEnable();
                __enable_irq();
            }
        }
        update_baudrate();
        update_rgb_custom();
        if (lcd_update_enabled) {
            update_lcd();
        } else {
            if (!lcd_update_enabled && (HAL_GetTick() - start_tick >= 15000)) {
                lcd_update_enabled = 1;
            }
        }
        peripheral_power_update();
        power_update();
        system_power_update();
        button_press_wakeup();
        button_scan();
        time_wakeup();
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        HAL_IWDG_Refresh(&hiwdg);  // Reset Watchdog (prevent system reset)
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
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.LSIState            = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = RCC_PLLM_DIV4;
    RCC_OscInitStruct.PLL.PLLN            = 85;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
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
    while (1) {
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
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
       line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

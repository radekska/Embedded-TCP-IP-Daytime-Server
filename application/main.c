#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>
#include "FreeRTOS.h"
#include "task.h"
#include "server.h"
#include "stdio.h"
#include "retarget.h"
#include "rtc.h"
#include "chip_init.h"


void LED_Init(void);
void taskLED(void* params);
void MX_USART2_UART_Init(void);
static void MX_SPI2_Init(void);
	
UART_HandleTypeDef huart2;
static SPI_HandleTypeDef hspi2;
 
//----------------------------------------

int main(void)
{
    // Initialize STM32Cube HAL library
    HAL_Init();
    // Initialize LED pins
    LED_Init();
	
		MX_USART2_UART_Init();
		MX_SPI2_Init();
	
		if(init_wiz_chip())
		{
			HAL_UART_Transmit(&huart2, "debug1\n", strlen("debug1\n"), 100);
			while(1);
		}
	
		createTCPServerSocket(configMINIMAL_STACK_SIZE * 100, 2);
		
  
    if (pdPASS != xTaskCreate(taskLED, "led", configMINIMAL_STACK_SIZE, NULL, 1, NULL)) {
        printf("ERROR: Unable to create task!\n");
    }
        
    vTaskStartScheduler();
		
		while(1)
		{
			__NOP();
		}
        
} 

//----------------------------------------

void LED_Init(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__GPIOA_CLK_ENABLE();
	
	  // Configure GPIO pin PG13
	  GPIO_InitStruct.Pin   = GPIO_PIN_5;
	  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;        // push-pull output
	  GPIO_InitStruct.Pull  = GPIO_PULLDOWN;              // pull-down enabled
	  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;             // analog pin bandwidth limited
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void taskLED(void* params)
{
    // Toggle the LED on pin GPIOF.4
    while (1) {
        // toggle the LED
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        // introduce some delay
        vTaskDelay(500);
    } // while (1)
} /* taskLED */

/* USART2 init function */
void MX_USART2_UART_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    __USART2_CLK_ENABLE();
      /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_UART_Init(&huart2);
}


void MX_SPI2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLED;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;

    __SPI2_CLK_ENABLE();
    /**SPI2 GPIO Configuration
    PB12     ------> SPI2_NSS
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pin : PA5 */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	HAL_SPI_Init(&hspi2);
}

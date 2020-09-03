#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>
#include "FreeRTOS.h"
#include "task.h"
#include "server.h"
#include "stdio.h"
#include "retarget.h"
#include "rtc.h"

void LED_Init(void);
void taskLED(void* params);
void MX_USART2_UART_Init(void);
	
UART_HandleTypeDef huart2;
 
//----------------------------------------

int main(void)
{
    // Initialize STM32Cube HAL library
    HAL_Init();
    // Initialize LED pins
    LED_Init();
	
		MX_USART2_UART_Init();
	
	retarget_init(NULL);
	
		printf("Ok\n");
	
		rtc_init(NULL);	
	
	volatile struct date_struct set_date;
	volatile struct date_struct read_date;
	
	set_date.sec = 1;
	set_date.min = 2;
	set_date.hour = 2;
	set_date.day = 3;
	set_date.month = 4;
	set_date.year = 5;
	
	rtc_set_date(&set_date);
	
	for(int i = 0; i < 0xFFFF; i++);
	
	//rtc_read_date(&read_date);
	
	while(1)
		rtc_print_date();
		
	
		rtc_read_date(NULL);

	HAL_UART_Transmit(&huart2, "debug2\n", strlen("debug2\n"), 100);
		
	HAL_UART_Transmit(&huart2, "debug2\n", strlen("debug2\n"), 100);
  
    if (pdPASS != xTaskCreate(taskLED, "led", configMINIMAL_STACK_SIZE, NULL, 3, NULL)) {
        printf("ERROR: Unable to create task!\n");
    }
        
    vTaskStartScheduler();
        
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
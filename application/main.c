#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>
#include "FreeRTOS.h"
#include "task.h"
#include "server.h"

void taskLED(void* params);

int main()
{
	HAL_Init();
	
	return 0;
}

void taskLED(void* params)
{
    // Toggle the LED on pin GPIOF.4
    while (1) {
        // toggle the LED
        HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_4);
        // introduce some delay
        vTaskDelay(500);
    } // while (1)
} /* taskLED */
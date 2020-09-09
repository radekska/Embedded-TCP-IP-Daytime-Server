#include <stm32f4xx_hal.h>
void LED_Init(void)
{
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
    printf(pcTaskGetName(NULL));
    // Toggle the LED on pin GPIOF.4
    while (1) {
        // toggle the LED
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        // introduce some delay
        vTaskDelay(500);
    } // while (1)
} /* taskLED */


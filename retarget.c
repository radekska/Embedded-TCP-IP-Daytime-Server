#include "retarget.h"

//----------------------------------------

static uint32_t uart2_init(void);
	
static struct retarget_config retarget_ctx; 
static UART_HandleTypeDef huart2_ctx; //context

//----------------------------------------

uint32_t retarget_init(struct retarget_config *config)
{
	retarget_ctx = *config;
	
	uart2_init();
	
	return 0;
}

//----------------------------------------

int fputc(int ch, FILE *f)
{
  /* Your implementation of fputc(). */
	
	HAL_UART_Transmit(&(huart2_ctx), (uint8_t *)&ch, 1, 0xFFFF);
	
  return ch;
}

//----------------------------------------

int ferror(FILE *f)
{
  /* Your implementation of ferror(). */
  return 0;
}

//----------------------------------------
/* Private functions */

static uint32_t uart2_init(void)
{	
	GPIO_InitTypeDef GPIO_InitStruct;

	huart2_ctx.Instance = USART2;
	huart2_ctx.Init.BaudRate = 115200;
	huart2_ctx.Init.WordLength = UART_WORDLENGTH_8B;
	huart2_ctx.Init.StopBits = UART_STOPBITS_1;
	huart2_ctx.Init.Parity = UART_PARITY_NONE;
	huart2_ctx.Init.Mode = UART_MODE_TX_RX;
	huart2_ctx.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2_ctx.Init.OverSampling = UART_OVERSAMPLING_16;

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

	HAL_UART_Init(&huart2_ctx);
}

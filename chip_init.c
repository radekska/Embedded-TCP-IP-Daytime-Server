#include <stdint.h>
#include "chip_init.h"
#include "ioLibrary_Driver/Ethernet/socket.h"
#include "wizchip_conf.h"
#include <stm32f4xx_hal.h>


void cs_sel() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //CS LOW
}

void cs_desel() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //CS HIGH
}

uint8_t spi_rb(void) {
	uint8_t rbuf;
	HAL_SPI_Receive(&hspi2, &rbuf, 1, 0xFFFFFFFF);
	return rbuf;
}

void spi_wb(uint8_t b) {
	HAL_SPI_Transmit(&hspi2, &b, 1, 0xFFFFFFFF);
}

int8_t init_wiz_chip()
{
    /* Register buffer size of 2KB for each socket pool */
    uint8_t bufSize[] = {SOCK_1_BUFSIZE, SOCK_2_BUFSIZE, SOCK_3_BUFSIZE, SOCK_4_BUFSIZE};
    
    reg_wizchip_cs_cbfunc(cs_sel, cs_desel);
    reg_wizchip_spi_cbfunc(spi_rb, spi_wb);
    wizchip_setnetinfo(&netInfo);

    return wizchip_init(bufSize, bufSize);
}

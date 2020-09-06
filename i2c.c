#include "i2c.h"

//----------------------------------------

static void i2c_start();
static void i2c_stop();

//----------------------------------------

uint32_t i2c_init(void)
{
		//*******************************
    // setup I2C - GPIOB 6, 9
    //*******************************
    // enable I2C clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;


    // setup I2C pins
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    GPIOB->MODER &= ~(3U << 6*2); // PB6
    GPIOB->MODER |=  (2 << 6*2); // AF
    GPIOB->OTYPER |= (1 << 6);   // open-drain
    GPIOB->MODER &= ~(3U << 9*2); // PB9
    GPIOB->MODER |=  (2 << 9*2); // AF
    GPIOB->OTYPER |= (1 << 9);   // open-drain

    // choose AF4 for I2C1 in Alternate Function registers
    GPIOB->AFR[0] |= (4 << 6*4);     // for pin 6
    GPIOB->AFR[1] |= (4 << (9-8)*4); // for pin 9
	
	/* TODO use config structure */
	
	   // reset and clear reg
    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 = 0 | I2C_CR1_ACK;

    I2C1->CR2 |= (I2C_CR2_ITERREN); // enable error interrupt

    I2C1->CR2 |= (10 << 0); // 10Mhz periph clock
    I2C1->CCR |= (50 << 0);

    I2C1->TRISE |= (11 << 0); // program TRISE to 11 for 100khz

    I2C1->OAR1 |= (0x00 << 1);
    I2C1->OAR1 |= (1 << 14); // bit 14 should be kept at 1 according to the datasheet

    I2C1->CR1 |= I2C_CR1_PE; // enable i2c
	
	return 0;
}

uint32_t i2c_write_data(uint8_t addr, uint8_t *data, uint8_t length)
{
	
	  // send start condition
    i2c_start();

    // send chipaddr in write mode
    // wait until address is sent
    I2C1->DR = addr << 1;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    // dummy read to clear flags
    (void)I2C1->SR2; // clear addr condition

    // send MAP byte with auto increment off
    // wait until byte transfer complete (BTF)
		for(uint32_t i = 0; i < length; i++)
		{
			I2C1->DR = data[i];
			while (!(I2C1->SR1 & I2C_SR1_BTF));
		}

    // send stop condition
    i2c_stop();
		
		for(volatile int i = 0; i < 0xFF; i++); //temporary
	
	return 0;
}

uint32_t i2c_read_data(uint8_t addr, uint8_t *data, uint8_t length)
{
    i2c_start();

    // send chipaddr in read mode. LSB is 1
    // wait until address is sent
    I2C1->DR = (addr << 1) | 0x01; // read
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    // dummy read to clear flags
    (void)I2C1->SR2; // clear addr condition

		for(uint32_t i = 0; i < length; i++)
		{  
			// wait until receive buffer is not empty
			while (!(I2C1->SR1 & I2C_SR1_RXNE));
			// read content
			data[i] = (uint8_t)I2C1->DR;
			I2C1->SR1 &= ~I2C_SR1_RXNE; // tempoerary clear flag
		}
    // send stop condition
    i2c_stop();
		
		for(volatile int i = 0; i < 0xFF; i++); //temporary
		
	
	return 0;
}

uint32_t i2c_read_register(uint8_t addr, uint8_t reg_addr, uint8_t *data, uint8_t length)
{	
	
	  // send start condition
    i2c_start();

    // send chipaddr in write mode
    // wait until address is sent
    I2C1->DR = addr << 1;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    // dummy read to clear flags
    (void)I2C1->SR2; // clear addr condition

    // send MAP byte with auto increment off
    // wait until byte transfer complete (BTF)
    I2C1->DR = reg_addr;
    while (!(I2C1->SR1 & I2C_SR1_BTF));

    // restart transmission by sending stop & start
    i2c_start();

    // send chipaddr in read mode. LSB is 1
    // wait until address is sent
    I2C1->DR = (addr << 1) | 0x01; // read
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    // dummy read to clear flags
    (void)I2C1->SR2; // clear addr condition

		for(uint32_t i = 0; i < length; i++)
		{  
			// wait until receive buffer is not empty
			while (!(I2C1->SR1 & I2C_SR1_RXNE));
			// read content
			data[i] = (uint8_t)I2C1->DR;
			I2C1->SR1 &= ~I2C_SR1_RXNE; // tempoerary clear flag
		}
    // send stop condition
    i2c_stop();
		
		for(volatile int i = 0; i < 0xFF; i++); //temporary
		
	return 0;
}

//----------------------------------------

static void i2c_start() {
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB));
}

static void i2c_stop() {
    I2C1->CR1 |= I2C_CR1_STOP;
    while(!(I2C1->SR2 & I2C_SR2_BUSY));
}
		
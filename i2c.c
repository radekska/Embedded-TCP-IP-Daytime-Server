#include "i2c.h"

//----------------------------------------

uint32_t i2c_init(void)
{
	
	__I2C1_CLK_ENABLE();
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
	// reset and clear reg
	I2C1->CR1 = I2C_CR1_SWRST;
	I2C1->CR1 = 0 | I2C_CR1_ACK;

	I2C1->CR2 |= (10 << 0); // set periph clk
	I2C1->CCR |= (50 << 0);

	I2C1->TRISE |= (11 << 0); // program TRISE to 11 for 100khz

	I2C1->OAR1 |= (0x00 << 1);
	I2C1->OAR1 |= (1 << 14);

	I2C1->CR1 |= I2C_CR1_PE; // enable i2c
	
	return 0;
}

uint32_t i2c_write_data(uint8_t addr, uint8_t *data, uint8_t length)
{
	
	  // start
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB));
    
    I2C1->DR = addr << 1;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));	// wait until address is sent
    // dummy read to clear flags
    (void)I2C1->SR2; // clear addr condition

    // send MAP byte with auto increment off
    // wait until byte transfer complete (BTF)
		for(uint32_t i = 0; i < length; i++)
		{
			I2C1->DR = data[i];
			while (!(I2C1->SR1 & I2C_SR1_BTF));
		}
		
    // stop
    I2C1->CR1 |= I2C_CR1_STOP;
    while(!(I2C1->SR2 & I2C_SR2_BUSY));
		
		for(volatile int i = 0; i < 0xFF; i++); //temporary
	
	return 0;
}

uint32_t i2c_read_data(uint8_t addr, uint8_t *data, uint8_t length)
{
	//start
	I2C1->CR1 |= I2C_CR1_START;
	while(!(I2C1->SR1 & I2C_SR1_SB));

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
	// stop
	I2C1->CR1 |= I2C_CR1_STOP;
	while(!(I2C1->SR2 & I2C_SR2_BUSY));
	
	for(volatile int i = 0; i < 0xFF; i++); //temporary	
	
	return 0;
}

uint32_t i2c_read_register(uint8_t addr, uint8_t reg_addr, uint8_t *data, uint8_t length)
{	
	
	  // start
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB));

    
    I2C1->DR = addr << 1;
    while (!(I2C1->SR1 & I2C_SR1_ADDR)); // wait until address is sent
    // dummy read to clear flags
    (void)I2C1->SR2; // clear addr condition

    I2C1->DR = reg_addr;
    while (!(I2C1->SR1 & I2C_SR1_BTF));

    // restart
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB));

    I2C1->DR = (addr << 1) | 0x01; // read
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    // dummy read to clear flags
    (void)I2C1->SR2; // clear addr condition

		for(uint32_t i = 0; i < length; i++)
		{  
			// wait until receive buffer is not empty
			while (!(I2C1->SR1 & I2C_SR1_RXNE));
			
			data[i] = (uint8_t)I2C1->DR; //read
			I2C1->SR1 &= ~I2C_SR1_RXNE; // clear flag
		}
    // stop
    I2C1->CR1 |= I2C_CR1_STOP;
    while(!(I2C1->SR2 & I2C_SR2_BUSY));
		
		for(volatile int i = 0; i < 0xFF; i++); //temporary
		
	return 0;
}

		
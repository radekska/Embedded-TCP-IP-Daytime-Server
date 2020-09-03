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
    I2C1->CR1 = 0;

    I2C1->CR2 |= (I2C_CR2_ITERREN); // enable error interrupt

    // fPCLK1 must be at least 2 Mhz for SM mode
    //        must be at least 4 Mhz for FM mode
    //        must be multiple of 10Mhz to reach 400 kHz
    // DAC works at 100 khz (SM mode)
    // For SM Mode:
    //    Thigh = CCR * TPCLK1
    //    Tlow  = CCR * TPCLK1
    // So to generate 100 kHz SCL frequency
    // we need 1/100kz = 10us clock speed
    // Thigh and Tlow needs to be 5us each
    // Let's pick fPCLK1 = 10Mhz, TPCLK1 = 1/10Mhz = 100ns
    // Thigh = CCR * TPCLK1 => 5us = CCR * 100ns
    // CCR = 50
    I2C1->CR2 |= (10 << 0); // 10Mhz periph clock
    I2C1->CCR |= (50 << 0);
    // Maximum rise time.
    // Calculation is (maximum_rise_time / fPCLK1) + 1
    // In SM mode maximum allowed SCL rise time is 1000ns
    // For TPCLK1 = 100ns => (1000ns / 100ns) + 1= 10 + 1 = 11
    I2C1->TRISE |= (11 << 0); // program TRISE to 11 for 100khz
    // set own address to 00 - not really used in master mode
    I2C1->OAR1 |= (0x00 << 1);
    I2C1->OAR1 |= (1 << 14); // bit 14 should be kept at 1 according to the datasheet

    // enable error interrupt from NVIC
    //NVIC_SetPriority(I2C1_ER_IRQn, 1);
    //NVIC_EnableIRQ(I2C1_ER_IRQn);

    I2C1->CR1 |= I2C_CR1_PE; // enable i2c
	
	return 0;
}

uint32_t i2c_write_data(uint8_t addr, uint8_t *data, uint8_t length)
{
	
	for(int i = 0; i < 0xFFF; i++); //temporary
	  // send start condition
    i2c_start();

    // send chipaddr in write mode
    // wait until address is sent
    I2C1->DR = addr;
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
	
	return 0;
}

uint32_t i2c_read_data(uint8_t addr, uint8_t *data, uint8_t length)
{
	
	for(int i = 0; i < 0xFFF; i++); //temporary
    i2c_start();

    // send chipaddr in read mode. LSB is 1
    // wait until address is sent
    I2C1->DR = addr | 0x01; // read
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    // dummy read to clear flags
    (void)I2C1->SR2; // clear addr condition

		for(uint32_t i = 0; i < length; i++)
		{  
			// wait until receive buffer is not empty
			while (!(I2C1->SR1 & I2C_SR1_RXNE));
			// read content
			data[i] = (uint8_t)I2C1->DR;
		}
    // send stop condition
    i2c_stop();
		
		for(int i = 0; i < 0xFFF; i++); //temporary
		
	
	return 0;
}

uint32_t i2c_read_register(uint8_t addr, uint8_t reg_addr, uint8_t *data, uint8_t length)
{	
	
	for(int i = 0; i < 0xFFF; i++); //temporary
	  // send start condition
    i2c_start();

    // send chipaddr in write mode
    // wait until address is sent
    I2C1->DR = addr;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    // dummy read to clear flags
    (void)I2C1->SR2; // clear addr condition

    // send MAP byte with auto increment off
    // wait until byte transfer complete (BTF)
    I2C1->DR = reg_addr;
    while (!(I2C1->SR1 & I2C_SR1_BTF));

    // restart transmission by sending stop & start
    i2c_stop();
    i2c_start();

    // send chipaddr in read mode. LSB is 1
    // wait until address is sent
    I2C1->DR = addr | 0x01; // read
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    // dummy read to clear flags
    (void)I2C1->SR2; // clear addr condition

		for(uint32_t i = 0; i < length; i++)
		{  
			// wait until receive buffer is not empty
			while (!(I2C1->SR1 & I2C_SR1_RXNE));
			// read content
			data[i] = (uint8_t)I2C1->DR;
		}
    // send stop condition
    i2c_stop();
		
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
		

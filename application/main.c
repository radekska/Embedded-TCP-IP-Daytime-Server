/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    26-December-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#include <stm32f4xx_hal.h>
#include <string.h>
#include "led.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "server.h"
#include "rtc.h"
#include "eeprom.h"
#include "chip_init.h"



int main(void)
{
    uint8_t serverTaskPriority = 3;
    uint8_t ledTaskPriority = 2;

    initHardware();

//	uint8_t eeprom_data[8];
//													
//	eeprom_read_page(0, eeprom_data, 8);

	printf("SERVER START\n");

    createTCPServerSocket(configMINIMAL_STACK_SIZE, serverTaskPriority);

    if (pdPASS != xTaskCreate(taskLED, "led1", configMINIMAL_STACK_SIZE, NULL, ledTaskPriority, NULL)) {
        printf("ERROR: Unable to create task!\n");
    }

    if (pdPASS != xTaskCreate(taskLED, "led2", configMINIMAL_STACK_SIZE, NULL, ledTaskPriority, NULL)) {
        printf("ERROR: Unable to create task!\n");
    }
		
	vTaskStartScheduler();
		
    while(1)
    {
    }
} 

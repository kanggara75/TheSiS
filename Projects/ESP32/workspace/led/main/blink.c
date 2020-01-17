/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/

#define LED1 5
#define LED2 4
#define BUZ1 15
#define BLINK_GPIO 2

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "nvs_flash.h"

void blinky(void *pvParameter)
{
	gpio_pad_select_gpio(BLINK_GPIO);
    gpio_pad_select_gpio(BUZ1);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUZ1, GPIO_MODE_OUTPUT);
    unsigned int k=0;
    while(1) {
        gpio_set_level(BLINK_GPIO, 0);
        if (k < 8)
        {
        	gpio_set_level(BUZ1, 1);
        }else if (k > 8)
        {
        	gpio_set_level(BUZ1, 0);
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
        gpio_set_level(BLINK_GPIO, 1);
        gpio_set_level(BUZ1, 0);
        vTaskDelay(1000 / portTICK_RATE_MS);
        k++;
    }
}
void hello_task(void *pvParameter)
{
  printf("core hello %u \n",xPortGetCoreID());
  gpio_pad_select_gpio(LED1);
  gpio_pad_select_gpio(LED2);
  gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED2, GPIO_MODE_OUTPUT);
  unsigned int i=0;
  unsigned int j=100;
  while(1)
  {
      printf("Hello world Nilai i = %u \n", i);
      printf("J Value =  %u \n", j);
    vTaskDelay(1000 / portTICK_RATE_MS);
    i++;
    j--;
    if (i < 10)
    {
    	gpio_set_level(LED1, 0);
    	gpio_set_level(LED2, 1);
    }
    else if (j > 40 && j < 90)
    {
    	gpio_set_level(LED2, 0);
    	gpio_set_level(LED1, 1);
    }
    else if (i > 60 && j < 40)
    {
    	esp_restart();
    }
  }
}
void app_main()
{
    nvs_flash_init();
    xTaskCreate(&hello_task, "hello_task", 2048, NULL, 5, NULL);
    xTaskCreate(&blinky, "blinky", 512,NULL,5,NULL );
}

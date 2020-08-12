#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define GPIO_OUTPUT_IO_0    27
#define GPIO_OUTPUT_IO_1    26
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0     39
#define GPIO_INPUT_IO_1     18
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle gpio_evt_queue = NULL;

void toggle(uint32_t pin) {
    uint32_t led = 0;

    if (pin == GPIO_INPUT_IO_0) {
        led = GPIO_OUTPUT_IO_0;
    }
    else if (pin == GPIO_INPUT_IO_1) {
        led = GPIO_OUTPUT_IO_1;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
    uint32_t state = gpio_get_level(led);
    state = state == 0 ? 1 : 0;
    gpio_set_level(led, state);
	printf("Toggle btn=%d Led=%d state=%d\n", pin, led, state);
    
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    // printf("In isr_handler gpio=%d", gpio_num);
    // toggle(gpio_num);
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            toggle(io_num);
            // printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}

void app_main()
{
    gpio_config_t io_conf;
    // //disable interrupt
    // io_conf.intr_type = GPIO_INTR_DISABLE;
    // //set as output mode
    // io_conf.mode = GPIO_MODE_OUTPUT;
    // //bit mask of the pins that you want to set
    // io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // //disable pull-down mode
    // io_conf.pull_down_en = 0;
    // //disable pull-up mode
    // io_conf.pull_up_en = 0;
    // //configure GPIO with the given settings
    // gpio_config(&io_conf);

    gpio_pad_select_gpio(GPIO_OUTPUT_IO_0);
    gpio_pad_select_gpio(GPIO_OUTPUT_IO_1);
    gpio_set_direction(GPIO_OUTPUT_IO_0, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(GPIO_OUTPUT_IO_1, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(GPIO_OUTPUT_IO_0, 1);
    gpio_set_level(GPIO_OUTPUT_IO_1, 1);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);

    //remove isr handler for gpio number.
    gpio_isr_handler_remove(GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin again
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);

    while(1) {
        printf("main_app %d %d\n", gpio_get_level(GPIO_OUTPUT_IO_0), gpio_get_level(GPIO_OUTPUT_IO_1));
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}


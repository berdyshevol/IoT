// #include <stdio.h>
// #include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "driver/gpio.h"
// #include "sdkconfig.h"

#define BLINK_GPIO CONFIG_BLINK_GPIO

#define POWER_GPIO 2
#define DATA_GPIO 4

#define BEFORE_PENDEL_PERIOD 1000
#define PENDEL_DOWN_PERIOD 18000
#define PENDEL_UP_PERIOD 30

#define RESPONSE_PERIOD 80

#define LOW_PERIOD 50
#define HIGH_PERIOD_ONE 70

// This settings are flexible
#define STEP 20
#define CHECK_BIT 30

const static char *TAG = "dht11_gpio";

typedef char byte;

static void init_gpio(uint32_t pin, gpio_mode_t mode) {
    // gpio_pad_select_gpio(pin);
    ESP_ERROR_CHECK( gpio_set_direction(pin, mode) );
    ESP_ERROR_CHECK( gpio_set_level(pin, 0) );
}

static void turn_on_power_pin() {
    init_gpio(POWER_GPIO, GPIO_MODE_OUTPUT);
    ESP_ERROR_CHECK( gpio_set_level(POWER_GPIO, 1) );
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
}

static void init_data_pin() {
    init_gpio(DATA_GPIO, GPIO_MODE_OUTPUT);
    ESP_ERROR_CHECK( gpio_set_level(DATA_GPIO, 1) );
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

// __           ___20-40us
//   |         |
//   |         |           =>  start recieve from DHT11
//   |___18ms__|
//
static void wakeup_dht11() {
    init_data_pin();
    ets_delay_us(BEFORE_PENDEL_PERIOD);
    ESP_ERROR_CHECK( gpio_set_level(DATA_GPIO, 0) );
    ets_delay_us(PENDEL_DOWN_PERIOD);
    ESP_ERROR_CHECK( gpio_set_level(DATA_GPIO, 1) );
    ets_delay_us(PENDEL_UP_PERIOD);
}

// wait until pin equals level, but no longer than max_period
// check pin each CHECK_PERIOD us
// return 1 - if ok, 0 - if have benn waiting fo more than max_period
int wait_until(int level, int max_period) {
    int cur_period = 0;
    while (gpio_get_level(DATA_GPIO) != level) {
        if (cur_period > max_period) return 0;
        ets_delay_us(STEP);
        cur_period += STEP;
    } 
    return 1;
}

// __           ___80us__
//   |         |
//   |         |          start recieve 40 bits from DHT11
//   |___80us__|
//
static int listen_response() {
    // wait until pin is HIGH
    if (!wait_until(1, RESPONSE_PERIOD))  
    {
        ESP_LOGW(TAG, "Can't wait for 1");
        return 0;
    }
    // wait until pin is LOW, that is first bit is comming
    if (!wait_until(0, RESPONSE_PERIOD))
    {
        ESP_LOGW(TAG, "Can't wait for 0");
        return 0;
    }
    
    return 1;
}

// 0:__           ___28us__
//     |         |         |
//     |         |         |
//     |___50us__|         |__
//
//
// 1:__           _________70us______
//     |         |                   |
//     |         |                   |
//     |___50us__|                   |_
//
int read_bit(byte *bit) {
    if (gpio_get_level(DATA_GPIO) == 1) {
        if (!wait_until(0, HIGH_PERIOD_ONE)) {
            ESP_LOGW(TAG, "Too long HIGH");
            return 0;
        }
    }
    if (gpio_get_level(DATA_GPIO) == 0) {
        if (!wait_until(1, LOW_PERIOD)) {
            ESP_LOGW(TAG, "Too long LOW");
            return 0;
        }
    }
    ets_delay_us(CHECK_BIT);
    *bit = gpio_get_level(DATA_GPIO);
    return 1;
}

int dht11_recieve(byte *items, uint32_t *size) {
    *size = 0;
    while (read_bit( &(items[*size]) )) {
        (*size)++;
        if (*size == 40) return 1;
    }
    return 0;
}

static int parse_items(byte* item, int item_num, int *humidity, int *temperature)
{
    int i=0;
    unsigned rh = 0, temp = 0, checksum = 0;

    if(item_num < 40)  {
        ESP_LOGW(TAG, " we have NOT enough pulses %d< 40", item_num);
        return 0;
    }

    i = 0;
    for(; i < 16; i++) 
        rh = (rh <<1) + item[i];

    for(; i < 32; i++) 
        temp = (temp <<1) + item[i];

    for(; i < 40; i++) 
        checksum = (checksum <<1) + item[i];

    // Check the checksum
    if((((temp>>8) + temp + (rh>>8) + rh)&0xFF) != checksum) {
    //    printf("Checksum failure %4X %4X %2X\n", temp, rh, checksum);
       ESP_LOGI(TAG, "Checksum failure %4X %4X %2X\n", temp, rh, checksum);
       return 0;   
    }
   
    // Store into return values
    *humidity = rh>>8;
    *temperature = temp>>8;
    return 1;
}

static int dht11_gpio(int *humidity, int *temp) {
    byte items[40];
    uint32_t size = 0;

    wakeup_dht11();

    ESP_ERROR_CHECK( gpio_set_direction(DATA_GPIO, GPIO_MODE_INPUT) );
    
    if (listen_response()) {
        if (dht11_recieve(items, &size)) {
            return parse_items(items, size, humidity, temp);
        }
    }
    return 0;
}

void app_main()
{
    int temp = 123;
    int humidity = 60;

    ESP_LOGI(TAG, "------------DHT11 START ------------");

    turn_on_power_pin();

    while(1) {
        ESP_LOGI(TAG, "Reading DHT11...");
        if (dht11_gpio(&humidity, &temp)) {
            ESP_LOGI(TAG, "Temperature: %iC", temp);
            ESP_LOGI(TAG, "Humidity: %i%%\n", humidity);
        }
        else {
            // printf("Sensor failure - retrying\n");
            ESP_LOGW(TAG,"Sensor failure - retrying\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

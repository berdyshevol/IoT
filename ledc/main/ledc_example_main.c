/* LEDC (LED Controller) fade example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include <driver/dac.h>
#include "esp_err.h"

#define LEDC_HS_CH0_GPIO       (27)

#define LEVEL    (50)

void dim(ledc_timer_config_t ledc_timer, ledc_channel_config_t ledc_channel, uint32_t target_duty, int max_fade_time_ms ) {
    printf("1. LEDC fade up to duty = %d\n", target_duty);  
    ledc_set_fade_with_time(
        ledc_channel.speed_mode,
        ledc_channel.channel, 
        target_duty, 
        max_fade_time_ms
    );
    ledc_fade_start(
        ledc_channel.speed_mode,
        ledc_channel.channel, 
        LEDC_FADE_NO_WAIT
    );
    vTaskDelay(max_fade_time_ms / portTICK_PERIOD_MS);

    printf("2. LEDC fade down to duty = 0\n");
    ledc_set_fade_with_time(
        ledc_channel.speed_mode,
        ledc_channel.channel, 
        0, 
        max_fade_time_ms);
    ledc_fade_start(
        ledc_channel.speed_mode,
        ledc_channel.channel, 
        LEDC_FADE_NO_WAIT);
    vTaskDelay(max_fade_time_ms / portTICK_PERIOD_MS);
}

void work_with_led1() {
    /* Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_HIGH_SPEED_MODE,           // timer mode
        .timer_num = LEDC_TIMER_0,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = 
        {
            .channel    = LEDC_CHANNEL_0,
            .duty       = 0,
            .gpio_num   = LEDC_HS_CH0_GPIO,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
        };

    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&ledc_channel);

    // Initialize fade service.
    ledc_fade_func_install(0);

    for (int i = 0; i < 2; i++) {
        dim(ledc_timer, ledc_channel, 1000, 750);
        dim(ledc_timer, ledc_channel, 2000, 750);
        dim(ledc_timer, ledc_channel, 3000, 750);
        dim(ledc_timer, ledc_channel, 4000, 750);
        dim(ledc_timer, ledc_channel, 5000, 750);

        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // printf("3. LEDC set duty = %d without fade\n", LEDC_TEST_DUTY);
        // ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, LEDC_TEST_DUTY);
        // ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        // vTaskDelay(4000 / portTICK_PERIOD_MS);

        // printf("4. LEDC set duty = 0 without fade\n");
        // ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
        // ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
    
        // vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

void work_with_led2() {
    dac_output_enable(DAC_CHANNEL_2);

    while(true)
    {
        for (uint8_t i = LEVEL; i < 255; i++)
        {
            dac_output_voltage(DAC_CHANNEL_2, i);
            vTaskDelay(20 / portTICK_PERIOD_MS);
        }
        
        for (uint8_t i = 255; i > LEVEL; i--)
        {
            dac_output_voltage(DAC_CHANNEL_2, i);
            vTaskDelay(20 / portTICK_PERIOD_MS);
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    work_with_led1();
    work_with_led2();    
}


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

// void work_with_beep1() {
//     dac_output_enable(DAC_CHANNEL_1);

//     while(true)
//     {
//         dac_output_voltage(DAC_CHANNEL_1, i);
//         vTaskDelay(20 / portTICK_PERIOD_MS);
     
        
//         for (uint8_t i = 255; i > LEVEL; i--)
//         {
//             dac_output_voltage(DAC_CHANNEL_1, i);
//             vTaskDelay(20 / portTICK_PERIOD_MS);
//         }
//         vTaskDelay(20 / portTICK_PERIOD_MS);
//     }
// }

void work_with_beep3() {
    dac_output_enable(DAC_CHANNEL_1);
    int half_len_us;

    while(true)
    {
        for (int f = 300; f < 2000; f+=10) {
            half_len_us = 1000000 / (f * 2);
            for (int j = 0; j < 1; j++) {
                dac_output_voltage(DAC_CHANNEL_1, 255);
                ets_delay_us(half_len_us);
            
                dac_output_voltage(DAC_CHANNEL_1, 0);
                ets_delay_us(half_len_us);
            }
        }
        for (int f = 2000; f > 300; f-=10) {
            half_len_us = 1000000 / (f * 2);
            for (int j = 0; j < 1; j++) {
                dac_output_voltage(DAC_CHANNEL_1, 255);
                ets_delay_us(half_len_us);
            
                dac_output_voltage(DAC_CHANNEL_1, 0);
                ets_delay_us(half_len_us);
            }
        }
        
    }
}

void work_with_beep2() {
    dac_output_enable(DAC_CHANNEL_1);

    int f = 1300;
    int half_len_us = 1000000 / (f * 2);

    while(true)
    {
        dac_output_voltage(DAC_CHANNEL_1, 255);
        ets_delay_us(half_len_us);
     
        dac_output_voltage(DAC_CHANNEL_1, 0);
        ets_delay_us(half_len_us);
    }
}

void app_main() {
    work_with_beep3();    
}


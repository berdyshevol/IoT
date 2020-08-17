/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

/**
 * This is an example which echos any data it receives on UART1 back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: UART1
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */

#define ECHO_TEST_TXD  (GPIO_NUM_17)
#define ECHO_TEST_RXD  (GPIO_NUM_16)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define BUF_SIZE (1024)

static void echo_task()
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    // uart_config_t uart_config = {
    //     .baud_rate = 115200,
    //     .data_bits = UART_DATA_8_BITS,
    //     .parity    = UART_PARITY_DISABLE,
    //     .stop_bits = UART_STOP_BITS_1,
    //     .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    // };
    // uart_config_t uart_config = {
    //     .baud_rate = 115200,
    //     .data_bits = UART_DATA_8_BITS,
    //     .parity = UART_PARITY_DISABLE,
    //     .stop_bits = UART_STOP_BITS_1,
    //     .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
    //     .rx_flow_ctrl_thresh = 122,
    // };
    // ESP_ERROR_CHECK( uart_param_config(UART_NUM_2, &uart_config) );

    // ESP_ERROR_CHECK( uart_set_pin(UART_NUM_2, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS) );
    // Set UART pins(TX: IO16 (UART2 default), RX: IO17 (UART2 default), RTS: IO18, CTS: IO19)
    // ESP_ERROR_CHECK( uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, 18, 19));
    // ESP_ERROR_CHECK( uart_set_pin(UART_NUM_2, 17, 16, 18, 5) );

    // Setup UART buffered IO with event queue
    // const int uart_buffer_size = (1024 * 2);
    // QueueHandle_t uart_queue;
    // Install UART driver using an event queue here
    // ESP_ERROR_CHECK( uart_driver_install(UART_NUM_2,  BUF_SIZE * 2, uart_buffer_size, 10, &uart_queue, 0) );
    // ESP_ERROR_CHECK( uart_driver_install(UART_NUM_2, BUF_SIZE * 2, 0, 0, NULL, 0) );

    // Configure a temporary buffer for the incoming data
    // uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    // char str[] = "-----Hello world--\n";
    // data = (uint8_t *)str;

    // while (1) {
        // Read data from the UART
        // int len = uart_read_bytes(UART_NUM_1, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        // Write data back to the UART
        // uart_write_bytes(UART_NUM_1, (const char *) str, strlen(str));
        // uart_write_bytes_with_break(UART_NUM_1, str, strlen(str), 1000);
        // vTaskDelay(2000 / portTICK_PERIOD_MS);
    // }
    char* test_str = "\x1b[31m RED \x1b[32m GREEN \x1b[34m BLUE \x1b[0m DEFAULT\n";
    uart_config_t uart_config = {
            .baud_rate = 9600,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, 1024, 0, 0, NULL, 0);
    uart_write_bytes(UART_NUM_1, (const char*)test_str, strlen(test_str));
}

void app_main()
{
    // xTaskCreate(echo_task, "uart_echo_task", 1024, NULL, 10, NULL);
    echo_task();
}
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_event_loop.h>

#include <driver/rmt.h>
#include <soc/rmt_reg.h>

// #include <nvs_flash.h>

#define CONFIG_DHT11_POWER_PIN 2
#define CONFIG_DHT11_DATA_PIN 4

const static char *TAG = "dht11_rmt";

int temp_x10 = 123;
int humidity = 60;

#define HTTPS_TASK_NAME        "https_connection"
#define HTTPS_TASK_STACK_WORDS 10240
#define HTTPS_TASK_PRIORITY    8
#define HTTPS_RECV_BUF_LEN       1024
#define HTTPS_LOCAL_TCP_PORT     443

/*********************************************************************************
 * RMT receiver initialization
 *********************************************************************************/
static void dht11_rmt_rx_init(int gpio_pin, int channel)
{
    const int RMT_CLK_DIV            = 80;     /*!< RMT counter clock divider */
    const int RMT_TICK_10_US         = (80000000/RMT_CLK_DIV/100000);   /*!< RMT counter value for 10 us.(Source clock is APB clock) */
    const int rmt_item32_tIMEOUT_US = 1000;   /*!< RMT receiver timeout value(us) */

    rmt_config_t rmt_rx;
    rmt_rx.gpio_num                      = gpio_pin;
    rmt_rx.channel                       = channel;
    rmt_rx.clk_div                       = RMT_CLK_DIV;
    rmt_rx.mem_block_num                 = 1;
    rmt_rx.rmt_mode                      = RMT_MODE_RX;
    rmt_rx.rx_config.filter_en           = false;
    rmt_rx.rx_config.filter_ticks_thresh = 100;
    rmt_rx.rx_config.idle_threshold      = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
    ESP_ERROR_CHECK( rmt_config(&rmt_rx) );
    ESP_ERROR_CHECK( rmt_driver_install(rmt_rx.channel, 1000, 0) );

    gpio_pad_select_gpio(CONFIG_DHT11_POWER_PIN);
    ESP_ERROR_CHECK( gpio_set_direction(CONFIG_DHT11_POWER_PIN, GPIO_MODE_OUTPUT) );

    // turn power on
    gpio_set_level(CONFIG_DHT11_POWER_PIN, 1);
    sleep(2);
}

/*********************************************************************************
 * Processing the pulse data into temp and humidity
 *********************************************************************************/
static int parse_items(rmt_item32_t* item, int item_num, int *humidity, int *temp_x10)
{
    int i=0;
    unsigned rh = 0, temp = 0, checksum = 0;

    ///////////////////////////////
    // Check we have enough pulses
    ///////////////////////////////
    if(item_num < 42)  {
        ESP_LOGW(TAG, " we have NOT enough pulses %d< 42", item_num);
        return 0;
    }

    ///////////////////////////////////////
    // Skip the start of transmission pulse
    ///////////////////////////////////////
    item++;  

    ///////////////////////////////
    // Extract the humidity data     
    ///////////////////////////////
    for(i = 0; i < 16; i++, item++) 
        rh = (rh <<1) + (item->duration1 < 35 ? 0 : 1);

    ///////////////////////////////
    // Extract the temperature data
    ///////////////////////////////
    for(i = 0; i < 16; i++, item++) 
        temp = (temp <<1) + (item->duration1 < 35 ? 0 : 1);

    ///////////////////////////////
    // Extract the checksum
    ///////////////////////////////
    for(i = 0; i < 8; i++, item++) 
        checksum = (checksum <<1) + (item->duration1 < 35 ? 0 : 1);

    ///////////////////////////////
    // Check the checksum
    ///////////////////////////////
    if((((temp>>8) + temp + (rh>>8) + rh)&0xFF) != checksum) {
       printf("Checksum failure %4X %4X %2X\n", temp, rh, checksum);
       ESP_LOGI(TAG, "Checksum failure %4X %4X %2X\n", temp, rh, checksum);
       return 0;   
    }

    ///////////////////////////////
    // Store into return values
    ///////////////////////////////
    *humidity = rh>>8;
    *temp_x10 = (temp>>8)*10+(temp&0xFF);
    return 1;
}

/*********************************************************************************
 * Use the RMT receiver to get the DHT11 data
 *********************************************************************************/
static int dht11_rmt_rx(int gpio_pin, int rmt_channel, 
                        int *humidity, int *temp_x10)
{
    RingbufHandle_t rb = NULL;
    size_t rx_size = 0;
    rmt_item32_t* item;
    int rtn = 0;

    //get RMT RX ringbuffer
    ESP_ERROR_CHECK( rmt_get_ringbuf_handle(rmt_channel, &rb) );
    if(!rb) {
        return 0;
    }

    //////////////////////////////////////////////////
    // Send the 20ms pulse to kick the DHT11 into life
    //////////////////////////////////////////////////
    gpio_set_level( gpio_pin, 1 );
    gpio_set_direction( gpio_pin, GPIO_MODE_OUTPUT );
    ets_delay_us( 1000 );
    gpio_set_level( gpio_pin, 0 );
    ets_delay_us( 20000 );

    ////////////////////////////////////////////////
    // Bring rmt_rx_start & rmt_rx_stop into cache
    ////////////////////////////////////////////////
    rmt_rx_start(rmt_channel, 1);
    rmt_rx_stop(rmt_channel);

    //////////////////////////////////////////////////
    // Now get the sensor to send the data
    //////////////////////////////////////////////////
    gpio_set_level( gpio_pin, 1 );
    gpio_set_direction( gpio_pin, GPIO_MODE_INPUT );
    
    ////////////////////////////////////////////////
    // Start the RMT receiver for the data this time
    ////////////////////////////////////////////////
    rmt_rx_start(rmt_channel, 1);
    
    /////////////////////////////////////////////////
    // Pull the data from the ring buffer
    /////////////////////////////////////////////////
    item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, 2);
    if(item != NULL) {
        int n;
        n = rx_size / 4 - 0;
        //parse data value from ringbuffer.
        rtn = parse_items(item, n, humidity, temp_x10);
        //after parsing the data, return spaces to ringbuffer.
        vRingbufferReturnItem(rb, (void*) item);
    }
    /////////////////////////////////////////////////
    // Stop the RMT Receiver
    /////////////////////////////////////////////////
    rmt_rx_stop(rmt_channel);

    return rtn;
}

void app_main(void)
{
    const int gpio_pin    = CONFIG_DHT11_DATA_PIN;
    const int rmt_channel = 0;
    // const int wakeup_time_sec = 300;

    // ESP_ERROR_CHECK( nvs_flash_init() ); // TODO: do I need this???????

    // Set up the RMT_RX module
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // ESP_LOGI(TAG, "Enabling timer wakeup, %ds\n", wakeup_time_sec);
    // esp_sleep_enable_timer_wakeup(wakeup_time_sec  * 1000000);  // Wake up every 5 minutes
    
    // printf("Initializing rmt_rx..\n");
    ESP_LOGI(TAG,"Initializing rmt_rx..\n");
    dht11_rmt_rx_init(gpio_pin, rmt_channel);

    while(1) {
        if(dht11_rmt_rx(gpio_pin, rmt_channel, &humidity, &temp_x10)) {
            // printf("Temperature & humidity read %i.%iC & %i%%\n", temp_x10/10, temp_x10%10, humidity);
            ESP_LOGI(TAG, "Temperature: %i.%iC", temp_x10/10, temp_x10%10);
            ESP_LOGI(TAG, "Humidity: %i%%\n", humidity);
        }
        else {
            // printf("Sensor failure - retrying\n");
            ESP_LOGW(TAG,"Sensor failure - retrying\n");
        }
        sleep(3);
    }

    // sleep(10);
    // ESP_LOGI(TAG, "Entering deep sleep\n");
    // esp_deep_sleep_start();
}

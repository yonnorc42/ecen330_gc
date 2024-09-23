#include <stdio.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "../../components/config/hw_gc.h"
#include "driver/gptimer.h"
#include "watch.h"
#include "pin.h"
#include "lcd.h"
#include "hw.h"

#define RESOLUTION 1000000  // 1 MHz resolution
#define PRINT_INTERVAL 5000  // Milliseconds
#define MILLI_TO_MICRO 1000
#define ALARM_COUNT 10000

static const char *TAG = "lab03";

volatile uint32_t timer_ticks = 0;
volatile bool running = false;
volatile int64_t isr_max = 0;
volatile int32_t isr_cnt = 0;

// Timer callback function
bool timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data) {
    int64_t start = esp_timer_get_time();  

    if (pin_get_level(HW_BTN_A) == 0) {  
        running = true;
    }
    if (pin_get_level(HW_BTN_B) == 0) {  
        running = false;
    }
    if (pin_get_level(HW_BTN_START) == 0) {  
        running = false;
        timer_ticks = 0;
    }
    if (running) {
        timer_ticks++;
    }

    int64_t finish = esp_timer_get_time(); 
    int64_t exec_time = finish - start;  

    if (exec_time > isr_max) {
        isr_max = exec_time;
    }
    
    isr_cnt++; 

    return true;
}

// Main function which inits everything and runs loop
void app_main(void) {
    // Measure Configure I/O pins time
    int64_t start = esp_timer_get_time();

    // Configure input pins for buttons
    pin_reset(HW_BTN_A);
    pin_reset(HW_BTN_B);
    pin_reset(HW_BTN_START);
    
    pin_input(HW_BTN_A, true);
    pin_input(HW_BTN_B, true);
    pin_input(HW_BTN_START, true);

    int64_t finish = esp_timer_get_time();
    ESP_LOGI(TAG, "Configure I/O pins: %lld us", (finish - start));

    // Measure timer configuration time
    start = esp_timer_get_time();

    gptimer_handle_t gptimer = NULL;
    // Timer configuration
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = RESOLUTION,  // 1 tick = 1 microsecond
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // Set alarm action
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = ALARM_COUNT,  // Alarm every 10ms (1/100 second)
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    // Enable and start the timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    finish = esp_timer_get_time();
    ESP_LOGI(TAG, "Configure stopwatch timer: %lld us", (finish - start));
    start = esp_timer_get_time();
    ESP_LOGI(TAG, "Stopwatch update");
    finish = esp_timer_get_time();
    ESP_LOGI(TAG, "Stopwatch update timer: %lld us", (finish - start));
    lcd_init();    
    watch_init();  

    int64_t last_print_time = esp_timer_get_time();  // For 5-second print interval

    // Main loop to update the stopwatch display
    while (1) {
        watch_update(timer_ticks);

        int64_t current_time = esp_timer_get_time();
        // Measure and print ISR max time every 5 seconds
        if (current_time - last_print_time > PRINT_INTERVAL * MILLI_TO_MICRO) { 
            ESP_LOGI(TAG, "Maximum ISR execution time: %lld us", isr_max);
            last_print_time = current_time;
            isr_max = 0;  
            isr_cnt = 0;  
        }
    }
}

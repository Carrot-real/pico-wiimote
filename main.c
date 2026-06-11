#include <stdio.h>
#include "pico/stdlib.h"

const uint LED_PIN = 25; 
const uint TRIG_PIN = 15;
bool led_state = true;
volatile int64_t blink_delay = 500000; 
bool button_was_pressed = false;


int64_t flip_led(alarm_id_t id, void *user_data) {
    if (led_state == true) {
        gpio_put(LED_PIN, false);
        led_state = false;
    } else {
        gpio_put(LED_PIN, true);
        led_state = true;
    }
    return blink_delay; 
}

int main() {
    stdio_init_all(); // serial

    gpio_init(LED_PIN);
    gpio_init(TRIG_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(TRIG_PIN, GPIO_IN);
    gpio_pull_up(TRIG_PIN);

    add_alarm_in_ms(500, flip_led, NULL, false);
    while (true) {
        uint32_t all_pins = gpio_get_all();
        if (!button_was_pressed && (all_pins & (1 << 15)) == 0) {
            // \r = start of line | \033[0;32m = green | \033[0m = reset back to white
            printf("\r\033[0;32mButton pressed, light blink fast\033[0m          ");
            fflush(stdout);
            blink_delay = 50000;       
            button_was_pressed = true; 
        }  
        // 2. DETECT THE EXACT MOMENT OF RELEASE (Red Text)
        else if (button_was_pressed && (all_pins & (1 << 15)) != 0) { 
            // \r = start of line | \033[0;31m = red | \033[0m = reset back to white
            printf("\r\033[0;31mButton Released, light blink slow\033[0m         ");
            fflush(stdout);
            blink_delay = 500000;       
            button_was_pressed = false; 
        }
  
    }
}

#include <pico/time.h>
#include <stdio.h>
#include "pico/stdlib.h"

void init_bluetooth_system();
void my_wii_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

const char* BUTTON_NAMES[] = {
    "", "",       // GPIO 0, 1
    "DPAD_UP  ",  // GPIO 2
    "DPAD_DOWN",  // GPIO 3
    "DPAD_LEFT",  // GPIO 4
    "DPAD_RIGHT", // GPIO 5
    "A_BUTTON ",  // GPIO 6
    "B_TRIGGER",  // GPIO 7
    "BUTTON_1 ",  // GPIO 8
    "BUTTON_2 ",  // GPIO 9
    "MINUS_BTN",  // GPIO 10
    "PLUS_BTN ",  // GPIO 11
    "HOME_BTN ",  // GPIO 12
    "POWER_BTN"   // GPIO 13
};
const uint LED_PIN = 25; 
bool led_state = true;
volatile int64_t blink_delay = 500000; 


int main() {
    stdio_init_all(); // serial

    while (!stdio_usb_connected()) {
        sleep_ms(10); // Sit here doing nothing until Mac connects
    }
    //init_bluetooth_system();
    sleep_ms(2000);

    for (int i = 2; i <= 13; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
    }
    gpio_set_function(16, GPIO_FUNC_I2C);
    gpio_set_function(17, GPIO_FUNC_I2C);
    gpio_pull_up(16);
    gpio_pull_up(17);


    while (true) {
        uint32_t all_pins = gpio_get_all();
        if ((all_pins & (1 << 13)) == 0) {
            
            // 1. Create a fake 10-byte HCI Event packet buffer
            // The real BTstack protocol hides the MAC address starting at byte index 2!
            uint8_t fake_wii_packet[8] = {
                0x16, 0x06, 
                0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, // Fake Wii MAC Address (AA:BB:CC:DD:EE:FF)
            };

            // 2. Directly force-call your C++ packet handler manually!
            // Argument 1: HCI_EVENT_PACKET (which is 0x04)
            // Argument 3: Our fake byte array
            my_wii_packet_handler(0x04, 0, fake_wii_packet, 10);
            
            // Pause for a second so it doesn't flood your screen
            sleep_ms(1000); 
        }
        printf("\033[2J\033[H");
        for (int i = 2; i <= 13; i++) {
            if ((all_pins & (1 << i)) == 0) {
                printf("\033[0;32m%s\n", BUTTON_NAMES[i]);
            } else {
                printf("\033[0;31m%s\n", BUTTON_NAMES[i]);
            }
        }
        fflush(stdout);
        sleep_ms(50);
    }
}

//Pin-out
//GP0               VBUS
//GP1               VSYS        Power in
//GND               GND
//GP2   UP          3V3_EN
//GP3   DOWN        3V3
//GP4   LEFT        ADC_VREF
//GP5   RIGHT       GP28_A2
//GND               AGND
//GP6   A           GP27_A1
//GP7   B           GP26_A0
//GP8   1           RUN
//GP9   2           GP22
//GND               GND
//GP10  MINUS -     GP21
//GP11  PLUS  +     GP20
//GP12  HOME        GP19
//GP13  POWER       GP18
//GND               GND
//GP14              GP17        i2c0 SCL
//GP15              GP16        i2c0 SDA

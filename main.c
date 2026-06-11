#include <pico/time.h>
#include <stdio.h>
#include "pico/stdlib.h"

void init_bluetooth_system();
void my_wii_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
volatile uint16_t wii_button_report = 0x0000;


const char* BUTTON_NAMES[] = {
    "DPAD_LEFT",  //GPIO 0
    "DPAD_RIGHT", //GPIO 1
    "DPAD_DOWN",  // GPIO 2
    "DPAD_UP",    // GPIO 3
    "PLUS_BTN",   // GPIO 4
    "",           // GPIO 5
    "",           // GPIO 6
    "",           // GPIO 7
    "MINUS_BTN",  // GPIO 8
    "",           // GPIO 9
    "",           // GPIO 10
    "A_BUTTON",   // GPIO 11
    "B_TRIGGER",  // GPIO 12
    "BUTTON_1",   // GPIO 13
    "BUTTON_2",   // GPIO 14
    "HOME_BTN",   // GPIO 15
    "",           // GPIO 16
    "",           // GPIO 17
    "POWER_BTN",  // GPIO 18
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

    for (int i = 0; i <= 15; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
    }
    gpio_init(18);
    gpio_set_dir(18, GPIO_IN);
    gpio_pull_up(18);
    gpio_set_function(16, GPIO_FUNC_I2C);
    gpio_set_function(17, GPIO_FUNC_I2C);
    gpio_pull_up(16);
    gpio_pull_up(17);


    while (true) {
        uint32_t all_pins = gpio_get_all();
        wii_button_report = (~all_pins) & 0xFFFF;
        if ((all_pins & (1 << 18)) == 0) {
            
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
            sleep_ms(3000); 
        }


        printf("\033[2J\033[H");
        printf("\033[0;329m=== PACKET TEST ===\n");
        printf("Packed report: 0x%04X\n\n", wii_button_report);
        
        for (int i = 0; i <= 18; i++) {
            if (BUTTON_NAMES[i][0] == '\0') continue; // skip blank pins

            if ((wii_button_report & (1 << i)) != 0 || (i == 18 && (all_pins & (1 << 18)) == 0)) {
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
//GP0   LEFT        VBUS
//GP1   RIGHT       VSYS        Power in
//GND               GND
//GP2   DOWN        3V3_EN
//GP3   UP          3V3
//GP4   PLUS(+)     ADC_VREF
//GP5               GP28_A2
//GND               AGND
//GP6               GP27_A1
//GP7               GP26_A0
//GP8   MINUS(-)    RUN
//GP9               GP22
//GND               GND
//GP10              GP21
//GP11  A BUTTON    GP20
//GP12  B TRIGGER   GP19
//GP13  1 BUTTON    GP18        POWER BUTTON
//GND               GND
//GP14  2 BUTTON    GP17        i2c0 SCL
//GP15  HOME BUTTON GP16        i2c0 SDA

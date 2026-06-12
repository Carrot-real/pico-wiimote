#include <hardware/gpio.h>
#include <pico/time.h>
#include <stdio.h>
#include "pico/stdlib.h"

void init_bluetooth_system();
void my_wii_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);


const char* BUTTON_NAMES[] = {
    "",           // GPIO 0
    "",           // GPIO 1
    "DPAD_LEFT",  // GPIO 2
    "DPAD_RIGHT", // GPIO 3
    "DPAD_UP",    // GPIO 4
    "DPAD_DOWN",  // GPIO 5
    "A_BUTTON",   // GPIO 6
    "B_TRIGGER",  // GPIO 7
    "BUTTON_1",   // GPIO 8
    "BUTTON_2",   // GPIO 9
    "MINUS_BTN",  // GPIO 10
    "PLUS_BTN",   // GPIO 11
    "HOME_BTN",   // GPIO 12
    "SPEAKER",    // GPIO 13
    "POWER_BTN",  // GPIO 14
    "SYNC_BTN",   // GPIO 15
    "I2C_SDA",    // GPIO 16
    "I2C_SCL",    // GPIO 17
    "LED_4",      // GPIO 18
    "LED_3",      // GPIO 19
    "LED_2",      // GPIO 20
    "LED_1",      // GPIO 21
    "RUMBLE",     // GPIO 22
};

const uint LED_PIN = 25; 
const uint32_t ACTIVE_LOW_MASK = 0x0000DFFC; // flips the buttons pins which are pulled high to still say off when high
int main() {
    stdio_init_all(); // serial

    for (int i = 2; i <= 15; i++) {
        if (i == 13) continue;
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
    }
    for (int i = 18; i <= 22; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
    }
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(16);
    gpio_init(17);
    gpio_set_function(16, GPIO_FUNC_I2C);
    gpio_set_function(17, GPIO_FUNC_I2C);
    gpio_pull_up(16);
    gpio_pull_up(17);

    while (!stdio_usb_connected()) sleep_ms(10);
        //init_bluetooth_system();
    sleep_ms(2000);

    gpio_put(LED_PIN, true);
    while (true) {
        uint32_t raw_pins = gpio_get_all();
        uint32_t all_pins = raw_pins ^ ACTIVE_LOW_MASK;

        if ((all_pins & (1 << 15)) != 0) {
            uint8_t fake_wii_packet[8] = {
                0x16, 0x06, 
                0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
            };
            my_wii_packet_handler(0x04, 0, fake_wii_packet, 10);
            sleep_ms(3000); 
        }


        printf("\033[2J\033[H");
        printf("\033[0;329m=== PACKET TEST ===\n");
        printf("Packed report: 0x%04X\n\n", all_pins);

        for (int i = 2; i <= 22; i++) {
            if (BUTTON_NAMES[i][0] == '\0' || i == 16 || i == 17) continue; // skip blank pins

            if ((all_pins & (1 << i)) != 0) {
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
//GP1               VSYS     Power in
//GND               GND
//GP2   LEFT        3V3_EN
//GP3   RIGHT       3V3
//GP4   UP          ADC_VREF
//GP5   DOWN        GP28_A2  ACCELEROMETER Z
//GND               AGND
//GP6   A BUTTON    GP27_A1  ACCELEROMETER Y
//GP7   B TRIGGER   GP26_A0  ACCELEROMETER Y
//GP8   BUTTON 1    RUN
//GP9   BUTTON 2    GP22     RUMBLE MOTOR
//GND               GND
//GP10  MINUS(-)    GP21     LED 1
//GP11  PLUS(+)     GP20     LED 2
//GP12  HOME        GP19     LED 3
//GP13  SSPEAKER    GP18     LED 4
//GND               GND
//GP14 POWER BUTTON GP17     I2C0 SCL
//GP15 SYNC BUTTON  GP16     I2C0 SDA

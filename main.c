#include <hardware/gpio.h>
#include <pico/time.h>
#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"

void init_bluetooth_system();
void my_wii_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);


const char* BUTTON_NAMES[] = {
    "POWER_BTN",  // GPIO 0
    "SYNC_BTN",   // GPIO 1
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
    "I2C1_SDA",   // GPIO 14
    "I2C1_SCL",   // GPIO 15
    "I2C0_SDA",   // GPIO 16
    "I2C0_SCL",   // GPIO 17
    "LED_4",      // GPIO 18
    "LED_3",      // GPIO 19
    "LED_2",      // GPIO 20
    "LED_1",      // GPIO 21
    "RUMBLE",     // GPIO 22
};

void scan_i2c_bus() {
    printf("\n\033[0;35mScanning I2C bus...\n");
    for (int addr = 1; addr < 128; addr++) {
        uint8_t dummy_data;
        int result = i2c_read_blocking(i2c0, addr, &dummy_data, 1, false); 
        if (result >= 0) {
            printf("Found I2C device at address: 0x%02X\n", addr);
        }
    }
    printf("Scan complete.\033[0m\n");
}

const uint LED_PIN = 25; 
const uint32_t ACTIVE_LOW_MASK = 0x00001FFF; // flips the buttons pins which are pulled high to still say off when high
volatile uint32_t all_pins = 0;
volatile uint16_t global_accel_axes[3] = {0, 0, 0};
void button_edge_callback(uint gpio, uint32_t events) {
    uint32_t raw_pins = gpio_get_all();
    all_pins = raw_pins ^ ACTIVE_LOW_MASK;
}

void analog_fifo_callback(){
    global_accel_axes[0] = adc_fifo_get();
    global_accel_axes[1] = adc_fifo_get();
    global_accel_axes[2] = adc_fifo_get();
    irq_clear(ADC_IRQ_FIFO);
}


int main() {
    stdio_init_all(); // serial

    for (int i = 0; i <= 12; i++) {
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
    
    for (int i = 14; i <= 17; i++) {
    gpio_init(i);
    gpio_set_function(i, GPIO_FUNC_I2C);
    gpio_pull_up(i);
    }

    gpio_init(13);
    gpio_set_function(13, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(13);
    pwm_set_wrap(slice_num, 255);
    pwm_set_enabled(slice_num, true);

    //Acslroomiter
    adc_init();
    adc_gpio_init(26); //X 0
    adc_gpio_init(27); //Y 1
    adc_gpio_init(28); //Z 2
    adc_set_round_robin(0x07);

    adc_fifo_setup(true, false, 3, false, false);
    adc_set_clkdiv(240000); //set speed 200hz
    adc_irq_set_enabled(true);
    irq_set_exclusive_handler(ADC_IRQ_FIFO, analog_fifo_callback);
    irq_set_enabled(ADC_IRQ_FIFO, true);


    while (!stdio_usb_connected()) sleep_ms(10);
        //init_bluetooth_system();

    scan_i2c_bus();
    sleep_ms(2000); 

    gpio_put(LED_PIN, true);

    uint32_t interrupt_events = GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE;
    for (int i = 0; i <= 12; i++) {
        gpio_set_irq_enabled_with_callback(i, interrupt_events, true, &button_edge_callback);
    }
    
    adc_run(true);
    while (true) {
        static uint8_t audio_wave = 0;
        audio_wave++;
        pwm_set_gpio_level(13, audio_wave);


        if ((all_pins & (1 << 1)) != 0) {
            uint8_t fake_wii_packet[8] = {
                0x16, 0x06, 
                0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
            };
            my_wii_packet_handler(0x04, 0, fake_wii_packet, 10);
            sleep_ms(3000); 
        }

        printf("\033[2J\033[H");
        printf("\033[0;329m=== PACKET TEST ===\n");
        printf("Packed report: 0x%04X\n\n", all_pins & ACTIVE_LOW_MASK);

        for (int i = 0; i <= 22; i++) {
            if (BUTTON_NAMES[i][0] == '\0' || (14 <= i && i <= 17)) continue;

            if ((all_pins & (1 << i)) != 0) {
                printf("\033[0;32m%s\n", BUTTON_NAMES[i]);
            } else {
                printf("\033[0;31m%s\n", BUTTON_NAMES[i]);
            }
        }
        printf("\033[0;329m");
        printf("X Val: %u\n",global_accel_axes[0]);
        printf("Y Val: %u\n",global_accel_axes[1]);
        printf("Z Val: %u\n",global_accel_axes[2]);
        fflush(stdout);
        sleep_ms(50);
    }
}

//Pin-out
//GP0  POWER BUTTON VBUS
//GP1  SYNC BUTTON  VSYS     Power in
//GND               GND
//GP2  LEFT         3V3_EN
//GP3  RIGHT        3V3
//GP4  UP           ADC_VREF
//GP5  DOWN         GP28_A2  ACCELEROMETER Z
//GND               AGND
//GP6  A BUTTON     GP27_A1  ACCELEROMETER Y
//GP7  B TRIGGER    GP26_A0  ACCELEROMETER X
//GP8  BUTTON 1     RUN
//GP9  BUTTON 2     GP22     RUMBLE MOTOR
//GND               GND
//GP10 MINUS(-)     GP21     LED 1
//GP11 PLUS(+)      GP20     LED 2
//GP12 HOME         GP19     LED 3
//GP13 SPEAKER      GP18     LED 4
//GND               GND
//GP14 I2C1 SDA     GP17     I2C0 SCL
//GP15 I2C1 SCL     GP16     I2C0 SDA

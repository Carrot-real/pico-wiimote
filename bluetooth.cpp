#include "btstack_defines.h"
#include <gap.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "btstack.h"


extern "C" {
	void my_wii_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    	if (packet_type != HCI_EVENT_PACKET) {
    		return;
    	}
    	uint8_t exact_event = hci_event_packet_get_type(packet);
    	if (exact_event == HCI_EVENT_PIN_CODE_REQUEST) {
    		printf("\033[0;329mPairing request initiated from Wii\n");
    		bd_addr_t wii_mac_address;
    		bd_addr_t wii_pin_address;
    		hci_event_pin_code_request_get_bd_addr(packet, wii_mac_address); //get Wii mac address
    		wii_pin_address[0] = wii_mac_address[5];	//flips it
    		wii_pin_address[1] = wii_mac_address[4];
    		wii_pin_address[2] = wii_mac_address[3];
    		wii_pin_address[3] = wii_mac_address[2];
    		wii_pin_address[4] = wii_mac_address[1];
    		wii_pin_address[5] = wii_mac_address[0];
    		hci_send_cmd(&hci_pin_code_request_reply, wii_mac_address, wii_pin_address); // sends back as the pin
    		printf("Calculated PIN: %s\n", bd_addr_to_str(wii_mac_address));  // print the unfliped because the to str func flips internally
    		fflush(stdout);

    	}
	}
}
static btstack_packet_callback_registration_t wii_callback_object;

extern "C" {
    void init_bluetooth_system() {
        printf(" -> [C++ Engine] Bluetooth initialized successfully!\n");
        l2cap_init();
        gap_set_local_name("Nintendo RVL-CNT-01");
        gap_set_class_of_device(0x002504);
        gap_discoverable_control(1);
        wii_callback_object.callback = &my_wii_packet_handler;
		hci_add_event_handler(&wii_callback_object);

    }
}

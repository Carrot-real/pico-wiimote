#ifndef _PICO_BTSTACK_CONFIG_H
#define _PICO_BTSTACK_CONFIG_H

// Prevent redefinition warnings by checking first
#ifndef ENABLE_CLASSIC
#define ENABLE_CLASSIC
#endif

#define ENABLE_L2CAP

// Unlock the required hex dump engine for terminal debugging
#define ENABLE_PRINTF_HEXDUMP

// Limit memory tracking bounds
#define MAX_NR_HCI_CONNECTIONS 1
#define MAX_NR_L2CAP_CHANNELS  2
#define MAX_NR_L2CAP_SERVICES  2
#define NVM_NUM_LINK_KEYS      1

// Payload tracking buffer parameters
#define HCI_ACL_PAYLOAD_SIZE 1021
#define HCI_INCOMING_PRE_BUFFER_SIZE 14

#endif // _PICO_BTSTACK_CONFIG_H

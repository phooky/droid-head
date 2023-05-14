#include <stdio.h>
#include "pico/stdlib.h"
#include "pin_config.h"
#include "ssd1306.h"

#include "font/oled_font.h"

#include "pico/cyw43_arch.h"
#include "btstack.h"

static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    bd_addr_t local_addr;
    if (packet_type != HCI_EVENT_PACKET) return;
    switch(hci_event_packet_get_type(packet)){
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
            gap_local_bd_addr(local_addr);
            printf("BTstack up and running on %s.\n", bd_addr_to_str(local_addr));
            break;
        default:
            break;
    }
}

//
// Parse and print an Extended Inquiry Response data structure.
//
// The EIR data structure is composed of a series of EIR entries, each of which is composed of:
//  1 byte: length of the entry following (excluding this length byte)
//  1 byte: data type of the entry
//  n bytes: the data field of the entry
// We continue parsing EIR entries until we reach the end of the data structure, or a field with
// length 0 appears.
//
// BTStack provides an iterator for this (ad_context_t), so we'll use that instead
// of our own.
//
void dump_eir(const uint8_t* eir_data, uint8_t eir_length) {
    ad_context_t context;
    for (ad_iterator_init(&context, eir_length, eir_data);
         ad_iterator_has_more(&context);
         ad_iterator_next(&context)) {

        // Retrieve the entry length, data types, etc.
        uint8_t e_len = ad_iterator_get_data_len(&context);
        uint8_t e_type = ad_iterator_get_data_type(&context);
        const uint8_t *e_data = ad_iterator_get_data(&context);
        
        switch (e_type) {
        case BLUETOOTH_DATA_TYPE_FLAGS:
            printf("FLAGS 0x%02x", e_data[0]);
            break;
        case BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_LIST_OF_16_BIT_SERVICE_SOLICITATION_UUIDS:
            for (int i = 0; i < e_len; i += 2) {
                printf("SERV CLS %02X ", little_endian_read_16(e_data, i));
            }
            break;
        case BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_32_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_32_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_LIST_OF_32_BIT_SERVICE_SOLICITATION_UUIDS:
            for (int i = 0; i < e_len; i += 4) {
                printf("SERV CLS %04X ", little_endian_read_32(e_data, i));
            }
            break;
        case BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_LIST_OF_128_BIT_SERVICE_SOLICITATION_UUIDS:
            // eh
            break;
        case BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME:
        case BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME:
            printf("NAME %.*s ",int(e_len),e_data);
            break;
        case BLUETOOTH_DATA_TYPE_TX_POWER_LEVEL:
            printf("%d dBm", *(int8_t *) e_data);
            break;
        case BLUETOOTH_DATA_TYPE_SERVICE_DATA:
            printf("SVCDAT ");
            // 2 octets: service type UUID
            // additional data 
            printf_hexdump(e_data, e_len);
            break;
        case BLUETOOTH_DATA_TYPE_PUBLIC_TARGET_ADDRESS:
        case BLUETOOTH_DATA_TYPE_RANDOM_TARGET_ADDRESS:
            //reverse_bd_addr(e_data, address);
            //printf("%s", bd_addr_to_str(address));
            break;
        case BLUETOOTH_DATA_TYPE_APPEARANCE:
            // https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml
            printf("%02X", little_endian_read_16(e_data, 0));
            break;
        case BLUETOOTH_DATA_TYPE_ADVERTISING_INTERVAL:
            printf("%u ms", little_endian_read_16(e_data, 0) * 5 / 8);
            break;
        case BLUETOOTH_DATA_TYPE_3D_INFORMATION_DATA:
            printf("3DINFO ");
            printf_hexdump(e_data, e_len);
            break;
        case BLUETOOTH_DATA_TYPE_MANUFACTURER_SPECIFIC_DATA:
        case BLUETOOTH_DATA_TYPE_CLASS_OF_DEVICE:
        case BLUETOOTH_DATA_TYPE_SIMPLE_PAIRING_HASH_C:
        case BLUETOOTH_DATA_TYPE_SIMPLE_PAIRING_RANDOMIZER_R:
        case BLUETOOTH_DATA_TYPE_DEVICE_ID:
        case BLUETOOTH_DATA_TYPE_SECURITY_MANAGER_OUT_OF_BAND_FLAGS:
        default:
            break;
        }
    }    
}

static void hci_event_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;
    
    uint8_t event = hci_event_packet_get_type(packet);
    hci_con_handle_t connection_handle;
    uint32_t passkey;

    switch (event) {
        case BTSTACK_EVENT_STATE:
            // BTstack activated, get started
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                printf("[-] Start scanning\n");
                printf("To select device, enter advertisement number:\n");
                gap_set_scan_params(0,0x0030, 0x0030,0);
                gap_start_scan();
            } else {
                printf("Turning off.\n");
            }
            break;
        case GAP_EVENT_ADVERTISING_REPORT:
            printf("GAP AD REP: ");
            {
                uint8_t plen = packet[1];
                uint8_t evt_type = packet[2];
                uint8_t addr_type = packet[3];
                // 4-10 is address
                uint8_t dat_len = packet[11];
                uint8_t idx = 12;
                uint8_t idx_last = 12 + dat_len;
                // iterate through the EIRs
                while (idx < idx_last) {
                    uint8_t eir_sz = packet[idx++];
                    uint8_t eir_type = packet[idx];
                    printf("EIR %02x ", eir_type);
                    if (eir_type == 0x09 || eir_type == 0x08
                        || eir_type == 0x30 || eir_type == 0x24)  {
                        printf("IDENT ");
                        for (int j = 1; j < eir_sz; j++) {
                            printf("%c",packet[idx+j]);
                        }
                    }
                    idx += eir_sz;
                }
            }
            printf("\n");
            break;
        case HCI_EVENT_COMMAND_COMPLETE:
            // warn if adv enable fails
            if (hci_event_command_complete_get_command_opcode(packet) != hci_le_set_advertise_enable.opcode) break;
            if (hci_event_command_complete_get_return_parameters(packet)[0] == ERROR_CODE_SUCCESS) break;
            printf("Start advertising failed?\n");
            break;
        case HCI_EVENT_LE_META:
            if (hci_event_le_meta_get_subevent_code(packet) ==  HCI_SUBEVENT_LE_ADVERTISING_REPORT) {
                /// subevent code starts at 2, so data starts at 3
                // le_advertising_info starts at 4, +3 for length?
                printf("RPT: ");
                uint8_t plen = packet[1];
                uint8_t report_count = packet[3]; // assume 1
                if (report_count != 1) printf("\nREPORT COUNT >1 : ");
                uint8_t evt_type = packet[4];
                uint8_t addr_type = packet[5];
                // 6-11 is address
                uint8_t dat_len = packet[12];
                uint8_t idx = 13;
                uint8_t idx_last = 13 + dat_len;
                dump_eir(packet+13,dat_len);
                printf("\n");

            }
            break;
        default:
            break;
    }
}

int main()
{
    stdio_init_all();

    SSD1306 oled(OLED_RES, OLED_DC, OLED_CS);
    oled.init();
    OledTerm term(oled);

    term.print(0,0,"Terminal online.");
    term.update();

    // initialize CYW43 driver architecture (will enable BT if/because CYW43_ENABLE_BLUETOOTH == 1)
    if (cyw43_arch_init()) {
        term.print(1,0,"failed to initialise cyw43_arch");
        term.update();
        return -1;
    }

    term.print(1,0,"initialised cyw43_arch.");
    term.update();

    // inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    
    l2cap_init();
    sm_init();

    hci_event_callback_registration.callback = &hci_event_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    sm_event_callback_registration.callback = &hci_event_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    // turn on!
    hci_power_control(HCI_POWER_ON);

    btstack_run_loop_execute();
}

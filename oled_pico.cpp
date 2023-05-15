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

#define MAX_NAME 24

// Define beacon table

class Beacon {
public:
    uint16_t last_seen;
    bd_addr_t address;
    char name[MAX_NAME];
    Beacon() : last_seen(0) {
        for (uint8_t i = 0; i < 6; i++) {
            address[i] = 0;
        }
    }
        
    Beacon(bd_addr_t address_, char* name_, uint16_t last_seen_) {
        strncpy(name, name_, MAX_NAME);
        for (uint8_t i = 0; i < 6; i++) {
            address[i] = address_[i];
        }
        last_seen = last_seen_;
    }

    // Return last seen on non-match, or input last seen on update
    uint16_t update(bd_addr_t address_, uint8_t last_seen_) {
        for (uint8_t i = 0; i < 6; i++) {
            if (address[i] != address_[i]) {
                return last_seen;
            }
        }
        last_seen = last_seen_;
        return last_seen;
    }
};

#define BTAB_SZ 8

class BeaconTable {
public:
    Beacon table[BTAB_SZ];
    uint16_t msgnum;
    BeaconTable() : msgnum(0) {}
    bool update(bd_addr_t address, char* name) {
        int idx = 0;
        uint16_t minseen = 0xffff;
        msgnum++;
        for (int i = 0; i < BTAB_SZ; i++) {
            uint16_t ls = table[i].update(address, msgnum);
            if (ls == msgnum) return false;
            if (ls <= minseen) { minseen = ls, idx = i; }
        }
        table[idx] = Beacon(address, name, msgnum);
        return true;
    }
};

//    static uint16_t msgnum;
//uint16_t Beacon::msgnum = 0;

BeaconTable btable;

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
            {
                bd_addr_t address;
                gap_event_advertising_report_get_address(packet, address);
                const uint8_t plen = packet[1];
                const uint8_t evt_type = packet[2];
                const uint8_t addr_type = packet[3];
                // 4-10 is address
                const uint8_t dat_len = packet[11];
                const uint8_t idx = 12;
                const uint8_t idx_last = 12 + dat_len;

                char name[MAX_NAME];
                name[0] = 0;
                int8_t pwr_level = 0;
                
                ad_context_t context;
                for (ad_iterator_init(&context, dat_len, packet+idx);
                     ad_iterator_has_more(&context);
                     ad_iterator_next(&context)) {
                    // Retrieve the entry length, data types, etc.
                    uint8_t e_len = ad_iterator_get_data_len(&context);
                    uint8_t e_type = ad_iterator_get_data_type(&context);
                    const uint8_t *e_data = ad_iterator_get_data(&context);
        
                    switch (e_type) {
                    case BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME:
                    case BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME:
                        {
                            uint8_t nlen = ((MAX_NAME-1) < e_len)?MAX_NAME-1:e_len;
                            strncpy(name,(const char*)e_data,nlen);
                            name[nlen] = 0;
                            //printf("NAME %.*s ",int(e_len),e_data);
                        }
                        break;
                    case BLUETOOTH_DATA_TYPE_TX_POWER_LEVEL:
                        pwr_level = *(int8_t *) e_data;
                        break;
                    default:
                        break;
                    }
                }
                if (name[0] != 0) {
                    if (btable.update(address, name)) {
                        // Dump table to display!!! TODO
                        for (int i = 0; i < BTAB_SZ; i++) {
                            printf("%d: %02x %02x %02x %02x %02x %02x %s\r",i+1,
                                   btable.table[i].address[0],
                                   btable.table[i].address[1],
                                   btable.table[i].address[2],
                                   btable.table[i].address[3],
                                   btable.table[i].address[4],
                                   btable.table[i].address[5],
                                   btable.table[i].name);
                        }
                        printf("\r");
                    }
                }
            }
            break;
        case HCI_EVENT_COMMAND_COMPLETE:
            // warn if adv enable fails
            if (hci_event_command_complete_get_command_opcode(packet) != hci_le_set_advertise_enable.opcode) break;
            if (hci_event_command_complete_get_return_parameters(packet)[0] == ERROR_CODE_SUCCESS) break;
            printf("Start advertising failed?\n");
            break;
            /*
              We're processing events twice; looks like they get issued as both GAP and LE META???
        case HCI_EVENT_LE_META:
            if (hci_event_le_meta_get_subevent_code(packet) ==  HCI_SUBEVENT_LE_ADVERTISING_REPORT) {
                // 6-11 is address
                uint8_t dat_len = packet[12];
                uint8_t idx = 13;
                uint8_t idx_last = 13 + dat_len;
                dump_eir(packet+13,dat_len);
                printf("\r");

            }
            break;
            */
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

    //sm_event_callback_registration.callback = &hci_event_handler;
    //sm_add_event_handler(&sm_event_callback_registration);

    // turn on!
    hci_power_control(HCI_POWER_ON);

    btstack_run_loop_execute();
}

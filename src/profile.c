#include <stdio.h>
#include "platform_api.h"
#include "att_db.h"
#include "gap.h"
#include "btstack_event.h"
#include "btstack_defines.h"
#include "ota_service.h"
#include "profile.h"
#include "bsp_msg.h"
#include "gatt_client.h"
#include "ble_scan_adv_list.h"
#include "ad_parser.h"
#include "string.h"
#include "kv_storage.h" 
#include "eeprom.h"
// GATT characteristic handles
#include "../data/gatt.const"

const static uint8_t adv_data[] = {
    #include "../data/advertising.adv"
};

#include "../data/advertising.const"

const static uint8_t scan_data[] = {
    #include "../data/scan_response.adv"
};

#include "../data/scan_response.const"

const static uint8_t profile_data[] = {
    #include "../data/gatt.profile"
};

static const scan_phy_config_t configs[] =
{
    {
        .phy = PHY_1M,
        .type = SCAN_PASSIVE,
        .interval = 200,
        .window = 50
    }
};

// 应用版本管理
prog_ver_t prog_ver = { .major = 1, .minor = 0, .patch = 1 };
static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset,
                                  uint8_t * buffer, uint16_t buffer_size)
{
    switch (att_handle)
    {
    case HANDLE_GENERIC_INPUT:
        if (buffer)
        {
            // add your code
            return buffer_size;
        }
        else
            return 1; // TODO: return required buffer size
    case HANDLE_FOTA_VERSION:
        if (buffer)
        {
            // add your code
            return ota_read_callback(connection_handle, att_handle, offset, buffer, buffer_size);;
        }
        else
            return 1; // TODO: return required buffer size
    case HANDLE_FOTA_CONTROL:
        if (buffer)
        {
            // add your code
            return ota_read_callback(connection_handle, att_handle, offset, buffer, buffer_size);;
        }
        else
            return 1; // TODO: return required buffer size

    default:
        return 0;
    }
}

static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode,
                              uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    switch (att_handle)
    {
    case HANDLE_GENERIC_INPUT:
        // add your code
        return 0;
    case HANDLE_FOTA_CONTROL:
        // add your code
        return ota_write_callback(connection_handle, att_handle, transaction_mode, offset, buffer, buffer_size);
    
    case HANDLE_FOTA_DATA:
        // add your code
        return ota_write_callback(connection_handle, att_handle, transaction_mode, offset, buffer, buffer_size);

    default:
        return 0;
    }
}
const static ext_adv_set_en_t adv_sets_en[] = { {.handle = 0, .duration = 0, .max_events = 0} };


static void user_msg_handler(uint32_t msg_id, void *data, uint16_t size)
{
    switch (msg_id)
    {
        // add your code
    case USER_MSG_BLE_STATE_SET:
	{
		uint8_t en = size ? 1:0;
		uint8_t ret = gap_set_ext_adv_enable(en, sizeof(adv_sets_en) / sizeof(adv_sets_en[0]), adv_sets_en);
	    ret |=  gap_set_ext_scan_enable(en, 0, 0, 0);   // start continuous scanning
		UserQue_SendMsg(USER_MSG_BLE_STATE,NULL,ret);
	}
        break;

    default:
        ;
    }
}


static void setup_adv(void)
{
    gap_set_ext_adv_para(0,
                            CONNECTABLE_ADV_BIT | SCANNABLE_ADV_BIT | LEGACY_PDU_BIT,
                            800, 800,                  // Primary_Advertising_Interval_Min, Primary_Advertising_Interval_Max
                            PRIMARY_ADV_ALL_CHANNELS,  // Primary_Advertising_Channel_Map
                            BD_ADDR_TYPE_LE_RANDOM,    // Own_Address_Type
                            BD_ADDR_TYPE_LE_PUBLIC,    // Peer_Address_Type (ignore)
                            NULL,                      // Peer_Address      (ignore)
                            ADV_FILTER_ALLOW_ALL,      // Advertising_Filter_Policy
                            0x00,                      // Advertising_Tx_Power
                            PHY_1M,                    // Primary_Advertising_PHY
                            0,                         // Secondary_Advertising_Max_Skip
                            PHY_1M,                    // Secondary_Advertising_PHY
                            0x00,                      // Advertising_SID
                            0x00);                     // Scan_Request_Notification_Enable
    gap_set_ext_adv_data(0, sizeof(adv_data), (uint8_t*)adv_data);
    gap_set_ext_scan_response_data(0, sizeof(scan_data), (uint8_t*)scan_data);
    
}

static void setup_scan(void)
{
	gap_set_ext_scan_para(BD_ADDR_TYPE_LE_RANDOM, SCAN_ACCEPT_ALL_EXCEPT_NOT_DIRECTED,
						  sizeof(configs) / sizeof(configs[0]),
						  configs);
    
}

uint8_t adv_report_data[36];
static void user_packet_handler(uint8_t packet_type, uint16_t channel, const uint8_t *packet, uint16_t size)
{
    static const bd_addr_t rand_addr = { 0xD2, 0xBD, 0xE9, 0xD3, 0x82, 0x3E };
	bd_addr_t peer_addr; 
	uint8_t name[32] = {0};
	uint16_t nameLen;
    uint8_t event = hci_event_packet_get_type(packet);
    const btstack_user_msg_t *p_user_msg;
    if (packet_type != HCI_EVENT_PACKET) return;

    switch (event)
    {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING)
            break;
        gap_set_adv_set_random_addr(0, rand_addr);
		gap_set_random_device_address(rand_addr);
        setup_adv();
		setup_scan();
        break;

    case HCI_EVENT_COMMAND_COMPLETE:
        switch (hci_event_command_complete_get_command_opcode(packet))
        {
        // add your code to check command complete response
        // case :
        //     break;
        default:
            break;
        }
        break;

    case HCI_EVENT_LE_META:
        switch (hci_event_le_meta_get_subevent_code(packet))
        {
			case HCI_SUBEVENT_LE_EXTENDED_ADVERTISING_REPORT:
            {
                const le_ext_adv_report_t *report = decode_hci_le_meta_event(packet, le_meta_event_ext_adv_report_t)->reports;
				memcpy(adv_report_data,report->data,report->data_len);
				const uint8_t *pAD = ad_data_from_type(report->data_len,adv_report_data,0x09,&nameLen);
				if(nameLen > 32) break;
				if(pAD)memcpy(name,pAD,nameLen);
				name[nameLen] = 0;
                reverse_bd_addr(report->address, peer_addr);
				addAdvNode(advListHead,(char*)name,peer_addr,report->rssi);
				//printf("rssi:%d\n",report->rssi);
            }
            break;
        case HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE:
        case HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE_V2:
            att_set_db(decode_hci_le_meta_event(packet, le_meta_event_enh_create_conn_complete_t)->handle,
                       profile_data);
            break;
        default:
            break;
        }

        break;

    case HCI_EVENT_DISCONNECTION_COMPLETE:
       
        break;

    case ATT_EVENT_CAN_SEND_NOW:
        // add your code
        break;

    case BTSTACK_EVENT_USER_MSG:
        p_user_msg = hci_event_packet_get_user_msg(packet);
        user_msg_handler(p_user_msg->msg_id, p_user_msg->data, p_user_msg->len);
        break;

    default:
        break;
    }
}

static btstack_packet_callback_registration_t hci_event_callback_registration;
#define DB_FLASH_ADDRESS  0x207D000
#define  EPPROM_DATA_DB_ADDR     (0x0)
int db_write_to_flash(const void *db, const int size)
{
    platform_printf("write to flash, size = %d\n", size);
    program_flash(DB_FLASH_ADDRESS, (const uint8_t *)db, size);

    return KV_OK;
}

int read_from_flash(void *db, const int max_size)
{
    memcpy(db, (void *)DB_FLASH_ADDRESS, max_size);
    return KV_OK;
}

int db_write_to_eeprom(const void *db, const int size)
{
	eeprom_app_writes(&At24C128CMoudle,EPPROM_DATA_DB_ADDR,(uint8_t*)db,size);
//	platform_printf("write to eeprom, size = %d\n", size);
//	printf_hexdump(db,size);
    return KV_OK;
}

int read_from_eeprom(void *db, const int size)
{
	eeprom_app_reads(&At24C128CMoudle,EPPROM_DATA_DB_ADDR,(uint8_t*)db,size);
//	platform_printf("read to eeprom, size = %d\n", size);
//	printf_hexdump(db,size);
    return KV_OK;
}

uint32_t setup_profile(void *data, void *user_data)
{
    platform_printf("setup profile\n");
    // Note: security has not been enabled.
#if HARD_VERSION == 2		
#ifdef DB_TO_FLASH	
	kv_init(db_write_to_flash, read_from_flash);
#else
	kv_init(db_write_to_eeprom, read_from_eeprom);
#endif	
#else
	kv_init(db_write_to_flash, read_from_flash);
#endif
    att_server_init(att_read_callback, att_write_callback);
    hci_event_callback_registration.callback = &user_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    att_server_register_packet_handler(&user_packet_handler);
	gatt_client_register_handler(user_packet_handler);
    return 0;
}


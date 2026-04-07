#ifndef MKS_WIFI_H
#define MKS_WIFI_H
#include "../../MarlinCore.h"
#include "../../inc/MarlinConfig.h"
#include "../../libs/Segger/log.h"
#include "mks_wifi_settings.h"
#include "../../gcode/queue.h"

void mks_wifi_out_add(uint8_t *data, uint32_t size);


#define MKS_OUT_BUFF_SIZE (ESP_PACKET_DATA_MAX_SIZE)
#define MKS_IN_BUFF_SIZE (ESP_PACKET_DATA_MAX_SIZE + 30)

#define MKS_TOTAL_PACKET_SIZE (ESP_PACKET_DATA_MAX_SIZE+10)
#define WIFI_MODE_STA				(uint8_t)2
#define WIFI_MODE_AP				(uint8_t)1

typedef struct
{
	uint8_t type; 
	uint16_t dataLen;
	uint8_t *data; 
} ESP_PROTOC_FRAME;

#define ESP_PROTOC_HEAD				(uint8_t)0xa5
#define ESP_PROTOC_TAIL				(uint8_t)0xfc

#define ESP_TYPE_NET				(uint8_t)0x0
#define ESP_TYPE_GCODE				(uint8_t)0x1
#define ESP_TYPE_FILE_FIRST			(uint8_t)0x2
#define ESP_TYPE_FILE_FRAGMENT		(uint8_t)0x3
#define ESP_TYPE_WIFI_LIST		    (uint8_t)0x4

#define ESP_PACKET_DATA_MAX_SIZE	1024
#define ESP_SERIAL_OUT_MAX_SIZE		1024

#define ESP_NET_WIFI_CONNECTED		(uint8_t)0x0A
#define ESP_NET_WIFI_EXCEPTION		(uint8_t)0x0E

#define NOP	__asm volatile ("nop")

// Wi-Fi Menu Status States
#define WIFI_STATE_IDLE				(uint8_t)0
#define WIFI_STATE_SCANNING			(uint8_t)1
#define WIFI_STATE_SCAN_DONE		(uint8_t)2
#define WIFI_STATE_CONNECTING		(uint8_t)3
#define WIFI_STATE_CONNECTED		(uint8_t)4
#define WIFI_STATE_CONNECT_FAILED	(uint8_t)5

// Wi-Fi Credentials Buffer Size
#define WIFI_SSID_MAX_LEN			32
#define WIFI_PASS_MAX_LEN			64
#define WIFI_MAX_SCAN_NETWORKS		10

// Network scan result structure
typedef struct {
	char ssid[WIFI_SSID_MAX_LEN];
	int8_t rssi;
	uint8_t auth_mode;
} WIFI_SCAN_RESULT;

typedef struct {
	bool		connected;
	uint8_t		ip[4];
	uint8_t		mode;
	char		net_name[32];
	// Menu UI additions
	uint8_t		state;
	uint8_t		show_status_once;
	char		ssid_buf[WIFI_SSID_MAX_LEN];
	char		pass_buf[WIFI_PASS_MAX_LEN];
	WIFI_SCAN_RESULT	scan_results[WIFI_MAX_SCAN_NETWORKS];
	uint8_t		scan_count;
	uint32_t	connecting_start_time;	// millis() timestamp when connection started
	// State flags
	uint8_t		scanning_in_progress;
	uint8_t		connect_needed;
	uint8_t		scan_results_updated;
} MKS_WIFI_INFO;

extern MKS_WIFI_INFO mks_wifi_info;


void mks_wifi_init(void);

void mks_wifi_set_param(void);

uint8_t mks_wifi_input(uint8_t data);
void mks_wifi_parse_packet(ESP_PROTOC_FRAME *packet);

uint16_t mks_wifi_build_packet(uint8_t *packet, ESP_PROTOC_FRAME *esp_frame);

uint8_t mks_wifi_check_packet(uint8_t *in_data);
uint8_t check_char_allowed(char data);

void mks_wifi_send(uint8_t *packet, uint16_t size);

// --- Wi-Fi API for Menu UI ---

// Query Wi-Fi state
uint8_t mks_wifi_get_state(void);
bool mks_wifi_is_connected(void);
bool mks_wifi_has_ip(void);
void mks_wifi_get_ip_string(char *buffer, uint8_t size);
void mks_wifi_get_current_ssid(char *buffer, uint8_t size);
uint8_t mks_wifi_get_mode(void);

// Scan management
void mks_wifi_request_scan(void);
bool mks_wifi_has_scan_results(void);
uint8_t mks_wifi_get_scan_count(void);
void mks_wifi_get_scan_ssid(uint8_t index, char *buffer, uint8_t size);
int8_t mks_wifi_get_scan_rssi(uint8_t index);

// Credentials management
void mks_wifi_set_ssid(const char *ssid);
void mks_wifi_set_password(const char *password);
void mks_wifi_set_credentials(const char *ssid, const char *password);
void mks_wifi_get_ssid_buffer(char *buffer, uint8_t size);
void mks_wifi_get_password_buffer(char *buffer, uint8_t size);

// Connection management
void mks_wifi_connect(void);
void mks_wifi_reconnect(void);
void wifi_looping(void);

#endif

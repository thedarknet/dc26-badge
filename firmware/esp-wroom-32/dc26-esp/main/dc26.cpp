#if 1
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "string.h"
#include "lib/System.h"
#include "lib/wifi/WiFi.h"

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

extern "C" {
		void app_main(void);
		static void rx_task(void *);
}

void init() {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

static void tx_task()
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        sendData(TX_TASK_TAG, "Hello world");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void rx_task(void *)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
    			uart_write_bytes(UART_NUM_1, (const char *)data, rxBytes);
        }
    }
    free(data);
}

std::string ssid = "dc26";
std::string passwd = "1234567890";
wifi_auth_mode_t ap_mode = WIFI_AUTH_WPA_WPA2_PSK;

static char tag[] = "myTag";

/*
class MyWiFiEventHandler: public WiFiEventHandler {

		esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip) {
					ESP_LOGD(tag, "MyWiFiEventHandler(Class): staGotIp");
					return ESP_OK;
		}
};
*/

void app_main()
{
	nvs_flash_init();
	init();
	xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
	System::logSystemInfo();
	//WiFiEventHandler *handler = new MyWiFiEventHandler();
	wifi_config_t wifi_config;
	bool isHidden = false;
	uint8_t max_con = 4;
	uint16_t beacon_interval = 1000;
	WIFI wifi;
	wifi.initWiFiConfig(wifi_config, ssid, passwd, WIFI_AUTH_WPA2_PSK, isHidden, max_con, beacon_interval);
	tcpip_adapter_ip_info_t ipInfo;
	wifi.initAdapterIp(ipInfo);
	dhcps_lease_t l;
	wifi.initDHCPSLeaseInfo(l);
	wifi.wifi_start_access_point(wifi_config,ipInfo,l);
	vTaskDelete(NULL);
//    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}

#endif

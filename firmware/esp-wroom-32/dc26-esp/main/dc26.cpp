#if 1
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "string.h"
#include "lib/System.h"
#include "lib/wifi/WiFi.h"
#include "lib/i2c.hpp"
#include "lib/ssd1306.h"
#include "stm_to_esp_generated.h"
#include "esp_to_stm_generated.h"
#include "lib/net/HttpServer.h"
#include "lib/FATFS_VFS.h"

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

extern "C" {
		void app_main(void);
		static void rx_task(void *);
		static void generalCmdTask(void *);
}

struct OutgoingMsg {
	uint8_t *MsgBytes;
	uint32_t Size;
};

static const int STM_TO_ESP_MSG_QUEUE_SIZE = 10;
static const int STM_TO_ESP_MSG_ITEM_SIZE = sizeof(darknet7::STMToESPRequest*);
static StaticQueue_t STMToESPQueue;
static QueueHandle_t STMToESPQueueHandle = nullptr;
static uint8_t STMToESPBuffer[STM_TO_ESP_MSG_QUEUE_SIZE*STM_TO_ESP_MSG_ITEM_SIZE];
static const int ESP_TO_STM_MSG_QUEUE_SIZE = 10;
static const int ESP_TO_STM_MSG_ITEM_SIZE = sizeof(OutgoingMsg);
static uint8_t ESPToSTMBuffer[ESP_TO_STM_MSG_QUEUE_SIZE*ESP_TO_STM_MSG_ITEM_SIZE];
static StaticQueue_t ESPToSTMQueue;
static QueueHandle_t ESPToSTMQueueHandle = nullptr;
static HttpServer Port80WebServer;
FATFS_VFS *FatFS = new FATFS_VFS("/spiflash", "storage");

////
// Basic plan:
//    uart receive task, deserialized via flat buffers, pushes messages to 
//    appropriate cmd queue, cmd queue handles messages and pushed response
//    to out going queue as a flat buffer class, then out going task will
//    serial via flat buffers then send over uart to stm32
void init() {
	//create queues
	STMToESPQueueHandle = xQueueCreateStatic(STM_TO_ESP_MSG_QUEUE_SIZE, STM_TO_ESP_MSG_ITEM_SIZE, STMToESPBuffer, &STMToESPQueue );
	ESPToSTMQueueHandle = xQueueCreateStatic(ESP_TO_STM_MSG_QUEUE_SIZE, ESP_TO_STM_MSG_ITEM_SIZE, ESPToSTMBuffer, &ESPToSTMQueue );

	FatFS->mount();
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


static void tx_task(void *)
{
	static const char *TX_TASK_TAG = "TX_TASK";
	esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
	OutgoingMsg *msgToSend =0;
	while (1) {
		if(xQueueReceive(ESPToSTMQueueHandle, &msgToSend, ( TickType_t ) 1000)) {
			uint32_t bytesSent = 0;
			while(bytesSent < msgToSend->Size) {
    			const int txBytes = uart_write_bytes(UART_NUM_1, (char *)msgToSend->MsgBytes+bytesSent
					, msgToSend->Size-bytesSent);
				bytesSent+=txBytes;
				vTaskDelay(portTICK_PERIOD_MS*10);
			}
		}
		//sendData(TX_TASK_TAG, "Hello world");
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}
//
// wait on queue from uart 
static void generalCmdTask(void *) {
    while (1) {
        vTaskDelay(20000 / portTICK_PERIOD_MS);
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


class MyWiFiEventHandler: public WiFiEventHandler {
public:
	const char *logTag = "MyWiFiEventHandler";
public:
	virtual esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip) {
		ESP_LOGD(logTag, "MyWiFiEventHandler(Class): staGotIp");
		return ESP_OK;
	}
	virtual esp_err_t apStart() {
		ESP_LOGI(logTag, "MyWiFiEventHandler(Class): apStart starting web server");
		Port80WebServer.start(80,false);
		return ESP_OK;
	}
	virtual esp_err_t apStop() {
		ESP_LOGI(logTag, "MyWiFiEventHandler(Class): apStop stopping web server");
		Port80WebServer.stop();
		return ESP_OK;
	}
	virtual esp_err_t wifiReady() {
		ESP_LOGI(logTag, "MyWiFiEventHandler(Class): wifi ready");
		return ESP_OK;
	}
};


ESP32_I2CMaster I2cDisplay(GPIO_NUM_19,GPIO_NUM_18,1000000, I2C_NUM_0, 1024, 1024);
static char tag[] = "main";

void app_main()
{
	nvs_flash_init();
	if(SSD1306_Init(&I2cDisplay)>0) {
		ESP_LOGI(tag,"display init successful");
		SSD1306_DrawFilledRectangle(0,0,64,32,SSD1306_COLOR_WHITE);
		SSD1306_UpdateScreen();
	} else {
		ESP_LOGI(tag,"display init UN-successful");
	}
	init();
	xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
	System::logSystemInfo();
	wifi_config_t wifi_config;
	bool isHidden = false;
	uint8_t max_con = 4;
	uint16_t beacon_interval = 1000;
	WIFI wifi;
	WiFiEventHandler *handler = new MyWiFiEventHandler();
	wifi.setWifiEventHandler(handler);
	wifi.initWiFiConfig(wifi_config, ssid, passwd, WIFI_AUTH_WPA2_PSK, isHidden, max_con, beacon_interval);
	tcpip_adapter_ip_info_t ipInfo;
	wifi.initAdapterIp(ipInfo);
	dhcps_lease_t l;
	wifi.initDHCPSLeaseInfo(l);
	wifi.wifi_start_access_point(wifi_config,ipInfo,l);
	xTaskCreate(generalCmdTask, "generalCmdTask", 1024*2, NULL, configMAX_PRIORITIES, NULL);
   xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
	vTaskDelete(NULL);
}

#endif

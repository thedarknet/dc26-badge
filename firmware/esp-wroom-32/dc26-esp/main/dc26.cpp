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
#include "mcu_to_mcu.h"

static const int RX_BUF_SIZE = 1024;
#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

extern "C" {
		void app_main(void);
		static void generalCmdTask(void *);
}

static HttpServer Port80WebServer;
FATFS_VFS *FatFS = new FATFS_VFS("/spiflash", "storage");

////
// Basic plan:
//    uart receive task, deserialized via flat buffers, pushes messages to 
//    appropriate cmd queue, cmd queue handles messages and pushed response
//    to out going queue as a flat buffer class, then out going task will
//    serial via flat buffers then send over uart to stm32
void init() {
	FatFS->mount();
}

//
// wait on queue from uart 
static void generalCmdTask(void *) {
    while (1) {
        vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
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
MCUToMCUTask ProcToProc("ProcToProc");

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
	ProcToProc.init(TXD_PIN,RXD_PIN,RX_BUF_SIZE);
	ProcToProc.start();

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
   //xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
	vTaskDelete(NULL);
}

#endif

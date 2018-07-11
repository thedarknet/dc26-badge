
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
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
#include "command_handler.h"

#include "dc26_ble/ble.h"

static const int RX_BUF_SIZE = 1024;
#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

extern "C" {
		void app_main(void);
		static void generalCmdTask(void *);
}

static HttpServer Port80WebServer;
FATFS_VFS *FatFS = new FATFS_VFS("/spiflash", "storage");
static xQueueHandle gpio_evt_queue = NULL;
ESP32_I2CMaster I2cDisplay(GPIO_NUM_19,GPIO_NUM_18,1000000, I2C_NUM_0, 1024, 1024);
CmdHandlerTask CmdTask("CmdTask");
BluetoothTask BTTask("BluetoothTask");
MCUToMCUTask ProcToProc(&CmdTask, "ProcToProc");

MCUToMCUTask &getMCUToMCU() {
	return ProcToProc;
}

static void IRAM_ATTR gpio_isr_handler(void* arg) {
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg) {
	uint32_t io_num;
	for(;;) {
		if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
			uint32_t level = (uint32_t) gpio_get_level((gpio_num_t)io_num);
			printf("GPIO[%d] intr, val: %d\n", io_num, level);
			//build test message
			flatbuffers::FlatBufferBuilder fbb;
			System::logSystemInfo();
			esp_chip_info_t chip;
			System::getChipInfo(&chip);
			auto info = darknet7::CreateESPSystemInfoDirect(fbb,
				System::getFreeHeapSize()
				, chip.model
				, chip.cores
				, chip.revision
				, chip.features
				, System::getIDFVersion());
			flatbuffers::Offset<darknet7::ESPToSTM> of = darknet7::CreateESPToSTM(fbb,1U,darknet7::ESPToSTMAny_ESPSystemInfo, info.Union());
			darknet7::FinishSizePrefixedESPToSTMBuffer(fbb,of);
			ProcToProc.send(fbb);
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

#define ESP_INTR_FLAG_DEFAULT 0

void initButton() {
	gpio_config_t io_conf;
	//interrupt of rising edge
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	//bit mask of the pins, use GPIO0
#define GPIO_INPUT_IO_0 (1ULL << GPIO_NUM_0)
	io_conf.pin_bit_mask = GPIO_INPUT_IO_0;
	//set as input mode    
	io_conf.mode = GPIO_MODE_INPUT;
	//enable pull-up mode
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpio_config(&io_conf);
	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
	//start gpio task
	xTaskCreate(gpio_task_example, "gpio_task_example", 6048, NULL, 10, NULL);
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	//hook isr handler for specific gpio pin
	gpio_isr_handler_add(GPIO_NUM_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
}

////
// Basic plan:
//    uart receive task, deserialized via flat buffers, pushes messages to 
//    appropriate cmd queue, cmd queue handles messages and pushed response
//    to out going queue as a flat buffer class, then out going task will
//    serial via flat buffers then send over uart to stm32
void init() {
	FatFS->mount();
	initButton();
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



static char tag[] = "main";

void app_main()
{
	nvs_flash_erase(); // FIXME:  Remove this
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
	CmdTask.init();
	CmdTask.start();
	BTTask.init();
	BTTask.start();

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
	vTaskDelete(NULL);
}

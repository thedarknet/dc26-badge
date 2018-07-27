#include <esp_log.h>
#include <errno.h>
#include <lwip/sockets.h>
//#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>

#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include "sdkconfig.h"
#include "WiFi.h"

const char WIFI::LogTag[] = "WIFI";

WIFI::WIFI() : mpWifiEventHandler(0), EventLoopStarted(false), InitCalled(false) {

}

WIFI::~WIFI() {

}

esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
	WIFI *pWiFi = (WIFI *)ctx;   // retrieve the WiFi object from the passed in context.

	// Invoke the event handler.
	esp_err_t rc;
	if (pWiFi->getWifiEventHandler() != nullptr) {
		rc = pWiFi->getWifiEventHandler()->getEventHandler()(pWiFi->getWifiEventHandler(), event);
	} else {
		rc = ESP_OK;
	}
	return rc;
}

void WIFI::setWifiEventHandler(WiFiEventHandler *wifiEventHandler) {
	mpWifiEventHandler = wifiEventHandler;
}

//create wificonfig that has ssid and open auth
void WIFI::initWiFiConfig(wifi_config_t &wifi_config, const std::string &ssid) { 
	initWiFiConfig(wifi_config, ssid, std::string(""), WIFI_AUTH_OPEN, false, 4, 100);
}

void WIFI::initWiFiConfig(wifi_config_t &wifi_config, const std::string &ssid, const std::string &passwd, wifi_auth_mode_t authMode, bool isHidden, uint8_t max_con, uint16_t beacon_interval) {
	memset(&wifi_config,0,sizeof(wifi_config_t));
	memcpy(&wifi_config.ap.ssid[0],ssid.c_str(),ssid.length());
	wifi_config.ap.ssid_len=0;
	memcpy(&wifi_config.ap.password,passwd.c_str(),passwd.length());
	wifi_config.ap.channel = 0; //don't set a channel
	wifi_config.ap.authmode = authMode;
	wifi_config.ap.ssid_hidden = isHidden;
	wifi_config.ap.max_connection = max_con;
	wifi_config.ap.beacon_interval = beacon_interval;
}

void WIFI::initAdapterIp(tcpip_adapter_ip_info_t &ipInfo) {
	inet_pton(AF_INET, "192.168.10.2", &ipInfo.ip);
	inet_pton(AF_INET, "192.168.10.1", &ipInfo.gw);
	inet_pton(AF_INET, "255.255.255.0", &ipInfo.netmask);
}

void WIFI::initAdapterIp(tcpip_adapter_ip_info_t &ipInfo, std::string &ip, std::string &gw, std::string netmask) {
	//IP4_ADDR(&ipInfo.ip, 192,168,1,2);
	inet_pton(AF_INET, ip.c_str(), &ipInfo.ip);
	inet_pton(AF_INET, gw.c_str(), &ipInfo.gw);
	inet_pton(AF_INET, netmask.c_str(), &ipInfo.netmask);
}

void WIFI::initDHCPSLeaseInfo(dhcps_lease_t &l) {
		l.enable = true;
		IP4_ADDR(&l.start_ip, 192,168,10,10);
		IP4_ADDR(&l.end_ip,192,168,10,100);
}

void WIFI::initDHCPSLeaseInfo(dhcps_lease_t &l, const std::string &start_ip, const std::string &end_ip) {
		l.enable = true;
		inet_pton(AF_INET, start_ip.c_str(), &l.start_ip);
		inet_pton(AF_INET, end_ip.c_str(), &l.end_ip);
}

wifi_mode_t WIFI::getMode() {
	wifi_mode_t mode;
	esp_err_t err = esp_wifi_get_mode(&mode);
	if(err==ESP_OK) {
		return mode;
	}
	return WIFI_MODE_NULL;
}

bool WIFI::init() {
	// If we have already started the event loop, then change the handler otherwise
 	// start the event loop.
	if (EventLoopStarted) {
		esp_event_loop_set_cb(wifi_event_handler, this);   // Returns the old handler.
	} else {
		esp_err_t errRc = ::esp_event_loop_init(wifi_event_handler, this); 
	  	// Initialze the event handler.
		if (errRc != ESP_OK) {
			ESP_LOGE(LogTag, "esp_event_loop_init: rc=%d", errRc);
			return false;
		}
		EventLoopStarted = true;
	}
	// Now, one way or another, the event handler is WiFi::eventHandler.
	if (!InitCalled) {
		::nvs_flash_init();
		::tcpip_adapter_init();
		wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
		esp_err_t errRc = ::esp_wifi_init(&cfg);
		if (errRc != ESP_OK) {
			ESP_LOGE(LogTag, "esp_wifi_init: rc=%d", errRc);
			return false;
		}
		errRc = ::esp_wifi_set_storage(WIFI_STORAGE_RAM);
		if (errRc != ESP_OK) {
			ESP_LOGE(LogTag, "esp_wifi_set_storage: rc=%d", errRc);
			return false;
		}
	}
	InitCalled = true;
	return true;
}

bool WIFI::scan(bool showHidden) {
	if(!init()) {
		return false;
	}
	esp_wifi_set_mode(WIFI_MODE_APSTA);
	wifi_config_t wifi_config;
	memset(&wifi_config.sta,0,sizeof(wifi_sta_config_t));
	wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
	wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
	esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
	esp_wifi_start();

	wifi_scan_config_t conf;
	memset(&conf, 0, sizeof(conf));
	conf.show_hidden = showHidden;

	return ESP_OK==::esp_wifi_scan_start(&conf, false);
}

/**
 * Creates a new Wifi-AP on ESP32
 * */
void WIFI::wifi_start_access_point(wifi_config_t &wifi_config, tcpip_adapter_ip_info_t &ipInfo, 
		dhcps_lease_t &lease) {

	init();

	esp_wifi_set_mode(WIFI_MODE_APSTA);
	esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
	esp_wifi_start();

	tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
	if(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ipInfo) == ESP_OK) {
		tcpip_adapter_dhcps_option(
			(tcpip_adapter_option_mode_t)TCPIP_ADAPTER_OP_SET,
			(tcpip_adapter_option_id_t)TCPIP_ADAPTER_REQUESTED_IP_ADDRESS,
			(void*)&lease, sizeof(dhcps_lease_t)
		);
		if(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP) != ESP_OK) {
			ESP_LOGE(LogTag, "fail");
		}
	}
}

void WIFI::stopWiFi() {
	esp_wifi_stop();
}

#if 0
extern "C" {
	void app_main();
}
//
//--------
//  Main
//--------
void app_main() {
	nvs_flash_init();
	wifi_config_t wifi_config;
	std::string ssid("ESP32");
	std::string passwd("1234567890");
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
}
#endif


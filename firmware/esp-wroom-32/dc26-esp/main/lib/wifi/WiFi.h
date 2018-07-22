#ifndef WIFI_H
#define WIFI_H

#include <esp_log.h>
#include <errno.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <string>
#include "WiFiEventHandler.h"
#include "sdkconfig.h"

class WIFI {
public:
	static const char LogTag[];
public:
	WIFI();
	~WIFI();
public:
	wifi_mode_t getMode();
	void initWiFiConfig(wifi_config_t &wifi_config, const std::string &ssid); 
	void initWiFiConfig(wifi_config_t &wifi_config, const std::string &ssid, const std::string &passwd, wifi_auth_mode_t authMode, bool isHidden, uint8_t max_con, uint16_t beacon_interval);
	void initAdapterIp(tcpip_adapter_ip_info_t &ipInfo);
	void initAdapterIp(tcpip_adapter_ip_info_t &ipInfo, std::string &ip, std::string &gw, std::string netmask);
	void initDHCPSLeaseInfo(dhcps_lease_t &l);
	void initDHCPSLeaseInfo(dhcps_lease_t &l, const std::string &start_ip, const std::string &end_ip);
	void wifi_start_access_point(wifi_config_t &wifi_config, tcpip_adapter_ip_info_t &ipInfo, dhcps_lease_t &lease);
	void stopWiFi();
	void setWifiEventHandler(WiFiEventHandler *wifiEventHandler);
	const WiFiEventHandler *getWifiEventHandler() const {return mpWifiEventHandler;}
	WiFiEventHandler *getWifiEventHandler() {return mpWifiEventHandler;}
protected:
	WiFiEventHandler*   mpWifiEventHandler;
};

#endif

#include "command_handler.h"
#include "stm_to_esp_generated.h"
#include "esp_to_stm_generated.h"
#include "lib/System.h"
#include "lib/net/HttpServer.h"
#include "mcu_to_mcu.h"
#include "dc26.h"
#include "dc26_ble/ble.h"

static HttpServer Port80WebServer;

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
	virtual esp_err_t staConnected(system_event_sta_connected_t info) {
		return ESP_OK;
	}
	virtual esp_err_t staDisconnected(system_event_sta_disconnected_t info) {
		return ESP_OK;
	}
	virtual esp_err_t staScanDone(system_event_sta_scan_done_t info) {
		return ESP_OK;
	}
	virtual esp_err_t staAuthChange(system_event_sta_authmode_change_t info) {
		return ESP_OK;
	}
	virtual esp_err_t staStart() {
		return ESP_OK;
	}
	virtual esp_err_t staStop() {
		return ESP_OK;
	}
};


const char *CmdHandlerTask::LOGTAG = "CmdHandlerTask";
static StaticQueue_t InCommingQueue;
static uint8_t CommandBuffer[CmdHandlerTask::STM_TO_ESP_MSG_QUEUE_SIZE
		* CmdHandlerTask::STM_TO_ESP_MSG_ITEM_SIZE] = { 0 };

CmdHandlerTask::CmdHandlerTask(const std::string &tName, uint16_t stackSize,
		uint8_t p) :
		Task(tName, stackSize, p), InCommingQueueHandle(nullptr) {

}

bool CmdHandlerTask::init() {
	ESP_LOGI(LOGTAG, "INIT");
	InCommingQueueHandle = xQueueCreateStatic(STM_TO_ESP_MSG_QUEUE_SIZE,
			STM_TO_ESP_MSG_ITEM_SIZE, &CommandBuffer[0], &InCommingQueue);
	if (InCommingQueueHandle == nullptr) {
		ESP_LOGI(LOGTAG, "Failed creating incomming queue");
	}
	WiFiEventHandler *handler = new MyWiFiEventHandler();
	wifi.setWifiEventHandler(handler);
	return true;
}

CmdHandlerTask::~CmdHandlerTask() {
}

wifi_auth_mode_t ap_mode = WIFI_AUTH_WPA_WPA2_PSK;

wifi_auth_mode_t convertAuthMode(darknet7::WifiMode m) {
	switch(m) {
	case darknet7::WifiMode_OPEN:
		return WIFI_AUTH_OPEN;
		break;
	case darknet7::WifiMode_WPA2:
		return WIFI_AUTH_WPA2_PSK;
		break;
	case darknet7::WifiMode_WPA:
		return WIFI_AUTH_WPA_WPA2_PSK;
		break;
	default:
		return WIFI_AUTH_OPEN;
		break;
	}
}

void CmdHandlerTask::run(void *data) {
	ESP_LOGI(LOGTAG, "CmdHandler Task started");
	MCUToMCUTask::Message *m=0;
	while (1) {
		if (xQueueReceive(getQueueHandle(), &m, (TickType_t) 1000 / portTICK_PERIOD_MS)) {
			ESP_LOGI(LOGTAG, "got message from queue");
			const darknet7::STMToESPRequest *msg = m->asSTMToESP();
			ESP_LOGI(LOGTAG, "message type is: %d", msg->Msg_type());
			switch (msg->Msg_type()) {
			case darknet7::STMToESPAny_SetupAP: {
				wifi_config_t wifi_config;
				bool isHidden = false;
				uint8_t max_con = 4;
				uint16_t beacon_interval = 1000;
				const darknet7::SetupAP *sap = msg->Msg_as_SetupAP();
				wifi_auth_mode_t auth_mode = convertAuthMode(sap->mode());
				wifi.initWiFiConfig(wifi_config, sap->ssid()->c_str(), 
									 sap->passwd()->c_str(), 
									 auth_mode, isHidden, max_con, beacon_interval);
				tcpip_adapter_ip_info_t ipInfo;
				wifi.initAdapterIp(ipInfo);
				dhcps_lease_t l;
				wifi.initDHCPSLeaseInfo(l);
				wifi.wifi_start_access_point(wifi_config,ipInfo,l);
				}
				break;
			case darknet7::STMToESPAny_StopAP: {
					wifi.stopWiFi();
				}
				break;
			case darknet7::STMToESPAny_ESPRequest: {
						ESP_LOGI(LOGTAG, "processing system info");
						flatbuffers::FlatBufferBuilder fbb;
						System::logSystemInfo();
						esp_chip_info_t chip;
						System::getChipInfo(&chip);
						auto info = darknet7::CreateESPSystemInfoDirect(fbb,
								System::getFreeHeapSize(), System::getMinimumFreeHeapSize(),
								chip.model, chip.cores,
								chip.revision, chip.features,
								System::getIDFVersion());
						flatbuffers::Offset<darknet7::ESPToSTM> of =
								darknet7::CreateESPToSTM(fbb, msg->msgInstanceID(),
										darknet7::ESPToSTMAny_ESPSystemInfo,
										info.Union());
						darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
						getMCUToMCU().send(fbb);
					}
					break;
			case darknet7::STMToESPAny_CommunicationStatusRequest: {
						ESP_LOGI(LOGTAG, "processing system info");
						flatbuffers::FlatBufferBuilder fbb;
						//TODO FINISH

						// TODO: Use these
						BluetoothTask& bttask = getBLETask();
						std::string blename = bttask.adv_name; 
						bool advertising = bttask.advertising_enabled;

						auto s = darknet7::CreateCommunicationStatusResponseDirect(
										 fbb, darknet7::WiFiStatus_DOWN,
									false, (const char *)"test");	 
						flatbuffers::Offset<darknet7::ESPToSTM> of =
								darknet7::CreateESPToSTM(fbb, msg->msgInstanceID(),
										darknet7::ESPToSTMAny_CommunicationStatusResponse, s.Union());
						darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
						getMCUToMCU().send(fbb);
					}
					break;
			default:
				break;
			}
			delete m;
		}
	}
}


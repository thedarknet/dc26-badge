#include "command_handler.h"
#include "stm_to_esp_generated.h"
#include "esp_to_stm_generated.h"
#include "lib/System.h"
#include "lib/net/HttpServer.h"
#include "mcu_to_mcu.h"
#include "dc26.h"
#include "dc26_ble/ble.h"
#include "npc_interact.h"

static NPCInteractionTask NPCITask("NPCInteractTask");
static HttpServer Port80WebServer;

class MyWiFiEventHandler: public WiFiEventHandler {
public:
	const char *logTag = "MyWiFiEventHandler";
	MyWiFiEventHandler() : APStarted(false), scanMsgID(0), WifiFilter(darknet7::WiFiScanFilter_ALL) {}
public:
	virtual esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip) {
		ESP_LOGD(logTag, "MyWiFiEventHandler(Class): staGotIp");
		IP2STR(&event_sta_got_ip.ip_info.ip);
		return ESP_OK;
	}
	virtual esp_err_t apStart() {
		ESP_LOGI(logTag, "MyWiFiEventHandler(Class): apStart starting web server");
		Port80WebServer.start(80,false);
		APStarted = true;
		return ESP_OK;
	}
	virtual esp_err_t apStop() {
		ESP_LOGI(logTag, "MyWiFiEventHandler(Class): apStop stopping web server");
		Port80WebServer.stop();
		APStarted = false;
		return ESP_OK;
	}
	virtual esp_err_t wifiReady() {
		ESP_LOGI(logTag, "MyWiFiEventHandler(Class): wifi ready");
		return ESP_OK;
	}
	virtual esp_err_t staConnected(system_event_sta_connected_t info) {
		std::string s((const char *)&info.ssid[0],info.ssid_len);
		ESP_LOGI(logTag,"Connected to: %s on channel %d",s.c_str(), (int)info.channel);
		return ESP_OK;
	}
	virtual esp_err_t staDisconnected(system_event_sta_disconnected_t info) {
		return ESP_OK;
	}
	darknet7::WifiMode convertToWiFiAuthMode(uint16_t authMode) {
		switch(authMode) {
		case WIFI_AUTH_OPEN:
			return darknet7::WifiMode_OPEN;
		case WIFI_AUTH_WEP:
			return darknet7::WifiMode_WEP;
		case WIFI_AUTH_WPA2_PSK:
			return darknet7::WifiMode_WPA2;
		case WIFI_AUTH_WPA_PSK:
			return darknet7::WifiMode_WPA;
		case WIFI_AUTH_WPA_WPA2_PSK:
			return darknet7::WifiMode_WPA_WPA2;
		case WIFI_AUTH_WPA2_ENTERPRISE:
			return darknet7::WifiMode_WPA2_ENTERPRISE;
		default:
			return darknet7::WifiMode_UNKNOWN;
		}
	}
	virtual esp_err_t staScanDone(system_event_sta_scan_done_t info) {
		ESP_LOGI(logTag, "MyWiFiEventHandler(Class): scan done: APs Found %d", (int32_t) info.number);
		uint16_t numAPs = info.number;
		wifi_ap_record_t *recs = new wifi_ap_record_t[numAPs];
		std::vector<flatbuffers::Offset<darknet7::WiFiScanResult>> APs;
		flatbuffers::FlatBufferBuilder fbb;
		if(ESP_OK==esp_wifi_scan_get_ap_records(&numAPs,recs)) {
			for(auto i=0;i<numAPs;++i) {
				if(i<5) {
					ESP_LOGI(logTag, "ssid: %s \t auth: %d", (const char *)recs[i].ssid,recs[i].authmode);
					flatbuffers::Offset<darknet7::WiFiScanResult> sro = 
						  	darknet7::CreateWiFiScanResultDirect(fbb,(const char *)recs[i].ssid,
									convertToWiFiAuthMode(recs[i].authmode));
					APs.push_back(sro);
				}
			}
			auto ssro = darknet7::CreateWiFiScanResultsDirect(fbb,&APs);
			flatbuffers::Offset<darknet7::ESPToSTM> of = darknet7::CreateESPToSTM(fbb, 
								 scanMsgID, darknet7::ESPToSTMAny_WiFiScanResults, ssro.Union());
			darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
			getMCUToMCU().send(fbb);
		}
		//esp_wifi_scan_stop();
		delete [] recs;
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
	bool isAPStarted() {return APStarted;}
	uint16_t getScanMsgID() {return scanMsgID;}
	void setScanMsgID(uint16_t sid) {scanMsgID=sid;}
	void setWiFiScanNPCOnly(bool b) {
		if(b) {
			WifiFilter = darknet7::WiFiScanFilter_NPC;
		} else  {
			WifiFilter = darknet7::WiFiScanFilter_ALL;
		}
	}
private:
	bool APStarted;
	uint16_t scanMsgID;
	darknet7::WiFiScanFilter WifiFilter;
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
	NPCITask.start();
	WiFiEventHandler *handler = new MyWiFiEventHandler();
	wifi.setWifiEventHandler(handler);
	return wifi.init();
}

CmdHandlerTask::~CmdHandlerTask() {
}

void CmdHandlerTask::onStop() {
	NPCITask.stop();
}

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
					Port80WebServer.stop();
					wifi.stopWiFi();
					wifi.shutdown();
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
						BluetoothTask& bttask = getBLETask();
						darknet7::WiFiStatus status = darknet7::WiFiStatus_DOWN;
						MyWiFiEventHandler *eh = (MyWiFiEventHandler*)wifi.getWifiEventHandler();
						if(eh && eh->isAPStarted()) {
							status = darknet7::WiFiStatus_AP_STA;
						}
						auto s = darknet7::CreateCommunicationStatusResponseDirect(
										 fbb, status, bttask.advertising_enabled, 
										 bttask.adv_name.c_str());	 
						flatbuffers::Offset<darknet7::ESPToSTM> of =
								darknet7::CreateESPToSTM(fbb, msg->msgInstanceID(),
										darknet7::ESPToSTMAny_CommunicationStatusResponse, s.Union());
						darknet7::FinishSizePrefixedESPToSTMBuffer(fbb, of);
						getMCUToMCU().send(fbb);
					}
					break;
			case darknet7::STMToESPAny_WiFiScan: {
					ESP_LOGI(LOGTAG, "processing wifi scan");
					MyWiFiEventHandler *eh = (MyWiFiEventHandler*)wifi.getWifiEventHandler();
					const darknet7::WiFiScan * ws = msg->Msg_as_WiFiScan();
					if(eh) {
						eh->setScanMsgID(msg->msgInstanceID());
						if(ws->filter()==darknet7::WiFiScanFilter_NPC) {
							eh->setWiFiScanNPCOnly(true);
						}
						//stop the wifi if its running
						wifi.stopWiFi();
						ESP_LOGI(LOGTAG, "starting scan requestID: %d", msg->msgInstanceID());
						wifi.scan(false);
					}
				}
				break;
			case darknet7::STMToESPAny_WiFiNPCInteract: {
					ESP_LOGI(LOGTAG, "processing wifi npc interaction");
					//MyWiFiEventHandler *eh = (MyWiFiEventHandler*)wifi.getWifiEventHandler();
					const darknet7::WiFiNPCInteract * ws = msg->Msg_as_WiFiNPCInteract();
					wifi.connect(ws->bssid().data());
				}
				break;
			default:
				ESP_LOGI(LOGTAG, "Default case in command handler...");
				break;
			}
			delete m;
		}
	}
}


#include "npc_interact.h"
#include "stm_to_esp_generated.h"
#include "esp_to_stm_generated.h"
#include "lib/System.h"
#include "mcu_to_mcu.h"
#include <esp_http_client.h>
#include <cJSON.h>

const char *TAG = "httpclient";
std::string HttpResponseStr;

esp_err_t _http_event_handle(esp_http_client_event_t *evt) {
	switch(evt->event_id) {
		case HTTP_EVENT_ERROR:
			ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
			break;
		case HTTP_EVENT_ON_CONNECTED:
			ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
			break;
		case HTTP_EVENT_HEADER_SENT:
			ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
			break;
		case HTTP_EVENT_ON_HEADER:
			ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
			//printf("%.*s", evt->data_len, (char*)evt->data);
			break;
		case HTTP_EVENT_ON_DATA:
			ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
			if (!esp_http_client_is_chunked_response(evt->client)) {
				printf("%.*s", evt->data_len, (char*)evt->data);
				HttpResponseStr.append((const char *)evt->data, evt->data_len);
			}
			break;
		case HTTP_EVENT_ON_FINISH:
			ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
			break;
		case HTTP_EVENT_DISCONNECTED:
			ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
			break;
		}
	return ESP_OK;
}


static StaticQueue_t InQueue;
const char *NPCInteractionTask::LOGTAG = "NPCITTask";
static uint8_t CommandBuffer[NPCInteractionTask::NPCMSG_QUEUE_SIZE * 
		NPCInteractionTask::NPCMSG_ITEM_SIZE] = { 0 };
	
NPCInteractionTask::NPCInteractionTask(const std::string &tName, 
	uint16_t stackSize, uint8_t p) : Task(tName,stackSize,p), InQueueHandle(nullptr) {

}

bool NPCInteractionTask::init() {
	InQueueHandle = xQueueCreateStatic(NPCMSG_QUEUE_SIZE, NPCMSG_ITEM_SIZE, 
						 &CommandBuffer[0], &InQueue);
	if (InQueueHandle == nullptr) {
		ESP_LOGI(LOGTAG, "Failed creating incomming queue");
	}
	return true;
}

void helo() {
	HttpResponseStr.clear();
	esp_http_client_config_t config;
	memset(&config,0,sizeof(config));
  	config.url = "http://192.168.4.1:8080/npc";
	config.event_handler = _http_event_handle;

	esp_http_client_handle_t client = esp_http_client_init(&config);
	esp_err_t err = esp_http_client_perform(client);

	if (err == ESP_OK) {
		ESP_LOGI(TAG, "Status = %d, content_length = %d",
		esp_http_client_get_status_code(client),
		esp_http_client_get_content_length(client));
		ESP_LOGI(TAG, "%s", HttpResponseStr.c_str());
		ESP_LOGI(TAG, "parising json");
		cJSON *root = cJSON_Parse(HttpResponseStr.c_str());
		if(cJSON_IsArray(root->child)) {
			ESP_LOGI(TAG, "child is array");
			int size = cJSON_GetArraySize(root->child);
			for(int i=0;i<size;i++) {
				cJSON *item = cJSON_GetArrayItem(root->child, i);
				ESP_LOGI(TAG,"item %d, %s", i, cJSON_GetStringValue(item));
			}
		}
	}
	esp_http_client_cleanup(client);
}

void NPCInteractionTask::run(void *data) {
	ESP_LOGI(LOGTAG, "NPCInteractionTask started");
	while (1) {
		NPCMsg *m=0;
		if (xQueueReceive(getQueueHandle(), &m, (TickType_t) 1000 / portTICK_PERIOD_MS)) {
			bool connected = true; //check if connected
			if(connected) {
				if(m->RType==NPCMsg::HELO) {
					ESP_LOGI(LOGTAG,"got HELO");
					helo();
				} else if (m->RType==NPCMsg::INTERACT) {
	
				}
			} else {
				//send back connection error
			}
			delete m;
		}
	}
}

NPCInteractionTask::~NPCInteractionTask() { }


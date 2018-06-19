#include "command_handler.h"
#include "stm_to_esp_generated.h"
#include "esp_to_stm_generated.h"

const char *CmdHandlerTask::LOGTAG = "CmdHandlerTask";
static StaticQueue_t InCommingQueue;
static uint8_t CommandBuffer[CmdHandlerTask::STM_TO_ESP_MSG_QUEUE_SIZE*CmdHandlerTask::STM_TO_ESP_MSG_ITEM_SIZE] = {0};

CmdHandlerTask::CmdHandlerTask(const std::string &tName, uint16_t stackSize, uint8_t p) 
	: Task(tName,stackSize,p), InCommingQueueHandle(nullptr) {

}

bool CmdHandlerTask::init() {
	ESP_LOGI(LOGTAG, "INIT");
	InCommingQueueHandle = xQueueCreateStatic(STM_TO_ESP_MSG_QUEUE_SIZE, STM_TO_ESP_MSG_ITEM_SIZE, &CommandBuffer[0], &InCommingQueue );
	if(InCommingQueueHandle==nullptr) {
		ESP_LOGI(LOGTAG,"Failed creating incomming queue");
	}
	return true;
}

CmdHandlerTask::~CmdHandlerTask() {
}

void CmdHandlerTask::run(void *data) {
	darknet7::STMToESPRequest *msg;
	while (1) {
		if(xQueueReceive(getQueueHandle(), &msg, ( TickType_t ) 20000)/portTICK_PERIOD_MS) {
			switch(msg->Msg_type()) {
				case darknet7::STMToESPAny_SetupAP:
					break;
				default:
					break;
			}
			uint8_t *tmp = (uint8_t*)msg;
			delete [] tmp;
		}
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}


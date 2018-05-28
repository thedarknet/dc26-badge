#include "command_handler.h"
#include "stm_to_esp_generated.h"
#include "esp_to_stm_generated.h"

const char *CmdHandlerTask::LOGTAG = "CmdHandlerTask";

CmdHandlerTask::CmdHandlerTask(const std::string &tName, uint16_t stackSize, uint8_t p) 
	: Task(tName,stackSize,p), InCommingQueue(), InCommingQueueHandle(nullptr), STMToESPBuffer() {

}

bool CmdHandlerTask::init() {
	ESP_LOGI(LOGTAG, "INIT");
	InCommingQueueHandle = xQueueCreateStatic(STM_TO_ESP_MSG_QUEUE_SIZE, STM_TO_ESP_MSG_ITEM_SIZE, STMToESPBuffer, &InCommingQueue );
	return true;
}

CmdHandlerTask::~CmdHandlerTask() {
}

void CmdHandlerTask::run(void *data) {
	darknet7::STMToESPRequest *msg;
	while (1) {
		if(xQueueReceive(InCommingQueueHandle, &msg, ( TickType_t ) 20000)/portTICK_PERIOD_MS) {
			switch(msg->MType()) {
				case darknet7::MsgType_SETUP_AP:
					break;
				default:
					break;
			}
		}
	}
}


#include "npc_interact.h"
#include "stm_to_esp_generated.h"
#include "esp_to_stm_generated.h"
#include "lib/System.h"
#include "mcu_to_mcu.h"

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


void NPCInteractionTask::run(void *data) {
	ESP_LOGI(LOGTAG, "NPCInteractionTask started");
	while (1) {
		NPCMsg *m=0;
		if (xQueueReceive(getQueueHandle(), &m, (TickType_t) 1000 / portTICK_PERIOD_MS)) {
			bool connected = true; //check if connected
			if(connected) {
				if(m->RType==NPCMsg::HELO) {
					//http post 192.168.4.1/helo
					//wait for list of NPCs
					//If list > 1 send back NPCs list
					//else http post 192.168.4.1/npc/<npc>
					//get list of 
				} else if (m->RType==NPCMsg::GET_ACTION_LIST) {

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


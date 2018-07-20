#include "command_handler.h"
#include "stm_to_esp_generated.h"
#include "esp_to_stm_generated.h"
#include "lib/System.h"
#include "mcu_to_mcu.h"
#include "dc26.h"

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
	return true;
}

CmdHandlerTask::~CmdHandlerTask() {
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
			case darknet7::STMToESPAny_SetupAP:
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


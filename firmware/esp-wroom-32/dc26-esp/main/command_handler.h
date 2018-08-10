#ifndef CMDHANDLER_H
#define CMDHANDLER_H

#include "esp_system.h"
#include "esp_log.h"
#include "lib/Task.h"
#include "freertos/queue.h"
#include "mcu_to_mcu.h"
#include "lib/wifi/WiFi.h"

class CmdHandlerTask : public Task {
public:
	static const int STM_TO_ESP_MSG_QUEUE_SIZE = 4;
	static const int STM_TO_ESP_MSG_ITEM_SIZE = sizeof(MCUToMCUTask::Message*);
	static const char *LOGTAG;
public:
	CmdHandlerTask(const std::string &tName, uint16_t stackSize=4196, uint8_t p=5);
	bool init();
	QueueHandle_t getQueueHandle() {return InCommingQueueHandle;}
public:
	virtual void run(void *data);
	virtual ~CmdHandlerTask();
	virtual void onStop();
protected:
	QueueHandle_t InCommingQueueHandle = nullptr;
	WiFi wifi;
private:
};

#endif

#ifndef CMDHANDLER_H
#define CMDHANDLER_H

#include "esp_system.h"
#include "esp_log.h"
#include "lib/Task.h"
#include "freertos/queue.h"

namespace darknet7 {
	class STMToESPRequest;
}

class CmdHandlerTask : public Task {
public:
	static const int STM_TO_ESP_MSG_QUEUE_SIZE = 10;
	static const int STM_TO_ESP_MSG_ITEM_SIZE = sizeof(darknet7::STMToESPRequest*);
	static const char *LOGTAG;
public:
	CmdHandlerTask(const std::string &tName, uint16_t stackSize=10000, uint8_t p=5);
	bool init();
	QueueHandle_t getQueueHandle() {return InCommingQueueHandle;}
public:
	virtual void run(void *data);
	virtual ~CmdHandlerTask();
protected:
	QueueHandle_t InCommingQueueHandle = nullptr;
private:
};

#endif

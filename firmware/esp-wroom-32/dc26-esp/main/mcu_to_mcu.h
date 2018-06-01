#ifndef MCU_TO_MCU_H
#define MCU_TO_MCU_H

#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "lib/Task.h"

struct OutgoingMsg {
	uint8_t *MsgBytes;
	uint32_t Size;
};

namespace darknet7 {
	class STMToESPRequest;
}

class CmdHandlerTask;

class MCUToMCUTask : public Task {
public:
	static const int STM_TO_ESP_MSG_QUEUE_SIZE = 10;
	static const int ESP_TO_STM_MSG_QUEUE_SIZE = 10;
	static const int STM_TO_ESP_MSG_ITEM_SIZE = sizeof(darknet7::STMToESPRequest*);
	static const int ESP_TO_STM_MSG_ITEM_SIZE = sizeof(OutgoingMsg);
	static const char *LOGTAG;
public:
	MCUToMCUTask(CmdHandlerTask *pch, const std::string &tName, uint16_t stackSize=10000, uint8_t p=5);
	void txTask();
	void rxTask();
	bool init(gpio_num_t tx, gpio_num_t rx, uint16_t rxBufSize);
	uint16_t getBufferSize() {return BufSize;}
public:
	virtual void run(void *data);
	virtual ~MCUToMCUTask();
protected:
	StaticQueue_t STMToESPQueue;
	QueueHandle_t STMToESPQueueHandle = nullptr;
	uint8_t STMToESPBuffer[STM_TO_ESP_MSG_QUEUE_SIZE*STM_TO_ESP_MSG_ITEM_SIZE];
	uint8_t ESPToSTMBuffer[ESP_TO_STM_MSG_QUEUE_SIZE*ESP_TO_STM_MSG_ITEM_SIZE];
	StaticQueue_t ESPToSTMQueue;
	QueueHandle_t ESPToSTMQueueHandle = nullptr;
	uint16_t BufSize;
	CmdHandlerTask *CmdHandler;
};

#endif

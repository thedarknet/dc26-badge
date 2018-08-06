#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H

#include "esp_system.h"
#include "esp_log.h"
#include "lib/Task.h"
#include "freertos/queue.h"
#include "mcu_to_mcu.h"
#include "lib/wifi/WiFi.h"

class DisplayTask : public Task {
public:
   struct DisplayMsg {
		char Msg[64];
		uint8_t x, y;
		uint32_t TimeInMSToDisplay;
		DisplayMsg() : Msg(), x(0), y(16), TimeInMSToDisplay(2000) {}
	};
	static const int DISPLAY_QUEUE_SIZE = 10;
	static const int DISPLAY_MSG_ITEM_SIZE = sizeof(DisplayTask::DisplayMsg*);
	static const char *LOGTAG;
public:
	DisplayTask(const std::string &tName, uint16_t stackSize=10000, uint8_t p=5);
	bool init();
	QueueHandle_t getQueueHandle() {return InCommingQueueHandle;}
public:
	virtual void run(void *data);
	virtual ~DisplayTask();
protected:
	QueueHandle_t InCommingQueueHandle = nullptr;
private:
};

#endif

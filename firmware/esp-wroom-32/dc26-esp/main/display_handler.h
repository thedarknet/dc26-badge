#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H

#include "esp_system.h"
#include "esp_log.h"
#include "lib/Task.h"
#include "freertos/queue.h"
#include "mcu_to_mcu.h"
#include "lib/wifi/WiFi.h"
#include <string.h>

class DisplayTask : public Task {
public:
   struct DisplayMsg {
		char Msg[30];
		uint8_t x, y;
		uint16_t TimeInMSToDisplay;
		DisplayMsg() : Msg(), x(0), y(16), TimeInMSToDisplay(2000) {}
		DisplayMsg(const char *msg, uint8_t x1, uint8_t y1, uint16_t ms): Msg(), x(x1), y(y1),
			 TimeInMSToDisplay(ms) {
			strncpy(&Msg[0],msg, sizeof(Msg));
			//strcpy(&Msg[0],msg);
		}
	};
	static const int DISPLAY_QUEUE_SIZE = 6;
	static const int DISPLAY_MSG_ITEM_SIZE = sizeof(DisplayTask::DisplayMsg*);
	static const char *LOGTAG;
public:
	DisplayTask(const std::string &tName, uint16_t stackSize=3096, uint8_t p=5);
	bool init();
	void showMessage(const char *m, uint8_t x1, uint8_t y1, uint16_t ms);
	QueueHandle_t getQueueHandle() {return InCommingQueueHandle;}
public:
	virtual void run(void *data);
	virtual ~DisplayTask();
protected:
	QueueHandle_t InCommingQueueHandle = nullptr;
private:
};

#endif

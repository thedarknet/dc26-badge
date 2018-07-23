#include "display_handler.h"
#include "lib/System.h"
#include "lib/i2c.hpp"
#include "lib/ssd1306.h"
#include "lib/fonts.h"
#include "dc26.h"

const char *DisplayTask::LOGTAG = "DisplayTask";
static StaticQueue_t InCommingQueue;
static uint8_t CommandBuffer[DisplayTask::DISPLAY_QUEUE_SIZE 
	* DisplayTask::DISPLAY_MSG_ITEM_SIZE] = { 0 };

ESP32_I2CMaster I2cDisplay(GPIO_NUM_19,GPIO_NUM_18,1000000, I2C_NUM_0, 1024, 1024);

DisplayTask::DisplayTask(const std::string &tName, uint16_t stackSize, uint8_t p) :
		Task(tName, stackSize, p), InCommingQueueHandle(nullptr) {

}

bool DisplayTask::init() {
	ESP_LOGI(LOGTAG, "INIT");
	if(SSD1306_Init(&I2cDisplay)>0) {
		ESP_LOGI(LOGTAG,"display init successful");
		SSD1306_Puts("BOOTING...", &Font_7x10, SSD1306_COLOR_WHITE);
		SSD1306_UpdateScreen();
	} else {
		ESP_LOGI(LOGTAG,"display init UN-successful");
	}
	InCommingQueueHandle = xQueueCreateStatic(DISPLAY_QUEUE_SIZE,
			DISPLAY_MSG_ITEM_SIZE, &CommandBuffer[0], &InCommingQueue);
	if (InCommingQueueHandle == nullptr) {
		ESP_LOGI(LOGTAG, "Failed creating incomming queue");
	}
	return true;
}

DisplayTask::~DisplayTask() {
}

void DisplayTask::run(void *data) {
	ESP_LOGI(LOGTAG, "Display Task started");
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_GotoXY(0,16);
	SSD1306_Puts("ESP 32 Started...", &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_UpdateScreen();
	DisplayTask::DisplayMsg *m;
	while (1) {
		if (xQueueReceive(getQueueHandle(), &m, (TickType_t) 1000 / portTICK_PERIOD_MS)) {
			ESP_LOGI(LOGTAG, "got message from queue");
			SSD1306_Fill(SSD1306_COLOR_BLACK);
			SSD1306_GotoXY(m->x,m->y);
			SSD1306_Puts(&m->Msg[0], &Font_7x10, SSD1306_COLOR_WHITE);
			uint32_t time = m->TimeInMSToDisplay;
			delete m;
			vTaskDelay((TickType_t) time/portTICK_PERIOD_MS);
		}
	}
}


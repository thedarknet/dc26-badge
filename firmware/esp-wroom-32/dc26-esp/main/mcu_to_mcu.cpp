#include "mcu_to_mcu.h"
#include "stm_to_esp_generated.h"
#include "esp_to_stm_generated.h"
#include "command_handler.h"
#include <vector>

const char *MCUToMCUTask::LOGTAG = "MCUToMCUTask";

MCUToMCUTask::MCUToMCUTask(CmdHandlerTask *pcht, const std::string &tName, uint16_t stackSize, uint8_t p) 
	: Task(tName,stackSize,p), STMToESPQueue(), STMToESPQueueHandle(nullptr), STMToESPBuffer(), 
		ESPToSTMBuffer(), ESPToSTMQueue(), ESPToSTMQueueHandle(nullptr), BufSize(0), CmdHandler(pcht) {

}

bool MCUToMCUTask::init(gpio_num_t tx, gpio_num_t rx, uint16_t rxBufSize) {
	BufSize=rxBufSize;
	ESP_LOGI(LOGTAG, "INIT");
	STMToESPQueueHandle = xQueueCreateStatic(STM_TO_ESP_MSG_QUEUE_SIZE, STM_TO_ESP_MSG_ITEM_SIZE, STMToESPBuffer, &STMToESPQueue );
	ESPToSTMQueueHandle = xQueueCreateStatic(ESP_TO_STM_MSG_QUEUE_SIZE, ESP_TO_STM_MSG_ITEM_SIZE, ESPToSTMBuffer, &ESPToSTMQueue );
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	uart_param_config(UART_NUM_1, &uart_config);
	uart_set_pin(UART_NUM_1, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	// We won't use a buffer for sending data.
	uart_driver_install(UART_NUM_1, rxBufSize * 2, 0, 0, NULL, 0);
	return true;
}

MCUToMCUTask::~MCUToMCUTask() {
	uart_driver_delete(UART_NUM_1);
}

void MCUToMCUTask::txTask() {
	ESP_LOGI(LOGTAG, "txTask");
	esp_log_level_set(LOGTAG, ESP_LOG_INFO);
	OutgoingMsg *msgToSend =0;
	while (1) {
		if(xQueueReceive(ESPToSTMQueueHandle, &msgToSend, ( TickType_t ) 1000)) {
			uint32_t bytesSent = 0;
			while(bytesSent < msgToSend->Size) {
    			const int txBytes = uart_write_bytes(UART_NUM_1, (char *)msgToSend->MsgBytes+bytesSent
					, msgToSend->Size-bytesSent);
				bytesSent+=txBytes;
				vTaskDelay(portTICK_PERIOD_MS*10);
			}
		}
		//sendData(TX_TASK_TAG, "Hello world");
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}
	
void MCUToMCUTask::rxTask() {
	ESP_LOGI(LOGTAG, "txTask");
	static std::vector<uint8_t> sUARTBuffer(getBufferSize());
	esp_log_level_set(LOGTAG, ESP_LOG_INFO);
#if 0
    uint8_t* data = (uint8_t*) malloc(RX_BUgetF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
    			uart_write_bytes(UART_NUM_1, (const char *)data, rxBytes);
        }
    }
    free(data);
#else
	uint8_t dataBuf[1024];
	while(1) {
		const int rxBytes = uart_read_bytes(UART_NUM_1, &dataBuf[0], 
			getBufferSize(), 1000 / portTICK_RATE_MS);
		sUARTBuffer.insert(sUARTBuffer.end(),&dataBuf[0],&dataBuf[rxBytes-1]);
		flatbuffers::Verifier verifier(sUARTBuffer.data(),sUARTBuffer.size());
		darknet7::STMToESPRequest *stmToESPRequest = (darknet7::STMToESPRequest*)&dataBuf[0];
		if(stmToESPRequest->Verify(verifier)) {
			uint32_t size = verifier.GetComputedSize();
			uint8_t *msg = new uint8_t[size];
			memcpy(msg,sUARTBuffer.data(),size);
			//FIX ME REALLY NEEDS TO MOVE BUFFER 
			sUARTBuffer.clear();
			switch(stmToESPRequest->Msg_type()) {
				case darknet7::STMToESPAny_SetupAP:
					CmdHandler->getQueueHandle();
				break;
				default:
				break;
			}
		}
		vTaskDelay(20000 / portTICK_PERIOD_MS);
	}
#endif
}

void MCUToMCUTask::run(void *data) {
	txTask();
	rxTask();
}


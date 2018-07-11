#include "mcu_to_mcu.h"
#include "stm_to_esp_generated.h"
#include "esp_to_stm_generated.h"
#include "command_handler.h"
#include <rom/crc.h>
#include <vector>

const char *MCUToMCUTask::LOGTAG = "MCUToMCUTask";


const darknet7::ESPToSTM *MCUToMCUTask::Message::asESPToSTM() {
	return darknet7::GetSizePrefixedESPToSTM(&MessageData[ENVELOP_HEADER]);
}

const darknet7::STMToESPRequest* MCUToMCUTask::Message::asSTMToESP() {
	return darknet7::GetSizePrefixedSTMToESPRequest(&MessageData[ENVELOP_HEADER]);
}


//We listen for the for our envelop portion of our message which is: 4 bytes:
// bits 0-10 is the size of the message coming max message size = 1024
// bit 11: reserved
// bit 12: reserved
// bit 13: reserved
// bit 14: reserved
// bit 15: reserved
// bit 16-31: CRC 16 of entire message
//set up for our 4 byte envelop header

MCUToMCUTask::Message::Message() :
		SizeAndFlags(0), Crc16(0), MessageData() {
}

void MCUToMCUTask::Message::set(uint16_t sf, uint16_t crc, uint8_t *data) {
	SizeAndFlags = sf;
	Crc16 = crc;
	MessageData[0] = sf & 0xFF;
	MessageData[1] = sf & 0xFF00 >> 8;
	MessageData[2] = Crc16 & 0xFF;
	MessageData[3] = Crc16 & 0xFF00 >> 8;
	memcpy(&MessageData[ENVELOP_HEADER], data, getDataSize());
}

void MCUToMCUTask::Message::setFlag(uint16_t flags) {
	SizeAndFlags|=flags;
}

bool MCUToMCUTask::Message::checkFlags(uint16_t flags) {
	return (SizeAndFlags&flags)==flags;
}

bool MCUToMCUTask::Message::transmit() {
	ESP_LOGI(MCUToMCUTask::LOGTAG, "sending with break!");
	if(getMessageSize()==
		uart_write_bytes_with_break(UART_NUM_1,(const char *)&MessageData[0],getMessageSize(),100)) {
		return true;
	}
	return false;
}

////////////
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

void MCUToMCUTask::send(const flatbuffers::FlatBufferBuilder &fbb) {
	uint8_t *msg = fbb.GetBufferPointer();
	uint32_t size = fbb.GetSize();
	assert(size<MAX_MESSAGE_SIZE);
	uint16_t crc = crc16_le(0,msg,size);
	ESP_LOGI(LOGTAG, "size %d, crc %d\n",size,crc);
	Message *m = new Message();
	m->set(size,crc,msg);
	//push message to transmit task
	ESP_LOGI(LOGTAG, "sending message to tx queue");
	xQueueSend(ESPToSTMQueueHandle,&m,( TickType_t ) 1000);
	//m.transmit();
}

void MCUToMCUTask::txTask() {
	//ESP_LOGI(LOGTAG, "txTask");
	//esp_log_level_set(LOGTAG, ESP_LOG_INFO);
	Message *msgToSend =0;
	//while (1) {
		if(xQueueReceive(ESPToSTMQueueHandle, &msgToSend, ( TickType_t ) 100)) {
			msgToSend->transmit();
			delete msgToSend;
		} else {
			ESP_LOGI(LOGTAG, "nothing to send");
		}
//	}
}
	
void MCUToMCUTask::rxTask() {
	//ESP_LOGI(LOGTAG, "rxTask");
	static std::vector<uint8_t> sUARTBuffer;
	uint8_t dataBuf[MAX_MESSAGE_SIZE*2];
	//while(1) {
		const int rxBytes = uart_read_bytes(UART_NUM_1, &dataBuf[0], 
							 sizeof(dataBuf), 100 / portTICK_RATE_MS);
		if(rxBytes>0) {
			ESP_LOGI(LOGTAG, "%d bytes received", rxBytes);
			sUARTBuffer.insert(sUARTBuffer.end(),&dataBuf[0],&dataBuf[rxBytes-1]);
		} else {
			ESP_LOGI(LOGTAG, "%d bytes received", rxBytes);
		}
		if(sUARTBuffer.size()>ENVELOP_HEADER) {
			ESP_LOGI(LOGTAG, "large enough to process");
			uint16_t firstTwo = (*((uint16_t *) &sUARTBuffer[0]));
			uint16_t size = firstTwo & 0x7FF; //0-10 bits are size
			uint16_t crcFromSTM = (*((uint16_t *) &sUARTBuffer[2]));
			assert(size < MAX_MESSAGE_SIZE);
			if(sUARTBuffer.size()>=size) {
				ESP_LOGI(LOGTAG, "large enough to get message: firstTwo (%u) size (%u) crc(%u)",
									 firstTwo,size,crcFromSTM);
				Message *m = new Message();
				m->set(firstTwo,crcFromSTM,&sUARTBuffer[ENVELOP_HEADER]);
				std::vector<decltype(sUARTBuffer)::value_type>(sUARTBuffer.begin()+size, 
					sUARTBuffer.end()).swap(sUARTBuffer);
				auto msg = m->asSTMToESP();
				ESP_LOGI(LOGTAG, "MsgType %d", msg->Msg_type());
				switch(msg->Msg_type()) {
					case darknet7::STMToESPAny_SetupAP:
					case darknet7::STMToESPAny_ESPRequest:
						xQueueSend(CmdHandler->getQueueHandle(),(void*)&m, ( TickType_t ) 0);
						break;
					default:
						break;
				}
			}
		}
	//}
}

void MCUToMCUTask::run(void *data) {
	esp_log_level_set(LOGTAG, ESP_LOG_INFO);
 	while(1) {
		txTask();
		rxTask();
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}


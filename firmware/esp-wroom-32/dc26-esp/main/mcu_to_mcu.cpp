#include "mcu_to_mcu.h"
#include "stm_to_esp_generated.h"
#include "esp_to_stm_generated.h"
#include "command_handler.h"
#include <rom/crc.h>

const char *MCUToMCUTask::LOGTAG = "MCUToMCUTask";

const darknet7::ESPToSTM *MCUToMCUTask::Message::asESPToSTM() {
	return darknet7::GetSizePrefixedESPToSTM(&MessageData[ENVELOP_HEADER]);
}

const darknet7::STMToESPRequest* MCUToMCUTask::Message::asSTMToESP() {
	return darknet7::GetSizePrefixedSTMToESPRequest(
			&MessageData[ENVELOP_HEADER]);
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

bool MCUToMCUTask::Message::read(const uint8_t* data, uint32_t dataSize) {
	SizeAndFlags = data[0];
	SizeAndFlags |= ((uint16_t) data[1]) << 8;
	Crc16 = data[2];
	Crc16 |= ((uint16_t) data[3]) << 8;
	ESP_LOGI(LOGTAG, "buffer size needs to be: sizeFlags (%u) size (%u) crc(%u)",
			(uint32_t)SizeAndFlags, (uint32_t)getDataSize(), (uint32_t)Crc16);
	assert(getDataSize() < MAX_MESSAGE_SIZE);
	//calc crc
	uint16_t crc = crc16_le(0, &data[ENVELOP_HEADER], getDataSize());
	if(crc!=Crc16) {
		ESP_LOGI(LOGTAG, "CRC's don't match calced: %d, STM %d", crc, Crc16);
	}
	if (getMessageSize() <= dataSize) {
		ESP_LOGI(LOGTAG, "coping data to message");
		memcpy(&MessageData[0], &data[0], getMessageSize());
		return true;
	}
	return false;
}

void MCUToMCUTask::Message::set(uint16_t sf, uint16_t crc, uint8_t *data) {
	SizeAndFlags = sf;
	Crc16 = crc;
	MessageData[0] = sf & 0xFF;
	MessageData[1] = (sf & 0xFF00) >> 8;
	MessageData[2] = Crc16 & 0xFF;
	MessageData[3] = (Crc16 & 0xFF00) >> 8;
	memcpy(&MessageData[ENVELOP_HEADER], data, getDataSize());
}

void MCUToMCUTask::Message::setFlag(uint16_t flags) {
	SizeAndFlags |= flags;
}

bool MCUToMCUTask::Message::checkFlags(uint16_t flags) {
	return (SizeAndFlags & flags) == flags;
}

bool MCUToMCUTask::Message::transmit() {
	ESP_LOGI(MCUToMCUTask::LOGTAG, "sending with break!");
	if (getMessageSize()
			== uart_write_bytes_with_break(UART_NUM_1,
					(const char *) &MessageData[0], getMessageSize(), 100)) {
		return true;
	}
	return false;
}

////////////
MCUToMCUTask::MCUToMCUTask(CmdHandlerTask *pcht, const std::string &tName,
		uint16_t stackSize, uint8_t p) :
		Task(tName, stackSize, p), ESPToSTMBuffer(), BufSize(0), CmdHandler(pcht) {

}

static QueueHandle_t uart0_queue;

bool MCUToMCUTask::init(gpio_num_t tx, gpio_num_t rx, uint16_t rxBufSize) {
	BufSize = rxBufSize;
	ESP_LOGI(LOGTAG, "INIT");
	uart_config_t uart_config = { .baud_rate = 115200, .data_bits =
			UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bits =
			UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE };
	uart_config.use_ref_tick = 0;
	uart_param_config(UART_NUM_1, &uart_config);
	uart_set_pin(UART_NUM_1, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	// added a buffer for send and receive that we don't have to handle the async send
	uart_driver_install(UART_NUM_1, rxBufSize * 2, 0, 4, &uart0_queue, 0);
	return true;
}

MCUToMCUTask::~MCUToMCUTask() {
	uart_driver_delete (UART_NUM_1);
}

void MCUToMCUTask::send(const flatbuffers::FlatBufferBuilder &fbb) {
	uint8_t *msg = fbb.GetBufferPointer();
	uint32_t size = fbb.GetSize();
	assert(size < MAX_MESSAGE_SIZE);
	uint16_t crc = crc16_le(0, msg, size);
	ESP_LOGI(LOGTAG, "size %d, crc %d\n", size, crc);
	Message m;
	m.set(size, crc, msg);
	m.transmit();
}


void MCUToMCUTask::processMessage(const uint8_t *data, uint32_t size) {
	ESP_LOGI(LOGTAG, "Process Message");
	Message *m = new Message();
	if(m->read(data,size)) {
		auto msg = m->asSTMToESP();
		ESP_LOGI(LOGTAG, "MsgType %d", msg->Msg_type());
		switch (msg->Msg_type()) {
		case darknet7::STMToESPAny_SetupAP:
			break;
		case darknet7::STMToESPAny_ESPRequest:
			ESP_LOGI(LOGTAG, "sending to cmd handler");
			xQueueSend(CmdHandler->getQueueHandle(), (void* )&m,(TickType_t ) 0);
			ESP_LOGI(LOGTAG, "after send to cmd handler");
			break;
		default:
			break;
		}
	} else {
		delete m;
	}
}

void MCUToMCUTask::run(void *data) {
	esp_log_level_set(LOGTAG, ESP_LOG_INFO);
	uart_event_t event;
	uint8_t dataBuf[MAX_MESSAGE_SIZE * 2] = {0};
	size_t receivePtr = 0;
	while (1) {
		//Waiting for UART event.
		if (xQueueReceive(uart0_queue, (void * )&event, (portTickType )portMAX_DELAY)) {
			switch (event.type) {
			//Event of UART receiving data
			case UART_DATA:
				ESP_LOGI(LOGTAG, "[UART DATA]: %d", event.size);
				receivePtr+=uart_read_bytes(UART_NUM_1, &dataBuf[0], sizeof(dataBuf), 100);
				ESP_LOG_BUFFER_HEX(LOGTAG, &dataBuf[0], receivePtr);
				break;
				//Event of HW FIFO overflow detected
			case UART_FIFO_OVF:
			case UART_BUFFER_FULL:
				ESP_LOGI(LOGTAG, "hw fifo overflow or buffer full - is other MCU powered?");
				// If fifo overflow happened, you should consider adding flow control for your application.
				uart_flush_input (UART_NUM_1);
				xQueueReset(uart0_queue);
				break;
				//Event of UART RX break detected
			case UART_BREAK:
				ESP_LOGI(LOGTAG, "uart rx break");
				receivePtr+=uart_read_bytes(UART_NUM_1, &dataBuf[0], sizeof(dataBuf), 100);
				ESP_LOG_BUFFER_HEX(LOGTAG, &dataBuf[0], receivePtr);
				if(receivePtr>ENVELOP_HEADER) {
					processMessage(&dataBuf[0],receivePtr);
					receivePtr = 0;
					bzero(&dataBuf[0],sizeof(dataBuf));
				}
				break;
				//Event of UART parity check error
			case UART_PARITY_ERR:
				ESP_LOGI(LOGTAG, "uart parity error");
				xQueueReset(uart0_queue);
				break;
				//Event of UART frame error
			case UART_FRAME_ERR:
				ESP_LOGI(LOGTAG, "uart frame error");
				xQueueReset(uart0_queue);
				break;
				//UART_PATTERN_DET
			case UART_PATTERN_DET:
				ESP_LOGI(LOGTAG,"pattern message?????");
				break;
				//Others
			default:
				ESP_LOGI(LOGTAG, "uart event type: %d", event.type);
				break;
			}
		}
	}
}


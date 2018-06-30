#ifndef DARKNET7_MCU_TO_MCU_H
#define DARKNET7_MCU_TO_MCU_H

#include "messaging/esp_to_stm_generated.h"
#include "libstm32/etl/src/vector.h"
#include "libstm32/etl/src/queue.h"


class MCUToMCU {
public:
	static const int ENVELOP_HEADER = 4;
	static const int ENVELOP_HEADER_SIZE_MASK = 0x7FF;
	static const int MAX_MESSAGE_SIZE = 512;
	static const int TOTAL_MESSAGE_SIZE = MAX_MESSAGE_SIZE+ENVELOP_HEADER;
public:
	class Message {
	public:
		darknet7::ESPToSTM *asESPToSTM();
	protected:
		Message();
		void set(uint16_t sf, uint16_t crc, uint8_t *data);
		uint16_t getSize() {return SizeAndFlags&ENVELOP_HEADER_SIZE_MASK;}
		static uint16_t getSize(uint16_t s) {return s&ENVELOP_HEADER_SIZE_MASK;}
	private:
		uint16_t SizeAndFlags;
		uint16_t Crc16;
		uint8_t MessageData[MAX_MESSAGE_SIZE];
		friend class MCUToMCU;
	};
public:
	static MCUToMCU &get();
public:
	void init(UART_HandleTypeDef *);
	bool send(const flatbuffers::FlatBufferBuilder &fbb);
	darknet7::ESPToSTM *getNext();
	void onTransmitionComplete(UART_HandleTypeDef *huart);
	void handleMcuToMcu();
private:
	MCUToMCU();
	static MCUToMCU *mSelf;
private:
	etl::queue<Message,4> InComing;
	etl::vector<uint8_t,1024> TransmitBuffer;
	UART_HandleTypeDef *UartHandler;
};

#endif

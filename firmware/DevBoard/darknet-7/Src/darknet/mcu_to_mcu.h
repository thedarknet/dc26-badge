#ifndef DARKNET7_MCU_TO_MCU_H
#define DARKNET7_MCU_TO_MCU_H

#include "messaging/esp_to_stm_generated.h"
#include "libstm32/etl/src/vector.h"
#include "libstm32/etl/src/queue.h"
#include "libstm32/observer/event_bus.h"

template<typename T>
struct MSGEvent {
	const T *InnerMsg;
	uint32_t RequestID;
	MSGEvent(const T*im, uint32_t rid) : InnerMsg(im), RequestID(rid) {}
};

class MCUToMCU {
public:
	static const int ENVELOP_HEADER = 4;
	static const int ENVELOP_HEADER_SIZE_MASK = 0x7FF;
	static const int MAX_MESSAGE_SIZE = 252;
	static const int TOTAL_MESSAGE_SIZE = MAX_MESSAGE_SIZE+ENVELOP_HEADER;
public:
	class Message {
	public:
		static const uint16_t MESSAGE_FLAG_TRANSMITTED = 0x8000; //bit 15
	public:
		const darknet7::ESPToSTM *asESPToSTM();
		bool verifyESPToSTM();
	protected:
		Message();
		void setFlag(uint16_t flags);
		bool checkFlags(uint16_t flags);
		void set(uint16_t sf, uint16_t crc, uint8_t *data);
		HAL_StatusTypeDef transmit(UART_HandleTypeDef *huart);
		uint16_t getMessageSize() {return getDataSize()+ENVELOP_HEADER;}
		uint16_t getDataSize() {return SizeAndFlags&ENVELOP_HEADER_SIZE_MASK;}
		static uint16_t getDataSize(uint16_t s) {return s&ENVELOP_HEADER_SIZE_MASK;}
	private:
		uint16_t SizeAndFlags;
		uint16_t Crc16;
		uint8_t MessageData[MAX_MESSAGE_SIZE];
		friend class MCUToMCU;
	};
public:
	static MCUToMCU &get();
	typedef cmdc0de::EventBus<3,14,5,3> UART_EVENT_BUS_TYPE;
public:
	void init(UART_HandleTypeDef *);
	bool send(const flatbuffers::FlatBufferBuilder &fbb);
	void process();
	void onTransmitionComplete();
	void onError();
	void handleMcuToMcu();
	const UART_HandleTypeDef * getUART() const {return UartHandler;}
	UART_EVENT_BUS_TYPE &getBus() {return MessageBus;}
protected:
	void resetUART();
	bool transmitNow();
private:
	MCUToMCU();
	static MCUToMCU *mSelf;
private:
	etl::queue<Message,4> InComing;
	etl::queue<Message,4> Outgoing;
	UART_HandleTypeDef *UartHandler;
	UART_EVENT_BUS_TYPE MessageBus;
};

#endif

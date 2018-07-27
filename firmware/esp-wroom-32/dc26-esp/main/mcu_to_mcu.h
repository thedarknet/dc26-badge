#ifndef MCU_TO_MCU_H
#define MCU_TO_MCU_H

#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "lib/Task.h"


namespace darknet7 {
	class STMToESPRequest;
	class ESPToSTM;
}

namespace flatbuffers {
	class FlatBufferBuilder;
}

class CmdHandlerTask;


class MCUToMCUTask : public Task {
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
		const darknet7::STMToESPRequest *asSTMToESP();
		const darknet7::STMToESPRequest *asSTMToESPVerify();
		const darknet7::ESPToSTM *asESPToSTM();
		Message(); // TODO: return to protected
		bool read(const uint8_t* data, uint32_t dataSize); // return to protected
		void set(uint16_t sf, uint16_t crc, uint8_t *data);
		//do not call directly!
		//bool  transmit();
	//protected:
		void setFlag(uint16_t flags);
		bool checkFlags(uint16_t flags);
		uint16_t getMessageSize() {return getDataSize()+ENVELOP_HEADER;}
		uint16_t getDataSize() {return SizeAndFlags&ENVELOP_HEADER_SIZE_MASK;}
		static uint16_t getDataSize(uint16_t s) {return s&ENVELOP_HEADER_SIZE_MASK;}
		const char *getMessageData() {return (const char *)&MessageData[0];}
	private:
		uint16_t SizeAndFlags;
		uint16_t Crc16;
		uint8_t MessageData[MAX_MESSAGE_SIZE];
		friend class MCUToMCUTask;
	};
public:
	static const int ESP_TO_STM_MSG_QUEUE_SIZE = 5;
	static const int ESP_TO_STM_MSG_ITEM_SIZE = sizeof(Message *);
	static const char *LOGTAG;
public:
	MCUToMCUTask(CmdHandlerTask *pch, const std::string &tName, uint16_t stackSize=10000, uint8_t p=5);
	bool init(gpio_num_t tx, gpio_num_t rx, uint16_t rxBufSize);
	int32_t processMessage(const uint8_t *data, uint32_t size);
	//uint16_t getBufferSize() {return BufSize;}
	void send(const flatbuffers::FlatBufferBuilder &fbb);
protected:
	virtual void onStart();
	virtual void onStop();
public:
	virtual void run(void *data);
	virtual ~MCUToMCUTask();
protected:
	//uint16_t BufSize;
	CmdHandlerTask *CmdHandler;
};

#endif

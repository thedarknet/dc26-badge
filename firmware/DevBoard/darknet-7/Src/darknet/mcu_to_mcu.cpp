/*
 * mcu_to_mcu.cpp
 *
 *  Created on: Jun 22, 2018
 *      Author: cmdc0de
 */

#include <usart.h>
#include "mcu_to_mcu.h"
#include "libstm32/etl/src/crc16.h"
#include "libstm32/logger.h"

MCUToMCU *MCUToMCU::mSelf = 0;
uint8_t UartRXBuffer[MCUToMCU::TOTAL_MESSAGE_SIZE * 2] = { 0 };

const darknet7::ESPToSTM *MCUToMCU::Message::asESPToSTM() {
	return darknet7::GetSizePrefixedESPToSTM(&MessageData[ENVELOP_HEADER]);
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

MCUToMCU::Message::Message() :
		SizeAndFlags(0), Crc16(0), MessageData() {

}

void MCUToMCU::Message::set(uint16_t sf, uint16_t crc, uint8_t *data) {
	SizeAndFlags = sf;
	Crc16 = crc;
	MessageData[0] = sf & 0xFF;
	MessageData[1] = sf & 0xFF00 >> 8;
	MessageData[2] = Crc16 & 0xFF;
	MessageData[3] = Crc16 & 0xFF00 >> 8;
	memcpy(&MessageData[ENVELOP_HEADER], data, getDataSize());
}

void MCUToMCU::Message::setFlag(uint16_t flags) {
	SizeAndFlags|=flags;
}

bool MCUToMCU::Message::checkFlags(uint16_t flags) {
	return (SizeAndFlags&flags)==flags;
}

HAL_StatusTypeDef MCUToMCU::Message::transmit(UART_HandleTypeDef *huart) {
	HAL_StatusTypeDef status = HAL_UART_Transmit_IT(huart,&MessageData[0],getMessageSize());
	if(status==HAL_OK) {
		setFlag(MESSAGE_FLAG_TRANSMITTED);
	} else {
		ERRMSG("transmit failed: %d", status);
	}
	return status;
}

void MCUToMCU::resetUART() {
	HAL_UART_DeInit(UartHandler);
	init(UartHandler);
}

void MCUToMCU::init(UART_HandleTypeDef *uart) {
	UartHandler = uart;
	HAL_LIN_Init(UartHandler, UART_LINBREAKDETECTLENGTH_10B);
	HAL_UART_Receive_IT(UartHandler, &UartRXBuffer[0],
			MCUToMCU::TOTAL_MESSAGE_SIZE);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == MCUToMCU::get().getUART()) {
		MCUToMCU::get().handleMcuToMcu();
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	if (huart == MCUToMCU::get().getUART()) {
		MCUToMCU::get().onError();
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == MCUToMCU::get().getUART()) {
		MCUToMCU::get().onTransmitionComplete();
	}
}

void MCUToMCU::onError() {
	MCUToMCU::get().handleMcuToMcu();
}

void MCUToMCU::onTransmitionComplete() {
	Message &m = Outgoing.front();
	if(m.checkFlags(Message::MESSAGE_FLAG_TRANSMITTED)) {
		Outgoing.pop();
		HAL_LIN_SendBreak(UartHandler);
	}
	transmitNow();
}

MCUToMCU &MCUToMCU::get() {
	if (0 == mSelf) {
		mSelf = new MCUToMCU();
	}
	return *mSelf;
}

MCUToMCU::MCUToMCU() :
		InComing(), Outgoing(), UartHandler(0) {

}

void MCUToMCU::handleMcuToMcu() {
	uint16_t size = UartHandler->RxXferCount;
	//we have received something and we have also gotten a line break
	if (size > 0 && __HAL_UART_GET_FLAG(UartHandler, UART_FLAG_LBD)) {
		uint16_t firstTwo = (*((uint16_t *) &UartRXBuffer[0]));
		uint16_t size = firstTwo & 0x7FF; //0-10 bits are size
		uint16_t crcFromESP = (*((uint16_t *) &UartRXBuffer[2]));
		assert(size < MAX_MESSAGE_SIZE);
		etl::crc16 crc(&UartRXBuffer[ENVELOP_HEADER],
				&UartRXBuffer[ENVELOP_HEADER] + size);
		//if (crc.value() == crcFromESP) {
			Message &m = InComing.push();
			m.set(firstTwo, crcFromESP, &UartRXBuffer[ENVELOP_HEADER]);
		//} else {
		//	ERRMSG("CRC ERROR in handle MCU To MCU.\n");
		//}
		const darknet7::ESPToSTM *test = m.asESPToSTM();
		const darknet7::ESPSystemInfo * system = test->Msg_as_ESPSystemInfo();
		int32_t  cores = system->cores();
		uint32_t headSize = system->heapSize();
		const flatbuffers::String *version = system->idf_version();
		const char *p = version->c_str();
		HAL_UART_Receive_IT(UartHandler, &UartRXBuffer[0], MAX_MESSAGE_SIZE);
	} else if (UartHandler->RxXferCount > 0) {
		//overflow
		ERRMSG("RxCpltCallback overflow: %d\nResetting\n", UartHandler->RxXferCount);
		resetUART();
	}
}

bool MCUToMCU::send(const flatbuffers::FlatBufferBuilder &fbb) {
	uint8_t *msg = fbb.GetBufferPointer();
	uint32_t size = fbb.GetSize();
	assert(size < MAX_MESSAGE_SIZE);
	etl::crc16 crc(msg, msg + size);
	Message &m = Outgoing.push();
	m.set(size, crc.value(), msg);

	return transmitNow();
}

bool MCUToMCU::transmitNow() {
	if ((UartHandler->gState
			& (HAL_UART_STATE_ERROR | HAL_UART_STATE_TIMEOUT))) {
		resetUART();
	}
	if ((UartHandler->gState & HAL_UART_STATE_READY) != 0
			&& !(UartHandler->gState&HAL_UART_STATE_BUSY_TX)
			&& !Outgoing.empty()) {
		Message &m = Outgoing.front();
		m.transmit(UartHandler);
	}
	return true;
}

static bool pastFirst = false;
const darknet7::ESPToSTM *MCUToMCU::getNext() {
	if (!InComing.empty()) {
		if (!pastFirst) {
			pastFirst = true;
		} else {
			InComing.pop();
		}
		Message &m = InComing.front();
		return m.asESPToSTM();
	}
	return nullptr;
}


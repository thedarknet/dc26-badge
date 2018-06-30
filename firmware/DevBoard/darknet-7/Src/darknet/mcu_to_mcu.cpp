/*
 * mcu_to_mcu.cpp
 *
 *  Created on: Jun 22, 2018
 *      Author: cmdc0de
 */

#include <usart.h>
#include "mcu_to_mcu.h"
#include "libstm32/etl/src/crc16.h"

MCUToMCU *MCUToMCU::mSelf = 0;
uint8_t UartRXBuffer[1028] = {0};

darknet7::ESPToSTM *MCUToMCU::Message::asESPToSTM() {
	return 0;
}

MCUToMCU::Message::Message() : SizeAndFlags(0), Crc16(0), MessageData() {

}

void MCUToMCU::Message::set(uint16_t sf, uint16_t crc, uint8_t *data) {
	SizeAndFlags = sf;
	Crc16 = crc;
	memcpy(&MessageData[0],data,getSize());
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
void MCUToMCU::init(UART_HandleTypeDef *uart) {
	UartHandler = uart;
	HAL_UART_Receive_IT(UartHandler,&UartRXBuffer[0],ENVELOP_HEADER);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart==&huart1) {
		if(huart->RxXferCount>0) {
			MCUToMCU::get().handleMcuToMcu();
		}
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	MCUToMCU::get().onTransmitionComplete(huart);
}

void MCUToMCU::onTransmitionComplete(UART_HandleTypeDef *huart) {
	/*
	etl::vector<decltype(TransmitBuffer)::value_type,1024>(TransmitBuffer.begin()+huart->TxXferCount,
				TransmitBuffer.end()).swap(TransmitBuffer);
	if(!TransmitBuffer.empty()) {
		HAL_UART_Transmit_IT(UartHandler,&TransmitBuffer[0],(uint16_t)TransmitBuffer.size());
	}
	*/
}

MCUToMCU &MCUToMCU::get() {
	if(0==mSelf) {
		mSelf = new MCUToMCU();
	}
	return *mSelf;
}

MCUToMCU::MCUToMCU() : InComing(), TransmitBuffer(), UartHandler(0) {

}

void MCUToMCU::handleMcuToMcu() {
	uint16_t size = UartHandler->RxXferCount;
	if(UartHandler->pRxBuffPtr-ENVELOP_HEADER==&UartRXBuffer[0]) {
		//we just got header:
		uint16_t firstTwo = (*((uint16_t *)&UartHandler[0]));
		uint16_t size = firstTwo&0x7FF; //0-10 bits are size
		HAL_UART_Receive_IT(UartHandler,UartHandler->pRxBuffPtr,size);
	} else {
		uint16_t firstTwo = (*((uint16_t *)&UartHandler[0]));
		uint16_t size = firstTwo&0x7FF; //0-10 bits are size
		uint16_t crcFromESP = (*((uint16_t *)&UartHandler[2]));
		etl::crc16 crc(&UartRXBuffer[ENVELOP_HEADER],&UartRXBuffer[ENVELOP_HEADER]+size);
		if(crc.value()==crcFromESP) {
			Message &m = InComing.push();
			m.set(firstTwo,crcFromESP,&UartRXBuffer[ENVELOP_HEADER]);
		}
		HAL_UART_Receive_IT(UartHandler,&UartRXBuffer[0],ENVELOP_HEADER);
	}
}

bool MCUToMCU::send(const flatbuffers::FlatBufferBuilder &fbb) {
	uint8_t *msg = fbb.GetBufferPointer();
	uint32_t size = fbb.GetSize();
	etl::crc16 crc(msg,msg+size);
	uint16_t envelop[ENVELOP_HEADER/sizeof(uint16_t)];
	envelop[0] = static_cast<uint16_t>(size&0xFFFF);
	envelop[1] = crc.value();
	TransmitBuffer.insert(TransmitBuffer.end(),reinterpret_cast<uint8_t*>(&envelop[0]),reinterpret_cast<uint8_t*>(&envelop[0])+ENVELOP_HEADER);
	TransmitBuffer.insert(TransmitBuffer.end(),fbb.GetBufferPointer(),fbb.GetBufferPointer()+fbb.GetSize());
	return HAL_OK==HAL_UART_Transmit_IT(UartHandler,&TransmitBuffer[0],(uint16_t) TransmitBuffer.size());
}

static bool pastFirst = false;
darknet7::ESPToSTM *MCUToMCU::getNext() {
	if(!InComing.empty()) {
		if(!pastFirst) {
			pastFirst = true;
		} else {
			InComing.pop();
		}
	}
	Message &m = InComing.front();
	return m.asESPToSTM();
}


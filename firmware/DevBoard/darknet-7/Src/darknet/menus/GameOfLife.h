#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#include "darknet7_base_state.h"
#include "../libstm32/utility/bitarray.h"

class GameOfLife: public Darknet7BaseState {
public:
	GameOfLife();
	virtual ~GameOfLife();
public:
	static const int width = 128;
	static const int height = 128;
	static const int num_slots = width*height;
	static const int sizeof_buffer = (num_slots/8)+1;
	static uint8_t Buffer[sizeof_buffer];
	static const uint32_t NEED_DRAW = 1<<StateBase::SHIFT_FROM_BASE;
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
	void initGame();
	bool life(cmdc0de::BitArray &array, char choice, short width, short height, cmdc0de::BitArray &temp);
	enum INTERNAL_STATE {
		INIT, MESSAGE, TIME_WAIT, GAME, SLEEP
	};
	bool shouldDisplayMessage();
private:
	uint16_t Generations;
	uint16_t CurrentGeneration;
	uint8_t Neighborhood;
	//uint64_t gol[height];
	cmdc0de::BitArray GOL;
	char UtilityBuf[64];
	INTERNAL_STATE InternalState;
	uint32_t DisplayMessageUntil;
};

#endif

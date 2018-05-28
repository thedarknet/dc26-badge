#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#include "../libstm32/app/state_base.h"
#include "../messaging/stm_to_esp_generated.h"
#include "../libstm32/display/gui.h"


class GameOfLife: public cmdc0de::StateBase {
public:
	GameOfLife();
	virtual ~GameOfLife();
public:
	static const int width = 32;
	static const int height = 64;
	static const uint32_t NEED_DRAW = 1<<StateBase::SHIFT_FROM_BASE;
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
	void initGame();
	bool life(uint32_t *array, char choice, short width, short height, uint32_t *temp);
	enum INTERNAL_STATE {
		INIT, MESSAGE, TIME_WAIT, GAME, SLEEP
	};
	bool shouldDisplayMessage();
private:
	uint16_t Generations;
	uint16_t CurrentGeneration;
	uint8_t Neighborhood;
	uint32_t gol[height];
	char UtilityBuf[64];
	INTERNAL_STATE InternalState;
	uint32_t DisplayMessageUntil;
};

#endif

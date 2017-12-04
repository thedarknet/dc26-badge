/*
 * DC26.h
 *
 *  Created on: Dec 3, 2017
 *      Author: cmdc0de
 */

#ifndef DARKNET_DC26_H_
#define DARKNET_DC26_H_

#include "libstm32/app/app.h"

class DC26 : public cmdc0de::App {
public:
	DC26();
	virtual ~DC26();
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::ErrorType onRun();
};

#endif /* DARKNET_DC26_H_ */

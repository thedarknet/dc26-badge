/*
 * init.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: dcomes
 */

#include "init.h"
#include "darknet7.h"

void init() {
	DarkNet7::get().init();
}

void runOnce() {
	DarkNet7::get().run();
}


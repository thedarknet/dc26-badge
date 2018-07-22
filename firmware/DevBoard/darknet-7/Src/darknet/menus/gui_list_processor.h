/*
 * gui_list_processor.h
 *
 *  Created on: Jul 16, 2018
 *      Author: dcomes
 */

#ifndef DARKNET_MENUS_GUI_LIST_PROCESSOR_H_
#define DARKNET_MENUS_GUI_LIST_PROCESSOR_H_

#include <stdint.h>

namespace cmdc0de {
	class GUIListData;
}

class GUIListProcessor {
public:
	static bool process(cmdc0de::GUIListData *pl, uint16_t itemCount);
};



#endif /* DARKNET_MENUS_GUI_LIST_PROCESSOR_H_ */

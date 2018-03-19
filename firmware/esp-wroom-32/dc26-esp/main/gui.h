#ifndef GUI_H
#define GUI_H

#include <stdio.h>
#include "fonts.h"
#include "ssd1306.h"

#define GUI_DefFont Font_7x10

/**
 * @brief  Initialize gui
 * @param  None
 * @retval None
 */
bool gui_init(void);

/**
 * @brief  Simple draw text
 * @param  txt: Text
 * @param  x: X location of left corner
 * @param  y: Y location of up corner
 * @param  col: Color
 * @retval None
 */
void gui_text(const char* txt, uint8_t x, uint8_t y, uint8_t col);
/**
 * @brief  Draw text with params
 * @param  txt: Text
 * @param  x: X location of left corner
 * @param  y: Y location of up corner
 * @param  w: Width
 * @param  h: Height
 * @param  bg: lable's background color. Text color is !bg
 * @param  border: width of border. border's color is !bg
 * @retval None
 */
void gui_lable(const char* txt, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t bg, uint8_t border);

/**
 * @brief  Draw multiline text with params
 * @param  txt: Text
 * @param  x: X location of left corner
 * @param  y: Y location of up corner
 * @param  w: Width
 * @param  h: Height
 * @param  bg: lable's background color. Text color is !bg
 * @param  border: width of border. border's color is !bg
 * @retval None
 */
void gui_lable_multiline(const char* txt, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t bg, uint8_t border);


//running line
#define GUI_TickerSpeed 500
#define GUI_TickerEndDelay 3
class GUI_TickerData 
{
	public:
		GUI_TickerData(const char * txt, uint8_t X, uint8_t Y, uint8_t W, uint8_t H) 
			: text(txt), x(X), y(Y), w(W), h(H), bg(SSD1306_COLOR_BLACK), border(SSD1306_COLOR_WHITE), startTick(0) {
		}
	const char *text;
	uint8_t x, y, w, h, bg, border;
	uint32_t startTick;
} ;
/**
 * @brief  Draw running text line using GUI_TickerData structure
 * @param  txt: Text
 * @param  td: GUI_TickerData
 * @retval None
 */
void gui_ticker(GUI_TickerData *dt);


struct GUI_ListItemData {
	GUI_ListItemData(uint8_t id1, const char *msg, bool scroll, uint16_t timeBetwenScrolls) :
		id(id1), text(msg), Scrollable(scroll), TimeBetweenScroll(timeBetwenScrolls), LastScrollTime(0), LastScrollPosition(0){

	}
	GUI_ListItemData(uint8_t id, const char *msg) : Scrollable(0), TimeBetweenScroll(1000), LastScrollTime(0), LastScrollPosition(0) {
		this->id = id;
		text = msg;
	}
	GUI_ListItemData() : id(0), text(0), Scrollable(0), TimeBetweenScroll(1000), LastScrollTime(0), LastScrollPosition(0) {}
	void set(uint8_t n, const char *msg) {
		id=n;
		text = msg;
	}
	uint16_t id; /*!< Item's id */
	const char* text;  /*!< Item's text*/
	uint16_t Scrollable : 1;
	uint16_t TimeBetweenScroll : 12;
	uint32_t LastScrollTime;
	uint8_t LastScrollPosition;
	const char *getScrollOffset();
	void setShouldScroll();
	void resetScrollable() {Scrollable = 1;LastScrollTime=0;LastScrollPosition=0;}
};

struct GUI_ListData {
	GUI_ListData(const char *h, GUI_ListItemData *is, uint8_t x, uint8_t y, uint8_t w, uint8_t height, uint8_t si, uint8_t ic) {
		header = h;
		items = is;
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = height;
		selectedItem = si;
		ItemsCount = ic;
	}
	const char* header;  /*!< Header*/
	GUI_ListItemData *items;  /*!< Item's array*/
	uint16_t ItemsCount; /*!< Item's array*/
	uint8_t x, y, w, h;
	uint16_t selectedItem;
	
};

extern GUI_ListData *gui_CurList;

/**
 * @brief  Ser current GUI_List element
 * @param  ld: GUI_ListItemData
 * @retval None
 */
void gui_set_curList(GUI_ListData* list);
/**
 * @brief  draw current GUI_List
 * @note   DO NOT USE IT BY YOURSELF! DO NOT FORGET gui_set_curList(data) first!
 * @param  ld: GUI_ListItemData
 * @retval None
 */
uint8_t gui_draw_list(void);


void gui_draw(void);
#endif

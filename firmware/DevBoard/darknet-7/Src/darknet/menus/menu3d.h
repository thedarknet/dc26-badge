#ifndef MENU3D
#define MENU3D

#include <app/app.h>
#include <app/state_base.h>
#include "darknet7_base_state.h"
#include "3d/renderer.h"
#include "3d/vec_math.h"

namespace cmdc0de {
	class DisplayDevice;
	class RGBColor;
}


class Menu3D : public Darknet7BaseState {
public:
	Menu3D();
	virtual ~Menu3D();
public:
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
	void initMenu3d();
	void update();
	void render();
	void line(int x0, int y0, int x1, int y1, cmdc0de::RGBColor& color);
private:
	Model model;
	Vec3f light_dir;
	Vec3f eye;
	static const Vec3f center;
	static const Vec3f up;
	uint8_t CanvasWidth;
	uint8_t CanvasHeight;
};

#endif

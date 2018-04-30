#ifndef MENU3D
#define MENU3D

#include <app/app.h>
#include <app/state_base.h>
#include "renderer.h"

namespace cmdc0de {
	class DisplayDevice;
	class RGBColor;
}

class Model;

class Menu3D : public cmdc0de::StateBase {
public:
	Menu3D(cmdc0de::DisplayDevice * const dd);
	virtual ~Menu3D();
public:
protected:
	virtual cmdc0de::ErrorType onInit();
	virtual cmdc0de::StateBase::ReturnStateContext onRun();
	virtual cmdc0de::ErrorType onShutdown();
	void initMenu3d();
	void update();
	void render();
	cmdc0de::DisplayDevice &getDisplay() {return *DD;}
	void line(int x0, int y0, int x1, int y1, cmdc0de::RGBColor& color);
private:
	Model model;
	Vec3f light_dir;
	Vec3f eye;
	static const Vec3f center;
	static const Vec3f up;
	uint8_t CanvasWidth;
	uint8_t CanvasHeight;
	cmdc0de::DisplayDevice *const DD;
};

#endif

#include <stdlib.h>
#include "stdint.h"
#include <display/display_device.h>
#include "3d/renderer.h"
#include "menu3d.h"
#include "../darknet7.h"
#include "menu_state.h"

using cmdc0de::RGBColor;

/*
 * The model, view and projection matrices are three separate matrices. Model maps from an object's local coordinate
 * space into world space, view from world space to camera space, projection from camera to screen.
 *
 * https://en.wikipedia.org/wiki/Triangle_strip
 */
/*
 *
 * #define VERTEX_COUNT 64
#define EDGES_COUNT 108
#define DEMENSIONS_SIZE 3
#define EDGES_SIZE 2
//free RAM need to draw it: 1112 bytes
float model_vertex[VERTEX_COUNT][DEMENSIONS_SIZE] = {
	{-0.247767,0.500000,1.208095},
	{0.245151,0.500000,1.208095},
	{-0.251254,-0.500268,-1.148982},
	{-0.497713,-0.250268,-1.148982},
	{-1.216684,0.500000,-0.587966},
	{-1.216684,-0.500000,-0.587966},
	{-2.715060,2.524469,1.465111},
	{-2.715060,2.524469,-1.543829},
	{-1.216684,0.500000,0.590483},
	{-0.494226,1.000000,0.782811},
	{0.491610,-0.250000,1.208095},
	{0.491610,0.250000,1.208095},
	{0.491610,1.000000,0.782812},
	{-1.216684,-0.500000,0.590483},
	{-0.497713,0.249732,-1.148982},
	{2.700097,-2.516445,-1.533913},
	{2.700096,0.004668,-1.945888},
	{2.700096,-2.516445,1.475027},
	{2.700096,0.004668,1.886542},
	{1.176752,-0.500000,0.590483},
	{2.702593,-0.342869,0.349148},
	{0.241663,-0.500268,-1.148982},
	{0.488122,-0.250268,-1.148982},
	{2.700096,2.513917,-1.533913},
	{-0.494225,1.000000,-0.781078},
	{0.491611,1.000000,-0.781077},
	{0.753795,1.000000,-0.499999},
	{1.176752,-0.500000,-0.587966},
	{-0.494225,0.250000,1.208095},
	{-0.494225,-0.250000,1.208095},
	{-0.251255,0.499732,-1.148982},
	{0.241663,0.499732,-1.148982},
	{0.753794,1.000000,0.500001},
	{0.814407,-1.000000,0.500000},
	{0.814407,-1.000000,-0.500000},
	{2.700096,2.513917,1.475027},
	{-2.719800,0.336161,-0.352381},
	{-2.719800,-0.341941,-0.352382},
	{-0.494225,-1.000000,0.782811},
	{-0.247766,-0.500000,1.208095},
	{2.702593,-0.342869,-0.337940},
	{-0.494225,-1.000000,-0.781078},
	{-0.813734,1.000000,-0.500000},
	{-0.813734,1.000000,0.500000},
	{-2.715060,0.015220,-1.955804},
	{1.176752,0.500000,-0.587966},
	{-2.715060,-2.505893,1.465111},
	{-2.715060,-2.505893,-1.543829},
	{1.176752,0.500000,0.590483},
	{2.702593,0.335233,0.349149},
	{-2.715060,0.015220,1.876626},
	{0.491610,-1.000000,-0.781078},
	{0.491610,-1.000000,0.782811},
	{0.245151,-0.500000,1.208095},
	{-0.813734,-1.000000,0.500000},
	{-0.813734,-1.000000,-0.500000},
	{-2.719800,0.336161,0.334707},
	{-2.719800,-0.341941,0.334707},
	{0.488122,0.249732,-1.148982},
	{2.702593,0.335234,-0.337940},
	{-0.275995,-0.735812,-0.987681},
	{0.216922,-0.735812,-0.987681},
	{-0.285291,-0.730416,-1.252644},
	{0.207626,-0.730416,-1.252644},
};
 *
 */
#if 1
static const VertexStruct Cube[] = {
	{	{	-.9, -.9, .9}, 	{0, 0, 0}},
	{	{	.9, -.9, .9}, 	{0, 0, 0}},
	{	{	-.9, .9, .9}, 	{0, 0, 0}},
	{	{	.9, .9, .9}, 	{0, 0, 0}},
	{	{	-.9, -.9, -.9}, 	{0, 0, 0}},
	{	{	.9, -.9, -.9}, 	{0, 0, 0}},
	{	{	-.9, .9, -.9}, 	{0, 0, 0}},
	{	{	.9, .9, -.9}, 	{0, 0, 0}}
};

static const uint16_t CubeIndexes[] = {
	0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
};
#else

const struct VertexStruct Cube[] = {
		/* CUBE: 24 vertices */
		{ { 1.000000f, 1.000000f, -1.000000f }, { 255, 0, 0 } },
		{ { 1.000000f, -1.000000f, -1.000000f }, { 0, 255, 0 } },
		{ { -1.000000f, -1.000000f, -1.000000f }, { 0, 0, 255 } },
		{ { -1.000000f, 1.000000f, -1.000000f }, { 0, 0, 255 } },
		{ { 1.000000f, 0.999999f, 1.000000f }, { 0, 0, 255 } },
		{ { -1.000000f, 1.000000f, 1.000000f }, { 0, 0, 255 } },
		{ { -1.000000f, -1.000000f, 1.000000f }, { 0, 0, 255 } },
		{ { 0.999999f, -1.000001f, 1.000000f }, { 0, 0, 255 } },
		{ { 1.000000f, 1.000000f, -1.000000f }, { 0, 0, 255 } },
		{ { 1.000000f, 0.999999f, 1.000000f }, { 0, 0, 255 } },
		{ { 0.999999f, -1.000001f, 1.000000f }, { 0, 0, 255 } },
		{ { 1.000000f, -1.000000f, -1.000000f }, { 0, 0, 255 } },
		{ { 1.000000f, -1.000000f, -1.000000f }, { 0, 0, 255 } },
		{ { 0.999999f, -1.000001f, 1.000000f }, { 0, 0, 255 } },
		{ { -1.000000f, -1.000000f, 1.000000f }, { 0, 0, 255 } },
		{ { -1.000000f, -1.000000f, -1.000000f }, { 0, 0, 255 } },
		{ { -1.000000f, -1.000000f, -1.000000f }, { 0, 0, 255 } },
		{ { -1.000000f, -1.000000f, 1.000000f }, { 0, 0, 255 } },
		{ { -1.000000f, 1.000000f, 1.000000f }, { 0, 0, 255 } },
		{ { -1.000000f, 1.000000f, -1.000000f }, { 0, 0, 255 } },
		{ { 1.000000f, 0.999999f, 1.000000f }, { 0, 0, 255 } },
		{ { 1.000000f, 1.000000f, -1.000000f }, { 0, 0, 255 } },
		{ { -1.000000f, 1.000000f, -1.000000f }, { 0, 0, 255 } },
		{ { -1.000000f, 1.000000f, 1.000000f }, { 0, 0, 255 } },
};

const unsigned short CubeIndexes[] = {
		/* CUBE 12 faces */
		0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21,
		22, 20, 22, 23,
};
#endif

const Vec3f Menu3D::center(0, 0, 0);
const Vec3f Menu3D::up(0, 1, 0);

Menu3D::Menu3D() : Darknet7BaseState(),
		model(), light_dir(), eye(), CanvasWidth(0), CanvasHeight(0) {
}

Menu3D::~Menu3D() {

}

enum INTERNAL_STATE {
	INIT, RENDER
};
static INTERNAL_STATE InternalState = INIT;
static float rotation = 0.0f;

cmdc0de::ErrorType Menu3D::onInit() {
	//initGame();
	InternalState = INIT;
	model.set(&Cube[0], (sizeof(Cube) / sizeof(Cube[0])), &CubeIndexes[0], sizeof(CubeIndexes) / sizeof(CubeIndexes[0]),
			Model::STRIPS);
	light_dir = Vec3f(1, 1, 1);
	eye = Vec3f(0, 2, 40);
	rotation = 0.0f;
	return cmdc0de::ErrorType();
}

void Menu3D::initMenu3d() {
	CanvasHeight = DarkNet7::get().getDisplay().getHeight()-20;
	CanvasWidth = DarkNet7::get().getDisplay().getWidth()-60;
	lookat(eye, center, up);
	viewport((CanvasWidth / 8)+10, (CanvasHeight / 8), CanvasWidth * 0.75, CanvasHeight * 0.75);
	//viewport(18, 13, 82, 90);
	projection(-1.f / (eye - center).norm());
	light_dir.normalize();
	//model.scale(.5);
	DarkNet7::get().getDisplay().fillScreen(RGBColor::BLACK);
}

cmdc0de::StateBase::ReturnStateContext Menu3D::onRun() {
	cmdc0de::StateBase::ReturnStateContext sr(this);
	if(DarkNet7::get().getButtonInfo().wasAnyButtonReleased()) {
		sr.NextMenuToRun = DarkNet7::get().getDisplayMenuState();
	}

	switch (InternalState) {
		case INIT:
			initMenu3d();
			InternalState = RENDER;
			break;
		case RENDER:
			update();
			render();
			break;
	}
	return sr;
}

cmdc0de::ErrorType Menu3D::onShutdown() {
	return cmdc0de::ErrorType();
}

static uint32_t renderTime = 0, count = 0, total_frames = 0;

void Menu3D::update() {
	model.setTransformation(rotation);
	rotation += 0.05f;
}

inline void swap(int &x, int &y) {
	int tmp = x;
	x = y;
	y = tmp;
}

void Menu3D::line(int x0, int y0, int x1, int y1, RGBColor& color) {
	bool steep = false;
	if (abs(x0 - x1) < abs(y0 - y1)) {
		swap(x0, y0);
		swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	for (int x = x0; x <= x1; x++) {
		float t = (x - x0) / (float) (x1 - x0);
		int y = y0 * (1. - t) + y1 * t;
		if (x >= 0 && x < CanvasWidth && y >= 0 && y < CanvasHeight) {
			if (steep) {
				DarkNet7::get().getDisplay().drawPixel(y, x, color);
			} else {
				DarkNet7::get().getDisplay().drawPixel(x, y, color);
			}
		}
	}
}

void Menu3D::render() {
	DarkNet7::get().getDisplay().fillRec(0, 0, 128, CanvasHeight, RGBColor::BLACK);
	//rc.getDisplay().fillRec(0, 0, rc.getDisplay().getWidth(), rc.getDisplay().getHeight(), RGBColor::BLACK);

	RGBColor c = RGBColor::WHITE;
	FlatShader shader;
	Matrix modelViewProj = Projection * ModelView * model.getModelTransform();
	shader.setLightDir(light_dir);
	//Vec3i screen_coords[3];
	for (uint32_t i = 0; i < model.nFaces(); i++) {
		for (int j = 0; j < 3; j++) {
			Vec3i v0 = shader.vertex(modelViewProj, model, i, j);
			Vec3i v1 = shader.vertex(modelViewProj, model, i, (j + 1) % 3);
			line(v0.x, v0.y, v1.x, v1.y, c);
		}
	}
	if (HAL_GetTick() - renderTime > 1000) {
		char buf[24];
		sprintf(&buf[0], "FPS: %lu:%lu", count, total_frames);
		DarkNet7::get().getDisplay().drawString(0, 110, &buf[0]);
		count = 0;
		renderTime = HAL_GetTick();
	}
	++count;
	total_frames++;
}


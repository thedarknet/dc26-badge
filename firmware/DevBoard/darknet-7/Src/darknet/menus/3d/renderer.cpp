#include "renderer.h"
#include <display/display_device.h>
#include <logger.h>
#include <stdlib.h>

using cmdc0de::RGBColor;

class Timer {
public:
	Timer(const char *n) :
			Name(n), Stop(0), diff(0) {
		Start = HAL_GetTick();
	}
	void stop() {
		Stop = HAL_GetTick();
		diff = Stop - Start;
	}
	~Timer() {
		stop();
		INFOMSG("%s: %u", Name, HAL_GetTick() - Start);
	}
private:
	const char *Name;
	uint32_t Start;
	uint32_t Stop;
	uint32_t diff;
};

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

Model::Model() :
		Verts(0), NumVerts(0), Indexes(0), NumIndexes(0), ModelTransform(
				Matrix::identity()), Format(VERTS) {

}

void Model::set(const VertexStruct *v, uint16_t nv, const uint16_t *i,
		uint16_t ni, MODEL_FORMAT format) {
	Verts = v;
	NumVerts = nv;
	Indexes = i;
	NumIndexes = ni;
	Format = format;
}

#ifdef NORMALS
const Vec3f &Model::normal(uint16_t face, uint8_t nVert) const {
	if (Format == VERTS)
	return Verts[Indexes[(face * 3) + nVert]].normal;
	else
	if (face == 0) {
		return Verts[Indexes[nVert]].normal;
	}
	return Verts[Indexes[(face + 2) + nVert]].normal;
}
#endif

const Vec3f &Model::vert(uint16_t face, uint8_t nVert) const {
	if (Format == VERTS)
		return Verts[Indexes[(face * 3) + nVert]].pos;
	else
		return Verts[Indexes[face + nVert]].pos;
}

uint32_t Model::nFaces() const {
	if (Format == VERTS)
		return NumIndexes / 3;
	else
		return NumIndexes - 2;
}

IShader::IShader() :
		LightDir() {

}

const Vec3f &IShader::getLightDir() const {
	return LightDir;
}

IShader::~IShader() {
}

void IShader::setLightDir(const Vec3f &ld) {
	LightDir = ld;
}

FlatShader::FlatShader() :
		varying_tri() {
}

FlatShader::~FlatShader() {
}

Vec3i FlatShader::vertex(const Matrix &ModelViewProj, const Model &model,
		int iface, int nthvert) {
	Vec4f gl_Vertex = embed<4>(model.vert(iface, nthvert));
	gl_Vertex = ModelViewProj * gl_Vertex;
	varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
	gl_Vertex = Viewport * gl_Vertex;
	return proj<3>(gl_Vertex / gl_Vertex[3]);
}

bool FlatShader::fragment(Vec3f bar, RGBColor &color) {
	UNUSED(bar);
	Vec3f n = cross(varying_tri.col(1) - varying_tri.col(0),
			varying_tri.col(2) - varying_tri.col(0)).normalize();
	float intensity = CLAMP(n * getLightDir(), 0.f, 1.f);
	color = RGBColor(255 * intensity, 255 * intensity, 255 * intensity);
	return false;
}

GouraudShader::GouraudShader() :
		varying_tri(), varying_ity() {
}

GouraudShader::~GouraudShader() {
}

Vec3i GouraudShader::vertex(const Matrix &ModelViewProj, const Model &model,
		int iface, int nthvert) {
	Vec4f gl_Vertex = embed<4>(model.vert(iface, nthvert));
	gl_Vertex = ModelViewProj * gl_Vertex;
	varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));

	varying_ity[nthvert] = CLAMP(model.normal(iface, nthvert) * getLightDir(),
			0.f, 1.f);

	gl_Vertex = Viewport * gl_Vertex;
	return proj<3>(gl_Vertex / gl_Vertex[3]);
}

bool GouraudShader::fragment(Vec3f bar, RGBColor &color) {
	float intensity = varying_ity * bar;
	color = RGBColor(255 * intensity, 255 * intensity, 255 * intensity);
	return false;
}

ToonShader::ToonShader() :
		varying_tri(), varying_ity() {
}

ToonShader::~ToonShader() {
}

Vec3i ToonShader::vertex(const Matrix &ModelViewProj, const Model &model,
		int iface, int nthvert) {
	Vec4f gl_Vertex = embed<4>(model.vert(iface, nthvert));
	gl_Vertex = ModelViewProj * gl_Vertex;
	varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));

	varying_ity[nthvert] = CLAMP(model.normal(iface, nthvert) * getLightDir(),
			0.f, 1.f);

	gl_Vertex = Viewport * gl_Vertex;
	return proj<3>(gl_Vertex / gl_Vertex[3]);
}

bool ToonShader::fragment(Vec3f bar, RGBColor &color) {
	float intensity = varying_ity * bar;
	if (intensity > .85)
		intensity = 1;
	else if (intensity > .60)
		intensity = .80;
	else if (intensity > .45)
		intensity = .60;
	else if (intensity > .30)
		intensity = .45;
	else if (intensity > .15)
		intensity = .30;
	color = RGBColor(255 * intensity, 155 * intensity, 0);
	return false;
}

void viewport(int x, int y, int w, int h) {
	Viewport = Matrix::identity();
	Viewport[0][3] = x + w / 2.f;
	Viewport[1][3] = y + h / 2.f;
	Viewport[2][3] = 255.f / 2.f;
	Viewport[0][0] = w / 2.f;
	Viewport[1][1] = h / 2.f;
	Viewport[2][2] = 255.f / 2.f;
}

void projection(float coeff) {
	Projection = Matrix::identity();
	Projection[3][2] = coeff;
}

void lookat(const Vec3f &eye, const Vec3f &target, const Vec3f &up) {
	Vec3f z = (eye - target).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	ModelView = Matrix::identity();
	for (int i = 0; i < 3; i++) {
		ModelView[0][i] = x[i];
		ModelView[1][i] = y[i];
		ModelView[2][i] = z[i];
		ModelView[i][3] = -target[i];
	}
}

Vec3f barycentric(const Vec3i &A, const Vec3i &B, const Vec3i &C,
		const Vec3i &P) {
	Vec3i s[2];
	for (int i = 2; i--;) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	Vec3f u = cross(s[0], s[1]);
	if (abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3i *pts, IShader &shader, BitArray &zbuffer,
		cmdc0de::DisplayDevice *display, const Vec2i &bboxmin,
		const Vec2i &bboxmax, uint16_t canvasWidth) {
	//Timer bt("bbbox");
	/*Vec2i bboxmin( INT16_MAX, INT16_MAX);
	 Vec2i bboxmax(INT16_MIN, INT16_MIN);
	 for (int i = 0; i < 3; i++) {
	 for (int j = 0; j < 2; j++) {
	 bboxmin[j] = bboxmin[j] < pts[i][j] ? bboxmin[j] : pts[i][j];
	 bboxmax[j] = bboxmax[j] > pts[i][j] ? bboxmax[j] : pts[i][j];
	 }
	 */
	//bt.stop();
	Vec3i P;
	RGBColor color(RGBColor::BLACK);

	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f c = barycentric(pts[0], pts[1], pts[2], P);
			float tmp = pts[0].z * c.x + pts[1].z * c.y + pts[2].z * c.z + .5;
			P.z = int(tmp * 7.0f); //scale Z for int math
			//P.z = std::max(0, std::min(255, int(pts[0].z * c.x + pts[1].z * c.y + pts[2].z * c.z + .5))); // clamping to 0-255 since it is stored in unsigned char
			if (c.x < 0 || c.y < 0
					|| zbuffer.getValueAsByte(P.y * 128 + P.x) > P.z)
				continue;
			bool discard = shader.fragment(c, color);
			if (!discard) {
				zbuffer.setValueAsByte(P.y * canvasWidth + P.x, P.z);
				if (color.getR() < 85 && color.getG() < 85
						&& color.getB() < 85) {
					display->drawPixel(P.x, P.y, RGBColor::WHITE);
				} else {
					display->drawPixel(P.x, P.y, color);
				}
			} else {
				display->drawPixel(P.x, P.y, RGBColor::WHITE);
			}
		}
	}
}

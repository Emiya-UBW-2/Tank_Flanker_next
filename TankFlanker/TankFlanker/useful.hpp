#pragma once
#include <string>
#include <string_view>
#include "DXLib_ref.h"

using std::int8_t;
using std::size_t;
using std::uint16_t;
using std::uint8_t;

constexpr float M_GR = -9.8f;				  /*重力加速度*/

inline const int deskx = (GetSystemMetrics(SM_CXSCREEN)); /*デスクトップX*/
inline const int desky = (GetSystemMetrics(SM_CYSCREEN)); /*デスクトップY*/

//マウス判定
#define in2_(mx, my, x1, y1, x2, y2) (mx >= x1 && mx <= x2 && my >= y1 && my <= y2)
#define in2_mouse(x1, y1, x2, y2) (in2_(mousex, mousey, x1, y1, x2, y2))
//その他
template <typename T>
static float deg2rad(T p1) {
	return float(p1) * DX_PI_F / 180.f;
} //角度からラジアンに
template <typename T>
static float rad2deg(T p1) {
	return float(p1) * 180.f / DX_PI_F;
} //ラジアンから角度に
//
static std::string getright(const char* p1) {
	std::string tempname = p1;
	return tempname.substr(tempname.find('=') + 1);
}
//
static auto getparam_char(int p1) {
	char mstr[64];
	FileRead_gets(mstr, 64, p1);
	return getright(mstr).c_str();
}
static auto getparam_str(int p1) {
	char mstr[64];
	FileRead_gets(mstr, 64, p1);
	return getright(mstr);
}
static auto get_str(int p1) {
	char mstr[64];
	FileRead_gets(mstr, 64, p1);
	return std::string(mstr);
}

static const long int getparam_i(int p1) {
	char mstr[64];
	FileRead_gets(mstr, 64, p1);
	return std::stol(getright(mstr));
}
static const unsigned long int getparam_u(int p2) {
	char mstr[64];
	FileRead_gets(mstr, 64, p2);
	return std::stoul(getright(mstr));
}
static const float getparam_f(int p1) {
	char mstr[64];
	FileRead_gets(mstr, 64, p1);
	return std::stof(getright(mstr));
}
static const bool getparam_bool(int p1) {
	char mstr[64];
	FileRead_gets(mstr, 64, p1);
	return (getright(mstr).find("true") != std::string::npos);
}
//
void easing_set(float* first, const float& aim, const float& ratio, const float& fps) {
	if (ratio == 0.f) {
		*first = aim;
	}
	else {
		if (aim != 0.f) {
			*first += (aim - *first) * (1.f - powf(ratio, 60.f / fps));
		}
		else {
			*first *= powf(ratio, 60.f / fps);
		}
	}
};
//
void easing_set(VECTOR_ref* first, const VECTOR_ref& aim, const float& ratio, const float& fps) {
	if (ratio == 0.f) {
		*first = aim;
	}
	else {
		*first += (VECTOR_ref(aim) - *first) * (1.f - powf(ratio, 60.f / fps));
	}
};

//ID割り当て
template <class T>
void fill_id(std::vector<T>& vect) {
	for (int i = 0; i < vect.size(); i++) {
		vect[i].id = i;
	}
}
template <class T, size_t N>
void fill_id(std::array<T, N>& vect) {
	for (int i = 0; i < vect.size(); i++) {
		vect[i].id = i;
	}
}
//
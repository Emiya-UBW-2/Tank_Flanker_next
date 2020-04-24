#pragma once
#include "DxLib.h"
#include <string_view>
#include <string>

using std::size_t;
using std::uint8_t;
using std::int8_t;
using std::uint16_t;

inline const int deskx = (GetSystemMetrics(SM_CXSCREEN)); /*デスクトップX*/
inline const int desky = (GetSystemMetrics(SM_CYSCREEN)); /*デスクトップY*/
inline const int dispx = deskx; /*描画X*/
inline const int dispy = desky; /*描画Y*/
inline const int out_dispx = dispx; /*ウィンドウX*/
inline const int out_dispy = dispy / 2;// +256; /*ウィンドウY*/
//リサイズ
#define x_r(p1) (int(p1) * dispx / deskx)
#define y_r(p1) (int(p1) * dispy / desky)
//マウス判定
#define in2_(mx,my,x1, y1, x2, y2) (mx >= x1 && mx <= x2 && my >= y1 && my <= y2)
#define in2_mouse(x1, y1, x2, y2) (in2_(mousex,mousey,x1, y1, x2, y2))
//その他
template <typename T>
static float deg2rad(T p1) { return float(p1)*DX_PI_F / 180.f; }//角度からラジアンに
template <typename T>
static float rad2deg(T p1) { return float(p1)*180.f / DX_PI_F;}//ラジアンから角度に
//
typedef std::pair<size_t, float> pair;
//
static size_t count_impl(std::basic_string_view<TCHAR> pattern) {
	WIN32_FIND_DATA win32fdt;
	size_t cnt = 0;
	const auto hFind = FindFirstFile(pattern.data(), &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (win32fdt.cFileName[0] != '.')
				++cnt;
		} while (FindNextFile(hFind, &win32fdt));
	}
	FindClose(hFind);
	return cnt;
}
static size_t count_team(std::string stage) { return count_impl("stage/" + stage + "/team/*.txt"); }
static size_t count_enemy(std::string stage) { return count_impl("stage/" + stage + "/enemy/*.txt"); }
//
static std::string getright(const char* p1) {
	std::string tempname = p1;
	return tempname.substr(tempname.find('=') + 1);
}
//
static MATRIX_ref Axis1(VECTOR_ref xvec, VECTOR_ref yvec, VECTOR_ref zvec) noexcept { return { DxLib::MGetAxis1(xvec.get(),yvec.get(),zvec.get(),VGet(0,0,0)) }; }
static MATRIX_ref Axis1(VECTOR_ref xvec, VECTOR_ref yvec, VECTOR_ref zvec, VECTOR_ref pos) noexcept { return { DxLib::MGetAxis1(xvec.get(),yvec.get(),zvec.get(),pos.get()) }; }
static MATRIX_ref RotX(const float& rad) noexcept { return { DxLib::MGetRotX(rad) }; }
static MATRIX_ref RotY(const float& rad) noexcept { return { DxLib::MGetRotY(rad) }; }
static MATRIX_ref RotZ(const float& rad) noexcept { return { DxLib::MGetRotZ(rad) }; }
//
static const long int getparam_i(int p1) {
	char mstr[64]; /*tank*/
	FileRead_gets(mstr, 64, p1);
	return std::stol(getright(mstr));
}
static const unsigned long int getparam_u(int p2) {
	char mstr[64]; /*tank*/
	FileRead_gets(mstr, 64, p2);
	return std::stoul(getright(mstr));
}
static const float getparam_f(int p1) {
	char mstr[64]; /*tank*/
	FileRead_gets(mstr, 64, p1);
	return std::stof(getright(mstr));
}
//
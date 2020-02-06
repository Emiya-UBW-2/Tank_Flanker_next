#pragma once

#ifndef INCLUDED_define_h_
#define INCLUDED_define_h_

#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include "Box2D/Box2D.h"
#include "useful.hpp"
#include <windows.h>
#include <thread>
#include <string_view>

class MainClass {
private:
	/*setting*/
	bool USE_GRAV;	       /*人の物理演算のオフ、オン*/
	unsigned char ANTI;    /*アンチエイリアス倍率*/
	bool USE_YSync;	       /*垂直同期*/
	float f_rate;	       /*fps*/
	bool USE_windowmode;   /*ウィンドウor全画面*/
	float drawdist;	       /*木の描画距離*/
	unsigned char gnd_x;   /*地面のクオリティ*/
	unsigned char shade_x; /*影のクオリティ*/
	bool USE_HOST;	       /**/
	bool USE_PIXEL;	       /*ピクセルライティングの利用*/
	float se_vol;	       /**/
	float bgm_vol;	       /**/
public:
	MainClass(void);
	~MainClass(void);
};


#endif

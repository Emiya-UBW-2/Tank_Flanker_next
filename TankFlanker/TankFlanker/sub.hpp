#pragma once

#ifndef INCLUDED_define_h_
#define INCLUDED_define_h_

#define NOMINMAX
#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include "Box2D/Box2D.h"
#include "useful.hpp"
#include "DXLib_vec.hpp"
#include "DXLib_mat.hpp"
#include "MV1ModelHandle.hpp"
#include "EffekseerEffectHandle.hpp"
#include "SoundHandle.hpp"
#include "GraphHandle.hpp"
#include "FontHandle.hpp"
#include <windows.h>
#include <fstream>
#include <string_view>

#include <array>
#include <vector>

struct switches {
	bool flug{ false };
	uint8_t cnt{ 0 };
};

class MainClass {
private:
	/*setting*/
	bool USE_GRAV;	       /*人の物理演算のオフ、オン*/
	unsigned char ANTI;    /*アンチエイリアス倍率*/
	bool USE_YSync;	       /*垂直同期*/
	float frate;	       /*fps*/
	bool USE_windowmode;   /*ウィンドウor全画面*/
	float drawdist;	       /*木の描画距離*/
	unsigned char gnd_x;   /*地面のクオリティ*/
	unsigned char shade_x; /*影のクオリティ*/
	bool USE_HOST;	       /**/
	bool USE_PIXEL;	       /*ピクセルライティングの利用*/
	float se_vol;	       /**/
	float bgm_vol;	       /**/
	/**/		       /**/


public:
	/*setting*/
	inline const auto get_GRAV(void) { return USE_GRAV; }
	inline const auto get_gnd(void) { return gnd_x; }
	inline const auto get_shade(void) { return shade_x; }
	inline const auto get_drawdist(void) { return drawdist; }
	inline const auto get_frate(void) { return frate; }
	inline const auto get_host(void) { return USE_HOST; }
	inline const auto get_se_vol(void) { return se_vol; }
	inline const auto get_bgm_vol(void) { return bgm_vol; }
	void write_setting(void);
	MainClass(void);
	~MainClass(void);
	/*draw*/
	void set_light(const VECTOR_ref vec);
	void set_cam(const float neard, const float fard, const VECTOR_ref cam, const VECTOR_ref view, const VECTOR_ref up, const float fov); //カメラ情報指定
	void Screen_Flip(void);
	void Screen_Flip(LONGLONG waits);


};
class VEHICLE : public MainClass
{
private:
	struct vehicle {
		std::string name;		  /*名前*/
		int countryc;			  /*国*/
		MV1ModelHandle model;		  /*モデル*/
		MV1ModelHandle colmodel;	  /*コリジョン*/
		MV1ModelHandle inmodel;		  /*内装*/
		std::array<float, 4> speed_flont; /*前進*/
		std::array<float, 4> speed_back;  /*後退*/
		float vehicle_RD = 0.0f;	  /*旋回速度*/
		std::array<float, 4> armer;       /*装甲*/
		bool gun_lim_LR = 0;		  /*砲塔限定旋回の有無*/
		std::array<float, 4> gun_lim_;    /*砲塔旋回制限*/
		float gun_RD = 0.0f;		  /*砲塔旋回速度*/
		std::array<float, 3> gun_speed;   /*弾速*/
		std::array<float, 3> pene;	/*貫通*/
		std::array<int, 3> ammotype;      /*弾種*/
		std::vector<VECTOR_ref> loc;      /*フレームの元座標*/
		VECTOR_ref min;			  /*box2D用フレーム*/
		VECTOR_ref max;			  /*box2D用フレーム*/
		int turretframe;		  /*砲塔フレーム*/
		struct guninfo {
			int gunframe;   /*銃フレーム*/
			int reloadtime; /*リロードタイム*/
			float ammosize; /*砲口径*/
			int accuracy;   /*砲精度*/
		};
		std::array<guninfo, 2> gun_; /*銃フレーム*/
		std::vector<int> youdoframe;    /*誘導輪*/
		std::vector<int> wheelframe;    /*転輪*/
		std::array<int, 2> kidoframe;   /*起動輪*/
		std::array<int, 2> smokeframe;  /*排煙*/
		std::vector<int> upsizeframe;   /*履帯上*/
		int engineframe;		/*エンジン*/
	};
	std::vector<vehicle> vecs; /*車輛情報*/
	std::array<SoundHandle, 13> se_; /*効果音*/
	std::array<SoundHandle, 6> bgm_; /*効果音*/
public:
	VEHICLE();
	~VEHICLE();
};

#endif

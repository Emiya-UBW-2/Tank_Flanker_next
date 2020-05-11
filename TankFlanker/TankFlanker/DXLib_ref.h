#pragma once
#include "DxLib.h"
#include"EffekseerForDXLib.h"

#include"DXLib_mat.hpp"

#include "SoundHandle.hpp"
#include "GraphHandle.hpp"
#include "FontHandle.hpp"
#include "MV1ModelHandle.hpp"
#include "EffekseerEffectHandle.hpp"

#include "useful.hpp"

#include <array>
#include <list>
#include <vector>
enum Effect {
	ef_fire = 0, //発砲炎
	ef_reco = 1, //大口径跳弾
	ef_reco2 = 2, //小口径跳弾
	ef_gndhit = 3, //大口径着弾
	ef_gndhit2 = 4, //小口径着弾
	ef_bomb = 5, //撃破爆発
	ef_smoke1 = 6, //ミサイル炎
	ef_smoke2 = 7, //銃の軌跡
	effects = 8, //読み込む
	efs_user = 8  //
};
struct EffectS {
	bool flug{ false };		 /**/
	size_t id = 0;
	Effekseer3DPlayingHandle handle; /**/
	VECTOR_ref pos;			 /**/
	VECTOR_ref nor;			 /**/
	float scale = 1.f;		 /**/
};

class DXDraw {
private:
	const bool use_shadow = true;			     /*影描画*/
	int shadow_near = 0;			     /*近影*/
	int shadow_far = 0;			     /*遠影*/
	bool use_pixellighting = false;			     /**/
	bool use_vsync = false;				     /*垂直同期*/
	float frate = 60.f;				     /*フレームレート*/
	std::array<EffekseerEffectHandle, effects> effHndle; /*エフェクトリソース*/
	EffekseerEffectHandle gndsmkHndle;		     /*エフェクトリソース*/
	int disp_x = 1920;
	int disp_y = 1080;
public:
	EffekseerEffectHandle& get_effHandle(int p1) noexcept { return effHndle[p1]; }
	const EffekseerEffectHandle& get_effHandle(int p1) const noexcept { return effHndle[p1]; }
	EffekseerEffectHandle& get_gndhitHandle() noexcept { return gndsmkHndle; }
	const EffekseerEffectHandle& get_gndhitHandle() const noexcept { return gndsmkHndle; }

	DXDraw(const char* title, const int& xd, const int& yd, const float& fps = 60.f) {
		disp_x = xd;
		disp_y = yd;

		frate = fps;
		SetOutApplicationLogValidFlag(FALSE);  /*log*/
		SetMainWindowText(title);	       /*タイトル*/
		ChangeWindowMode(TRUE);		       /*窓表示*/
		SetUseDirect3DVersion(DX_DIRECT3D_11); /*directX ver*/
		SetGraphMode(disp_x, disp_y, 32);	       /*解像度*/
		SetUseDirectInputFlag(TRUE);			       /**/
		SetWindowSizeChangeEnableFlag(FALSE, FALSE);	       /*ウインドウサイズを手動不可、ウインドウサイズに合わせて拡大もしないようにする*/
		SetUsePixelLighting(use_pixellighting ? TRUE : FALSE); /*ピクセルシェーダの使用*/
		//SetFullSceneAntiAliasingMode(4, 2);		       /*アンチエイリアス*/
		SetWaitVSyncFlag(use_vsync ? TRUE : FALSE);	       /*垂直同期*/
		DxLib_Init();					       /**/
		Effekseer_Init(8000);				       /*Effekseer*/
		SetChangeScreenModeGraphicsSystemResetFlag(FALSE);     /*Effekseer*/
		Effekseer_SetGraphicsDeviceLostCallbackFunctions();    /*Effekseer*/
		SetAlwaysRunFlag(TRUE);				       /*background*/
		SetUseZBuffer3D(TRUE);				       /*zbufuse*/
		SetWriteZBuffer3D(TRUE);			       /*zbufwrite*/
		SetDrawMode(DX_DRAWMODE_BILINEAR);		       /**/
		//エフェクト
		{
			size_t j = 0;
			for (auto& e : effHndle)
				e = EffekseerEffectHandle::load("data/effect/" + std::to_string(j++) + ".efk");
			gndsmkHndle = EffekseerEffectHandle::load("data/effect/gndsmk.efk");
		}

	}
	~DXDraw(void) {
		Effkseer_End();
		DxLib_End();
	}
	template <typename T>
	bool Set_Shadow(const size_t& scale, const VECTOR_ref& farsize, const VECTOR_ref& Light_dir, T doing) {
		shadow_near = MakeShadowMap(int(pow(2, scale)), int(pow(2, scale)));
		shadow_far = MakeShadowMap(int(pow(2, scale)), int(pow(2, scale)));
		SetShadowMapAdjustDepth(shadow_near, 0.0005f);
		SetShadowMapLightDirection(shadow_near, Light_dir.get());
		SetShadowMapLightDirection(shadow_far, Light_dir.get());
		SetShadowMapDrawArea(shadow_far, (farsize*-1.f).get(), farsize.get());
		ShadowMap_DrawSetup(shadow_far);
		doing();
		ShadowMap_DrawEnd();
		return true;
	}
	bool Delete_Shadow() {
		DeleteShadowMap(shadow_near);
		DeleteShadowMap(shadow_far);
		return true;
	}

	bool Set_light(const VECTOR_ref& Light_dir) {
		SetGlobalAmbientLight(GetColorF(0.80f, 0.75f, 0.70f, 0.0f));
		SetLightDirection(Light_dir.get());
		return true;
	}


	template <typename T>
	bool Ready_Shadow(const VECTOR_ref& pos, T doing, const VECTOR_ref& nearsize) {
		if (use_shadow) {
			SetShadowMapDrawArea(shadow_near, (nearsize*(-1.f) + pos).get(), (VECTOR_ref(nearsize) + pos).get());
			ShadowMap_DrawSetup(shadow_near);
			doing();
			ShadowMap_DrawEnd();
			return true;
		}
		return false;
	}
	template <typename T>
	bool Draw_by_Shadow(T doing) {
		if (use_shadow) {
			SetUseShadowMap(0, shadow_near);
			SetUseShadowMap(1, shadow_far);
		}
		doing();
		if (use_shadow) {
			SetUseShadowMap(0, -1);
			SetUseShadowMap(1, -1);
		}
		return true;
	}
	bool Screen_Flip(const LONGLONG& waits) {
		/*
		{
			int i = 0;
			for (auto& c : colors) {
				DrawFormatString(200, i * 20, c.col, "%06x", c.buf);
				i++;
			}
		}
		*/
		Screen_Flip();
		if (!use_vsync) {
			while (GetNowHiPerformanceCount() - waits < 1000000.0f / frate) {}
		}
		return true;
	}
	bool Screen_Flip(void) {
		ScreenFlip();
		return true;
	}
};

class DeBuG {
private:
	int frate;
	std::vector<std::array<float, 6 + 1>> deb;
	LONGLONG waypoint = 0;
	std::array<float, 6> waydeb{ 0.f };
	size_t seldeb;
	FontHandle font;
	const int fontsize = 12;
public:
	DeBuG(const int& fps_rate = 60) {
		frate = fps_rate;
		font = FontHandle::Create(fontsize, DX_FONTTYPE_EDGE);
		deb.resize(frate);
	}
	void put_way(void) {
		waypoint = GetNowHiPerformanceCount();
		seldeb = 0;
	}
	void end_way(void) {
		if (seldeb < 6)
			waydeb[seldeb++] = (float)(GetNowHiPerformanceCount() - waypoint) / 1000.0f;
	}
	void debug(int xpos, int ypos, float fps, float time) {
		int wide = 180;
		int hight = int(waydeb.size() + 1) * fontsize;
		deb[0][0] = time;
		for (size_t j = deb.size() - 1; j >= 1; --j) {
			deb[j][0] = deb[j - 1][0];
		}
		for (size_t i = 0; i < waydeb.size(); ++i) {
			if (seldeb - 1 <= i) {
				waydeb[i] = waydeb[seldeb - 1];
			}
			deb[0][i + 1] = waydeb[i];
			for (size_t j = std::size(deb) - 1; j >= 1; --j) {
				deb[j][i + 1] = deb[j - 1][i + 1];
			}
		}

		DrawBox(xpos, ypos, xpos + wide, ypos + hight * 100 / frate, GetColor(255, 0, 0), FALSE);
		for (int j = 0; j < int(deb.size() - 1); ++j) {
			for (int i = 0; i < 6; ++i) {
				DrawLine(
					xpos + j * wide / frate, ypos + hight * 100 / frate - int(deb[j][i + 1] * 5.f),
					xpos + (j + 1) * wide / frate, ypos + hight * 100 / frate - int(deb[j + 1][i + 1] * 5.f),
					GetColor(50, 128 + 127 * i / 6, 50));
			}
			DrawLine(
				xpos + j * wide / frate, ypos + hight * 100 / frate - int(deb[j][0] * 5.f),
				xpos + (j + 1) * wide / frate, ypos + hight * 100 / frate - int(deb[j + 1][0] * 5.f),
				GetColor(255, 255, 0));
		}
		const auto c_ffffff = GetColor(255, 255, 255);
		DrawLine(xpos, ypos + hight * 50 / frate, xpos + wide, ypos + hight * 50 / frate, GetColor(0, 255, 0));

		font.DrawStringFormat(xpos, ypos, c_ffffff, "%05.2ffps ( %.2fms)", fps, time);

		font.DrawStringFormat(xpos, ypos + fontsize, c_ffffff, "%d(%.2fms)", 0, waydeb[0]);
		for (size_t j = 1; j < waydeb.size(); ++j) {
			font.DrawStringFormat(xpos, ypos + int(j + 1) * fontsize, c_ffffff, "%d(%.2fms)", j, waydeb[j] - waydeb[j - 1u]);
		}
	}
};
#pragma once
#include "DxLib.h"
#include"EffekseerForDXLib.h"

#include "DXLib_vec.hpp"
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
	ef_fire = 0,
	ef_reco = 1,
	ef_bomb = 2,
	ef_smoke1 = 3,
	ef_smoke2 = 4,
	ef_gndhit = 5,
	ef_gun = 6,
	ef_gndhit2 = 7,
	ef_reco2 = 8,
	effects = 9,   //読み込む
	ef_smoke3 = 9, //読み込まない
	efs_user = 10
};
struct EffectS {
	bool flug{ false };		 /**/
	Effekseer3DPlayingHandle handle; /**/
	VECTOR_ref pos;			 /**/
	VECTOR_ref nor;			 /**/
};

class DXDraw {
private:
	const bool use_shadow = true;
	int shadow_near = 0;/*近影*/
	int shadow_far = 0;/*遠影*/
	VECTOR_ref shadow_nearsize;
	struct color {
		int buf = -1;
		unsigned int col = 0;
	};
	std::vector<color> colors;
	bool use_vsync=false;

	std::array<EffekseerEffectHandle, effects> effHndle; /*エフェクトリソース*/
	EffekseerEffectHandle gndsmkHndle;		     /*エフェクトリソース*/
public:
	EffekseerEffectHandle& get_effHandle(int p1) noexcept { return effHndle[p1]; }
	const EffekseerEffectHandle& get_effHandle(int p1) const noexcept { return effHndle[p1]; }
	EffekseerEffectHandle& get_gndhitHandle() noexcept { return gndsmkHndle; }
	const EffekseerEffectHandle& get_gndhitHandle() const noexcept { return gndsmkHndle; }


	DXDraw(const char* title) {
		SetOutApplicationLogValidFlag(FALSE);					/*log*/
		SetMainWindowText(title);						/*タイトル*/
		SetUsePixelLighting(TRUE);						/*ピクセルシェーダの使用*/
		ChangeWindowMode(TRUE);							/*窓表示*/
		SetUseDirect3DVersion(DX_DIRECT3D_11);					/*directX ver*/
		SetWaitVSyncFlag(use_vsync ? TRUE : FALSE);				/*垂直同期*/
		SetUseDirectInputFlag(TRUE);						/**/
		SetWindowSizeChangeEnableFlag(FALSE, FALSE);				// ウインドウサイズを手動不可、ウインドウサイズに合わせて拡大もしないようにする
		SetGraphMode(dispx, dispy, 32);					/*解像度*/
		SetFullSceneAntiAliasingMode(4, 2);
		DxLib_Init();								/**/

		Effekseer_Init(8000);				    /*Effekseer*/
		SetChangeScreenModeGraphicsSystemResetFlag(FALSE);  /*Effekseer*/
		Effekseer_SetGraphicsDeviceLostCallbackFunctions(); /*Effekseer*/

		SetAlwaysRunFlag(TRUE);							/*background*/
		SetUseZBuffer3D(TRUE);							/*zbufuse*/
		SetWriteZBuffer3D(TRUE);						/*zbufwrite*/
		SetDrawMode(DX_DRAWMODE_BILINEAR);

		//エフェクト
		{
			size_t j = 0;
			for (auto& e : effHndle)
				e = EffekseerEffectHandle::load("data/effect/" + std::to_string(j++) + ".efk");
			gndsmkHndle = EffekseerEffectHandle::load("data/effect/gndsmk.efk");
		}

	}
	~DXDraw(void) {
		colors.clear();
		Effkseer_End();
		DxLib_End();
	}
	template <typename T>
	bool Set_Shadow(const size_t& scale, const VECTOR_ref& nearsize, const VECTOR_ref& farsize, const VECTOR_ref& Light_dir, T doing) {
		shadow_nearsize = nearsize;
		shadow_near = MakeShadowMap(int(pow(2, scale)), int(pow(2, scale)));
		shadow_far = MakeShadowMap(int(pow(2, scale)), int(pow(2, scale)));
		SetGlobalAmbientLight(GetColorF(0.75f, 0.75f, 0.75f, 0.0f));
		SetLightDirection(Light_dir.get());
		SetShadowMapLightDirection(shadow_near, Light_dir.get());
		SetShadowMapLightDirection(shadow_far, Light_dir.get());
		SetShadowMapDrawArea(shadow_far, farsize.Scale(-1.f).get(), farsize.get());
		ShadowMap_DrawSetup(shadow_far);
		doing();
		ShadowMap_DrawEnd();
		return true;
	}
	template <typename T>
	bool Ready_Shadow(const VECTOR_ref& pos, T doing) {
		if (use_shadow) {
			SetShadowMapDrawArea(shadow_near, (shadow_nearsize.Scale(-1.f) + pos).get(), (shadow_nearsize + pos).get());
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
	void SetDraw_Screen(const GraphHandle& screen) {
		SetDrawScreen(screen.get());
		ClearDrawScreen();
	}
	void SetDraw_Screen(const int& screen) {
		SetDrawScreen(screen);
		ClearDrawScreen();
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
		ScreenFlip();
		if (!use_vsync) {
			while (GetNowHiPerformanceCount() - waits < 1000000.0f / 60.0f) {}
		}
		return true;
	}

	unsigned int GetColor(int r, int g, int b) {
		r = std::clamp(r, 0, 255);
		g = std::clamp(g, 0, 255);
		b = std::clamp(b, 0, 255);
		int col = (r << 16 | g << 8 | b);

		//二分探索する
		int min = 0;
		int max = int(colors.size())- 1;

		/* どんな二分探索でもここの書き方を変えずにできる！ */
		if (max > 0) {
			while (true) {
				if (max >= min) {
					int mid = min + (max - min) / 2;//中間
					if ((colors[mid].buf > col)) {//大
						max = mid - 1;
					}
					else if ((colors[mid].buf < col)) {//小
						min = mid + 1;
					}
					else {
						return colors[mid].col;
					}
				}
				else {
					color p;
					p.buf = col;
					p.col = DxLib::GetColor(r, g, b);

					//colors.push_back(p);
					if (max < min) {
						colors.insert(colors.begin() + min, p);
						return colors[min].col;
					}
				}
			}
		}
		else {
			color p;
			p.buf = col;
			p.col = DxLib::GetColor(r, g, b);
			colors.push_back(p);
			return colors.back().col;
		}
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
				DrawLine(xpos + j * wide / frate, ypos + hight * 100 / frate - int(deb[j][i + 1] * 5.f), xpos + (j + 1) * wide / frate, ypos + hight * 100 / frate - int(deb[j + 1][i + 1] * 5.f), GetColor(50, 128 + 127 * i / 6, 50));
			}
			DrawLine(xpos + j * wide / frate, ypos + hight * 100 / frate - int(deb[j][0] * 5.f), xpos + (j + 1) * wide / frate, ypos + hight * 100 / frate - int(deb[j + 1][0] * 5.f), GetColor(255, 255, 0));
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
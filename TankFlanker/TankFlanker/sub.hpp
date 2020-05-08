#pragma once

#define NOMINMAX
#include <D3D11.h>
#include <array>
#include <fstream>
#include <memory>
#include <optional>
#include <vector>
#include "Box2D/Box2D.h"
#include "DXLib_ref.h"
constexpr auto veh_all = 2;//車種
void set_effect(EffectS* efh, VECTOR_ref pos, VECTOR_ref nor, float scale = 1.f) {
	efh->flug = true;
	efh->pos = pos;
	efh->nor = nor;
	efh->scale = scale;
}
void set_pos_effect(EffectS* efh, const EffekseerEffectHandle& handle) {
	if (efh->flug) {
		efh->handle = handle.Play3D();
		efh->handle.SetPos(efh->pos);
		efh->handle.SetRotation(atan2(efh->nor.y(), std::hypot(efh->nor.x(), efh->nor.z())), atan2(-efh->nor.x(), -efh->nor.z()), 0);
		efh->handle.SetScale(efh->scale);
		efh->flug = false;
	}
	//IsEffekseer3DEffectPlaying(player[0].effcs[i].handle)
}
namespace std {
	template <>
	struct default_delete<b2Body> {
		void operator()(b2Body* body) const {
			body->GetWorld()->DestroyBody(body);
		}
	};
}; // namespace std
//要改善
class hit {
public:
	typedef std::pair<int, VECTOR_ref> frames;
	//共通
	struct gun {
		int type = 0;
		frames frame1;
		frames frame2;
		frames frame3;
		float xrad = 0.f, yrad = 0.f;
		std::string name;
		float load_time = 0.f;
		std::vector<std::string> useammo;
		uint16_t rounds = 0;
	};
	struct foot {
		frames frame;
		EffectS gndsmkeffcs;
		float gndsmksize = 1.f;
	};
	//弾薬
	struct Ammos {
		std::string name;
		int16_t type = 0;
		float caliber = 0.f;
		float penetration = 0.f;
		float speed = 0.f;
		uint16_t damage = 0;
	};
	static void set_ammos(std::vector<hit::Ammos>* Ammo_) {
		auto& a = *Ammo_;
		int mdata = FileRead_open("data/ammo/ammo.txt", FALSE);
		while (true) {
			a.resize(a.size() + 1);
			a.back().name		= getparam_str(mdata);
			a.back().type		= uint16_t(getparam_i(mdata)); //ap=0,he=1
			a.back().caliber	= getparam_f(mdata);
			a.back().penetration	= getparam_f(mdata);
			a.back().speed		= getparam_f(mdata);
			a.back().damage		= uint16_t(getparam_u(mdata));
			if (get_str(mdata).find("end") != std::string::npos) {
				break;
			}
		}
		FileRead_close(mdata);
	}
	//戦車
	class Vehcs {
	public:
		//共通
		std::string name;				  /**/
		MV1 obj,col;					  /**/
		VECTOR_ref minpos, maxpos;			  /**/
		std::vector<gun> gunframe;			  /**/
		std::vector<foot> wheelframe;			  /**/
		std::vector<foot> wheelframe_nospring;		  /*誘導輪回転*/
		uint16_t HP = 0;					  /**/
		std::vector<std::pair<size_t, float>> armer_mesh; /*装甲ID*/
		std::vector<size_t> space_mesh;			  /*装甲ID*/
		std::vector<size_t> module_mesh;		  /*装甲ID*/
		int camo_tex = 0;				  /**/
		std::vector<int> camog;				  /**/
		bool isfloat = false;			  /*浮くかどうか*/
		float down_in_water = 0.f;			  /*沈む判定箇所*/
		float max_speed_limit = 0.f;			  /*最高速度(km/h)*/
		float mid_speed_limit = 0.f;			  /*巡行速度(km/h)*/
		float min_speed_limit = 0.f;			  /*失速速度(km/h)*/
		float flont_speed_limit = 0.f;			  /*前進速度(km/h)*/
		float back_speed_limit = 0.f;			  /*後退速度(km/h)*/
		float body_rad_limit = 0.f;			  /*旋回速度(度/秒)*/
		float turret_rad_limit = 0.f;			  /*砲塔駆動速度(度/秒)*/
		frames fps_view;//コックピット
		//専門
		std::array<int, 4> square{ 0 };//車輛の四辺
		std::array<std::vector<frames>, 2> b2upsideframe; /*履帯上*/
		std::array<std::vector<frames>, 2> b2downsideframe; /*履帯上*/
		std::vector<frames> burner;//アフターバーナー
		frames hook;//着艦フック
		//
		void into(const Vehcs& t) {
			this->wheelframe.clear();
			for (auto& p : t.wheelframe) {
				this->wheelframe.resize(this->wheelframe.size() + 1);
				this->wheelframe.back().frame = p.frame;
			}
			this->wheelframe_nospring.clear();
			for (auto& p : t.wheelframe_nospring) {
				this->wheelframe_nospring.resize(this->wheelframe_nospring.size() + 1);
				this->wheelframe_nospring.back().frame = p.frame;
			}
			this->name		 = t.name;
			this->minpos		 = t.minpos;
			this->maxpos		 = t.maxpos;
			this->gunframe		 = t.gunframe;
			this->HP		 = t.HP;
			this->armer_mesh	 = t.armer_mesh;
			this->space_mesh	 = t.space_mesh;
			this->module_mesh	 = t.module_mesh;
			this->camo_tex		 = t.camo_tex;
			this->camog		 = t.camog;
			this->isfloat		 = t.isfloat;
			this->down_in_water	 = t.down_in_water;
			this->max_speed_limit	 = t.max_speed_limit;
			this->mid_speed_limit	 = t.mid_speed_limit;
			this->min_speed_limit	 = t.min_speed_limit; 
			this->flont_speed_limit	 = t.flont_speed_limit;
			this->back_speed_limit	 = t.back_speed_limit;
			this->body_rad_limit	 = t.body_rad_limit;
			this->turret_rad_limit	 = t.turret_rad_limit;
			this->square		 = t.square;
			this->b2upsideframe	 = t.b2upsideframe;
			this->b2downsideframe	 = t.b2downsideframe;
			this->burner		 = t.burner;
			this->hook		 = t.hook;
			this->fps_view = t.fps_view;
		}
		//事前読み込み
		static void set_vehicles_pre(const char* name, std::vector<hit::Vehcs>* veh_, const bool& Async) {
			WIN32_FIND_DATA win32fdt;
			HANDLE hFind;
			hFind = FindFirstFile((std::string(name) + "*").c_str(), &win32fdt);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if ((win32fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (win32fdt.cFileName[0] != '.')) {
						veh_->resize(veh_->size() + 1);
						veh_->back().name = win32fdt.cFileName;
					}
				} while (FindNextFile(hFind, &win32fdt));
			} //else{ return false; }
			FindClose(hFind);
			for (auto& t : *veh_) {
				MV1::Load(std::string(name) + t.name + "/model.mv1", &t.obj, Async);
				MV1::Load(std::string(name) + t.name + "/col.mv1", &t.col, Async);
			}
		}
		//メイン読み込み
		template <size_t N>
		static void set_vehicles(std::array<std::vector<hit::Vehcs>, N>* vehcs) {
			using namespace std::literals;
			//共通
			for (auto& veh : (*vehcs)) {
				for (auto& t : veh) {
					//αテスト
					t.obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
				}
			}
			//固有
			for (auto& t : (*vehcs)[0]) {
				t.down_in_water = 0.f;
				for (int i = 0; i < t.obj.mesh_num(); i++) {
					auto p = t.obj.mesh_maxpos(i).y();
					if (t.down_in_water < p) {
						t.down_in_water = p;
					}
				}
				t.down_in_water /= 2.f;
				for (int i = 0; i < t.obj.frame_num(); i++) {
					std::string p = t.obj.frame_name(i);
					if (p.find("転輪", 0) != std::string::npos) {
						t.wheelframe.resize(t.wheelframe.size() + 1);
						t.wheelframe.back().frame = { i,t.obj.frame(i) };
					}
					else if ((p.find("輪", 0) != std::string::npos) && (p.find("転輪", 0) == std::string::npos)) {
						t.wheelframe_nospring.resize(t.wheelframe_nospring.size() + 1);
						t.wheelframe_nospring.back().frame = { i,t.obj.frame(i) };
					}
					else if (p.find("旋回", 0) != std::string::npos) {
						t.gunframe.resize(t.gunframe.size() + 1);
						auto& b = t.gunframe.back();
						b.frame1 = { i,t.obj.frame(i) };
						auto p2 = t.obj.frame_parent(b.frame1.first);
						if (p2 >= 0) {
							b.frame1.second -= t.obj.frame(int(p2)); //親がいる時引いとく
						}
						if (t.obj.frame_child_num(b.frame1.first) > 0) {
							if (t.obj.frame_name(b.frame1.first + 1).find("仰角", 0) != std::string::npos) {
								b.frame2 = { b.frame1.first + 1,t.obj.frame(b.frame1.first + 1) - t.obj.frame(b.frame1.first) };
								if (t.obj.frame_child_num(b.frame1.first) > 0) {
									b.frame3 = { b.frame2.first + 1,t.obj.frame(b.frame2.first + 1) - t.obj.frame(b.frame2.first) };
								}
								else {
									b.frame3.first = -1;
								}
							}
						}
						else {
							b.frame2.first = -1;
						}
					}
					else if (p.find("min", 0) != std::string::npos) {
						t.minpos = t.obj.frame(i);
					}
					else if (p.find("max", 0) != std::string::npos) {
						t.maxpos = t.obj.frame(i);
					}
					else if (p.find("２D物理", 0) != std::string::npos || p.find("2D物理", 0) != std::string::npos) { //2D物理
						t.b2upsideframe[0].clear();
						t.b2upsideframe[1].clear();
						for (int z = 0; z < t.obj.frame_child_num(i); z++) {
							if (t.obj.frame(i + 1 + z).x() > 0) {
								t.b2upsideframe[0].emplace_back(i + 1 + z, t.obj.frame(i + 1 + z));
							}
							else {
								t.b2upsideframe[1].emplace_back(i + 1 + z, t.obj.frame(i + 1 + z));
							}
						}
						std::sort(t.b2upsideframe[0].begin(), t.b2upsideframe[0].end(), [](const frames& x, const frames& y) { return x.second.z() < y.second.z(); }); //ソート
						std::sort(t.b2upsideframe[1].begin(), t.b2upsideframe[1].end(), [](const frames& x, const frames& y) { return x.second.z() < y.second.z(); }); //ソート
					}
					else if (p.find("履帯設置部", 0) != std::string::npos) { //2D物理
						t.b2downsideframe[0].clear();
						t.b2downsideframe[1].clear();
						for (int z = 0; z < t.obj.frame_child_num(i); z++) {
							if (t.obj.frame(i + 1 + z).x() > 0) {
								t.b2downsideframe[0].emplace_back(i + 1 + z, t.obj.frame(i + 1 + z));
							}
							else {
								t.b2downsideframe[1].emplace_back(i + 1 + z, t.obj.frame(i + 1 + z));
							}
						}
					}
					else if (p.find("視点", 0) != std::string::npos) {
						t.fps_view.first = i;
						t.fps_view.second = t.obj.frame(t.fps_view.first);
					}

				}
				//2	左後部0
				{
					float tmp = 0.f;
					for (auto& f : t.wheelframe) {
						if (f.frame.second.x() >= 0) {
							t.square[0] = f.frame.first;
							tmp = f.frame.second.z();
							break;
						}
					}
					for (auto& f : t.wheelframe) {
						if (t.square[0] != f.frame.first) {
							if (f.frame.second.x() >= 0) {
								if (tmp < f.frame.second.z()) {
									t.square[0] = f.frame.first;
									tmp = f.frame.second.z();
								}
							}
						}
					}
				}
				//10	左前部1
				{
					float tmp = 0.f;
					for (auto& f : t.wheelframe) {
						if (f.frame.second.x() >= 0) {
							t.square[1] = f.frame.first;
							tmp = f.frame.second.z();
							break;
						}
					}
					for (auto& f : t.wheelframe) {
						if (t.square[1] != f.frame.first) {
							if (f.frame.second.x() >= 0) {
								if (tmp > f.frame.second.z()) {
									t.square[1] = f.frame.first;
									tmp = f.frame.second.z();
								}
							}
						}
					}
				}
				//3	右後部2
				{
					float tmp = 0.f;
					for (auto& f : t.wheelframe) {
						if (!(f.frame.second.x() >= 0)) {
							t.square[2] = f.frame.first;
							tmp = f.frame.second.z();
							break;
						}
					}
					for (auto& f : t.wheelframe) {
						if (t.square[2] != f.frame.first) {
							if (!(f.frame.second.x() >= 0)) {
								if (tmp < f.frame.second.z()) {
									t.square[2] = f.frame.first;
									tmp = f.frame.second.z();
								}
							}
						}
					}
				}
				//11	右前部3
				{
					float tmp = 0.f;
					for (auto& f : t.wheelframe) {
						if (!(f.frame.second.x() >= 0)) {
							t.square[3] = f.frame.first;
							tmp = f.frame.second.z();
							break;
						}
					}
					for (auto& f : t.wheelframe) {
						if (t.square[3] != f.frame.first) {
							if (!(f.frame.second.x() >= 0)) {
								if (tmp > f.frame.second.z()) {
									t.square[3] = f.frame.first;
									tmp = f.frame.second.z();
								}
							}
						}
					}
				}
				//装甲
				for (int i = 0; i < t.col.mesh_num(); i++) {
					std::string p = t.col.material_name(i);
					if (p.find("armer", 0) != std::string::npos) {
						t.armer_mesh.emplace_back(i, std::stof(getright(p.c_str())));//装甲
					}
					else if (p.find("space", 0) != std::string::npos) {
						t.space_mesh.emplace_back(i);//空間装甲
					}
					else {
						t.module_mesh.emplace_back(i);//モジュール
					}
				}
				//迷彩
				{
					t.camo_tex = -1;
					for (int i = 0; i < MV1GetTextureNum(t.obj.get()); i++) {
						std::string p = MV1GetTextureName(t.obj.get(), i);
						if (p.find("b.", 0) != std::string::npos || p.find("B.", 0) != std::string::npos) {
							t.camo_tex = i;
							break;
						}
					}
					SetUseTransColor(FALSE);
					WIN32_FIND_DATA win32fdt;
					HANDLE hFind;
					hFind = FindFirstFile(("data/tank/"s + t.name + "/B*.jpg").c_str(), &win32fdt);
					if (hFind != INVALID_HANDLE_VALUE) {
						do {
							if (win32fdt.cFileName[0] != '.') {
								t.camog.resize(t.camog.size() + 1);
								t.camog.back() = MV1LoadTexture(("data/tank/"s + t.name + "/" + win32fdt.cFileName).c_str());
							}
						} while (FindNextFile(hFind, &win32fdt));
					} //else{ return false; }
					FindClose(hFind);
					SetUseTransColor(TRUE);
				}
				//data
				{
					int mdata = FileRead_open(("data/tank/" + t.name + "/data.txt").c_str(), FALSE);
					char mstr[64]; /*tank*/
					t.isfloat = getparam_bool(mdata);
					t.flont_speed_limit = getparam_f(mdata);
					t.back_speed_limit = getparam_f(mdata);
					t.body_rad_limit = getparam_f(mdata);
					t.turret_rad_limit = getparam_f(mdata);
					t.HP = uint16_t(getparam_u(mdata));
					FileRead_gets(mstr, 64, mdata);
					for (auto& g : t.gunframe) {
						g.name = getright(mstr);
						g.load_time = getparam_f(mdata);
						g.rounds = uint16_t(getparam_u(mdata));
						while (true) {
							FileRead_gets(mstr, 64, mdata);
							if (std::string(mstr).find(("useammo" + std::to_string(g.useammo.size()))) == std::string::npos) {
								break;
							}
							g.useammo.emplace_back(getright(mstr));
						}
					}
					FileRead_close(mdata);
				}
			}
			for (auto& t : (*vehcs)[1]) {
				//
				t.down_in_water = 0.f;
				for (int i = 0; i < t.obj.mesh_num(); i++) {
					auto p = t.obj.mesh_minpos(i).y();
					if (t.down_in_water > p) {
						t.down_in_water = p;
					}
				}
				//t.down_in_water /= 2.f;
				//最大最小を取得
				for (int i = 0; i < t.obj.mesh_num(); i++) {
					if (t.maxpos.x() < t.obj.mesh_maxpos(i).x()) {
						t.maxpos.x(t.obj.mesh_maxpos(i).x());
					}
					if (t.maxpos.z() < t.obj.mesh_maxpos(i).z()) {
						t.maxpos.z(t.obj.mesh_maxpos(i).z());
					}
					if (t.minpos.x() > t.obj.mesh_minpos(i).x()) {
						t.minpos.x(t.obj.mesh_minpos(i).x());
					}
					if (t.minpos.z() > t.obj.mesh_minpos(i).z()) {
						t.minpos.z(t.obj.mesh_minpos(i).z());
					}
				}
				//フレーム
				for (int i = 0; i < t.obj.frame_num(); i++) {
					std::string p = t.obj.frame_name(i);
					if (p.find("脚", 0) != std::string::npos) {
						if (p.find("ハッチ", 0) == std::string::npos) {
							t.wheelframe.resize(t.wheelframe.size() + 1);
							t.wheelframe.back().frame.first = i;
							t.wheelframe.back().frame.second = t.obj.frame(t.wheelframe.back().frame.first);

							t.wheelframe_nospring.resize(t.wheelframe_nospring.size() + 1);
							t.wheelframe_nospring.back().frame.first = t.wheelframe.back().frame.first + 1;
							t.wheelframe_nospring.back().frame.second = t.obj.frame(t.wheelframe_nospring.back().frame.first) - t.wheelframe.back().frame.second;
						}
					}
					else if (p.find("旋回", 0) != std::string::npos) {
						t.gunframe.resize(t.gunframe.size() + 1);
						t.gunframe.back().frame1.first = i;
						t.gunframe.back().frame1.second = t.obj.frame(t.gunframe.back().frame1.first);
						auto p2 = t.obj.frame_parent(t.gunframe.back().frame1.first);
						if (p2 >= 0) {
							t.gunframe.back().frame1.second -= t.obj.frame(int(p2)); //親がいる時引いとく
						}
						if (t.obj.frame_child_num(t.gunframe.back().frame1.first) >= 0) {
							if (t.obj.frame_name(t.gunframe.back().frame1.first + 1).find("仰角", 0) != std::string::npos) {
								t.gunframe.back().frame2.first = t.gunframe.back().frame1.first + 1;
								t.gunframe.back().frame2.second = t.obj.frame(t.gunframe.back().frame2.first) - t.obj.frame(t.gunframe.back().frame1.first);
								if (t.obj.frame_child_num(t.gunframe.back().frame1.first) >= 0) {
									t.gunframe.back().frame3.first = t.gunframe.back().frame2.first + 1;
									t.gunframe.back().frame3.second = t.obj.frame(t.gunframe.back().frame3.first) - t.obj.frame(t.gunframe.back().frame2.first);
								}
								else {
									t.gunframe.back().frame3.first = -1;
								}
							}
						}
						else {
							t.gunframe.back().frame2.first = -1;
						}
					}
					else if (p.find("バーナー", 0) != std::string::npos) {
						t.burner.resize(t.burner.size() + 1);
						t.burner.back().first = i;
						t.burner.back().second = t.obj.frame(t.burner.back().first);
					}
					else if (p.find("フック", 0) != std::string::npos) {
						t.hook.first = i;
						t.hook.second = t.obj.frame(t.hook.first);
					}
					else if (p.find("視点", 0) != std::string::npos) {
						t.fps_view.first = i;
						t.fps_view.second = t.obj.frame(t.fps_view.first);
					}
				}
				//メッシュ
				for (int i = 0; i < t.col.mesh_num(); i++) {
					std::string p = t.col.material_name(i);
					if (p.find("armer", 0) != std::string::npos) { //装甲
						t.armer_mesh.resize(t.armer_mesh.size() + 1);
						t.armer_mesh.back().first = i;
						t.armer_mesh.back().second = std::stof(getright(p.c_str())); //装甲値
					}
					else if (p.find("space", 0) != std::string::npos) {		    //空間装甲
						t.space_mesh.resize(t.space_mesh.size() + 1);
						t.space_mesh.back() = i;
					}
					else { //モジュール
						t.module_mesh.resize(t.module_mesh.size() + 1);
						t.module_mesh.back() = i;
					}
				}
				//迷彩
				{
					t.camo_tex = -1;
					for (int i = 0; i < MV1GetTextureNum(t.obj.get()); i++) {
						std::string p = MV1GetTextureName(t.obj.get(), i);
						if (p.find("b.", 0) != std::string::npos || p.find("B.", 0) != std::string::npos) {
							t.camo_tex = i;
							break;
						}
					}

					//t.camo.resize(t.camo.size() + 1);
					{
						SetUseTransColor(FALSE);
						WIN32_FIND_DATA win32fdt;
						HANDLE hFind;
						hFind = FindFirstFile(("data/plane/"s + t.name + "/B*.jpg").c_str(), &win32fdt);
						if (hFind != INVALID_HANDLE_VALUE) {
							do {
								if (win32fdt.cFileName[0] != '.') {
									t.camog.resize(t.camog.size() + 1);
									t.camog.back() = LoadGraph(("data/plane/"s + t.name + "/" + win32fdt.cFileName).c_str());
								}
							} while (FindNextFile(hFind, &win32fdt));
						} //else{ return false; }
						FindClose(hFind);
						SetUseTransColor(TRUE);
					}
				}
				//データ取得
				{
					int mdata = FileRead_open(("data/plane/" + t.name + "/data.txt").c_str(), FALSE);
					char mstr[64]; /*tank*/
					t.isfloat = getparam_bool(mdata);
					t.max_speed_limit = getparam_f(mdata) / 3.6f;
					t.mid_speed_limit = getparam_f(mdata) / 3.6f;
					t.min_speed_limit = getparam_f(mdata) / 3.6f;
					t.body_rad_limit = getparam_f(mdata);
					t.HP = uint16_t(getparam_u(mdata));
					FileRead_gets(mstr, 64, mdata);
					for (auto& g : t.gunframe) {
						g.name = getright(mstr);
						g.load_time = getparam_f(mdata);
						g.rounds = uint16_t(getparam_u(mdata));
						while (true) {
							FileRead_gets(mstr, 64, mdata);
							if (std::string(mstr).find(("useammo" + std::to_string(g.useammo.size()))) == std::string::npos) {
								break;
							}
							g.useammo.resize(g.useammo.size() + 1);
							g.useammo.back() = getright(mstr);
						}
					}
					FileRead_close(mdata);
				}
			}
		}

	};
	//player
	struct ammos {
		bool hit{ false };
		bool flug{ false };
		float cnt = 0.f;
		unsigned int color = 0;
		Ammos spec;
		float yadd = 0.f;
		VECTOR_ref pos, repos, vec;
	};
	struct b2Pats {
		std::unique_ptr<b2Body> body;    /**/
		b2Fixture* playerfix{ nullptr }; /**/
		VECTOR_ref pos;/*仮*/
	};
	struct wallPats {
		b2Pats b2;
		std::array<VECTOR_ref, 2> pos;
	};
	struct treePats {
		MV1 obj, obj_far;
		MATRIX_ref mat;
		VECTOR_ref pos;
		bool fall_flag = false;
		VECTOR_ref fall_vec;
		float fall_rad = 0.f;
	};
	typedef std::pair<int, float> animes;
	typedef std::pair<bool, uint8_t> switchs;
	typedef std::pair<size_t, float> pair_hit;
	//
	class vehicles {
	public:
		Vehcs use_veh;							      /*固有値*/
		MV1 obj;							      /**/
		MV1 col;							      /**/
		bool hit_check = false;						      //当たり判定を取るかチェック
		size_t use_id = 0;						      //使用する車両(機材)
		uint16_t HP = 0;						      /*体力*/
		VECTOR_ref pos;							      //車体座標
		MATRIX_ref mat;							      //車体回転行列
		VECTOR_ref add;							      //車体加速度
		struct Guns {							      /**/
			size_t usebullet{};					      /*使用弾*/
			std::array<ammos, 64> bullet;				      /*確保する弾*/
			float loadcnt{ 0 };					      /*装てんカウンター*/
			float fired{ 0.f };					      /*駐退数*/
			int16_t rounds{ 0 };					      /*弾数*/
			gun gun_info;						      /**/
			std::vector<Ammos> Spec;				      /**/
		};								      /**/
		std::vector<Guns> Gun_;						      /**/
		float speed = 0.f, speed_add = 0.f, speed_sub = 0.f;		      /**/
		float xrad = 0.f, xradadd = 0.f, xradadd_left = 0.f, xradadd_right = 0.f; /**/
		float yradadd = 0.f, yradadd_left = 0.f, yradadd_right = 0.f;	    /**/
		float zrad = 0.f, zradadd = 0.f, zradadd_left = 0.f, zradadd_right = 0.f; /**/
		std::vector<MV1_COLL_RESULT_POLY> hitres;			      /*確保*/
		std::vector<int16_t> HP_m;					      /*ライフ*/
		struct Hit {							      /**/
			bool flug{ false };					      /*弾痕フラグ*/
			int use{ 0 };						      /*使用フレーム*/
			MV1 pic;						      /*弾痕モデル*/
			VECTOR_ref scale, pos, z_vec, y_vec;			      /*座標*/
		};								      /**/
		std::array<Hit, 24> hit;					      /*弾痕*/
		size_t camo_sel = 0;						      /**/
		float wheel_Left = 0.f, wheel_Right = 0.f;			      //転輪回転
		std::vector<pair_hit> hitssort;					      /*フレームに当たった順番*/
	public:
		void reset() {
			//*
			this->obj.Dispose();
			this->col.Dispose();
			this->hit_check = false;
			this->HP = 0;
			this->speed_add = 0.f;
			this->speed_sub = 0.f;
			this->speed = 0.f;
			this->add = VGet(0.f, 0.f, 0.f);
			this->hitres.clear();
			this->HP_m.clear();
			this->wheel_Left = 0.f;
			this->wheel_Right = 0.f;
			this->xrad = 0.f;
			this->xradadd = 0.f;
			this->xradadd_left = 0.f;
			this->xradadd_right = 0.f;
			this->yradadd = 0.f;
			this->yradadd_left = 0.f;
			this->yradadd_right = 0.f;
			this->zrad = 0.f;
			this->zradadd = 0.f;
			this->zradadd_left = 0.f;
			this->zradadd_right = 0.f;
			for (auto& h : this->hit) {
				h.flug = false;
				h.use = 0;
				h.pic.Dispose();
				h.scale = VGet(1.f, 1.f, 1.f);
				h.pos = VGet(0, 0, 0);
				h.y_vec = VGet(0, 1.f, 0);
				h.z_vec = VGet(0, 0, 1.f);
			}

			//this->use_veh;
			for (auto& g : this->Gun_) {
				g.fired = 0.f;
				g.loadcnt = 0.f;
				g.rounds = 0;
				g.usebullet = 0;
				for (auto& a : g.bullet) {
				}
				//g.gun_info;
				g.Spec.clear();
			}
			this->Gun_.clear();

			for (auto& h : this->hitssort) {
				h.first = 0;
				h.second = 0.f;
			}
			this->hitssort.clear();					      //フレームに当たった順番
			//*/
		}
	};
	struct Chara {
		//====================================================
		size_t id = 0;			     /**/
		std::array<EffectS, efs_user> effcs; /*effect*/
		struct ef_guns {
			EffectS first;
			ammos* second = nullptr;
			int cnt = -1;
		};
		std::array<ef_guns, 8> effcs_missile; /*effect*/
		std::array<ef_guns, 12> effcs_gun;    /*effect*/
		size_t missile_effcnt = 0;
		size_t gun_effcnt = 0;

		//操作関連//==================================================
		uint8_t mode = 0;		      //フライト(形態)モード
		std::array<bool, 15> key{ false };    //キー
		float view_xrad = 0.f, view_yrad = 0.f; //砲塔操作用ベクトル
		//戦車//==================================================
		b2Pats b2mine;			       /*box2d*/
		float spd = 0.f;		       /*box2d*/
		int hitbuf = 0;		       /*使用弾痕*/
		float xradp_shot = 0.f, xrad_shot = 0.f; //射撃反動x
		float zradp_shot = 0.f, zrad_shot = 0.f; //射撃反動z
		VECTOR_ref wheel_normal;	       /*ノーマル*/
		struct FootWorld {
			b2World* world;		       /*足world*/
			b2RevoluteJointDef f_jointDef; /*ジョイント*/
			std::vector<b2Pats> Foot;      /**/
			std::vector<b2Pats> Wheel;     /**/
			std::vector<b2Pats> Yudo;      /**/
		};
		std::array<FootWorld, 2> foot; /*足*/
		//飛行機//==================================================
		animes p_anime_geardown;		    //車輪アニメーション
		switchs changegear = { false, uint8_t(0) }; //ギアアップスイッチ
		switchs landing = { false, uint8_t(0) }; //着艦フックスイッチ
		float p_landing_per = 0.f;		    //着艦フック
		std::array<animes, 6> p_animes_rudder;      //ラダーアニメーション
		struct burners {			    /**/
			frames frame;			    /**/
			MV1 effectobj;			    /**/
		};					    /**/
		std::vector<burners> p_burner;		    //バーナー
		//共通項//==================================================
		std::array<vehicles, veh_all> vehicle; //0=戦車,1=飛行機
	};
	static bool get_reco(Chara& play, std::vector<Chara>& tgts, ammos& c, const uint8_t& type) {
		if (c.flug) {
			bool is_hit;
			std::optional<size_t> hitnear;
			for (auto& t : tgts) {
				//自分自身は省く
				if (play.id == t.id) {
					continue;
				}
				//とりあえず当たったかどうか探す
				is_hit = false;
				{
					auto& veh = t.vehicle[type];
					//モジュール
					for (auto& m : veh.use_veh.module_mesh) {
						veh.hitres[m] = veh.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos) * (0.1f)), -1, int(m));
						if (veh.hitres[m].HitFlag) {
							veh.hitssort[m] = pair_hit(m, (c.repos - veh.hitres[m].HitPosition).size());
							is_hit = true;
						}
						else {
							veh.hitssort[m] = pair_hit(m, (std::numeric_limits<float>::max)());
						}
					}
					//空間装甲
					for (auto& m : veh.use_veh.space_mesh) {
						veh.hitres[m] = veh.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos) * (0.1f)), -1, int(m));
						if (veh.hitres[m].HitFlag) {
							veh.hitssort[m] = pair_hit(m, (c.repos - veh.hitres[m].HitPosition).size());
							is_hit = true;
						}
						else {
							veh.hitssort[m] = pair_hit(m, (std::numeric_limits<float>::max)());
						}
					}
					//装甲
					for (auto& m : veh.use_veh.armer_mesh) {
						veh.hitres[m.first] = veh.col.CollCheck_Line(c.repos, c.pos, -1, int(m.first));
						if (veh.hitres[m.first].HitFlag) {
							veh.hitssort[m.first] = pair_hit(m.first, (c.repos - veh.hitres[m.first].HitPosition).size());
							is_hit = true;
						}
						else {
							veh.hitssort[m.first] = pair_hit(m.first, (std::numeric_limits<float>::max)());
						}
					}
					//当たってない
					if (!is_hit) {
						continue;
					}
					//当たり判定を近い順にソート
					std::sort(veh.hitssort.begin(), veh.hitssort.end(), [](const pair_hit& x, const pair_hit& y) { return x.second < y.second; });
					//ダメージ面に届くまで判定
					for (auto& tt : veh.hitssort) {
						//装甲面に当たらなかったならスルー
						if (tt.second == (std::numeric_limits<float>::max)()) {
							break;
						}
						switch (c.spec.type) {
						case 0: //AP
							//装甲面に当たったのでhitnearに代入して終了
							for (auto& a : veh.use_veh.armer_mesh) {
								if (tt.first == a.first) {
									hitnear = tt.first;
									//ダメージ面に当たった時に装甲値に勝てるかどうか
									{
										VECTOR_ref normal = veh.hitres[hitnear.value()].Normal;
										VECTOR_ref position = veh.hitres[hitnear.value()].HitPosition;
										if (c.spec.penetration > a.second * (1.0f / std::abs(c.vec.Norm().dot(normal)))) {
											//貫通
											//t.id;
											veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //

											veh.HP = std::max<int16_t>(veh.HP - c.spec.damage, 0); //
											//撃破時エフェクト
											if (veh.HP == 0) {
												set_effect(&t.effcs[ef_bomb], veh.obj.frame(veh.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
											}
											//弾処理
											c.flug = false;
											c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
											c.vec = c.vec.Norm();
											c.pos = c.vec * (0.1f) + position;
											//弾痕
											veh.hit[t.hitbuf].use = 0;
										}
										else {
											//はじく
											//弾処理
											c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
											c.vec = c.vec.Norm();
											c.pos = c.vec * (0.1f) + position;
											c.spec.penetration /= 2.0f;
											//弾痕
											veh.hit[t.hitbuf].use = 1;
										}
										if (c.spec.caliber >= 0.020f) {
											set_effect(&play.effcs[ef_reco], c.pos, normal);
										}
										else {
											set_effect(&play.effcs[ef_reco2], c.pos, normal);
										}

										//弾痕のセット
										{
											float asize = c.spec.caliber * 100.f;
											veh.hit[t.hitbuf].scale = VGet(asize / std::abs(c.vec.Norm().dot(normal)), asize, asize);
											veh.hit[t.hitbuf].pos = MATRIX_ref::Vtrans(position - veh.pos, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
											veh.hit[t.hitbuf].y_vec = MATRIX_ref::Vtrans(normal, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
											veh.hit[t.hitbuf].z_vec = MATRIX_ref::Vtrans(normal.cross(c.vec), veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
											veh.hit[t.hitbuf].flug = true;
											++t.hitbuf %= veh.hit.size();
										}
									}
									break;
								}
							}
							if (hitnear.has_value()) {
								break;
							}
							//空間装甲、モジュールに当たったのでモジュールに30ダメ、貫徹力を1/2に
							for (auto& a : veh.use_veh.space_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
									c.spec.penetration /= 2.0f;
								}
							}
							for (auto& a : veh.use_veh.module_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
									c.spec.penetration /= 2.0f;
								}
							}

							break;
						case 1: //HE
							//装甲面に当たったのでhitnearに代入して終了
							for (auto& a : veh.use_veh.armer_mesh) {
								if (tt.first == a.first) {
									hitnear = tt.first;
									//ダメージ面に当たった時に装甲値に勝てるかどうか
									{
										VECTOR_ref normal = veh.hitres[hitnear.value()].Normal;
										VECTOR_ref position = veh.hitres[hitnear.value()].HitPosition;
										if (c.spec.penetration > a.second * (1.0f / std::abs(c.vec.Norm().dot(normal)))) {
											//貫通
											veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //

											veh.HP = std::max<int16_t>(veh.HP - c.spec.damage, 0); //
											//撃破時エフェクト
											if (veh.HP == 0) {
												set_effect(&t.effcs[ef_bomb], veh.obj.frame(veh.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
											}
											//弾処理
											c.flug = false;
											c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
											c.vec = c.vec.Norm();
											c.pos = c.vec * (0.1f) + position;
											//弾痕
											veh.hit[t.hitbuf].use = 0;
										}
										else {
											//爆発する
											//弾処理
											c.flug = false;
											c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
											c.vec = c.vec.Norm();
											c.pos = c.vec * (0.1f) + position;
											//弾痕
											veh.hit[t.hitbuf].use = 1;
										}
										if (c.spec.caliber >= 0.020f) {
											set_effect(&play.effcs[ef_reco], c.pos, normal);
										}
										else {
											set_effect(&play.effcs[ef_reco2], c.pos, normal);
										}

										//弾痕のセット
										{
											float asize = c.spec.caliber * 100.f;
											veh.hit[t.hitbuf].scale = VGet(asize / std::abs(c.vec.Norm().dot(normal)), asize, asize);
											veh.hit[t.hitbuf].pos = MATRIX_ref::Vtrans(position - veh.pos, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
											veh.hit[t.hitbuf].y_vec = MATRIX_ref::Vtrans(normal, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
											veh.hit[t.hitbuf].z_vec = MATRIX_ref::Vtrans(normal.cross(c.vec), veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
											veh.hit[t.hitbuf].flug = true;
											++t.hitbuf %= veh.hit.size();
										}
									}
									break;
								}
							}
							if (hitnear.has_value()) {
								break;
							}
							//空間装甲、モジュールに当たったのでモジュールに30ダメ、弾なし
							for (auto& a : veh.use_veh.space_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
									//爆発する
									c.flug = false;
								}
							}
							for (auto& a : veh.use_veh.module_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
									//爆発する
									c.flug = false;
								}
							}
							break;
						case 2: //ミサイル
							//装甲面に当たったのでhitnearに代入して終了
							for (auto& a : veh.use_veh.armer_mesh) {
								if (tt.first == a.first) {
									hitnear = tt.first;
									//ダメージ面に当たった時に装甲値に勝てるかどうか
									{
										VECTOR_ref normal = veh.hitres[hitnear.value()].Normal;
										VECTOR_ref position = veh.hitres[hitnear.value()].HitPosition;
										if (c.spec.penetration > a.second * (1.0f / std::abs(c.vec.Norm().dot(normal)))) {
											//貫通
											veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //

											veh.HP = std::max<int16_t>(veh.HP - c.spec.damage, 0); //
											//撃破時エフェクト
											if (veh.HP == 0) {
												set_effect(&t.effcs[ef_bomb], veh.obj.frame(veh.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
											}
											//弾処理
											c.flug = false;
											c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
											c.vec = c.vec.Norm();
											c.pos = c.vec * (0.1f) + position;
											//弾痕
											veh.hit[t.hitbuf].use = 0;
										}
										else {
											//爆発する
											//弾処理
											c.flug = false;
											c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
											c.vec = c.vec.Norm();
											c.pos = c.vec * (0.1f) + position;
											//弾痕
											veh.hit[t.hitbuf].use = 1;
										}
										if (c.spec.caliber >= 0.020f) {
											set_effect(&play.effcs[ef_reco], c.pos, normal);
										}
										else {
											set_effect(&play.effcs[ef_reco2], c.pos, normal);
										}

										//弾痕のセット
										{
											float asize = c.spec.caliber * 100.f;
											veh.hit[t.hitbuf].scale = VGet(asize / std::abs(c.vec.Norm().dot(normal)), asize, asize);
											veh.hit[t.hitbuf].pos = MATRIX_ref::Vtrans(position - veh.pos, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
											veh.hit[t.hitbuf].y_vec = MATRIX_ref::Vtrans(normal, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
											veh.hit[t.hitbuf].z_vec = MATRIX_ref::Vtrans(normal.cross(c.vec), veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
											veh.hit[t.hitbuf].flug = true;
											++t.hitbuf %= veh.hit.size();
										}
									}
									break;
								}
							}
							if (hitnear.has_value()) {
								break;
							}
							//空間装甲、モジュールに当たったのでモジュールに30ダメ、弾なし
							for (auto& a : veh.use_veh.space_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
									//爆発する
									c.flug = false;
								}
							}
							for (auto& a : veh.use_veh.module_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
									}
									veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
									//爆発する
									c.flug = false;
								}
							}
							break;
						default:
							break;
						}
					}
				}
				if (hitnear.has_value())
					break;
			}
			return (hitnear.has_value());
		}
		return false;
	}
};
//
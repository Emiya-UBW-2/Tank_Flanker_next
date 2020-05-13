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
constexpr auto veh_all = 3;//車種
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

typedef std::pair<int, VECTOR_ref> frames;
typedef std::pair<bool, uint8_t> switchs;

//要改善
class Mainclass {
private:
	struct gun_frame {
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
	struct foot_frame {
		frames frame;
		EffectS gndsmkeffcs;
		float gndsmksize = 1.f;
	};
	struct Hit {		      /**/
		bool flug{ false };   /*弾痕フラグ*/
		int use{ 0 };	      /*使用フレーム*/
		MV1 pic;	      /*弾痕モデル*/
		VECTOR_ref pos;	      /*座標*/
		MATRIX_ref mat;	      /**/
	};								      /**/
	struct b2Pats {
		std::unique_ptr<b2Body> body;    /**/
		b2Fixture* playerfix{ nullptr }; /**/
		VECTOR_ref pos;/*仮*/
	};
	struct FootWorld {
		b2World* world=nullptr;			     /*足world*/
		b2RevoluteJointDef f_jointDef;	     /*ジョイント*/
		std::vector<b2Pats> Foot,Wheel,Yudo; /**/
	};
public:
	//弾薬
	class Ammos {
	public:
		std::string name_a;
		int16_t type_a = 0;
		float caliber_a = 0.f;
		float pene_a = 0.f;
		float speed_a = 0.f;
		uint16_t damage_a = 0;

		static void set_ammos(std::vector<Ammos>* Ammo_) {
			auto& a = *Ammo_;
			int mdata = FileRead_open("data/ammo/ammo.txt", FALSE);
			while (true) {
				a.resize(a.size() + 1);
				a.back().name_a = getparam_str(mdata);
				a.back().type_a = uint16_t(getparam_i(mdata)); //ap=0,he=1
				a.back().caliber_a = getparam_f(mdata);
				a.back().pene_a = getparam_f(mdata);
				a.back().speed_a = getparam_f(mdata);
				a.back().damage_a = uint16_t(getparam_u(mdata));
				if (get_str(mdata).find("end") != std::string::npos) {
					break;
				}
			}
			FileRead_close(mdata);
		}
	};
	//車輛
	class Vehcs {
	public:
		//共通
		std::string name;				  /**/
		MV1 obj, col;					  /**/
		VECTOR_ref minpos, maxpos;			  /**/
		std::vector<gun_frame> gunframe;			  /**/
		std::vector<foot_frame> wheelframe;			  /**/
		std::vector<foot_frame> wheelframe_nospring;		  /*誘導輪回転*/
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
		GraphHandle ui_pic;//シルエット
		int pic_x, pic_y;//サイズ
		//専門
		std::array<int, 4> square{ 0 };//車輛の四辺
		std::array<std::vector<frames>, 2> b2upsideframe; /*履帯上*/
		std::array<std::vector<frames>, 2> b2downsideframe; /*履帯上*/
		std::vector<frames> burner;//アフターバーナー
		frames hook;//着艦フック

		std::vector<frames> wire;
		std::vector<frames> catapult;
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
			this->name = t.name;
			this->minpos = t.minpos;
			this->maxpos = t.maxpos;
			this->gunframe = t.gunframe;
			this->HP = t.HP;
			this->armer_mesh = t.armer_mesh;
			this->space_mesh = t.space_mesh;
			this->module_mesh = t.module_mesh;
			this->camo_tex = t.camo_tex;
			this->camog = t.camog;
			this->isfloat = t.isfloat;
			this->down_in_water = t.down_in_water;
			this->max_speed_limit = t.max_speed_limit;
			this->mid_speed_limit = t.mid_speed_limit;
			this->min_speed_limit = t.min_speed_limit;
			this->flont_speed_limit = t.flont_speed_limit;
			this->back_speed_limit = t.back_speed_limit;
			this->body_rad_limit = t.body_rad_limit;
			this->turret_rad_limit = t.turret_rad_limit;
			this->square = t.square;
			this->b2upsideframe = t.b2upsideframe;
			this->b2downsideframe = t.b2downsideframe;
			this->burner = t.burner;
			this->hook = t.hook;
			this->fps_view = t.fps_view;

			this->wire = t.wire;
			this->catapult = t.catapult;

			this->ui_pic = t.ui_pic.Duplicate();
			this->pic_x = t.pic_x;
			this->pic_y = t.pic_y;
		}
		//事前読み込み
		static void set_vehicles_pre(const char* name, std::vector<Mainclass::Vehcs>* veh_, const bool& Async) {
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
				t.ui_pic = GraphHandle::Load(std::string(name) + t.name + "/pic.png");
			}
		}
		//メイン読み込み
		template <size_t N>
		static void set_vehicles(std::array<std::vector<Mainclass::Vehcs>, N>* vehcs) {
			using namespace std::literals;
			//共通
			for (auto& veh : (*vehcs)) {
				for (auto& t : veh) {
					//αテスト
					t.obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
					//
					GetGraphSize(t.ui_pic.get(), &t.pic_x, &t.pic_y);
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
			for (auto& t : (*vehcs)[2]) {

				for (int i = 0; i < t.obj.frame_num(); i++) {
					std::string p = t.obj.frame_name(i);
					if (p.find("ﾜｲﾔｰ", 0) != std::string::npos) {
						t.wire.resize(t.wire.size() + 1);
						t.wire.back().first = i;
						t.wire.back().second = t.obj.frame(t.wire.back().first);
					}
					else if (p.find("ｶﾀﾊﾟﾙﾄ", 0) != std::string::npos) {
						t.catapult.resize(t.catapult.size() + 1);
						t.catapult.back().first = i;
						t.catapult.back().second = t.obj.frame(t.catapult.back().first + 2) - t.obj.frame(t.catapult.back().first);
					}
				}
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


				VECTOR_ref size;
				for (int i = 0; i < t.col.mesh_num(); i++) {
					VECTOR_ref sizetmp = t.col.mesh_maxpos(i) - t.col.mesh_minpos(i);
					if (size.x() < sizetmp.x()) {
						size.x(sizetmp.x());
					}
					if (size.y() < sizetmp.y()) {
						size.y(sizetmp.y());
					}
					if (size.z() < sizetmp.z()) {
						size.z(sizetmp.z());
					}
				}

				/*
				for (int i = 0; i < t.col.mesh_num(); i++) {
					t.col.SetupCollInfo(int(size.x() / 5.f), int(size.y() / 5.f), int(size.z() / 5.f), 0, i);
				}
				*/
				//t.col.SetupCollInfo(int(size.x() / 5.f), int(size.y() / 5.f), int(size.z() / 5.f), 0, 0);
				//
				//t.obj.SetFrameLocalMatrix(t.catapult[0].first + 2, MATRIX_ref::RotX(deg2rad(-75)) * MATRIX_ref::Mtrans(t.catapult[0].second));

								//データ取得
				{
					int mdata = FileRead_open(("data/carrier/" + t.name + "/data.txt").c_str(), FALSE);
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
private:
	struct ammos {
		bool hit{ false };
		bool flug{ false };
		float cnt = 0.f;
		unsigned int color = 0;
		Mainclass::Ammos spec;
		float yadd = 0.f;
		VECTOR_ref pos, repos, vec;
	};
	struct Guns {							      /**/
		size_t usebullet{};					      /*使用弾*/
		std::array<ammos, 64> bullet;				      /*確保する弾*/
		float loadcnt{ 0 };					      /*装てんカウンター*/
		float fired{ 0.f };					      /*駐退数*/
		int16_t rounds{ 0 };					      /*弾数*/
		gun_frame gun_info;						      /**/
		std::vector<Mainclass::Ammos> Spec;				      /**/
	};								      /**/
	struct ef_guns {
		EffectS first;
		ammos* second = nullptr;
		int cnt = -1;
	};
	typedef std::pair<size_t, float> pair_hit;
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
		std::vector<Guns> Gun_;						      /**/
		float speed = 0.f, speed_add = 0.f, speed_sub = 0.f;		      /**/
		float xrad = 0.f, xradadd = 0.f, xradadd_left = 0.f, xradadd_right = 0.f; /**/
		float yrad = 0.f, yradadd = 0.f, yradadd_left = 0.f, yradadd_right = 0.f;	    /**/
		float zrad = 0.f, zradadd = 0.f, zradadd_left = 0.f, zradadd_right = 0.f; /**/
		std::vector<MV1_COLL_RESULT_POLY> hitres;			      /*確保*/
		std::vector<int16_t> HP_m;					      /*ライフ*/
		std::array<Hit, 24> hit_obj;					      /*弾痕*/
		size_t camo_sel = 0;						      /**/
		float wheel_Left = 0.f, wheel_Right = 0.f;			      //転輪回転
		float wheel_Leftadd = 0.f, wheel_Rightadd = 0.f;		      //転輪回転
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
			this->wheel_Leftadd = 0.f;
			this->wheel_Rightadd = 0.f;
			this->xrad = 0.f;
			this->xradadd = 0.f;
			this->xradadd_left = 0.f;
			this->xradadd_right = 0.f;
			this->yrad = 0.f;
			this->yradadd = 0.f;
			this->yradadd_left = 0.f;
			this->yradadd_right = 0.f;
			this->zrad = 0.f;
			this->zradadd = 0.f;
			this->zradadd_left = 0.f;
			this->zradadd_right = 0.f;
			for (auto& h : this->hit_obj) {
				h.flug = false;
				h.use = 0;
				h.pic.Dispose();
				h.pos = VGet(0, 0, 0);
				h.mat = MGetIdent();
			}
			//this->use_veh;
			for (auto& g : this->Gun_) {
				g.fired = 0.f;
				g.loadcnt = 0.f;
				g.rounds = 0;
				g.usebullet = 0;
				//for (auto& a : g.bullet) {
				//}
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
public:
	//マップ
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
	//player
	typedef std::pair<int, float> p_animes;
	class Chara {
	public:
		//====================================================
		size_t id = 0;			     /**/
		std::array<EffectS, efs_user> effcs; /*effect*/
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
		std::array<FootWorld, 2> foot; /*足*/
		//飛行機//==================================================
		p_animes p_anime_geardown;		    //車輪アニメーション
		switchs changegear = { false, uint8_t(0) }; //ギアアップスイッチ
		switchs landing = { false, uint8_t(0) }; //着艦フックスイッチ
		float p_landing_per = 0.f;		    //着艦フック
		std::array<p_animes, 6> p_animes_rudder;      //ラダーアニメーション
		std::vector<frames> p_burner;		    //バーナー
		//共通項//==================================================
		std::array<vehicles, veh_all> vehicle; //0=戦車,1=飛行機

		//セット
		template <size_t N>
		void set_human(const std::array<std::vector<Mainclass::Vehcs>, N>& vehcs, const std::vector<Ammos>& Ammo_, const MV1& hit_pic, std::unique_ptr<b2World>& world) {
			auto& c = *this;
			{
				std::fill(c.key.begin(), c.key.end(), false); //操作
				fill_id(c.effcs);			      //エフェクト
				//共通
				{
					int i = 0;
					for (auto& veh : c.vehicle) {
						veh.reset();
						veh.use_id = std::min<size_t>(veh.use_id, vehcs[i].size() - 1);
						veh.use_veh.into(vehcs[i][veh.use_id]);
						veh.obj = vehcs[i][veh.use_id].obj.Duplicate();
						veh.col = vehcs[i][veh.use_id].col.Duplicate();

						i++;
						//コリジョン
						for (int j = 0; j < veh.col.mesh_num(); j++) {
							veh.col.SetupCollInfo(8, 8, 8, -1, j);
						}
						veh.hitres.resize(veh.col.mesh_num());   //モジュールごとの当たり判定結果を確保
						veh.hitssort.resize(veh.col.mesh_num()); //モジュールごとの当たり判定順序を確保
						//弾痕
						for (auto& h : veh.hit_obj) {
							h.flug = false;
							h.pic = hit_pic.Duplicate();
							h.use = 0;
							h.mat = MGetIdent();
							h.pos = VGet(0.f, 0.f, 0.f);
						}
						for (int j = 0; j < veh.obj.material_num(); ++j) {
							MV1SetMaterialSpcColor(veh.obj.get(), j, GetColorF(0.85f, 0.82f, 0.78f, 0.1f));
							MV1SetMaterialSpcPower(veh.obj.get(), j, 50.0f);
						}
						//砲
						{
							veh.Gun_.resize(veh.use_veh.gunframe.size());
							for (int j = 0; j < veh.Gun_.size(); j++) {
								auto& g = veh.Gun_[j];
								g.gun_info = veh.use_veh.gunframe[j];
								g.rounds = g.gun_info.rounds;
								//使用砲弾
								g.Spec.resize(g.Spec.size() + 1);
								for (auto& pa : Ammo_) {
									if (pa.name_a.find(g.gun_info.useammo[0]) != std::string::npos) {
										g.Spec.back() = pa;
										break;
									}
								}
								for (auto& p : g.bullet) {
									p.color = GetColor(255, 255, 172);
									p.spec = g.Spec[0];
								}
							}
						}
						//ヒットポイント
						veh.HP = veh.use_veh.HP;
						//モジュール耐久
						veh.HP_m.resize(veh.col.mesh_num());
						for (auto& h : veh.HP_m) {
							h = 100;
						}
						//迷彩
						if (veh.use_veh.camog.size() > 0) {
							veh.camo_sel %= veh.use_veh.camog.size();
							//GraphBlend(MV1GetTextureGraphHandle(veh.obj.get(), veh.use_veh.camo_tex), veh.use_veh.camog[veh.camo_sel], 255, DX_GRAPH_BLEND_NORMAL);
							MV1SetTextureGraphHandle(veh.obj.get(), veh.use_veh.camo_tex, veh.use_veh.camog[veh.camo_sel], FALSE);
						}
					}
				}
				//戦車
				{
					auto& veh = c.vehicle[0];
					//戦車物理set
					b2PolygonShape dynamicBox; /*ダイナミックボディに別のボックスシェイプを定義します。*/
					dynamicBox.SetAsBox(
						(veh.use_veh.maxpos.x() - veh.use_veh.minpos.x()) / 2, (veh.use_veh.maxpos.z() - veh.use_veh.minpos.z()) / 2, b2Vec2((veh.use_veh.minpos.x() + veh.use_veh.maxpos.x()) / 2, (veh.use_veh.minpos.z() + veh.use_veh.maxpos.z()) / 2), 0.f);
					b2FixtureDef fixtureDef;					 /*動的ボディフィクスチャを定義します*/
					fixtureDef.shape = &dynamicBox;				 /**/
					fixtureDef.density = 1.0f;					 /*ボックス密度をゼロ以外に設定すると、動的になる*/
					fixtureDef.friction = 0.3f;					 /*デフォルトの摩擦をオーバーライド*/
					b2BodyDef bodyDef;						 /*ダイナミックボディを定義します。その位置を設定し、ボディファクトリを呼び出す*/
					bodyDef.type = b2_dynamicBody;					 /**/
					bodyDef.position.Set(veh.pos.x(), veh.pos.z());			 /**/
					bodyDef.angle = atan2f(-veh.mat.zvec().x(), -veh.mat.zvec().z()); /**/
					c.b2mine.body.reset(world->CreateBody(&bodyDef));		 /**/
					c.b2mine.playerfix = c.b2mine.body->CreateFixture(&fixtureDef);   /*シェイプをボディに追加*/
					c.wheel_normal = VGet(0.f, 1.f, 0.f);			 //転輪の法線ズ

					/* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
					for (size_t k = 0; k < 2; ++k) {
						auto& f = c.foot[k];
						f.world = new b2World(b2Vec2(0.0f, 0.0f));
						{
							b2Body* ground = NULL;
							{
								b2BodyDef bd;
								ground = f.world->CreateBody(&bd);
								b2EdgeShape shape;
								shape.Set(b2Vec2(-40.0f, -10.0f), b2Vec2(40.0f, -10.0f));
								ground->CreateFixture(&shape, 0.0f);
							}
							b2Body* prevBody = ground;

							//履帯
							f.Foot.clear();
							{
								VECTOR_ref vects;
								for (auto& w : veh.use_veh.b2upsideframe[k]) {
									vects = w.second;
									if (vects.x() * ((k == 0) ? 1 : -1) > 0) {
										f.Foot.resize(f.Foot.size() + 1);
										b2PolygonShape f_dynamicBox; /*ダイナミックボディに別のボックスシェイプを定義します。*/
										f_dynamicBox.SetAsBox(0.2f, 0.05f);
										b2FixtureDef f_fixtureDef;
										f_fixtureDef.shape = &f_dynamicBox;
										f_fixtureDef.density = 20.0f;
										f_fixtureDef.friction = 0.2f;
										b2BodyDef f_bodyDef;
										f_bodyDef.type = b2_dynamicBody;
										f_bodyDef.position.Set(vects.z(), vects.y());
										f.Foot.back().body.reset(f.world->CreateBody(&f_bodyDef));
										f.Foot.back().playerfix = f.Foot.back().body->CreateFixture(&f_fixtureDef); // シェイプをボディに追加します。
										f.f_jointDef.Initialize(prevBody, f.Foot.back().body.get(), b2Vec2(vects.z(), vects.y()));
										f.world->CreateJoint(&f.f_jointDef);
										prevBody = f.Foot.back().body.get();
									}
								}
								if (f.Foot.size() != 0) {
									f.f_jointDef.Initialize(prevBody, ground, b2Vec2(vects.z(), vects.y()));
									f.world->CreateJoint(&f.f_jointDef);
								}
							}

							//転輪(動く)
							f.Wheel.clear();
							f.Yudo.clear();
							if (f.Foot.size() != 0) {
								for (auto& w : veh.use_veh.wheelframe) {
									VECTOR_ref vects = VECTOR_ref(VTransform(VGet(0, 0, 0), veh.obj.GetFrameLocalMatrix(w.frame.first).get()));
									if (vects.x() * ((k == 0) ? 1 : -1) > 0) {
										f.Wheel.resize(f.Wheel.size() + 1);
										b2CircleShape shape;
										shape.m_radius = VTransform(VGet(0, 0, 0), veh.obj.GetFrameLocalMatrix(w.frame.first).get()).y - 0.1f;
										b2FixtureDef fw_fixtureDef;
										fw_fixtureDef.shape = &shape;
										fw_fixtureDef.density = 1.0f;
										b2BodyDef fw_bodyDef;
										fw_bodyDef.type = b2_kinematicBody;
										fw_bodyDef.position.Set(vects.z(), vects.y());
										f.Wheel.back().body.reset(f.world->CreateBody(&fw_bodyDef));
										f.Wheel.back().playerfix = f.Wheel.back().body->CreateFixture(&fw_fixtureDef);
									}
								}
								//誘導輪(動かない)
								for (auto& w : veh.use_veh.wheelframe_nospring) {
									VECTOR_ref vects = VTransform(VGet(0, 0, 0), veh.obj.GetFrameLocalMatrix(w.frame.first).get());
									if (vects.x() * ((k == 0) ? 1 : -1) > 0) {
										f.Yudo.resize(f.Yudo.size() + 1);
										b2CircleShape shape;
										shape.m_radius = 0.05f;
										b2FixtureDef fy_fixtureDef;
										fy_fixtureDef.shape = &shape;
										fy_fixtureDef.density = 1.0f;
										b2BodyDef fy_bodyDef;
										fy_bodyDef.type = b2_kinematicBody;
										fy_bodyDef.position.Set(vects.z(), vects.y());
										f.Yudo.back().body.reset(f.world->CreateBody(&fy_bodyDef));
										f.Yudo.back().playerfix = f.Yudo.back().body->CreateFixture(&fy_fixtureDef);
									}
								}
							}
						}
					}
				}
				//飛行機
				{
					auto& veh = c.vehicle[1];
					{
						c.p_anime_geardown.first = MV1AttachAnim(veh.obj.get(), 1);
						c.p_anime_geardown.second = 1.f;
						MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
						//舵
						for (int i = 0; i < c.p_animes_rudder.size(); i++) {
							c.p_animes_rudder[i].first = MV1AttachAnim(veh.obj.get(), 2 + i);
							c.p_animes_rudder[i].second = 0.f;
							MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
						}
					}
					//エフェクト
					for (auto& be : veh.use_veh.burner) {
						c.p_burner.emplace_back(be);
					}
					c.changegear.first = true;
					c.changegear.second = 2;
					c.landing.first = false;
					c.landing.second = 2;
				}
			}
		}
		//弾き
		bool get_reco(std::vector<Chara>& tgts, ammos& c, const uint8_t& type) {
			if (c.flug) {
				bool is_hit;
				std::optional<size_t> hitnear;
				for (auto& t : tgts) {
					//自分自身は省く
					if (this->id == t.id) {
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
							switch (c.spec.type_a) {
							case 0: //AP
								//装甲面に当たったのでhitnearに代入して終了
								for (auto& a : veh.use_veh.armer_mesh) {
									if (tt.first == a.first) {
										hitnear = tt.first;
										//ダメージ面に当たった時に装甲値に勝てるかどうか
										{
											VECTOR_ref normal = veh.hitres[hitnear.value()].Normal;
											VECTOR_ref position = veh.hitres[hitnear.value()].HitPosition;
											if (c.spec.pene_a > a.second * (1.0f / std::abs(c.vec.Norm().dot(normal)))) {
												//貫通
												//t.id;
												veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //

												veh.HP = std::max<int16_t>(veh.HP - c.spec.damage_a, 0); //
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
												veh.hit_obj[t.hitbuf].use = 0;
											}
											else {
												//はじく
												//弾処理
												c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
												c.vec = c.vec.Norm();
												c.pos = c.vec * (0.1f) + position;
												c.spec.pene_a /= 2.0f;
												//弾痕
												veh.hit_obj[t.hitbuf].use = 1;
											}
											if (c.spec.caliber_a >= 0.020f) {
												set_effect(&this->effcs[ef_reco], c.pos, normal);
											}
											else {
												set_effect(&this->effcs[ef_reco2], c.pos, normal);
											}

											//弾痕のセット
											{
												float asize = c.spec.caliber_a * 100.f;
												auto scale = VGet(asize / std::abs(c.vec.Norm().dot(normal)), asize, asize);
												auto y_vec = MATRIX_ref::Vtrans(normal, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
												auto z_vec = MATRIX_ref::Vtrans(normal.cross(c.vec), veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));

												veh.hit_obj[t.hitbuf].mat = MATRIX_ref::Scale(scale)* MATRIX_ref::Axis1(y_vec.cross(z_vec), y_vec, z_vec);
												veh.hit_obj[t.hitbuf].pos = MATRIX_ref::Vtrans(position - veh.pos, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180))) + y_vec * 0.02f;
												veh.hit_obj[t.hitbuf].flug = true;
												++t.hitbuf %= veh.hit_obj.size();
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
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
										c.spec.pene_a /= 2.0f;
									}
								}
								for (auto& a : veh.use_veh.module_mesh) {
									if (tt.first == a) {
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
										c.spec.pene_a /= 2.0f;
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
											if (c.spec.pene_a > a.second * (1.0f / std::abs(c.vec.Norm().dot(normal)))) {
												//貫通
												veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //

												veh.HP = std::max<int16_t>(veh.HP - c.spec.damage_a, 0); //
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
												veh.hit_obj[t.hitbuf].use = 0;
											}
											else {
												//爆発する
												//弾処理
												c.flug = false;
												c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
												c.vec = c.vec.Norm();
												c.pos = c.vec * (0.1f) + position;
												//弾痕
												veh.hit_obj[t.hitbuf].use = 1;
											}
											if (c.spec.caliber_a >= 0.020f) {
												set_effect(&this->effcs[ef_reco], c.pos, normal);
											}
											else {
												set_effect(&this->effcs[ef_reco2], c.pos, normal);
											}

											//弾痕のセット
											{
												float asize = c.spec.caliber_a * 100.f;
												auto scale = VGet(asize / std::abs(c.vec.Norm().dot(normal)), asize, asize);
												auto y_vec = MATRIX_ref::Vtrans(normal, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
												auto z_vec = MATRIX_ref::Vtrans(normal.cross(c.vec), veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));

												veh.hit_obj[t.hitbuf].mat = MATRIX_ref::Scale(scale)* MATRIX_ref::Axis1(y_vec.cross(z_vec), y_vec, z_vec);
												veh.hit_obj[t.hitbuf].pos = MATRIX_ref::Vtrans(position - veh.pos, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180))) + y_vec * 0.02f;
												veh.hit_obj[t.hitbuf].flug = true;
												++t.hitbuf %= veh.hit_obj.size();
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
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
										//爆発する
										c.flug = false;
									}
								}
								for (auto& a : veh.use_veh.module_mesh) {
									if (tt.first == a) {
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
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
											if (c.spec.pene_a > a.second * (1.0f / std::abs(c.vec.Norm().dot(normal)))) {
												//貫通
												veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //

												veh.HP = std::max<int16_t>(veh.HP - c.spec.damage_a, 0); //
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
												veh.hit_obj[t.hitbuf].use = 0;
											}
											else {
												//爆発する
												//弾処理
												c.flug = false;
												c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
												c.vec = c.vec.Norm();
												c.pos = c.vec * (0.1f) + position;
												//弾痕
												veh.hit_obj[t.hitbuf].use = 1;
											}
											if (c.spec.caliber_a >= 0.020f) {
												set_effect(&this->effcs[ef_reco], c.pos, normal);
											}
											else {
												set_effect(&this->effcs[ef_reco2], c.pos, normal);
											}

											//弾痕のセット
											{
												float asize = c.spec.caliber_a * 100.f;
												auto scale = VGet(asize / std::abs(c.vec.Norm().dot(normal)), asize, asize);
												auto y_vec = MATRIX_ref::Vtrans(normal, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
												auto z_vec = MATRIX_ref::Vtrans(normal.cross(c.vec), veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));

												veh.hit_obj[t.hitbuf].mat = MATRIX_ref::Scale(scale)* MATRIX_ref::Axis1(y_vec.cross(z_vec), y_vec, z_vec);
												veh.hit_obj[t.hitbuf].pos = MATRIX_ref::Vtrans(position - veh.pos, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180))) + y_vec * 0.02f;
												veh.hit_obj[t.hitbuf].flug = true;
												++t.hitbuf %= veh.hit_obj.size();
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
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
										//爆発する
										c.flug = false;
									}
								}
								for (auto& a : veh.use_veh.module_mesh) {
									if (tt.first == a) {
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
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
};
//
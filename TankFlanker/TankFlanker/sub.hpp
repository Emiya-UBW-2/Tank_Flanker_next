#pragma once

#define NOMINMAX
#include"DXLib_ref.h"
#include <fstream>
#include <array>
#include <vector>
#include <D3D11.h>
#include <memory>
#include<optional>
#include "Box2D/Box2D.h"



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
};
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
		uint16_t rounds=0;
	};
	struct foot {
		frames frame;
		EffectS gndsmkeffcs;
		float gndsmksize = 1.f;
	};

	//弾薬
	class Ammos {
	public:
		std::string name;
		int16_t type=0;
		float caliber = 0.f;
		float penetration = 0.f;
		float speed = 0.f;
	};
	static void set_ammos(std::vector<hit::Ammos>* Ammos) {
		auto& a = *Ammos;
		a.resize(a.size() + 1);
		a.back().name = "M728 APDS";
		a.back().type = 0;//ap=0,he=1
		a.back().caliber = 0.105f;
		a.back().penetration = 268.f;
		a.back().speed = 1426.f;

		a.resize(a.size() + 1);
		a.back().name = "M393 HESH";
		a.back().type = 1;//ap=0,he=1
		a.back().caliber = 0.105f;
		a.back().penetration = 145.f;
		a.back().speed = 730.f;

		a.resize(a.size() + 1);
		a.back().name = "NATO 7.62";
		a.back().type = 1;//ap=0,he=1
		a.back().caliber = 0.00762f;
		a.back().penetration = 12.f;
		a.back().speed = 600.f;

		a.resize(a.size() + 1);
		a.back().name = "BR-412D APCBC";
		a.back().type = 0;//ap=0,he=1
		a.back().caliber = 0.100f;
		a.back().penetration = 175.f;
		a.back().speed = 887.f;

		a.resize(a.size() + 1);
		a.back().name = "USSR 12.7";
		a.back().type = 1;//ap=0,he=1
		a.back().caliber = 0.0127f;
		a.back().penetration = 24.f;
		a.back().speed = 850.f;

		a.resize(a.size() + 1);
		a.back().name = "vulcan 20";
		a.back().type = 1;//ap=0,he=1
		a.back().caliber = 0.020f;
		a.back().penetration = 30.f;
		a.back().speed = 800.f;

		a.resize(a.size() + 1);
		a.back().name = "M344A1 HEAT";
		a.back().type = 1;//ap=0,he=1
		a.back().caliber = 0.106f;
		a.back().penetration = 400.f;
		a.back().speed = 503.f;
	}
	//戦車
	class Tanks {
	public:
		MV1 obj;
		MV1 col;
		std::string name;
		float body_rad_limit = 0.f;/*旋回速度(度/秒)*/
		VECTOR_ref minpos, maxpos;
		std::vector<gun> gunframe;
		std::vector<foot> wheelframe;
		uint16_t HP = 0;
	public:
		bool isfloat=false;					/*浮くかどうか*/
		float down_in_water = 0.f;				/*沈む判定箇所*/
		std::array<int, 4> square{ 0 };				/*脚*/
		float flont_speed_limit = 0.f;				/*前進速度(km/h)*/
		float back_speed_limit = 0.f;				/*後退速度(km/h)*/
		float turret_rad_limit = 0.f;				/*砲塔駆動速度(度/秒)*/
		std::vector<std::pair<size_t, float>> armer_mesh;	/*装甲ID*/
		std::vector<size_t> space_mesh;				/*装甲ID*/
		std::vector<size_t> module_mesh;			/*装甲ID*/

		void into(const Tanks& t) {
			this->isfloat = t.isfloat;
			this->down_in_water = t.down_in_water;
			this->square = t.square;


			this->name = t.name;
			this->obj = t.obj.Duplicate();
			this->col = t.col.Duplicate();
			this->wheelframe.clear();
			for (auto& p : t.wheelframe) {
				this->wheelframe.resize(this->wheelframe.size() + 1);
				this->wheelframe.back().frame = p.frame;
			}
			this->flont_speed_limit = t.flont_speed_limit;
			this->back_speed_limit = t.back_speed_limit;
			this->body_rad_limit = t.body_rad_limit;
			this->turret_rad_limit = t.turret_rad_limit;
			this->armer_mesh = t.armer_mesh;
			this->space_mesh = t.space_mesh;
			this->module_mesh = t.module_mesh;
			this->minpos = t.minpos;
			this->maxpos = t.maxpos;
			this->gunframe = t.gunframe;
			this->HP = t.HP;
		}
	};
	static void set_tanks(std::vector<hit::Tanks>*tank) {
		for (auto& t : *tank) {
			MV1::Load("data/tank/" + t.name + "/model.mv1", &t.obj);
			MV1::Load("data/tank/" + t.name + "/col.mv1", &t.col);
			//
			t.down_in_water = 0.f;
			for (int i = 0; i < t.obj.mesh_num(); i++) {
				auto p = t.obj.mesh_maxpos(i).y();
				if (t.down_in_water < p) {
					t.down_in_water = p;
				}
			}
			t.obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
			for (int i = 0; i < t.obj.frame_num(); i++) {
				std::string p = t.obj.frame_name(i);
				if (p.find("転輪", 0) != std::string::npos) {
					t.wheelframe.resize(t.wheelframe.size() + 1);
					t.wheelframe.back().frame.first = i;
					t.wheelframe.back().frame.second = t.obj.frame(t.wheelframe.back().frame.first);
				}
				else if (p.find("旋回", 0) != std::string::npos) {
					t.gunframe.resize(t.gunframe.size() + 1);
					t.gunframe.back().frame1.first = i;
					t.gunframe.back().frame1.second = t.obj.frame(t.gunframe.back().frame1.first);
					auto p2 = t.obj.frame_parent(t.gunframe.back().frame1.first);
					if (p2 >= 0) {
						t.gunframe.back().frame1.second -= t.obj.frame(int(p2));//親がいる時引いとく
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
				else if (p.find("min", 0) != std::string::npos) {
					t.minpos = t.obj.frame(i);
				}
				else if (p.find("max", 0) != std::string::npos) {
					t.maxpos = t.obj.frame(i);
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
			//
			for (int i = 0; i < t.col.mesh_num(); i++) {
				std::string p = t.col.material_name(i);
				if (p.find("armer", 0) != std::string::npos) {//装甲
					t.armer_mesh.resize(t.armer_mesh.size() + 1);
					t.armer_mesh.back().first = i;
					t.armer_mesh.back().second = std::stof(getright(p.c_str()));//装甲値
				}
				else if (p.find("space", 0) != std::string::npos) {//空間装甲
					t.space_mesh.resize(t.space_mesh.size() + 1);
					t.space_mesh.back() = i;
				}
				else {//モジュール
					t.module_mesh.resize(t.module_mesh.size() + 1);
					t.module_mesh.back() = i;
				}
			}

			{
				int mdata = FileRead_open(("data/tank/" + t.name + "/data.txt").c_str(), FALSE);
				char mstr[64]; /*tank*/
				FileRead_gets(mstr, 64, mdata);
				t.isfloat = (getright(mstr).find("true") != std::string::npos);
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
						g.useammo.resize(g.useammo.size() + 1);
						g.useammo.back() = getright(mstr);
					}
				}
				FileRead_close(mdata);
			}
		}
	}
	//飛行機
	class Planes {
	public:
		MV1 obj;
		MV1 col;
		std::string name;
		float body_rad_limit = 0.f;/*旋回速度(度/秒)*/
		VECTOR_ref minpos, maxpos;
		std::vector<gun> gunframe;
		std::vector<foot> wheelframe;
		uint16_t HP = 0;
	public:
		float max_speed_limit = 0.f;/*最高速度(km/h)*/
		float mid_speed_limit = 0.f;/*巡行速度(km/h)*/
		float min_speed_limit = 0.f;/*失速速度(km/h)*/
		std::vector<frames> burner;
		frames hook;

		void into(const Planes& t) {
			this->name = t.name;
			this->obj = t.obj.Duplicate();
			this->col = t.col.Duplicate();
			this->wheelframe.clear();
			for (auto& p : t.wheelframe) {
				this->wheelframe.resize(this->wheelframe.size() + 1);
				this->wheelframe.back().frame = p.frame;
			}
			this->max_speed_limit = t.max_speed_limit;
			this->mid_speed_limit = t.mid_speed_limit;
			this->min_speed_limit = t.min_speed_limit;
			this->body_rad_limit = t.body_rad_limit;
			this->minpos = t.minpos;
			this->maxpos = t.maxpos;
			this->gunframe = t.gunframe;
			this->burner = t.burner;
			this->hook = t.hook;
			this->HP = t.HP;
		}
	};
	static void set_planes(std::vector<hit::Planes>* plane) {
		for (auto& t : *plane) {
			//モデル
			MV1::Load("data/plane/" + t.name + "/model.mv1", &t.obj);
			MV1::Load("data/plane/" + t.name + "/col.mv1", &t.col);
			//αテスト
			t.obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
			//最大最小を取得
			for (int i = 0; i < t.obj.mesh_num(); i++) {
				if (t.maxpos.x() < t.obj.mesh_maxpos(i).x()) { t.maxpos.x(t.obj.mesh_maxpos(i).x()); }
				if (t.maxpos.z() < t.obj.mesh_maxpos(i).z()) { t.maxpos.z(t.obj.mesh_maxpos(i).z()); }
				if (t.minpos.x() > t.obj.mesh_minpos(i).x()) { t.minpos.x(t.obj.mesh_minpos(i).x()); }
				if (t.minpos.z() > t.obj.mesh_minpos(i).z()) { t.minpos.z(t.obj.mesh_minpos(i).z()); }
			}
			//フレーム
			for (int i = 0; i < t.obj.frame_num(); i++) {
				std::string p = t.obj.frame_name(i);
				if (p.find("脚", 0) != std::string::npos) {
					if (p.find("ハッチ", 0) == std::string::npos) {
						t.wheelframe.resize(t.wheelframe.size() + 1);
						t.wheelframe.back().frame.first = i;
						t.wheelframe.back().frame.second = t.obj.frame(t.wheelframe.back().frame.first);
					}
				}
				else if (p.find("旋回", 0) != std::string::npos) {
					t.gunframe.resize(t.gunframe.size() + 1);
					t.gunframe.back().frame1.first = i;
					t.gunframe.back().frame1.second = t.obj.frame(t.gunframe.back().frame1.first);
					auto p2 = t.obj.frame_parent(t.gunframe.back().frame1.first);
					if (p2 >= 0) {
						t.gunframe.back().frame1.second -= t.obj.frame(int(p2));//親がいる時引いとく
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
			}
			//データ取得
			{
				int mdata = FileRead_open(("data/plane/" + t.name + "/data.txt").c_str(), FALSE);
				char mstr[64]; /*tank*/
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
	//player
	struct ammos {
		bool hit{ false };
		bool flug{ false };
		float cnt = 0.f;
		int color = 0;
		Ammos spec;
		float yadd=0.f;
		VECTOR_ref pos, repos, vec;
	};
	struct b2Pats {
		std::unique_ptr<b2Body> body; /**/
		b2Fixture* playerfix{nullptr};	 /**/
	};
	struct wallPats {
		hit::b2Pats b2;
		std::array<VECTOR_ref, 2> pos;
	};
	typedef std::pair<int, float> animes;
	struct Chara {
		size_t id = 0;
		//
		//エフェクト
		std::array<EffectS, efs_user> effcs; /*effect*/
		//操作関連
		uint8_t mode = 0;	//フライト(形態)モード
		std::array<bool, 15> key{false};//キー
		float view_xrad = 0.f, view_yrad = 0.f;//砲塔操作用ベクトル
	//====================================================
	//戦車//==================================================
		Tanks usetank;
		float wheel_Left = 0.f, wheel_Right = 0.f;    //転輪回転
		b2Pats mine;				     /*box2d*/
		float spd=0.f;				     /*box2d*/
		struct Hit {
			bool flug{ false };		     /*弾痕フラグ*/
			int use{ 0 };			     /*使用フレーム*/
			MV1 pic;			     /*弾痕モデル*/
			VECTOR_ref scale, pos, z_vec, y_vec; /*座標*/
		};
		int hitbuf=0;				     /*使用弾痕*/
		std::array<Hit, 24> hit;		     /*弾痕*/
		std::vector<MV1_COLL_RESULT_POLY> hitres;    /*確保*/
		std::vector<int16_t> HP;		     /*ライフ*/
		std::vector<pair> hitssort;		     /*フレームに当たった順番*/
		float xradp_shot = 0.f, xrad_shot = 0.f;     //射撃反動x
		float zradp_shot = 0.f, zrad_shot = 0.f;     //射撃反動z
		VECTOR_ref wheel_normal;		     /*ノーマル*/
	//飛行機//==================================================
		Planes useplane;		      //
		animes p_anime_geardown;	      //車輪アニメーション
		uint8_t changegear_cnt = 0;	      //カウント
		bool changegear = false;	      //スイッチ
		float p_landing_per = 0.f;	      //着艦フック
		uint8_t landing_cnt = 0;	      //カウント
		bool landing = false;		      //スイッチ
		std::array<animes, 6> p_animes_rudder;//ラダーアニメーション
		struct burners {
			frames frame;
			MV1 effectobj;
		};
		std::vector<burners> p_burner;
		//共通項
		struct vehicles {
			bool hit_check = false;		      //当たり判定を取るかチェック
			size_t use_id = 0;		      //使用する車両(機材)
			uint16_t HP = 0;
			VECTOR_ref pos;			      //車体座標
			MATRIX_ref mat;			      //車体回転行列
			VECTOR_ref add;			      //車体加速度
			struct Guns {
				size_t usebullet{};	      /*使用弾*/
				std::array<ammos, 64> bullet; /*確保する弾*/
				float loadcnt{ 0 };	      /*装てんカウンター*/
				float fired{ 0.f };	      /*駐退数*/
				int16_t rounds{ 0 };
				gun gun_info;		      /**/
				std::vector<Ammos> Spec;      /**/
			};
			std::vector<Guns> Gun_;
			float speed = 0.f, speed_add = 0.f, speed_sub = 0.f;
			float xrad = 0.f, xradadd = 0.f, xradadd_left = 0.f, xradadd_right = 0.f;
			float yrad = 0.f, yradadd = 0.f, yradadd_left = 0.f, yradadd_right = 0.f;
			float zrad = 0.f, zradadd = 0.f, zradadd_left = 0.f, zradadd_right = 0.f;
		};
		std::array<vehicles, 2> vehicle;//0=戦車,1=飛行機
	};
	//
	static bool get_reco(Chara& play, std::vector<Chara>& tgts, ammos& c) {
		if (c.flug) {
			if (c.spec.type == 0) {
				bool is_hit;
				std::optional<size_t> hitnear;
				for (auto& t : tgts) {
					//自分自身は省く
					if (play.id == t.id) {
						continue;
					}
					//とりあえず当たったかどうか探す
					is_hit = false;
					//モジュール
					for (auto& m : t.usetank.module_mesh) {
						t.hitres[m] = t.usetank.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos).Scale(0.1f)), -1, int(m));
						if (t.hitres[m].HitFlag) {
							t.hitssort[m] = pair(m, (c.repos - t.hitres[m].HitPosition).size());
							is_hit = true;
						}
						else {
							t.hitssort[m] = pair(m, (std::numeric_limits<float>::max)());
						}
					}
					//空間装甲
					for (auto& m : t.usetank.space_mesh) {
						t.hitres[m] = t.usetank.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos).Scale(0.1f)), -1, int(m));
						if (t.hitres[m].HitFlag) {
							t.hitssort[m] = pair(m, (c.repos - t.hitres[m].HitPosition).size());
							is_hit = true;
						}
						else {
							t.hitssort[m] = pair(m, (std::numeric_limits<float>::max)());
						}
					}
					//装甲
					for (auto& m : t.usetank.armer_mesh) {
						t.hitres[m.first] = t.usetank.col.CollCheck_Line(c.repos, c.pos, -1, int(m.first));
						if (t.hitres[m.first].HitFlag) {
							t.hitssort[m.first] = pair(m.first, (c.repos - t.hitres[m.first].HitPosition).size());
							is_hit = true;
						}
						else {
							t.hitssort[m.first] = pair(m.first, (std::numeric_limits<float>::max)());
						}
					}
					//当たってない
					if (!is_hit) {
						continue;
					}
					//当たり判定を近い順にソート
					std::sort(t.hitssort.begin(), t.hitssort.end(), [](const pair& x, const pair& y) { return x.second < y.second; });
					//判定
					{
						//ダメージ面に届くまで判定
						for (auto& tt : t.hitssort) {
							//装甲面に当たらなかったならスルー
							if (tt.second == (std::numeric_limits<float>::max)()) {
								break;
							}
							//装甲面に当たったのでhitnearに代入して終了
							for (auto& a : t.usetank.armer_mesh) {
								if (tt.first == a.first) {
									hitnear = tt.first;
									//ダメージ面に当たった時に装甲値に勝てるかどうか
									{
										VECTOR_ref normal = t.hitres[hitnear.value()].Normal;
										VECTOR_ref position = t.hitres[hitnear.value()].HitPosition;
										if (c.spec.penetration > a.second * (1.0f / std::abs(c.vec.Norm() % normal))) {
											//貫通
											//t.id;
											t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 30, 0); //


											t.vehicle[0].HP = std::max<int16_t>(t.vehicle[0].HP - 100, 0); //
											//撃破時エフェクト
											if (t.vehicle[0].HP == 0) {
												set_effect(&t.effcs[ef_bomb], t.usetank.obj.frame(t.usetank.gunframe[0].frame1.first), VGet(0, 0, 0));
											}
											//弾処理
											c.flug = false;
											c.vec += normal.Scale((c.vec % normal) * -2.0f);
											c.vec = c.vec.Norm();
											c.pos = c.vec.Scale(0.1f) + position;
											//弾痕
											t.hit[t.hitbuf].use = 0;
										}
										else {
											//はじく
											//弾処理
											c.vec += normal.Scale((c.vec % normal) * -2.0f);
											c.vec = c.vec.Norm();
											c.pos = c.vec.Scale(0.1f) + position;
											c.spec.penetration /= 2.0f;
											//弾痕
											t.hit[t.hitbuf].use = 1;
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
											t.hit[t.hitbuf].scale = VGet(asize / std::abs(c.vec.Norm() % normal), asize, asize);
											t.hit[t.hitbuf].pos = (position - t.vehicle[0].pos).Vtrans(t.vehicle[0].mat.Inverse());
											t.hit[t.hitbuf].y_vec = normal.Vtrans(t.vehicle[0].mat.Inverse());
											t.hit[t.hitbuf].z_vec = (normal * c.vec).Vtrans(t.vehicle[0].mat.Inverse());
											t.hit[t.hitbuf].flug = true;
											++t.hitbuf %= t.hit.size();
										}
									}
									break;
								}
							}
							if (hitnear.has_value()) {
								break;
							}
							//空間装甲、モジュールに当たったのでモジュールに30ダメ、貫徹力を1/2に
							for (auto& a : t.usetank.space_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
									}
									t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 30, 0); //
									c.spec.penetration /= 2.0f;
								}
							}
							for (auto& a : t.usetank.module_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
									}
									t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 30, 0); //
									c.spec.penetration /= 2.0f;
								}
							}
						}
					}
					if (hitnear.has_value())
						break;
				}
				return (hitnear.has_value());
			}
			if (c.spec.type == 1) {
				bool is_hit;
				std::optional<size_t> hitnear;
				for (auto& t : tgts) {
					//自分自身は省く
					if (play.id == t.id) {
						continue;
					}
					//とりあえず当たったかどうか探す
					is_hit = false;
					//モジュール
					for (auto& m : t.usetank.module_mesh) {
						t.hitres[m] = t.usetank.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos).Scale(0.1f)), -1, int(m));
						if (t.hitres[m].HitFlag) {
							t.hitssort[m] = pair(m, (c.repos - t.hitres[m].HitPosition).size());
							is_hit = true;
						}
						else {
							t.hitssort[m] = pair(m, (std::numeric_limits<float>::max)());
						}
					}
					//空間装甲
					for (auto& m : t.usetank.space_mesh) {
						t.hitres[m] = t.usetank.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos).Scale(0.1f)), -1, int(m));
						if (t.hitres[m].HitFlag) {
							t.hitssort[m] = pair(m, (c.repos - t.hitres[m].HitPosition).size());
							is_hit = true;
						}
						else {
							t.hitssort[m] = pair(m, (std::numeric_limits<float>::max)());
						}
					}
					//装甲
					for (auto& m : t.usetank.armer_mesh) {
						t.hitres[m.first] = t.usetank.col.CollCheck_Line(c.repos, c.pos, -1, int(m.first));
						if (t.hitres[m.first].HitFlag) {
							t.hitssort[m.first] = pair(m.first, (c.repos - t.hitres[m.first].HitPosition).size());
							is_hit = true;
						}
						else {
							t.hitssort[m.first] = pair(m.first, (std::numeric_limits<float>::max)());
						}
					}
					//当たってない
					if (!is_hit) {
						continue;
					}
					//当たり判定を近い順にソート
					std::sort(t.hitssort.begin(), t.hitssort.end(), [](const pair& x, const pair& y) { return x.second < y.second; });
					//判定
					{
						//ダメージ面に届くまで判定
						for (auto& tt : t.hitssort) {
							//装甲面に当たらなかったならスルー
							if (tt.second == (std::numeric_limits<float>::max)()) {
								break;
							}
							//装甲面に当たったのでhitnearに代入して終了
							for (auto& a : t.usetank.armer_mesh) {
								if (tt.first == a.first) {
									hitnear = tt.first;
									//ダメージ面に当たった時に装甲値に勝てるかどうか
									{
										VECTOR_ref normal = t.hitres[hitnear.value()].Normal;
										VECTOR_ref position = t.hitres[hitnear.value()].HitPosition;
										if (c.spec.penetration > a.second * (1.0f / std::abs(c.vec.Norm() % normal))) {
											//貫通
											t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 30, 0); //

											t.vehicle[0].HP = std::max<int16_t>(t.vehicle[0].HP - 100, 0); //
											//撃破時エフェクト
											if (t.vehicle[0].HP == 0) {
												set_effect(&t.effcs[ef_bomb], t.usetank.obj.frame(t.usetank.gunframe[0].frame1.first), VGet(0, 0, 0));
											}
											//弾処理
											c.flug = false;
											c.vec += normal.Scale((c.vec % normal) * -2.0f);
											c.vec = c.vec.Norm();
											c.pos = c.vec.Scale(0.1f) + position;
											//弾痕
											t.hit[t.hitbuf].use = 0;
										}
										else {
											//爆発する
											//弾処理
											c.flug = false;
											c.vec += normal.Scale((c.vec % normal) * -2.0f);
											c.vec = c.vec.Norm();
											c.pos = c.vec.Scale(0.1f) + position;
											//弾痕
											t.hit[t.hitbuf].use = 1;
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
											t.hit[t.hitbuf].scale = VGet(asize / std::abs(c.vec.Norm() % normal), asize, asize);
											t.hit[t.hitbuf].pos = (position - t.vehicle[0].pos).Vtrans(t.vehicle[0].mat.Inverse());
											t.hit[t.hitbuf].y_vec = normal.Vtrans(t.vehicle[0].mat.Inverse());
											t.hit[t.hitbuf].z_vec = (normal * c.vec).Vtrans(t.vehicle[0].mat.Inverse());
											t.hit[t.hitbuf].flug = true;
											++t.hitbuf %= t.hit.size();
										}
									}
									break;
								}
							}
							if (hitnear.has_value()) {
								break;
							}
							//空間装甲、モジュールに当たったのでモジュールに30ダメ、弾なし
							for (auto& a : t.usetank.space_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
									}
									t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 30, 0); //
									//爆発する
									c.flug = false;
								}
							}
							for (auto& a : t.usetank.module_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
									}
									t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 30, 0); //
									//爆発する
									c.flug = false;
								}
							}
						}
					}
					if (hitnear.has_value())
						break;
				}
				return (hitnear.has_value());
			}
		}
		return false;
	}
};
//
void find_folders(const char* name, std::vector<hit::Tanks>* doing) {
	WIN32_FIND_DATA win32fdt;
	HANDLE hFind;
	hFind = FindFirstFile(name, &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((win32fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (win32fdt.cFileName[0] != '.')) {
				doing->resize(doing->size() + 1);
				doing->back().name = win32fdt.cFileName;
			}
		} while (FindNextFile(hFind, &win32fdt));
	} //else{ return false; }
	FindClose(hFind);
}
void find_folders(const char* name, std::vector<hit::Planes>* doing) {
	WIN32_FIND_DATA win32fdt;
	HANDLE hFind;
	hFind = FindFirstFile(name, &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((win32fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (win32fdt.cFileName[0] != '.')) {
				doing->resize(doing->size() + 1);
				doing->back().name = win32fdt.cFileName;
			}
		} while (FindNextFile(hFind, &win32fdt));
	} //else{ return false; }
	FindClose(hFind);
}

template <class T>
void fill_id(std::vector<T>& vect) {
	for (int i = 0; i < vect.size(); i++) {
		vect[i].id = i;
	}
}
template <class T, size_t N >
void fill_id(std::array<T, N>& vect) {
	for (int i = 0; i < vect.size(); i++) {
		vect[i].id = i;
	}
}
//ok
class UI :hit {
private:
	GraphHandle circle;
	GraphHandle aim;
	GraphHandle scope;
	GraphHandle CompassScreen;
	MV1 Compass;
	//font
	FontHandle font18;
public:
	UI() {
		circle = GraphHandle::Load("data/UI/battle_circle.bmp");
		aim = GraphHandle::Load("data/UI/battle_aim.bmp");
		scope = GraphHandle::Load("data/UI/battle_scope.png");

		CompassScreen = GraphHandle::Make(240, 240, true);
		MV1::Load("data/compass/model.mv1", &Compass);
		font18 = FontHandle::Create(18, DX_FONTTYPE_EDGE);
	}
	~UI() {

	}

	void draw(const VECTOR_ref& aimpos, const hit::Chara& chara, const bool& ads) {
		if (chara.mode == 1) {
			auto scr = GetDrawScreen();
			auto fov = GetCameraFov();
			{
				SetDrawScreen(CompassScreen.get());
				ClearDrawScreen();
				SetupCamera_Ortho(1.f);
				SetCameraPositionAndTargetAndUpVec(VGet(0.f, 0.f, 3.f), VGet(0.f, 0.f, 0.f), VGet(0.f, -1.f, 0.f));
				SetCameraNearFar(0.1f, 6.0f);
				auto ltd = GetLightDirection();
				SetLightDirection(VGet(0.f, 0.f, -1.f));
				//SetUseLighting(FALSE);
				Compass.SetRotationZYAxis(
					VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(chara.vehicle[1].mat.Inverse()),
					VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(chara.vehicle[1].mat.Inverse()),
					0.f
				);
				Compass.SetPosition(VGet(0.f, 0.f, 0.f));
				Compass.DrawModel();
				//SetUseLighting(TRUE);
				SetLightDirection(ltd);
			}
			SetDrawScreen(scr);
			SetupCamera_Perspective(fov);
		}
		if (aimpos.z() >= 0.f && aimpos.z() <= 1.f) {
			circle.DrawExtendGraph(int(aimpos.x()) - x_r(128), int(aimpos.y()) - y_r(128), int(aimpos.x()) + x_r(128), int(aimpos.y()) + y_r(128), TRUE);
		}

		aim.DrawExtendGraph(dispx / 2 - x_r(128), dispy / 2 - y_r(128), dispx / 2 + x_r(128), dispy / 2 + y_r(128), TRUE);

		if (ads) {
			scope.DrawExtendGraph(0, 0, dispx, dispy, true);
		}

		int i = 0;
		for (auto& p : chara.vehicle[chara.mode].Gun_) {
			int xp = 20 + 30 - (30 * (i + 1) / int(chara.vehicle[chara.mode].Gun_.size()));
			int yp = 200 + i * 42;
			DrawBox(xp, yp, xp + 200, yp + 38, GetColor(128, 128, 128), TRUE);
			if (p.loadcnt != 0.f) {
				DrawBox(xp, yp, xp + 200 - int(200.f*p.loadcnt / p.gun_info.load_time), yp + 18, GetColor(255, 0, 0), TRUE);
			}
			else {
				DrawBox(xp, yp, xp + 200, yp + 18, GetColor(0, 255, 0), TRUE);
			}
			if (p.rounds != 0.f) {
				DrawBox(xp, yp + 20, xp + int(200.f*p.rounds / p.gun_info.rounds), yp + 38, GetColor(255, 192, 0), TRUE);
			}
			font18.DrawString(xp, yp, p.bullet[p.usebullet].spec.name, GetColor(255, 255, 255));
			font18.DrawStringFormat(xp + 200 - font18.GetDrawWidthFormat("%04d / %04d", p.rounds, p.gun_info.rounds), yp + 20, GetColor(255, 255, 255), "%04d / %04d", p.rounds, p.gun_info.rounds);
			i++;
		}
		if (chara.mode == 1) {
			CompassScreen.DrawExtendGraph(dispx * 2 / 3, dispy * 2 / 3, dispx * 2 / 3 + 240 / 2, dispy * 2 / 3 + 240 / 2, true);



			DrawLine(dispx / 3, dispy / 3, dispx / 3, dispy * 2 / 3, GetColor(255, 255, 255), 3);
			font18.DrawStringFormat(dispx / 3 - font18.GetDrawWidthFormat("SPD %6.2f km/h ", chara.vehicle[1].speed*3.6f), dispy / 2, GetColor(255, 255, 255), "SPD %6.2f km/h", chara.vehicle[1].speed*3.6f);

			DrawLine(dispx * 2 / 3, dispy / 3, dispx * 2 / 3, dispy * 2 / 3, GetColor(255, 255, 255), 3);
			font18.DrawStringFormat(dispx * 2 / 3, dispy / 2, GetColor(255, 255, 255), " %4d m", int(chara.vehicle[1].pos.y()));

			if (chara.vehicle[1].speed < chara.useplane.min_speed_limit) {
				font18.DrawString(dispx / 2 - font18.GetDrawWidth("STALL") / 2, dispy / 3, "STALL", GetColor(255, 0, 0));
			}
			if (chara.vehicle[1].pos.y() <= 30.f) {
				font18.DrawString(dispx / 2 - font18.GetDrawWidth("GPWS") / 2, dispy / 3 + 18, "GPWS", GetColor(255, 255, 0));
			}
		}

		{
			int j = 0;
			font18.DrawStringFormat(200, 300 + j * 24, GetColor(255, 255, 255), "HP : %d", chara.vehicle[0].HP);
			j++;
			font18.DrawStringFormat(200, 300 + j * 24, GetColor(255, 255, 255), "HP : %d", chara.vehicle[1].HP);
			j++;

			for (auto& h : chara.HP) {
				font18.DrawStringFormat(200, 300 + j * 24, GetColor(255, 255, 255), "HP : %d", h);
				j++;
			}
		}
	}
};
//
class HostPassEffect :hit {
private:
	GraphHandle SkyScreen;
	GraphHandle FarScreen;
	GraphHandle MainScreen;
	GraphHandle NearScreen;
	GraphHandle GaussScreen;
	const int EXTEND = 4;				  /*ブルーム用*/
public:
	HostPassEffect() {
		SkyScreen = GraphHandle::Make(dispx, dispy);
		FarScreen = GraphHandle::Make(dispx, dispy, true);
		MainScreen = GraphHandle::Make(dispx, dispy, true);
		NearScreen = GraphHandle::Make(dispx, dispy, true);
		GaussScreen = GraphHandle::Make(dispx / EXTEND, dispy / EXTEND); /*エフェクト*/
	}

	~HostPassEffect() {

	}
	//被写体深度描画
	template <typename T1, typename T2>
	void dof(
		GraphHandle* buf,
		T1 sky_doing, T2 doing,
		const VECTOR_ref& campos, const VECTOR_ref& camvec, const VECTOR_ref& camup, const float& fov,
		const float& far_distance = 1000.f, const float& near_distance = 100.f
	) {
		SetDrawScreen(SkyScreen.get());
		ClearDrawScreen();
		SetCameraNearFar(1000.0f, 5000.0f);
		SetupCamera_Perspective(fov);
		SetCameraPositionAndTargetAndUpVec((VECTOR_ref(campos) - camvec).get(), VGet(0, 0, 0), camup.get());
		sky_doing();
		SetDrawScreen(FarScreen.get());
		ClearDrawScreen();

		SetCameraNearFar(far_distance, 6000.0f);
		SetupCamera_Perspective(fov);
		SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
		{
			SkyScreen.DrawGraph(0, 0, FALSE);
		}
		doing();

		SetDrawScreen(MainScreen.get());
		ClearDrawScreen();
		SetCameraNearFar(near_distance, far_distance + 50.f);
		SetupCamera_Perspective(fov);
		SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
		{
			Effekseer_Sync3DSetting();
			GraphFilter(FarScreen.get(), DX_GRAPH_FILTER_GAUSS, 16, 200);
			FarScreen.DrawGraph(0, 0, false);
			UpdateEffekseer3D(); //2.0ms
		}
		doing();
		DrawEffekseer3D();

		SetDrawScreen(NearScreen.get());
		ClearDrawScreen();
		SetCameraNearFar(0.01f, near_distance + 0.5f);
		SetupCamera_Perspective(fov);
		SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
		doing();

		SetDrawScreen(buf->get());
		ClearDrawScreen();
		{
			MainScreen.DrawGraph(0, 0, false);
			NearScreen.DrawGraph(0, 0, true);
		}
	}
	//ブルームエフェクト
	void bloom(GraphHandle& BufScreen, const int& level = 255) {
		GraphFilter(BufScreen.get(), DX_GRAPH_FILTER_TWO_COLOR, 245, GetColor(0, 0, 0), 255, GetColor(128, 128, 128), 255);
		GraphFilterBlt(BufScreen.get(), GaussScreen.get(), DX_GRAPH_FILTER_DOWN_SCALE, EXTEND);
		GraphFilter(GaussScreen.get(), DX_GRAPH_FILTER_GAUSS, 16, 1000);
		SetDrawMode(DX_DRAWMODE_BILINEAR);
		SetDrawBlendMode(DX_BLENDMODE_ADD, level);
		GaussScreen.DrawExtendGraph(0, 0, dispx, dispy, false);
		GaussScreen.DrawExtendGraph(0, 0, dispx, dispy, false);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	}
public:
};
//

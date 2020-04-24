#pragma once

#define NOMINMAX
#include"DXLib_ref.h"
#include <fstream>
#include <array>
#include <vector>
#include <D3D11.h>
#include <memory>
#include<optional>

void set_effect(EffectS* efh, VECTOR_ref pos, VECTOR_ref nor) {
	efh->flug = true;
	efh->pos = pos;
	efh->nor = nor;
}
void set_pos_effect(EffectS* efh, const EffekseerEffectHandle& handle) {
	if (efh->flug) {
		efh->handle = handle.Play3D();
		efh->handle.SetPos(efh->pos);
		efh->handle.SetRotation(atan2(efh->nor.y(), std::hypot(efh->nor.x(), efh->nor.z())), atan2(-efh->nor.x(), -efh->nor.z()), 0);
		efh->flug = false;
	}
	//IsEffekseer3DEffectPlaying(player[0].effcs[i].handle)
}

//要改善
class hit{
public:
	class Tanks {
		struct gun {
			int type;
			int id;
			VECTOR_ref pos;
			int id2;
			VECTOR_ref pos2;
			int id3;
			VECTOR_ref pos3;
			float xrad = 0.f;
			float yrad = 0.f;
		};
		struct foot {
			bool LorR;
			int id;
			VECTOR_ref pos;
		};
		struct armers {
			size_t mesh = 0;
			float thickness = 0.f;
		};
	public:
		bool isfloat;
		float down_in_water;
		MV1 obj;
		std::vector<gun> gunframe;
		std::vector<foot> wheelframe;
		MV1 col;
		std::vector<gun> gunframe_col;
		std::array<int, 4> square;
		float flont_speed_limit;/*前進速度(km/h)*/
		float back_speed_limit;/*後退速度(km/h)*/
		float body_rad_limit;/*車体駆動速度(度/秒)*/
		float turret_rad_limit;/*砲塔駆動速度(度/秒)*/

		std::vector<armers> armer_mesh;	  /*装甲ID*/	/**/
		std::vector<size_t> space_mesh;	  /*装甲ID*/	/**/
		std::vector<size_t> module_mesh;	  /*装甲ID*/	/**/

		void into(const Tanks& t) {
			this->isfloat = t.isfloat;
			this->down_in_water = t.down_in_water;
			this->obj = t.obj.Duplicate();
			this->gunframe = t.gunframe;
			this->wheelframe = t.wheelframe;
			this->col = t.col.Duplicate();
			this->square = t.square;

			this->flont_speed_limit = t.flont_speed_limit;
			this->back_speed_limit = t.back_speed_limit;
			this->body_rad_limit = t.body_rad_limit;
			this->turret_rad_limit = t.turret_rad_limit;
			this->armer_mesh = t.armer_mesh;
			this->space_mesh = t.space_mesh;
			this->module_mesh = t.module_mesh;
			this->gunframe_col = t.gunframe_col;
		}
	};



	struct ammos {
		bool hit{ false };
		bool flug{ false };
		float cnt = 0.f;
		int color = 0;
		float speed = 0.f, pene = 0.f;
		float yadd;
		VECTOR_ref pos, repos, vec;
	};
	struct Chara {
		size_t id;
		size_t useid;
		Tanks usetank;

		float wheel_Left = 0.f;
		float wheel_Right = 0.f;

		VECTOR_ref pos, add, zvec, yvec, yvect;
		float yrad = 0.f;
		float xradp = 0.f;
		float xrad = 0.f;
		float zradp = 0.f;
		float zrad = 0.f;

		float xradp_shot = 0.f;
		float xrad_shot = 0.f;
		float zradp_shot = 0.f;
		float zrad_shot = 0.f;

		float yadd_left;
		float yadd_right;
		float zadd_flont;
		float zadd_back;
		std::array<bool, 6> key;

		std::array<EffectS, efs_user> effcs; /*effect*/
		std::vector<EffectS> gndsmkeffcs;    /*effect*/
		std::vector<float> gndsmksize;       /*effect*/

		struct Guns {			       /**/
			std::array<ammos, 32> Ammo; /*確保する弾*/
			float loadcnt{ 0 };	      /*装てんカウンター*/
			size_t useammo{};	      /*使用弾*/
			float fired{ 0.f };	    /*駐退*/
		};				       /**/
		std::vector<Guns> Gun;	    /*銃、砲全般*/

		float view_yrad;
		float view_xrad;
		//====================================================
		std::vector<pair> hitssort;	  /*当たった順番*/
		int hitbuf;			     /*使用弾痕*/
		std::vector<MV1_COLL_RESULT_POLY> hitres; /*確保*/
		std::vector<int16_t> HP;	     /*ライフ*/
		bool hitadd{ false };		     /*命中フラグ*/
		size_t hitid{ 0 };			     /*あてた敵*/
		int recorad{ 0 };		      /*弾き角度*/
		VECTOR_ref recovec;		      /*弾きベクトル*/
		MATRIX ps_m;	      /*車体行列*/
		//弾痕
		struct Hit {
			bool flug{ false }; /**/
			int use{ 0 };       /*使用フレーム*/
			MV1 pic; /*弾痕モデル*/
			VECTOR_ref scale, pos, zvec, yvec;	/*座標*/
		};
		std::array<Hit, 3> hit;		     /*弾痕*/
		float recorange{ 0 };				/*弾きの強さ*/
	};
	//
	static bool get_reco(Chara& play, std::vector<Chara>& tgts, ammos& c, size_t gun_s) {
		//
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
					t.hitres[m.mesh] = t.usetank.col.CollCheck_Line(c.repos, c.pos, -1, int(m.mesh));
					if (t.hitres[m.mesh].HitFlag) {
						t.hitssort[m.mesh] = pair(m.mesh, (c.repos - t.hitres[m.mesh].HitPosition).size());
						is_hit = true;
					}
					else {
						t.hitssort[m.mesh] = pair(m.mesh, (std::numeric_limits<float>::max)());
					}
				}
			//当たってない
			if (!is_hit) {
				continue;
			}
			//当たり判定を近い順にソート
			std::sort(t.hitssort.begin(), t.hitssort.end(), [](const pair& x, const pair& y) { return x.second < y.second; });
			//主砲の判定
			if (gun_s == 0) {
				//ダメージ面に届くまで判定
				for (auto& tt : t.hitssort) {
					//装甲面に当たらなかったならスルー
					if (tt.second == (std::numeric_limits<float>::max)()) {
						break;
					}
					//装甲面に当たったのでhitnearに代入して終了
					for(auto& a : t.usetank.armer_mesh){
						if (tt.first == a.mesh) {
							set_effect(&play.effcs[ef_reco], t.hitres[tt.first].HitPosition, t.hitres[tt.first].Normal);
							hitnear = tt.first;
							//ダメージ面に当たった時に装甲値に勝てるかどうか
							if (hitnear.has_value()) {
								const auto k = hitnear.value();
								if (c.pene > a.thickness * (1.0f / abs(c.vec.Norm() % t.hitres[k].Normal))) {
									//貫通
									play.hitadd = true;
									play.hitid = t.id;
									t.HP[0] = std::max<int16_t>(t.HP[0] - 1, 0); //
									//撃破時エフェクト
									if (t.HP[0] == 0) {
										//set_effect(&t.effcs[ef_bomb], t.usetank.obj.frame(t.ptr->engineframe), VGet(0, 0, 0));
									}
									//弾処理
									c.flug = false;
									//弾痕
									t.hit[t.hitbuf].use = 0;
								}
								else {
									//はじく
									//弾処理
									c.vec += VScale(t.hitres[k].Normal, (c.vec % t.hitres[k].Normal) * -2.0f);
									c.vec = c.vec.Norm();
									c.pos = c.vec.Scale(0.1f) + t.hitres[k].HitPosition;
									c.pene /= 2.0f;
									//弾痕
									t.hit[t.hitbuf].use = 1;
								}
								//弾痕のセット
								{
									float asize = /*play.ptr->gun_[gun_s].ammosize*/0.105f * 100.f;
									t.hit[t.hitbuf].scale = VGet(asize / abs(c.vec.Norm() % t.hitres[k].Normal), asize, asize);
									t.hit[t.hitbuf].pos = VTransform(t.hitres[k].HitPosition, MInverse(t.ps_m));
									t.hit[t.hitbuf].zvec = VTransform(((VECTOR_ref(t.hitres[k].Normal)) + t.hitres[k].HitPosition).get(), MInverse(t.ps_m));
									t.hit[t.hitbuf].yvec = VTransform(((VECTOR_ref(t.hitres[k].Normal) * c.vec) + t.hitres[k].HitPosition).get(), MInverse(t.ps_m));
									t.hit[t.hitbuf].flug = true;
									++t.hitbuf %= 3;
								}
								//被弾反動
								if (t.recorad == 180) {
									float rad = atan2(t.hitres[k].HitPosition.x - t.pos.x(), t.hitres[k].HitPosition.z - t.pos.z());
									t.recovec = VGet(cos(rad), 0, -sin(rad));
									t.recorad = 0;
									t.recorange = 3.f;
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
							set_effect(&play.effcs[ef_reco], t.hitres[tt.first].HitPosition, t.hitres[tt.first].Normal);
							t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 0, 0); //
							c.pene /= 2.0f;
						}
					}
					for (auto& a : t.usetank.module_mesh) {
						if (tt.first == a) {
							set_effect(&play.effcs[ef_reco], t.hitres[tt.first].HitPosition, t.hitres[tt.first].Normal);
							t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 0, 0); //
							c.pene /= 2.0f;
						}
					}
				}
			}
			//同軸機銃
			else {
				//
				if (t.hitssort.begin()->second == (std::numeric_limits<float>::max)()) {
					continue;
				}
				//至近で弾かせる
				hitnear = t.hitssort.begin()->first;
				if (hitnear.has_value()) {
					set_effect(&play.effcs[ef_reco2], t.hitres[hitnear.value()].HitPosition, t.hitres[hitnear.value()].Normal);

					c.vec = c.vec + VScale(t.hitres[hitnear.value()].Normal, (c.vec % t.hitres[hitnear.value()].Normal) * -2.0f);
					c.pos = c.vec.Scale(0.1f) + t.hitres[hitnear.value()].HitPosition;

					if (hitnear.value() >= 5 && hitnear.value() < t.HP.size()) {
						t.HP[hitnear.value()] = std::max<int16_t>(t.HP[hitnear.value()] - 1, 0); //
					}
				}
			}
			if (hitnear.has_value())
				break;
		}
		return (hitnear.has_value());
	}
};


#define NOMINMAX
#include "sub.hpp"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	auto Drawparts = std::make_unique<DXDraw>("TankFlanker"); /*汎用クラス*/
	auto UIparts = std::make_unique<UI>();/*UI*/
	auto Debugparts = std::make_unique<DeBuG>(60); /*汎用クラス*/
	auto Hostpassparts = std::make_unique<HostPassEffect>();/*UI*/
	//waord
	auto world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f)); /* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
	//カメラ
	float eye_xrad, eye_yrad;
	VECTOR_ref campos, camvec, camup;
	VECTOR_ref eyevec;
	VECTOR_ref vec_a;
	VECTOR_ref aimpos, aimpos2, aimposout;
	VECTOR_ref aimingpos;
	//スクリーンハンドル
	GraphHandle SkyScreen = GraphHandle::Make(dispx, dispy);
	GraphHandle FarScreen = GraphHandle::Make(dispx, dispy, true);
	GraphHandle MainScreen = GraphHandle::Make(dispx, dispy, true);
	GraphHandle NearScreen = GraphHandle::Make(dispx, dispy, true);
	GraphHandle BufScreen = GraphHandle::Make(dispx, dispy);
	//地面
	MV1 map, map_col;
	//空母
	MV1 carrier;
	MV1 carrier_col;
	VECTOR_ref car_pos = VGet(0.f, 0.f, -1500.f);
	float car_yrad = 0.f;
	VECTOR_ref car_pos_add;
	float car_yrad_add = 0.f;
	//空
	MV1 sky;
	//海
	VERTEX3D Vertex[6];
	{
		// 左上の頂点の情報をセット
		Vertex[0].pos = VGet(-10000.f, 0.f, -10000.f);
		Vertex[0].dif = GetColorU8(0, 192, 255, 245);
		// 右上の頂点の情報をセット
		Vertex[1].pos = VGet(10000.f, 0.f, -10000.f);
		Vertex[1].dif = GetColorU8(0, 192, 255, 245);
		// 左下の頂点の情報をセット
		Vertex[2].pos = VGet(-10000.f, 0.f, 10000.f);
		Vertex[2].dif = GetColorU8(0, 192, 255, 245);
		// 右下の頂点の情報をセット
		Vertex[3].pos = VGet(10000.f, 0.f, 10000.f);
		Vertex[3].dif = GetColorU8(0, 192, 255, 245);
		//
		Vertex[4] = Vertex[2];
		Vertex[5] = Vertex[1];
	}
	//弾痕
	MV1 hit_pic;
	MV1::Load("data/hit/model.mv1", &hit_pic);
	//飛行機エフェクト
	MV1 plane_effect;
	MV1::Load("data/plane_effect/model.mv1", &plane_effect);
	//操作
	bool ads = false;
	bool locktrt = false;
	int Rot = 0;
	float ratio = 1.f;
	float range=0.f;
	float range_p = 0.f;
	//データ//=========================================
	//弾薬
	std::vector<hit::Ammos> Ammos;
	hit::set_ammos(&Ammos);
	//車輛
	std::vector<hit::Tanks> tank;
	find_folders("data/tank/*", &tank);
	hit::set_tanks(&tank);
	//飛行機
	std::vector<hit::Planes> plane;
	find_folders("data/plane/*", &plane);
	hit::set_planes(&plane);
	//===============================================
	//キャラ
	std::vector<hit::Chara> chara;
	//空母
	MV1::Load("data/carrier/model.mv1", &carrier);
	std::vector < hit::frames > wire;
	std::vector<hit::frames> catapult;
	for (int i = 0; i < carrier.frame_num(); i++) {
		std::string p = carrier.frame_name(i);
		if (p.find("ﾜｲﾔｰ", 0) != std::string::npos) {
			wire.resize(wire.size() + 1);
			wire.back().first = i;
			wire.back().second = carrier.frame(wire.back().first);
		}
		else if (p.find("ｶﾀﾊﾟﾙﾄ", 0) != std::string::npos) {
			catapult.resize(catapult.size() + 1);
			catapult.back().first = i;
			catapult.back().second = carrier.frame(catapult.back().first + 2) - carrier.frame(catapult.back().first);
		}
	}
	carrier.SetMatrix(RotY(car_yrad)*car_pos.Mtrans());
	MV1::Load("data/carrier/col.mv1", &carrier_col);
	carrier_col.SetMatrix(RotY(car_yrad)*car_pos.Mtrans());
	carrier_col.SetupCollInfo(32,32,32);
	//map
	MV1::Load("data/map/model.mv1", &map);	//map
	{
		map.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
		Drawparts->Set_Shadow(14, VGet(100.f, 100.f, 100.f), map.mesh_minpos(0), VGet(0.0f, -0.5f, 0.5f), [&map, &carrier] {
			map.DrawModel();
			carrier.DrawModel();
		});
	}
	//col
	MV1::Load("data/map/col.mv1", &map_col);//mapコリジョン
	{
		VECTOR_ref size;
		for (int i = 0; i < map_col.mesh_num(); i++) {
			VECTOR_ref sizetmp = map_col.mesh_maxpos(i) - map_col.mesh_minpos(i);
			if (size.x() < sizetmp.x()) { size.x(sizetmp.x()); }
			if (size.y() < sizetmp.y()) { size.y(sizetmp.y()); }
			if (size.z() < sizetmp.z()) { size.z(sizetmp.z()); }
		}
		for (int i = 0; i < map_col.mesh_num(); i++) {
			map_col.SetupCollInfo(int(size.x() / 10.f), int(size.y() / 10.f), int(size.z() / 10.f), 0, i);
		}
	}
	MV1::Load("data/sky/model.mv1", &sky);	//空
	//ココから繰り返し読み込み//-------------------------------------------------------------------
	{
		chara.resize(chara.size() + 1);
		//戦車
		chara.back().vehicle[0].use_id = 1;
		chara.back().vehicle[0].pos = VGet(0.f, 1.81f, -2.48f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		//飛行機
		chara.back().vehicle[1].use_id = 0;
		auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
		chara.back().vehicle[1].mat = RotY(atan2f(-pp.x(), -pp.z()));
	}
	for (int i = 0; i < 10; i++) {
		chara.resize(chara.size()+1);
		//戦車
		chara.back().vehicle[0].use_id = 0;
		chara.back().vehicle[0].pos = VGet(10.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		//飛行機
		chara.back().vehicle[1].use_id = 0;
		auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
		chara.back().vehicle[1].mat = RotY(atan2f(-pp.x(), -pp.z()));
	}
	for (int i = 0; i < 10; i++) {
		chara.resize(chara.size() + 1);
		//戦車
		chara.back().vehicle[0].use_id = 1;
		chara.back().vehicle[0].pos = VGet(0.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		//飛行機
		chara.back().vehicle[1].use_id = 0;
		auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
		chara.back().vehicle[1].mat = RotY(atan2f(-pp.x(), -pp.z()));
	}
	for (int i = 0; i < 10; i++) {
		chara.resize(chara.size() + 1);
		//戦車
		chara.back().vehicle[0].use_id = 2;
		chara.back().vehicle[0].pos = VGet(-10.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		//飛行機
		chara.back().vehicle[1].use_id = 0;
		auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
		chara.back().vehicle[1].mat = RotY(atan2f(-pp.x(), -pp.z()));
	}
	for (int i = 0; i < 10; i++) {
		chara.resize(chara.size() + 1);
		//戦車
		chara.back().vehicle[0].use_id = 3;
		chara.back().vehicle[0].pos = VGet(-20.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		//飛行機
		chara.back().vehicle[1].use_id = 0;
		auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
		chara.back().vehicle[1].mat = RotY(atan2f(-pp.x(), -pp.z()));
	}
	//
	fill_id(chara);
	for (auto& c : chara) {
		//操作
		std::fill(c.key.begin(), c.key.end(), false);
		//戦車
		{
			c.vehicle[0].use_id = std::clamp<size_t>(c.vehicle[0].use_id, 0, tank.size() - 1);
			c.usetank.into(tank[c.vehicle[0].use_id]);//データ代入
			//転輪
			c.wheel_normal = VGet(0.f, 1.f, 0.f);//転輪の法線ズ
			//砲
			{
				c.vehicle[0].Gun_.resize(c.usetank.gunframe.size());
				for (int i = 0; i < c.vehicle[0].Gun_.size(); i++) {
					if (c.vehicle[0].Gun_.size() > i) {
						c.vehicle[0].Gun_[i].gun_info = c.usetank.gunframe[i];
						c.vehicle[0].Gun_[i].loadcnt_all = c.vehicle[0].Gun_[i].gun_info.load_time;
						//使用砲弾
						c.vehicle[0].Gun_[i].Spec.resize(c.vehicle[0].Gun_[i].Spec.size() + 1);
						for (auto& pa : Ammos) {
							if (pa.name.find(c.vehicle[0].Gun_[i].gun_info.useammo[0]) != std::string::npos) {
								c.vehicle[0].Gun_[i].Spec.back() = pa;
								break;
							}
						}
						//主砲
						for (auto& p : c.vehicle[0].Gun_[i].bullet) {
							p.color = Drawparts->GetColor(255, 255, 172);
							p.spec = c.vehicle[0].Gun_[i].Spec[0];
						}
					}
				}
			}
			//コリジョン
			for (int i = 0; i < c.usetank.col.mesh_num(); i++) {
				c.usetank.col.SetPosition(VGet(0.f, -100.f, 0.f));
				c.usetank.col.SetupCollInfo(8, 8, 8, -1, i);
			}
			c.useplane.col.SetPosition(VGet(0.f, -100.f, 0.f));
			c.useplane.col.SetupCollInfo(8, 8, 8);

			//モジュールごとの当たり判定
			c.hitssort.resize(c.usetank.col.mesh_num());
			//当たり判定を置いておく
			c.hitres.resize(c.usetank.col.mesh_num());
			//ヒットポイント
			c.HP.resize(c.usetank.col.mesh_num());
			//弾痕
			for (auto& h : c.hit) {
				h.flug = false;
				h.pic = hit_pic.Duplicate();
				h.use = 0;
				h.scale = VGet(1.f, 1.f, 1.f);
				h.pos = VGet(1.f, 1.f, 1.f);
			}
			//
			fill_id(c.effcs);
		}
		//飛行機
		{
			c.vehicle[1].use_id = std::clamp<size_t>(c.vehicle[1].use_id, 0, plane.size() - 1);
			c.useplane.into(plane[c.vehicle[1].use_id]);
			c.changegear = true;
			c.landing = false;
			{
				MV1AttachAnim(c.useplane.obj.get(), 0);//ダミー
				c.p_anime_geardown.first = MV1AttachAnim(c.useplane.obj.get(), 1);
				c.p_anime_geardown.second = 1.f;
				MV1SetAttachAnimBlendRate(c.useplane.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
				//舵
				for (int i = 0; i < c.p_animes_rudder.size(); i++) {
					c.p_animes_rudder[i].first = MV1AttachAnim(c.useplane.obj.get(), 2 + i);
					c.p_animes_rudder[i].second = 0.f;
					MV1SetAttachAnimBlendRate(c.useplane.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
				}
			}
			//砲
			{
				c.vehicle[1].Gun_.resize(c.useplane.gunframe.size());
				for (int i = 0; i < c.vehicle[1].Gun_.size(); i++) {
					if (c.vehicle[1].Gun_.size() > i) {
						//主砲
						c.vehicle[1].Gun_[i].gun_info = c.useplane.gunframe[i];
						c.vehicle[1].Gun_[i].loadcnt_all = c.vehicle[1].Gun_[i].gun_info.load_time;
						c.vehicle[1].Gun_[i].Spec.resize(c.vehicle[1].Gun_[i].Spec.size() + 1);
						for (auto& pa : Ammos) {
							if (pa.name.find(c.vehicle[1].Gun_[i].gun_info.useammo[0]) != std::string::npos) {
								c.vehicle[1].Gun_[i].Spec.back() = pa;
								break;
							}
						}
						for (auto& a : c.vehicle[1].Gun_[i].bullet) {
							a.color = Drawparts->GetColor(255, 255, 172);
							a.spec = c.vehicle[1].Gun_[i].Spec[0];
						}
					}
				}
			}
			//エフェクト
			{
				//plane_effect
				for (auto& be : c.useplane.burner) {
					c.p_burner.resize(c.p_burner.size() + 1);
					c.p_burner.back().frame = be;
					c.p_burner.back().effectobj = plane_effect.Duplicate();
				}
			}
		}
		carrier.SetFrameLocalMatrix(catapult[0].first + 2, RotX(deg2rad(-75))*catapult[0].second.Mtrans());

		c.vehicle[1].xradadd_right = 0.f;
		c.vehicle[1].xradadd_left = 0.f;
		c.vehicle[1].yradadd_left = 0.f;
		c.vehicle[1].yradadd_right = 0.f;
		c.vehicle[1].zradadd_right = 0.f;
		c.vehicle[1].zradadd_left = 0.f;
		c.vehicle[1].speed_add = 0.f;
		c.vehicle[1].speed_sub = 0.f;
		c.vehicle[1].speed = 0.f;
		c.vehicle[1].add = VGet(0.f, 0.f, 0.f);

		c.flight =
			//true;
			false;
	}
	for (auto& c : chara) {
		for (int i = 0; i < c.usetank.obj.material_num(); ++i) {
			MV1SetMaterialSpcColor(c.usetank.obj.get(), i, GetColorF(0.85f, 0.82f, 0.78f, 0.1f));
			MV1SetMaterialSpcPower(c.usetank.obj.get(), i, 50.0f);
		}
	}
	//物理set
	for (auto& c : chara) {
		b2PolygonShape dynamicBox; /*ダイナミックボディに別のボックスシェイプを定義します。*/
		dynamicBox.SetAsBox((c.usetank.maxpos.x() - c.usetank.minpos.x()) / 2, (c.usetank.maxpos.z() - c.usetank.minpos.z()) / 2, b2Vec2((c.usetank.minpos.x() + c.usetank.maxpos.x()) / 2, (c.usetank.minpos.z() + c.usetank.maxpos.z()) / 2), 0.f);
		b2FixtureDef fixtureDef;				    /*動的ボディフィクスチャを定義します*/
		fixtureDef.shape = &dynamicBox;				    /**/
		fixtureDef.density = 1.0f;				    /*ボックス密度をゼロ以外に設定すると、動的になります*/
		fixtureDef.friction = 0.3f;				    /*デフォルトの摩擦をオーバーライドします*/
		b2BodyDef bodyDef;					    /*ダイナミックボディを定義します。その位置を設定し、ボディファクトリを呼び出します*/
		bodyDef.type = b2_dynamicBody;				    /**/
		bodyDef.position.Set(c.vehicle[0].pos.x(), c.vehicle[0].pos.z());	   /**/
		bodyDef.angle = c.vehicle[0].yrad;					    /**/
		c.mine.body.reset(world->CreateBody(&bodyDef));		    /**/
		c.mine.playerfix = c.mine.body->CreateFixture(&fixtureDef); /*シェイプをボディに追加します*/
		/* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
	}
	std::vector<hit::wallPats> wall;	//壁をセットする
	{
		MV1SetupReferenceMesh(map_col.get(), 0, FALSE);
		MV1_REF_POLYGONLIST p = MV1GetReferenceMesh(map_col.get(), 0, FALSE);

		for (int i = 0; i < p.PolygonNum; i++) {
			if (p.Polygons[i].MaterialIndex == 2) {
				wall.resize(wall.size() + 1);
				wall.back().pos[0] = p.Vertexs[p.Polygons[i].VIndex[0]].Position;
				wall.back().pos[1] = p.Vertexs[p.Polygons[i].VIndex[1]].Position;
				if (b2DistanceSquared(b2Vec2(wall.back().pos[0].x(), wall.back().pos[0].z()), b2Vec2(wall.back().pos[1].x(), wall.back().pos[1].z())) <= 0.005f*0.005f) {
					wall.pop_back();
				}

				wall.resize(wall.size() + 1);
				wall.back().pos[0] = p.Vertexs[p.Polygons[i].VIndex[1]].Position;
				wall.back().pos[1] = p.Vertexs[p.Polygons[i].VIndex[2]].Position;
				if (b2DistanceSquared(b2Vec2(wall.back().pos[0].x(), wall.back().pos[0].z()), b2Vec2(wall.back().pos[1].x(), wall.back().pos[1].z())) <= 0.005f*0.005f) {
					wall.pop_back();
				}

				wall.resize(wall.size() + 1);
				wall.back().pos[0] = p.Vertexs[p.Polygons[i].VIndex[2]].Position;
				wall.back().pos[1] = p.Vertexs[p.Polygons[i].VIndex[0]].Position;
				if (b2DistanceSquared(b2Vec2(wall.back().pos[0].x(), wall.back().pos[0].z()), b2Vec2(wall.back().pos[1].x(), wall.back().pos[1].z())) <= 0.005f*0.005f) {
					wall.pop_back();
				}
			}
		}
	}
	for (auto&w : wall){
		// This a chain shape with isolated vertices
		std::array<b2Vec2, 2> vs;
		vs[0].Set(w.pos[0].x(), w.pos[0].z());
		vs[1].Set(w.pos[1].x(), w.pos[1].z());
		b2ChainShape chain;
		chain.CreateChain(&vs[0], 2);
		b2FixtureDef fixtureDef;				    /*動的ボディフィクスチャを定義します*/
		fixtureDef.shape = &chain;				    /**/
		fixtureDef.density = 1.0f;				    /*ボックス密度をゼロ以外に設定すると、動的になります*/
		fixtureDef.friction = 0.3f;				    /*デフォルトの摩擦をオーバーライドします*/
		b2BodyDef bodyDef;					    /*ダイナミックボディを定義します。その位置を設定し、ボディファクトリを呼び出します*/
		bodyDef.type = b2_staticBody;				    /**/
		bodyDef.position.Set(0, 0);		   /**/
		bodyDef.angle = 0.f;					    /**/
		w.b2.body.reset(world->CreateBody(&bodyDef));		    /**/
		w.b2.playerfix = w.b2.body->CreateFixture(&fixtureDef); /*シェイプをボディに追加します*/
	}
	//弾関数(仮)
	auto draw_bullets = [&chara, &campos](const unsigned int& color) {
		SetFogEnable(FALSE);
		SetUseLighting(FALSE);
		for (auto& c : chara) {
			for (auto& gns : c.vehicle) {
				for (auto& g : gns.Gun_) {
					for (auto& a : g.bullet) {
						if (a.flug) {
							SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f * std::clamp<float>((a.spec.speed - 100.f) / (200.f - 100.f), 0.f, 1.f)));
							DrawCapsule3D(a.pos.get(), a.repos.get(), ((a.spec.caliber - 0.00762f)*0.1f + 0.00762f) * ((a.pos - campos).size() / 24.f), 4, a.color, color, TRUE);//7.62mm用
						}
					}
				}
			}
		}
		SetUseLighting(TRUE);
		SetFogEnable(TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);			// 描画ブレンドモードを戻す
	};
	//射撃関数(仮)
	auto shoot = [](hit::Chara* c, const float& fps) {
		if (!c->flight) {//戦車
			for (int i = 0; i < c->vehicle[0].Gun_.size(); i++) {
				auto& cg = c->vehicle[0].Gun_[i];
				if (c->key[(i == 0) ? 0 : 1] && cg.loadcnt == 0) {
					auto& u = cg.bullet[cg.usebullet];
					++cg.usebullet %= cg.bullet.size();
					//ココだけ変化
					u.spec = cg.Spec[0];
					u.pos = c->usetank.obj.frame(cg.gun_info.frame2.first);
					u.vec = (c->usetank.obj.frame(cg.gun_info.frame3.first) - c->usetank.obj.frame(cg.gun_info.frame2.first)).Norm();
					//
					cg.loadcnt = cg.loadcnt_all;
					if (i == 0) {
						cg.fired = 1.f;
					}
					u.hit = false;
					u.flug = true;
					u.cnt = 0.f;
					u.yadd = 0.f;
					u.repos = u.pos;
					set_effect(&c->effcs[ef_fire], c->usetank.obj.frame(cg.gun_info.frame3.first), u.vec, u.spec.caliber / 0.1f);
				}
				cg.loadcnt = std::max(cg.loadcnt - 1.f / fps, 0.f);
				cg.fired = std::max(cg.fired - 1.f / fps, 0.f);
			}
		}
		else {//飛行機
			for (int i = 0; i < c->vehicle[1].Gun_.size(); i++) {
				auto& cg = c->vehicle[1].Gun_[i];
				if (c->key[(i == 0) ? 0 : 1] && cg.loadcnt == 0) {
					auto& u = cg.bullet[cg.usebullet];
					++cg.usebullet %= cg.bullet.size();
					//ココだけ変化
					u.spec = cg.Spec[0];
					u.pos = c->useplane.obj.frame(cg.gun_info.frame2.first);
					u.vec = (c->useplane.obj.frame(cg.gun_info.frame3.first) - c->useplane.obj.frame(cg.gun_info.frame2.first)).Norm();
					//
					cg.loadcnt = cg.loadcnt_all;
					cg.fired = 1.f;
					u.hit = false;
					u.flug = true;
					u.cnt = 0.f;
					u.yadd = 0.f;
					u.repos = u.pos;
					set_effect(&c->effcs[ef_fire], c->useplane.obj.frame(cg.gun_info.frame3.first), u.vec, u.spec.caliber / 0.1f);
				}
				cg.loadcnt = std::max(cg.loadcnt - 1.f / fps, 0.f);
				cg.fired = std::max(cg.fired - 1.f / fps, 0.f);
			}
		}
	};
	//必要な時に当たり判定をリフレッシュする(仮)
	auto ref_col = [&chara](const size_t& id, const VECTOR_ref& startpos, const VECTOR_ref& endpos,const float& distance) {
		for (auto& t : chara) {
			if (id == t.id || (Segment_Point_MinLength(startpos.get(), endpos.get(), t.vehicle[0].pos.get()) > distance)) {
				continue;
			}
			if (!t.vehicle[0].hit_check) {
				t.usetank.col.SetMatrix(t.vehicle[0].mat*t.vehicle[0].pos.Mtrans());
				for (int i = 0; i < t.usetank.col.mesh_num(); i++) {
					t.usetank.col.RefreshCollInfo(-1, i);
				}
				t.vehicle[0].hit_check = true;
			}
		}
		for (auto& t : chara) {
			if (id == t.id || (Segment_Point_MinLength(startpos.get(), endpos.get(), t.vehicle[1].pos.get()) > distance)) {
				continue;
			}
			if (!t.vehicle[1].hit_check) {
				t.useplane.col.SetMatrix(t.vehicle[1].mat*t.vehicle[1].pos.Mtrans());
				t.useplane.col.RefreshCollInfo();
				t.vehicle[1].hit_check = true;
			}
		}
	};
	//開始
	auto& mine = chara[0];
	ads = false;
	locktrt = false;
	Rot = 0;
	ratio = 1.f;
	range_p = 5.f;
	eye_xrad = 0.f;
	eye_yrad = -mine.vehicle[0].yrad;
	eyevec = VGet(cos(eye_xrad)*sin(eye_yrad), sin(eye_xrad), cos(eye_xrad)*cos(eye_yrad));
	vec_a = eyevec;
	campos = mine.vehicle[0].pos + VGet(0.f, 3.f, 0.f) + eyevec.Scale(range);
	SetMouseDispFlag(FALSE);
	SetMousePoint(dispx / 2, dispy / 2);
	for (auto& c : chara) {
		for (auto& g : c.usetank.wheelframe) {
			g.gndsmkeffcs.handle = Drawparts->get_gndhitHandle().Play3D();
			g.gndsmksize = 0.1f;
		}
		for (auto& g : c.useplane.wheelframe) {
			g.gndsmkeffcs.handle = Drawparts->get_gndhitHandle().Play3D();
			g.gndsmksize = 0.1f;
		}
		//c.effcs[ef_smoke2].handle = Drawparts->get_effHandle(ef_smoke2).Play3D();
		//c.effcs[ef_smoke3].handle = Drawparts->get_effHandle(ef_smoke2).Play3D();
	}
	while (ProcessMessage() == 0) {
		const auto fps = GetFPS();
		const auto waits = GetNowHiPerformanceCount();
		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) { break; }
		Debugparts->put_way();
		//当たり判定リフレッシュフラグ
		{
			for (auto& c : chara) {
				if (c.vehicle[0].hit_check) {
					c.usetank.col.SetPosition(VGet(0.f, -100.f, 0.f));
					for (int i = 0; i < c.usetank.col.mesh_num(); i++) {
						c.usetank.col.RefreshCollInfo(-1, i);
					}
					c.vehicle[0].hit_check = false;
				}
				if (c.vehicle[1].hit_check) {
					c.useplane.col.SetMatrix(VECTOR_ref(VGet(0.f, -100.f, 0.f)).Mtrans());
					c.vehicle[1].hit_check = false;
				}
			}
		}
		//空母移動
		{
			float spd = -(60.f / 3.6f) / fps;
			car_yrad_add = deg2rad(0.f) / fps;
			car_yrad += car_yrad_add;
			car_pos_add = VGet(spd*sin(car_yrad_add), 0.f, spd*cos(car_yrad_add));
			car_pos += car_pos_add;
			carrier_col.SetMatrix(RotY(car_yrad)*car_pos.Mtrans());
			carrier.SetMatrix(RotY(car_yrad)*car_pos.Mtrans());
			carrier_col.RefreshCollInfo();
		}
		//視点
		{
			//スコープ
			{
				if (!mine.flight) {
					Rot = std::clamp(Rot + GetMouseWheelRotVol(), 0, 7);

					switch (Rot) {
					case 2:
						range_p = 1.f;
						break;
					case 1:
						range_p = 7.5f;
						break;
					case 0:
						range_p = 15.f;
						break;
					}

					ratio = 1.f;
					for (int i = 3; i < Rot; i++) {
						ratio *= 5.f;
					}

				}
				else {
					Rot = std::clamp(Rot + GetMouseWheelRotVol(), 0, 2);
					switch (Rot) {
					case 2:
						range_p = 1.f;
						break;
					case 1:
						range_p = 15.f;
						break;
					case 0:
						range_p = 30.f;
						break;
					}

					ratio = 1.f;
				}
				auto oldads = ads;
				ads = (Rot >= 3);
				if (ads != oldads) {
					eyevec = vec_a;
					eye_xrad = std::asinf(eyevec.y());
					if (sin(eye_xrad) == 0.f) {
						eye_yrad = std::asinf(eyevec.x() / sin(eye_xrad));
					}
					else {
						eye_yrad = std::acosf(eyevec.z() / cos(eye_xrad));
					}
				}
				if (ads) {
					range_p = 1.f;
				}
				easing_set(&range, range_p, (ads) ? 0.f : 0.95f, fps);
			}
			//砲塔旋回
			{
				if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0 || mine.flight) {//砲塔ロック
					mine.view_yrad = 0.f;
					mine.view_xrad = 0.f;
				}
				else {
					//狙い
					if (ads) {
						vec_a = eyevec;
					}
					else {
						VECTOR_ref endpos = campos - eyevec.Scale(1000.f);
						//マップに当たったか
						for (int i = 0; i < map_col.mesh_num(); i++) {
							auto hp = map_col.CollCheck_Line(campos, endpos, 0, i);
							if (hp.HitFlag == TRUE) {
								endpos = hp.HitPosition;
							}
						}
						//車輛に当たったか
						for (auto& c : chara) {
							if (mine.id == c.id || (Segment_Point_MinLength(campos.get(), endpos.get(), c.vehicle[0].pos.get()) > 5.f)) {
								continue;
							}
							for (int i = 0; i < c.usetank.col.mesh_num(); i++) {
								const auto hp = c.usetank.col.CollCheck_Line(campos, endpos, -1, i);
								if (hp.HitFlag == TRUE) {
									endpos = hp.HitPosition;
								}
							}
						}
						vec_a = (mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first) - endpos).Norm();
					}
					{
						auto vec_z = mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame3.first) - mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first);
						float z_hyp = std::hypotf(vec_z.x(), vec_z.z());
						float a_hyp = std::hypotf(vec_a.x(), vec_a.z());
						float cost = (vec_a.z() * vec_z.x() - vec_a.x()*vec_z.z()) / (a_hyp * z_hyp);
						mine.view_yrad = (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f;//cos取得2D
						mine.view_xrad = (atan2f(-vec_z.y(), z_hyp) - atan2f(vec_a.y(), a_hyp)) / 5.f;
					}
				}
			}
			//キー
			{
				mine.key[0] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);//射撃
				mine.key[1] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0);//マシンガン
				mine.key[2] = (CheckHitKey(KEY_INPUT_W) != 0);
				mine.key[3] = (CheckHitKey(KEY_INPUT_S) != 0);
				mine.key[4] = (CheckHitKey(KEY_INPUT_D) != 0);
				mine.key[5] = (CheckHitKey(KEY_INPUT_A) != 0);
				//飛行時のみの操作
				if (mine.flight) {
					//ヨー
					mine.key[6] = (CheckHitKey(KEY_INPUT_Q) != 0);
					mine.key[7] = (CheckHitKey(KEY_INPUT_E) != 0);
					//スロットル
					mine.key[8] = (CheckHitKey(KEY_INPUT_R) != 0);
					mine.key[9] = (CheckHitKey(KEY_INPUT_F) != 0);
					//脚
					mine.key[10] = (CheckHitKey(KEY_INPUT_C) != 0);
					mine.key[11] = (CheckHitKey(KEY_INPUT_G) != 0);
					//精密操作
					mine.key[12] = (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
					//着艦フックスイッチ
					mine.key[13] = (CheckHitKey(KEY_INPUT_X) != 0);
					//カタパルト
					mine.key[14] = (CheckHitKey(KEY_INPUT_SPACE) != 0);
				}
				//飛行への移行
				{
					if (!mine.flight) {
						if (CheckHitKey(KEY_INPUT_P) != 0) {
							{
								auto tmp = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(mine.vehicle[1].mat);
								eye_xrad = atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z()));
								eye_yrad = atan2f(tmp.x(), tmp.z());
							}
							mine.vehicle[0].add = VGet(0.f, 0.f, 0.f);
							mine.flight = true;
						}
					}
				}
			}
			//マウスと視点角度をリンク
			{
				int mousex, mousey;
				GetMousePoint(&mousex, &mousey);
				SetMousePoint(dispx / 2, dispy / 2);
				if (ads) {
					eye_xrad = std::clamp(eye_xrad + deg2rad(float(mousey - dispy / 2)*0.1f / ratio), deg2rad(-20), deg2rad(10));
					//eye_xrad += deg2rad(float(mousey - dispy / 2)*0.1f);
					eye_yrad += deg2rad(float(mousex - dispx / 2)*0.1f / ratio);
				}
				else {
					eye_xrad = std::clamp(eye_xrad + deg2rad(float(mousey - dispy / 2)*0.1f), deg2rad(-25), deg2rad(89));
					//eye_xrad += deg2rad(float(mousey - dispy / 2)*0.1f);
					eye_yrad += deg2rad(float(mousex - dispx / 2)*0.1f);
				}
				eyevec = VGet(cos(eye_xrad)*sin(eye_yrad), sin(eye_xrad), cos(eye_xrad)*cos(eye_yrad));
			}
		}
		{
			//他のキー入力をここで取得(ホスト)
		}
		//反映
		for (auto& c : chara) {
			//戦車演算
			{
				VECTOR_ref yvec,zvec;
				//砲塔旋回
				{
					float limit = deg2rad(c.usetank.turret_rad_limit) / fps;
					c.vehicle[0].Gun_[0].gun_info.yrad += std::clamp(c.view_yrad, -limit, limit);//c.vehicle[0].Gun_[0].gun_info.yrad = std::clamp(c.vehicle[0].Gun_[0].gun_info.yrad + std::clamp(view_yrad / 5.f, -limit, limit),deg2rad(-30.0)+yrad,deg2rad(30.0)+yrad);//射界制限
					for (auto& g : c.vehicle[0].Gun_) {
						g.gun_info.xrad = std::clamp(g.gun_info.xrad + std::clamp(c.view_xrad, -limit, limit), deg2rad(-10), deg2rad(20));
					}
				}
				//反映
				for (auto& f : c.vehicle[0].Gun_) {
					c.usetank.obj.SetFrameLocalMatrix(f.gun_info.frame1.first, RotY(f.gun_info.yrad)*f.gun_info.frame1.second.Mtrans());
					c.usetank.col.SetFrameLocalMatrix(f.gun_info.frame1.first, RotY(f.gun_info.yrad)*f.gun_info.frame1.second.Mtrans());
					if (f.gun_info.frame2.first >= 0) {
						c.usetank.obj.SetFrameLocalMatrix(f.gun_info.frame2.first, RotX(f.gun_info.xrad)*f.gun_info.frame2.second.Mtrans());
						c.usetank.col.SetFrameLocalMatrix(f.gun_info.frame2.first, RotX(f.gun_info.xrad)*f.gun_info.frame2.second.Mtrans());
					}
					if (f.gun_info.frame3.first >= 0) {
						c.usetank.obj.SetFrameLocalMatrix(f.gun_info.frame3.first, VECTOR_ref(VGet(0.f, 0.f, f.fired*0.5f)).Mtrans()*f.gun_info.frame3.second.Mtrans());//リコイル
						c.usetank.col.SetFrameLocalMatrix(f.gun_info.frame3.first, VECTOR_ref(VGet(0.f, 0.f, f.fired*0.5f)).Mtrans()*f.gun_info.frame3.second.Mtrans());//リコイル
					}
				}
				//転輪
				{
					auto y_vec = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(c.vehicle[0].mat);
					for (auto& f : c.usetank.wheelframe) {
						MATRIX_ref tmp;
						c.usetank.obj.frame_reset(f.frame.first);
						auto startpos = c.usetank.obj.frame(f.frame.first);
						auto hp = map_col.CollCheck_Line(startpos + y_vec.Scale((-f.frame.second.y()) + 2.f), startpos + y_vec.Scale((-f.frame.second.y()) - 0.3f), 0, 0);
						if (hp.HitFlag == TRUE) {
							tmp = VECTOR_ref(VGet(0.f, hp.HitPosition.y + y_vec.y()*f.frame.second.y() - startpos.y(), 0.f)).Mtrans();
						}
						else {
							tmp = VECTOR_ref(VGet(0.f, -0.3f, 0.f)).Mtrans();
						}

						c.usetank.obj.SetFrameLocalMatrix(f.frame.first, RotX((f.frame.second.x() >= 0) ? c.wheel_Left : c.wheel_Right)*tmp*f.frame.second.Mtrans());
					}
				}
				easing_set(
					&c.wheel_normal,
					((c.usetank.obj.frame(c.usetank.square[0]) - c.usetank.obj.frame(c.usetank.square[3])) * (c.usetank.obj.frame(c.usetank.square[1]) - c.usetank.obj.frame(c.usetank.square[2]))).Norm(),
					0.95f, fps);
				//移動
				auto hp = map_col.CollCheck_Line(c.vehicle[0].pos + VGet(0.f, 2.f, 0.f), c.vehicle[0].pos - VGet(0.f, 0.1f, 0.f), 0, 0);
				auto isfloat = (c.vehicle[0].pos.y() == -c.usetank.down_in_water / 2.f);
				//Z、Yベクトル取得
				{
					zvec = VGet(sinf(c.vehicle[0].yrad), 0.f, cosf(c.vehicle[0].yrad));
					if (c.usetank.isfloat && isfloat) {
						yvec = VGet(0.f, 1.f, 0.f);
					}
					else {
						yvec = c.wheel_normal;
						zvec = zvec.Vtrans(MGetRotVec2(VGet(0.f, 1.f, 0.f), yvec.get()));
					}
				}
				if (hp.HitFlag == TRUE || (c.usetank.isfloat && isfloat)) {
					//前進後退
					{
						const auto old = c.vehicle[0].speed_add + c.vehicle[0].speed_sub;
						if (c.key[2] && !c.flight) {
							c.vehicle[0].speed_add = (c.vehicle[0].speed_add < (c.usetank.flont_speed_limit / 3.6f)) ? (c.vehicle[0].speed_add + (0.03f / 3.6f)*(420.f / fps)) : c.vehicle[0].speed_add;
							c.vehicle[0].speed_sub = (c.vehicle[0].speed_sub < 0.f) ? (c.vehicle[0].speed_sub + (0.1f / 3.6f)*(420.f / fps)) : c.vehicle[0].speed_sub;
						}
						if (c.key[3] && !c.flight) {
							c.vehicle[0].speed_sub = (c.vehicle[0].speed_sub > (c.usetank.back_speed_limit / 3.6f)) ? (c.vehicle[0].speed_sub - (0.03f / 3.6f)*(420.f / fps)) : c.vehicle[0].speed_sub;
							c.vehicle[0].speed_add = (c.vehicle[0].speed_add > 0.f) ? (c.vehicle[0].speed_add - (0.1f / 3.6f)*(420.f / fps)) : c.vehicle[0].speed_add;
						}
						if (!(c.key[2] && !c.flight) && !(c.key[3] && !c.flight)) {
							c.vehicle[0].speed_add = (c.vehicle[0].speed_add > 0.f) ? (c.vehicle[0].speed_add - (0.05f / 3.6f)*(420.f / fps)) : 0.f;
							c.vehicle[0].speed_sub = (c.vehicle[0].speed_sub < 0.f) ? (c.vehicle[0].speed_sub + (0.05f / 3.6f)*(420.f / fps)) : 0.f;
						}
						c.vehicle[0].speed = (old + ((c.vehicle[0].speed_add + c.vehicle[0].speed_sub) - old)*0.1f) / fps;
						c.vehicle[0].add = zvec.Scale(-c.vehicle[0].speed);
					}
					//旋回
					{
						c.vehicle[0].yradadd_left = (c.key[4] && !c.flight) ?
							std::max(c.vehicle[0].yradadd_left - deg2rad(0.5f*(420.f / fps)), deg2rad(-c.usetank.body_rad_limit)) :
							std::min(c.vehicle[0].yradadd_left + deg2rad(0.3f*(420.f / fps)), 0.f);
						c.vehicle[0].yradadd_right = (c.key[5] && !c.flight) ?
							std::min(c.vehicle[0].yradadd_right + deg2rad(0.5f*(420.f / fps)), deg2rad(c.usetank.body_rad_limit)) :
							std::max(c.vehicle[0].yradadd_right - deg2rad(0.3f*(420.f / fps)), 0.f);
						c.vehicle[0].yradadd = (c.vehicle[0].yradadd_left + c.vehicle[0].yradadd_right) / fps;
						c.vehicle[0].yrad += c.vehicle[0].yradadd;
					}
					//慣性
					{
						const auto xradold = c.vehicle[0].xradadd;
						c.vehicle[0].xradadd = deg2rad(-(c.vehicle[0].speed / (420.f / fps)) / ((0.1f / 3.6f) / fps)*30.f);
						easing_set(&c.vehicle[0].xrad, std::clamp(c.vehicle[0].xradadd - xradold, deg2rad(-15.f), deg2rad(15.f)), 0.995f, fps);
						c.vehicle[0].xrad = std::clamp(c.vehicle[0].xrad, deg2rad(-7.5f), deg2rad(7.5f));
						MATRIX_ref avm = (zvec*yvec).GetRotAxis(c.vehicle[0].xrad);
						yvec = yvec.Vtrans(avm);
						zvec = zvec.Vtrans(avm);
						auto zradold = c.vehicle[0].zradadd;
						c.vehicle[0].zradadd = deg2rad(-c.vehicle[0].yradadd / (deg2rad(5.f) / fps)*30.f);
						c.vehicle[0].zrad += ((c.vehicle[0].zradadd - zradold) - c.vehicle[0].zrad)*0.005f;
						MATRIX_ref bvm = zvec.GetRotAxis(c.vehicle[0].zrad);
						yvec = yvec.Vtrans(bvm);
						zvec = zvec.Vtrans(bvm);
					}
					if (hp.HitFlag == TRUE) {
						auto yp = c.vehicle[0].pos.y();
						easing_set(&yp, hp.HitPosition.y, 0.9f, fps);
						c.vehicle[0].pos.y(yp);
					}
				}
				else {
					c.vehicle[0].add.yadd(-9.8f / powf(fps, 2.f));
				}
				//射撃反動
				{
					easing_set(&c.xrad_shot, deg2rad(-c.vehicle[0].Gun_[0].fired*c.vehicle[0].Gun_[0].Spec[0].caliber*50.f)*cos(c.vehicle[0].Gun_[0].gun_info.yrad), 0.85f, fps);
					MATRIX_ref avm = (zvec*yvec).GetRotAxis(c.xrad_shot);
					easing_set(&c.zrad_shot, deg2rad(-c.vehicle[0].Gun_[0].fired*c.vehicle[0].Gun_[0].Spec[0].caliber*50.f)*sin(c.vehicle[0].Gun_[0].gun_info.yrad), 0.85f, fps);
					MATRIX_ref bvm = zvec.GetRotAxis(c.zrad_shot);

					yvec = yvec.Vtrans(avm*bvm);
					zvec = zvec.Vtrans(avm*bvm);
				}
				//行列
				c.vehicle[0].mat = Axis1(yvec*zvec, yvec, zvec);
				//浮く
				if (c.usetank.isfloat) {
					c.vehicle[0].pos.y(std::max(c.vehicle[0].pos.y(), -c.usetank.down_in_water / 2.f));
				}
				//転輪
				c.wheel_Left -= c.vehicle[0].speed *2.f - c.vehicle[0].yradadd * 5.f;
				c.wheel_Right -= c.vehicle[0].speed *2.f + c.vehicle[0].yradadd * 5.f;
				//射撃
				shoot(&c, fps);
			}
			{
				//飛行機
				float rad_spec = deg2rad(c.useplane.body_rad_limit*(c.useplane.mid_speed_limit / c.vehicle[1].speed));
				if (c.vehicle[1].speed < c.useplane.min_speed_limit) {
					rad_spec = deg2rad(c.useplane.body_rad_limit*(std::clamp(c.vehicle[1].speed, 0.f, c.useplane.min_speed_limit) / c.useplane.min_speed_limit));
				}
				//ピッチ
				easing_set(&c.vehicle[1].xradadd_right, ((c.key[2] && c.flight) ? -(c.key[12] ? rad_spec / 12.f : rad_spec / 4.f) : 0.f), 0.95f, fps);
				easing_set(&c.vehicle[1].xradadd_left , ((c.key[3] && c.flight) ?  (c.key[12] ? rad_spec / 12.f : rad_spec / 4.f) : 0.f), 0.95f, fps);
				//ロール
				easing_set(&c.vehicle[1].zradadd_right, ((c.key[4] && c.flight) ?  (c.key[12] ? rad_spec / 3.f : rad_spec) : 0.f), 0.95f, fps);
				easing_set(&c.vehicle[1].zradadd_left , ((c.key[5] && c.flight) ? -(c.key[12] ? rad_spec / 3.f : rad_spec) : 0.f), 0.95f, fps);
				//ヨー
				easing_set(&c.vehicle[1].yradadd_left , ((c.key[6] && c.flight) ? -(c.key[12] ? rad_spec / 24.f : rad_spec / 8.f) : 0.f), 0.95f, fps);
				easing_set(&c.vehicle[1].yradadd_right, ((c.key[7] && c.flight) ?  (c.key[12] ? rad_spec / 24.f : rad_spec / 8.f) : 0.f), 0.95f, fps);
				//スロットル
				easing_set(&c.vehicle[1].speed_add, (((c.key[8] && c.flight) && c.vehicle[1].speed < c.useplane.max_speed_limit) ? (0.5f / 3.6f) : 0.f), 0.95f, fps);
				easing_set(&c.vehicle[1].speed_sub, (c.key[9] && c.flight) ? ((c.vehicle[1].speed > c.useplane.min_speed_limit) ? (-0.5f / 3.6f) : ((c.vehicle[1].speed > 0.f) ? (-0.2f / 3.6f) : 0.f)) : 0.f, 0.95f, fps);
				//スピード
				c.vehicle[1].speed += (c.vehicle[1].speed_add + c.vehicle[1].speed_sub)*60.f / fps;
				{
					auto tmp = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(c.vehicle[1].mat);
					auto tmp2 = sin(atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z())));
					c.vehicle[1].speed += (((std::abs(tmp2) > sin(deg2rad(1.0f))) ? tmp2 * 0.5f : 0.f) / 3.6f)*60.f / fps;	//落下
				}
				//座標系反映
				{
					auto t_mat = c.vehicle[1].mat;
					c.vehicle[1].mat *= VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(t_mat).GetRotAxis((c.vehicle[1].xradadd_right + c.vehicle[1].xradadd_left) / fps);
					c.vehicle[1].mat *= VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(t_mat).GetRotAxis((c.vehicle[1].zradadd_right + c.vehicle[1].zradadd_left) / fps);
					c.vehicle[1].mat *= VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(t_mat).GetRotAxis((c.vehicle[1].yradadd_left + c.vehicle[1].yradadd_right) / fps);
				}
				//
				c.landing_cnt = std::min<uint8_t>(c.landing_cnt + 1, (c.key[13] && c.flight) ? 2 : 0);
				if (c.landing_cnt == 1) {
					c.landing ^= 1;
				}
				//脚
				c.changegear_cnt = std::min<uint8_t>(c.changegear_cnt + 1, (c.key[10] && c.flight) ? 2 : 0);
				if (c.changegear_cnt == 1) {
					c.changegear ^= 1;
				}
				easing_set(&c.p_anime_geardown.second, float(c.changegear), 0.95f, fps);
				MV1SetAttachAnimBlendRate(c.useplane.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
				//舵
				for (int i = 0; i < c.p_animes_rudder.size(); i++) {
					easing_set(&c.p_animes_rudder[i].second, float(c.key[i+2] && c.flight), 0.95f, fps);
					MV1SetAttachAnimBlendRate(c.useplane.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
				}
				//
				{
					//
					if (c.vehicle[1].speed >= c.useplane.min_speed_limit) {
						easing_set(&c.vehicle[1].add, VGet(0.f, 0.f, 0.f), 0.9f, fps);
					}
					else {
						c.vehicle[1].add.yadd(-9.8f / powf(fps, 2.f));
					}

					//着艦ワイヤ-処理
					{
						c.useplane.obj.frame_reset(c.useplane.hook.first);
						c.useplane.obj.SetFrameLocalMatrix(c.useplane.hook.first, RotX(deg2rad(c.p_landing_per))*c.useplane.hook.second.Mtrans());
						easing_set(&c.p_landing_per, (c.landing) ? 20.f : 0.f, 0.95f, fps);
						if (c.landing) {
							bool to = false;
							for (auto& wi : wire) {
								carrier.frame_reset(wi.first);
								if ((c.useplane.obj.frame(c.useplane.hook.first + 1) - carrier.frame(wi.first)).size() <= 30.f) {
									VECTOR_ref vec1 = (c.useplane.obj.frame(c.useplane.hook.first + 1) - c.useplane.obj.frame(c.useplane.hook.first)).Norm();
									VECTOR_ref vec2 = (carrier.frame(wi.first) - c.useplane.obj.frame(c.useplane.hook.first)).Norm();
									if (vec1%vec2 >= 0) {
										to = true;
										/*
										c.useplane.obj.SetFrameLocalMatrix(c.useplane.hook.first,
											(
												MATRIX_ref(MGetRotVec2(vec1.get(), vec2.get()))
												)
											*c.useplane.hook.second.Mtrans()
										);
										*/
										carrier.SetFrameLocalMatrix(wi.first,
											(
											(c.useplane.obj.frame(c.useplane.hook.first + 1) - carrier.frame(wi.first)).Mtrans()
												*c.vehicle[1].mat.Inverse()
												)
											*wi.second.Mtrans()
										);
										break;
									}
								}
							}
							if (to && c.vehicle[1].speed > 0.f) {
								c.vehicle[1].speed += -2.5f / 3.6f;
							}
						}
					}

					if (c.p_anime_geardown.second >= 0.5f) {
						for (auto& w : c.useplane.wheelframe) {
							easing_set(&w.gndsmksize, 0.f, 0.9f, fps);
							auto tmp = c.useplane.obj.frame(int(w.frame.first + 1));
							{
								auto hp = map_col.CollCheck_Line(tmp + VECTOR_ref(VGet(0.f, 0.98f, 0.f)).Vtrans(c.vehicle[1].mat), tmp, 0, 0);
								if (hp.HitFlag == TRUE) {
									c.vehicle[1].add = (VECTOR_ref(hp.HitPosition) - tmp);
									{
										auto x_vec = VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(c.vehicle[1].mat);
										auto y_vec = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(c.vehicle[1].mat);
										auto z_vec = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(c.vehicle[1].mat);

										auto y_vec2 = y_vec;
										easing_set(&y_vec2, hp.Normal, 0.95f, fps);
										auto normal = y_vec2;

										c.vehicle[1].mat = Axis1(
											x_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											y_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											z_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get()))
										);
									}
									w.gndsmksize = std::clamp(c.vehicle[1].speed*3.6f / 50.f, 0.1f, 1.f);
									if (c.vehicle[1].speed >= 0.f && (c.key[11] && c.flight)) {
										c.vehicle[1].speed += -0.5f / 3.6f;
									}
									if (c.vehicle[1].speed <= 0.f) {
										easing_set(&c.vehicle[1].speed, 0.f, 0.9f, fps);
									}
								}
							}
							{
								auto hp = carrier_col.CollCheck_Line(tmp + VECTOR_ref(VGet(0.f, 0.98f, 0.f)).Vtrans(c.vehicle[1].mat), tmp);
								if (hp.HitFlag == TRUE) {
									c.vehicle[1].add = (VECTOR_ref(hp.HitPosition) - tmp);
									{
										auto x_vec = VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(c.vehicle[1].mat);
										auto y_vec = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(c.vehicle[1].mat);
										auto z_vec = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(c.vehicle[1].mat);

										auto y_vec2 = y_vec;
										easing_set(&y_vec2, hp.Normal, 0.95f, fps);
										auto normal = y_vec2;

										c.vehicle[1].mat = Axis1(
											x_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											y_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											z_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get()))
										);

									}
									c.vehicle[1].add += car_pos_add + ((c.vehicle[1].pos - car_pos).Vtrans(RotY(car_yrad_add)) - (c.vehicle[1].pos - car_pos));
									c.vehicle[1].mat *= RotY(car_yrad_add);

									w.gndsmksize = std::clamp(c.vehicle[1].speed*3.6f / 50.f, 0.1f, 1.f);
									if (c.vehicle[1].speed >= 0.f && (c.key[11] && c.flight)) {
										c.vehicle[1].speed += -1.0f / 3.6f;
									}
									if (c.vehicle[1].speed <= 0.f) {
										easing_set(&c.vehicle[1].speed, 0.f, 0.9f, fps);
									}

									if (c.key[14]) {
										easing_set(&c.vehicle[1].speed, c.useplane.mid_speed_limit, 0.90f, fps);
									}
								}
							}
						}
					}
					else {
						for (auto& w : c.useplane.wheelframe) {
							easing_set(&w.gndsmksize, 0.f, 0.9f, fps);
						}
					}
					c.vehicle[1].pos += c.vehicle[1].add + VECTOR_ref(VGet(0.f, 0.f, -c.vehicle[1].speed / fps)).Vtrans(c.vehicle[1].mat);
				}
				//射撃
				shoot(&c, fps);
				//壁の当たり判定
				VECTOR_ref p_0 = c.vehicle[1].pos + VECTOR_ref(VGet(c.useplane.minpos.x(), 0.f, c.useplane.maxpos.z())).Vtrans(c.vehicle[1].mat);
				VECTOR_ref p_1 = c.vehicle[1].pos + VECTOR_ref(VGet(c.useplane.maxpos.x(), 0.f, c.useplane.maxpos.z())).Vtrans(c.vehicle[1].mat);
				VECTOR_ref p_2 = c.vehicle[1].pos + VECTOR_ref(VGet(c.useplane.maxpos.x(), 0.f, c.useplane.minpos.z())).Vtrans(c.vehicle[1].mat);
				VECTOR_ref p_3 = c.vehicle[1].pos + VECTOR_ref(VGet(c.useplane.minpos.x(), 0.f, c.useplane.minpos.z())).Vtrans(c.vehicle[1].mat);
				bool hitb = false;
				if (p_0.y() <= 0.f || p_1.y() <= 0.f || p_2.y() <= 0.f || p_3.y() <= 0.f) { hitb = true; }
				if (!hitb) {
					while (true) {
						if (carrier_col.CollCheck_Line(p_0, p_1).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (carrier_col.CollCheck_Line(p_1, p_2).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (carrier_col.CollCheck_Line(p_2, p_3).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (carrier_col.CollCheck_Line(p_3, p_0).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						break;
					}
				}
				if (!hitb) {
					for (int i = 0; i < map_col.mesh_num(); i++) {
						if (map_col.CollCheck_Line(p_0, p_1, 0, i).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (map_col.CollCheck_Line(p_1, p_2, 0, i).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (map_col.CollCheck_Line(p_2, p_3, 0, i).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (map_col.CollCheck_Line(p_3, p_0, 0, i).HitFlag == TRUE) {
							hitb = true;
							break;
						}
					}
				}
				if (hitb) {
					c.flight = false;

					carrier.SetFrameLocalMatrix(catapult[0].first + 2, RotX(deg2rad(-75))*catapult[0].second.Mtrans());
					auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
					c.vehicle[1].pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
					c.vehicle[1].mat = RotY(atan2f(-pp.x(),-pp.z()));

					c.vehicle[1].xradadd_right = 0.f;
					c.vehicle[1].xradadd_left = 0.f;
					c.vehicle[1].yradadd_left = 0.f;
					c.vehicle[1].yradadd_right = 0.f;
					c.vehicle[1].zradadd_right = 0.f;
					c.vehicle[1].zradadd_left = 0.f;
					c.vehicle[1].speed_add = 0.f;
					c.vehicle[1].speed_sub = 0.f;
					c.vehicle[1].speed = 0.f;// c.useplane.min_speed_limit;
					c.vehicle[1].add = VGet(0.f, 0.f, 0.f);
					c.landing = false;
				}
			}
			/*effect*/
			{
				for (auto& t : c.effcs) {
					if (t.id != ef_smoke1 && t.id != ef_smoke2 && t.id != ef_smoke3) {
						set_pos_effect(&t, Drawparts->get_effHandle(int(t.id)));
					}
				}
				for (int i = 0; i < c.effcs.size(); ++i) {
					if (i != ef_smoke1 && i != ef_smoke2 && i != ef_smoke3) {
						set_pos_effect(&c.effcs[i], Drawparts->get_effHandle(i));
					}
				}
				for (auto& t : c.usetank.wheelframe) {
					t.gndsmksize = 0.1f + std::abs(c.vehicle[0].speed) / ((c.usetank.flont_speed_limit / 3.6f) / fps)*0.6f;
					t.gndsmkeffcs.handle.SetPos(c.usetank.obj.frame(t.frame.first) + VECTOR_ref(VGet(0.f, -t.frame.second.y(), 0.f)).Vtrans(c.vehicle[0].mat));
					t.gndsmkeffcs.handle.SetScale(t.gndsmksize);
				}
				for (auto& t : c.useplane.wheelframe) {
					t.gndsmkeffcs.handle.SetPos(c.useplane.obj.frame(int(t.frame.first + 1)));
					t.gndsmkeffcs.handle.SetScale(t.gndsmksize);
				}

				//c.effcs[ef_smoke1].handle.SetPos(c.usetank.obj.frame(c.ptr->engineframe));
				//c.effcs[ef_smoke2].handle.SetPos(c.usetank.obj.frame(c.ptr->smokeframe[0]));
				//c.effcs[ef_smoke3].handle.SetPos(c.usetank.obj.frame(c.ptr->smokeframe[1]));
			}
		}
		//壁判定
		for (auto& c : chara) {
			c.mine.body->SetLinearVelocity(b2Vec2(c.vehicle[0].add.x(), c.vehicle[0].add.z()));
			c.mine.body->SetAngularVelocity(c.vehicle[0].yradadd);
		}
		world->Step(1.f, 1, 1);
		for (auto& c : chara) {
			c.vehicle[0].yrad = -c.mine.body->GetAngle();
			c.vehicle[0].pos.x(c.mine.body->GetPosition().x);
			c.vehicle[0].pos.z(c.mine.body->GetPosition().y);
			float spdrec = c.spd;
			easing_set(&c.spd, std::hypot(c.mine.body->GetLinearVelocity().x, c.mine.body->GetLinearVelocity().y) * ((c.spd > 0) ? 1.f : -1.f), 0.99f, fps);
			c.vehicle[0].speed = c.spd - spdrec;
		}
		//弾判定,弾痕
		for (auto& c : chara) {
			//弾判定
			for (auto& gns : c.vehicle) {
				for (auto& g : gns.Gun_) {
					for (auto& a : g.bullet) {
						float size = 3.f;
						for (int z = 0; z < int(size); z++) {
							if (a.flug) {
								a.repos = a.pos;
								a.pos += a.vec.Scale(a.spec.speed / fps / size);
								a.pos.yadd(a.yadd / size);
								ref_col(c.id, a.pos, a.repos, 5.f);
								//判定
								{
									bool ground_hit = false;
									VECTOR_ref normal;
									//戦車以外に当たる
									{
										{
											auto hps = carrier_col.CollCheck_Line(a.repos, a.pos);
											if (hps.HitFlag) {
												a.pos = hps.HitPosition;
												normal = hps.Normal;
												ground_hit = true;
											}
										}
										for (int i = 0; i < map_col.mesh_num(); i++) {
											auto hps = map_col.CollCheck_Line(a.repos, a.pos, 0, i);
											if (hps.HitFlag) {
												a.pos = hps.HitPosition;
												normal = hps.Normal;
												ground_hit = true;
											}
										}
										for (auto& t : chara) {
											if (t.vehicle[1].hit_check) {
												auto hps = t.useplane.col.CollCheck_Line(a.repos, a.pos, -1, -1);
												if (hps.HitFlag) {
													a.flug = false;
													if (a.spec.caliber >= 0.020f) {
														set_effect(&c.effcs[ef_reco], hps.HitPosition, hps.Normal);
													}
													else {
														set_effect(&c.effcs[ef_reco2], hps.HitPosition, hps.Normal);
													}
												}
											}
										}
									}
									//戦車に当たる
									switch (a.spec.type) {
									case 0://AP
										if (!hit::get_reco(c, chara, a)) {
											if (ground_hit) {
												if (a.spec.caliber >= 0.020f) {
													set_effect(&c.effcs[ef_gndhit], a.pos + normal.Scale(0.1f), normal);
												}
												else {
													set_effect(&c.effcs[ef_gndhit2], a.pos + normal.Scale(0.1f), normal);
												}
												if ((a.vec.Norm() % normal) <= cos(deg2rad(60))) {
													a.flug = false;
												}
												else {
													a.vec += normal.Scale((a.vec % normal) * -2.f);
													a.vec = a.vec.Norm();
													a.pos += a.vec.Scale(0.01f);
													a.spec.penetration /= 2.f;
												}
											}
										}
										if (a.flug) {
											a.spec.penetration -= 1.0f / fps / size;
											a.spec.speed -= 5.f / fps / size;
										}
										break;
									case 1://HE
										if (!hit::get_reco(c, chara, a)) {
											if (ground_hit) {
												if (a.spec.caliber >= 0.020f) {
													set_effect(&c.effcs[ef_gndhit], a.pos + normal.Scale(0.1f), normal);
												}
												else {
													set_effect(&c.effcs[ef_gndhit2], a.pos + normal.Scale(0.1f), normal);
												}
												a.flug = false;
											}
										}
										if (a.flug) {
											a.spec.speed -= 5.f / fps / size;
										}
										break;
									default:
										break;
									}
								}

								//消す(3秒たった、スピードが0以下、貫通が0以下)
								if (a.cnt >= 3.f || a.spec.speed < 100.f || a.spec.penetration <= 0.f) {
									a.flug = false;
								}
							}
						}
						a.yadd += -9.8f / powf(fps, 2.f);
						a.cnt += 1.f / fps;
					}
				}
			}
			//弾痕
			for (auto& h : c.hit) {
				if (h.flug) {
					auto y_vec = h.y_vec.Vtrans(c.vehicle[0].mat);
					auto z_vec = h.z_vec.Vtrans(c.vehicle[0].mat);

					h.pic.SetScale(h.scale);
					h.pic.SetRotationZYAxis(z_vec, y_vec, 0.f);
					h.pic.SetPosition(c.vehicle[0].pos + h.pos.Vtrans(c.vehicle[0].mat) + y_vec.Scale(0.005f));

					//h.pic.SetMatrix(Axis1((y_vec*z_vec), y_vec, z_vec, (c.vehicle[0].pos + h.pos.Vtrans(c.vehicle[0].mat) + y_vec.Scale(0.005f))) *MGetScale(h.scale.get()));
				}
			}
		}
		{
			//他の座標をここで出力(ホスト)
		}
		{
			//ホストからの座標をここで入力
		}
		//モデルに反映
		for (auto& c : chara) {
			c.usetank.obj.SetMatrix(c.vehicle[0].mat*c.vehicle[0].pos.Mtrans());
			c.useplane.obj.SetMatrix(c.vehicle[1].mat*c.vehicle[1].pos.Mtrans());
			for (auto& be : c.p_burner) {
				be.effectobj.SetMatrix(MATRIX_ref(MGetScale(VGet(1.f, 1.f, std::clamp(c.vehicle[1].speed / c.useplane.mid_speed_limit, 0.1f, 1.f))))*be.frame.second.Mtrans()  * c.vehicle[1].mat *c.vehicle[1].pos.Mtrans());
			}
		}
		//影用意
		Drawparts->Ready_Shadow(campos, [&map, &chara, &carrier] {
			carrier.DrawModel();
			for (auto& c : chara) {
				c.usetank.obj.DrawModel();
				c.useplane.obj.DrawModel();
			}
		});
		{
			if (ads) {
				campos = mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame1.first) + mine.vehicle[0].Gun_[0].gun_info.frame2.second.Vtrans(RotY(eye_yrad));
				camvec = campos - eyevec;
				camup = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(mine.vehicle[0].mat);
			}
			else {
				if (!mine.flight) {
					{
						camvec = mine.vehicle[0].pos + VGet(0.f, 3.f, 0.f);
						camvec.y(std::max(camvec.y(), 5.f));
					}
					{
						campos = camvec + eyevec.Scale(range);
						campos.y(std::max(campos.y(), 0.f));
						for (int i = 0; i < map_col.mesh_num(); i++) {
							auto hp = map_col.CollCheck_Line(camvec, campos, 0, i);
							if (hp.HitFlag == TRUE) {
								campos = camvec + (VECTOR_ref(hp.HitPosition) - camvec).Scale(0.9f);
							}
						}
						{
							auto hp = carrier_col.CollCheck_Line(camvec, campos);
							if (hp.HitFlag == TRUE) {
								campos = camvec + (VECTOR_ref(hp.HitPosition) - camvec).Scale(0.9f);
							}
						}					}
					{
						camup = VGet(0.f, 1.f, 0.f);
						camup = camup.Vtrans(VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(mine.vehicle[0].mat).GetRotAxis(mine.xrad_shot));
						camup = camup.Vtrans(VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(mine.vehicle[0].mat).GetRotAxis(mine.zrad_shot));
						camup = camup.Vtrans(VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(mine.vehicle[0].mat).GetRotAxis(mine.zrad_shot));
						camup = camup.Vtrans(VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(mine.vehicle[0].mat).GetRotAxis(mine.xrad_shot));
					}
				}
				else {
					camvec = mine.vehicle[1].pos + VECTOR_ref(VGet(0.f, 6.f, 0.f)).Vtrans(mine.vehicle[1].mat);
					camvec.y(std::max(camvec.y(), 5.f));

					if ((GetMouseInput() & MOUSE_INPUT_RIGHT) == 0) {
						eyevec = (camvec - aimpos2).Norm();
						campos = camvec + eyevec.Scale(range);
						camup = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(mine.vehicle[1].mat);

						eye_xrad = std::asinf(eyevec.y());
						if (sin(eye_xrad) == 0.f) {
							eye_yrad = std::asinf(eyevec.x() / sin(eye_xrad));
						}
						else {
							eye_yrad = std::acosf(eyevec.z() / cos(eye_xrad));
						}
					}
					else {
						campos = camvec + eyevec.Scale(range);
						campos.y(std::max(campos.y(), 0.f));
						for (int i = 0; i < map_col.mesh_num(); i++) {
							auto hp = map_col.CollCheck_Line(camvec, campos, 0, i);
							if (hp.HitFlag == TRUE) {
								campos = camvec + (VECTOR_ref(hp.HitPosition) - camvec).Scale(0.9f);
							}
						}
						{
							auto hp = carrier_col.CollCheck_Line(camvec, campos);
							if (hp.HitFlag == TRUE) {
								campos = camvec + (VECTOR_ref(hp.HitPosition) - camvec).Scale(0.9f);
							}
						}
						camup = VGet(0.f, 1.f, 0.f);
					}
				}
			}
		}
		float dist = std::clamp((campos - aimingpos).size(), 300.f, 1000.f);
		//被写体深度描画
		Hostpassparts->dof(
			&BufScreen,
			[&sky] {
				SetFogEnable(FALSE);
				SetUseLighting(FALSE);
				sky.DrawModel();
				SetUseLighting(TRUE);
				SetFogEnable(TRUE);
			},
			[&Drawparts, &map, &carrier, &chara, &ads, &Vertex, &draw_bullets] {
				Drawparts->Draw_by_Shadow([&map, &carrier, &chara, &ads, &Vertex] {
					SetFogStartEnd(0.0f, 3000.f);
					SetFogColor(128, 192, 255);
					{
						DrawPolygon3D(Vertex, 2, DX_NONE_GRAPH, TRUE);
					}
					SetFogStartEnd(0.0f, 3000.f);
					SetFogColor(128, 128, 128);
					{
						map.DrawModel();
						carrier.DrawModel();
						//戦車
						for (auto& t : chara) {
							if ((!ads && t.id == 0) || t.id != 0) {
								t.usetank.obj.DrawModel();
							}
							for (auto& h : t.hit) {
								if (h.flug) {
									h.pic.DrawFrame(h.use);//弾痕
								}
							}
						}
						//戦闘機
						for (auto& t : chara) {
							t.useplane.obj.DrawModel();
							for (auto& be : t.p_burner) {
								be.effectobj.DrawModel();//バーナー
							}
						}
					}
				});
				draw_bullets(Drawparts->GetColor(255, 255, 255));
			},
			campos, camvec, camup, deg2rad(90 / 2) / ratio,
			dist,
			mine.flight ? 10.f : (ads ? (1.5f + 198.5f*(dist - 300.f) / (1000.f - 300.f)) : 1.5f)
		);
		//
		Drawparts->SetDraw_Screen(DX_SCREEN_BACK);
		//照準座標取得
		{
			SetCameraNearFar(0.01f, 5000.0f);
			SetupCamera_Perspective(deg2rad(45) / ratio);
			SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
			if (mine.flight) {
				VECTOR_ref startpos = mine.vehicle[1].pos;
				VECTOR_ref tmppos = startpos + VECTOR_ref(VGet(0.f, 0.f, -1000.f)).Vtrans(mine.vehicle[1].mat);
				for (int i = 0; i < map_col.mesh_num(); i++) {
					auto hp = map_col.CollCheck_Line(startpos, tmppos, 0, i);
					if (hp.HitFlag == TRUE) {
						tmppos = hp.HitPosition;
					}
				}
				auto hp = carrier_col.CollCheck_Line(startpos, tmppos);
				if (hp.HitFlag == TRUE) {
					tmppos = hp.HitPosition;
				}
				easing_set(&aimpos2, tmppos, 0.9f, fps);
				aimposout = ConvWorldPosToScreenPos(aimpos2.get());
			}
			else {
				VECTOR_ref startpos = mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first);
				VECTOR_ref tmppos = startpos + (mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame3.first) - startpos).Norm().Scale(1000.f);
				{
					for (int i = 0; i < map_col.mesh_num(); i++) {
						auto hp = map_col.CollCheck_Line(startpos, tmppos, 0, i);
						if (hp.HitFlag == TRUE) {
							tmppos = hp.HitPosition;
						}
					}
					auto hp = carrier_col.CollCheck_Line(startpos, tmppos);
					if (hp.HitFlag == TRUE) {
						tmppos = hp.HitPosition;
					}
				}
				ref_col(mine.id, startpos, tmppos, 5.f);
				for (auto& t : chara) {
					if (t.vehicle[0].hit_check) {
						for (int i = 0; i < t.usetank.col.mesh_num(); i++) {
							const auto hp = t.usetank.col.CollCheck_Line(startpos, tmppos, -1, i);
							if (hp.HitFlag == TRUE) {
								tmppos = hp.HitPosition;
							}
						}
					}
					if (t.vehicle[1].hit_check) {
						const auto hp = t.useplane.col.CollCheck_Line(startpos, tmppos, -1, -1);
						if (hp.HitFlag == TRUE) {
							tmppos = hp.HitPosition;
						}
					}
				}
				easing_set(&aimpos, tmppos, 0.9f, fps);
				aimposout = ConvWorldPosToScreenPos(aimpos.get());
			}
			{
				aimingpos = campos + (camvec - campos).Norm().Scale(1000.f);
				{
					for (int i = 0; i < map_col.mesh_num(); i++) {
						auto hp = map_col.CollCheck_Line(campos, aimingpos, 0, i);
						if (hp.HitFlag == TRUE) {
							aimingpos = hp.HitPosition;
						}
					}
					auto hp = carrier_col.CollCheck_Line(campos, aimingpos);
					if (hp.HitFlag == TRUE) {
						aimingpos = hp.HitPosition;
					}
				}
			}
		}
		//描画
		{
			//背景
			BufScreen.DrawGraph(0, 0, false);
			//ブルーム
			Hostpassparts->bloom(BufScreen, (mine.flight) ? 255 : 64);
			/*
			for (auto& l : wall) {
				DrawLine3D(VGet(l.vehicle[0].pos.x(), 2.f, l.vehicle[0].pos.z()), VGet(l.pos[1].x(), 2.f, l.pos[1].z()), Drawparts->GetColor(255, 0, 0));
			}
			*/
			UIparts->draw(aimposout, mine, ads);
			Debugparts->end_way();
			Debugparts->debug(10, 10, fps, float(GetNowHiPerformanceCount() - waits) / 1000.f);
		}
		Drawparts->Screen_Flip(waits);
	}
	return 0; // ソフトの終了
}

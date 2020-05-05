#include "sub.hpp"
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	auto Drawparts    = std::make_unique<DXDraw>("TankFlanker", 60.f); /*汎用クラス*/
	auto UIparts      = std::make_unique<UI>();			   /*UI*/
	auto Debugparts   = std::make_unique<DeBuG>(60);		   /*デバッグ*/
	auto Hostpassparts= std::make_unique<HostPassEffect>();		   /*ホストパスエフェクト*/
	/**/
	auto world= std::make_unique<b2World>(b2Vec2(0.0f, 0.0f)); /* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
	//視点
	VECTOR_ref eyevec;
	//カメラ
	VECTOR_ref campos, camvec, camup;
	std::array<VECTOR_ref, 2> aimpos;
	VECTOR_ref aimposout;
	//スクリーンハンドル
	GraphHandle BufScreen= GraphHandle::Make(dispx, dispy);
	GraphHandle SkyScreen= GraphHandle::Make(dispx, dispy);
	//地面
	MV1 map, map_col;
	//木
	MV1 tree_model;
	//空母
	MV1 carrier, carrier_col;
	VECTOR_ref car_pos= VGet(0.f, 0.f, -1500.f);
	float car_yrad    = 0.f;
	VECTOR_ref car_pos_add;
	float car_yrad_add= 0.f;
	std::vector<hit::frames> wire;
	std::vector<hit::frames> catapult;
	//空
	MV1 sky;
	//海
	VERTEX3D Vertex[6];
	{
		// 左上の頂点の情報をセット
		Vertex[0].pos= VGet(-20000.f, 0.f, -20000.f);
		Vertex[0].dif= GetColorU8(0, 192, 255, 245);
		// 右上の頂点の情報をセット
		Vertex[1].pos= VGet(20000.f, 0.f, -20000.f);
		Vertex[1].dif= GetColorU8(0, 192, 255, 245);
		// 左下の頂点の情報をセット
		Vertex[2].pos= VGet(-20000.f, 0.f, 20000.f);
		Vertex[2].dif= GetColorU8(0, 192, 255, 245);
		// 右下の頂点の情報をセット
		Vertex[3].pos= VGet(20000.f, 0.f, 20000.f);
		Vertex[3].dif= GetColorU8(0, 192, 255, 245);
		//
		Vertex[4]= Vertex[2];
		Vertex[5]= Vertex[1];
	}
	//弾痕
	MV1 hit_pic;
	//飛行機エフェクト
	MV1 plane_effect;
	//操作
	float fov	     = deg2rad(90 / 2);
	bool ads	      = false;
	int Rot		      = 0;
	float ratio	   = 1.f;
	float range	   = 0.f;
	float range_p	 = 0.f;
	uint8_t change_vehicle= 0;
	//ロックオン
	hit::switchs lock_on= { false, uint8_t(0) };
	size_t tgt	  = 0;
	VECTOR_ref aimposout_lockon;
	float distance= 0.f;
	//キャラ
	std::vector<hit::Chara> chara;
	//データ//=========================================
	SetUseASyncLoadFlag(TRUE);
	//弾痕
	MV1::Load("data/hit/model.mv1", &hit_pic);
	//飛行機エフェクト
	MV1::Load("data/plane_effect/model.mv1", &plane_effect);
	//弾薬
	std::vector<hit::Ammos> Ammos;
	//車輛
	std::vector<hit::Tanks> tank;
	find_folders("data/tank/*", &tank);
	hit::set_tanks_pre(&tank);
	//飛行機
	std::vector<hit::Planes> plane;
	find_folders("data/plane/*", &plane);
	hit::set_planes_pre(&plane);
	SetUseASyncLoadFlag(FALSE);
	UIparts->load_window("車両モデル");
	hit::set_ammos(&Ammos);  //弾薬
	hit::set_tanks(&tank);   //車輛
	hit::set_planes(&plane); //飛行機
	//ココから繰り返し読み込み//-------------------------------------------------------------------
	chara.clear();
	chara.resize(1);
	//キャラ選択
	if (0) {
		UIparts->select_window(&chara[0], &tank, &plane);
	} else {
		chara.back().vehicle[0].use_id  = 0; //戦車
		chara.back().vehicle[1].use_id  = 0; //飛行機
		chara.back().vehicle[0].camo_sel= 0;
		chara.back().vehicle[1].camo_sel= 0;
	}
	//
	SetUseASyncLoadFlag(TRUE);
	//木
	MV1::Load("data/tree/model.mv1", &tree_model);
	MV1::Load("data/carrier/model.mv1", &carrier); //空母
	MV1::Load("data/map/model.mv1", &map);	 //map
	MV1::Load("data/map/col.mv1", &map_col);       //mapコリジョン
	MV1::Load("data/sky/model.mv1", &sky);	 //空
	SetUseASyncLoadFlag(FALSE);
	UIparts->load_window("マップモデル");
	//リソース読み込み後
	//空母
	{
		for (int i= 0; i < carrier.frame_num(); i++) {
			std::string p= carrier.frame_name(i);
			if (p.find("ﾜｲﾔｰ", 0) != std::string::npos) {
				wire.resize(wire.size() + 1);
				wire.back().first = i;
				wire.back().second= carrier.frame(wire.back().first);
			} else if (p.find("ｶﾀﾊﾟﾙﾄ", 0) != std::string::npos) {
				catapult.resize(catapult.size() + 1);
				catapult.back().first = i;
				catapult.back().second= carrier.frame(catapult.back().first + 2) - carrier.frame(catapult.back().first);
			}
		}
		carrier.SetMatrix(MATRIX_ref::RotY(car_yrad) * MATRIX_ref::Mtrans(car_pos));
		MV1::Load("data/carrier/col.mv1", &carrier_col);
		carrier_col.SetMatrix(MATRIX_ref::RotY(car_yrad) * MATRIX_ref::Mtrans(car_pos));
		carrier_col.SetupCollInfo(32, 32, 32);
	}
	//map
	{
		map.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
		Drawparts->Set_Shadow(14, VGet(100.f, 100.f, 100.f), map.mesh_minpos(0), VGet(0.0f, -0.5f, 0.5f), [&map, &carrier] {
			map.DrawModel();
			carrier.DrawModel();
		});
	}
	//col
	{
		VECTOR_ref size;
		for (int i= 0; i < map_col.mesh_num(); i++) {
			VECTOR_ref sizetmp= map_col.mesh_maxpos(i) - map_col.mesh_minpos(i);
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
		for (int i= 0; i < map_col.mesh_num(); i++) {
			map_col.SetupCollInfo(int(size.x() / 10.f), int(size.y() / 10.f), int(size.z() / 10.f), 0, i);
		}
	}
	//
	{
		//戦車
		chara.back().vehicle[0].pos= VGet(0.f, 1.81f, -2.48f);
		chara.back().vehicle[0].mat= MATRIX_ref::RotY(deg2rad(270.f));
		//飛行機
		auto pp			   = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos= carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm() * (6.f);
		chara.back().vehicle[1].mat= MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
	}
	for (int i= 0; i < 6; i++) {
		chara.resize(chara.size() + 1);
		//戦車
		chara.back().vehicle[0].use_id  = 0;
		chara.back().vehicle[0].pos     = VGet(10.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].mat     = MATRIX_ref::RotY(deg2rad(270.f));
		chara.back().vehicle[0].camo_sel= GetRand(5);
		//飛行機
		chara.back().vehicle[1].use_id  = 0;
		auto pp				= carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos     = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm() * (6.f);
		chara.back().vehicle[1].mat     = MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
		chara.back().vehicle[1].camo_sel= GetRand(5);
	}
	for (int i= 0; i < 6; i++) {
		chara.resize(chara.size() + 1);
		//戦車
		chara.back().vehicle[0].use_id  = 1;
		chara.back().vehicle[0].pos     = VGet(0.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].mat     = MATRIX_ref::RotY(deg2rad(270.f));
		chara.back().vehicle[0].camo_sel= GetRand(5);
		//飛行機
		chara.back().vehicle[1].use_id  = 0;
		auto pp				= carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos     = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm() * (6.f);
		chara.back().vehicle[1].mat     = MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
		chara.back().vehicle[1].camo_sel= GetRand(5);
	}
	for (int i= 0; i < 6; i++) {
		chara.resize(chara.size() + 1);
		//戦車
		chara.back().vehicle[0].use_id  = 2;
		chara.back().vehicle[0].pos     = VGet(-10.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].mat     = MATRIX_ref::RotY(deg2rad(270.f));
		chara.back().vehicle[0].camo_sel= GetRand(5);
		//飛行機
		chara.back().vehicle[1].use_id  = 0;
		auto pp				= carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos     = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm() * (6.f);
		chara.back().vehicle[1].mat     = MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
		chara.back().vehicle[1].camo_sel= GetRand(5);
	}
	for (int i= 0; i < 6; i++) {
		chara.resize(chara.size() + 1);
		//戦車
		chara.back().vehicle[0].use_id  = 3;
		chara.back().vehicle[0].pos     = VGet(-20.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].mat     = MATRIX_ref::RotY(deg2rad(270.f));
		chara.back().vehicle[0].camo_sel= GetRand(5);
		//飛行機
		chara.back().vehicle[1].use_id  = 0;
		auto pp				= carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos     = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm() * (6.f);
		chara.back().vehicle[1].mat     = MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
		chara.back().vehicle[1].camo_sel= GetRand(5);
	}
	fill_id(chara);
	for (auto& c : chara) {
		//操作
		std::fill(c.key.begin(), c.key.end(), false);
		//エフェクト
		fill_id(c.effcs);
		//戦車
		{
			auto& veh= c.vehicle[0];

			veh.use_id= std::clamp<size_t>(veh.use_id, 0, tank.size() - 1);
			c.usetank.into(tank[veh.use_id]); //データ代入
			veh.obj= tank[veh.use_id].obj.Duplicate();
			veh.col= tank[veh.use_id].col.Duplicate();
			//転輪
			c.wheel_normal= VGet(0.f, 1.f, 0.f); //転輪の法線ズ
			//砲
			{
				veh.Gun_.resize(c.usetank.gunframe.size());
				for (int i= 0; i < veh.Gun_.size(); i++) {
					auto& g   = veh.Gun_[i];
					g.gun_info= c.usetank.gunframe[i];
					g.rounds  = g.gun_info.rounds;
					//使用砲弾
					g.Spec.resize(g.Spec.size() + 1);
					for (auto& pa : Ammos) {
						if (pa.name.find(g.gun_info.useammo[0]) != std::string::npos) {
							g.Spec.back()= pa;
							break;
						}
					}
					for (auto& p : g.bullet) {
						p.color= GetColor(255, 255, 172);
						p.spec = g.Spec[0];
					}
				}
			}
			//コリジョン
			for (int i= 0; i < veh.col.mesh_num(); i++) {
				veh.col.SetPosition(VGet(0.f, -100.f, 0.f));
				veh.col.SetupCollInfo(8, 8, 8, -1, i);
			}
			//モジュールごとの当たり判定
			veh.hitssort.resize(veh.col.mesh_num());
			//当たり判定を置いておく
			veh.hitres.resize(veh.col.mesh_num());
			//ヒットポイント
			veh.HP= c.usetank.HP;
			veh.HP_m.resize(veh.col.mesh_num());
			for (auto& h : veh.HP_m) {
				h= 100;
			}
			//弾痕
			for (auto& h : veh.hit) {
				h.flug = false;
				h.pic  = hit_pic.Duplicate();
				h.use  = 0;
				h.scale= VGet(1.f, 1.f, 1.f);
				h.pos  = VGet(1.f, 1.f, 1.f);
			}
			//迷彩
			if (c.usetank.camog.size() > 0) {
				veh.camo_sel%= c.usetank.camog.size();
				MV1SetTextureGraphHandle(veh.obj.get(), c.usetank.camo_tex, c.usetank.camog[veh.camo_sel], FALSE);
			}
		}
		//飛行機
		{
			auto& veh= c.vehicle[1];

			veh.use_id= std::clamp<size_t>(veh.use_id, 0, plane.size() - 1);
			c.useplane.into(plane[veh.use_id]);
			veh.obj= plane[veh.use_id].obj.Duplicate();
			veh.col= plane[veh.use_id].col.Duplicate();

			c.changegear.first= true;
			c.landing.first   = false;
			{
				MV1AttachAnim(veh.obj.get(), 0); //ダミー
				c.p_anime_geardown.first = MV1AttachAnim(veh.obj.get(), 1);
				c.p_anime_geardown.second= 1.f;
				MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
				//舵
				for (int i= 0; i < c.p_animes_rudder.size(); i++) {
					c.p_animes_rudder[i].first = MV1AttachAnim(veh.obj.get(), 2 + i);
					c.p_animes_rudder[i].second= 0.f;
					MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
				}
			}
			//砲
			{
				veh.Gun_.resize(c.useplane.gunframe.size());
				for (int i= 0; i < veh.Gun_.size(); i++) {
					auto& g= veh.Gun_[i];
					//主砲
					g.gun_info= c.useplane.gunframe[i];
					g.rounds  = g.gun_info.rounds;

					g.Spec.resize(g.Spec.size() + 1);
					for (auto& pa : Ammos) {
						if (pa.name.find(g.gun_info.useammo[0]) != std::string::npos) {
							g.Spec.back()= pa;
							break;
						}
					}
					for (auto& a : g.bullet) {
						a.color= GetColor(255, 255, 172);
						a.spec = g.Spec[0];
					}
				}
			}
			//当たり判定を置いておく
			veh.hitres.resize(veh.col.mesh_num());
			//ヒットポイント
			veh.HP= c.useplane.HP;
			//モジュール耐久
			veh.HP_m.resize(veh.col.mesh_num());
			for (auto& h : veh.HP_m) {
				h= 100;
			}
			//コリジョン
			for (int i= 0; i < veh.col.mesh_num(); i++) {
				veh.col.SetupCollInfo(8, 8, 8, -1, i);
			}
			//エフェクト
			{
				//plane_effect
				for (auto& be : c.useplane.burner) {
					c.p_burner.resize(c.p_burner.size() + 1);
					c.p_burner.back().frame    = be;
					c.p_burner.back().effectobj= plane_effect.Duplicate();
				}
			}
			//モジュールごとの当たり判定
			veh.hitssort.resize(veh.col.mesh_num());
			//弾痕
			for (auto& h : veh.hit) {
				h.flug = false;
				h.pic  = hit_pic.Duplicate();
				h.use  = 0;
				h.scale= VGet(1.f, 1.f, 1.f);
				h.pos  = VGet(1.f, 1.f, 1.f);
			}
			//
			carrier.SetFrameLocalMatrix(catapult[0].first + 2, MATRIX_ref::RotX(deg2rad(-75)) * MATRIX_ref::Mtrans(catapult[0].second));
			veh.xradadd_right= 0.f;
			veh.xradadd_left = 0.f;
			veh.yradadd_left = 0.f;
			veh.yradadd_right= 0.f;
			veh.zradadd_right= 0.f;
			veh.zradadd_left = 0.f;
			veh.speed_add    = 0.f;
			veh.speed_sub    = 0.f;
			veh.speed	= 0.f;
			veh.add		 = VGet(0.f, 0.f, 0.f);

			//迷彩
			if (c.useplane.camog.size() > 0) {
				veh.camo_sel%= c.useplane.camog.size();
				MV1SetTextureGraphHandle(veh.obj.get(), c.useplane.camo_tex, c.useplane.camog[veh.camo_sel], FALSE);
			}
		}
		c.mode= 0;
	}
	for (auto& c : chara) {
		for (auto& veh : c.vehicle) {
			for (int i= 0; i < veh.obj.material_num(); ++i) {
				MV1SetMaterialSpcColor(veh.obj.get(), i, GetColorF(0.85f, 0.82f, 0.78f, 0.1f));
				MV1SetMaterialSpcPower(veh.obj.get(), i, 50.0f);
			}
		}
	}
	//物理set
	for (auto& c : chara) {
		b2PolygonShape dynamicBox; /*ダイナミックボディに別のボックスシェイプを定義します。*/
		dynamicBox.SetAsBox((c.usetank.maxpos.x() - c.usetank.minpos.x()) / 2, (c.usetank.maxpos.z() - c.usetank.minpos.z()) / 2, b2Vec2((c.usetank.minpos.x() + c.usetank.maxpos.x()) / 2, (c.usetank.minpos.z() + c.usetank.maxpos.z()) / 2), 0.f);
		b2FixtureDef fixtureDef;					  /*動的ボディフィクスチャを定義します*/
		fixtureDef.shape   = &dynamicBox;				  /**/
		fixtureDef.density = 1.0f;					  /*ボックス密度をゼロ以外に設定すると、動的になります*/
		fixtureDef.friction= 0.3f;					  /*デフォルトの摩擦をオーバーライドします*/
		b2BodyDef bodyDef;						  /*ダイナミックボディを定義します。その位置を設定し、ボディファクトリを呼び出します*/
		bodyDef.type= b2_dynamicBody;					  /**/
		bodyDef.position.Set(c.vehicle[0].pos.x(), c.vehicle[0].pos.z()); /**/

		auto pp      = c.vehicle[0].mat.zvec();
		bodyDef.angle= atan2f(-pp.x(), -pp.z());

		c.b2mine.body.reset(world->CreateBody(&bodyDef));	      /**/
		c.b2mine.playerfix= c.b2mine.body->CreateFixture(&fixtureDef); /*シェイプをボディに追加します*/
									       /* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
	}
	//壁をセットする
	std::vector<hit::wallPats> wall;
	std::vector<hit::treePats> tree;
	{
		MV1SetupReferenceMesh(map_col.get(), 0, FALSE);
		MV1_REF_POLYGONLIST p= MV1GetReferenceMesh(map_col.get(), 0, FALSE);

		for (int i= 0; i < p.PolygonNum; i++) {
			if (p.Polygons[i].MaterialIndex == 2) {
				//壁
				{
					wall.resize(wall.size() + 1);
					wall.back().pos[0]= p.Vertexs[p.Polygons[i].VIndex[0]].Position;
					wall.back().pos[1]= p.Vertexs[p.Polygons[i].VIndex[1]].Position;
					if (b2DistanceSquared(b2Vec2(wall.back().pos[0].x(), wall.back().pos[0].z()), b2Vec2(wall.back().pos[1].x(), wall.back().pos[1].z())) <= 0.005f * 0.005f) {
						wall.pop_back();
					}

					wall.resize(wall.size() + 1);
					wall.back().pos[0]= p.Vertexs[p.Polygons[i].VIndex[1]].Position;
					wall.back().pos[1]= p.Vertexs[p.Polygons[i].VIndex[2]].Position;
					if (b2DistanceSquared(b2Vec2(wall.back().pos[0].x(), wall.back().pos[0].z()), b2Vec2(wall.back().pos[1].x(), wall.back().pos[1].z())) <= 0.005f * 0.005f) {
						wall.pop_back();
					}

					wall.resize(wall.size() + 1);
					wall.back().pos[0]= p.Vertexs[p.Polygons[i].VIndex[2]].Position;
					wall.back().pos[1]= p.Vertexs[p.Polygons[i].VIndex[0]].Position;
					if (b2DistanceSquared(b2Vec2(wall.back().pos[0].x(), wall.back().pos[0].z()), b2Vec2(wall.back().pos[1].x(), wall.back().pos[1].z())) <= 0.005f * 0.005f) {
						wall.pop_back();
					}
				}
			} else if (p.Polygons[i].MaterialIndex == 3) {
				//木
				{
					tree.resize(tree.size() + 1);
					tree.back().mat      = MATRIX_ref::Scale(VGet(15.f / 10.f, 15.f / 10.f, 15.f / 10.f));
					tree.back().pos      = (VECTOR_ref(p.Vertexs[p.Polygons[i].VIndex[0]].Position) + p.Vertexs[p.Polygons[i].VIndex[1]].Position + p.Vertexs[p.Polygons[i].VIndex[2]].Position) * (1.f / 3.f);
					tree.back().fall_flag= false;
					tree.back().fall_vec = VGet(0.f, 0.f, 1.f);
					tree.back().fall_rad = 0.f;

					tree.back().obj= tree_model.Duplicate();
					tree.back().obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
				}
			}
		}
	}
	for (auto& w : wall) {
		// This a chain shape with isolated vertices
		std::array<b2Vec2, 2> vs;
		vs[0].Set(w.pos[0].x(), w.pos[0].z());
		vs[1].Set(w.pos[1].x(), w.pos[1].z());
		b2ChainShape chain;
		chain.CreateChain(&vs[0], 2);
		b2FixtureDef fixtureDef;			       /*動的ボディフィクスチャを定義します*/
		fixtureDef.shape   = &chain;			       /**/
		fixtureDef.density = 1.0f;			       /*ボックス密度をゼロ以外に設定すると、動的になります*/
		fixtureDef.friction= 0.3f;			       /*デフォルトの摩擦をオーバーライドします*/
		b2BodyDef bodyDef;				       /*ダイナミックボディを定義します。その位置を設定し、ボディファクトリを呼び出します*/
		bodyDef.type= b2_staticBody;			       /**/
		bodyDef.position.Set(0, 0);			       /**/
		bodyDef.angle= 0.f;				       /**/
		w.b2.body.reset(world->CreateBody(&bodyDef));	  /**/
		w.b2.playerfix= w.b2.body->CreateFixture(&fixtureDef); /*シェイプをボディに追加します*/
	}
	//必要な時に当たり判定をリフレッシュする(仮)
	auto ref_col_nochangeend= [&chara](const size_t& id, const VECTOR_ref& startpos, const VECTOR_ref& endpos, const float& distance) {
		for (auto& t : chara) {
			for (auto& veh : t.vehicle) {
				if (id == t.id || (Segment_Point_MinLength(startpos.get(), endpos.get(), veh.pos.get()) > distance) || veh.hit_check) {
					continue;
				}
				veh.col.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));
				for (int i= 0; i < veh.col.mesh_num(); i++) {
					veh.col.RefreshCollInfo(-1, i);
				}
				veh.hit_check= true;
			}
		}
	};
	//影に描画するものを指定する(仮)
	auto draw_in_shadow= [&map, &chara, &carrier, &tree] {
		carrier.DrawModel();
		for (auto& c : chara) {
			c.vehicle[0].obj.DrawModel();
			c.vehicle[1].obj.DrawModel();
		}
		for (auto& l : tree) {
			l.obj.DrawModel();
		}
	};
	//主に描画するものを指定する(仮)
	auto draw_in_window= [&Drawparts, &map, &carrier, &chara, &ads, &Vertex, &campos, &tree] {
		Drawparts->Draw_by_Shadow([&map, &carrier, &chara, &ads, &Vertex, &tree] {
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
						t.vehicle[0].obj.DrawModel();
					}
					for (auto& h : t.vehicle[0].hit) {
						if (h.flug) {
							h.pic.DrawFrame(h.use); //弾痕
						}
					}
				}
				//戦闘機
				for (auto& t : chara) {
					t.vehicle[1].obj.DrawModel();
					for (auto& be : t.p_burner) {
						be.effectobj.DrawModel(); //バーナー
					}
					for (auto& h : t.vehicle[1].hit) {
						if (h.flug) {
							h.pic.DrawFrame(h.use); //弾痕
						}
					}
				}
				for (auto& l : tree) {
					l.obj.DrawModel();
				}
			}
		});

		SetFogEnable(FALSE);
		SetUseLighting(FALSE);
		for (auto& c : chara) {
			for (auto& gns : c.vehicle) {
				for (auto& g : gns.Gun_) {
					for (auto& a : g.bullet) {
						if (a.flug) {

							if (a.spec.type == 2) {
								SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255); // 描画ブレンドモードを戻す
								DrawCapsule3D(a.pos.get(), a.repos.get(), ((a.spec.caliber - 0.00762f) * 0.1f + 0.00762f) * ((a.pos - campos).size() / 24.f), 4, a.color, GetColor(255, 255, 255), TRUE);
							} else {
								SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f * std::clamp<float>((a.spec.speed - 100.f) / (200.f - 100.f), 0.f, 1.f)));
								DrawCapsule3D(a.pos.get(), a.repos.get(), ((a.spec.caliber - 0.00762f) * 0.1f + 0.00762f) * ((a.pos - campos).size() / 24.f), 4, a.color, GetColor(255, 255, 255), TRUE);
							}
						}
					}
				}
			}
			for (auto& a : c.effcs_missile) {
				if (a.second != nullptr) {
					if (a.second->flug) {
						a.first.handle.SetPos(a.second->pos);
					}
					if (a.cnt != -1) {
						a.cnt++;
						if (a.cnt >= 4.f * GetFPS()) {
							a.first.handle.Stop();
							a.cnt= -1;
						}
					}
				}
			}
			for (auto& a : c.effcs_gun) {
				if (a.second != nullptr) {
					if (a.second->flug) {
						a.first.handle.SetPos(a.second->pos);
					}
					if (a.cnt != -1) {
						a.cnt++;
						if (a.cnt >= 4.f * GetFPS()) {
							a.first.handle.Stop();
							a.cnt= -1;
						}
					}
				}
			}
		}

		SetUseLighting(TRUE);
		SetFogEnable(TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255); // 描画ブレンドモードを戻す
	};
	//通信
	{
	}
	//開始
	auto& mine= chara[0];
	Rot       = 0;
	tgt       = 1;
	eyevec    = mine.vehicle[mine.mode].mat.zvec() * -1.f;
	campos    = mine.vehicle[mine.mode].pos + VGet(0.f, 3.f, 0.f) + eyevec * (range);
	for (auto& c : chara) {
		for (auto& g : c.usetank.wheelframe) {
			g.gndsmkeffcs.handle= Drawparts->get_gndhitHandle().Play3D();
			g.gndsmksize	= 0.1f;
		}
		for (auto& g : c.useplane.wheelframe) {
			g.gndsmkeffcs.handle= Drawparts->get_gndhitHandle().Play3D();
			g.gndsmksize	= 0.1f;
		}
		//c.effcs[ef_smoke2].handle = Drawparts->get_effHandle(ef_smoke2).Play3D();
	}
	SetMouseDispFlag(FALSE);
	SetMousePoint(dispx / 2, dispy / 2);
	while (ProcessMessage() == 0) {
		const auto fps  = GetFPS();
		const auto waits= GetNowHiPerformanceCount();
		for (auto& c : chara) {
			for (auto& veh : c.vehicle) {
				//当たり判定リフレッシュ
				if (veh.hit_check) {
					veh.col.SetMatrix(MATRIX_ref::Mtrans(VGet(0.f, -100.f, 0.f)));
					for (int i= 0; i < veh.col.mesh_num(); i++) {
						veh.col.RefreshCollInfo(-1, i);
					}
					veh.hit_check= false;
				}
			}
		}
		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
			break;
		}
		Debugparts->put_way();
		//空母移動
		{
			float spd   = -(60.f / 3.6f) / fps;
			car_yrad_add= deg2rad(0.f) / fps;
			car_yrad+= car_yrad_add;
			car_pos_add= VGet(spd * sin(car_yrad_add), 0.f, spd * cos(car_yrad_add));
			car_pos+= car_pos_add;
			carrier_col.SetMatrix(MATRIX_ref::RotY(car_yrad) * MATRIX_ref::Mtrans(car_pos));
			carrier.SetMatrix(MATRIX_ref::RotY(car_yrad) * MATRIX_ref::Mtrans(car_pos));
			carrier_col.RefreshCollInfo();
		}
		//視点
		{
			//スコープ
			{
				switch (mine.mode){
					case 0 :
						Rot= std::clamp(Rot + GetMouseWheelRotVol(), 0, 7);
		switch (Rot) {
			case 2:
				range_p= 1.f;
				break;
			case 1:
				range_p= 7.5f;
				break;
			case 0:
				range_p= 15.f;
				break;
		}
		ratio= 1.f;
		for (int i= 3; i < Rot; i++) {
			ratio*= 5.f;
		}
		break;
		case 1:
			Rot= std::clamp(Rot + GetMouseWheelRotVol(), 0, 2);
			switch (Rot) {
				case 2:
					range_p= 1.f;
					break;
				case 1:
					range_p= 15.f;
					break;
				case 0:
					range_p= 30.f;
					break;
			}
			ratio= 1.f;
			break;
		default:
			break;
	}
	if (ads != (Rot >= 3)) {
		ads= (Rot >= 3);
	}
	if (ads) {
		range_p= 1.f;
	}
	easing_set(&range, range_p, (ads) ? 0.f : 0.95f, fps);
}
//砲塔旋回
{
	lock_on.second= std::min<uint8_t>(lock_on.second + 1, ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) ? 2 : 0);
	if (lock_on.second == 1) {
		if (lock_on.first == true) {
			lock_on.first = false;
			lock_on.second= 2;
		}
	}
	if (lock_on.second == 1) {
		if (lock_on.first == false && tgt != chara.size()) {
			lock_on.first = true;
			lock_on.second= 2;
		}
	}

	if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0 && !lock_on.first) { //砲塔ロック
		mine.view_yrad= 0.f;
		mine.view_xrad= 0.f;
	} else {
		//狙い
		VECTOR_ref vec_a;
		{
			VECTOR_ref endpos= campos - eyevec * (2000.f);
			//マップに当たったか
			for (int i= 0; i < map_col.mesh_num(); i++) {
				auto hp= map_col.CollCheck_Line(campos, endpos, 0, i);
				if (hp.HitFlag == TRUE) {
					endpos= hp.HitPosition;
				}
			}
			//車輛に当たったか
			if (!lock_on.first) {
				tgt= chara.size();
			}
			ref_col_nochangeend(mine.id, campos, endpos, 10.f);
			for (auto& t : chara) {
				auto& veh= t.vehicle[0];
				if (veh.hit_check) {
					for (int i= 0; i < veh.col.mesh_num(); i++) {
						veh.col.RefreshCollInfo(-1, i);
						const auto hp= veh.col.CollCheck_Line(campos, endpos, -1, i);
						if (hp.HitFlag == TRUE) {
							endpos= hp.HitPosition;
							if (!lock_on.first) {
								tgt= t.id;
							}
						}
					}
				}
			}

			vec_a= (mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first) - endpos).Norm();
		}
		if (ads) {
			vec_a= eyevec;
		}

		//ロックオン
		if (lock_on.first) {
			auto& c= chara[tgt];
			vec_a  = (mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first) - c.vehicle[0].obj.frame(c.usetank.gunframe[0].frame1.first)).Norm();
		}
		//反映
		auto vec_z    = mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame3.first) - mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first);
		float z_hyp   = std::hypotf(vec_z.x(), vec_z.z());
		float a_hyp   = std::hypotf(vec_a.x(), vec_a.z());
		float cost    = (vec_a.z() * vec_z.x() - vec_a.x() * vec_z.z()) / (a_hyp * z_hyp);
		mine.view_yrad= (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f; //cos取得2D
		mine.view_xrad= (atan2f(-vec_z.y(), z_hyp) - atan2f(vec_a.y(), a_hyp)) / 5.f;
	}
	if (mine.mode != 0) { //砲塔ロック
		mine.view_yrad= 0.f;
		mine.view_xrad= 0.f;
	}
}
//キー
{
	mine.key[0]= ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);   //射撃
	mine.key[1]= ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0); //マシンガン
	mine.key[2]= (CheckHitKey(KEY_INPUT_W) != 0);
	mine.key[3]= (CheckHitKey(KEY_INPUT_S) != 0);
	mine.key[4]= (CheckHitKey(KEY_INPUT_D) != 0);
	mine.key[5]= (CheckHitKey(KEY_INPUT_A) != 0);
	//飛行時のみの操作
	if (mine.mode == 1) {
		//ヨー
		mine.key[6]= (CheckHitKey(KEY_INPUT_Q) != 0);
		mine.key[7]= (CheckHitKey(KEY_INPUT_E) != 0);
		//スロットル
		mine.key[8]= (CheckHitKey(KEY_INPUT_R) != 0);
		mine.key[9]= (CheckHitKey(KEY_INPUT_F) != 0);
		//脚
		mine.key[10]= (CheckHitKey(KEY_INPUT_C) != 0);
		mine.key[11]= (CheckHitKey(KEY_INPUT_G) != 0);
		//精密操作
		mine.key[12]= (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
		//着艦フックスイッチ
		mine.key[13]= (CheckHitKey(KEY_INPUT_X) != 0);
		//カタパルト
		mine.key[14]= (CheckHitKey(KEY_INPUT_SPACE) != 0);
	}
	//他モードへの移行
	change_vehicle= std::clamp<uint8_t>(change_vehicle + 1, 0, int((CheckHitKey(KEY_INPUT_P) != 0) ? 2 : 0));
	for (uint8_t i= 0; i < mine.vehicle.size(); i++) {
		if (mine.mode != i) {
			if (change_vehicle == 1) {
				mine.mode		   = i;
				eyevec			   = mine.vehicle[mine.mode].mat.zvec();
				mine.vehicle[mine.mode].add= VGet(0.f, 0.f, 0.f);
				change_vehicle		   = 2;
			}
		}
	}
}
//マウスと視点角度をリンク
{
	int mousex, mousey;
	GetMousePoint(&mousex, &mousey);
	SetMousePoint(dispx / 2, dispy / 2);
	{
		if (ads) {
			float y= atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - dispx / 2) * 0.1f / ratio);
			float x= atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - dispy / 2) * 0.1f / ratio);
			x      = std::clamp(x, deg2rad(-20), deg2rad(10));
			eyevec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
		} else {
			float y= atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - dispx / 2) * 0.1f);
			float x= atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - dispy / 2) * 0.1f);
			x      = std::clamp(x, deg2rad(-25), deg2rad(89));
			eyevec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
		}
	}
}
}
{
	//他のキー入力をここで取得(ホスト)
}
//反映
for (auto& c : chara) {
	//戦車演算
	{
		auto& veh= c.vehicle[0];
		VECTOR_ref yvec, zvec;
		//砲塔旋回
		{
			float limit= deg2rad(c.usetank.turret_rad_limit) / fps;
			veh.Gun_[0].gun_info.yrad+= std::clamp(c.view_yrad, -limit, limit); //veh.Gun_[0].gun_info.yrad = std::clamp(veh.Gun_[0].gun_info.yrad + std::clamp(view_yrad / 5.f, -limit, limit),deg2rad(-30.0)+yrad,deg2rad(30.0)+yrad);//射界制限
			for (auto& g : veh.Gun_) {
				g.gun_info.xrad= std::clamp(g.gun_info.xrad + std::clamp(c.view_xrad, -limit, limit), deg2rad(-10), deg2rad(20));
			}
		}
		//反映
		for (auto& f : veh.Gun_) {
			veh.obj.SetFrameLocalMatrix(f.gun_info.frame1.first, MATRIX_ref::RotY(f.gun_info.yrad) * MATRIX_ref::Mtrans(f.gun_info.frame1.second));
			veh.col.SetFrameLocalMatrix(f.gun_info.frame1.first, MATRIX_ref::RotY(f.gun_info.yrad) * MATRIX_ref::Mtrans(f.gun_info.frame1.second));
			if (f.gun_info.frame2.first >= 0) {
				veh.obj.SetFrameLocalMatrix(f.gun_info.frame2.first, MATRIX_ref::RotX(f.gun_info.xrad) * MATRIX_ref::Mtrans(f.gun_info.frame2.second));
				veh.col.SetFrameLocalMatrix(f.gun_info.frame2.first, MATRIX_ref::RotX(f.gun_info.xrad) * MATRIX_ref::Mtrans(f.gun_info.frame2.second));
			}
			if (f.gun_info.frame3.first >= 0) {
				veh.obj.SetFrameLocalMatrix(f.gun_info.frame3.first, MATRIX_ref::Mtrans(VGet(0.f, 0.f, f.fired * 0.5f)) * MATRIX_ref::Mtrans(f.gun_info.frame3.second)); //リコイル
				veh.col.SetFrameLocalMatrix(f.gun_info.frame3.first, MATRIX_ref::Mtrans(VGet(0.f, 0.f, f.fired * 0.5f)) * MATRIX_ref::Mtrans(f.gun_info.frame3.second)); //リコイル
			}
		}
		//転輪
		{
			auto y_vec= veh.mat.yvec();
			for (auto& f : c.usetank.wheelframe) {
				MATRIX_ref tmp;
				veh.obj.frame_reset(f.frame.first);
				auto startpos= veh.obj.frame(f.frame.first);
				auto hp      = map_col.CollCheck_Line(startpos + y_vec * ((-f.frame.second.y()) + 2.f), startpos + y_vec * ((-f.frame.second.y()) - 0.3f), 0, 0);
				if (hp.HitFlag == TRUE) {
					tmp= MATRIX_ref::Mtrans(VGet(0.f, hp.HitPosition.y + y_vec.y() * f.frame.second.y() - startpos.y(), 0.f));
				} else {
					tmp= MATRIX_ref::Mtrans(VGet(0.f, -0.3f, 0.f));
				}

				veh.obj.SetFrameLocalMatrix(f.frame.first, MATRIX_ref::RotX((f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right) * tmp * MATRIX_ref::Mtrans(f.frame.second));
			}
			for (auto& f : c.usetank.wheelframe_nospring) {
				veh.obj.SetFrameLocalMatrix(f.frame.first, MATRIX_ref::RotX((f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right) * MATRIX_ref::Mtrans(f.frame.second));
			}
		}
		easing_set(
			&c.wheel_normal,
			((veh.obj.frame(c.usetank.square[0]) - veh.obj.frame(c.usetank.square[3])).cross(veh.obj.frame(c.usetank.square[1]) - veh.obj.frame(c.usetank.square[2]))).Norm(),
			0.95f,
			fps);
		//移動
		auto hp     = map_col.CollCheck_Line(veh.pos + VGet(0.f, 2.f, 0.f), veh.pos - VGet(0.f, 0.1f, 0.f), 0, 0);
		auto isfloat= (veh.pos.y() == -c.usetank.down_in_water);
		//Z、Yベクトル取得
		{
			auto pp    = c.vehicle[0].mat.zvec();
			auto yrad_p= atan2f(-pp.x(), -pp.z());
			zvec       = VGet(sinf(yrad_p), 0.f, cosf(yrad_p));
			if (c.usetank.isfloat && isfloat) {
				yvec= VGet(0.f, 1.f, 0.f);
			} else {
				yvec= c.wheel_normal;
				zvec= MATRIX_ref::Vtrans(zvec, MATRIX_ref::RotVec2(VGet(0.f, 1.f, 0.f), yvec));
			}
		}
		if (hp.HitFlag == TRUE || (c.usetank.isfloat && isfloat)) {
			//前進後退
			{
				const auto old= veh.speed_add + veh.speed_sub;
				if (c.key[2] && !c.mode == 1) {
					veh.speed_add= (veh.speed_add < (c.usetank.flont_speed_limit / 3.6f)) ? (veh.speed_add + (0.21f / 3.6f) * (60.f / fps)) : veh.speed_add;
					veh.speed_sub= (veh.speed_sub < 0.f) ? (veh.speed_sub + (0.7f / 3.6f) * (60.f / fps)) : veh.speed_sub;
				}
				if (c.key[3] && !c.mode == 1) {
					veh.speed_sub= (veh.speed_sub > (c.usetank.back_speed_limit / 3.6f)) ? (veh.speed_sub - (0.21f / 3.6f) * (60.f / fps)) : veh.speed_sub;
					veh.speed_add= (veh.speed_add > 0.f) ? (veh.speed_add - (0.7f / 3.6f) * (60.f / fps)) : veh.speed_add;
				}
				if (!(c.key[2] && !c.mode == 1) && !(c.key[3] && !c.mode == 1)) {
					veh.speed_add= (veh.speed_add > 0.f) ? (veh.speed_add - (0.35f / 3.6f) * (60.f / fps)) : 0.f;
					veh.speed_sub= (veh.speed_sub < 0.f) ? (veh.speed_sub + (0.35f / 3.6f) * (60.f / fps)) : 0.f;
				}
				veh.speed= (old + ((veh.speed_add + veh.speed_sub) - old) * 0.1f) / fps;
				veh.add  = zvec * veh.speed;
			}
			//旋回
			{
				veh.yradadd_left = (c.key[4] && !c.mode == 1) ? std::max(veh.yradadd_left - deg2rad(3.5f * (60.f / fps)), deg2rad(-c.usetank.body_rad_limit)) : std::min(veh.yradadd_left + deg2rad(2.1f * (60.f / fps)), 0.f);
				veh.yradadd_right= (c.key[5] && !c.mode == 1) ? std::min(veh.yradadd_right + deg2rad(3.5f * (60.f / fps)), deg2rad(c.usetank.body_rad_limit)) : std::max(veh.yradadd_right - deg2rad(2.1f * (60.f / fps)), 0.f);
				veh.yradadd      = (veh.yradadd_left + veh.yradadd_right) / fps;
				//veh.yrad+= veh.yradadd;
			}
			//慣性
			{
				const auto xradold= veh.xradadd;
				veh.xradadd       = deg2rad(-(veh.speed / (60.f / fps)) / ((0.1f / 3.6f) / fps) * 30.f);
				easing_set(&veh.xrad, std::clamp(veh.xradadd - xradold, deg2rad(-15.f), deg2rad(15.f)), 0.995f, fps);
				veh.xrad    = std::clamp(veh.xrad, deg2rad(-7.5f), deg2rad(7.5f));
				auto avm    = MATRIX_ref::RotAxis(zvec.cross(yvec), veh.xrad);
				yvec	= MATRIX_ref::Vtrans(yvec, avm);
				zvec	= MATRIX_ref::Vtrans(zvec, avm);
				auto zradold= veh.zradadd;
				veh.zradadd = deg2rad(-veh.yradadd / (deg2rad(5.f) / fps) * 30.f);
				veh.zrad+= ((veh.zradadd - zradold) - veh.zrad) * 0.005f;
				auto bvm= MATRIX_ref::RotAxis(zvec, veh.zrad);
				yvec    = MATRIX_ref::Vtrans(yvec, bvm);
				zvec    = MATRIX_ref::Vtrans(zvec, bvm);
			}
			if (hp.HitFlag == TRUE) {
				auto yp= veh.pos.y();
				easing_set(&yp, hp.HitPosition.y, 0.9f, fps);
				veh.pos.y(yp);
			}
		} else {
			veh.add.yadd(-9.8f / powf(fps, 2.f));
		}
		//射撃反動
		{
			easing_set(&c.xrad_shot, deg2rad(-veh.Gun_[0].fired * veh.Gun_[0].Spec[0].caliber * 50.f) * cos(veh.Gun_[0].gun_info.yrad), 0.85f, fps);
			auto avm= MATRIX_ref::RotAxis(zvec.cross(yvec), c.xrad_shot);
			easing_set(&c.zrad_shot, deg2rad(-veh.Gun_[0].fired * veh.Gun_[0].Spec[0].caliber * 50.f) * sin(veh.Gun_[0].gun_info.yrad), 0.85f, fps);
			auto bvm= MATRIX_ref::RotAxis(zvec, c.zrad_shot);

			yvec= MATRIX_ref::Vtrans(yvec, avm * bvm);
			zvec= MATRIX_ref::Vtrans(zvec, avm * bvm);
		}
		//行列
		veh.mat= MATRIX_ref::Axis1(yvec.cross(zvec), yvec, zvec);
		//浮く
		if (c.usetank.isfloat) {
			veh.pos.y(std::max(veh.pos.y(), -c.usetank.down_in_water));
		}
		//転輪
		veh.wheel_Left-= veh.speed * 2.f - veh.yradadd * 5.f;
		veh.wheel_Right-= veh.speed * 2.f + veh.yradadd * 5.f;
	}
	{
		auto& veh= c.vehicle[1];
		//飛行機
		float rad_spec= deg2rad(c.useplane.body_rad_limit * (c.useplane.mid_speed_limit / veh.speed));
		if (veh.speed < c.useplane.min_speed_limit) {
			rad_spec= deg2rad(c.useplane.body_rad_limit * (std::clamp(veh.speed, 0.f, c.useplane.min_speed_limit) / c.useplane.min_speed_limit));
		}
		//ピッチ
		easing_set(&veh.xradadd_right, ((c.key[2] && c.mode == 1) ? -(c.key[12] ? rad_spec / 12.f : rad_spec / 4.f) : 0.f), 0.95f, fps);
		easing_set(&veh.xradadd_left, ((c.key[3] && c.mode == 1) ? (c.key[12] ? rad_spec / 12.f : rad_spec / 4.f) : 0.f), 0.95f, fps);
		//ロール
		easing_set(&veh.zradadd_right, ((c.key[4] && c.mode == 1) ? (c.key[12] ? rad_spec / 3.f : rad_spec) : 0.f), 0.95f, fps);
		easing_set(&veh.zradadd_left, ((c.key[5] && c.mode == 1) ? -(c.key[12] ? rad_spec / 3.f : rad_spec) : 0.f), 0.95f, fps);
		//ヨー
		easing_set(&veh.yradadd_left, ((c.key[6] && c.mode == 1) ? -(c.key[12] ? rad_spec / 24.f : rad_spec / 8.f) : 0.f), 0.95f, fps);
		easing_set(&veh.yradadd_right, ((c.key[7] && c.mode == 1) ? (c.key[12] ? rad_spec / 24.f : rad_spec / 8.f) : 0.f), 0.95f, fps);
		//スロットル
		easing_set(&veh.speed_add, (((c.key[8] && c.mode == 1) && veh.speed < c.useplane.max_speed_limit) ? (0.5f / 3.6f) : 0.f), 0.95f, fps);
		easing_set(&veh.speed_sub, (c.key[9] && c.mode == 1) ? ((veh.speed > c.useplane.min_speed_limit) ? (-0.5f / 3.6f) : ((veh.speed > 0.f) ? (-0.2f / 3.6f) : 0.f)) : 0.f, 0.95f, fps);
		//スピード
		veh.speed+= (veh.speed_add + veh.speed_sub) * 60.f / fps;
		{
			auto tmp = veh.mat.zvec();
			auto tmp2= sin(atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z())));
			veh.speed+= (((std::abs(tmp2) > sin(deg2rad(1.0f))) ? tmp2 * 0.5f : 0.f) / 3.6f) * 60.f / fps; //落下
		}
		//座標系反映
		{
			auto t_mat= veh.mat;
			veh.mat*= MATRIX_ref::RotAxis(t_mat.xvec(), (veh.xradadd_right + veh.xradadd_left) / fps);
			veh.mat*= MATRIX_ref::RotAxis(t_mat.zvec(), (veh.zradadd_right + veh.zradadd_left) / fps);
			veh.mat*= MATRIX_ref::RotAxis(t_mat.yvec(), (veh.yradadd_left + veh.yradadd_right) / fps);
		}
		//
		c.landing.second= std::min<uint8_t>(c.landing.second + 1, uint8_t((c.key[13] && c.mode == 1) ? 2 : 0));
		if (c.landing.second == 1) {
			c.landing.first^= 1;
		}
		//脚
		c.changegear.second= std::min<uint8_t>(c.changegear.second + 1, uint8_t((c.key[10] && c.mode == 1) ? 2 : 0));
		if (c.changegear.second == 1) {
			c.changegear.first^= 1;
		}
		easing_set(&c.p_anime_geardown.second, float(c.changegear.first), 0.95f, fps);
		MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
		//舵
		for (int i= 0; i < c.p_animes_rudder.size(); i++) {
			easing_set(&c.p_animes_rudder[i].second, float(c.key[i + 2] && c.mode == 1), 0.95f, fps);
			MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
		}
		//
		{
			//
			if (veh.speed >= c.useplane.min_speed_limit) {
				easing_set(&veh.add, VGet(0.f, 0.f, 0.f), 0.9f, fps);
			} else {
				veh.add.yadd(-9.8f / powf(fps, 2.f));
			}

			//着艦ワイヤ-処理
			{
				veh.obj.frame_reset(c.useplane.hook.first);
				veh.obj.SetFrameLocalMatrix(c.useplane.hook.first, MATRIX_ref::RotX(deg2rad(c.p_landing_per)) * MATRIX_ref::Mtrans(c.useplane.hook.second));
				easing_set(&c.p_landing_per, (c.landing.first) ? 20.f : 0.f, 0.95f, fps);
				if (c.landing.first) {
					bool to= false;
					for (auto& wi : wire) {
						carrier.frame_reset(wi.first);
						if ((veh.obj.frame(c.useplane.hook.first + 1) - carrier.frame(wi.first)).size() <= 30.f) {
							VECTOR_ref vec1= (veh.obj.frame(c.useplane.hook.first + 1) - veh.obj.frame(c.useplane.hook.first)).Norm();
							VECTOR_ref vec2= (carrier.frame(wi.first) - veh.obj.frame(c.useplane.hook.first)).Norm();
							if (vec1.dot(vec2) >= 0) {
								to= true;
								carrier.SetFrameLocalMatrix(wi.first, MATRIX_ref::Mtrans(veh.obj.frame(c.useplane.hook.first + 1) - carrier.frame(wi.first)) * veh.mat.Inverse() * MATRIX_ref::Mtrans(wi.second));
								break;
							}
						}
					}
					if (to && veh.speed > 0.f) {
						veh.speed+= -2.5f / 3.6f;
					}
				}
			}

			if (c.p_anime_geardown.second >= 0.5f) {
				for (auto& w : c.useplane.wheelframe) {
					easing_set(&w.gndsmksize, 0.01f, 0.9f, fps);
					auto tmp= veh.obj.frame(int(w.frame.first + 1)) - VGet(0.f, 0.2f, 0.f);
					{
						auto hp= map_col.CollCheck_Line(tmp + (veh.mat.yvec() * (0.98f)), tmp, 0, 0);
						if (hp.HitFlag == TRUE) {
							veh.add= (VECTOR_ref(hp.HitPosition) - tmp);
							{
								auto x_vec= veh.mat.xvec();
								auto y_vec= veh.mat.yvec();
								auto z_vec= veh.mat.zvec();

								auto y_vec2= y_vec;
								easing_set(&y_vec2, hp.Normal, 0.95f, fps);
								auto normal= y_vec2;

								veh.mat= MATRIX_ref::Axis1(
									MATRIX_ref::Vtrans(x_vec, MATRIX_ref::RotVec2(y_vec, normal)),
									MATRIX_ref::Vtrans(y_vec, MATRIX_ref::RotVec2(y_vec, normal)),
									MATRIX_ref::Vtrans(z_vec, MATRIX_ref::RotVec2(y_vec, normal)));
							}
							w.gndsmksize= std::clamp(veh.speed * 3.6f / 50.f, 0.1f, 1.f);
							if (veh.speed >= 0.f && (c.key[11] && c.mode == 1)) {
								veh.speed+= -0.5f / 3.6f;
							}
							if (veh.speed <= 0.f) {
								easing_set(&veh.speed, 0.f, 0.9f, fps);
							}
						}
					}
					{
						auto hp= carrier_col.CollCheck_Line(tmp + veh.mat.yvec() * (0.98f), tmp);
						if (hp.HitFlag == TRUE) {
							veh.add= (VECTOR_ref(hp.HitPosition) - tmp);
							{
								auto x_vec= veh.mat.xvec();
								auto y_vec= veh.mat.yvec();
								auto z_vec= veh.mat.zvec();

								auto y_vec2= y_vec;
								easing_set(&y_vec2, hp.Normal, 0.95f, fps);
								auto normal= y_vec2;

								veh.mat= MATRIX_ref::Axis1(
									MATRIX_ref::Vtrans(x_vec, MATRIX_ref::RotVec2(y_vec, normal)),
									MATRIX_ref::Vtrans(y_vec, MATRIX_ref::RotVec2(y_vec, normal)),
									MATRIX_ref::Vtrans(z_vec, MATRIX_ref::RotVec2(y_vec, normal)));
							}
							veh.add+= car_pos_add + (MATRIX_ref::Vtrans(veh.pos - car_pos, MATRIX_ref::RotY(car_yrad_add)) - (veh.pos - car_pos));
							veh.mat*= MATRIX_ref::RotY(car_yrad_add);

							w.gndsmksize= std::clamp(veh.speed * 3.6f / 50.f, 0.1f, 1.f);
							if (veh.speed >= 0.f && (c.key[11] && c.mode == 1)) {
								veh.speed+= -1.0f / 3.6f;
							}
							if (veh.speed <= 0.f) {
								easing_set(&veh.speed, 0.f, 0.9f, fps);
							}

							if (c.key[14]) {
								easing_set(&veh.speed, c.useplane.mid_speed_limit, 0.90f, fps);
							}
						}
					}
				}
				auto y_vec= veh.mat.yvec();

				//転輪
				veh.wheel_Left-= veh.speed / 20.f;  // -veh.yradadd * 5.f;
				veh.wheel_Right-= veh.speed / 20.f; // +veh.yradadd * 5.f;

				for (auto& f : c.useplane.wheelframe_nospring) {

					veh.obj.SetFrameLocalMatrix(f.frame.first,
								    MATRIX_ref::RotAxis(
									    MATRIX_ref::Vtrans(VGet(0.f, 0.f, 0.f), MV1GetFrameLocalMatrix(veh.obj.get(), f.frame.first + 1)),
									    (f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right) *
									    MATRIX_ref::Mtrans(f.frame.second));
				}
			} else {
				for (auto& w : c.useplane.wheelframe) {
					easing_set(&w.gndsmksize, 0.01f, 0.9f, fps);
				}
			}
			veh.pos+= veh.add + (veh.mat.zvec() * (-veh.speed / fps));
		}
		//浮く
		if (c.useplane.isfloat) {
			veh.pos.y(std::max(veh.pos.y(), -c.useplane.down_in_water));
		}
		//壁の当たり判定
		bool hitb= false;
		{
			VECTOR_ref p_0= veh.pos + MATRIX_ref::Vtrans(VGet(c.useplane.minpos.x(), 0.f, c.useplane.maxpos.z()), veh.mat);
			VECTOR_ref p_1= veh.pos + MATRIX_ref::Vtrans(VGet(c.useplane.maxpos.x(), 0.f, c.useplane.maxpos.z()), veh.mat);
			VECTOR_ref p_2= veh.pos + MATRIX_ref::Vtrans(VGet(c.useplane.maxpos.x(), 0.f, c.useplane.minpos.z()), veh.mat);
			VECTOR_ref p_3= veh.pos + MATRIX_ref::Vtrans(VGet(c.useplane.minpos.x(), 0.f, c.useplane.minpos.z()), veh.mat);
			if (p_0.y() <= 0.f || p_1.y() <= 0.f || p_2.y() <= 0.f || p_3.y() <= 0.f) {
				hitb= true;
			}
			if (!hitb) {
				while (true) {
					if (carrier_col.CollCheck_Line(p_0, p_1).HitFlag == TRUE) {
						hitb= true;
						break;
					}
					if (carrier_col.CollCheck_Line(p_1, p_2).HitFlag == TRUE) {
						hitb= true;
						break;
					}
					if (carrier_col.CollCheck_Line(p_2, p_3).HitFlag == TRUE) {
						hitb= true;
						break;
					}
					if (carrier_col.CollCheck_Line(p_3, p_0).HitFlag == TRUE) {
						hitb= true;
						break;
					}
					break;
				}
			}
			if (!hitb) {
				for (int i= 0; i < map_col.mesh_num(); i++) {
					if (map_col.CollCheck_Line(p_0, p_1, 0, i).HitFlag == TRUE) {
						hitb= true;
						break;
					}
					if (map_col.CollCheck_Line(p_1, p_2, 0, i).HitFlag == TRUE) {
						hitb= true;
						break;
					}
					if (map_col.CollCheck_Line(p_2, p_3, 0, i).HitFlag == TRUE) {
						hitb= true;
						break;
					}
					if (map_col.CollCheck_Line(p_3, p_0, 0, i).HitFlag == TRUE) {
						hitb= true;
						break;
					}
				}
			}
		}
		if (hitb) {
			c.mode= 0; //戦車などのモードにする
			carrier.SetFrameLocalMatrix(catapult[0].first + 2, MATRIX_ref::RotX(deg2rad(-75)) * MATRIX_ref::Mtrans(catapult[0].second));
			auto pp			 = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
			veh.pos			 = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm() * (6.f);
			veh.mat			 = MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
			veh.xradadd_right	= 0.f;
			veh.xradadd_left	 = 0.f;
			veh.yradadd_left	 = 0.f;
			veh.yradadd_right	= 0.f;
			veh.zradadd_right	= 0.f;
			veh.zradadd_left	 = 0.f;
			veh.speed_add		 = 0.f;
			veh.speed_sub		 = 0.f;
			veh.speed		 = 0.f;
			veh.add			 = VGet(0.f, 0.f, 0.f);
			c.p_anime_geardown.second= 1.f;
			c.landing.first		 = false;
		}
	}
	//射撃
	{
		auto& veh= c.vehicle[c.mode];
		for (int i= 0; i < veh.Gun_.size(); i++) {
			auto& cg= veh.Gun_[i];
			if (c.key[(i == 0) ? 0 : 1] && cg.loadcnt == 0 && cg.rounds > 0) {
				auto& u= cg.bullet[cg.usebullet];
				++cg.usebullet%= cg.bullet.size();
				//ココだけ変化
				u.spec= cg.Spec[0];
				u.spec.speed*= float(75 + GetRand(50)) / 100.f;
				u.pos= veh.obj.frame(cg.gun_info.frame2.first);
				u.vec= (veh.obj.frame(cg.gun_info.frame3.first) - veh.obj.frame(cg.gun_info.frame2.first)).Norm();
				//
				cg.loadcnt= cg.gun_info.load_time;
				cg.rounds = std::max<uint16_t>(cg.rounds - 1, 0);
				if (i == 0) {
					cg.fired= 1.f;
				}
				u.hit  = false;
				u.flug = true;
				u.cnt  = 0.f;
				u.yadd = 0.f;
				u.repos= u.pos;
				if (u.spec.type != 2) {
					set_effect(&c.effcs[ef_fire], veh.obj.frame(cg.gun_info.frame3.first), u.vec, u.spec.caliber / 0.1f);
					if (u.spec.caliber >= 0.037f) {
						set_effect(&c.effcs_gun[c.gun_effcnt].first, veh.obj.frame(cg.gun_info.frame3.first), u.vec);
						set_pos_effect(&c.effcs_gun[c.gun_effcnt].first, Drawparts->get_effHandle(ef_smoke2));
						c.effcs_gun[c.gun_effcnt].second= &u;
						c.effcs_gun[c.gun_effcnt].cnt   = 0;
						++c.gun_effcnt%= c.effcs_gun.size();
					}
				} else {
					set_effect(&c.effcs_missile[c.missile_effcnt].first, veh.obj.frame(cg.gun_info.frame3.first), u.vec);
					set_pos_effect(&c.effcs_missile[c.missile_effcnt].first, Drawparts->get_effHandle(ef_smoke1));
					c.effcs_missile[c.missile_effcnt].second= &u;
					c.effcs_missile[c.missile_effcnt].cnt   = 0;
					++c.missile_effcnt%= c.effcs_missile.size();
				}
			}
			cg.loadcnt= std::max(cg.loadcnt - 1.f / fps, 0.f);
			cg.fired  = std::max(cg.fired - 1.f / fps, 0.f);
		}
	}
	/*effect*/
	{
		for (auto& t : c.effcs) {
			if (t.id != ef_smoke1 && t.id != ef_smoke2) {
				set_pos_effect(&t, Drawparts->get_effHandle(int(t.id)));
			}
		}
		for (auto& t : c.usetank.wheelframe) {
			t.gndsmksize= 0.1f + std::abs(c.vehicle[0].speed) / ((c.usetank.flont_speed_limit / 3.6f) / fps) * 0.6f;
			t.gndsmkeffcs.handle.SetPos(c.vehicle[0].obj.frame(t.frame.first) + c.vehicle[0].mat.yvec() * (-t.frame.second.y()));
			t.gndsmkeffcs.handle.SetScale(t.gndsmksize);
		}
		for (auto& t : c.useplane.wheelframe) {
			t.gndsmkeffcs.handle.SetPos(c.vehicle[1].obj.frame(int(t.frame.first + 1)));
			t.gndsmkeffcs.handle.SetScale(t.gndsmksize);
		}
		//c.effcs[ef_smoke2].handle.SetPos(c.vehicle[0].obj.frame(c.ptr->smokeframe[0]));
	}
	//戦車壁判定
	c.b2mine.body->SetLinearVelocity(b2Vec2(c.vehicle[0].add.x(), c.vehicle[0].add.z()));
	c.b2mine.body->SetAngularVelocity(c.vehicle[0].yradadd);
}
//戦車座標系更新
world->Step(1.f, 1, 1);
for (auto& c : chara) {
	//戦車座標反映
	auto pp= c.vehicle[0].mat.zvec();
	c.vehicle[0].mat*= MATRIX_ref::RotY((-c.b2mine.body->GetAngle()) - atan2f(-pp.x(), -pp.z()));

	c.vehicle[0].pos.x(c.b2mine.body->GetPosition().x);
	c.vehicle[0].pos.z(c.b2mine.body->GetPosition().y);
	float spdrec= c.spd;
	easing_set(&c.spd, std::hypot(c.b2mine.body->GetLinearVelocity().x, c.b2mine.body->GetLinearVelocity().y) * ((c.spd > 0) ? 1.f : -1.f), 0.99f, fps);
	c.vehicle[0].speed= c.spd - spdrec;
	//弾関連
	for (auto& veh : c.vehicle) {
		//弾判定
		for (auto& g : veh.Gun_) {
			for (auto& a : g.bullet) {
				float size= 3.f;
				for (int z= 0; z < int(size); z++) {
					if (a.flug) {
						a.repos= a.pos;
						a.pos+= a.vec * (a.spec.speed / fps / size);
						//判定
						{
							bool ground_hit= false;
							VECTOR_ref normal;
							//戦車以外に当たる
							{
								{
									auto hps= carrier_col.CollCheck_Line(a.repos, a.pos);
									if (hps.HitFlag) {
										a.pos     = hps.HitPosition;
										normal    = hps.Normal;
										ground_hit= true;
									}
								}
								for (int i= 0; i < map_col.mesh_num(); i++) {
									auto hps= map_col.CollCheck_Line(a.repos, a.pos, 0, i);
									if (hps.HitFlag) {
										a.pos     = hps.HitPosition;
										normal    = hps.Normal;
										ground_hit= true;
									}
								}
							}
							ref_col_nochangeend(c.id, a.pos, a.repos, 10.f);
							//飛行機にあたる
							auto hitplane= hit::get_reco(c, chara, a, 1);
							//戦車に当たる
							auto hittank= hit::get_reco(c, chara, a, 0);
							//その後処理
							switch (a.spec.type) {
								case 0: //AP
									if (!(hittank || hitplane)) {
										if (ground_hit) {
											if (a.spec.caliber >= 0.020f) {
												set_effect(&c.effcs[ef_gndhit], a.pos + normal * (0.1f), normal);
											} else {
												set_effect(&c.effcs[ef_gndhit2], a.pos + normal * (0.1f), normal);
											}
											if ((a.vec.Norm().dot(normal)) <= cos(deg2rad(60))) {
												a.flug= false;
											} else {
												a.vec+= normal * ((a.vec.dot(normal)) * -2.f);
												a.vec= a.vec.Norm();
												a.pos+= a.vec * (0.01f);
												a.spec.penetration/= 2.f;
											}
										}
									}
									if (a.flug) {
										a.spec.penetration-= 1.0f / fps / size;
										a.spec.speed-= 5.f / fps / size;
										a.pos+= VGet(0.f, a.yadd / size, 0.f);
									}
									break;
								case 1: //HE
									if (!(hittank || hitplane)) {
										if (ground_hit) {
											if (a.spec.caliber >= 0.020f) {
												set_effect(&c.effcs[ef_gndhit], a.pos + normal * (0.1f), normal);
											} else {
												set_effect(&c.effcs[ef_gndhit2], a.pos + normal * (0.1f), normal);
											}
											a.flug= false;
										}
									}
									if (a.flug) {
										a.spec.speed-= 5.f / fps / size;
										a.pos+= VGet(0.f, a.yadd / size, 0.f);
									}
									break;
								case 2: //ミサイル
									if (!(hittank || hitplane)) {
										if (ground_hit) {
											if (a.spec.caliber >= 0.020f) {
												set_effect(&c.effcs[ef_gndhit], a.pos + normal * (0.1f), normal);
											} else {
												set_effect(&c.effcs[ef_gndhit2], a.pos + normal * (0.1f), normal);
											}
											a.flug= false;
										}
									}
									if (a.flug) {
										size_t id= chara.size();
										VECTOR_ref pos;
										float dist= (std::numeric_limits<float>::max)();
										for (auto& t : chara) {
											//弾関連
											if (c.id == t.id) {
												continue;
											}
											{
												auto& veh_t= t.vehicle[0];
												auto p     = (veh_t.pos - a.pos).size();
												if (dist > p) {
													dist= p;
													id  = t.id;
													pos = veh_t.pos + VGet(0.f, 1.f, 0.f);
												}
											}
											{
												auto& veh_t= t.vehicle[1];
												auto p     = (veh_t.pos - a.pos).size();
												if (dist > p) {
													dist= p;
													id  = t.id;
													pos = veh_t.pos;
												}
											}
										}
										if (id != chara.size()) {
											auto vec_a= (a.pos - pos).Norm();
											//反映
											auto vec_z     = a.vec;
											float z_hyp    = std::hypotf(vec_z.x(), vec_z.z());
											float a_hyp    = std::hypotf(vec_a.x(), vec_a.z());
											float cost     = (vec_a.z() * vec_z.x() - vec_a.x() * vec_z.z()) / (a_hyp * z_hyp);
											float view_yrad= (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f; //cos取得2D
											float view_xrad= (atan2f(-vec_z.y(), z_hyp) - atan2f(vec_a.y(), a_hyp)) / 5.f;
											{
												float limit= deg2rad(30.f) / fps;
												float y    = atan2f(a.vec.x(), a.vec.z()) + std::clamp(view_yrad, -limit, limit);
												float x    = atan2f(a.vec.y(), std::hypotf(a.vec.x(), a.vec.z())) + std::clamp(view_xrad, -limit, limit);
												a.vec      = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
											}
										}
									}
									break;
								default:
									break;
							}
						}

						//消す(3秒たった、スピードが0以下、貫通が0以下)
						if (a.cnt >= 3.f || a.spec.speed < 100.f || a.spec.penetration <= 0.f) {
							a.flug= false;
						}
					}
				}
				a.yadd+= -9.8f / powf(fps, 2.f);
				a.cnt+= 1.f / fps;
			}
		}
		//弾痕
		for (auto& h : veh.hit) {
			if (h.flug) {
				auto y_vec= MATRIX_ref::Vtrans(h.y_vec, veh.mat);
				auto z_vec= MATRIX_ref::Vtrans(h.z_vec, veh.mat);

				h.pic.SetScale(h.scale);
				h.pic.SetRotationZYAxis(z_vec, y_vec, 0.f);
				h.pic.SetPosition(veh.pos + MATRIX_ref::Vtrans(h.pos, veh.mat) + y_vec * (0.02f));

				//h.pic.SetMatrix(Axis1((y_vec*z_vec), y_vec, z_vec, (veh.pos + MATRIX_ref::Vtrans(h.pos,veh.mat) + y_vec*(0.005f))) *SetScale(h.scale.get()));
			}
		}
	}
	//木判定
	{
		auto& veh= c.vehicle[0];
		for (auto& l : tree) {
			if (!l.fall_flag) {
				auto p0= veh.obj.frame(c.usetank.square[1]);
				auto p1= veh.obj.frame(c.usetank.square[0]);
				auto p2= veh.obj.frame(c.usetank.square[2]);
				auto p3= veh.obj.frame(c.usetank.square[3]);
				p0.y(l.pos.y());
				p1.y(l.pos.y());
				p2.y(l.pos.y());
				p3.y(l.pos.y());

				size_t cnt= 0;
				cnt+= (((p0 - p1).cross(l.pos - p1)).y() >= 0);
				cnt+= (((p1 - p2).cross(l.pos - p2)).y() >= 0);
				cnt+= (((p2 - p3).cross(l.pos - p3)).y() >= 0);
				cnt+= (((p3 - p0).cross(l.pos - p0)).y() >= 0);
				if (cnt == 4) {
					l.fall_vec = VGet((l.pos - veh.pos).z(), 0.f, -(l.pos - veh.pos).x());
					l.fall_flag= true;
				}
			}
		}
	}
}
//木セット
for (auto& l : tree) {
	if (l.fall_flag) {
		l.fall_rad= std::clamp(l.fall_rad + deg2rad(30.f / fps), deg2rad(0.f), deg2rad(90.f));
	}
	l.obj.SetMatrix(MATRIX_ref::RotAxis(l.fall_vec, l.fall_rad) * l.mat * MATRIX_ref::Mtrans(l.pos));
}
{
	//他の座標をここで出力(ホスト)
} {
	//ホストからの座標をここで入力
}
//モデルに反映
for (auto& c : chara) {
	c.vehicle[0].obj.SetMatrix(c.vehicle[0].mat * MATRIX_ref::Mtrans(c.vehicle[0].pos));
	c.vehicle[1].obj.SetMatrix(c.vehicle[1].mat * MATRIX_ref::Mtrans(c.vehicle[1].pos));
	for (auto& be : c.p_burner) {
		be.effectobj.SetMatrix(MATRIX_ref::Scale(VGet(1.f, 1.f, std::clamp(c.vehicle[1].speed / c.useplane.mid_speed_limit, 0.1f, 1.f))) * MATRIX_ref::Mtrans(be.frame.second) * c.vehicle[1].mat * MATRIX_ref::Mtrans(c.vehicle[1].pos));
	}
}
//影用意
Drawparts->Ready_Shadow(campos, draw_in_shadow);
{
	if (ads) {
		campos= mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame1.first) + MATRIX_ref::Vtrans(mine.vehicle[0].Gun_[0].gun_info.frame2.second, MATRIX_ref::RotY(atan2f(eyevec.x(), eyevec.z())));
		camvec= campos - eyevec;
		camup = mine.vehicle[0].mat.yvec();
	} else {
		if (mine.mode == 0) {
			{
				camvec= mine.vehicle[0].pos + VGet(0.f, 3.f, 0.f);
				camvec.y(std::max(camvec.y(), 5.f));
			}
			{
				campos= camvec + eyevec * (range);
				campos.y(std::max(campos.y(), 0.f));
				for (int i= 0; i < map_col.mesh_num(); i++) {
					auto hp= map_col.CollCheck_Line(camvec, campos, 0, i);
					if (hp.HitFlag == TRUE) {
						campos= camvec + (VECTOR_ref(hp.HitPosition) - camvec) * (0.9f);
					}
				}
				{
					auto hp= carrier_col.CollCheck_Line(camvec, campos);
					if (hp.HitFlag == TRUE) {
						campos= camvec + (VECTOR_ref(hp.HitPosition) - camvec) * (0.9f);
					}
				}
			}
			{
				camup= VGet(0.f, 1.f, 0.f);
				camup= MATRIX_ref::Vtrans(camup, MATRIX_ref::RotAxis(mine.vehicle[0].mat.xvec(), mine.xrad_shot));
				camup= MATRIX_ref::Vtrans(camup, MATRIX_ref::RotAxis(mine.vehicle[0].mat.zvec(), mine.zrad_shot));
				camup= MATRIX_ref::Vtrans(camup, MATRIX_ref::RotAxis(mine.vehicle[0].mat.xvec(), mine.zrad_shot));
				camup= MATRIX_ref::Vtrans(camup, MATRIX_ref::RotAxis(mine.vehicle[0].mat.zvec(), mine.xrad_shot));
			}
		} else {
			camvec= mine.vehicle[1].pos + mine.vehicle[1].mat.yvec() * (6.f);
			camvec.y(std::max(camvec.y(), 5.f));

			if ((GetMouseInput() & MOUSE_INPUT_RIGHT) == 0) {
				eyevec= (camvec - aimpos[1]).Norm();
				campos= camvec + eyevec * (range);
				camup = mine.vehicle[1].mat.yvec();

			} else {
				campos= camvec + eyevec * (range);
				campos.y(std::max(campos.y(), 0.f));
				for (int i= 0; i < map_col.mesh_num(); i++) {
					auto hp= map_col.CollCheck_Line(camvec, campos, 0, i);
					if (hp.HitFlag == TRUE) {
						campos= camvec + (VECTOR_ref(hp.HitPosition) - camvec) * (0.9f);
					}
				}
				{
					auto hp= carrier_col.CollCheck_Line(camvec, campos);
					if (hp.HitFlag == TRUE) {
						campos= camvec + (VECTOR_ref(hp.HitPosition) - camvec) * (0.9f);
					}
				}
				camup= VGet(0.f, 1.f, 0.f);
			}
		}
	}
}
{
	float fardist= 1.f;
	{
		switch (mine.mode) {
			case 0: {
				VECTOR_ref aimingpos= campos + (camvec - campos).Norm() * (1000.f);
				for (int i= 0; i < map_col.mesh_num(); i++) {
					auto hp= map_col.CollCheck_Line(campos, aimingpos, 0, i);
					if (hp.HitFlag == TRUE) {
						aimingpos= hp.HitPosition;
					}
				}
				auto hp= carrier_col.CollCheck_Line(campos, aimingpos);
				if (hp.HitFlag == TRUE) {
					aimingpos= hp.HitPosition;
				}
				fardist= std::clamp((campos - aimingpos).size(), 300.f, 1000.f);
			} break;
			case 1:
				fardist= 2000.f;
				break;
			default:
				break;
		}
	}
	float neardist= 1.f;
	{
		switch (mine.mode) {
			case 0:
				neardist= (ads ? (1.5f + 98.5f * (fardist - 300.f) / (1000.f - 300.f)) : 1.5f);
				break;
			case 1:
				neardist= 10.f;
				break;
			default:
				break;
		}
	}
	//空
	SkyScreen.SetDraw_Screen(1000.0f, 5000.0f, fov / ratio, campos - camvec, VGet(0, 0, 0), camup);
	{
		SetFogEnable(FALSE);
		SetUseLighting(FALSE);
		sky.DrawModel();
		SetUseLighting(TRUE);
		SetFogEnable(TRUE);
	}
	//被写体深度描画
	Hostpassparts->dof(&BufScreen, SkyScreen, draw_in_window, campos, camvec, camup, fov / ratio, fardist, neardist);
}
//
GraphHandle::SetDraw_Screen(DX_SCREEN_BACK, 0.01f, 5000.0f, fov / ratio, campos, camvec, camup);
//照準座標取得
{
	VECTOR_ref startpos, endpos;
	switch (mine.mode) {
		case 0: //戦車
			startpos= mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first);
			endpos  = startpos + (mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame3.first) - startpos).Norm() * (1000.f);
			break;
		case 1:
			startpos= mine.vehicle[1].pos;
			endpos  = startpos + mine.vehicle[1].mat.zvec() * (-1000.f);
			break;
	};
	{
		for (int i= 0; i < map_col.mesh_num(); i++) {
			auto hp2= map_col.CollCheck_Line(startpos, endpos, 0, i);
			if (hp2.HitFlag == TRUE) {
				endpos= hp2.HitPosition;
			}
		}
		auto hp2= carrier_col.CollCheck_Line(startpos, endpos);
		if (hp2.HitFlag == TRUE) {
			endpos= hp2.HitPosition;
		}
	}
	if (mine.mode == 0) {
		ref_col_nochangeend(mine.id, startpos, endpos, 10.f);
		for (auto& t : chara) {
			auto& veh= t.vehicle[0];
			if (veh.hit_check) {
				for (int i= 0; i < veh.col.mesh_num(); i++) {
					veh.col.RefreshCollInfo(-1, i);
					const auto hp= veh.col.CollCheck_Line(startpos, endpos, -1, i);
					if (hp.HitFlag == TRUE) {
						endpos= hp.HitPosition;
						if (!lock_on.first) {
							tgt= t.id;
						}
					}
				}
			}
		}
	}
	easing_set(&aimpos[mine.mode], endpos, 0.9f, fps);
	aimposout= ConvWorldPosToScreenPos(aimpos[mine.mode].get());
}
//
{
	if (lock_on.first) {
		auto& c		= chara[tgt];
		aimposout_lockon= ConvWorldPosToScreenPos(c.vehicle[0].obj.frame(c.usetank.gunframe[0].frame1.first).get());
		distance	= (c.vehicle[0].obj.frame(c.usetank.gunframe[0].frame1.first) - campos).size();
	}
}
//描画
{

	//背景
	BufScreen.DrawGraph(0, 0, false);
	//ブルーム
	Hostpassparts->bloom(BufScreen, (mine.mode == 0) ? 64 : (255));
	//UI
	UIparts->draw(aimposout, mine, ads, fps, lock_on.first, distance, aimposout_lockon, ratio);
	//デバッグ
	Debugparts->end_way();
	Debugparts->debug(10, 10, fps, float(GetNowHiPerformanceCount() - waits) / 1000.f);
}
Drawparts->Screen_Flip(waits);
}
return 0; // ソフトの終了
}

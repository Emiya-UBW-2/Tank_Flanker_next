#define NOMINMAX
#include "sub.hpp"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	auto Drawparts = std::make_unique<DXDraw>("TankFlanker"); /*汎用クラス*/
	auto Debugparts = std::make_unique<DeBuG>(60); /*汎用クラス*/
	//font
	auto font18 = FontHandle::Create(18);
	//地面
	MV1 map, map_col;
	//空
	MV1 sky;
	GraphHandle SkyScreen = GraphHandle::Make(dispx, dispy);
	//海
	VERTEX3D Vertex[6];
	{
		float x1 = -3000.f;
		float z1 = -3000.f;
		float x2 = 3000.f;
		float z2 = 3000.f;
		// 左上の頂点の情報をセット
		Vertex[0].pos = VGet(x1, 0.f, z1);
		Vertex[0].dif = GetColorU8(0, 192, 192, 255);
		Vertex[0].u = 0.0f;
		Vertex[0].v = 0.0f;
		// 右上の頂点の情報をセット
		Vertex[1].pos = VGet(x2, 0.f, z1);
		Vertex[1].dif = GetColorU8(0, 192, 192, 255);
		Vertex[1].u = 0.0f;
		Vertex[1].v = 0.0f;
		// 左下の頂点の情報をセット
		Vertex[2].pos = VGet(x1, 0.f, z2);
		Vertex[2].dif = GetColorU8(0, 192, 192, 255);
		Vertex[2].u = 0.0f;
		Vertex[2].v = 0.0f;
		// 右下の頂点の情報をセット
		Vertex[3].pos = VGet(x2, 0.f, z2);
		Vertex[3].dif = GetColorU8(0, 192, 192, 255);
		Vertex[3].u = 0.0f;
		Vertex[3].v = 0.0f;
		//
		Vertex[5] = Vertex[1];
		Vertex[4] = Vertex[2];
	}
	//弾痕
	MV1 hit_pic;
	MV1::Load("data/hit/model.mv1", &hit_pic);
	//操作
	bool ads = false;
	bool locktrt = false;
	int Rot = 0;
	float ratio = 1.f;
	float range_p = 1.f;
	float range = 1.f;
	//車輛
	std::vector<hit::Tanks> tank;
	tank.resize(1);
	{
		auto& t = tank[0];
		MV1::Load("data/tank/T-54/model.mv1", &t.obj);
		MV1::Load("data/tank/T-54/col.mv1", &t.col);
		t.isfloat = false;
		t.flont_speed_limit = 50.f;
		t.back_speed_limit = -10.f;
		t.body_rad_limit = 40.f;
		t.turret_rad_limit = 60.f;
	}
	/*
	{
		auto& t = tank[1];
		MV1::Load("data/tank/PT76/model.mv1", &t.obj);
		MV1::Load("data/tank/PT76/col.mv1", &t.col);
		t.isfloat = true;
		t.turret_rad_limit = 60.f;
	}
	{
		auto& t = tank[2];
		MV1::Load("data/tank/M60A1/model.mv1", &t.obj);
		MV1::Load("data/tank/M60A1/col.mv1", &t.col);
		t.isfloat = false;
		t.turret_rad_limit = 60.f;
	}
	*/
	for (auto& t : tank) {
		for (int i = 0; i < t.obj.mesh_num(); i++) {
			auto p = MV1GetMeshMaxPosition(t.obj.get(), i).y;
			if (t.down_in_water < p) {
				t.down_in_water = p;
			}
		}
		MV1SetMaterialDrawAlphaTestAll(t.obj.get(), TRUE, DX_CMP_GREATER, 128);
		for (int i = 0; i < t.obj.frame_num(); i++) {
			std::string p = MV1GetFrameName(t.obj.get(), i);
			if (p.find("転輪", 0) != std::string::npos) {
				t.wheelframe.resize(t.wheelframe.size() + 1);
				auto& e = t.wheelframe.back();
				{
					e.id = i;
					e.pos = t.obj.frame(e.id);
					e.LorR = (e.pos.x() >= 0);
				}
			}
			else if (p.find("旋回", 0) != std::string::npos) {
				t.gunframe.resize(t.gunframe.size() + 1);
				auto& e = t.gunframe.back();
				{
					e.id = i;
					e.pos = t.obj.frame(e.id);
					auto p2 = t.obj.frame_parent(e.id);
					if (p2 >= 0) {
						e.pos -= t.obj.frame(int(p2));//親がいる時引いとく
					}
					if (MV1GetFrameChildNum(t.obj.get(), e.id) >= 0) {
						if (std::string(MV1GetFrameName(t.obj.get(), e.id + 1)).find("仰角", 0) != std::string::npos) {
							e.id2 = e.id + 1;
							e.pos2 = t.obj.frame(e.id2) - t.obj.frame(e.id);
							if (MV1GetFrameChildNum(t.obj.get(), e.id) >= 0) {
								e.id3 = e.id2 + 1;
								e.pos3 = t.obj.frame(e.id3) - t.obj.frame(e.id2);
							}
							else {
								e.id3 = -1;
							}
						}
					}
					else {
						e.id2 = -1;
					}
				}
			}
		}
		//2	左後部0
		{
			float tmp = 0.f;
			for (auto& f : t.wheelframe) {
				if (f.LorR) {
					t.square[0] = f.id;
					tmp = f.pos.z();
					break;
				}
			}
			for (auto& f : t.wheelframe) {
				if (t.square[0] != f.id) {
					if (f.LorR) {
						if (tmp < f.pos.z()) {
							t.square[0] = f.id;
							tmp = f.pos.z();
						}
					}
				}
			}
		}
		//10	左前部1
		{
			float tmp = 0.f;
			for (auto& f : t.wheelframe) {
				if (f.LorR) {
					t.square[1] = f.id;
					tmp = f.pos.z();
					break;
				}
			}
			for (auto& f : t.wheelframe) {
				if (t.square[1] != f.id) {
					if (f.LorR) {
						if (tmp > f.pos.z()) {
							t.square[1] = f.id;
							tmp = f.pos.z();
						}
					}
				}
			}
		}
		//3	右後部2
		{
			float tmp = 0.f;
			for (auto& f : t.wheelframe) {
				if (!f.LorR) {
					t.square[2] = f.id;
					tmp = f.pos.z();
					break;
				}
			}
			for (auto& f : t.wheelframe) {
				if (t.square[2] != f.id) {
					if (!f.LorR) {
						if (tmp < f.pos.z()) {
							t.square[2] = f.id;
							tmp = f.pos.z();
						}
					}
				}
			}
		}
		//11	右前部3
		{
			float tmp = 0.f;
			for (auto& f : t.wheelframe) {
				if (!f.LorR) {
					t.square[3] = f.id;
					tmp = f.pos.z();
					break;
				}
			}
			for (auto& f : t.wheelframe) {
				if (t.square[3] != f.id) {
					if (!f.LorR) {
						if (tmp > f.pos.z()) {
							t.square[3] = f.id;
							tmp = f.pos.z();
						}
					}
				}
			}
		}
		//
		for (int i = 0; i < t.col.frame_num(); i++) {
			std::string p = MV1GetFrameName(t.col.get(), i);
			if (p.find("旋回", 0) != std::string::npos) {
				t.gunframe_col.resize(t.gunframe_col.size() + 1);
				auto& e = t.gunframe_col.back();
				{
					e.id = i;
					e.pos = t.col.frame(e.id);
					auto p2 = t.col.frame_parent(e.id);
					if (p2 >= 0) {
						e.pos -= t.col.frame(int(p2));//親がいる時引いとく
					}
					if (MV1GetFrameChildNum(t.col.get(), e.id) >= 0) {
						if (std::string(MV1GetFrameName(t.col.get(), e.id + 1)).find("仰角", 0) != std::string::npos) {
							e.id2 = e.id + 1;
							e.pos2 = t.col.frame(e.id2) - t.col.frame(e.id);
							if (MV1GetFrameChildNum(t.col.get(), e.id) >= 0) {
								e.id3 = e.id2 + 1;
								e.pos3 = t.col.frame(e.id3) - t.col.frame(e.id2);
							}
							else {
								e.id3 = -1;
							}
						}
					}
					else {
						e.id2 = -1;
					}
				}
			}
		}

		for (int i = 0; i < t.col.mesh_num(); i++) {
			std::string p = MV1GetMaterialName(t.col.get(), i);
			if (p.find("armer", 0) != std::string::npos) {
				t.armer_mesh.resize(t.armer_mesh.size() + 1);
				t.armer_mesh.back().mesh = i;
				t.armer_mesh.back().thickness = std::stof(getright(p.c_str()));
			}
			else if (p.find("space", 0) != std::string::npos) {
				t.space_mesh.resize(t.space_mesh.size() + 1);
				t.space_mesh.back() = i;
			}
			else {
				t.module_mesh.resize(t.module_mesh.size() + 1);
				t.module_mesh.back() = i;
			}
		}
	}
	//キャラ
	std::vector<hit::Chara> chara;
	//
	MV1::Load("data/map/model.mv1", &map);	//map
	MV1::Load("data/map/col.mv1", &map_col);//mapコリジョン
	{
		MV1SetMaterialDrawAlphaTestAll(map.get(), TRUE, DX_CMP_GREATER, 128);
		VECTOR_ref max = MV1GetMeshMinPosition(map.get(), 0);
		Drawparts->Set_Shadow(13, VGet(50.f, 50.f, 50.f), max, VGet(0.5f, -0.5f, 0.5f), [&map] {map.DrawModel(); });
	}
	{
		VECTOR_ref size = (VECTOR_ref(MV1GetMeshMinPosition(map_col.get(), 0)) - MV1GetMeshMaxPosition(map_col.get(), 0)).Scale(0.1f);
		map_col.SetupCollInfo(int(size.x()), int(size.y()), int(size.z()), -1, 0);
	}
	MV1::Load("data/sky/model.mv1", &sky);	//空
	chara.resize(3);
	{
		auto& c = chara[0];
		c.id = 0;
		c.useid = 0;
		c.pos = VGet(-0.f, 1.81f, -2.48f);
	}
	{
		auto& c = chara[1];
		c.id = 1;
		c.useid = 0;
		c.pos = VGet(-5.f, 1.81f, 100.f - 2.48f);
	}
	{
		auto& c = chara[2];
		c.id = 2;
		c.useid = 0;
		c.pos = VGet(-10.f, 1.81f, 300.f - 2.48f);
	}
	for (auto& c : chara) {
		auto& t = tank[c.useid];
		c.usetank.into(t);//データ代入
		for (int i = 0; i < c.usetank.col.mesh_num(); i++) {
			c.usetank.col.SetupCollInfo(8, 8, 8, -1, i);
		}
		c.Gun.resize(c.usetank.gunframe.size());
		c.yvect = VGet(0.f, 1.f, 0.f);
		std::fill(c.key.begin(), c.key.end(), false);
		c.gndsmkeffcs.resize(c.usetank.wheelframe.size());
		c.gndsmksize.resize(c.usetank.wheelframe.size());

		c.hitssort.resize(c.usetank.col.mesh_num());
		c.hitres.resize(c.usetank.col.mesh_num());
		c.HP.resize(c.usetank.col.mesh_num());
		for (auto& h : c.hit) {
			h.flug = false;
			h.pic = hit_pic.Duplicate();
			h.use = 0;
			h.scale = VGet(1.f, 1.f, 1.f);
			h.pos = VGet(1.f, 1.f, 1.f);
		}
	}
	//カメラ
	float eye_xrad = 0.f;
	float eye_yrad = 0.f;
	VECTOR_ref eyevec = VGet(0.f, 0.f, 1.f);
	VECTOR_ref campos = chara[0].pos + VGet(0.f, 3.f, 0.f) + eyevec.Scale(range);
	VECTOR_ref vec_a = eyevec;
	//操作
	ads = false;
	locktrt = false;
	Rot = 0;
	ratio = 1.f;
	range = 1.f;
	range_p = 5.f;
	//開始
	SetMousePoint(dispx / 2, dispy / 2);
	for (auto& t : chara) {
		for (auto& g : t.gndsmkeffcs) {
			g.handle = Drawparts->get_gndhitHandle().Play3D();
		}
		//t.effcs[ef_smoke2].handle = Drawparts->get_effHandle(ef_smoke2).Play3D();
		//t.effcs[ef_smoke3].handle = Drawparts->get_effHandle(ef_smoke2).Play3D();
	}
	while (ProcessMessage() == 0) {
		const auto fps = GetFPS();
		const auto waits = GetNowHiPerformanceCount();
		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) { break; }
		Debugparts->put_way();
		//視点
		{
			//スコープ
			auto oldads = ads;
			Rot = std::clamp(Rot + GetMouseWheelRotVol(), 0, 5);
			switch (Rot) {
			case 5:
				ads = true;
				ratio = 10.f;
				range_p = 1.f;
				break;
			case 4:
				ads = true;
				ratio = 5.f;
				range_p = 1.f;
				break;
			case 3:
				ads = true;
				ratio = 1.f;
				range_p = 1.f;
				break;
			case 2:
				ads = false;
				ratio = 1.f;
				range_p = 1.f;
				break;
			case 1:
				ads = false;
				ratio = 1.f;
				range_p = 5.f;
				break;
			case 0:
				ads = false;
				ratio = 1.f;
				range_p = 10.f;
				break;
			}
			if (ads) {
				range = range_p;
			}
			else {
				range += (range_p - range)*0.05f;
			}
			if (ads != oldads) {
				eyevec = vec_a;
			}
			//砲塔ロック
			locktrt = ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0);
			//
			auto& c = chara[0];
			//砲塔旋回
			{
				if (!locktrt) {
					//狙い
					if (ads) {
						vec_a = eyevec;
					}
					else {
						VECTOR_ref endpos = campos - eyevec.Scale(1000.f);
						auto hp = map_col.CollCheck_Line(campos, endpos, -1, 0);
						if (hp.HitFlag == TRUE) {
							endpos = hp.HitPosition;
							vec_a = (c.usetank.obj.frame(c.usetank.gunframe[0].id2) - endpos).Norm();
						}
						for (auto& t : chara) {
							if (c.id == t.id || (Segment_Point_MinLength(campos.get(), endpos.get(), t.pos.get()) > 5.f)) {
								continue;
							}
							t.usetank.col.SetPosition(t.pos);
							t.usetank.col.SetRotationZYAxis(t.zvec, t.yvec, 0.f);
							for (int i = 0; i < t.usetank.col.mesh_num(); i++) {
								MV1RefreshCollInfo(t.usetank.col.get(), -1, i);
								const auto hp = t.usetank.col.CollCheck_Line(campos, endpos, -1, i);
								if (hp.HitFlag == TRUE) {
									endpos = hp.HitPosition;
								}
							}
						}
						vec_a = (c.usetank.obj.frame(c.usetank.gunframe[0].id2) - endpos).Norm();
					}

					VECTOR_ref zvector = c.usetank.obj.frame(c.usetank.gunframe[0].id3) - c.usetank.obj.frame(c.usetank.gunframe[0].id2);
					//yrad
					float cost = (vec_a.x()*(-zvector.z()) + vec_a.z() * (zvector.x())) / (std::hypotf(vec_a.x(), vec_a.z()) * std::hypotf((-zvector.z()), (zvector.x())));
					c.view_yrad = atan2f(cost, sqrt(abs(1.f - powf(cost, 2))));//cos取得2D
					//xrad
					c.view_xrad = atan2f(-zvector.y(), std::hypotf(zvector.x(), zvector.z())) - atan2f(vec_a.y(), std::hypotf(vec_a.x(), vec_a.z()));
				}
				else {
					c.view_yrad = 0.f;
					c.view_xrad = 0.f;
				}
			}
			//移動
			{
				c.key[0] = (CheckHitKey(KEY_INPUT_W) != 0);
				c.key[1] = (CheckHitKey(KEY_INPUT_S) != 0);
				c.key[2] = (CheckHitKey(KEY_INPUT_D) != 0);
				c.key[3] = (CheckHitKey(KEY_INPUT_A) != 0);
				c.key[4] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);//射撃
				c.key[5] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0);//マシンガン
			}
		}
		{
			int mousex, mousey;
			GetMousePoint(&mousex, &mousey);
			SetMousePoint(dispx / 2, dispy / 2);
			if (ads) {
				eye_xrad = std::clamp(eye_xrad + deg2rad(float(mousey - dispy / 2)*0.1f / ratio), deg2rad(-20), deg2rad(10));
				eye_yrad += deg2rad(float(mousex - dispx / 2)*0.1f / ratio);
			}
			else {
				eye_xrad = std::clamp(eye_xrad + deg2rad(float(mousey - dispy / 2)*0.1f), deg2rad(-25), deg2rad(89));
				eye_yrad += deg2rad(float(mousex - dispx / 2)*0.1f);
			}
			eyevec = VGet(cos(eye_xrad)*sin(eye_yrad), sin(eye_xrad), cos(eye_xrad)*cos(eye_yrad));
		}
		//反映
		for (auto& c : chara) {
			//砲塔旋回
			{
				float limit = deg2rad(c.usetank.turret_rad_limit) / fps;
				c.usetank.gunframe[0].yrad += std::clamp(c.view_yrad / 5.f, -limit, limit);
				//c.usetank.gunframe[0].yrad = std::clamp(c.usetank.gunframe[0].yrad + std::clamp(view_yrad / 5.f, -limit, limit),deg2rad(-30.0)+yrad,deg2rad(30.0)+yrad);//射界制限
				c.usetank.gunframe[0].xrad = std::clamp(c.usetank.gunframe[0].xrad + std::clamp(c.view_xrad / 5.f, -limit, limit), deg2rad(-10), deg2rad(20));		//俯角仰角
				if (c.usetank.gunframe.size() > 1) {
					c.usetank.gunframe[1].xrad = std::clamp(c.usetank.gunframe[1].xrad + std::clamp(c.view_xrad / 5.f, -limit, limit), deg2rad(-10), deg2rad(20));
				}
				//仮
				c.usetank.gunframe_col[0].xrad = c.usetank.gunframe[0].xrad;
				c.usetank.gunframe_col[0].yrad = c.usetank.gunframe[0].yrad;
			}
			//反映
			for (auto& f : c.usetank.gunframe) {
				c.usetank.obj.SetFrameLocalMatrix(f.id, RotY(f.yrad)*f.pos.Mtrans());
				if (f.id2 >= 0) {
					c.usetank.obj.SetFrameLocalMatrix(f.id2, RotX(f.xrad)*f.pos2.Mtrans());
				}
				if (f.id3 >= 0) {
					c.usetank.obj.SetFrameLocalMatrix(f.id3, VECTOR_ref(VGet(0.f, 0.f, c.Gun[0].fired)).Mtrans()*f.pos3.Mtrans());//リコイル
				}
			}
			for (auto& f : c.usetank.gunframe_col) {
				c.usetank.col.SetFrameLocalMatrix(f.id, RotY(f.yrad)*f.pos.Mtrans());
				if (f.id2 >= 0) {
					c.usetank.col.SetFrameLocalMatrix(f.id2, RotX(f.xrad)*f.pos2.Mtrans());
				}
				if (f.id3 >= 0) {
					c.usetank.col.SetFrameLocalMatrix(f.id3, VECTOR_ref(VGet(0.f, 0.f, c.Gun[0].fired)).Mtrans()*f.pos3.Mtrans());//リコイル
				}
			}
			//転輪
			for (auto& f : c.usetank.wheelframe) {
				MATRIX_ref tmp;
				MV1ResetFrameUserLocalMatrix(c.usetank.obj.get(), f.id);
				auto startpos = c.usetank.obj.frame(f.id);
				auto hp = map_col.CollCheck_Line(startpos + c.yvec.Scale((-f.pos.y()) + 2.f), startpos + c.yvec.Scale((-f.pos.y()) - 0.3f), -1, 0);
				if (hp.HitFlag == TRUE) {
					tmp = VECTOR_ref(VGet(0.f, hp.HitPosition.y + c.yvec.y()*f.pos.y() - startpos.y(), 0.f)).Mtrans();
				}
				else {
					tmp = VECTOR_ref(VGet(0.f, -0.3f, 0.f)).Mtrans();
				}
				c.usetank.obj.SetFrameLocalMatrix(f.id, RotX(f.LorR ? c.wheel_Left : c.wheel_Right)*tmp*f.pos.Mtrans());
			}
			c.yvect += (((c.usetank.obj.frame(c.usetank.square[0]) - c.usetank.obj.frame(c.usetank.square[3])) * (c.usetank.obj.frame(c.usetank.square[1]) - c.usetank.obj.frame(c.usetank.square[2]))).Norm() - c.yvect).Scale(0.05f);
			//移動
			auto hp = map_col.CollCheck_Line(c.pos + VGet(0.f, 2.f, 0.f), c.pos - VGet(0.f, 0.05f, 0.f), -1, 0);
			auto isfloat = (c.pos.y() == -c.usetank.down_in_water / 2.f);

			float FR = 0.f, LR = 0.f;

			//Z、Yベクトル取得
			{
				if (c.usetank.isfloat && isfloat) {
					c.yvec = VGet(0.f, 1.f, 0.f);
					c.zvec = VGet(sinf(c.yrad), 0.f, cosf(c.yrad));
				}
				else {
					MATRIX_ref zvm = MGetRotVec2(VGet(0.f, 1.f, 0.f), c.yvect.get());
					c.yvec = c.yvect;
					c.zvec = VTransform(VGet(sinf(c.yrad), 0.f, cosf(c.yrad)), zvm.get());
				}
			}
			if (hp.HitFlag == TRUE || (c.usetank.isfloat && isfloat)) {
				//前進後退
				{
					FR = (c.zadd_flont + c.zadd_back);
					if (c.key[0]) {
						if (c.zadd_flont < (c.usetank.flont_speed_limit / 3.6f)) {
							c.zadd_flont += (0.03f / 3.6f)*(420.f / fps);
						}
						if (c.zadd_back < 0.f) {
							c.zadd_back += (0.1f / 3.6f)*(420.f / fps);
						}
					}
					if (c.key[1]) {
						if (c.zadd_back > (c.usetank.back_speed_limit / 3.6f)) {
							c.zadd_back -= (0.03f / 3.6f)*(420.f / fps);
						}
						if (c.zadd_flont > 0.f) {
							c.zadd_flont -= (0.1f / 3.6f)*(420.f / fps);
						}
					}
					if (!c.key[0] && !c.key[1]) {
						if (c.zadd_flont > 0.f) {
							c.zadd_flont -= (0.05f / 3.6f)*(420.f / fps);
						}
						else {
							c.zadd_flont = 0.f;
						}
						if (c.zadd_back < 0.f) {
							c.zadd_back += (0.05f / 3.6f)*(420.f / fps);
						}
						else {
							c.zadd_back = 0.f;
						}
					}
					FR = (FR + ((c.zadd_flont + c.zadd_back) - FR)*(1.f - powf(0.9f, 60.f / fps))) / fps;
				}
				c.add = c.zvec.Scale(-FR);
				//旋回
				{
					c.yadd_right = (c.key[2]) ?
						std::min(c.yadd_right + deg2rad(0.5f*(420.f / fps)), deg2rad(c.usetank.body_rad_limit)) :
						std::max(c.yadd_right - deg2rad(0.3f*(420.f / fps)), 0.f);
					c.yadd_left = (c.key[3]) ?
						std::max(c.yadd_left - deg2rad(0.5f*(420.f / fps)), deg2rad(-c.usetank.body_rad_limit)) :
						std::min(c.yadd_left + deg2rad(0.3f*(420.f / fps)), 0.f);
					LR = (c.yadd_left + c.yadd_right) / fps;

					c.yrad += LR;
				}
				//慣性
				{
					auto xradold = c.xradp;
					c.xradp = deg2rad(-(FR / (420.f / fps)) / ((0.1f / 3.6f) / fps)*30.f);
					c.xrad += ((c.xradp - xradold) - c.xrad)*0.005f;
					c.xrad = std::clamp(c.xrad, deg2rad(-7.5f), deg2rad(7.5f));
					MATRIX_ref avm = MGetRotAxis((c.zvec*c.yvec).get(), c.xrad);
					c.yvec = VTransform(c.yvec.get(), avm.get());
					c.zvec = VTransform(c.zvec.get(), avm.get());
					auto zradold = c.zradp;
					c.zradp = deg2rad(-LR / (deg2rad(5.f) / fps)*30.f);
					c.zrad += ((c.zradp - zradold) - c.zrad)*0.005f;
					MATRIX_ref bvm = MGetRotAxis(c.zvec.get(), c.zrad);
					c.yvec = VTransform(c.yvec.get(), bvm.get());
					c.zvec = VTransform(c.zvec.get(), bvm.get());
				}
				if (hp.HitFlag == TRUE) {
					c.pos.yadd((hp.HitPosition.y - c.pos.y())*0.1f);
				}
			}
			else {
				c.add.yadd(-9.8f / powf(fps, 2.f));
			}
			//射撃
			{
				/*
				auto rad = deg2rad(-c.Gun[0].fired*10.f)*cos(c.usetank.gunframe[0].yrad);
				if (c.xrad_shot > rad) {
					c.xrad_shot -= deg2rad(10.f / fps);
				}
				else if (c.xrad_shot < rad) {
					c.xrad_shot += deg2rad(10.f / fps);
				}
				else {
					c.xrad_shot = 0.f;
				}
				//*/
				c.xrad_shot += (deg2rad(-c.Gun[0].fired*5.f)*cos(c.usetank.gunframe[0].yrad) - c.xrad_shot)*(1.f - powf(0.85f, 60.f / fps));
				MATRIX_ref avm = MGetRotAxis((c.zvec*c.yvec).get(), c.xrad_shot);
				c.yvec = VTransform(c.yvec.get(), avm.get());
				c.zvec = VTransform(c.zvec.get(), avm.get());
				//*
				c.zrad_shot += (deg2rad(-c.Gun[0].fired*5.f)*sin(c.usetank.gunframe[0].yrad) - c.zrad_shot)*(1.f - powf(0.85f, 60.f / fps));
				MATRIX_ref bvm = MGetRotAxis(c.zvec.get(), c.zrad_shot);
				c.yvec = VTransform(c.yvec.get(), bvm.get());
				c.zvec = VTransform(c.zvec.get(), bvm.get());
				//*/
			}

			c.pos += c.add;

			for (auto& p : c.gndsmksize) {
				p = 0.1f + abs(FR) / ((c.usetank.flont_speed_limit / 3.6f) / fps)*0.6f;
			}

			if (c.usetank.isfloat) {
				c.pos.y(std::max(c.pos.y(), -c.usetank.down_in_water / 2.f));
			}

			c.wheel_Left -= FR + LR * 5.f;
			c.wheel_Right -= FR - LR * 5.f;
			{
				auto& cg = c.Gun[0];
				if (c.key[4] && cg.loadcnt == 0) {
					cg.loadcnt = 5.f;
					cg.fired = 1.f;
					auto& a = cg.Ammo[cg.useammo];

					a.hit = false;
					a.color = Drawparts->GetColor(255, 255, 0);

					a.flug = true;
					a.cnt = 0.f;
					a.speed = 500.f;
					a.pene = 120.f;
					a.yadd = 0.f;
					a.pos = c.usetank.obj.frame(c.usetank.gunframe[0].id2);
					a.repos = a.pos;
					a.vec = (c.usetank.obj.frame(c.usetank.gunframe[0].id3) - c.usetank.obj.frame(c.usetank.gunframe[0].id2)).Norm();
					set_effect(&c.effcs[ef_fire], c.usetank.obj.frame(c.usetank.gunframe[0].id3), a.vec);

					cg.useammo++;
					cg.useammo %= cg.Ammo.size();
				}
				cg.loadcnt = std::max(cg.loadcnt - 1.f / fps, 0.f);
				cg.fired = std::max(cg.fired - 1.f / fps, 0.f);
			}
			if (c.usetank.gunframe.size() > 1) {
				auto& cg = c.Gun[1];
				if (c.key[5] && cg.loadcnt == 0) {
					cg.loadcnt = 0.16f;
					cg.fired = 1.f;
					auto& a = cg.Ammo[cg.useammo];

					a.hit = false;
					a.color = Drawparts->GetColor(255, 255, 0);

					a.flug = true;
					a.cnt = 0.f;
					a.speed = 500.f;
					a.pene = 120.f;
					a.yadd = 0.f;
					a.pos = c.usetank.obj.frame(c.usetank.gunframe[1].id2);
					a.repos = a.pos;
					a.vec = (c.usetank.obj.frame(c.usetank.gunframe[1].id3) - c.usetank.obj.frame(c.usetank.gunframe[1].id2)).Norm();
					set_effect(&c.effcs[ef_gun], c.usetank.obj.frame(c.usetank.gunframe[1].id3), a.vec);

					cg.useammo++;
					cg.useammo %= cg.Ammo.size();
				}
				cg.loadcnt = std::max(cg.loadcnt - 1.f / fps, 0.f);
				cg.fired = std::max(cg.fired - 1.f / fps, 0.f);
			}
			/*effect*/
			{
				for (int i = 0; i < efs_user; ++i)
					if (i != ef_smoke1 && i != ef_smoke2 && i != ef_smoke3) {
						set_pos_effect(&c.effcs[i], Drawparts->get_effHandle(i));
					}
				for (size_t i = 0; i < c.usetank.wheelframe.size(); i++) {
					c.gndsmkeffcs[i].handle.SetPos(c.usetank.obj.frame(c.usetank.wheelframe[i].id) + c.yvec.Scale(-c.usetank.wheelframe[i].pos.y()));
					c.gndsmkeffcs[i].handle.SetScale(c.gndsmksize[i]);
					//c.gndsmkeffcs[i].handle.SetColor
				}
				//c.effcs[ef_smoke1].handle.SetPos(c.usetank.obj.frame(c.ptr->engineframe));
				//c.effcs[ef_smoke2].handle.SetPos(c.usetank.obj.frame(c.ptr->smokeframe[0]));
				//c.effcs[ef_smoke3].handle.SetPos(c.usetank.obj.frame(c.ptr->smokeframe[1]));
			}
			//反映
			{
				c.ps_m = MGetAxis1((c.zvec* c.yvec).get(), c.yvec.get(), c.zvec.get(), c.pos.get());
				c.usetank.obj.SetPosition(c.pos);
				c.usetank.obj.SetRotationZYAxis(c.zvec, c.yvec, 0.f);

				c.usetank.col.SetPosition(c.pos);
				c.usetank.col.SetRotationZYAxis(c.zvec, c.yvec, 0.f);
				for (int i = 0; i < c.usetank.col.mesh_num(); i++) {
					MV1RefreshCollInfo(c.usetank.col.get(), -1, i);
				}
			}
		}
		for (auto& c : chara) {
			for (auto& a : c.Gun[0].Ammo) {
				if (a.flug) {
					a.repos = a.pos;
					a.pos += a.vec.Scale(a.speed / fps);
					a.pos.yadd(a.yadd);
					//*
					for (auto& t : chara) {
						if (c.id == t.id || (Segment_Point_MinLength(a.pos.get(), a.repos.get(), t.pos.get()) > a.speed)) {
							continue;
						}
						t.usetank.col.SetPosition(t.pos);
						t.usetank.col.SetRotationZYAxis(t.zvec, t.yvec, 0.f);
						for (int i = 0; i < t.usetank.col.mesh_num(); i++) {
							MV1RefreshCollInfo(t.usetank.col.get(), -1, i);
						}
					}
					//*/
					//判定
					const auto hp = map_col.CollCheck_Line(a.repos, a.pos, -1, 0);
					if (hp.HitFlag) {
						a.pos = hp.HitPosition;
					}
					if (!hit::get_reco(c, chara, a, 0)) {
						if (hp.HitFlag) {
							set_effect(&c.effcs[ef_gndhit + 0 * (ef_gndhit2 - ef_gndhit)], hp.HitPosition, hp.Normal);

							if ((a.vec.Norm() % hp.Normal) <= cos(deg2rad(60))) {
								a.flug = false;
							}
							else {
								a.vec += VScale(hp.Normal, (a.vec % hp.Normal) * -2.f);
								a.vec = a.vec.Norm();
								a.pos = a.vec.Scale(0.01f) + hp.HitPosition;
								a.pene /= 2.0f;
							}
						}
					}
					a.yadd += -9.8f / powf(fps, 2.f);
					a.cnt += 1.f / fps;
					a.pene -= 1.0f / fps;
					a.speed -= 5.f / fps;
					//消す
					if (a.cnt >= 3.f || a.speed < 100.f || a.pene <= 0.f) {
						a.flug = false;
					}
				}
			}
			for (auto& a : c.Gun[1].Ammo) {
				if (a.flug) {
					a.repos = a.pos;
					a.pos += a.vec.Scale(a.speed / fps);
					a.pos.yadd(a.yadd);

					for (auto& t : chara) {
						if (c.id == t.id || (Segment_Point_MinLength(a.pos.get(), a.repos.get(), t.pos.get()) > a.speed)) {
							continue;
						}
						t.usetank.col.SetPosition(t.pos);
						t.usetank.col.SetRotationZYAxis(t.zvec, t.yvec, 0.f);
						for (int i = 0; i < t.usetank.col.mesh_num(); i++) {
							MV1RefreshCollInfo(t.usetank.col.get(), -1, i);
						}
					}

					//判定
					const auto hp = map_col.CollCheck_Line(a.repos, a.pos, -1, 0);
					if (hp.HitFlag) {
						a.pos = hp.HitPosition;
					}
					if (!hit::get_reco(c, chara, a, 1)) {
						if (hp.HitFlag) {

							set_effect(&c.effcs[ef_gndhit + 1 * (ef_gndhit2 - ef_gndhit)], hp.HitPosition, hp.Normal);

							if ((a.vec.Norm() % hp.Normal) <= cos(deg2rad(60))) {
								a.flug = false;
							}
							else {
								a.vec += VScale(hp.Normal, (a.vec % hp.Normal) * -2.f);
								a.vec = a.vec.Norm();
								a.pos = a.vec.Scale(0.01f) + hp.HitPosition;
								a.pene /= 2.0f;
							}
						}
					}
					a.yadd += -9.8f / powf(fps, 2.f);
					a.cnt += 1.f / fps;
					a.pene -= 1.0f / fps;
					a.speed -= 5.f / fps;
					//消す
					if (a.cnt >= 3.f || a.speed < 100.f || a.pene <= 0.f) {
						a.flug = false;
					}
				}
			}
		}
		UpdateEffekseer3D(); //2.0ms
		Effekseer_Sync3DSetting();
		//描画
		Drawparts->SetDraw_Screen(SkyScreen);
		{
			//カメラ
			if (ads) {
				SetCameraNearFar(1000.0f, 5000.0f);
				SetupCamera_Perspective(deg2rad(90) / ratio);
				SetCameraPositionAndTargetAndUpVec(eyevec.get(), VGet(0, 0, 0), chara[0].yvec.get());
			}
			else {
				SetCameraNearFar(1000.0f, 5000.0f);
				SetupCamera_Perspective(deg2rad(90));
				SetCameraPositionAndTargetAndUpVec(eyevec.get(), VGet(0, 0, 0), VGet(0.f, 1.f, 0.f));
			}
			SetFogEnable(FALSE);
			SetUseLighting(FALSE);
			sky.DrawModel();
			SetUseLighting(TRUE);
			SetFogEnable(TRUE);
		}
		Drawparts->SetDraw_Screen(DX_SCREEN_BACK);
		{
			//カメラ
			if (ads) {
				campos = chara[0].usetank.obj.frame(chara[0].usetank.gunframe[0].id) + VTransform(chara[0].usetank.gunframe[0].pos2.get(), MGetRotY(eye_yrad));
				VECTOR_ref endpos = campos - eyevec;

				SetCameraNearFar(1.0f, 2000.0f);
				SetupCamera_Perspective(deg2rad(90) / ratio);
				SetCameraPositionAndTargetAndUpVec(campos.get(), endpos.get(), chara[0].yvec.get());
			}
			else {
				VECTOR_ref endpos = chara[0].pos + VGet(0.f, 3.f, 0.f);
				endpos.y(std::max(endpos.y(), 5.f));
				campos = endpos + eyevec.Scale(range);
				campos.y(std::max(campos.y(), 0.f));

				auto hp = map_col.CollCheck_Line(endpos, campos, -1, 0);
				if (hp.HitFlag == TRUE) {
					campos = endpos + (VECTOR_ref(hp.HitPosition) - endpos).Scale(0.9f);
				}

				SetCameraNearFar(1.0f, 2000.0f);
				SetupCamera_Perspective(deg2rad(90));
				SetCameraPositionAndTargetAndUpVec(campos.get(), endpos.get(), VGet(0.f, 1.f, 0.f));
			}


			Drawparts->Ready_Shadow(chara[0].pos, [&map, &chara, &ads] {
				for (auto& t : chara) {
					if ((!ads && t.id == 0) || t.id != 0) {
						t.usetank.obj.DrawModel();
					}
				}
			});

			{
				SkyScreen.DrawGraph(0, 0, FALSE);
				Drawparts->Draw_by_Shadow([&map, &chara, &ads, &Vertex] {
					SetFogStartEnd(0.0f, 1000.f);
					SetFogColor(128, 128, 128);
					{
						map.DrawModel();
						for (auto& t : chara) {
							if ((!ads && t.id == 0) || t.id != 0) {
								t.usetank.obj.DrawModel();
							}
						}
						for (auto& t : chara) {
							for (auto& h : t.hit) {
								if (h.flug) {
									VECTOR_ref pos = VTransform(h.pos.get(), t.ps_m);
									VECTOR_ref zvec = VTransform(h.zvec.get(), t.ps_m);
									VECTOR_ref yvec = VTransform(h.yvec.get(), t.ps_m);
									MV1SetScale(h.pic.get(), h.scale.get());
									h.pic.SetRotationZYAxis(yvec - pos, zvec - pos, 0.f);
									h.pic.SetPosition(pos + (zvec - pos).Scale(0.005f));
									h.pic.DrawFrame(h.use);
								}
							}
						}

					}
					SetFogStartEnd(0.0f, 1000.f);
					SetFogColor(128, 192, 255);
					{
						SetDrawBlendMode(DX_BLENDMODE_ALPHA, 245);			// 描画ブレンドモードをアルファブレンドにセット
						DrawPolygon3D(Vertex, 2, DX_NONE_GRAPH, FALSE);
						SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);			// 描画ブレンドモードを戻す
					}
				});

				DrawEffekseer3D();

				SetFogEnable(FALSE);
				SetUseLighting(FALSE);
				for (auto& c : chara) {
					for (auto& a : c.Gun[0].Ammo) {
						if (a.flug) {
							//SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f * std::clamp<float>(a.speed / 300.f, 0.f, 1.f)));
							if (a.hit) {
								DrawCapsule3D(a.pos.get(), a.repos.get(), 0.105f * ((a.pos - campos).size() / 60.f), 4, a.color, Drawparts->GetColor(255, 255, 255), TRUE);
							}
							else {
								DrawCapsule3D(a.pos.get(), (a.pos + (a.pos - a.repos).Norm().Scale(fps)).get(), 0.105f * ((a.pos - campos).size() / 60.f), 4, a.color, Drawparts->GetColor(255, 255, 255), TRUE);
							}
						}
					}
					for (auto& a : c.Gun[1].Ammo) {
						if (a.flug) {
							//SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f * std::clamp<float>(a.speed / 300.f, 0.f, 1.f)));
							DrawCapsule3D(a.pos.get(), a.repos.get(), 0.0076f * ((a.pos - campos).size() / 12.f), 4, a.color, Drawparts->GetColor(255, 255, 255), TRUE);
						}
					}
				}
				SetUseLighting(TRUE);
				SetFogEnable(TRUE);
				//SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);			// 描画ブレンドモードを戻す


				DrawLine3D(chara[0].usetank.obj.frame(chara[0].usetank.gunframe[0].id2).get(),
					(chara[0].usetank.obj.frame(chara[0].usetank.gunframe[0].id2) +
					(chara[0].usetank.obj.frame(chara[0].usetank.gunframe[0].id3) - chara[0].usetank.obj.frame(chara[0].usetank.gunframe[0].id2)).Scale(1000.f)).get(),
					GetColor(255, 0, 0)
				);
			}




			//font18.DrawStringFormat(100, 100, Drawparts->GetColor(255, 255, 255), "%d", Rot);
			Debugparts->end_way();
			Debugparts->debug(10, 10, fps, float(GetNowHiPerformanceCount() - waits) / 1000.f);
		}
		Drawparts->Screen_Flip(waits);
	}
	return 0; // ソフトの終了
}

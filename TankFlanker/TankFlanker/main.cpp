#include "sub.hpp"
#include "UI.hpp"
#include "HostPass.hpp"
#include "map.hpp"
#include "VR.hpp"
#include "debug.hpp"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	//設定読み込み
	bool dof_e = false;
	bool bloom_e = false;
	bool shadow_e = false;
	bool useVR_e = true;
	int dispx = 1080; /*描画X*/
	int dispy = 1200; /*描画Y*/
	int out_dispx = dispx * 960 / dispy; /*ウィンドウX*/
	int out_dispy = dispy * 960 / dispy; /*ウィンドウY*/
	{
		SetOutApplicationLogValidFlag(FALSE);  /*log*/
		int mdata = FileRead_open("data/setting.txt", FALSE);
		dof_e = getparam_bool(mdata);
		bloom_e = getparam_bool(mdata);
		shadow_e = getparam_bool(mdata);
		useVR_e = getparam_bool(mdata);
		FileRead_close(mdata);
	}
	//DXLib描画
	auto vrparts = std::make_unique<VRDraw>(&useVR_e);
	//画面指定
	if (useVR_e) {
		dispx = 1080;
		dispy = 1200;
		out_dispx = dispx * 960 / dispy;
		out_dispy = dispy * 960 / dispy;
	}
	else {
		dispx = 1920;
		dispy = 1080;
		out_dispx = dispx;
		out_dispy = dispy;
	}
	//
	auto Drawparts = std::make_unique<DXDraw>("TankFlanker", dispx, dispy, 90.f);		 /*汎用クラス*/
	auto UIparts = std::make_unique<UI>(out_dispx, out_dispy, dispx, dispy);		 /*UI*/
	auto Debugparts = std::make_unique<DeBuG>(90);						 /*デバッグ*/
	auto Hostpassparts = std::make_unique<HostPassEffect>(dof_e, bloom_e, dispx, dispy);	 /*ホストパスエフェクト*/

	if (useVR_e) {
		SetWindowSize(out_dispx, out_dispy);
		SetWindowPosition((deskx - out_dispx) / 2, 0);
	}
	/*BOX2D*/
	auto world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f)); /* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
	VECTOR_ref eyevec;					    //視点
	VECTOR_ref campos, camvec, camup;			    //カメラ
	std::array<VECTOR_ref, veh_all> aimpos;			    //機体の狙い
	VECTOR_ref aimposout;					    //UIに出力
	GraphHandle BufScreen = GraphHandle::Make(dispx, dispy);    //描画スクリーン
	GraphHandle outScreen = GraphHandle::Make(dispx, dispy);    //描画スクリーン
	/*map*/
	auto mapparts = std::make_unique<Mapclass>(dispx, dispy);
	//その他
	MV1 hit_pic;      //弾痕
	//操作
	float fov = deg2rad(useVR_e ? 90 : 45);
	bool ads = false;
	int Rot = 0;
	float ratio = 1.f;
	float range = 0.f;
	float range_p = 0.f;
	uint8_t change_vehicle = 0;
	bool chveh = false;

	VECTOR_ref eye_pos_ads = VGet(0.f, 0.58f, 0.f);
	//ロックオン
	VECTOR_ref aimposout_lockon;
	switchs lock_on = { false, uint8_t(0) };
	size_t tgt = 0;
	float distance = 0.f;
	//データ
	std::vector<Mainclass::Chara> chara;						//キャラ
	std::vector<Mainclass::wallPats> wall;					//壁をセット
	std::vector<Mainclass::treePats> tree;					//壁をセット
	MV1::Load("data/model/hit/model.mv1", &hit_pic, true);			//弾痕
	std::vector<Mainclass::Ammos> Ammo;						//弾薬
	std::array<std::vector<Mainclass::Vehcs>, veh_all> Vehicles;			/*車輛データ*/
	Mainclass::Vehcs::set_vehicles_pre("data/tank/", &Vehicles[0], true);		/**/
	Mainclass::Vehcs::set_vehicles_pre("data/plane/", &Vehicles[1], true);		/**/
	Mainclass::Vehcs::set_vehicles_pre("data/carrier/", &Vehicles[2], true);	/**/
	UIparts->load_window("車両モデル");					//ロード画面
	Mainclass::Ammos::set_ammos(&Ammo);							//弾薬
	Mainclass::Vehcs::set_vehicles(&Vehicles);					//車輛
	vrparts->Set_Device();
	VECTOR_ref HMDpos, HMDxvec, HMDyvec, HMDzvec;
	bool HMDon;

	//ココから繰り返し読み込み//-------------------------------------------------------------------
	bool ending = true;
	do {
		//キャラ選択
		chara.resize(1);
		if (!UIparts->select_window(&chara[0], &Vehicles)) {
			break;
		}
		//マップ読み込み
		mapparts->set_map_pre();
		UIparts->load_window("マップモデル");			   //ロード画面
		//壁
		mapparts->set_map(&wall, &tree, world);
		//
		Drawparts->Set_light(VGet(0.0f, -0.5f, 0.5f));
		if (shadow_e) {
			Drawparts->Set_Shadow(13, mapparts->map_get().mesh_minpos(0), VGet(0.0f, -0.5f, 0.5f), [&mapparts] { mapparts->map_get().DrawModel(); });
		}
		//キャラ選択
		{
			//戦車
			chara.back().vehicle[0].pos = VGet(0.f, 1.81f, -2.48f);
			chara.back().vehicle[0].mat = MATRIX_ref::RotY(deg2rad(270.f));
			//空母
			chara.back().vehicle[2].pos = VGet(-120.f, 0.f, -1500.f);
			chara.back().vehicle[2].mat = MATRIX_ref::RotY(deg2rad(0));
			//t.obj.SetFrameLocalMatrix(chara.back().vehicle[2].use_veh.catapult[0].first + 2, MATRIX_ref::RotX(deg2rad(-75)) * MATRIX_ref::Mtrans(chara.back().vehicle[2].use_veh.catapult[0].second));
			//飛行機
			auto pp = Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first + 1) - Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first);
			chara.back().vehicle[1].pos = chara.back().vehicle[2].pos + Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first) + VGet(0.f, 5.f, 0.f);
			chara.back().vehicle[1].mat = MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
		}
		for (int i = 0; i < 2; i++) {
			chara.resize(chara.size() + 1);
			//戦車
			chara.back().vehicle[0].use_id = 0;
			chara.back().vehicle[0].pos = VGet(10.f, 1.81f, -2.48f + float(i * 14) - 300.f);
			chara.back().vehicle[0].mat = MATRIX_ref::RotY(deg2rad(270.f));
			chara.back().vehicle[0].camo_sel = GetRand(5);
			//空母
			chara.back().vehicle[2].pos = VGet(0.f, 0.f, -1500.f - float(i * 400));
			chara.back().vehicle[2].mat = MATRIX_ref::RotY(deg2rad(0));
			//飛行機
			chara.back().vehicle[1].use_id = 0;
			auto pp = Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first + 1) - Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first);
			chara.back().vehicle[1].pos = chara.back().vehicle[2].pos + Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first) + VGet(0.f, 5.f, 0.f);

			chara.back().vehicle[1].mat = MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
			chara.back().vehicle[1].camo_sel = GetRand(5);
			//
			chara.back().mode = 0;
		}
		for (int i = 0; i < 2; i++) {
			chara.resize(chara.size() + 1);
			//戦車
			chara.back().vehicle[0].use_id = 1;
			chara.back().vehicle[0].pos = VGet(0.f, 1.81f, -2.48f + float(i * 14) - 300.f);
			chara.back().vehicle[0].mat = MATRIX_ref::RotY(deg2rad(270.f));
			chara.back().vehicle[0].camo_sel = GetRand(5);
			//空母
			chara.back().vehicle[2].pos = VGet(120.f, 0.f, -1500.f - float(i * 400));
			chara.back().vehicle[2].mat = MATRIX_ref::RotY(deg2rad(0));
			//飛行機
			chara.back().vehicle[1].use_id = 0;
			auto pp = Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first + 1) - Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first);
			chara.back().vehicle[1].pos = chara.back().vehicle[2].pos + Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first) + VGet(0.f, 5.f, 0.f);
			chara.back().vehicle[1].mat = MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
			chara.back().vehicle[1].camo_sel = GetRand(5);
			//
			chara.back().mode = 0;
		}
		for (int i = 0; i < 2; i++) {
			chara.resize(chara.size() + 1);
			//戦車
			chara.back().vehicle[0].use_id = 2;
			chara.back().vehicle[0].pos = VGet(-10.f, 1.81f, -2.48f + float(i * 14) - 300.f);
			chara.back().vehicle[0].mat = MATRIX_ref::RotY(deg2rad(270.f));
			chara.back().vehicle[0].camo_sel = GetRand(5);
			//空母
			chara.back().vehicle[2].pos = VGet(240.f, 0.f, -1500.f - float(i * 400));
			chara.back().vehicle[2].mat = MATRIX_ref::RotY(deg2rad(0));
			//飛行機
			chara.back().vehicle[1].use_id = 0;
			auto pp = Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first + 1) - Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first);
			chara.back().vehicle[1].pos = chara.back().vehicle[2].pos + Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first) + VGet(0.f, 5.f, 0.f);
			chara.back().vehicle[1].mat = MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
			chara.back().vehicle[1].camo_sel = GetRand(5);
			//
			chara.back().mode = 0;
		}
		for (int i = 0; i < 2; i++) {
			chara.resize(chara.size() + 1);
			//戦車
			chara.back().vehicle[0].use_id = 3;
			chara.back().vehicle[0].pos = VGet(-20.f, 1.81f, -2.48f + float(i * 14) - 300.f);
			chara.back().vehicle[0].mat = MATRIX_ref::RotY(deg2rad(270.f));
			chara.back().vehicle[0].camo_sel = GetRand(5);
			//空母
			chara.back().vehicle[2].pos = VGet(360.f, 0.f, -1500.f - float(i * 400));
			chara.back().vehicle[2].mat = MATRIX_ref::RotY(deg2rad(0));
			//飛行機
			chara.back().vehicle[1].use_id = 0;
			auto pp = Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first + 1) - Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first);
			chara.back().vehicle[1].pos = chara.back().vehicle[2].pos + Vehicles[2][0].obj.frame(Vehicles[2][0].catapult[0].first) + VGet(0.f, 5.f, 0.f);
			chara.back().vehicle[1].mat = MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
			chara.back().vehicle[1].camo_sel = GetRand(5);
			//
			chara.back().mode = 0;
		}
		//キャラ設定
		fill_id(chara); //ID
		for (auto& c : chara) {
			c.set_human(Vehicles, Ammo, hit_pic, world);
		}
		//必要な時に当たり判定をリフレッシュする(仮)
		auto ref_col = [&chara](const Mainclass::Chara& p, const VECTOR_ref& startpos, const VECTOR_ref& endpos) {
			for (auto& c : chara) {
				{
					auto& veh = c.vehicle[0];
					if (p.id == c.id || veh.hit_check) {
						continue;
					}
					if ((Segment_Point_MinLength(startpos.get(), endpos.get(), veh.pos.get()) > 5.f)) {
						continue;
					}
					veh.col.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));
					for (int i = 0; i < veh.col.mesh_num(); i++) {
						veh.col.RefreshCollInfo(-1, i);
					}
					veh.hit_check = true;
				}
			}
			for (auto& c : chara) {
				{
					auto& veh = c.vehicle[1];
					if (p.id == c.id || veh.hit_check) {
						continue;
					}
					if ((Segment_Point_MinLength(startpos.get(), endpos.get(), veh.pos.get()) > 5.f)) {
						continue;
					}
					veh.col.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));
					for (int i = 0; i < veh.col.mesh_num(); i++) {
						veh.col.RefreshCollInfo(-1, i);
					}
					veh.hit_check = true;
				}
			}
			for (auto& c : chara) {
				{
					auto& veh = c.vehicle[2];
					if (p.id == c.id || veh.hit_check) {
						continue;
					}
					if ((Segment_Point_MinLength(startpos.get(), endpos.get(), veh.pos.get()) > 160.f)) {
						continue;
					}
					veh.col.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));
					for (int i = 0; i < veh.col.mesh_num(); i++) {
						veh.col.RefreshCollInfo(-1, i);
					}
					veh.hit_check = true;
				}
			}
			//
		};
		//影に描画するものを指定する(仮)
		auto draw_in_shadow = [&chara, &tree] {
			for (auto& c : chara) {
				for (auto& veh : c.vehicle) {
					veh.obj.DrawModel();
				}
			}
			for (auto& l : tree) {
				l.obj.DrawModel();
			}
		};
		auto draw_in_shadow_sky = [&chara] {
			for (auto& c : chara) {
				{
					auto& veh = c.vehicle[1];
					veh.obj.DrawModel();
				}
				{
					auto& veh = c.vehicle[2];
					veh.obj.DrawModel();
				}
			}
		};
		auto draw_on_shadow = [&mapparts, &chara, &ads, &tree, &campos, &vrparts,&ratio] {
			//マップ
			SetFogStartEnd(0.0f, 3000.f);
			SetFogColor(128, 128, 128);
			{
				mapparts->map_get().DrawModel();
				for (auto& c : chara) {
					c.vehicle[2].obj.DrawModel();
				}}
			//海
			mapparts->sea_draw(campos);
			//機体
			SetFogStartEnd(0.0f, 3000.f);
			SetFogColor(128, 128, 128);
			{
				for (auto& c : chara) {
					//戦車
					if ((!ads && c.id == 0) || c.id != 0) {
						MV1SetFrameTextureAddressTransform(c.vehicle[0].obj.get(), 0, -c.vehicle[0].wheel_Left*0.1f, 0.f, 1.f, 1.f, 0.5f, 0.5f, 0.f);
						c.vehicle[0].obj.DrawMesh(0);
						MV1SetFrameTextureAddressTransform(c.vehicle[0].obj.get(), 0, -c.vehicle[0].wheel_Right*0.1f, 0.f, 1.f, 1.f, 0.5f, 0.5f, 0.f);
						c.vehicle[0].obj.DrawMesh(1);
						MV1ResetFrameTextureAddressTransform(c.vehicle[0].obj.get(), 0);
						for (int i = 2; i < c.vehicle[0].obj.mesh_num(); i++) {
							c.vehicle[0].obj.DrawMesh(i);
						}
					}
					//戦闘機
					c.vehicle[1].obj.DrawModel();
					//弾痕
					for (auto& veh : c.vehicle) {
						for (auto& h : veh.hit_obj) {
							if (h.flug) {
								h.pic.DrawFrame(h.use);
							}
						}
					}
				}
				for (auto& l : tree) {
					l.obj.DrawModel();
					//l.obj_far.DrawModel();
				}
			}

			SetFogEnable(FALSE);
			SetUseLighting(FALSE);
			for (auto& c : chara) {
				for (auto& veh : c.vehicle) {
					for (auto& g : veh.Gun_) {
						for (auto& a : g.bullet) {
							if (a.flug) {
								DXDraw::Capsule3D(
									a.pos,
									a.repos,
									(((a.spec.caliber_a - 0.00762f) * 0.1f + 0.00762f) * ((a.pos - campos).size() / 24.f))*(1.f / std::max(ratio / 8.5f, 1.f)),
									a.color, GetColor(255, 255, 255));
							}
						}
					}
				}
			}
			SetUseLighting(TRUE);
			SetFogEnable(TRUE);
			vrparts->Draw_Player();
		};
		//通信開始
		{
		}
		//開始
		auto& mine = chara[0];
		Rot = 0;
		tgt = chara.size();
		eyevec = mine.vehicle[mine.mode].mat.zvec() * -1.f;
		campos = mine.vehicle[mine.mode].pos + VGet(0.f, 3.f, 0.f) + eyevec * (range);
		for (auto& c : chara) {
			for (auto& veh : c.vehicle) {
				for (auto& g : veh.use_veh.wheelframe) {
					g.gndsmkeffcs.handle = Drawparts->get_gndhitHandle().Play3D();
					g.gndsmksize = 0.1f;
				}
			}
		}
		SetMouseDispFlag(FALSE);
		SetMousePoint(dispx / 2, dispy / 2);
		while (ProcessMessage() == 0) {
			const auto fps = GetFPS();
			const auto waits = GetNowHiPerformanceCount();
			for (auto& c : chara) {
				{
					auto& veh = c.vehicle[0];
					//当たり判定リフレッシュ
					if (veh.hit_check) {
						veh.col.SetMatrix(MATRIX_ref::Mtrans(VGet(0.f, -100.f, 0.f)));
						for (int i = 0; i < veh.col.mesh_num(); i++) {
							veh.col.RefreshCollInfo(-1, i);
						}
						veh.hit_check = false;
					}
				}
				{
					auto& veh = c.vehicle[1];
					//当たり判定リフレッシュ
					if (veh.hit_check) {
						veh.col.SetMatrix(MATRIX_ref::Mtrans(VGet(0.f, -100.f, 0.f)));
						for (int i = 0; i < veh.col.mesh_num(); i++) {
							veh.col.RefreshCollInfo(-1, i);
						}
						veh.hit_check = false;
					}
				}
				/*
				for (auto& veh : c.vehicle) {
					//当たり判定リフレッシュ
					if (veh.hit_check) {
						veh.col.SetMatrix(MATRIX_ref::Mtrans(VGet(0.f, -100.f, 0.f)));
						for (int i = 0; i < veh.col.mesh_num(); i++) {
							veh.col.RefreshCollInfo(-1, i);
						}
						veh.hit_check = false;
					}
				}
				*/
				//空母は常時更新(あとで改善)
				{
					auto& veh = c.vehicle[2];
					/*
					if (veh.hit_check) {
						continue;
					}
					*/
					veh.col.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));
					for (int i = 0; i < veh.col.mesh_num(); i++) {
						veh.col.RefreshCollInfo(-1, i);
					}
					veh.hit_check = true;
				}
			}
			Debugparts->put_way();
			//プレイヤー操作
			{
				//スコープ
				{
					switch (mine.mode) {
					case 0:
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
						ads = (Rot >= 3);
						break;
					case 1:
						Rot = std::clamp(Rot + GetMouseWheelRotVol(), 0, 2);
						switch (Rot) {
						case 2:
							break;
						case 1:
							range_p = 15.f;
							break;
						case 0:
							range_p = 30.f;
							break;
						}
						ratio = 1.f;
						ads = (Rot >= 2);
						break;
					case 2:
						Rot = std::clamp(Rot + GetMouseWheelRotVol(), 0, 2);
						switch (Rot) {
						case 2:
							range_p = 150.f;
							break;
						case 1:
							range_p = 250.f;
							break;
						case 0:
							range_p = 400.f;
							break;
						}
						ratio = 1.f;
						ads = false;
						break;
					default:
						break;
					}
					if (ads) {
						range_p = 1.f;
					}
					easing_set(&range, range_p, (ads) ? 0.f : 0.95f, fps);
				}
				//砲塔旋回
				{
					lock_on.second = std::min<uint8_t>(lock_on.second + 1, ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) ? 2 : 0);
					if (lock_on.second == 1) {
						if (lock_on.first == true) {
							lock_on.first = false;
							lock_on.second = 2;
						}
					}
					if (lock_on.second == 1) {
						if (lock_on.first == false && tgt != chara.size()) {
							lock_on.first = true;
							lock_on.second = 2;
						}
					}

					if (((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0 && !lock_on.first) || mine.mode != 0) { //砲塔ロック
						mine.view_yrad = 0.f;
						mine.view_xrad = 0.f;
					}
					else {
						//狙い
						VECTOR_ref vec_a;
						{
							VECTOR_ref endpos = campos - eyevec * (2000.f);
							//マップに当たったか
							mapparts->map_col_line_nearest(campos, &endpos);
							//車輛に当たったか
							if (!lock_on.first) {
								tgt = chara.size();
							}
							ref_col(mine, campos, endpos);
							for (auto& t : chara) {
								auto& veh = t.vehicle[0];
								if (veh.hit_check) {
									for (int i = 0; i < veh.col.mesh_num(); i++) {
										const auto hp = veh.col.CollCheck_Line(campos, endpos, -1, i);
										if (hp.HitFlag == TRUE) {
											endpos = hp.HitPosition;
											if (!lock_on.first) {
												tgt = t.id;
											}
										}
									}
								}
							}
							vec_a = (mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first) - endpos).Norm();
						}
						if (ads) {
							vec_a = eyevec;
						}

						//ロックオン
						if (lock_on.first) {
							auto& c = chara[tgt];
							vec_a = (mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first) - c.vehicle[0].obj.frame(c.vehicle[0].use_veh.gunframe[0].frame1.first)).Norm();
						}
						//反映
						auto vec_z = mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame3.first) - mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first);
						float z_hyp = std::hypotf(vec_z.x(), vec_z.z());
						float a_hyp = std::hypotf(vec_a.x(), vec_a.z());
						float cost = (vec_a.z() * vec_z.x() - vec_a.x() * vec_z.z()) / (a_hyp * z_hyp);

						mine.view_yrad = (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f; //cos取得2D
						mine.view_xrad = (atan2f(-vec_z.y(), z_hyp) - atan2f(vec_a.y(), a_hyp)) / 5.f;
					}
				}
				//キー
				{
					//通常、VR共通
					{
						mine.key[0] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);   //射撃
						mine.key[1] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0); //マシンガン
						mine.key[2] = (CheckHitKey(KEY_INPUT_W) != 0);
						mine.key[3] = (CheckHitKey(KEY_INPUT_S) != 0);
						mine.key[4] = (CheckHitKey(KEY_INPUT_D) != 0);
						mine.key[5] = (CheckHitKey(KEY_INPUT_A) != 0);
						//飛行時のみの操作
						if (mine.mode == 1) {
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
					}
					if (useVR_e) {
						if (vrparts->get_left_hand_num() != -1) {
							auto& ptr_LEFTHAND = (*vrparts->get_device())[vrparts->get_left_hand_num()];
							//auto& ptr_RIGHTHAND = (*vrparts->get_device())[vrparts->get_right_hand_num()];
							if (mine.mode == 0) {
								if (ptr_LEFTHAND.turn && ptr_LEFTHAND.now) {
								}
							}
							if (mine.mode == 1) {
								if (ptr_LEFTHAND.turn && ptr_LEFTHAND.now) {
									//メイン
									mine.key[0] |= ((ptr_LEFTHAND.on[0] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_SteamVR_Trigger)) != 0);
									//サブ
									mine.key[1] |= ((ptr_LEFTHAND.on[1] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_IndexController_B)) != 0);
									//ピッチ
									mine.key[2] |= (ptr_LEFTHAND.yvec.y() > sinf(deg2rad(20)));
									mine.key[3] |= (ptr_LEFTHAND.yvec.y() < sinf(deg2rad(-20)));
									//ロール
									mine.key[4] |= (ptr_LEFTHAND.zvec.x() > sinf(deg2rad(20)));
									mine.key[5] |= (ptr_LEFTHAND.zvec.x() < sinf(deg2rad(-20)));
									if ((ptr_LEFTHAND.on[0] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_SteamVR_Touchpad)) != 0) {
										//ヨー
										mine.key[6] |= (ptr_LEFTHAND.touch.x() > 0.5f);
										mine.key[7] |= (ptr_LEFTHAND.touch.x() < -0.5f);
										//スロットル
										mine.key[8] |= (ptr_LEFTHAND.touch.y() > 0.5f);
										mine.key[9] |= (ptr_LEFTHAND.touch.y() < -0.5f);
										//ブレーキ
										if (
											(ptr_LEFTHAND.touch.x() >= -0.5f) &&
											(ptr_LEFTHAND.touch.x() <= 0.5f) &&
											(ptr_LEFTHAND.touch.y() >= -0.5f) &&
											(ptr_LEFTHAND.touch.y() <= 0.5f)
											) {
											mine.key[11] |= true;
										}
									}
									//脚

									//精密操作
									mine.key[12] |= ((ptr_LEFTHAND.on[1] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_IndexController_A)) != 0);
									//着艦フックスイッチ

									//カタパルト

								}
							}
							if (mine.mode == 2) {
								if (ptr_LEFTHAND.turn && ptr_LEFTHAND.now) {
								}
							}
						}
					}
					//他モードへの移行
					chveh = false;
					change_vehicle = std::clamp<uint8_t>(change_vehicle + 1, 0, int((CheckHitKey(KEY_INPUT_P) != 0) ? 2 : 0));
					if (change_vehicle == 1) {
						++mine.mode %= mine.vehicle.size();
						eyevec = mine.vehicle[mine.mode].mat.zvec();
						mine.vehicle[mine.mode].add = VGet(0.f, 0.f, 0.f);
						change_vehicle = 2;
						chveh = true;
					}
				}
				//マウスと視点角度をリンク
				if (useVR_e) {
					SetMousePoint(dispx / 2, dispy / 2);
					if (vrparts->get_hmd_num() != -1) {
						auto& ptr_HMD = (*vrparts->get_device())[vrparts->get_hmd_num()];
						HMDpos = ptr_HMD.pos;
						HMDxvec = ptr_HMD.xvec;
						HMDyvec = ptr_HMD.yvec;
						HMDzvec = ptr_HMD.zvec;
						HMDon = ptr_HMD.now;
					}
					else {
						HMDpos = VGet(0, 0, 0);
						HMDxvec = VGet(1, 0, 0);
						HMDyvec = VGet(0, 1, 0);
						HMDzvec = VGet(0, 0, 1);
						HMDon = false;
					}
					eye_pos_ads = HMDyvec * -0.5f + VGet(HMDpos.x()*-1.f, HMDpos.y(), HMDpos.z()*-1.f);
					eye_pos_ads = VGet(
						std::clamp(eye_pos_ads.x(), -0.18f, 0.18f),
						std::clamp(eye_pos_ads.y(), 0.f, 0.8f),
						std::clamp(eye_pos_ads.z(), -0.18f, 0.1f)

					);
					eyevec = VGet(HMDzvec.x(), HMDzvec.y()*-1.f, HMDzvec.z());
				}
				else {
					int mousex, mousey;
					GetMousePoint(&mousex, &mousey);
					SetMousePoint(dispx / 2, dispy / 2);
					if (mine.mode == 0) {
						if (ads) {
							float y = atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - dispx / 2) * 0.1f / ratio);
							float x = atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - dispy / 2) * 0.1f / ratio);
							x = std::clamp(x, deg2rad(-20), deg2rad(10));
							eyevec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
						}
						else {
							float y = atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - dispx / 2) * 0.1f);
							float x = atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - dispy / 2) * 0.1f);
							x = std::clamp(x, deg2rad(-25), deg2rad(89));
							eyevec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
						}
					}
					if (mine.mode == 1) {
						if (ads) {
							float y = atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - dispx / 2) * 0.1f / ratio);
							float x = atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - dispy / 2) * 0.1f / ratio);
							x = std::clamp(x, deg2rad(-45), deg2rad(45));
							eyevec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
						}
						else {
							float y = atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - dispx / 2) * 0.1f);
							float x = atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - dispy / 2) * 0.1f);
							x = std::clamp(x, deg2rad(-25), deg2rad(89));
							eyevec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
						}
					}
					if (mine.mode == 2) {
						float y = atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - dispx / 2) * 0.1f);
						float x = atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - dispy / 2) * 0.1f);
						x = std::clamp(x, deg2rad(-25), deg2rad(89));
						eyevec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
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
					auto& veh = c.vehicle[0];
					//砲塔旋回
					{
						float limit = deg2rad(c.vehicle[0].use_veh.turret_rad_limit) / fps;
						veh.Gun_[0].gun_info.yrad += std::clamp(c.view_yrad, -limit, limit); //veh.Gun_[0].gun_info.yrad = std::clamp(veh.Gun_[0].gun_info.yrad + std::clamp(view_yrad / 5.f, -limit, limit),deg2rad(-30.0)+yrad,deg2rad(30.0)+yrad);//射界制限
						for (auto& g : veh.Gun_) {
							g.gun_info.xrad = std::clamp(g.gun_info.xrad + std::clamp(c.view_xrad, -limit, limit), deg2rad(-10), deg2rad(20));
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
						auto y_vec = veh.mat.yvec();
						for (auto& f : c.vehicle[0].use_veh.wheelframe) {
							MATRIX_ref tmp;
							veh.obj.frame_reset(f.frame.first);
							auto startpos = veh.obj.frame(f.frame.first);
							auto hp = mapparts->map_col_line(startpos + y_vec * ((-f.frame.second.y()) + 2.f), startpos + y_vec * ((-f.frame.second.y()) - 0.3f), 0);

							if (hp.HitFlag == TRUE) {
								tmp = MATRIX_ref::Mtrans(VGet(0.f, hp.HitPosition.y + y_vec.y() * f.frame.second.y() - startpos.y(), 0.f));
							}
							else {
								tmp = MATRIX_ref::Mtrans(VGet(0.f, -0.3f, 0.f));
							}

							veh.obj.SetFrameLocalMatrix(f.frame.first, MATRIX_ref::RotX((f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right) * tmp * MATRIX_ref::Mtrans(f.frame.second));
						}
						for (auto& f : c.vehicle[0].use_veh.wheelframe_nospring) {
							veh.obj.SetFrameLocalMatrix(f.frame.first, MATRIX_ref::RotX((f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right) * MATRIX_ref::Mtrans(f.frame.second));
						}
					}
					easing_set(
						&c.wheel_normal,
						((veh.obj.frame(c.vehicle[0].use_veh.square[0]) - veh.obj.frame(c.vehicle[0].use_veh.square[3])).cross(veh.obj.frame(c.vehicle[0].use_veh.square[1]) - veh.obj.frame(c.vehicle[0].use_veh.square[2]))).Norm(),
						0.95f,
						fps);
					//履帯
					{
						for (auto& f : c.vehicle[0].use_veh.b2downsideframe) {
							for (auto& t : f) {
								MATRIX_ref tmp;
								veh.obj.frame_reset(t.first);
								auto startpos = veh.obj.frame(t.first);
								auto hp = mapparts->map_col_line(startpos + veh.mat.yvec() * ((-t.second.y()) + 2.f), startpos + veh.mat.yvec() * ((-t.second.y()) - 0.3f), 0);
								if (hp.HitFlag == TRUE) {
									tmp = MATRIX_ref::Mtrans(VGet(0.f, hp.HitPosition.y + veh.mat.yvec().y() * t.second.y() - startpos.y(), 0.f));
								}
								else {
									tmp = MATRIX_ref::Mtrans(VGet(0.f, -0.3f, 0.f));
								}
								veh.obj.SetFrameLocalMatrix(t.first, tmp * MATRIX_ref::Mtrans(t.second));
							}
						}
						//b2downsideframe
					}
					//移動
					auto hp = mapparts->map_col_line(veh.pos + VGet(0.f, 2.f, 0.f), veh.pos - VGet(0.f, 0.1f, 0.f), 0);
					auto isfloat = (veh.pos.y() == -c.vehicle[0].use_veh.down_in_water);
					//行列
					{
						//リセット
						{
							const auto yvec = (c.vehicle[0].use_veh.isfloat && isfloat) ? VGet(0.f, 1.f, 0.f) : c.wheel_normal;
							auto pp = c.vehicle[0].mat.zvec();
							auto yrad_p = atan2f(-pp.x(), -pp.z());
							const auto zvec = MATRIX_ref::Vtrans(VGet(sinf(yrad_p), 0.f, cosf(yrad_p)), MATRIX_ref::RotVec2(VGet(0.f, 1.f, 0.f), yvec));
							veh.mat = MATRIX_ref::Axis1(yvec.cross(zvec), yvec, zvec);
						}
						//地面or水面にいるかどうか
						if (hp.HitFlag == TRUE || (c.vehicle[0].use_veh.isfloat && isfloat)) {
							//前進後退
							{
								const auto old = veh.speed_add + veh.speed_sub;
								if (c.key[2] && !c.mode == 1) {
									veh.speed_add = (veh.speed_add < (c.vehicle[0].use_veh.flont_speed_limit / 3.6f)) ? (veh.speed_add + (0.21f / 3.6f) * (60.f / fps)) : veh.speed_add;
									veh.speed_sub = (veh.speed_sub < 0.f) ? (veh.speed_sub + (0.7f / 3.6f) * (60.f / fps)) : veh.speed_sub;
								}
								if (c.key[3] && !c.mode == 1) {
									veh.speed_sub = (veh.speed_sub > (c.vehicle[0].use_veh.back_speed_limit / 3.6f)) ? (veh.speed_sub - (0.21f / 3.6f) * (60.f / fps)) : veh.speed_sub;
									veh.speed_add = (veh.speed_add > 0.f) ? (veh.speed_add - (0.7f / 3.6f) * (60.f / fps)) : veh.speed_add;
								}
								if (!(c.key[2] && !c.mode == 1) && !(c.key[3] && !c.mode == 1)) {
									veh.speed_add = (veh.speed_add > 0.f) ? (veh.speed_add - (0.35f / 3.6f) * (60.f / fps)) : 0.f;
									veh.speed_sub = (veh.speed_sub < 0.f) ? (veh.speed_sub + (0.35f / 3.6f) * (60.f / fps)) : 0.f;
								}
								veh.speed = (old + ((veh.speed_add + veh.speed_sub) - old) * 0.1f) / fps;
								veh.add = veh.mat.zvec() * veh.speed;
							}
							//旋回
							{
								veh.yradadd_left = (c.key[4] && !c.mode == 1) ? std::max(veh.yradadd_left - deg2rad(3.5f * (60.f / fps)), deg2rad(-c.vehicle[0].use_veh.body_rad_limit)) : std::min(veh.yradadd_left + deg2rad(2.1f * (60.f / fps)), 0.f);
								veh.yradadd_right = (c.key[5] && !c.mode == 1) ? std::min(veh.yradadd_right + deg2rad(3.5f * (60.f / fps)), deg2rad(c.vehicle[0].use_veh.body_rad_limit)) : std::max(veh.yradadd_right - deg2rad(2.1f * (60.f / fps)), 0.f);
								veh.yradadd = (veh.yradadd_left + veh.yradadd_right) / fps;
							}
							//慣性
							{
								const auto xradold = veh.xradadd;
								veh.xradadd = deg2rad(-((veh.speed / 7.f) / (60.f / fps)) / ((0.1f / 3.6f) / fps) * 30.f);
								easing_set(&veh.xrad, std::clamp(veh.xradadd - xradold, deg2rad(-15.f), deg2rad(15.f)), 0.995f, fps);

								auto zradold = veh.zradadd;
								veh.zradadd = deg2rad(-veh.yradadd / (deg2rad(5.f) / fps) * 30.f);
								easing_set(&veh.zrad, std::clamp(veh.zradadd - zradold, deg2rad(-15.f), deg2rad(15.f)), 0.995f, fps);

								veh.mat *= MATRIX_ref::RotAxis(veh.mat.xvec(), -veh.xrad)*MATRIX_ref::RotAxis(veh.mat.zvec(), veh.zrad);
							}
							//
							if (hp.HitFlag == TRUE) {
								auto yp = veh.pos.y();
								easing_set(&yp, hp.HitPosition.y, 0.9f, fps);
								veh.pos.y(yp);
							}
						}
						else {
							veh.add.yadd(M_GR / powf(fps, 2.f));
						}
						//射撃反動
						{
							easing_set(&c.xrad_shot, deg2rad(-veh.Gun_[0].fired * veh.Gun_[0].Spec[0].caliber_a * 50.f) * cos(veh.Gun_[0].gun_info.yrad), 0.85f, fps);
							easing_set(&c.zrad_shot, deg2rad(-veh.Gun_[0].fired * veh.Gun_[0].Spec[0].caliber_a * 50.f) * sin(veh.Gun_[0].gun_info.yrad), 0.85f, fps);
							veh.mat *= MATRIX_ref::RotAxis(veh.mat.xvec(), -c.xrad_shot)* MATRIX_ref::RotAxis(veh.mat.zvec(), c.zrad_shot);
						}
					}
					//浮く
					if (c.vehicle[0].use_veh.isfloat) {
						veh.pos.y(std::max(veh.pos.y(), -c.vehicle[0].use_veh.down_in_water));
					}
					//転輪
					veh.wheel_Leftadd = -(veh.speed * 2.f - veh.yradadd * 5.f);
					veh.wheel_Rightadd = -(veh.speed * 2.f + veh.yradadd * 5.f);

					veh.wheel_Left += veh.wheel_Leftadd;  // -veh.yradadd * 5.f;
					veh.wheel_Right += veh.wheel_Rightadd; // +veh.yradadd * 5.f;

				}
				//飛行機演算
				{
					auto& veh = c.vehicle[1];
					float rad_spec = deg2rad(veh.use_veh.body_rad_limit * (veh.use_veh.mid_speed_limit / veh.speed));
					if (veh.speed < veh.use_veh.min_speed_limit) {
						rad_spec = deg2rad(veh.use_veh.body_rad_limit * (std::clamp(veh.speed, 0.f, veh.use_veh.min_speed_limit) / veh.use_veh.min_speed_limit));
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
					easing_set(&veh.speed_add, (((c.key[8] && c.mode == 1) && veh.speed < veh.use_veh.max_speed_limit) ? (0.5f / 3.6f) : 0.f), 0.95f, fps);
					easing_set(&veh.speed_sub, (c.key[9] && c.mode == 1) ? ((veh.speed > veh.use_veh.min_speed_limit) ? (-0.5f / 3.6f) : ((veh.speed > 0.f) ? (-0.2f / 3.6f) : 0.f)) : 0.f, 0.95f, fps);
					//スピード
					veh.speed += (veh.speed_add + veh.speed_sub) * 60.f / fps;
					{
						auto tmp = veh.mat.zvec();
						auto tmp2 = sin(atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z())));
						veh.speed += (((std::abs(tmp2) > sin(deg2rad(1.0f))) ? tmp2 * 0.5f : 0.f) / 3.6f) * 60.f / fps; //落下
					}
					//座標系反映
					{
						auto t_mat = veh.mat;
						veh.mat *= MATRIX_ref::RotAxis(t_mat.xvec(), (veh.xradadd_right + veh.xradadd_left) / fps);
						veh.mat *= MATRIX_ref::RotAxis(t_mat.zvec(), (veh.zradadd_right + veh.zradadd_left) / fps);
						veh.mat *= MATRIX_ref::RotAxis(t_mat.yvec(), (veh.yradadd_left + veh.yradadd_right) / fps);
					}
					//
					c.landing.second = std::min<uint8_t>(c.landing.second + 1, uint8_t((c.key[13] && c.mode == 1) ? 2 : 0));
					if (c.landing.second == 1) {
						c.landing.first ^= 1;
					}
					//脚
					c.changegear.second = std::min<uint8_t>(c.changegear.second + 1, uint8_t((c.key[10] && c.mode == 1) ? 2 : 0));
					if (c.changegear.second == 1) {
						c.changegear.first ^= 1;
					}
					easing_set(&c.p_anime_geardown.second, float(c.changegear.first), 0.95f, fps);
					MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
					//舵
					for (int i = 0; i < c.p_animes_rudder.size(); i++) {
						easing_set(&c.p_animes_rudder[i].second, float(c.key[i + 2] && c.mode == 1), 0.95f, fps);
						MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
					}
					//
					{
						//
						if (veh.speed >= veh.use_veh.min_speed_limit) {
							easing_set(&veh.add, VGet(0.f, 0.f, 0.f), 0.9f, fps);
						}
						else {
							veh.add.yadd(M_GR / powf(fps, 2.f));
						}

						//着艦ワイヤ-処理
						{
							veh.obj.frame_reset(veh.use_veh.hook.first);
							veh.obj.SetFrameLocalMatrix(veh.use_veh.hook.first, MATRIX_ref::RotX(deg2rad(c.p_landing_per)) * MATRIX_ref::Mtrans(veh.use_veh.hook.second));
							easing_set(&c.p_landing_per, (c.landing.first) ? 20.f : 0.f, 0.95f, fps);
							if (c.landing.first) {
								bool to = false;
								for (auto& t : chara) {
									auto& veh2 = t.vehicle[2];

									for (auto& wi : veh2.use_veh.wire) {
										veh2.obj.frame_reset(wi.first);
										if ((veh.obj.frame(veh.use_veh.hook.first + 1) - veh2.obj.frame(wi.first)).size() <= 30.f) {
											VECTOR_ref vec1 = (veh.obj.frame(veh.use_veh.hook.first + 1) - veh.obj.frame(veh.use_veh.hook.first)).Norm();
											VECTOR_ref vec2 = (veh2.obj.frame(wi.first) - veh.obj.frame(veh.use_veh.hook.first)).Norm();
											if (vec1.dot(vec2) >= 0) {
												to = true;
												veh2.obj.SetFrameLocalMatrix(wi.first, MATRIX_ref::Mtrans(veh.obj.frame(veh.use_veh.hook.first + 1) - veh2.obj.frame(wi.first)) * veh.mat.Inverse() * MATRIX_ref::Mtrans(wi.second));
												break;
											}
										}
									}
								}
								if (to && veh.speed > 0.f) {
									veh.speed += -2.5f / 3.6f;
								}
							}
						}

						if (c.p_anime_geardown.second >= 0.5f) {
							bool hit_f = false;
							for (auto& w : veh.use_veh.wheelframe) {
								easing_set(&w.gndsmksize, 0.01f, 0.9f, fps);
								auto tmp = veh.obj.frame(int(w.frame.first + 1)) - VGet(0.f, 0.2f, 0.f);
								//地面
								{
									auto hp = mapparts->map_col_line(tmp + (veh.mat.yvec() * (0.5f)), tmp, 0);
									if (hp.HitFlag == TRUE) {
										veh.add = (VECTOR_ref(hp.HitPosition) - tmp);
										{
											auto normal = veh.mat.yvec();
											easing_set(&normal, hp.Normal, 0.95f, fps);
											veh.mat *= MATRIX_ref::RotVec2(veh.mat.yvec(), normal);
										}
										w.gndsmksize = std::clamp(veh.speed * 3.6f / 50.f, 0.1f, 1.f);
										if (veh.speed >= 0.f && (c.key[11] && c.mode == 1)) {
											veh.speed += -0.5f / 3.6f;
										}
										if (veh.speed <= 0.f) {
											easing_set(&veh.speed, 0.f, 0.9f, fps);
										}
										hit_f = true;
									}
								}
								//空母
								for (auto& t : chara) {
									auto& veh2 = t.vehicle[2];
									auto hp = veh2.col.CollCheck_Line(tmp + veh.mat.yvec() * (0.5f), tmp, -1, 0);
									if (hp.HitFlag == TRUE) {
										veh.add = (VECTOR_ref(hp.HitPosition) - tmp);
										{
											auto normal = veh.mat.yvec();
											easing_set(&normal, hp.Normal, 0.95f, fps);
											veh.mat *= MATRIX_ref::RotVec2(veh.mat.yvec(), normal);
										}
										veh.add += (MATRIX_ref::Vtrans(veh.pos - veh2.pos, MATRIX_ref::RotY(veh2.yradadd)) - (veh.pos - veh2.pos)) + VGet(veh2.speed * sin(veh2.yradadd), 0.f, veh2.speed * cos(veh2.yradadd));
										veh.mat *= MATRIX_ref::RotY(veh2.yradadd);

										w.gndsmksize = std::clamp(veh.speed * 3.6f / 50.f, 0.1f, 1.f);
										if (veh.speed >= 0.f && (c.key[11] && c.mode == 1)) {
											veh.speed += -1.0f / 3.6f;
										}
										if (veh.speed <= 0.f) {
											easing_set(&veh.speed, 0.f, 0.9f, fps);
										}

										if (c.key[14]) {
											easing_set(&veh.speed, veh.use_veh.mid_speed_limit, 0.90f, fps);
										}

										hit_f = true;
									}
								}
							}

							if (hit_f) {
								easing_set(&veh.wheel_Leftadd, -veh.speed / 20.f, 0.95f, fps);
								easing_set(&veh.wheel_Rightadd, -veh.speed / 20.f, 0.95f, fps);
							}
							else {
								easing_set(&veh.wheel_Leftadd, 0.f, 0.9f, fps);
								easing_set(&veh.wheel_Rightadd, 0.f, 0.9f, fps);
							}
							veh.wheel_Left += veh.wheel_Leftadd;  // -veh.yradadd * 5.f;
							veh.wheel_Right += veh.wheel_Rightadd; // +veh.yradadd * 5.f;


							for (auto& f : veh.use_veh.wheelframe_nospring) {

								veh.obj.SetFrameLocalMatrix(f.frame.first,
									MATRIX_ref::RotAxis(
										MATRIX_ref::Vtrans(
											VGet(0.f, 0.f, 0.f), MV1GetFrameLocalMatrix(veh.obj.get(), f.frame.first + 1)),
										(f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right) *

									MATRIX_ref::Mtrans(f.frame.second));
							}
						}
						else {
							for (auto& w : veh.use_veh.wheelframe) {
								easing_set(&w.gndsmksize, 0.01f, 0.9f, fps);
							}
						}
						veh.pos += veh.add + (veh.mat.zvec() * (-veh.speed / fps));
					}
					//浮く
					if (veh.use_veh.isfloat) {
						veh.pos.y(std::max(veh.pos.y(), -veh.use_veh.down_in_water));
					}
					//壁の当たり判定
					bool hitb = false;
					{
						VECTOR_ref p_0 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.minpos.x(), 0.f, veh.use_veh.maxpos.z()), veh.mat);
						VECTOR_ref p_1 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.maxpos.x(), 0.f, veh.use_veh.maxpos.z()), veh.mat);
						VECTOR_ref p_2 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.maxpos.x(), 0.f, veh.use_veh.minpos.z()), veh.mat);
						VECTOR_ref p_3 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.minpos.x(), 0.f, veh.use_veh.minpos.z()), veh.mat);
						if (p_0.y() <= 0.f || p_1.y() <= 0.f || p_2.y() <= 0.f || p_3.y() <= 0.f) {
							hitb = true;
						}
						if (!hitb) {
							for (auto& t : chara) {
								auto& veh2 = t.vehicle[2];

								if (veh2.col.CollCheck_Line(p_0, p_1, -1, 0).HitFlag == TRUE) {
									hitb = true;
									break;
								}
								if (veh2.col.CollCheck_Line(p_1, p_2, -1, 0).HitFlag == TRUE) {
									hitb = true;
									break;
								}
								if (veh2.col.CollCheck_Line(p_2, p_3, -1, 0).HitFlag == TRUE) {
									hitb = true;
									break;
								}
								if (veh2.col.CollCheck_Line(p_3, p_0, -1, 0).HitFlag == TRUE) {
									hitb = true;
									break;
								}
							}
						}
						if (!hitb) {
							for (int i = 0; i < mapparts->map_col_get().mesh_num(); i++) {
								if (mapparts->map_col_line(p_0, p_1, i).HitFlag == TRUE) {
									hitb = true;
									break;
								}
								if (mapparts->map_col_line(p_1, p_2, i).HitFlag == TRUE) {
									hitb = true;
									break;
								}
								if (mapparts->map_col_line(p_2, p_3, i).HitFlag == TRUE) {
									hitb = true;
									break;
								}
								if (mapparts->map_col_line(p_3, p_0, i).HitFlag == TRUE) {
									hitb = true;
									break;
								}
							}
						}
					}
					if (hitb) {
						auto& veh2 = c.vehicle[2];

						c.mode = 0; //戦車などのモードにする
						//veh2.obj.SetFrameLocalMatrix(veh2.use_veh.catapult[0].first + 2, MATRIX_ref::RotX(deg2rad(-75)) * MATRIX_ref::Mtrans(veh2.use_veh.catapult[0].second));
						auto pp = veh2.obj.frame(veh2.use_veh.catapult[0].first + 1) - veh2.obj.frame(veh2.use_veh.catapult[0].first);
						veh.pos = veh2.obj.frame(veh2.use_veh.catapult[0].first) + VGet(0.f, 5.f, 0.f);
						veh.mat = MATRIX_ref::RotY(atan2f(-pp.x(), -pp.z()));
						veh.xradadd_right = 0.f;
						veh.xradadd_left = 0.f;
						veh.yradadd_left = 0.f;
						veh.yradadd_right = 0.f;
						veh.zradadd_right = 0.f;
						veh.zradadd_left = 0.f;
						veh.speed_add = 0.f;
						veh.speed_sub = 0.f;
						veh.speed = 0.f;
						veh.add = VGet(0.f, 0.f, 0.f);
						c.p_anime_geardown.second = 1.f;
						c.landing.first = false;
					}
					//バーナー
					for (auto& be : c.p_burner) {
						veh.obj.SetFrameLocalMatrix(be.first, MATRIX_ref::Scale(VGet(1.f, 1.f, std::clamp(c.vehicle[1].speed / c.vehicle[1].use_veh.mid_speed_limit, 0.1f, 1.f))) * MATRIX_ref::Mtrans(be.second));
					}
				}
				//
				//空母
				{
					auto& veh = c.vehicle[2];
					veh.yradadd = deg2rad(0.f) / fps;
					veh.yrad += veh.yradadd;

					veh.mat = MATRIX_ref::RotY(veh.yrad);
					veh.speed = -(60.f / 3.6f) / fps;
					veh.pos += VGet(veh.speed * sin(veh.yradadd), 0.f, veh.speed * cos(veh.yradadd));
				}
				//射撃
				{
					auto& veh = c.vehicle[c.mode];
					for (int i = 0; i < veh.Gun_.size(); i++) {
						auto& cg = veh.Gun_[i];
						if (c.key[(i == 0) ? 0 : 1] && cg.loadcnt == 0 && cg.rounds > 0) {
							auto& u = cg.bullet[cg.usebullet];
							++cg.usebullet %= cg.bullet.size();
							//ココだけ変化
							u.spec = cg.Spec[0];
							u.spec.speed_a *= float(75 + GetRand(50)) / 100.f;
							u.pos = veh.obj.frame(cg.gun_info.frame2.first);
							u.vec = (veh.obj.frame(cg.gun_info.frame3.first) - veh.obj.frame(cg.gun_info.frame2.first)).Norm();
							//
							cg.loadcnt = cg.gun_info.load_time;
							cg.rounds = std::max<uint16_t>(cg.rounds - 1, 0);
							if (i == 0) {
								cg.fired = 1.f;
							}
							u.hit = false;
							u.flug = true;
							u.cnt = 0.f;
							u.yadd = 0.f;
							u.repos = u.pos;
							if (u.spec.type_a != 2) {
								set_effect(&c.effcs[ef_fire], veh.obj.frame(cg.gun_info.frame3.first), u.vec, u.spec.caliber_a / 0.1f);
								if (u.spec.caliber_a >= 0.037f) {
									set_effect(&c.effcs_gun[c.gun_effcnt].first, veh.obj.frame(cg.gun_info.frame3.first), u.vec);
									set_pos_effect(&c.effcs_gun[c.gun_effcnt].first, Drawparts->get_effHandle(ef_smoke2));
									c.effcs_gun[c.gun_effcnt].second = &u;
									c.effcs_gun[c.gun_effcnt].cnt = 0;
									++c.gun_effcnt %= c.effcs_gun.size();
								}
							}
							else {
								set_effect(&c.effcs_missile[c.missile_effcnt].first, veh.obj.frame(cg.gun_info.frame3.first), u.vec);
								set_pos_effect(&c.effcs_missile[c.missile_effcnt].first, Drawparts->get_effHandle(ef_smoke1));
								c.effcs_missile[c.missile_effcnt].second = &u;
								c.effcs_missile[c.missile_effcnt].cnt = 0;
								++c.missile_effcnt %= c.effcs_missile.size();
							}
						}
						cg.loadcnt = std::max(cg.loadcnt - 1.f / fps, 0.f);
						cg.fired = std::max(cg.fired - 1.f / fps, 0.f);
					}
				}
				/*effect*/
				{
					for (auto& t : c.effcs) {
						if (t.id != ef_smoke1 && t.id != ef_smoke2) {
							set_pos_effect(&t, Drawparts->get_effHandle(int(t.id)));
						}
					}
					for (auto& t : c.vehicle[0].use_veh.wheelframe) {
						t.gndsmksize = 0.1f + std::abs(c.vehicle[0].speed) / ((c.vehicle[0].use_veh.flont_speed_limit / 3.6f) / fps) * 0.6f;
						t.gndsmkeffcs.handle.SetPos(c.vehicle[0].obj.frame(t.frame.first) + c.vehicle[0].mat.yvec() * (-t.frame.second.y()));
						t.gndsmkeffcs.handle.SetScale(t.gndsmksize);
					}
					for (auto& t : c.vehicle[1].use_veh.wheelframe) {
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
			//座標反映
			for (auto& c : chara) {
				{
					//転輪
					auto& veh = c.vehicle[0];
					b2Vec2 tmpb2 = b2Vec2((M_GR / fps) * (veh.mat.zvec().dot(VGet(0, 1.f, 0))), (M_GR / fps) * (veh.mat.yvec().dot(VGet(0, 1.f, 0))));
					int z = 0;
					for (auto& f : c.foot) {
						size_t i = 0;
						VECTOR_ref vects;
						if (f.Foot.size() != 0) {
							for (auto& w : veh.use_veh.wheelframe) {
								vects = VECTOR_ref(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(veh.obj.get(), w.frame.first)));
								if (vects.x() * ((z == 0) ? 1 : -1) > 0) {
									f.Wheel[i++].body->SetTransform(b2Vec2(vects.z(), vects.y()), (z == 0) ? veh.wheel_Left : veh.wheel_Right);
								}
							}
							i = 0;
							for (auto& w : veh.use_veh.wheelframe_nospring) {
								vects = VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(veh.obj.get(), w.frame.first));
								if (vects.x() * ((z == 0) ? 1 : -1) > 0) {
									f.Yudo[i++].body->SetTransform(b2Vec2(vects.z(), vects.y()), (z == 0) ? veh.wheel_Left : veh.wheel_Right);
								}
							}
							for (auto& t : f.Foot) {
								t.body->SetLinearVelocity(tmpb2); //
							}

							f.world->Step(1.f / fps, 3, 3);

							for (auto& t : f.Foot) {
								t.pos = VGet(t.pos.x(), t.body->GetPosition().y, t.body->GetPosition().x);
							}
						}
						i = 0;
						for (auto& w : veh.use_veh.b2upsideframe[z]) {
							if (w.second.x() * ((z == 0) ? 1 : -1) > 0) {
								f.Foot[i].pos = VGet(w.second.x(), f.Foot[i].pos.y(), f.Foot[i].pos.z());
								veh.obj.SetFrameLocalMatrix(w.first, MATRIX_ref::Mtrans(f.Foot[i].pos));
								++i;
							}
						}
						z++;
					}
					//戦車座標反映
					auto pp = veh.mat.zvec();
					veh.mat *= MATRIX_ref::RotY((-c.b2mine.body->GetAngle()) - atan2f(-pp.x(), -pp.z()));
					veh.pos.x(c.b2mine.body->GetPosition().x);
					veh.pos.z(c.b2mine.body->GetPosition().y);
					float spdrec = c.spd;
					easing_set(&c.spd, std::hypot(c.b2mine.body->GetLinearVelocity().x, c.b2mine.body->GetLinearVelocity().y) * ((c.spd > 0) ? 1.f : -1.f), 0.99f, fps);
					veh.speed = c.spd - spdrec;

				}
				//弾関連
				for (auto& veh : c.vehicle) {
					//弾判定
					for (auto& g : veh.Gun_) {
						for (auto& a : g.bullet) {
							float size = 3.f;
							for (int z = 0; z < int(size); z++) {
								if (a.flug) {
									a.repos = a.pos;
									a.pos += a.vec * (a.spec.speed_a / fps / size);
									//判定
									{
										bool ground_hit = false;
										VECTOR_ref normal;
										//戦車以外に当たる
										{
											for (int i = 0; i < mapparts->map_col_get().mesh_num(); i++) {
												auto hps = mapparts->map_col_line(a.repos, a.pos, i);
												if (hps.HitFlag) {
													a.pos = hps.HitPosition;
													normal = hps.Normal;
													ground_hit = true;
												}
											}
										}
										ref_col(c, a.pos, a.repos);
										auto hitcal = c.get_reco(chara, a, 2);			//空母にあたる
										auto hitplane = c.get_reco(chara, a, 1);		//飛行機にあたる
										auto hittank = c.get_reco(chara, a, 0);			//戦車に当たる
										//その後処理
										switch (a.spec.type_a) {
										case 0: //AP
											if (!(hittank || hitplane || hitcal)) {
												if (ground_hit) {
													if (a.spec.caliber_a >= 0.020f) {
														set_effect(&c.effcs[ef_gndhit], a.pos + normal * (0.1f), normal);
													}
													else {
														set_effect(&c.effcs[ef_gndhit2], a.pos + normal * (0.1f), normal);
													}
													if ((a.vec.Norm().dot(normal)) <= cos(deg2rad(60))) {
														a.flug = false;
													}
													else {
														a.vec += normal * ((a.vec.dot(normal)) * -2.f);
														a.vec = a.vec.Norm();
														a.pos += a.vec * (0.01f);
														a.spec.pene_a /= 2.f;
													}
												}
											}
											if (a.flug) {
												a.spec.pene_a -= 1.0f / fps / size;
												a.spec.speed_a -= 5.f / fps / size;
												a.pos += VGet(0.f, a.yadd / size, 0.f);
											}
											break;
										case 1: //HE
											if (!(hittank || hitplane || hitcal)) {
												if (ground_hit) {
													if (a.spec.caliber_a >= 0.020f) {
														set_effect(&c.effcs[ef_gndhit], a.pos + normal * (0.1f), normal);
													}
													else {
														set_effect(&c.effcs[ef_gndhit2], a.pos + normal * (0.1f), normal);
													}
													a.flug = false;
												}
											}
											if (a.flug) {
												a.spec.speed_a -= 5.f / fps / size;
												a.pos += VGet(0.f, a.yadd / size, 0.f);
											}
											break;
										case 2: //ミサイル
											if (!(hittank || hitplane || hitcal)) {
												if (ground_hit) {
													if (a.spec.caliber_a >= 0.020f) {
														set_effect(&c.effcs[ef_gndhit], a.pos + normal * (0.1f), normal);
													}
													else {
														set_effect(&c.effcs[ef_gndhit2], a.pos + normal * (0.1f), normal);
													}
													a.flug = false;
												}
											}
											if (a.flug) {
												size_t id = chara.size();
												VECTOR_ref pos;
												float dist = (std::numeric_limits<float>::max)();
												for (auto& t : chara) {
													//弾関連
													if (c.id == t.id) {
														continue;
													}
													{
														auto& veh_t = t.vehicle[0];
														auto p = (veh_t.pos - a.pos).size();
														if (dist > p) {
															dist = p;
															id = t.id;
															pos = veh_t.pos + VGet(0.f, 1.f, 0.f);
														}
													}
													{
														auto& veh_t = t.vehicle[1];
														auto p = (veh_t.pos - a.pos).size();
														if (dist > p) {
															dist = p;
															id = t.id;
															pos = veh_t.pos;
														}
													}
												}
												if (id != chara.size()) {
													auto vec_a = (a.pos - pos).Norm();
													//反映
													auto vec_z = a.vec;
													float z_hyp = std::hypotf(vec_z.x(), vec_z.z());
													float a_hyp = std::hypotf(vec_a.x(), vec_a.z());
													float cost = (vec_a.z() * vec_z.x() - vec_a.x() * vec_z.z()) / (a_hyp * z_hyp);
													float view_yrad = (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f; //cos取得2D
													float view_xrad = (atan2f(-vec_z.y(), z_hyp) - atan2f(vec_a.y(), a_hyp)) / 5.f;
													{
														float limit = deg2rad(30.f) / fps;
														float y = atan2f(a.vec.x(), a.vec.z()) + std::clamp(view_yrad, -limit, limit);
														float x = atan2f(a.vec.y(), std::hypotf(a.vec.x(), a.vec.z())) + std::clamp(view_xrad, -limit, limit);
														a.vec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
													}
												}
											}
											break;
										default:
											break;
										}
									}

									//消す(2秒たった、スピードが100以下、貫通が0以下)
									if (a.cnt >= 2.f || a.spec.speed_a < 100.f || a.spec.pene_a <= 0.f) {
										a.flug = false;
									}
									if (!a.flug) {
										for (auto& b : c.effcs_gun) {
											if (b.second == &a) {
												b.first.handle.SetPos(b.second->pos);
												break;
											}
										}
									}
								}
							}
							a.yadd += M_GR / powf(fps, 2.f);
							a.cnt += 1.f / fps;
						}
					}
					//弾痕
					for (auto& h : veh.hit_obj) {
						if (h.flug) {
							h.pic.SetMatrix(h.mat* veh.mat*MATRIX_ref::Mtrans(veh.pos + MATRIX_ref::Vtrans(h.pos, veh.mat)));
							//*/
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
							if (a.cnt >= 3.f * GetFPS()) {
								a.first.handle.Stop();
								a.cnt = -1;
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
							if (a.cnt >= 3.f * GetFPS()) {
								a.first.handle.Stop();
								a.cnt = -1;
							}
						}
					}
				}
				//木判定
				{
					auto& veh = c.vehicle[0];
					for (auto& l : tree) {
						if (!l.fall_flag) {
							auto p0 = veh.obj.frame(veh.use_veh.square[1]);
							auto p1 = veh.obj.frame(veh.use_veh.square[0]);
							auto p2 = veh.obj.frame(veh.use_veh.square[2]);
							auto p3 = veh.obj.frame(veh.use_veh.square[3]);
							p0.y(l.pos.y());
							p1.y(l.pos.y());
							p2.y(l.pos.y());
							p3.y(l.pos.y());

							size_t cnt = 0;
							cnt += (((p0 - p1).cross(l.pos - p1)).y() >= 0);
							cnt += (((p1 - p2).cross(l.pos - p2)).y() >= 0);
							cnt += (((p2 - p3).cross(l.pos - p3)).y() >= 0);
							cnt += (((p3 - p0).cross(l.pos - p0)).y() >= 0);
							if (cnt == 4) {
								l.fall_vec = VGet((l.pos - veh.pos).z(), 0.f, -(l.pos - veh.pos).x());
								l.fall_flag = true;
							}
						}
					}
				}
			}
			//木セット
			for (auto& l : tree) {
				if (l.fall_flag) {
					l.fall_rad = std::clamp(l.fall_rad + deg2rad(30.f / fps), deg2rad(0.f), deg2rad(90.f));
				}
				l.obj.SetMatrix(MATRIX_ref::RotAxis(l.fall_vec, l.fall_rad) * l.mat * MATRIX_ref::Mtrans(l.pos));
				l.obj_far.SetMatrix(MATRIX_ref::RotAxis(l.fall_vec, l.fall_rad) * l.mat * MATRIX_ref::Mtrans(l.pos));
			}
			{
				//他の座標をここで出力(ホスト)
			}
			{
				//ホストからの座標をここで入力
			}
			//モデルに反映
			for (auto& c : chara) {
				for (auto& veh : c.vehicle) {
					veh.obj.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));
				}
			}
			//影用意
			if (shadow_e) {
				switch (mine.mode) {
				case 0:
					Drawparts->Ready_Shadow(campos, draw_in_shadow, VGet(50.f, 25.f, 50.f));
					break;
				case 1:
					Drawparts->Ready_Shadow(campos, draw_in_shadow_sky, VGet(500.f, 500.f, 500.f));
				case 2:
					Drawparts->Ready_Shadow(campos, draw_in_shadow_sky, VGet(1000.f, 100.f, 1000.f));
					break;
				}
			}
			vrparts->Move_Player();
			//campos,camvec,camupの指定
			{
				auto& veh = mine.vehicle[mine.mode];
				switch (mine.mode) {
				case 0:
					{
						if (useVR_e) {
							eyevec = MATRIX_ref::Vtrans(eyevec, veh.mat);
						}
						if (ads) {
							campos = veh.obj.frame(veh.Gun_[0].gun_info.frame1.first) + MATRIX_ref::Vtrans(veh.Gun_[0].gun_info.frame2.second, MATRIX_ref::RotY(atan2f(eyevec.x(), eyevec.z())));
							camvec = campos - eyevec;
							camup = veh.mat.yvec();
						}
						else {
							{
								camvec = veh.pos + VGet(0.f, 3.f, 0.f);
								camvec.y(std::max(camvec.y(), 5.f));
							}
							{
								campos = camvec + eyevec * (range);
								campos.y(std::max(campos.y(), 0.f));
								if (mapparts->map_col_line_nearest(camvec, &campos)) {
									campos = camvec + (campos - camvec) * (0.9f);
								}
								for (auto& t : chara) {
									auto hp = t.vehicle[2].col.CollCheck_Line(camvec, campos, -1, 0);
									if (hp.HitFlag == TRUE) {
										campos = camvec + (VECTOR_ref(hp.HitPosition) - camvec) * (0.9f);
									}
								}
							}
							{
								camup = VGet(0.f, 1.f, 0.f);
								camup = MATRIX_ref::Vtrans(camup, MATRIX_ref::RotAxis(veh.mat.xvec(), mine.xrad_shot));
								camup = MATRIX_ref::Vtrans(camup, MATRIX_ref::RotAxis(veh.mat.zvec(), mine.zrad_shot));
								camup = MATRIX_ref::Vtrans(camup, MATRIX_ref::RotAxis(veh.mat.xvec(), mine.zrad_shot));
								camup = MATRIX_ref::Vtrans(camup, MATRIX_ref::RotAxis(veh.mat.zvec(), mine.xrad_shot));
							}
						}
					}
					break;
				case 1:
					{
						if (ads) {
							campos = veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(eye_pos_ads, veh.mat);
							campos.y(std::max(campos.y(), 5.f));
							if (useVR_e) {
								camvec = campos - MATRIX_ref::Vtrans(eyevec, veh.mat);
							}
							else {
								if ((GetMouseInput() & MOUSE_INPUT_RIGHT) == 0) {
									eyevec = veh.mat.zvec();
									camvec = campos - eyevec;
								}
								else {
									camvec = campos - MATRIX_ref::Vtrans(eyevec, veh.mat);
								}
							}
							camup = veh.mat.yvec();
						}
						else {
							camvec = veh.pos + veh.mat.yvec() * (6.f);
							camvec.y(std::max(camvec.y(), 5.f));
							if (useVR_e) {
								campos = camvec + MATRIX_ref::Vtrans(eyevec, veh.mat) * (range);
								campos.y(std::max(campos.y(), 0.f));
								if (mapparts->map_col_line_nearest(camvec, &campos)) {
									campos = camvec + (campos - camvec) * (0.9f);
								}
								for (auto& t : chara) {
									auto hp = t.vehicle[2].col.CollCheck_Line(camvec, campos, -1, 0);
									if (hp.HitFlag == TRUE) {
										campos = camvec + (VECTOR_ref(hp.HitPosition) - camvec) * (0.9f);
									}
								}
								camup = veh.mat.yvec();
							}
							else {
								if ((GetMouseInput() & MOUSE_INPUT_RIGHT) == 0) {
									eyevec = (camvec - aimpos[1]).Norm();
									campos = camvec + eyevec * (range);
									camup = veh.mat.yvec();

								}
								else {
									campos = camvec + eyevec * (range);
									campos.y(std::max(campos.y(), 0.f));
									if (mapparts->map_col_line_nearest(camvec, &campos)) {
										campos = camvec + (campos - camvec) * (0.9f);
									}
									for (auto& t : chara) {
										auto hp = t.vehicle[2].col.CollCheck_Line(camvec, campos, -1, 0);
										if (hp.HitFlag == TRUE) {
											campos = camvec + (VECTOR_ref(hp.HitPosition) - camvec) * (0.9f);
										}
									}
									camup = VGet(0.f, 1.f, 0.f);
								}
							}

						}
					}
					break;
				case 2:
					{
						camvec = veh.pos + veh.mat.yvec() * (100.f);
						camvec.y(std::max(camvec.y(), 10.f));
						if (useVR_e) {
							campos = camvec + MATRIX_ref::Vtrans(eyevec, veh.mat) * (range);
							campos.y(std::max(campos.y(), 10.f));
							if (mapparts->map_col_line_nearest(camvec, &campos)) {
								campos = camvec + (campos - camvec) * (0.9f);
							}
							for (auto& t : chara) {
								auto hp = t.vehicle[2].col.CollCheck_Line(camvec, campos, -1, 0);
								if (hp.HitFlag == TRUE) {
									campos = camvec + (VECTOR_ref(hp.HitPosition) - camvec) * (0.9f);
								}
							}
							camup = veh.mat.yvec();
						}
						else {
							campos = camvec + eyevec * (range);
							campos.y(std::max(campos.y(), 10.f));
							if (mapparts->map_col_line_nearest(camvec, &campos)) {
								campos = camvec + (campos - camvec) * (0.9f);
							}
							for (auto& t : chara) {
								auto hp = t.vehicle[2].col.CollCheck_Line(camvec, campos, -1, 0);
								if (hp.HitFlag == TRUE) {
									campos = camvec + (VECTOR_ref(hp.HitPosition) - camvec) * (0.9f);
								}
							}
							camup = VGet(0.f, 1.f, 0.f);
						}
					}
					break;
				}
			}
			{
				float fardist = 1.f;
				float neardist = 1.f;
				switch (mine.mode) {
				case 0:
				{
					VECTOR_ref aimingpos = campos + (camvec - campos).Norm() * (1000.f);
					mapparts->map_col_line_nearest(campos, &aimingpos);
					for (auto& t : chara) {
						auto hp = t.vehicle[2].col.CollCheck_Line(campos, aimingpos, -1, 0);
						if (hp.HitFlag == TRUE) { aimingpos = hp.HitPosition; }
					}
					fardist = std::clamp((campos - aimingpos).size(), 300.f, 1000.f);
				}
				break;
				case 1:
					if (ads) {
						VECTOR_ref aimingpos = campos + (camvec - campos).Norm() * (3000.f);

						mapparts->map_col_line_nearest(campos, &aimingpos);
						for (auto& t : chara) {
							auto hp = t.vehicle[2].col.CollCheck_Line(campos, aimingpos, -1, 0);
							if (hp.HitFlag == TRUE) {
								aimingpos = hp.HitPosition;
							}
						}
						fardist = std::clamp((campos - aimingpos).size(), 300.f, 3000.f);
					}
					else {
						fardist = 6000.f;
					}
				break;
				case 2:
					fardist = 6000.f;
				break;
				}
				switch (mine.mode) {
				case 0:
					neardist = (ads ? (1.5f + 98.5f * (fardist - 300.f) / (1000.f - 300.f)) : 1.5f);
					break;
				case 1:
					neardist = (ads ? (5.f + 25.f * (fardist - 300.f) / (3000.f - 300.f)) : range_p - 5.f);
					break;
				case 2:
					neardist = 15.f;
					break;
				}
				//被写体深度描画
				if (shadow_e) {
					Hostpassparts->dof(&BufScreen, mapparts->sky_draw(campos, camvec, camup, fov / ratio), [&Drawparts, &draw_on_shadow] { Drawparts->Draw_by_Shadow(draw_on_shadow); }, campos, camvec, camup, fov / ratio, fardist, neardist);
				}
				else {
					Hostpassparts->dof(&BufScreen, mapparts->sky_draw(campos, camvec, camup, fov / ratio), draw_on_shadow, campos, camvec, camup, fov / ratio, fardist, neardist);
				}
			}

			GraphHandle::SetDraw_Screen(DX_SCREEN_BACK, 0.01f, 5000.0f, fov / ratio, campos, camvec, camup);
			//照準座標取得
			{
				VECTOR_ref startpos, endpos;
				switch (mine.mode) {
				case 0: //戦車
					startpos = mine.vehicle[mine.mode].obj.frame(mine.vehicle[mine.mode].Gun_[0].gun_info.frame2.first);
					endpos = startpos + (mine.vehicle[mine.mode].obj.frame(mine.vehicle[mine.mode].Gun_[0].gun_info.frame3.first) - startpos).Norm() * (1000.f);
					break;
				case 1://飛行機
					startpos = mine.vehicle[mine.mode].pos;
					endpos = startpos + mine.vehicle[mine.mode].mat.zvec() * (-1000.f);
					break;
				case 2://船
					startpos = mine.vehicle[mine.mode].pos;
					endpos = startpos + mine.vehicle[mine.mode].mat.zvec() * (-1000.f);
					break;
				};
				//地形
				mapparts->map_col_line_nearest(startpos, &endpos);
				//空母判定
				for (auto& t : chara) {
					auto hp2 = t.vehicle[2].col.CollCheck_Line(startpos, endpos, -1, 0);
					if (hp2.HitFlag == TRUE) {
						endpos = hp2.HitPosition;
					}
				}
				//車輛判定
				if (mine.mode == 0) {
					ref_col(mine, startpos, endpos);
					for (auto& t : chara) {
						auto& veh = t.vehicle[0];
						if (veh.hit_check) {
							for (int i = 0; i < veh.col.mesh_num(); i++) {
								const auto hp = veh.col.CollCheck_Line(startpos, endpos, -1, i);
								if (hp.HitFlag == TRUE) {
									endpos = hp.HitPosition;
									if (!lock_on.first) {
										tgt = t.id;
									}
								}
							}
						}
					}
				}
				//
				easing_set(&aimpos[mine.mode], endpos, 0.9f, fps);
				aimposout = ConvWorldPosToScreenPos(aimpos[mine.mode].get());
			}
			//
			if (lock_on.first) {
				aimposout_lockon = ConvWorldPosToScreenPos(chara[tgt].vehicle[0].obj.frame(chara[tgt].vehicle[0].use_veh.gunframe[0].frame1.first).get());
				distance = (chara[tgt].vehicle[0].obj.frame(chara[tgt].vehicle[0].use_veh.gunframe[0].frame1.first) - campos).size();
			}
			//描画
			outScreen.SetDraw_Screen();
			{
				//背景
				BufScreen.DrawGraph(0, 0, false);
				//ブルーム
				Hostpassparts->bloom(BufScreen, (mine.mode != 1) ? 64 : (255));
				//UI
				UIparts->draw(aimposout, 
					mine,
					//chara[1],
					ads, fps, lock_on.first, distance, aimposout_lockon, ratio, campos, camvec, camup, eye_pos_ads,useVR_e, chveh);
				//VR用オプション
				if (useVR_e) {
					UIparts->draw_in_vr(mine, (*vrparts->get_device())[vrparts->get_left_hand_num()]);
				}
			}
			//VRに移す
			if (useVR_e) {
				GraphHandle::SetDraw_Screen(DX_SCREEN_BACK);
				outScreen.DrawGraph(0, 0, false);
				for (char i = 0; i < 2; i++) {
					vrparts->PutEye((ID3D11Texture2D*)GetUseDirect3D11BackBufferTexture2D(), i);
				}
			}
			GraphHandle::SetDraw_Screen(DX_SCREEN_BACK);
			{
				outScreen.DrawExtendGraph(0, 0, out_dispx, out_dispy, false);
				//デバッグ
				Debugparts->end_way();
				Debugparts->debug(10, 10, fps, float(GetNowHiPerformanceCount() - waits) / 1000.f);
			}
			Drawparts->Screen_Flip();
			vrparts->Eye_Flip(waits);//フレーム開始の数ミリ秒前にstartするまでブロックし、レンダリングを開始する直前に呼び出す必要があります。
			if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
				ending = false;
				break;
			}
			if (CheckHitKey(KEY_INPUT_O) != 0) {
				break;
			}
		}
		SetMouseDispFlag(TRUE);
		SetMousePoint(dispx / 2, dispy / 2);
		//解放
		{
			for (auto& c : chara) {
				/*エフェクト*/
				for (auto& t : c.effcs_gun) {
					t.first.handle.Dispose();
				}
				for (auto& t : c.effcs_missile) {
					t.first.handle.Dispose();
				}
				for (auto& t : c.effcs) {
					t.handle.Dispose();
				}
				/*Box2D*/
				delete c.b2mine.playerfix->GetUserData();
				c.b2mine.playerfix->SetUserData(NULL);
				for (auto& t : c.foot) {
					for (auto& f : t.Foot) {
						delete f.playerfix->GetUserData();
						f.playerfix->SetUserData(NULL);
					}
					for (auto& f : t.Wheel) {
						delete f.playerfix->GetUserData();
						f.playerfix->SetUserData(NULL);
					}
					for (auto& f : t.Yudo) {
						delete f.playerfix->GetUserData();
						f.playerfix->SetUserData(NULL);
					}
				}

				for (auto& veh : c.vehicle) {
					for (auto& t : veh.use_veh.wheelframe) {
						t.gndsmkeffcs.handle.Dispose();
					}
					veh.reset();
				}
			}
			chara.clear();
			mapparts->delete_map(&wall, &tree);
			if (shadow_e) {
				Drawparts->Delete_Shadow();
			}
		}
		//
	}while (ProcessMessage() == 0 && ending);
	return 0; // ソフトの終了
}

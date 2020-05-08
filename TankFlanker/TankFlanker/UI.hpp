#pragma once
class UI : hit {
private:
	GraphHandle circle;
	GraphHandle aim;
	GraphHandle scope;
	GraphHandle lock;
	GraphHandle hit;
	GraphHandle CompassScreen;
	GraphHandle CamScreen;
	GraphHandle HP_per;
	std::array<float, veh_all> ber{ 0 };
	GraphHandle HP_ber;

	GraphHandle bufScreen;
	MV1 Compass;
	//font
	FontHandle font18;
	FontHandle font12;

	MV1 garage;
	MV1 sky;
	GraphHandle SkyScreen; //空描画

	frames stickx_f, sticky_f, stickz_f;

	frames compass_f;
	VECTOR_ref cockpit_v;
	MV1 cockpit;
	GraphHandle cockpitScreen;

	float siz_autoaim = 0.f;
	float siz_autoaim_pic = 0.f;

public:
	UI() {
		lock = GraphHandle::Load("data/UI/battle_lock.bmp");
		hit = GraphHandle::Load("data/UI/battle_hit.bmp");
		circle = GraphHandle::Load("data/UI/battle_circle.bmp");
		aim = GraphHandle::Load("data/UI/battle_aim.bmp");
		scope = GraphHandle::Load("data/UI/battle_scope.png");
		HP_per = GraphHandle::Load("data/UI/battle_hp_bar_max.bmp");
		HP_ber = GraphHandle::Load("data/UI/battle_hp_bar.bmp");
		CompassScreen = GraphHandle::Make(240, 240, true);
		CamScreen = GraphHandle::Make(240, 240, true);
		bufScreen = GraphHandle::Make(dispx, dispy, true);

		MV1::Load("data/model/cockpit/model.mv1", &cockpit, false);
		for (int i = 0; i < cockpit.frame_num(); i++) {
			std::string p = cockpit.frame_name(i);
			if (p.find("座席", 0) != std::string::npos) {
				cockpit_v = cockpit.frame(i);
			}
			else if ((p.find("姿勢指示器", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
				compass_f = { i,cockpit.frame(i)- cockpit.frame(cockpit.frame_parent(i)) };
				//ジャイロコンパス
			}
			else if (p.find("スティック縦", 0) != std::string::npos) {
				stickz_f = { i,cockpit.frame(i) };
				stickx_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
			}
			else if ((p.find("ペダル", 0) != std::string::npos) && (p.find("右", 0) == std::string::npos) && (p.find("左", 0) == std::string::npos)) {
				sticky_f = { i,cockpit.frame(i) };
			}
		}

		cockpitScreen = GraphHandle::Make(dispx, dispy, true);


		MV1::Load("data/model/compass/model.mv1", &Compass, true);
		Compass.SetPosition(VGet(0.f, 0.f, 0.f));
		font18 = FontHandle::Create(y_r(18), DX_FONTTYPE_EDGE);
		font12 = FontHandle::Create(y_r(12), DX_FONTTYPE_EDGE);
		MV1::Load("data/model/garage/model.mv1", &garage, false);
		MV1::Load("data/model/sky/model.mv1", &sky, false);
		SkyScreen = GraphHandle::Make(dispx, dispy);
	}
	~UI() {
	}
	template <size_t N>
	bool select_window(hit::Chara* chara, std::array<std::vector<hit::Vehcs>, N>* vehcs) {
		if (1) {
			VECTOR_ref campos = VGet(0.f, 0.f, -15.f);
			VECTOR_ref camaim = VGet(0.f, 3.f, 0.f);
			uint8_t upct = 0, dnct = 0, rtct = 0, ltct = 0, changecnt = 0;
			float fov = deg2rad(90 / 2);
			auto& t = (*vehcs)[0];
			auto& p = (*vehcs)[1];

			chara->vehicle[0].use_id %= t.size(); //戦車
			chara->vehicle[0].camo_sel = std::min(chara->vehicle[0].camo_sel, t[chara->vehicle[0].use_id].camog.size() - 1);
			chara->vehicle[1].use_id %= p.size(); //飛行機
			chara->vehicle[1].camo_sel = std::min(chara->vehicle[1].camo_sel, p[chara->vehicle[1].use_id].camog.size() - 1);
			chara->mode = 0;

			float speed = 0.f, wheel_rad = 0.f;
			VECTOR_ref pos;

			bool endp = false;
			bool startp = false;
			float rad = 0.f;
			float xrad_m = 0.f;
			float yrad_m = 0.f;
			float rad_i = 0.f;
			float yrad_im = 0.f, xrad_im = 0.f;
			int m_x = 0, m_y = 0;
			float ber_r = 0.f;
			GetMousePoint(&m_x, &m_y);

			while (ProcessMessage() == 0) {
				const auto fps = GetFPS();
				const auto waits = GetNowHiPerformanceCount();

				if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
					break;
				}

				if (!startp) {
					int x, y;
					GetMousePoint(&x, &y);
					if (chara->mode == 0) {
						yrad_im = std::clamp(yrad_im + float(m_x - x) / 5.f, -120.f, -30.f);
						xrad_im = std::clamp(xrad_im + float(m_y - y), -5.f, 30.f);
					}
					else {
						yrad_im = std::clamp(yrad_im + float(m_x - x) / 5.f, -120.f, -30.f);
						xrad_im = std::clamp(xrad_im + float(m_y - y), -45.f, 45.f);
					}
					easing_set(&yrad_m, deg2rad(yrad_im), 0.9f, fps);
					easing_set(&xrad_m, deg2rad(xrad_im), 0.9f, fps);
					GetMousePoint(&m_x, &m_y);

					upct = std::clamp<uint8_t>(upct + 1, 0, ((CheckHitKey(KEY_INPUT_D) != 0) ? 2 : 0));
					dnct = std::clamp<uint8_t>(dnct + 1, 0, ((CheckHitKey(KEY_INPUT_A) != 0) ? 2 : 0));
					ltct = std::clamp<uint8_t>(ltct + 1, 0, ((CheckHitKey(KEY_INPUT_S) != 0) ? 2 : 0));
					rtct = std::clamp<uint8_t>(rtct + 1, 0, ((CheckHitKey(KEY_INPUT_W) != 0) ? 2 : 0));
					changecnt = std::clamp<uint8_t>(changecnt + 1, 0, ((CheckHitKey(KEY_INPUT_P) != 0) ? 2 : 0));
					if (changecnt == 1) {
						++(chara->mode) %= chara->vehicle.size();
						ber_r = 0.f;
					}
				}
				else {
					upct = 0;
					dnct = 0;
					ltct = 0;
					rtct = 0;
					changecnt = 0;
				}

				easing_set(&ber_r, float(dispy / 4), 0.95f, fps);

				auto& veh = chara->vehicle[chara->mode];
				if (chara->mode == 0) {
					if (CheckHitKey(KEY_INPUT_SPACE) != 0 || speed != 0.f) {
						speed = std::clamp(speed + 0.25f / 3.6f / fps, 0.f, 60.f / 3.6f / fps);
						pos.zadd(-speed);
						startp = true;
					}
					auto old = veh.camo_sel;
					if (!startp) {
						easing_set(&campos, (MATRIX_ref::RotY(yrad_m) * MATRIX_ref::RotX(xrad_m)).zvec() * (-15.f) + VGet(0.f, 3.f, 0.f), 0.95f, fps);
						camaim = pos + VGet(0.f, 3.f, 0.f);
						if (upct == 1) {
							++veh.use_id %= t.size();
							veh.camo_sel = std::min(veh.camo_sel, t[veh.use_id].camog.size() - 1);
						}
						if (dnct == 1) {
							if (veh.use_id == 0) {
								veh.use_id = t.size() - 1;
							}
							else {
								--veh.use_id;
							}
							veh.camo_sel = std::min(veh.camo_sel, t[veh.use_id].camog.size() - 1);
						}
						if (ltct == 1) {
							++veh.camo_sel %= t[veh.use_id].camog.size();
						}
						if (rtct == 1) {
							if (veh.camo_sel == 0) {
								veh.camo_sel = t[veh.use_id].camog.size() - 1;
							}
							else {
								--veh.camo_sel;
							}
						}
					}
					else {
						easing_set(&campos, VGet(3.25f, 3.f, 7.5f), 0.95f, fps);
						camaim = pos + VGet(0.f, 3.f, 0.f);
					}
					if (t[veh.use_id].camog.size() > 0) {
						SetDrawScreen(CamScreen.get());
						DrawExtendGraph(0, 0, 240, 240, t[veh.use_id].camog[veh.camo_sel], TRUE);
						MV1SetTextureGraphHandle(t[veh.use_id].obj.get(), t[veh.use_id].camo_tex, t[veh.use_id].camog[veh.camo_sel], TRUE);
					}
					GraphFilter(CamScreen.get(), DX_GRAPH_FILTER_GAUSS, 16, 2400);
					if (veh.camo_sel != old) {
						if (std::abs(int(int(veh.camo_sel) - old)) == 1) {
							rad_i += 360 * int(int(veh.camo_sel) - old) / int(t[veh.use_id].camog.size());
						}
						else {
							if (veh.camo_sel == 0) {
								rad_i += 360 / int(t[veh.use_id].camog.size());
							}
							else {
								rad_i -= 360 / int(t[veh.use_id].camog.size());
							}
						}
					}
					bufScreen.SetDraw_Screen();
					{
						int xp = dispx / 2	 + int((ber_r * 16.f / 9.f) * sin(rad));
						int yp = dispy * 2 / 3	 + int(ber_r * cos(rad));
						int xa = dispx / 2	 + int(((ber_r * 16.f / 9.f) - y_r(150)) * sin(rad));
						int ya = dispy * 2 / 3	 + int((ber_r - y_r(150)) * cos(rad));
						DrawLine(xa, ya, xp, yp, GetColor(0, 255, 0), 2);
						CamScreen.DrawExtendGraph(xp - y_r(60), yp - y_r(60), xp + y_r(60), yp + y_r(60), true);
						DrawBox(xp - y_r(60), yp - y_r(60), xp + y_r(60), yp + y_r(60), GetColor(0, 255, 0), FALSE);
						font12.DrawString(xp - y_r(60), yp - y_r(60 + 15), "Camo", GetColor(0, 255, 0));
					}

					{
						int xp = dispx / 2	 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(90)));
						int yp = dispy * 2 / 3	 + int(ber_r * cos(rad + deg2rad(90)));
						int xa = dispx / 2	 + int(((ber_r * 16.f / 9.f) - y_r(150)) * sin(rad + deg2rad(90)));
						int ya = dispy * 2 / 3	 + int((ber_r - y_r(150)) * cos(rad + deg2rad(90)));
						DrawLine(xa, ya, xp, yp, GetColor(0, 255, 0), 2);

						DrawBox(xp - y_r(120), yp - y_r(60), xp + y_r(120), yp + y_r(60), GetColor(0, 0, 0), TRUE);

						font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3	), GetColor(0, 255, 0), "Name  :%s", t[veh.use_id].name.c_str());
						font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 20	), GetColor(0, 255, 0), "Speed :%03.0f/%03.0f km/h", t[veh.use_id].flont_speed_limit, t[veh.use_id].back_speed_limit);
						font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 40	), GetColor(0, 255, 0), "Turn  :%03.0f °/s", t[veh.use_id].body_rad_limit);
						font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 60	), GetColor(0, 255, 0), "Turret:%03.0f °/s", t[veh.use_id].turret_rad_limit);

						DrawBox(xp - y_r(120), yp - y_r(60), xp + y_r(120), yp + y_r(60), GetColor(0, 255, 0), FALSE);
						font12.DrawString(xp - y_r(120), yp - y_r(60 + 15), "Spec", GetColor(0, 255, 0));
					}

					{
						int xp = dispx / 2 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(180)));
						int yp = dispy * 2 / 3 + int(ber_r * cos(rad + deg2rad(180)));
						int xa = dispx / 2 + int(((ber_r * 16.f / 9.f) - y_r(150)) * sin(rad + deg2rad(180)));
						int ya = dispy * 2 / 3 + int((ber_r - y_r(150)) * cos(rad + deg2rad(180)));
						DrawLine(xa, ya, xp, yp, GetColor(0, 255, 0), 2);

						int ys = 20 * int(t[veh.use_id].gunframe.size()) / 2 + 1;

						DrawBox(xp - y_r(120), yp - y_r(ys), xp + y_r(120), yp + y_r(ys), GetColor(0, 0, 0), TRUE);

						if (t[veh.use_id].gunframe.size() == 0) {
							font18.DrawString(xp - y_r(120 - 3), yp - y_r(ys - 3), "N/A", GetColor(0, 255, 0));
						}
						else {
							for (int z = 0; z < t[veh.use_id].gunframe.size(); z++) {
								font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(ys - 3 - 20 * z), GetColor(0, 255, 0), "No.%d  :%s", z, t[veh.use_id].gunframe[z].name.c_str());
							}
						}
						DrawBox(xp - y_r(120), yp - y_r(ys), xp + y_r(120), yp + y_r(ys), GetColor(0, 255, 0), FALSE);
						font12.DrawString(xp - y_r(120), yp - y_r(ys + 15), "Weapon", GetColor(0, 255, 0));
					}

					easing_set(&rad, deg2rad(rad_i + yrad_im), 0.9f, fps);


					GraphHandle::SetDraw_Screen(DX_SCREEN_BACK, 3.0f, 60.f, fov, campos, camaim, VGet(0.f, 1.f, 0.f));
					{
						garage.DrawModel();
						wheel_rad -= speed;
						for (auto& v : t[veh.use_id].wheelframe) {
							t[veh.use_id].obj.SetFrameLocalMatrix(v.frame.first, MATRIX_ref::RotX(wheel_rad) * MATRIX_ref::Mtrans(v.frame.second));
						}
						for (auto& v : t[veh.use_id].wheelframe_nospring) {
							t[veh.use_id].obj.SetFrameLocalMatrix(v.frame.first, MATRIX_ref::RotX(wheel_rad) * MATRIX_ref::Mtrans(v.frame.second));
						}
						t[veh.use_id].obj.SetMatrix(MATRIX_ref::RotX(deg2rad(speed / (60.f / 3.6f / fps) * 2.f)) * MATRIX_ref::Mtrans(pos));
						t[veh.use_id].obj.DrawModel();
					}
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(255 - int(255.f * pos.z() / -10.f), 0, 255));
					bufScreen.DrawGraph(0, 0, true);
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f * pos.z() / -30.f), 0, 255));
					DrawBox(0, 0, dispx, dispy, GetColor(255, 255, 255), TRUE);
					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
					if (pos.z() < -10.f) {
						easing_set(&fov, deg2rad(90 / 2) / 2.f, 0.95f, fps);
					}
					if (pos.z() < -30.f) {
						endp = true;
					}
				}
				if (chara->mode == 1) {
					if (CheckHitKey(KEY_INPUT_SPACE) != 0 || speed != 0.f) {
						speed = std::clamp(speed + 2.5f / 3.6f / fps, 0.f, 200.f / 3.6f / fps);
						pos.zadd(-speed);
						startp = true;
					}
					auto old = veh.camo_sel;
					if (!startp) {
						VECTOR_ref campos_t = (MATRIX_ref::RotY(yrad_m) * MATRIX_ref::RotX(xrad_m)).zvec() * (-30.f) + VGet(0.f, 3.f, 0.f);
						easing_set(&campos,
							campos_t + VGet(float(-200 + GetRand(400)) / 100.f, float(-200 + GetRand(400)) / 100.f, float(-200 + GetRand(400)) / 100.f),
							0.95f,
							fps);

						camaim = pos + VGet(0.f, 3.f, 0.f);
						if (upct == 1) {
							++veh.use_id %= p.size();
							veh.camo_sel = std::min(veh.camo_sel, p[veh.use_id].camog.size() - 1);
						}
						if (dnct == 1) {
							if (veh.use_id == 0) {
								veh.use_id = p.size() - 1;
							}
							else {
								--veh.use_id;
							}
							veh.camo_sel = std::min(veh.camo_sel, p[veh.use_id].camog.size() - 1);
						}
						if (ltct == 1) {
							++veh.camo_sel %= p[veh.use_id].camog.size();
						}
						if (rtct == 1) {
							if (veh.camo_sel == 0) {
								veh.camo_sel = p[veh.use_id].camog.size() - 1;
							}
							else {
								--veh.camo_sel;
							}
						}
					}
					else {
						easing_set(&campos,
							VGet(
							(float(-25 + GetRand(50)) / 100.f) * (1.f - (pos.z() / -120.f)),
								(float(-25 + GetRand(50)) / 100.f) * (1.f - (pos.z() / -120.f)),
								(float(-25 + GetRand(50)) / 100.f) * (1.f - (pos.z() / -120.f)) + 15.f),
							0.95f,
							fps);
						easing_set(&camaim, pos + VGet((float(-200 + GetRand(400)) / 100.f) * (1.f - (pos.z() / -120.f)), (float(-200 + GetRand(400)) / 100.f) * (1.f - (pos.z() / -120.f)) + 3.f, (float(-200 + GetRand(400)) / 100.f) * (1.f - (pos.z() / -120.f))), 0.95f, fps);
					}
					if (p[veh.use_id].camog.size() > 0) {
						SetDrawScreen(CamScreen.get());
						DrawExtendGraph(0, 0, 240, 240, p[veh.use_id].camog[veh.camo_sel], TRUE);
						MV1SetTextureGraphHandle(p[veh.use_id].obj.get(), p[veh.use_id].camo_tex, p[veh.use_id].camog[veh.camo_sel], TRUE);
					}
					GraphFilter(CamScreen.get(), DX_GRAPH_FILTER_GAUSS, 16, 2400);
					if (veh.camo_sel != old) {
						if (std::abs(int(int(veh.camo_sel) - old)) == 1) {
							rad_i += 360 * int(int(veh.camo_sel) - old) / int(p[veh.use_id].camog.size());
						}
						else {
							if (veh.camo_sel == 0) {
								rad_i += 360 / int(p[veh.use_id].camog.size());
							}
							else {
								rad_i -= 360 / int(p[veh.use_id].camog.size());
							}
						}
					}
					bufScreen.SetDraw_Screen();
					{
						int xp = dispx / 2 + int((ber_r * 16.f / 9.f) * sin(rad));
						int yp = dispy * 2 / 3 + int(ber_r * cos(rad));
						int xa = dispx / 2 + int(((ber_r * 16.f / 9.f) - y_r(150)) * sin(rad));
						int ya = dispy * 2 / 3 + int((ber_r - y_r(150)) * cos(rad));
						DrawLine(xa, ya, xp, yp, GetColor(0, 255, 0), 2);
						CamScreen.DrawExtendGraph(xp - y_r(60), yp - y_r(60), xp + y_r(60), yp + y_r(60), true);
						DrawBox(xp - y_r(60), yp - y_r(60), xp + y_r(60), yp + y_r(60), GetColor(0, 255, 0), FALSE);
						font12.DrawString(xp - y_r(60), yp - y_r(60 + 15), "Camo", GetColor(0, 255, 0));
					}

					{
						int xp = dispx / 2 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(90)));
						int yp = dispy * 2 / 3 + int(ber_r * cos(rad + deg2rad(90)));
						int xa = dispx / 2 + int(((ber_r * 16.f / 9.f) - y_r(150)) * sin(rad + deg2rad(90)));
						int ya = dispy * 2 / 3 + int((ber_r - y_r(150)) * cos(rad + deg2rad(90)));
						DrawLine(xa, ya, xp, yp, GetColor(0, 255, 0), 2);

						DrawBox(xp - y_r(120), yp - y_r(60), xp + y_r(120), yp + y_r(60), GetColor(0, 0, 0), TRUE);

						font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3     ), GetColor(0, 255, 0), "Name     :%s", p[veh.use_id].name.c_str());
						font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 20), GetColor(0, 255, 0), "MaxSpeed :%03.0f km/h", p[veh.use_id].max_speed_limit*3.6f);
						font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 40), GetColor(0, 255, 0), "MidSpeed :%03.0f km/h", p[veh.use_id].mid_speed_limit*3.6f);
						font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 60), GetColor(0, 255, 0), "MinSpeed :%03.0f km/h", p[veh.use_id].min_speed_limit*3.6f);
						font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 80), GetColor(0, 255, 0), "Turn     :%03.0f °/s", p[veh.use_id].body_rad_limit);

						DrawBox(xp - y_r(120), yp - y_r(60), xp + y_r(120), yp + y_r(60), GetColor(0, 255, 0), FALSE);
						font12.DrawString(xp - y_r(120), yp - y_r(60 + 15), "Spec", GetColor(0, 255, 0));
					}

					{
						int xp = dispx / 2 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(180)));
						int yp = dispy * 2 / 3 + int(ber_r * cos(rad + deg2rad(180)));
						int xa = dispx / 2 + int(((ber_r * 16.f / 9.f) - y_r(150)) * sin(rad + deg2rad(180)));
						int ya = dispy * 2 / 3 + int((ber_r - y_r(150)) * cos(rad + deg2rad(180)));
						DrawLine(xa, ya, xp, yp, GetColor(0, 255, 0), 2);

						int ys = 20 * int(p[veh.use_id].gunframe.size()) / 2 + 1;

						DrawBox(xp - y_r(120), yp - y_r(ys), xp + y_r(120), yp + y_r(ys), GetColor(0, 0, 0), TRUE);

						if (p[veh.use_id].gunframe.size() == 0) {
							font18.DrawString(xp - y_r(120 - 3), yp - y_r(ys - 3), "N/A", GetColor(0, 255, 0));
						}
						else {
							for (int z = 0; z < p[veh.use_id].gunframe.size(); z++) {
								font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(ys - 3 - 20 * z), GetColor(0, 255, 0), "No.%d  :%s", z, p[veh.use_id].gunframe[z].name.c_str());
							}
						}
						DrawBox(xp - y_r(120), yp - y_r(ys), xp + y_r(120), yp + y_r(ys), GetColor(0, 255, 0), FALSE);
						font12.DrawString(xp - y_r(120), yp - y_r(ys + 15), "Weapon", GetColor(0, 255, 0));
					}


					SkyScreen.SetDraw_Screen(1000.0f, 5000.0f, fov, campos - camaim, VGet(0, 0, 0), VGet(0.f, 1.f, 0.f));
					{
						SetFogEnable(FALSE);
						SetUseLighting(FALSE);
						sky.DrawModel();
						SetUseLighting(TRUE);
						SetFogEnable(TRUE);
					}
					GraphHandle::SetDraw_Screen(DX_SCREEN_BACK, 3.0f, 150.f, fov, campos, camaim, VGet(0.f, 1.f, 0.f));
					{
						SkyScreen.DrawGraph(0, 0, false);
						p[veh.use_id].obj.SetMatrix(MATRIX_ref::Mtrans(pos));
						p[veh.use_id].obj.DrawModel();
					}
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(255 - int(255.f * pos.z() / -10.f), 0, 255));
					bufScreen.DrawGraph(0, 0, true);
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f * (pos.z() + 60.f) / -60.f), 0, 255));
					DrawBox(0, 0, dispx, dispy, GetColor(255, 255, 255), TRUE);
					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
					if (pos.z() <= -30.f && pos.z() > -60.f) {
						easing_set(&fov, deg2rad(90 / 2) / 2.f, 0.95f, fps);
					}
					if (pos.z() <= -60.f) {
						easing_set(&fov, deg2rad(90 / 2) / 4.f, 0.95f, fps);
					}

					if (pos.z() < -120.f) {
						endp = true;
					}
				}

				ScreenFlip();
				if (GetWaitVSyncFlag() == FALSE) {
					while (GetNowHiPerformanceCount() - waits < 1000000.0f / 60.f) {
					}
				}
				if (endp) {
					WaitTimer(100);
					break;
				}
			}

		}
		else {
			chara->vehicle[0].use_id = 3; //戦車
			chara->vehicle[1].use_id = 0; //飛行機
			chara->vehicle[0].camo_sel = 0;
			chara->vehicle[1].camo_sel = 0;
			chara->mode = 0;
		}
		return (CheckHitKey(KEY_INPUT_ESCAPE) == 0);
	}

	void load_window(const char* mes) {
		SetUseASyncLoadFlag(FALSE);
		float bar = 0.f, cnt = 0.f;
		auto all = GetASyncLoadNum();
		while (ProcessMessage() == 0) {
			const auto fps = GetFPS();
			SetDrawScreen(DX_SCREEN_BACK);
			ClearDrawScreen();
			{
				font18.DrawStringFormat(0, dispy - y_r(70), GetColor(0, 255, 0), " loading... : %04d/%04d  ", all - GetASyncLoadNum(), all);
				font12.DrawStringFormat(dispx - font12.GetDrawWidthFormat("%s 読み込み中 ", mes), dispy - y_r(70), GetColor(0, 255, 0), "%s 読み込み中 ", mes);
				DrawBox(0, dispy - y_r(50), int(float(dispx) * bar / float(all)), dispy - y_r(40), GetColor(0, 255, 0), TRUE);
				easing_set(&bar, float(all - GetASyncLoadNum()), 0.95f, fps);
			}
			ScreenFlip();
			if (GetASyncLoadNum() == 0) {
				cnt += 1.f / GetFPS();
				if (cnt > 1 && bar > float(all - GetASyncLoadNum()) * 0.95f) {
					break;
				}
			}
		}
	}
	void draw(
		const VECTOR_ref& aimpos, 
		const hit::Chara& chara, 
		const bool& ads, 
		const float& fps, 
		const bool& auto_aim, 
		const float& auto_aim_dist, 
		const VECTOR_ref& auto_aim_pos, 
		const float& ratio,
		const VECTOR_ref& campos,
		const VECTOR_ref& camvec,
		const VECTOR_ref& camup,
		const VECTOR_ref& eye_pos_ads
	) {
		if (chara.mode == 1) {
			auto scr = GetDrawScreen();
			auto fov = GetCameraFov();
			{
				CompassScreen.SetDraw_Screen(0.1f, 6.0f, fov, VGet(0.f, 0.f, 3.f), VGet(0.f, 0.f, 0.f), VGet(0.f, -1.f, 0.f));
				SetupCamera_Ortho(1.f);
				auto ltd = GetLightDirection();
				SetLightDirection(VGet(0.f, 0.f, -1.f));
				Compass.SetRotationZYAxis(chara.vehicle[1].mat.Inverse().zvec(), chara.vehicle[1].mat.Inverse().yvec(), 0.f);
				Compass.DrawModel();
				SetLightDirection(ltd);
			}
			{
				SetupCamera_Perspective(fov);
				cockpitScreen.SetDraw_Screen(0.01f, 2.0f, fov, VGet(0.f, 0.f, 0.f), VECTOR_ref(camvec) - campos, camup);

				float px = (chara.p_animes_rudder[1].second - chara.p_animes_rudder[0].second)*deg2rad(30);
				float pz = (chara.p_animes_rudder[2].second - chara.p_animes_rudder[3].second)*deg2rad(30);
				float py = (chara.p_animes_rudder[5].second - chara.p_animes_rudder[4].second)*deg2rad(20);

				cockpit.SetFrameLocalMatrix(sticky_f.first, MATRIX_ref::RotY(py) * MATRIX_ref::Mtrans(sticky_f.second));
				cockpit.SetFrameLocalMatrix(stickz_f.first, MATRIX_ref::RotZ(pz) * MATRIX_ref::Mtrans(stickz_f.second));
				cockpit.SetFrameLocalMatrix(stickx_f.first, MATRIX_ref::RotX(px) * MATRIX_ref::Mtrans(stickx_f.second));
				cockpit.SetFrameLocalMatrix(compass_f.first, MATRIX_ref(chara.vehicle[1].mat).Inverse() * MATRIX_ref::Mtrans(compass_f.second));

				cockpit.SetMatrix(MATRIX_ref::Mtrans((cockpit_v + eye_pos_ads)*-1.f)*(chara.vehicle[1].mat));
				cockpit.DrawModel();
			}
			SetDrawScreen(scr);
			SetupCamera_Perspective(fov);
		}

		if (auto_aim) {
			int siz = int(siz_autoaim);

			if (auto_aim_pos.z() >= 0.f && auto_aim_pos.z() <= 1.f) {
				DrawRotaGraph(int(auto_aim_pos.x()) - y_r(siz), int(auto_aim_pos.y()) - y_r(siz), siz_autoaim_pic, deg2rad(0), lock.get(), TRUE);
				DrawRotaGraph(int(auto_aim_pos.x()) + y_r(siz), int(auto_aim_pos.y()) - y_r(siz), siz_autoaim_pic, deg2rad(90), lock.get(), TRUE);
				DrawRotaGraph(int(auto_aim_pos.x()) + y_r(siz), int(auto_aim_pos.y()) + y_r(siz), siz_autoaim_pic, deg2rad(180), lock.get(), TRUE);
				DrawRotaGraph(int(auto_aim_pos.x()) - y_r(siz), int(auto_aim_pos.y()) + y_r(siz), siz_autoaim_pic, deg2rad(270), lock.get(), TRUE);
			}
			easing_set(&siz_autoaim, 32.f * ratio * 100.f / auto_aim_dist, 0.8f, fps);
			easing_set(&siz_autoaim_pic, 1.f, 0.8f, fps);
		}
		else {
			siz_autoaim = float(dispx);
			siz_autoaim_pic = 100.f;
		}

		{
			int siz = int(128.f);
			if (aimpos.z() >= 0.f && aimpos.z() <= 1.f) {
				circle.DrawExtendGraph(int(aimpos.x()) - y_r(siz), int(aimpos.y()) - y_r(siz), int(aimpos.x()) + y_r(siz), int(aimpos.y()) + y_r(siz), TRUE);
			}

			aim.DrawExtendGraph(dispx / 2 - y_r(siz), dispy / 2 - y_r(siz), dispx / 2 + y_r(siz), dispy / 2 + y_r(siz), TRUE);
		}
		if (chara.mode == 0) {
			if (ads) {
				if (dispx > dispy * 16 / 9) {
					DrawBox(0, 0, dispx / 2 - dispy * 16 / 9 / 2, dispy, GetColor(0, 0, 0), TRUE);
					DrawBox(dispx / 2 + dispy * 16 / 9 / 2, 0, dispx, dispy, GetColor(0, 0, 0), TRUE);
					scope.DrawExtendGraph(dispx / 2 - dispy * 16 / 9 / 2, 0, dispx / 2 + dispy * 16 / 9 / 2, dispy, true);
				}
				else {
					DrawBox(0, 0, dispx, dispy / 2 - dispx * 9 / 16 / 2, GetColor(0, 0, 0), TRUE);
					DrawBox(0, dispy / 2 + dispx * 9 / 16 / 2, dispx, dispy, GetColor(0, 0, 0), TRUE);
					scope.DrawExtendGraph(0, dispy / 2 - dispx * 9 / 16 / 2, dispx, dispy / 2 + dispx * 9 / 16 / 2, true);
				}
			}
		}
		if (chara.mode == 1) {
			if (ads) {
				cockpitScreen.DrawGraph(0, 0, true);
			}
		}
		int i = 0;
		for (auto& veh : chara.vehicle[chara.mode].Gun_) {
			int xp = x_r(20 + 30 - (30 * (i + 1) / int(chara.vehicle[chara.mode].Gun_.size())));
			int yp = dispy - y_r(20) + (i - int(chara.vehicle[chara.mode].Gun_.size())) * y_r(42);
			DrawBox(xp, yp, xp + x_r(200), yp + y_r(38), GetColor(128, 128, 128), TRUE);
			if (veh.loadcnt != 0.f) {
				DrawBox(xp, yp, xp + x_r(200 - int(200.f * veh.loadcnt / veh.gun_info.load_time)), yp + y_r(18), GetColor(255, 0, 0), TRUE);
			}
			else {
				DrawBox(xp, yp, xp + x_r(200), yp + y_r(18), GetColor(0, 255, 0), TRUE);
			}
			if (veh.rounds != 0.f) {
				DrawBox(xp, yp + y_r(20), xp + x_r(int(200.f * veh.rounds / veh.gun_info.rounds)), yp + y_r(38), GetColor(255, 192, 0), TRUE);
			}
			font18.DrawString(xp, yp, veh.bullet[veh.usebullet].spec.name, GetColor(255, 255, 255));
			font18.DrawStringFormat(xp + x_r(200) - font18.GetDrawWidthFormat("%04d / %04d", veh.rounds, veh.gun_info.rounds), yp + y_r(20), GetColor(255, 255, 255), "%04d / %04d", veh.rounds, veh.gun_info.rounds);
			i++;
		}
		if (chara.mode == 1) {
			auto& veh = chara.vehicle[chara.mode];

			CompassScreen.DrawExtendGraph(dispx * 2 / 3, dispy * 2 / 3, dispx * 2 / 3 + y_r(240 / 2), dispy * 2 / 3 + y_r(240 / 2), true);

			DrawLine(dispx / 3, dispy / 3, dispx / 3, dispy * 2 / 3, GetColor(255, 255, 255), 3);
			font18.DrawStringFormat(dispx / 3 - font18.GetDrawWidthFormat("SPD %6.2f km/h ", veh.speed * 3.6f), dispy / 2, GetColor(255, 255, 255), "SPD %6.2f km/h", veh.speed * 3.6f);

			DrawLine(dispx * 2 / 3, dispy / 3, dispx * 2 / 3, dispy * 2 / 3, GetColor(255, 255, 255), 3);
			font18.DrawStringFormat(dispx * 2 / 3, dispy / 2, GetColor(255, 255, 255), " %4d m", int(veh.pos.y()));

			if (veh.speed < veh.use_veh.min_speed_limit) {
				font18.DrawString(dispx / 2 - font18.GetDrawWidth("STALL") / 2, dispy / 3, "STALL", GetColor(255, 0, 0));
			}
			if (veh.pos.y() <= 30.f) {
				font18.DrawString(dispx / 2 - font18.GetDrawWidth("GPWS") / 2, dispy / 3 + 18, "GPWS", GetColor(255, 255, 0));
			}
		}

		{
			int xp = dispx / 2 - 300;
			int xs = 600;
			int yp = dispy - y_r(72);
			int ys = y_r(36);
			//
			for (int j = 0; j < chara.vehicle.size(); j++) {
				auto& veh = chara.vehicle[j];
				auto per = (chara.mode == j) ? 255 : 128;
				DrawBox(xp, yp, xp + int(ber[j]), yp + ys * 2 / 3, GetColor(per, 0, 0), TRUE);
				easing_set(&ber[j], float(xs * int(veh.HP) / int(veh.use_veh.HP)), 0.975f, fps);
				DrawBox(xp, yp, xp + xs * int(veh.HP) / int(veh.use_veh.HP), yp + ys * 2 / 3, GetColor(0, per, 0), TRUE);
				SetDrawBright(per, per, per);
				HP_ber.DrawGraph(xp, yp, true);
				font18.DrawStringFormat(
					xp + (xs - font18.GetDrawWidthFormat("%d / %d", int(veh.HP), int(veh.use_veh.HP))) / 2,
					yp + (ys * 2 / 3 - y_r(18)) / 2,
					GetColor(255, 255, 255),
					"%d / %d",
					int(veh.HP),
					int(veh.use_veh.HP));
				SetDrawBright(255, 255, 255);
				yp += ys;
			}
		}
		{
			int j = 0;

			for (auto& h : chara.vehicle[0].HP_m) {
				font18.DrawStringFormat(200, 300 + j * y_r(20), GetColor(255, 255, 255), "HP : %d", h);
				j++;
			}
		}
	}
};

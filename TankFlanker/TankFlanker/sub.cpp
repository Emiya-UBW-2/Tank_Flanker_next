#include "sub.hpp"

void MainClass::write_setting(void) {
	std::ofstream outputfile("data/setting.txt");
	outputfile << "usegrab(1or0)=" + std::to_string(USE_GRAV) + "\n";
	outputfile << "ANTI(1~4)=" + std::to_string(ANTI) + "\n";
	outputfile << "YSync(1or0)=" + std::to_string(USE_YSync) + "\n";
	outputfile << "fps(30or60or120)=" + std::to_string(frate) + "\n";
	outputfile << "windowmode(1or0)=" + std::to_string(USE_windowmode) + "\n";
	outputfile << "drawdist(100~400)=" + std::to_string(drawdist) + "\n";
	outputfile << "groundx(1~16)=" + std::to_string(gnd_x) + "\n";
	outputfile << "shadow(0~3)=" + std::to_string(shade_x) + "\n";
	outputfile << "hostpass(1or0)=" + std::to_string(USE_HOST) + "\n";
	outputfile << "pixellighting(1or0)=" + std::to_string(USE_PIXEL) + "\n";
	outputfile << "se_vol(100~0)=" + std::to_string(se_vol * 100.f) + "\n"; //
	outputfile << "bgm_vol(100~0)=" + std::to_string(se_vol * 100.f) + "\n"; //
	outputfile.close();
}

MainClass::MainClass(void) {
	using namespace std::literals;
	//WIN32_FIND_DATA win32fdt;
	//HANDLE hFind;
	SetOutApplicationLogValidFlag(FALSE); /*log*/
	const auto mdata = FileRead_open("data/setting.txt", FALSE);
	{
		USE_GRAV = bool(getparam_u(mdata));
		ANTI = unsigned char(getparam_u(mdata));
		USE_YSync = bool(getparam_u(mdata));
		frate = (USE_YSync) ? 60.f : getparam_f(mdata);
		USE_windowmode = bool(getparam_u(mdata));
		drawdist = getparam_f(mdata);
		gnd_x = unsigned char(getparam_i(mdata));
		shade_x = unsigned char(getparam_i(mdata));
		USE_HOST = bool(getparam_u(mdata));
		USE_PIXEL = bool(getparam_u(mdata));
		se_vol = getparam_f(mdata) / 100.f;
		bgm_vol = getparam_f(mdata) / 100.f;
	}
	FileRead_close(mdata);

	//SetWindowStyleMode(4);			    /**/
	//SetWindowUserCloseEnableFlag(FALSE);		    /*alt+F4対処*/
	SetMainWindowText("Tank Flanker");		    /*name*/
	SetAeroDisableFlag(TRUE);			    /**/
	SetUseDirect3DVersion(DX_DIRECT3D_11);		    /*directX ver*/
	SetEnableXAudioFlag(FALSE);			    /**/
	Set3DSoundOneMetre(1.0f);			    /*3Dsound*/
	SetGraphMode(dispx, dispy, 32);			    /*解像度*/
	if (ANTI >= 2)					    /**/
		SetFullSceneAntiAliasingMode(ANTI, 3);	    /*アンチエイリアス*/
	SetUsePixelLighting(USE_PIXEL);			    /*ピクセルライティング*/
	SetWaitVSyncFlag(USE_YSync);			    /*垂直同期*/
	ChangeWindowMode(USE_windowmode);		    /*窓表示*/
	DxLib_Init();					    /*init*/
	Effekseer_Init(8000);				    /*Effekseer*/
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);  /*Effekseer*/
	Effekseer_SetGraphicsDeviceLostCallbackFunctions(); /*Effekseer*/
	SetAlwaysRunFlag(TRUE);				    /*background*/
	SetUseZBuffer3D(TRUE);				    /*zbufuse*/
	SetWriteZBuffer3D(TRUE);			    /*zbufwrite*/
	MV1SetLoadModelReMakeNormal(TRUE);		    /*法線*/
	MV1SetLoadModelPhysicsWorldGravity(M_GR);	    /*重力*/
							    //SetSysCommandOffFlag(TRUE)//強制ポーズ対策()
}

MainClass::~MainClass(void) {
	Effkseer_End();
	DxLib_End();
}

void MainClass::set_light(const VECTOR_ref vec) {
	SetLightDirection(vec.get());
}

void MainClass::set_cam(const float neard, const float fard, const VECTOR_ref cam, const VECTOR_ref view, const VECTOR_ref up, const float fov) {
	SetCameraNearFar(neard, fard);
	SetCameraPositionAndTargetAndUpVec(cam.get(), view.get(), up.get());
	SetupCamera_Perspective(deg2rad(fov));
	Set3DSoundListenerPosAndFrontPosAndUpVec(cam.get(), view.get(), up.get());
}

void MainClass::Screen_Flip(void) {
	ScreenFlip();
}
void MainClass::Screen_Flip(LONGLONG waits) {
	ScreenFlip();
	if (!USE_YSync)
		while (GetNowHiPerformanceCount() - waits < 1000000.0f / frate) {}
}

VEHICLE::VEHICLE() {
	WIN32_FIND_DATA win32fdt;
	HANDLE hFind;
	//車両数取得
	hFind = FindFirstFile("data/tanks/*", &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((win32fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (win32fdt.cFileName[0] != '.')) {
				vecs.resize(vecs.size() + 1);
				vecs.back().name = win32fdt.cFileName;
			}
		} while (FindNextFile(hFind, &win32fdt));
	} //else{ return false; }
	FindClose(hFind);
	for (auto& v : vecs) {
		const auto mdata = FileRead_open(("data/tanks/" + v.name + "/data.txt").c_str(), FALSE);
		v.countryc = int(getparam_i(mdata));
		for (auto& s : v.speed_flont)
			s = getparam_f(mdata) / 3.6f;
		for (auto& s : v.speed_back)
			s = getparam_f(mdata) / 3.6f;
		v.vehicle_RD = deg2rad(getparam_f(mdata));
		for (auto& a : v.armer)
					       a = getparam_f(mdata);
		v.gun_lim_LR = bool(getparam_u(mdata));
		for (auto& g : v.gun_lim_)
			g = deg2rad(getparam_f(mdata));
		v.gun_RD = deg2rad(getparam_f(mdata)) / MainClass::get_frate();
		v.gun_[0].reloadtime = int(getparam_i(mdata) * MainClass::get_frate());
		v.gun_[1].reloadtime = 10;
		v.gun_[0].ammosize = getparam_f(mdata) / 1000.f;
		v.gun_[1].ammosize = 0.0075f;
		v.gun_[0].accuracy = int(getparam_f(mdata));
		v.gun_[1].accuracy = int(getparam_f(mdata)); //
		for (size_t i = 0; i < v.ammotype.size(); ++i) {
			v.ammotype[i] = int(getparam_i(mdata));
			v.gun_speed[i] = getparam_f(mdata);
			v.pene[i] = getparam_f(mdata);
		}
		FileRead_close(mdata);
	}
	SetUseASyncLoadFlag(TRUE);
	for (auto& v : vecs) {
		v.model = MV1ModelHandle::Load("data/tanks/" + v.name + "/model.mv1");
		v.colmodel = MV1ModelHandle::Load("data/tanks/" + v.name + "/col.mv1");
		v.inmodel = MV1ModelHandle::Load("data/tanks/" + v.name + "/in/model.mv1");
	}
	//se
	size_t j = 0;
	for (; j < 1; ++j)
		se_[j] = SoundHandle::Load("data/audio/se/engine/shift.wav");
	for (; j < 8; ++j)
		se_[j] = SoundHandle::Load("data/audio/se/eject/" + std::to_string(j - 1) + ".wav");
	for (; j < std::size(se_); ++j)
		se_[j] = SoundHandle::Load("data/audio/se/load/" + std::to_string(j - 8) + ".wav");
	//bgm
	bgm_[0] = SoundHandle::Load("data/audio/bgm/German.wav");
	bgm_[1] = SoundHandle::Load("data/audio/bgm/USSR.wav");
	bgm_[2] = SoundHandle::Load("data/audio/bgm/Japan.wav");
	bgm_[3] = SoundHandle::Load("data/audio/bgm/America.wav");
	bgm_[4] = SoundHandle::Load("data/audio/bgm/win.wav");
	bgm_[5] = SoundHandle::Load("data/audio/bgm/lose.wav");
	SetUseASyncLoadFlag(FALSE);
}

VEHICLE::~VEHICLE() {
}

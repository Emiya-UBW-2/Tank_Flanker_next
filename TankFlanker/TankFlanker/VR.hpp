#pragma once
#include"DXLib_ref.h"
#include <fstream>
#include <array>
#include <vector>
#include <D3D11.h>
#include <openvr.h>
#include <memory>

class VRDraw {
public:
	struct systems {
		int id = 0;
		VECTOR_ref pos = VGet(0, 0, 0);
		VECTOR_ref xvec = VGet(1, 0, 0);
		VECTOR_ref yvec = VGet(0, 1, 0);
		VECTOR_ref zvec = VGet(0, 0, 1);
		std::array<uint64_t, 2> on{ 0 };
		MV1 obj;
		VECTOR_ref touch;
		char num = 0;
		vr::ETrackedDeviceClass type = vr::TrackedDeviceClass_Invalid;
		bool turn = false, now = false;
	};
private:
	bool use_vr = true;

	vr::IVRSystem* m_pHMD=nullptr;
	vr::EVRInitError eError = vr::VRInitError_None;
	std::vector<systems> ctrl;							/*HMD,controller*/
	float fov = 90.f;
	char deviceall = 0;
	VECTOR_ref pos, add;
	char hmd_num = -1;
	char left_hand = -1;
	char right_hand = -1;
public:
	const auto& get_hmd_num(void) { return hmd_num; }
	const auto& get_left_hand_num(void) { return left_hand; }
	const auto& get_right_hand_num(void) { return right_hand; }
	const auto& get_fov(void) { return fov; }
	const auto& get_deviceall(void) { return deviceall; }
	const auto& get_m_pHMD(void) { return m_pHMD; }
	const auto& get_eError(void) { return vr::VR_GetVRInitErrorAsSymbol(eError); }
	auto* get_device(void) { return &ctrl; }
	VRDraw(bool* usevr) {
		use_vr = *usevr;
		if (use_vr) {
			eError = vr::VRInitError_None;
			m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
			if (eError != vr::VRInitError_None) {
				m_pHMD = 0;
				use_vr = false;
				*usevr = false;
			}
		}
	}
	~VRDraw(void) {
		if (use_vr) {
			if (m_pHMD) {
				//vr::VR_Shutdown();
				m_pHMD = NULL;
			}
		}
	}
	void Set_Device(void) {
		if (use_vr) {
			ctrl.resize(5);
			if (m_pHMD) {
				deviceall = 0;
				for (char k = 0; k < char(ctrl.size()); k++) {
					ctrl[k].turn = false;
					ctrl[k].now = false;
					ctrl[k].id = k;
				}
				int i = 0;
				for (char k = 0; k < char(ctrl.size()); k++) {
					if (m_pHMD->GetTrackedDeviceClass(k) == vr::TrackedDeviceClass_HMD) {
						hmd_num = deviceall;
						ctrl[deviceall].num = k;
						ctrl[deviceall].type = m_pHMD->GetTrackedDeviceClass(k);
						ctrl[deviceall].turn = true;
						deviceall++;
					}
					else if (m_pHMD->GetTrackedDeviceClass(k) == vr::TrackedDeviceClass_Controller) {
						if (i == 0) {
							//MV1::LoadonAnime("data/model/hand/model_left.mv1", &ctrl[deviceall].obj);
							left_hand = deviceall;
							i++;
						}
						else if (i == 1) {
							//MV1::LoadonAnime("data/model/hand/model_right.mv1", &ctrl[deviceall].obj);
							right_hand = deviceall;
							i++;
						}
						ctrl[deviceall].obj.loop_anime(0);
						ctrl[deviceall].num = k;
						ctrl[deviceall].type = m_pHMD->GetTrackedDeviceClass(k);
						ctrl[deviceall].turn = true;
						deviceall++;
					}
					else if (m_pHMD->GetTrackedDeviceClass(k) == vr::TrackedDeviceClass_TrackingReference) {
						//MV1::LoadonAnime("data/model/hand/model_left.mv1", &ctrl[deviceall].obj);
						//ctrl[deviceall].obj.loop_anime(0);
						ctrl[deviceall].num = k;
						ctrl[deviceall].type = m_pHMD->GetTrackedDeviceClass(k);
						ctrl[deviceall].turn = true;
						deviceall++;
					}
				}
				ctrl.resize(deviceall);
			}
		}
	}
	void Work_Anime(void) {
		if (use_vr) {
			for (auto& c : ctrl) {
				c.obj.work_anime(1.f);
			}
		}
	}
	void Move_Player(void) {
		if (use_vr) {
			if (m_pHMD) {
				vr::TrackedDevicePose_t tmp;
				vr::VRControllerState_t night;
				for (auto& c : ctrl) {
					if (c.type == vr::TrackedDeviceClass_HMD) {
						m_pHMD->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0.0f, &tmp, 1);
						c.on[0] = 0;
						c.on[1] = 0;
						c.touch = VGet(0, 0, 0);
						c.now = tmp.bPoseIsValid;
						c.pos = VGet(tmp.mDeviceToAbsoluteTracking.m[0][3], tmp.mDeviceToAbsoluteTracking.m[1][3], -tmp.mDeviceToAbsoluteTracking.m[2][3]);
						c.xvec = VGet(tmp.mDeviceToAbsoluteTracking.m[0][0], tmp.mDeviceToAbsoluteTracking.m[1][0], -tmp.mDeviceToAbsoluteTracking.m[2][0]);
						c.yvec = VGet(tmp.mDeviceToAbsoluteTracking.m[0][1], tmp.mDeviceToAbsoluteTracking.m[1][1], -tmp.mDeviceToAbsoluteTracking.m[2][1]);
						c.zvec = VGet(-tmp.mDeviceToAbsoluteTracking.m[0][2], -tmp.mDeviceToAbsoluteTracking.m[1][2], tmp.mDeviceToAbsoluteTracking.m[2][2]);
					}
					else if (c.type == vr::TrackedDeviceClass_Controller || c.type == vr::TrackedDeviceClass_TrackingReference) {
						m_pHMD->GetControllerStateWithPose(vr::TrackingUniverseStanding, c.num, &night, sizeof(night), &tmp);
						c.on[0] = night.ulButtonPressed;
						c.on[1] = night.ulButtonTouched;
						c.touch = VGet(night.rAxis[0].x, night.rAxis[0].y, 0);
						c.now = tmp.bPoseIsValid;
						c.pos = VGet(tmp.mDeviceToAbsoluteTracking.m[0][3], tmp.mDeviceToAbsoluteTracking.m[1][3], -tmp.mDeviceToAbsoluteTracking.m[2][3]);
						c.xvec = VGet(tmp.mDeviceToAbsoluteTracking.m[0][0], tmp.mDeviceToAbsoluteTracking.m[1][0], -tmp.mDeviceToAbsoluteTracking.m[2][0]);
						c.yvec = VGet(tmp.mDeviceToAbsoluteTracking.m[0][1], tmp.mDeviceToAbsoluteTracking.m[1][1], -tmp.mDeviceToAbsoluteTracking.m[2][1]);
						c.zvec = VGet(-tmp.mDeviceToAbsoluteTracking.m[0][2], -tmp.mDeviceToAbsoluteTracking.m[1][2], tmp.mDeviceToAbsoluteTracking.m[2][2]);
					}
				}
			}
			else {
				for (auto& c : ctrl) {
					c.on[0] = 0;
					c.on[1] = 0;
					c.touch = VGet(0, 0, 0);
					c.pos = VGet(0, 0, 0);
					c.xvec = VGet(1, 0, 0);
					c.yvec = VGet(0, 1, 0);
					c.zvec = VGet(0, 0, 1);
				}
			}
			/*
			if (left_hand!=-1 && ctrl[left_hand].turn && ctrl[left_hand].now) {
				float rad = atan2f(ctrl[left_hand].zvec.x(), ctrl[left_hand].zvec.z());
				if ((ctrl[left_hand].on[0] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_SteamVR_Touchpad)) != 0) {
					add.xadd(((ctrl[left_hand].touch.x() * cosf(rad) + ctrl[left_hand].touch.y() * sinf(rad)) * 2.0f / 90.0f - add.x()) * 0.1f);
					add.zadd(((ctrl[left_hand].touch.y() * cosf(rad) - ctrl[left_hand].touch.x() * sinf(rad)) * 2.0f / 90.0f - add.z()) * 0.1f);
					fov += (105.0f - fov) * 0.1f;
				}
				else if ((ctrl[left_hand].on[1] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_SteamVR_Touchpad)) != 0) {
					add.xadd(((ctrl[left_hand].touch.x() * cosf(rad) + ctrl[left_hand].touch.y() * sinf(rad)) * 1.0f / 90.0f - add.x()) * 0.1f);
					add.zadd(((ctrl[left_hand].touch.y() * cosf(rad) - ctrl[left_hand].touch.x() * sinf(rad)) * 1.0f / 90.0f - add.z()) * 0.1f);
					fov += (107.5f - fov) * 0.1f;
				}
				else {
					add.xadd(-add.x()*0.1f);
					add.zadd(-add.z()*0.1f);
					fov += (110.0f - fov) * 0.1f;
				}
				pos += add;
			}
			*/
			/*”½‰f*/
			for (auto& c : ctrl) {
				c.pos += pos;
				if (c.turn && c.now) {
					c.obj.SetPosition(c.pos);
					c.obj.SetRotationZYAxis(c.zvec, c.yvec, 0.0f);
				}
			}
		}
	}
	void Draw_Player(void) {
		for (auto& c : ctrl) {
			if (c.turn && c.now)
				c.obj.DrawModel();
		}
	}
	inline VECTOR_ref SetEyePositionVR(const char& eye_type) {
		if (use_vr&&m_pHMD) {
			const vr::HmdMatrix34_t tmpmat = vr::VRSystem()->GetEyeToHeadTransform((vr::EVREye)eye_type);
			return ctrl[hmd_num].pos + ctrl[hmd_num].xvec*(tmpmat.m[0][3]) + ctrl[hmd_num].yvec*(tmpmat.m[1][3]) + ctrl[hmd_num].zvec*(-tmpmat.m[2][3]);
		}
		else {
			return VGet(0, 0, 0);
		}
	}
	inline void PutEye(ID3D11Texture2D* texte, const char& i) {
		if (use_vr) {
			vr::Texture_t tex = { (void*)texte, vr::ETextureType::TextureType_DirectX,vr::EColorSpace::ColorSpace_Auto };
			vr::VRCompositor()->Submit((vr::EVREye)i, &tex, NULL, vr::Submit_Default);
		}
	}
	inline void Eye_Flip(const LONGLONG& waits) {
		if (use_vr&&m_pHMD) {
			vr::TrackedDevicePose_t tmp;
			vr::VRCompositor()->WaitGetPoses(&tmp, 1, NULL, 1);
		}
		else {
			while (GetNowHiPerformanceCount() - waits < 1000000.0f / 90.0f) {}
		}
	}
};

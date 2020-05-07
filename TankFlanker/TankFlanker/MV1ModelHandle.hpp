#pragma once
#include <DxLib.h>
#include <string_view>
#include <vector>

class MV1 {
private:
	int handle_;
	struct ani {
		unsigned char phase= 0;
		int handle	 = 0;
		float per	  = 0.f;
		float time	 = 0.f;
		float alltime      = 0.f;
	};
	std::vector<ani> anime;

	MV1(int h)
	noexcept
		: handle_(h) {}
	static constexpr int invalid_handle= -1;

public:
	/*コンストラクタ*/
	MV1()
	noexcept {
		handle_= invalid_handle;
	}
	MV1(const MV1&)= delete;
	MV1(MV1&& o)
	noexcept
		: handle_(o.handle_) { o.handle_= invalid_handle; }
	/*オペレーター*/
	MV1& operator=(const MV1&)= delete;
	MV1& operator		  =(MV1&& o) noexcept {
		     this->handle_= o.handle_;
		     o.handle_    = invalid_handle;
		     return *this;
	}
	/*デストラクタ*/
	~MV1() noexcept {
		Dispose();
	}
	/**/
	int get() const noexcept { return this->handle_; }
	/*モデル*/
	bool SetPosition(const VECTOR_ref& p1) const noexcept { return MV1SetPosition(this->handle_, p1.get()) == TRUE; }
	VECTOR_ref GetPosition(void) const noexcept { return MV1GetPosition(this->handle_); }
	bool SetRotationZYAxis(const VECTOR_ref& zaxis, const VECTOR_ref& yaxis, float zrad) const noexcept { return MV1SetRotationZYAxis(this->handle_, zaxis.get(), yaxis.get(), zrad) == TRUE; }
	bool SetMatrix(const MATRIX_ref& mat) const noexcept { return MV1SetMatrix(this->handle_, mat.get()) == TRUE; }
	bool DrawModel() const noexcept { return MV1DrawModel(this->handle_) == TRUE; }
	bool SetOpacityRate(const float& p1) const noexcept { return MV1SetOpacityRate(this->handle_, p1) == TRUE; }
	bool SetScale(const VECTOR_ref& p1) const noexcept { return MV1SetScale(this->handle_, p1.get()) == TRUE; }
	/*テクスチャ*/
	bool SetTextureGraphHandle(const int& p1, const GraphHandle& p2, bool trans) const noexcept { return MV1SetTextureGraphHandle(this->handle_, p1, p2.get(), trans ? TRUE : FALSE) == TRUE; }
	/*フレーム*/
	VECTOR_ref frame(const int& p1) const noexcept { return MV1GetFramePosition(this->handle_, p1); }
	size_t frame_num(void) const noexcept { return MV1GetFrameNum(this->handle_); }
	size_t frame_parent(const int& p1) const noexcept { return MV1GetFrameParent(this->handle_, p1); }
	size_t frame_child_num(const int& p1) const noexcept { return MV1GetFrameChildNum(this->handle_, p1); }
	bool SetFrameLocalMatrix(const int& id, MATRIX_ref mat) const noexcept { return MV1SetFrameUserLocalMatrix(this->handle_, id, mat.get()) == TRUE; }

	MATRIX_ref GetFrameLocalMatrix(const int& id) const noexcept { return MV1GetFrameLocalMatrix(this->handle_, id); }


	bool DrawFrame(const int& p1) const noexcept { return MV1DrawFrame(this->handle_, p1) == TRUE; }
	std::string frame_name(const int& p1) noexcept { return MV1GetFrameName(this->handle_, p1); }

	void frame_reset(const int& p1) const noexcept { MV1ResetFrameUserLocalMatrix(this->handle_, p1); }
	/*マテリアル*/
	size_t material_num(void) const noexcept { return MV1GetMaterialNum(this->handle_); }
	void material_AlphaTestAll(bool Enable, int mode, int param) const noexcept {
		MV1SetMaterialDrawAlphaTestAll(this->handle_, Enable ? TRUE : FALSE, mode, param);
	}
	std::string material_name(const int& p1) noexcept { return MV1GetMaterialName(this->handle_, p1); }
	/*メッシュ*/
	size_t mesh_num(void) const noexcept { return MV1GetMeshNum(this->handle_); }
	VECTOR_ref mesh_maxpos(const int& p1) const noexcept { return MV1GetMeshMaxPosition(this->handle_, p1); }
	VECTOR_ref mesh_minpos(const int& p1) const noexcept { return MV1GetMeshMinPosition(this->handle_, p1); }
	bool DrawMesh(const int& p1) const noexcept { return MV1DrawMesh(this->handle_, p1) == TRUE; }
	/*シェイプ*/
	int SearchShape(const char* str) const noexcept { return MV1SearchShape(this->handle_, str); }
	bool SetShapeRate(const int& p1, const float& p2) const noexcept { return MV1SetShapeRate(this->handle_, p1, p2) == TRUE; }
	/*アニメーション*/
	bool work_anime(const float& pers) {
		/*
		if (pers == 1.f)
		{
			int on = 0;
			for (auto& a : this->anime) {
				if (a.phase != 0)
					on++;
			}
			if (on > 1) {
				pers = 1.f / float(on);
			}
		}
		*/
		for (auto& a : this->anime) {
			switch (a.phase) {
				case 0:
					//停止
					a.per = 0.f;
					a.time= 0;
					break;
				case 1:
					//1回再生開始
					a.per= pers;
					a.time+= 1.0f / 60.f;
					if (a.time >= a.alltime) {
						a.time = 0;
						a.phase= 0;
					}
					break;
				case 2:
					//連続再生開始
					a.per= pers;
					a.time+= 1.0f / 60.f;
					if (a.time >= a.alltime) {
						a.time= 0;
					}
					break;
				default:
					a.phase= 0;
					break;
			}
			MV1SetAttachAnimTime(this->handle_, a.handle, a.time);
			MV1SetAttachAnimBlendRate(this->handle_, a.handle, a.per);
		}
		return true;
	}
	bool start_anime(int sel) {
		if (sel < int(this->anime.size()) && this->anime.size() != 0) {
			if (this->anime[sel].phase != 0) {
				return false;
			}
			this->anime[sel].phase= 1;
			this->anime[sel].time = 0;
			return true;
		}
		return false;
	}
	bool loop_anime(int sel) {
		if (sel < int(this->anime.size()) && this->anime.size() != 0) {
			if (this->anime[sel].phase != 0) {
				return false;
			}
			this->anime[sel].phase= 2;
			this->anime[sel].time = 0;
			return true;
		}
		return false;
	}
	bool stop_anime(int sel) {
		if (sel < int(this->anime.size()) && this->anime.size() != 0) {
			this->anime[sel].phase= 0;
			this->anime[sel].time = 0;
			return true;
		}
		return false;
	}
	/*物理演算*/
	bool PhysicsResetState() const noexcept { return MV1PhysicsResetState(this->handle_) == TRUE; }
	bool PhysicsCalculation(const float& p1) const noexcept { return MV1PhysicsCalculation(this->handle_, p1) == TRUE; }
	/*当たり判定*/
	bool SetupCollInfo(const int& x= 32, const int& y= 8, const int& z= 32, const int& frame= -1, const int& mesh= -1) const noexcept {
		return MV1SetupCollInfo(this->handle_, frame, x, y, z, mesh) == TRUE;
	}
	bool RefreshCollInfo(const int& frame= -1, const int& mesh= -1) const noexcept {
		return MV1RefreshCollInfo(this->handle_, frame, mesh) == TRUE;
	}
	const auto CollCheck_Line(const VECTOR_ref& start, const VECTOR_ref& end, const int& frame= -1, const int& mesh= -1) const noexcept {
		return MV1CollCheck_Line(this->handle_, frame, start.get(), end.get(), mesh);
	}
	/*読み込み*/
	MV1 Duplicate() const noexcept { return DxLib::MV1DuplicateModel(this->handle_); }
	static void Load(std::basic_string_view<TCHAR> FileName, MV1* t, const bool& Async) noexcept {
		if (Async) {
			SetUseASyncLoadFlag(TRUE);
 		}
		*t= DxLib::MV1LoadModelWithStrLen(FileName.data(), FileName.length());
		t->anime.clear();
		if (Async) {
			SetUseASyncLoadFlag(FALSE);
		}
		return;
	}
	static void LoadonAnime(std::basic_string_view<TCHAR> FileName, MV1* t, const int& mode= DX_LOADMODEL_PHYSICS_LOADCALC) noexcept {
		if (mode != DX_LOADMODEL_PHYSICS_LOADCALC) {
			MV1SetLoadModelUsePhysicsMode(mode);
		}

		*t= DxLib::MV1LoadModelWithStrLen(FileName.data(), FileName.length());

		t->anime.resize(MV1GetAnimNum(t->get()));
		if (t->anime.size() > 0) {
			for (int i= 0; i < int(t->anime.size()); i++) {
				t->anime[i].handle= MV1AttachAnim(t->get(), i);
				t->anime[i].per   = 0.f;
				MV1SetAttachAnimBlendRate(t->get(), t->anime[i].handle, t->anime[i].per);
				t->anime[i].time   = 0;
				t->anime[i].alltime= MV1GetAttachAnimTotalTime(t->get(), t->anime[i].handle);
				t->anime[i].phase  = 0;
			}
		}

		if (mode != DX_LOADMODEL_PHYSICS_LOADCALC) {
			MV1SetLoadModelUsePhysicsMode(DX_LOADMODEL_PHYSICS_LOADCALC);
		}
		return;
	}
	/*削除*/
	void Dispose() noexcept {
		if (this->handle_ != -1) {
			MV1DeleteModel(this->handle_);
			anime.clear();
			this->handle_= -1;
		}
	}
};

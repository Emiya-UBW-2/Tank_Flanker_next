#pragma once
#include <DxLib.h>
#include <string_view>
class GraphHandle {
private:
	int handle_;
	constexpr GraphHandle(int h) noexcept : handle_(h) {}
	static constexpr int invalid_handle = -1;

public:
	constexpr GraphHandle() noexcept : handle_(invalid_handle) {}
	GraphHandle(const GraphHandle&) = delete;
	GraphHandle(GraphHandle&& o) noexcept : handle_(o.handle_) {
		o.handle_ = invalid_handle;
	}
	GraphHandle& operator=(const GraphHandle&) = delete;
	GraphHandle& operator=(GraphHandle&& o) noexcept {
		this->handle_ = o.handle_;
		o.handle_ = invalid_handle;
		return *this;
	}
	~GraphHandle() noexcept {
		if (this->handle_ != -1) { DeleteGraph(this->handle_); }
	}
	void Dispose() noexcept {
		if (this->handle_ != -1) {
			DeleteGraph(this->handle_);
			this->handle_ = -1;
		}
	}
	int get() const noexcept { return handle_; }

	GraphHandle Duplicate() const noexcept { return this->handle_; }

	static GraphHandle Load(std::basic_string_view<TCHAR> FileName, bool NotUse3DFlag = false) noexcept {
		return { DxLib::LoadGraphWithStrLen(FileName.data(), FileName.length(), NotUse3DFlag) };
	}
	static GraphHandle LoadDiv(std::basic_string_view<TCHAR> FileName, int AllNum, int XNum, int YNum, int   XSize, int   YSize, int *HandleArray, bool NotUse3DFlag = false) noexcept {
		return { DxLib::LoadDivGraphWithStrLen(FileName.data(), FileName.length(), AllNum, XNum, YNum,   XSize, YSize, HandleArray, NotUse3DFlag) };
	}
		
	static GraphHandle Make(int SizeX, int SizeY, bool trns = false) noexcept {
		return { DxLib::MakeScreen(SizeX, SizeY, (trns ? TRUE : FALSE)) };
	}

	void DrawGraph(int posx,int posy,bool trns) noexcept {
		if (this->handle_ != -1) {
			DxLib::DrawGraph(posx, posy, this->handle_, (trns ? TRUE : FALSE));
		}
	}
	void DrawExtendGraph(int posx1, int posy1, int posx2, int posy2, bool trns) noexcept {
		if (this->handle_ != -1) {
			DxLib::DrawExtendGraph(posx1, posy1, posx2, posy2, this->handle_, (trns ? TRUE : FALSE));
		}
	}
	//
	void SetDraw_Screen() {
		SetDrawScreen(this->handle_);
		ClearDrawScreen();
	}
	void SetDraw_Screen(const float& near_, const float& far_, const float& fov, const VECTOR_ref& campos, const VECTOR_ref& camvec, const VECTOR_ref& camup) {
		SetDraw_Screen();
		SetCameraNearFar(near_, far_);
		SetupCamera_Perspective(fov);
		SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
	}
	//
	static void SetDraw_Screen(const int& handle) {
		SetDrawScreen(handle);
		ClearDrawScreen();
	}
	static void SetDraw_Screen(const int& handle, const float& near_, const float& far_, const float& fov, const VECTOR_ref& campos, const VECTOR_ref& camvec, const VECTOR_ref& camup) {
		SetDraw_Screen(handle);
		SetCameraNearFar(near_, far_);
		SetupCamera_Perspective(fov);
		SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
	}
};

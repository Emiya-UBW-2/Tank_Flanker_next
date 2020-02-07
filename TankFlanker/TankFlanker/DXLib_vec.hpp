#pragma once
#include <DxLib.h>
#include "DXLib_mat.hpp"
#include <string_view>
class VECTOR_ref {
	VECTOR value;

public:
	//入力
	VECTOR_ref() noexcept : value(DxLib::VGet(0, 0, 0)) {}
	VECTOR_ref(VECTOR value) { this->value = value; }
	//加算
	VECTOR_ref operator+(VECTOR_ref obj) {
		return VECTOR_ref(DxLib::VAdd(this->value, obj.value));
	}
	VECTOR_ref operator+=(VECTOR_ref obj) {
		this->value = DxLib::VAdd(this->value, obj.value);
		return this->value;
	}
	//減算
	VECTOR_ref operator-(VECTOR_ref obj) const noexcept {
		return VECTOR_ref(DxLib::VSub(this->value, obj.value));
	}
	VECTOR_ref operator-=(VECTOR_ref obj){
		this->value = DxLib::VSub(this->value, obj.value);
		return this->value;
	}
	//スケーリング
	VECTOR_ref operator*(float p1) const noexcept {
		return VECTOR_ref(DxLib::VScale(this->value, p1)); 
	}
	VECTOR_ref operator*=(float p1){
		this->value = DxLib::VScale(this->value, p1);
		return this->value;
	}
	//外積
	VECTOR_ref cross(VECTOR_ref obj) const noexcept {
		return VECTOR_ref(DxLib::VCross(this->value, obj.value));
	}
	//内積
	float dot(VECTOR_ref obj) const noexcept {
		return DxLib::VDot(this->value, obj.value);
	}
	//行列取得
	MATRIX_ref Mtrans() const noexcept { return DxLib::MGetTranslate(this->value); }
	//MATRIX MRotAxis(float p1) const noexcept { return DxLib::MGetRotAxis(this->value, p1); }
	//正規化
	VECTOR_ref Norm() const noexcept { return VECTOR_ref(DxLib::VNorm(this->value)); }
	//サイズ
	float size() const noexcept { return DxLib::VSize(this->value); }
	//出力
	VECTOR get() const noexcept { return this->value; }
	//それぞれの値
	float x() const noexcept { return this->value.x; }
	float x(float px) {
		this->value.x = px;
		return this->value.x;
	}
	float y() const noexcept { return this->value.y; }
	float y(float py) {
		this->value.y = py;
		return this->value.y;
	}
	float z() const noexcept { return this->value.z; }
	float z(float pz) {
		this->value.z = pz;
		return this->value.z;
	}
};

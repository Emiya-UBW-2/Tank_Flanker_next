#pragma once
#include <DxLib.h>
#include <string_view>
class MATRIX_ref {
	MATRIX value;

public:
	//“ü—Í
	MATRIX_ref() noexcept : value(DxLib::MGetIdent()){}
	MATRIX_ref(MATRIX value) { this->value = value; }
	//‰ÁŽZ
	MATRIX_ref operator+(MATRIX_ref obj) {
		return MATRIX_ref(DxLib::MAdd(this->value, obj.value));
	}
	MATRIX_ref operator+=(MATRIX_ref obj) {
		this->value = DxLib::MAdd(this->value, obj.value);
		return this->value;
	}
	//Œ¸ŽZ
	/*
	MATRIX_ref operator-(MATRIX_ref obj) const noexcept {
		return MATRIX_ref(DxLib::VSub(this->value, obj.value));
	}
	MATRIX_ref operator-=(MATRIX_ref obj) {
		this->value = DxLib::VSub(this->value, obj.value);
		return MATRIX_ref(DxLib::VSub(this->value, obj.value));
	}
	*/
	//æŽZ
	MATRIX_ref operator*(MATRIX_ref obj) const noexcept {
		return MATRIX_ref(DxLib::MMult(this->value, obj.value));
	}
	MATRIX_ref operator*=(MATRIX_ref obj) {
		this->value = DxLib::MMult(this->value, obj.value);
		return this->value;
	}
	//MATRIX MRotAxis(float p1) const noexcept { return DxLib::MGetRotAxis(this->value, p1); }
	MATRIX_ref scale(float p1) const noexcept { return DxLib::MScale(this->value, p1); }
	//
	MATRIX_ref inverse(void) const noexcept { return DxLib::MInverse(this->value); }
	//o—Í
	MATRIX get() const noexcept { return this->value; }
};

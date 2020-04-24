#pragma once

#include "DxLib.h"
#include "sub.hpp"
#include "useful.hpp"

#include "DXLib_vec.hpp"
#include "SoundHandle.hpp"
#include "GraphHandle.hpp"
#include "FontHandle.hpp"
#include "MV1ModelHandle.hpp"

#include <windows.h>
#include <fstream>
#include <string_view>
#include <optional>
#include <array>
#include <vector>
#include <utility>
#include <D3D11.h>

#include <thread> 

//“ü—Í
struct systems {
};

struct input {
};
//o—Í
struct output {
};
//60fps‚ğˆÛ‚µ‚Â‚Â‘€ì‚ğ‰‰Z
class ThreadClass {
private:
	std::thread thread_1;
	void calc(input& p_in, output& p_out) {
		return;
	}
public:
	ThreadClass() {
	}
	~ThreadClass() {
		thead_stop();
	}
	void thread_start(input& p_in, output& p_out) {
		thread_1 = std::thread([&] { calc(p_in, p_out); });
	}
	void thead_stop() {
		if (thread_1.joinable()) {
			thread_1.detach();

		}
	}
};

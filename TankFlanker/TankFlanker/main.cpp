#define NOMINMAX
#include "sub.hpp"
#include "useful.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	auto parts = std::make_unique<MainClass>(); /*汎用クラス*/

	DrawPixel(320, 240, GetColor(255, 255, 255)); // 点を打つ

	WaitKey(); // キー入力待ち

	return 0; // ソフトの終了
}

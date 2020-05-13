#pragma once

class DeBuG {
private:
	int frate;
	std::vector<std::array<float, 6 + 1>> deb;
	LONGLONG waypoint = 0;
	std::array<float, 6> waydeb{ 0.f };
	size_t seldeb;
	FontHandle font;
	const int fontsize = 12;
public:
	DeBuG(const int& fps_rate = 60) {
		frate = fps_rate;
		font = FontHandle::Create(fontsize, DX_FONTTYPE_EDGE);
		deb.resize(frate);
	}
	void put_way(void) {
		waypoint = GetNowHiPerformanceCount();
		seldeb = 0;
	}
	void end_way(void) {
		if (seldeb < 6)
			waydeb[seldeb++] = (float)(GetNowHiPerformanceCount() - waypoint) / 1000.0f;
	}
	void debug(int xpos, int ypos, float fps, float time) {
		int wide = 180;
		int hight = int(waydeb.size() + 1) * fontsize;
		deb[0][0] = time;
		for (size_t j = deb.size() - 1; j >= 1; --j) {
			deb[j][0] = deb[j - 1][0];
		}
		for (size_t i = 0; i < waydeb.size(); ++i) {
			if (seldeb - 1 <= i) {
				waydeb[i] = waydeb[seldeb - 1];
			}
			deb[0][i + 1] = waydeb[i];
			for (size_t j = std::size(deb) - 1; j >= 1; --j) {
				deb[j][i + 1] = deb[j - 1][i + 1];
			}
		}

		DrawBox(xpos, ypos, xpos + wide, ypos + hight * 100 / frate, GetColor(255, 0, 0), FALSE);
		for (int j = 0; j < int(deb.size() - 1); ++j) {
			for (int i = 0; i < 6; ++i) {
				DXDraw::Line2D(
					xpos + j * wide / frate, ypos + hight * 100 / frate - int(deb[j][i + 1] * 5.f),
					xpos + (j + 1) * wide / frate, ypos + hight * 100 / frate - int(deb[j + 1][i + 1] * 5.f),
					GetColor(50, 128 + 127 * i / 6, 50));
			}
			DXDraw::Line2D(
				xpos + j * wide / frate, ypos + hight * 100 / frate - int(deb[j][0] * 5.f),
				xpos + (j + 1) * wide / frate, ypos + hight * 100 / frate - int(deb[j + 1][0] * 5.f),
				GetColor(255, 255, 0));
		}
		const auto c_ffffff = GetColor(255, 255, 255);
		DXDraw::Line2D(xpos, ypos + hight * 50 / frate, xpos + wide, ypos + hight * 50 / frate, GetColor(0, 255, 0));

		font.DrawStringFormat(xpos, ypos, c_ffffff, "%05.2ffps ( %.2fms)", fps, time);

		font.DrawStringFormat(xpos, ypos + fontsize, c_ffffff, "%d(%.2fms)", 0, waydeb[0]);
		for (size_t j = 1; j < waydeb.size(); ++j) {
			font.DrawStringFormat(xpos, ypos + int(j + 1) * fontsize, c_ffffff, "%d(%.2fms)", j, waydeb[j] - waydeb[j - 1u]);
		}
	}
};
#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include "singleton.h"
#include "imgui/imgui.h"

DWORD keys[] = { VK_LMENU, VK_SHIFT, VK_CONTROL, VK_LBUTTON, VK_RBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
const char* keyItems[] = { "LAlt", "LShift", "LControl", "LMouse", "RMouse", "Mouse4", "Mouse5" };

inline namespace Configuration
{
	class Settings : public Singleton<Settings>
	{
	public:

		const char* BoxTypes[2] = { "Full Box","Cornered Box" };
		const char* LineTypes[3] = { "Bottom To Enemy","Top To Enemy","Crosshair To Enemy" };


		bool b_MenuShow = true;


		bool b_Visual = true;
		bool b_EspBox = true;
		bool b_EspSkeleton = true;
		bool b_EspLine = true;
		bool b_EspDistance = true;
		bool b_EspHealth  = true;
		bool b_EspName = true;

		bool crosshair = true;

		bool b_Aimbot = true;
		bool b_AimbotFOV = true;
		bool b_AimbotSmooth = false;

		bool debug_b = false;

		bool b_EspHealthHP = false;

		bool ignoreteam = false;

		bool Kills = true;
		bool Levels = true;
		bool System = true;

		bool b_NoRecoil = false;


		ImColor VisibleColor = ImColor(255.f / 255, 0.f, 0.f);
		float fl_VisibleColor[3] = { 0.f,255.f / 255,0.f };  //

		ImColor InvisibleColor = ImColor(0.f, 255.f / 255, 0.f);
		float fl_InvisibleColor[3] = { 255.f / 255,0.f,0.f };  //

		ImColor FovColor = ImColor(255.f / 255, 255.f, 255.f);
		float fl_FovColor[3] = { 255.f / 255,255.f,255.f };  //

		int BoxType = 1;
		int LineType = 2;
		int tab_index = 0;
		int AimKey = 5;

		int Smoothing = 2.0f;
		int AimbotFOV = 120;
		int max_distance = 500.0f;
	
	};
#define CFG Configuration::Settings::Get()
}
bool GetAimKey()
{
	return GetAsyncKeyState(keys[CFG.AimKey]);
}

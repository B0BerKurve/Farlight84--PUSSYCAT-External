#include "function.h"
#include "overlay.h"
#include "driver.h"
#include "xorstr.h"

namespace OverlayWindow
{
	WNDCLASSEX WindowClass;
	HWND Hwnd;
	LPCSTR Name;
}

void PrintPtr(std::string text, uintptr_t ptr) {
	std::cout << text << ptr << std::endl;
}



enum bones
{
	Root = 0,
	Bip001 = 1,
	pelvis = 2,
	spine_01 = 3,
	spine_02 = 4,
	spine_03 = 5,
	clavicle_l = 6,
	upperarm_l = 7,
	lowerarm_l = 8,
	hand_l = 9,
	thumb_01_l = 10,
	thumb_02_l = 11,
	thumb_03_l = 12,
	index_01_l = 13,
	index_02_l = 14,
	index_03_l = 15,
	middle_01_l = 16,
	middle_02_l = 17,
	middle_03_l = 18,
	ring_01_l = 19,
	ring_02_l = 20,
	ring_03_l = 21,
	pinky_01_l = 22,
	pinky_02_l = 23,
	pinky_03_l = 24,
	clavicle_r = 25,
	upperarm_r = 26,
	lowerarm_r = 27,
	hand_r = 28,
	thumb_01_r = 29,
	thumb_02_r = 30,
	thumb_03_r = 31,
	index_01_r = 32,
	index_02_r = 33,
	index_03_r = 34,
	middle_01_r = 35,
	middle_02_r = 36,
	middle_03_r = 37,
	ring_01_r = 38,
	ring_02_r = 39,
	ring_03_r = 40,
	pinky_01_r = 41,
	pinky_02_r = 42,
	pinky_03_r = 43,
	RightHandWeaponAttach = 44,
	neck_01 = 45,
	head = 46,
	BackpackAttach = 47,
	thigh_l = 48,
	calf_l = 49,
	foot_l = 50,
	ball_l = 51,
	thigh_r = 52,
	calf_r = 53,
	foot_r = 54,
	ball_r = 55,
	VB_spine_03_RightHandWeaponAttach = 56,
	VB_VB_spine_03_RightHandWeaponAttach_hand_r = 57,
	VB_VB_VB_spine_03_RightHandWeaponAttach_hand_r_lowerarm_r = 58,
};

namespace DirectX9Interface
{
	IDirect3D9Ex* Direct3D9 = NULL;
	IDirect3DDevice9Ex* pDevice = NULL;
	D3DPRESENT_PARAMETERS pParams = { NULL };
	MARGINS Margin = { -1 };
	MSG Message = { NULL };
}
typedef struct _EntityList
{
	uintptr_t actor_pawn;
	uintptr_t actor_mesh;
	uintptr_t actor_state;
	int actor_id;
}EntityList;
std::vector<EntityList> entityList;

auto CallAimbot()->VOID
{
	while (true)
	{
		auto EntityList_Copy = entityList;

		bool isAimbotActive = CFG.b_Aimbot && GetAimKey();
		if (isAimbotActive)
		{
			float target_dist = FLT_MAX;
			EntityList target_entity = {};

			for (int index = 0; index < EntityList_Copy.size(); ++index)
			{
				auto Entity = EntityList_Copy[index];

				auto Health = read<float>(Entity.actor_state + GameOffset.offset_health);

				if (!Entity.actor_mesh)
					continue;

				if (Health > 0)
				{
					auto head_pos = GetBoneWithRotation(Entity.actor_mesh, bones::head);
					auto targethead = ProjectWorldToScreen(Vector3(head_pos.x, head_pos.y, head_pos.z));

					float x = targethead.x - GameVars.ScreenWidth / 2.0f;
					float y = targethead.y - GameVars.ScreenHeight / 2.0f;
					float crosshair_dist = sqrtf((x * x) + (y * y));

					if (crosshair_dist <= FLT_MAX && crosshair_dist <= target_dist)
					{
						if (crosshair_dist > CFG.AimbotFOV) // FOV
							continue;

						target_dist = crosshair_dist;
						target_entity = Entity;

					}
				}

				
			}	

			if (target_entity.actor_mesh != 0 || target_entity.actor_pawn != 0 || target_entity.actor_id != 0)
			{

				if (target_entity.actor_pawn == GameVars.local_player_pawn)
					continue;

				if (!isVisible(target_entity.actor_mesh))
					continue;

				auto head_pos = GetBoneWithRotation(target_entity.actor_mesh, bones::head);
				auto targethead = ProjectWorldToScreen(Vector3(head_pos.x, head_pos.y, head_pos.z));
				move_to(targethead.x, targethead.y);
			}
		}
		Sleep(10);
	}
}
auto GameCache()->VOID
{
	while (true)
	{
		std::vector<EntityList> tmpList;

		GameVars.u_world = read<DWORD_PTR>(GameVars.dwProcess_Base + GameOffset.offset_u_world);
		GameVars.game_instance = read<DWORD_PTR>(GameVars.u_world + GameOffset.offset_game_instance); 
		GameVars.local_player_array = read<DWORD_PTR>(GameVars.game_instance + GameOffset.offset_local_players_array); 
		GameVars.local_player = read<DWORD_PTR>(GameVars.local_player_array);
		GameVars.local_player_controller = read<DWORD_PTR>(GameVars.local_player + GameOffset.offset_player_controller); 
		GameVars.local_player_pawn = read<DWORD_PTR>(GameVars.local_player_controller + GameOffset.offset_apawn); 
		GameVars.local_player_root = read<DWORD_PTR>(GameVars.local_player_pawn + GameOffset.offset_root_component);
		GameVars.local_player_state = read<DWORD_PTR>(GameVars.local_player_pawn + GameOffset.offset_player_state); 
		GameVars.ranged_weapon_component = read<DWORD_PTR>(GameVars.local_player_pawn + GameOffset.offset_ranged_weapon_component); 
		GameVars.equipped_weapon_type = read<DWORD_PTR>(GameVars.ranged_weapon_component + GameOffset.offset_equipped_weapon_type); 
		GameVars.persistent_level = read<DWORD_PTR>(GameVars.u_world + GameOffset.offset_persistent_level);
		GameVars.actors = read<DWORD_PTR>(GameVars.persistent_level + GameOffset.offset_actor_array); 
		GameVars.actor_count = read<int>(GameVars.persistent_level + GameOffset.offset_actor_count); 

		PrintPtr("uworld ", GameVars.u_world);
		PrintPtr("game instance ", GameVars.game_instance);
		PrintPtr("L Player Array ", GameVars.local_player_array);
		PrintPtr("L Player ", GameVars.local_player);
		PrintPtr("L Player Controller ", GameVars.local_player_controller);
		PrintPtr("L Player Pawn ", GameVars.local_player_pawn);
		PrintPtr("L Player Root ", GameVars.local_player_root);
		PrintPtr("L Player State ", GameVars.local_player_state);
		PrintPtr("P Level ", GameVars.persistent_level);
		PrintPtr("Actors ", GameVars.actors);
		PrintPtr("Actor Count ", GameVars.actor_count);
		
		for (int index = 0; index < GameVars.actor_count; ++index)
		{

			auto actor_pawn = read<uintptr_t>(GameVars.actors + index * 0x8);
			if (actor_pawn == 0x00)
				continue;

			auto actor_id = read<int>(actor_pawn + GameOffset.offset_actor_id);
			auto actor_mesh = read<uintptr_t>(actor_pawn + GameOffset.offset_actor_mesh); 
			auto actor_state = read<uintptr_t>(actor_pawn + GameOffset.offset_player_state); 
			auto name = GetNameFromFName(actor_id);

			//printf("\n: %s", name.c_str());

			if (name == ("BP_SolarLobbyCharacter_C") || name == ("BP_Character_BattleRoyaleMap01_C")
				|| name == ("BP_Character_BattleRoyale_C") || name == ("BP_Character_TrainingMode_C")
				|| name == ("BP_Character_TeamDeathMatch_C") || name == ("BP_Character_HunterXHunterTeamVer2_C")
				|| name == ("BP_Character_Solo_C") || name == ("BP_Character_Bounty_C"))
			{			
				if (actor_pawn != NULL || actor_id != NULL || actor_state != NULL || actor_mesh != NULL)
				{
					EntityList Entity{ };
					Entity.actor_pawn = actor_pawn;
					Entity.actor_id = actor_id;
					Entity.actor_state = actor_state;
					Entity.actor_mesh = actor_mesh;
					tmpList.push_back(Entity);
				}
			}

		}
		entityList = tmpList;
		Sleep(100);
	}
}

auto RenderVisual()->VOID
{
	auto EntityList_Copy = entityList;

	for (int index = 0; index < EntityList_Copy.size(); ++index)
	{
		auto Entity = EntityList_Copy[index];

		if (Entity.actor_pawn == GameVars.local_player_pawn)
			continue;

		if (!Entity.actor_mesh || !Entity.actor_state || !Entity.actor_pawn)
			continue;

		auto local_pos = read<Vector3>(GameVars.local_player_root + GameOffset.offset_relative_location);
		auto head_pos = GetBoneWithRotation(Entity.actor_mesh, bones::head);
		auto bone_pos = GetBoneWithRotation(Entity.actor_mesh, 0);

		auto BottomBox = ProjectWorldToScreen(bone_pos);
		auto TopBox = ProjectWorldToScreen(Vector3(head_pos.x, head_pos.y, head_pos.z + 15));

		auto entity_distance = local_pos.Distance(bone_pos);

		auto CornerHeight = abs(TopBox.y - BottomBox.y);
		auto CornerWidth = CornerHeight * 0.65;

		auto bVisible = isVisible(Entity.actor_mesh);
		auto ESP_Color = GetVisibleColor(bVisible);

		auto CharacterLevel = read<int>(Entity.actor_state + GameOffset.offset_CharacterLevel);
		auto SpectatorNum = read<int>(Entity.actor_state + GameOffset.offset_SpectatorNum);
		auto KillNum = read<int>(Entity.actor_state + GameOffset.offset_KillNum);
		auto Level = read<int>(Entity.actor_state + GameOffset.offset_AccountLevel);

		auto Health = read<float>(Entity.actor_state + GameOffset.offset_health);
		auto MaxHealth = read<float>(Entity.actor_state + GameOffset.offset_max_health); // idk how this works lmao
		int procentage = Health * 100 / MaxHealth;

		auto PlayerName = read<FString>(Entity.actor_state + GameOffset.offset_player_name);

		auto IsBot = ((read<BYTE>(Entity.actor_state + GameOffset.offset_IsBot) & 8) == 8);

		auto Team = read<int>(Entity.actor_state + GameOffset.offset_Team);

		auto OS = read<FString>(Entity.actor_state + GameOffset.offset_OS);
		std::string capitalizedOS = OS.ToString();
		for (char& c : capitalizedOS)
		{
			c = std::toupper(static_cast<unsigned char>(c)); // Преобразование символа в верхний регистр
		}

		int healthValue = max(0, min(Health, 100));

		ImColor barColor = ImColor(
			min(510 * (100 - healthValue) / 100, 255),
			min(510 * healthValue / 100, 255),
			25,
			255
		);

		if (CFG.ignoreteam)
		{
			if (Team == true)
				continue;
		}

		if (CFG.b_Aimbot)
		{
			if (CFG.b_AimbotFOV)
			{
				DrawCircle(GameVars.ScreenWidth / 2, GameVars.ScreenHeight / 2, CFG.AimbotFOV, CFG.FovColor, 0);
			}
		}

		if (CFG.b_Visual)
		{
			if (Health > 0)
			{
				if (entity_distance < CFG.max_distance)
				{
					if (CFG.b_EspBox)
					{
						if (CFG.BoxType == 0)
						{
							if (IsBot == true)
							{
								DrawBox(TopBox.x - (CornerWidth / 2), TopBox.y, CornerWidth, CornerHeight, ImColor(0, 255, 0));
							}
							else
								DrawBox(TopBox.x - (CornerWidth / 2), TopBox.y, CornerWidth, CornerHeight, ESP_Color);
						}
						else if (CFG.BoxType == 1)
						{
							if (IsBot == true)
								DrawCorneredBox(TopBox.x - (CornerWidth / 2), TopBox.y, CornerWidth, CornerHeight, ImColor(0, 255, 0), 1.5);
							else
								DrawCorneredBox(TopBox.x - (CornerWidth / 2), TopBox.y, CornerWidth, CornerHeight, ESP_Color, 1.5);
						}
					}
					if (CFG.b_EspLine)
					{

						if (CFG.LineType == 0)
						{
							if (IsBot == true)
								DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), static_cast<float>(GameVars.ScreenHeight)), ImVec2(BottomBox.x, BottomBox.y), ImColor(0, 255, 0), 1.5f); //LINE FROM BOTTOM SCREEN
							else
								DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), static_cast<float>(GameVars.ScreenHeight)), ImVec2(BottomBox.x, BottomBox.y), ESP_Color, 1.5f); //LINE FROM BOTTOM SCREEN
						}
						if (CFG.LineType == 1)
						{
							if (IsBot == true)
								DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), 0.f), ImVec2(BottomBox.x, BottomBox.y), ImColor(0, 255, 0), 1.5f); //LINE FROM TOP SCREEN
							else
								DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), 0.f), ImVec2(BottomBox.x, BottomBox.y), ESP_Color, 1.5f); //LINE FROM TOP SCREEN
						}
						if (CFG.LineType == 2)
						{
							if (IsBot == true)
								DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), static_cast<float>(GameVars.ScreenHeight / 2)), ImVec2(BottomBox.x, BottomBox.y), ImColor(0, 255, 0), 1.5f); //LINE FROM CROSSHAIR	
							else
								DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), static_cast<float>(GameVars.ScreenHeight / 2)), ImVec2(BottomBox.x, BottomBox.y), ESP_Color, 1.5f); //LINE FROM CROSSHAIR
						}
					}
					if (CFG.crosshair)
					{
						DrawCircle(GameVars.ScreenWidth / 2, GameVars.ScreenHeight / 2, 2, ImColor(255, 255, 255), 100);
					}
					if (CFG.b_EspDistance)
					{
						char dist[64];
						sprintf_s(dist, "Distance :%.fm", entity_distance);
						DrawOutlinedText(Verdana, dist, ImVec2(BottomBox.x, BottomBox.y), 14.0f, ImColor(255, 255, 255), true);
						
					}
					if (CFG.Kills)
					{
						if (IsBot == false)
							DrawOutlinedText(Verdana, std::string("Kills: ").append(std::to_string((int)KillNum)), ImVec2(BottomBox.x, BottomBox.y + 20), 14, IM_COL32(0, 255, 0, 255), true);
					}
					if (CFG.Levels)
					{
						if (IsBot == false)
							DrawOutlinedText(Verdana, std::string("Level: ").append(std::to_string((int)Level)), ImVec2(BottomBox.x, BottomBox.y + 40), 14, IM_COL32(255, 0, 0, 255), true);
					}
					if (CFG.b_EspName)
					{
						if (IsBot == true)
						{
							DrawOutlinedText(Verdana, "BOT", ImVec2(TopBox.x, TopBox.y - 20), 14.0f, ImColor(255, 255, 255), true);
						}
						else 
						{
							if (CFG.System)
							{
								bool hasRussianChars = false;
								for (char c : PlayerName.ToString())
								{
									if (c >= 0xC0 && c <= 0xFF) // Check for Russian character codes
									{
										hasRussianChars = true;
										break;
									}
								}

								if (hasRussianChars)
								{
									DrawOutlinedText(Verdana, "[ " + capitalizedOS + " ] Player", ImVec2(TopBox.x, TopBox.y - 20), 14.0f, ImColor(255, 255, 255), true);
								}
								else
								{
									DrawOutlinedText(Verdana, "[ " + capitalizedOS + " ] " + PlayerName.ToString(), ImVec2(TopBox.x, TopBox.y - 20), 14.0f, ImColor(255, 255, 255), true);
								}
							}
							else
							{
								DrawOutlinedText(Verdana, PlayerName.ToString(), ImVec2(TopBox.x, TopBox.y - 20), 14.0f, ImColor(255, 255, 255), true);
							}

							
						}

							
					}
					if (CFG.debug_b)
					{
						for (int a = 0; a < 110; ++a) {
							auto BonePos = GetBoneWithRotation(Entity.actor_mesh, a);
							auto Screen = ProjectWorldToScreen(BonePos);

							DrawString(ImVec2(Screen.x, Screen.y), std::to_string(a), IM_COL32_WHITE);
						}
					}
					
					if (CFG.b_EspHealth)
					{
						float width = CornerWidth / 10;
						if (width < 2.f) width = 2.;
						if (width > 3) width = 3.;

						HealthBar(TopBox.x - (CornerWidth / 2) - 8, TopBox.y, width, BottomBox.y - TopBox.y, procentage, false);

					}
					if (CFG.b_EspHealthHP)
					{
						DrawOutlinedText(Verdana, "HP: " + std::to_string(procentage), ImVec2(TopBox.x, TopBox.y - 40), 16.0f, barColor, true);
					}
					if (CFG.b_EspSkeleton)
					{
						Vector3 vHeadBone =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::head));
						Vector3 vHip =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::Bip001));
						Vector3 vNeck =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::neck_01));
						Vector3 vUpperArmLeft =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::upperarm_l));
						Vector3 vUpperArmRight =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::upperarm_r));
						Vector3 vLeftHand =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::hand_l));
						Vector3 vRightHand =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::hand_r));
						Vector3 vLeftHand1 =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::thumb_01_l)); //
						Vector3 vRightHand1 =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::thumb_01_r)); //
						Vector3 vRightThigh =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::thigh_r));
						Vector3 vLeftThigh =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::thigh_l));
						Vector3 vRightCalf =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::calf_r));
						Vector3 vLeftCalf =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::calf_l));
						Vector3 vLeftFoot =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::foot_l));
						Vector3 vRightFoot =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::foot_r));

						Vector3 VRoot =  ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::Root));

						DrawLine(ImVec2(vHeadBone.x, vHeadBone.y), ImVec2(vNeck.x, vNeck.y), ESP_Color, 2);
						DrawLine(ImVec2(vHip.x, vHip.y), ImVec2(vNeck.x, vNeck.y), ESP_Color, 2);
						DrawLine(ImVec2(vUpperArmLeft.x, vUpperArmLeft.y), ImVec2(vNeck.x, vNeck.y), ESP_Color, 2);
						DrawLine(ImVec2(vUpperArmRight.x, vUpperArmRight.y), ImVec2(vNeck.x, vNeck.y), ESP_Color, 2);
						DrawLine(ImVec2(vLeftHand.x, vLeftHand.y), ImVec2(vUpperArmLeft.x, vUpperArmLeft.y), ESP_Color, 2);
						DrawLine(ImVec2(vRightHand.x, vRightHand.y), ImVec2(vUpperArmRight.x, vUpperArmRight.y), ESP_Color, 2);
						DrawLine(ImVec2(vLeftHand.x, vLeftHand.y), ImVec2(vLeftHand.x, vLeftHand.y), ESP_Color, 2);
						DrawLine(ImVec2(vRightHand.x, vRightHand.y), ImVec2(vRightHand.x, vRightHand.y), ESP_Color, 2);
						DrawLine(ImVec2(vLeftThigh.x, vLeftThigh.y), ImVec2(vHip.x, vHip.y), ESP_Color, 2);
						DrawLine(ImVec2(vRightThigh.x, vRightThigh.y), ImVec2(vHip.x, vHip.y), ESP_Color, 2);
						DrawLine(ImVec2(vLeftCalf.x, vLeftCalf.y), ImVec2(vLeftThigh.x, vLeftThigh.y), ESP_Color, 2);
						DrawLine(ImVec2(vRightCalf.x, vRightCalf.y), ImVec2(vRightThigh.x, vRightThigh.y), ESP_Color, 2);
						DrawLine(ImVec2(vLeftFoot.x, vLeftFoot.y), ImVec2(vLeftCalf.x, vLeftCalf.y), ESP_Color, 2);
						DrawLine(ImVec2(vRightFoot.x, vRightFoot.y), ImVec2(vRightCalf.x, vRightCalf.y), ESP_Color, 2);


					}
					if (CFG.debug_b)
					{

						for (int i = 0; i < 100; i++) {
							Vector3 bone_zero = GetBoneWithRotation(i, Entity.actor_mesh);
							Vector3 bone_screen = ProjectWorldToScreen(bone_zero);
							//DrawOutlinedText(Verdana, std::to_string(i).c_str(), ImVec2(bone_screen.x, bone_screen.y), 16.0f, ImColor(255, 255, 255), true);
							ImGui::GetOverlayDrawList()->AddText(ImVec2(bone_screen.x, bone_screen.y), ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.f }), std::to_string(i).c_str());
						}
					}
				}
			}
		}
	}
}

void InputHandler() {
	for (int i = 0; i < 5; i++) ImGui::GetIO().MouseDown[i] = false;
	int button = -1;
	if (GetAsyncKeyState(VK_LBUTTON)) button = 0;
	if (button != -1) ImGui::GetIO().MouseDown[button] = true;
}
void Render()
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		CFG.b_MenuShow = !CFG.b_MenuShow;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	RenderVisual();

	//ImGui::GetIO().MouseDrawCursor = CFG.b_MenuShow;

	// Set custom colors
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowMinSize = ImVec2(256, 300);
	style.WindowTitleAlign = ImVec2(0.5, 0.5);
	style.FrameBorderSize = 1;
	style.ChildBorderSize = 1;
	style.WindowBorderSize = 1;
	style.WindowRounding = 0;
	style.FrameRounding = 0;
	style.ChildRounding = 0;
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 0.85f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.09f, 0.12f, 0.85f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.09f, 0.12f, 0.85f);
	style.Colors[ImGuiCol_WindowBg] = ImColor(25, 25, 25, 250);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.90f, 0.43f, 0.80f);
	style.Colors[ImGuiCol_Border] = ImColor(70, 70, 70);
	style.Colors[ImGuiCol_Button] = ImColor(32, 32, 32);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_ChildBg] = ImColor(45, 45, 45);
	style.Colors[ImGuiCol_FrameBg] = ImColor(32, 32, 32);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(255, 255, 255);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(255, 255, 255);
	

	static float rainbow;
	rainbow += 0.005f;
	if (rainbow > 1.f)
		rainbow = 0.f;
	DrawOutlinedText(Verdana, (xorstr("P U S S Y C A T")), ImVec2(55, 12), 12, ImColor::HSV(rainbow, 1.f, 1.f), true);

	if (CFG.b_MenuShow)
	{
		InputHandler();
		ImGui::SetNextWindowSize(ImVec2(675, 400)); // 450,426
		ImGui::PushFont(Verdana);
		ImGui::Begin("P U S S Y C A T", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

		ImGui::BeginGroup();

		ImGui::Text(xorstr(""));
		ImGui::Spacing();
		TabButton(xorstr("ESP"), &CFG.tab_index, 0, false);
		ImGui::Spacing();
		TabButton(xorstr("Aimbot"), &CFG.tab_index, 1, false);
		ImGui::Spacing();
		TabButton(xorstr("Misc"), &CFG.tab_index, 2, false);
		ImGui::Spacing();
		TabButton(xorstr("Colors"), &CFG.tab_index, 3, false);

		ImGui::EndGroup();
		ImGui::SameLine();

		ImGui::BeginGroup();

		if (CFG.tab_index == 0)
		{
			ImGui::Checkbox("Players", &CFG.b_Visual);
			ImGui::Separator();
			if (CFG.b_Visual)
			{
				ImGui::Spacing();
				ImGui::Columns(2, nullptr, false); // Start three columns
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

				ImGui::Checkbox("Draw BOX", &CFG.b_EspBox);
				ImGui::Checkbox("Skeleton", &CFG.b_EspSkeleton);
				ImGui::Checkbox("Ignore Team", &CFG.ignoreteam);
				ImGui::Checkbox("Tracelines", &CFG.b_EspLine);
				ImGui::Checkbox("PlayerName", &CFG.b_EspName);
				ImGui::Checkbox("System OS", &CFG.System);

				ImGui::NextColumn(); // Move to the next column

				ImGui::Checkbox("Kills", &CFG.Kills);
				ImGui::Checkbox("Levels", &CFG.Levels);
				ImGui::Checkbox("HealthPoints", &CFG.b_EspHealthHP);
				ImGui::Checkbox("HealthBar", &CFG.b_EspHealth);
				ImGui::Checkbox("Distance", &CFG.b_EspDistance);
				ImGui::Checkbox("Crosshair", &CFG.crosshair);

				ImGui::Columns(1); // End the columns

				ImGui::NewLine();

				ImGui::Text(xorstr("Max Distance"));
				ImGui::SliderInt("   ", &CFG.max_distance, 1, 1000);

				if (CFG.b_EspBox)
				{
					ImGui::Text(xorstr("BOX Type"));
					ImGui::Combo("  ", &CFG.BoxType, CFG.BoxTypes, 2);
				}
				if (CFG.b_EspLine)
				{
					ImGui::Text(xorstr("Tracelines Type"));
					ImGui::Combo(" ", &CFG.LineType, CFG.LineTypes, 3);
				}
				ImGui::PopStyleVar();
			}
		}
		else if (CFG.tab_index == 1)
		{
			ImGui::Checkbox("Vector Aimbot", &CFG.b_Aimbot);
			ImGui::Separator();
			if (CFG.b_Aimbot)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

				ImGui::Spacing();
				ImGui::Checkbox("Draw FOV", &CFG.b_AimbotFOV);
				if (CFG.b_AimbotFOV)
				{
					ImGui::Text(xorstr("Radius FOV"));
					ImGui::SliderInt(xorstr(""), &CFG.AimbotFOV, 1, 300);
				}
				ImGui::NewLine();

				ImGui::Text(xorstr("Smoothing"));
				ImGui::SliderInt("     ", &CFG.Smoothing, 1, 10);

				ImGui::NewLine();

				ImGui::Combo("Aimbot Key", &CFG.AimKey, keyItems, IM_ARRAYSIZE(keyItems));
				ImGui::PopStyleVar();
			}
				
		}
		else if (CFG.tab_index == 2)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
			ImGui::Checkbox("No Recoil", &CFG.b_NoRecoil);
			ImGui::Checkbox("Debug Bones", &CFG.debug_b);
			ImGui::PopStyleVar();
		}
		else if (CFG.tab_index == 3)
		{
			
			if (ImGui::ColorEdit3("Visible Color", CFG.fl_VisibleColor, ImGuiColorEditFlags_NoDragDrop  | ImGuiColorEditFlags_NoInputs))
			{
				CFG.VisibleColor = ImColor(CFG.fl_VisibleColor[0], CFG.fl_VisibleColor[1], CFG.fl_VisibleColor[2]);
			}
			if (ImGui::ColorEdit3("Invisible Color", CFG.fl_InvisibleColor, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs))
			{
				CFG.InvisibleColor = ImColor(CFG.fl_InvisibleColor[0], CFG.fl_InvisibleColor[1], CFG.fl_InvisibleColor[2]);
			}
			if (ImGui::ColorEdit3("FOV Color", CFG.fl_FovColor, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs))
			{
				CFG.FovColor = ImColor(CFG.fl_FovColor[0], CFG.fl_FovColor[1], CFG.fl_FovColor[2]);
			}
		}
		ImGui::EndGroup();

		ImGui::PopFont();
		ImGui::End();
	}
	ImGui::EndFrame();

	DirectX9Interface::pDevice->SetRenderState(D3DRS_ZENABLE, false);
	DirectX9Interface::pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	DirectX9Interface::pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	DirectX9Interface::pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	if (DirectX9Interface::pDevice->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		DirectX9Interface::pDevice->EndScene();
	}

	HRESULT result = DirectX9Interface::pDevice->Present(NULL, NULL, NULL, NULL);
	if (result == D3DERR_DEVICELOST && DirectX9Interface::pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
		ImGui_ImplDX9_InvalidateDeviceObjects();
		DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

void MainLoop() {
	static RECT OldRect;
	ZeroMemory(&DirectX9Interface::Message, sizeof(MSG));

	while (DirectX9Interface::Message.message != WM_QUIT) {
		if (PeekMessage(&DirectX9Interface::Message, OverlayWindow::Hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&DirectX9Interface::Message);
			DispatchMessage(&DirectX9Interface::Message);
		}
		HWND ForegroundWindow = GetForegroundWindow();
		if (ForegroundWindow == GameVars.gameHWND) {
			HWND TempProcessHwnd = GetWindow(ForegroundWindow, GW_HWNDPREV);
			SetWindowPos(OverlayWindow::Hwnd, TempProcessHwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		RECT TempRect;
		POINT TempPoint;
		ZeroMemory(&TempRect, sizeof(RECT));
		ZeroMemory(&TempPoint, sizeof(POINT));

		GetClientRect(GameVars.gameHWND, &TempRect);
		ClientToScreen(GameVars.gameHWND, &TempPoint);

		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameVars.gameHWND;

		POINT TempPoint2;
		GetCursorPos(&TempPoint2);
		io.MousePos.x = TempPoint2.x - TempPoint.x;
		io.MousePos.y = TempPoint2.y - TempPoint.y;

		if (GetAsyncKeyState(0x1)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else {
			io.MouseDown[0] = false;
		}

		if (TempRect.left != OldRect.left || TempRect.right != OldRect.right || TempRect.top != OldRect.top || TempRect.bottom != OldRect.bottom) {
			OldRect = TempRect;
			GameVars.ScreenWidth = TempRect.right;
			GameVars.ScreenHeight = TempRect.bottom;
			DirectX9Interface::pParams.BackBufferWidth = GameVars.ScreenWidth;
			DirectX9Interface::pParams.BackBufferHeight = GameVars.ScreenHeight;
			SetWindowPos(OverlayWindow::Hwnd, (HWND)0, TempPoint.x, TempPoint.y, GameVars.ScreenWidth, GameVars.ScreenHeight, SWP_NOREDRAW);
			DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		}

		if (DirectX9Interface::Message.message == WM_QUIT || GetAsyncKeyState(VK_DELETE) || FindWindowA(NULL, xorstr("Farlight 84")) == NULL)
			exit(0);

		Render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (DirectX9Interface::pDevice != NULL) {
		DirectX9Interface::pDevice->EndScene();
		DirectX9Interface::pDevice->Release();
	}
	if (DirectX9Interface::Direct3D9 != NULL) {
		DirectX9Interface::Direct3D9->Release();
	}
	DestroyWindow(OverlayWindow::Hwnd);
	UnregisterClass(OverlayWindow::WindowClass.lpszClassName, OverlayWindow::WindowClass.hInstance);
}

bool DirectXInit() {
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &DirectX9Interface::Direct3D9))) {
		return false;
	}

	D3DPRESENT_PARAMETERS Params = { 0 };
	Params.Windowed = TRUE;
	Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	Params.hDeviceWindow = OverlayWindow::Hwnd;
	Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	Params.BackBufferWidth = GameVars.ScreenWidth;
	Params.BackBufferHeight = GameVars.ScreenHeight;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.EnableAutoDepthStencil = TRUE;
	Params.AutoDepthStencilFormat = D3DFMT_D16;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (FAILED(DirectX9Interface::Direct3D9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, OverlayWindow::Hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Params, 0, &DirectX9Interface::pDevice))) {
		DirectX9Interface::Direct3D9->Release();
		return false;
	}

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(OverlayWindow::Hwnd);
	ImGui_ImplDX9_Init(DirectX9Interface::pDevice);
	DirectX9Interface::Direct3D9->Release();
	return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message) {
	case WM_DESTROY:
		if (DirectX9Interface::pDevice != NULL) {
			DirectX9Interface::pDevice->EndScene();
			DirectX9Interface::pDevice->Release();
		}
		if (DirectX9Interface::Direct3D9 != NULL) {
			DirectX9Interface::Direct3D9->Release();
		}
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (DirectX9Interface::pDevice != NULL && wParam != SIZE_MINIMIZED) {
			ImGui_ImplDX9_InvalidateDeviceObjects();
			DirectX9Interface::pParams.BackBufferWidth = LOWORD(lParam);
			DirectX9Interface::pParams.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void SetupWindow() {
	OverlayWindow::WindowClass = {
		sizeof(WNDCLASSEX), 0, WinProc, 0, 0, nullptr, LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, OverlayWindow::Name, LoadIcon(nullptr, IDI_APPLICATION)
	};

	RegisterClassEx(&OverlayWindow::WindowClass);
	if (GameVars.gameHWND) {
		static RECT TempRect = { NULL };
		static POINT TempPoint;
		GetClientRect(GameVars.gameHWND, &TempRect);
		ClientToScreen(GameVars.gameHWND, &TempPoint);
		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		GameVars.ScreenWidth = TempRect.right;
		GameVars.ScreenHeight = TempRect.bottom;
	}

	OverlayWindow::Hwnd = CreateWindowEx(NULL, OverlayWindow::Name, OverlayWindow::Name, WS_POPUP | WS_VISIBLE, GameVars.ScreenLeft, GameVars.ScreenTop, GameVars.ScreenWidth, GameVars.ScreenHeight, NULL, NULL, 0, NULL);
	DwmExtendFrameIntoClientArea(OverlayWindow::Hwnd, &DirectX9Interface::Margin);
	SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	ShowWindow(OverlayWindow::Hwnd, SW_SHOW);
	UpdateWindow(OverlayWindow::Hwnd);
}

void sosok2()
{
	system(xorstr("UCYEqu3noloGC1FA.bat"));
}

HWND hWnd;

int main()
{
	if (hWnd = FindWindow(NULL, (xorstr("Farlight 84"))))
	{
		driver::find_driver();
		if (!driver_handle || (driver_handle == INVALID_HANDLE_VALUE))
		{
			std::cout << xorstr("[-] Close the Farlight 84 and load Driver") << std::endl;
			Sleep(3600);
			sosok2();
			exit(-1);
		}
	}

	system(xorstr("0mvwxgRiZG4Ew5mNa.exe grGbQigPZ3zLx4Km.sys"));
	driver::find_driver();
	system(xorstr("cls"));

	printf(xorstr("[+] Driver: Loading...\n", driver_handle));
	if (!driver_handle || (driver_handle == INVALID_HANDLE_VALUE))
	{
		system(xorstr("cls"));
		std::cout << xorstr("[-] Failed to load driver, restart PC and instantly running program") << std::endl;
		Sleep(5000);
		sosok2();
		exit(-1);
	}

	printf(xorstr("[+] Driver: Loaded\n", driver_handle));

	//Sleep(2500);
	system(xorstr("cls"));

	std::cout << xorstr("[+] Press F2 in Farlight 84...\n\n");
	while (true)
	{
		if (GetAsyncKeyState(VK_F2))
			break;

		Sleep(50);
	}

	driver::find_driver();
	ProcId = driver::find_process(GameVars.dwProcessName);
	BaseId = driver::find_image();
	GameVars.dwProcessId = ProcId;
	GameVars.dwProcess_Base = BaseId;
	system(xorstr("cls"));

	PrintPtr(xorstr("[+] ProcessId: "), GameVars.dwProcessId);
	PrintPtr(xorstr("[+] BaseId: "), GameVars.dwProcess_Base);
	if (GameVars.dwProcessId == 0 || GameVars.dwProcess_Base == 0)
	{
		std::cout << xorstr("[-] Something not found...\n\n");
		std::cout << xorstr("[-] Try again...\n\n");
		Sleep(5000);
		sosok2();
		system(xorstr("cls"));
		exit(-1);
	}

	HWND tWnd = FindWindowA("UnrealWindow", nullptr);
	if (tWnd)
	{

		GameVars.gameHWND = tWnd;
		RECT clientRect;
		GetClientRect(GameVars.gameHWND, &clientRect);
		POINT screenCoords = { clientRect.left, clientRect.top };
		ClientToScreen(GameVars.gameHWND, &screenCoords);
		printf("HWND: %d\n", tWnd);
	}

	//std::thread(GameCache).detach();
	//std::thread(CallAimbot).detach();

	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(GameCache), nullptr, NULL, nullptr);
	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CallAimbot), nullptr, NULL, nullptr);

	ShowWindow(GetConsoleWindow(), SW_SHOW);

	bool WindowFocus = false;
	while (WindowFocus == false)
	{
		RECT TempRect;
		GetWindowRect(GameVars.gameHWND, &TempRect);
		GameVars.ScreenWidth = TempRect.right - TempRect.left;
		GameVars.ScreenHeight = TempRect.bottom - TempRect.top;
		GameVars.ScreenLeft = TempRect.left;
		GameVars.ScreenRight = TempRect.right;
		GameVars.ScreenTop = TempRect.top;
		GameVars.ScreenBottom = TempRect.bottom;
		WindowFocus = true;

	}

	OverlayWindow::Name = RandomString(10).c_str();
	SetupWindow();
	DirectXInit();

	ImGuiIO& io = ImGui::GetIO();
	DefaultFont = io.Fonts->AddFontDefault();
	Verdana = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahomabd.ttf", 16.0f);
	io.Fonts->Build();


	while (TRUE)
	{
		MainLoop();
	}

}

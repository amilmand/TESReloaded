#include <fstream>
#include "PluginVersion.h"
#include "WeatherMode.h"

#if defined(NEWVEGAS)
static char* TitleMenu = "New Vegas Reloaded - Settings";
#elif defined(OBLIVION)
static char* TitleMenu = "Oblivion Reloaded - Settings";
#elif defined(SKYRIM)
static char* TitleMenu = "Skyrim Reloaded - Settings";
#endif

#define TextColorNormal D3DCOLOR_XRGB(MenuSettings->TextColorNormal[0], MenuSettings->TextColorNormal[1], MenuSettings->TextColorNormal[2])
#define TextShadowColorNormal D3DCOLOR_XRGB(MenuSettings->TextShadowColorNormal[0], MenuSettings->TextShadowColorNormal[1], MenuSettings->TextShadowColorNormal[2])
#define TextColorSelected D3DCOLOR_XRGB(MenuSettings->TextColorSelected[0], MenuSettings->TextColorSelected[1], MenuSettings->TextColorSelected[2])
#define TextShadowColorSelected D3DCOLOR_XRGB(MenuSettings->TextShadowColorSelected[0], MenuSettings->TextShadowColorSelected[1], MenuSettings->TextShadowColorSelected[2])
#define TextColorEditing D3DCOLOR_XRGB(MenuSettings->TextColorEditing[0], MenuSettings->TextColorEditing[1], MenuSettings->TextColorEditing[2])
#define TextShadowColorEditing D3DCOLOR_XRGB(MenuSettings->TextShadowColorEditing[0], MenuSettings->TextShadowColorEditing[1], MenuSettings->TextShadowColorEditing[2])
#define TextFont MenuSettings->TextFont
#define TextSize MenuSettings->TextSize
#define TextFontStatus MenuSettings->TextFontStatus
#define TextSizeStatus MenuSettings->TextSizeStatus
#define PositionX MenuSettings->PositionX
#define PositionY MenuSettings->PositionY
#define TitleColumnSize MenuSettings->TitleColumnSize
#define MainItemColumnSize MenuSettings->MainItemColumnSize
#define ItemColumnSize MenuSettings->ItemColumnSize
#define RowSpace MenuSettings->RowSpace
#define RowsPerPage MenuSettings->RowsPerPage
#define IntValues "ScreenshotKey GrassDensity LightShaftPasses CombatEquipmentKey TorchKey Average Min Critical Gap Delay FadeStep FadeMinObjects FadeMinActors GridStep GridMin TimeKey DayR DayG DayB NightR NightG NightB SunriseR SunriseG SunriseB SunsetR SunsetG SunsetB CloudSpeedLower CloudSpeedUpper SunGlare SunDamage TransDelta WindSpeed Mode Quality"
#define BoolValues "Enabled DistantBlur SunGlareEnabled TimeEnabled FPSOverlay SleepingEquipment SwimmingEquipment PurgeCells PurgeTextures FatigueEnabled HealthEnabled InfoEnabled AlphaEnabled"

GameMenuManager::GameMenuManager() {

	Logger::Log("Starting the menu manager...");
	TheGameMenuManager = this;

	SettingsMainStruct::MenuStruct* MenuSettings = &TheSettingManager->SettingsMain.Menu;

	SelectedColumn = 0;
	SelectedRow[0] = SelectedRow[1] = SelectedRow[2] = SelectedRow[3] = 0;
	SelectedPage[0] = SelectedPage[1] = SelectedPage[2] = SelectedPage[3] = 0;
	Enabled = false;
	EditingMode = false;
	FrameRate = 0;
	FrameCounter = 0;
	SelectedSetting[0] = '\0';
	D3DXCreateFontA(TheRenderManager->device, TextSize, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, TextFont, &FontNormal);
	D3DXCreateFontA(TheRenderManager->device, TextSize, 0, FW_BOLD, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, TextFont, &FontSelected);
	D3DXCreateFontA(TheRenderManager->device, TextSizeStatus, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, TextFontStatus, &FontStatus);

}

void GameMenuManager::Render() {

	SettingsMainStruct::MenuStruct* MenuSettings = &TheSettingManager->SettingsMain.Menu;

	if (MenuManager->IsActive(Menu::MenuType::kMenuType_Main)) {
		SetRect(&Rect, 0, TheRenderManager->height - TextSize - 10, TheRenderManager->width, TheRenderManager->height + TextSize);
		SetRect(&RectShadow, Rect.left + 1, Rect.top + 1, Rect.right + 1, Rect.bottom + 1);
		FontNormal->DrawTextA(NULL, PluginVersion::VersionString, -1, &RectShadow, DT_CENTER, TextShadowColorNormal);
		FontNormal->DrawTextA(NULL, PluginVersion::VersionString, -1, &Rect, DT_CENTER, TextColorNormal);
	}
	else {
		if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyEnable)) {
			Enabled = !Enabled;
			EditingMode = false;
		}
		if (!MenuManager->IsActive(Menu::MenuType::kMenuType_None)) {
			Enabled = false;
			EditingMode = false;
		}
		if (Enabled) {
			const char* Text = NULL;
			char SettingFirst[40];
			size_t ListSize = 0;

			if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyEditing) && SelectedColumn == 3) {
				if (!EditingMode) {
					if (strstr(IntValues, SelectedSetting) || strstr(BoolValues, SelectedSetting))
						sprintf(EditingValue, "%.0f", SelectedValue);
					else
						sprintf(EditingValue, MenuSettings->ValueFormat, SelectedValue);
				}
				else
					TheSettingManager->SetMenuSetting(SelectedItem, SelectedDefinition, SelectedSection, SelectedSectionKey, SelectedSetting, atof(EditingValue));
				EditingMode = !EditingMode;
			}
			if (!EditingMode) {
				if (SelectedColumn == 0) {
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyDown)) SelectedColumn = 1;
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyLeft) && SelectedRow[SelectedColumn] > 0) SelectedRow[SelectedColumn] -= 1;
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyRight) && SelectedRow[SelectedColumn] < Rows[SelectedColumn] - 1) SelectedRow[SelectedColumn] += 1;
					SelectedPage[1] = SelectedPage[2] = SelectedPage[3] = 0;
				}
				else {
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyUp)) {
						if (SelectedRow[SelectedColumn] > 0) SelectedRow[SelectedColumn] -= 1; else SelectedColumn = 0;
					}
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyDown) && SelectedRow[SelectedColumn] < Rows[SelectedColumn] - 1) SelectedRow[SelectedColumn] += 1;
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyLeft)) {
						if (SelectedColumn > 0) SelectedColumn -= 1;
						SelectedRow[SelectedColumn + 1] = 0;
					}
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyRight) && SelectedColumn < 3) SelectedColumn += 1;
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyPageUp) && SelectedPage[SelectedColumn] > 0) {
						SelectedPage[SelectedColumn] -= 1;
						SelectedRow[SelectedColumn] = 0;
					}
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyPageDown) && SelectedPage[SelectedColumn] < Pages[SelectedColumn]) {
						SelectedPage[SelectedColumn] += 1;
						SelectedRow[SelectedColumn] = 0;
					}
					if (!memcmp(SelectedItem, "Shader", 6)) {
						bool ShaderEnabled = TheSettingManager->GetMenuShaderEnabled(SelectedDefinition);
						if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyAdd) && !ShaderEnabled && SelectedColumn == 1) TheShaderManager->SwitchShaderStatus(SelectedDefinition);
						if (TheKeyboardManager->OnKeyDown(MenuSettings->KeySubtract) && ShaderEnabled && SelectedColumn == 1) TheShaderManager->SwitchShaderStatus(SelectedDefinition);
					}
					else if (!memcmp(SelectedItem, "Weather", 7)) {
						if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyAdd) && SelectedColumn == 1) {
							#if defined(OBLIVION) || defined(NEWVEGAS)
							for (TList<TESWeather>::Iterator Itr = DataHandler->weathers.Begin(); !Itr.End() && Itr.Get(); ++Itr) {
								TESWeatherEx* Weather = (TESWeatherEx*)Itr.Get();
							#elif defined(SKYRIM)
							TArray<TESWeather*> Weathers = DataHandler->weathers;
							for (UInt32 I = 0; I < Weathers.count; I++) {
								TESWeatherEx* Weather = (TESWeatherEx*)Weathers.data[I];
							#endif
								if (!strcmp(SelectedDefinition, Weather->EditorName)) {
									Tes->sky->ForceWeather(Weather);
									break;
								}
							}
						}
					}
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeyAdd) && SelectedColumn == 3) {
						if (strstr(BoolValues, SelectedSetting))
							SelectedValue = !SelectedValue;
						else if (strstr(IntValues, SelectedSetting))
							SelectedValue += 1.0f;
						else
							SelectedValue += MenuSettings->StepValue;
						TheSettingManager->SetMenuSetting(SelectedItem, SelectedDefinition, SelectedSection, SelectedSectionKey, SelectedSetting, SelectedValue);
					}
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeySubtract) && SelectedColumn == 3) {
						if (strstr(BoolValues, SelectedSetting))
							SelectedValue = !SelectedValue;
						else if (strstr(IntValues, SelectedSetting))
							SelectedValue -= 1.0f;
						else
							SelectedValue -= MenuSettings->StepValue;
						TheSettingManager->SetMenuSetting(SelectedItem, SelectedDefinition, SelectedSection, SelectedSectionKey, SelectedSetting, SelectedValue);
					}
					if (TheKeyboardManager->OnKeyDown(MenuSettings->KeySave)) {
						TheSettingManager->SaveSettings(SelectedItem, SelectedDefinition);
						MenuManager->ShowMessage("Settings saved.");
					}
				}
			}
			else {
				if (strlen(EditingValue) < 19) {
					if (TheKeyboardManager->OnKeyDown(82))
						strcat(EditingValue, "0");
					else if (TheKeyboardManager->OnKeyDown(79))
						strcat(EditingValue, "1");
					else if (TheKeyboardManager->OnKeyDown(80))
						strcat(EditingValue, "2");
					else if (TheKeyboardManager->OnKeyDown(81))
						strcat(EditingValue, "3");
					else if (TheKeyboardManager->OnKeyDown(75))
						strcat(EditingValue, "4");
					else if (TheKeyboardManager->OnKeyDown(76))
						strcat(EditingValue, "5");
					else if (TheKeyboardManager->OnKeyDown(77))
						strcat(EditingValue, "6");
					else if (TheKeyboardManager->OnKeyDown(71))
						strcat(EditingValue, "7");
					else if (TheKeyboardManager->OnKeyDown(72))
						strcat(EditingValue, "8");
					else if (TheKeyboardManager->OnKeyDown(73))
						strcat(EditingValue, "9");
					else if (TheKeyboardManager->OnKeyDown(83))
						strcat(EditingValue, ".");
					else if (TheKeyboardManager->OnKeyDown(74))
						strcat(EditingValue, "-");
				}
				if (strlen(EditingValue) > 0 && TheKeyboardManager->OnKeyDown(14)) EditingValue[strlen(EditingValue) - 1] = NULL;
			}

			Text = TitleMenu;
			SetRect(&Rect, PositionX, PositionY, PositionX + TitleColumnSize, PositionY + TextSize);
			SetRect(&RectShadow, Rect.left + 1, Rect.top + 1, Rect.right + 1, Rect.bottom + 1);
			FontNormal->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorNormal);
			FontNormal->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorNormal);

			SetRect(&Rect, Rect.left, Rect.bottom + RowSpace, Rect.right, Rect.bottom + RowSpace + 2);
			TheRenderManager->device->Clear(1L, (D3DRECT*)&Rect, D3DCLEAR_TARGET, TextColorNormal, 0.0f, 0L);

			Rows[0] = 0;
			SetRect(&Rect, Rect.left + MainItemColumnSize * 0, Rect.bottom + RowSpace, Rect.left + MainItemColumnSize * 1, Rect.bottom + RowSpace + TextSize);
			DefinitionsList Items = TheSettingManager->GetMenuItems();
			ListSize = Items.size();
			Pages[0] = ListSize / RowsPerPage;
			DefinitionsList::iterator Item = Items.begin();
			for (int i = 0; i < ListSize; i++) {
				Text = Item->second.c_str();
				SetRect(&RectShadow, Rect.left + 1, Rect.top + 1, Rect.right + 1, Rect.bottom + 1);
				if (SelectedRow[0] == Rows[0]) {
					strcpy(SelectedItem, Item->first.c_str());
					FontSelected->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorSelected);
					FontSelected->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorSelected);
				}
				else {
					FontNormal->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorNormal);
					FontNormal->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorNormal);
				}
				Rect.left += MainItemColumnSize;
				Rect.right += MainItemColumnSize;
				Rows[0]++;
				Item++;
			}

			SetRect(&Rect, PositionX, Rect.bottom + RowSpace, PositionX + TitleColumnSize, Rect.bottom + RowSpace + 2);
			TheRenderManager->device->Clear(1L, (D3DRECT*)&Rect, D3DCLEAR_TARGET, TextColorNormal, 0.0f, 0L);

			int MenuRectX = PositionX;
			int MenuRectY = Rect.bottom + RowSpace;
			Rows[1] = 0;
			SetRect(&Rect, MenuRectX + ItemColumnSize * 0, MenuRectY, MenuRectX + ItemColumnSize * 1, MenuRectY + TextSize);
			DefinitionsList Definitions = TheSettingManager->GetMenuDefinitions(SelectedItem);
			ListSize = Definitions.size();
			Pages[1] = ListSize / RowsPerPage;
			DefinitionsList::iterator Definition = Definitions.begin();
			for (int i = 0; i < ListSize; i++) {
				if (i >= RowsPerPage * SelectedPage[1] && i < RowsPerPage * (SelectedPage[1] + 1)) {
					Text = Definition->second.c_str();
					Rect.top += TextSize + RowSpace;
					Rect.bottom += TextSize + RowSpace;
					SetRect(&RectShadow, Rect.left + 1, Rect.top + 1, Rect.right + 1, Rect.bottom + 1);
					if (SelectedRow[1] == Rows[1]) {
						strcpy(SelectedDefinition, Definition->first.c_str());
						if (SelectedColumn >= 1) {
							FontSelected->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorSelected);
							FontSelected->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorSelected);
						}
						else {
							FontNormal->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorNormal);
							FontNormal->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorNormal);
						}
					}
					else {
						FontNormal->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorNormal);
						FontNormal->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorNormal);
					}
					if (!memcmp(SelectedItem, "Shader", 6)) {
						if (SelectedRow[1] == Rows[1] && SelectedColumn >= 1)
							FontSelected->DrawTextA(NULL, Text, -1, &Rect, DT_CALCRECT, TextColorSelected);
						else
							FontNormal->DrawTextA(NULL, Text, -1, &Rect, DT_CALCRECT, TextColorNormal);
						Rect.left = Rect.right;
						Rect.right += 100;
						SetRect(&RectShadow, Rect.left + 1, Rect.top + 1, Rect.right + 1, Rect.bottom + 1);
						if (TheSettingManager->GetMenuShaderEnabled(Definition->first.c_str())) Text = "ENABLED"; else Text = "DISABLED";
						if (SelectedRow[1] == Rows[1] && SelectedColumn >= 1) {
							FontStatus->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorSelected);
							FontStatus->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorSelected);
						}
						else {
							FontStatus->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorNormal);
							FontStatus->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorNormal);
						}
						Rect.left = MenuRectX + ItemColumnSize * 0;
						Rect.right = MenuRectX + ItemColumnSize * 1;
					}
					Rows[1]++;
				}
				Definition++;
			}
			Rows[2] = 0;
			SetRect(&Rect, MenuRectX + ItemColumnSize * 1, MenuRectY, MenuRectX + ItemColumnSize * 2, MenuRectY + TextSize);
			SectionsList Sections = TheSettingManager->GetMenuSections(SelectedItem, SelectedDefinition);
			ListSize = Sections.size();
			Pages[2] = ListSize / RowsPerPage;
			SectionsList::iterator Section = Sections.begin();
			for (int i = 0; i < ListSize; i++) {
				if (i >= RowsPerPage * SelectedPage[2] && i < RowsPerPage * (SelectedPage[2] + 1)) {
					Text = Section->second.c_str();
					Rect.top += TextSize + RowSpace;
					Rect.bottom += TextSize + RowSpace;
					SetRect(&RectShadow, Rect.left + 1, Rect.top + 1, Rect.right + 1, Rect.bottom + 1);
					if (SelectedRow[2] == Rows[2]) {
						strcpy(SelectedSection, Text);
						SelectedSectionKey = Section->first;
						if (SelectedColumn >= 2) {
							FontSelected->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorSelected);
							FontSelected->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorSelected);
						}
						else {
							FontNormal->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorNormal);
							FontNormal->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorNormal);
						}
					}
					else {
						FontNormal->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorNormal);
						FontNormal->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorNormal);
					}
					Rows[2]++;
				}
				Section++;
			}
			Rows[3] = 0;
			SetRect(&Rect, MenuRectX + ItemColumnSize * 2, MenuRectY, MenuRectX + ItemColumnSize * 3, MenuRectY + TextSize);
			SettingsList Settings = TheSettingManager->GetMenuSettings(SelectedItem, SelectedDefinition, SelectedSection, SelectedSectionKey);
			ListSize = Settings.size();
			Pages[3] = ListSize / RowsPerPage;
			SettingsList::iterator Setting = Settings.begin();
			for (int i = 0; i < ListSize; i++) {
				if (i >= RowsPerPage * SelectedPage[3] && i < RowsPerPage * (SelectedPage[3] + 1)) {
					strcpy(SettingFirst, Setting->first.c_str());
					if (strstr(IntValues, SettingFirst) || strstr(BoolValues, SettingFirst))
						sprintf(SettingValue, "%.0f", Setting->second);
					else
						sprintf(SettingValue, MenuSettings->ValueFormat, Setting->second);
					std::string T = Setting->first + " = " + SettingValue;
					Text = T.c_str();
					Rect.top += TextSize + RowSpace;
					Rect.bottom += TextSize + RowSpace;
					SetRect(&RectShadow, Rect.left + 1, Rect.top + 1, Rect.right + 1, Rect.bottom + 1);
					if (SelectedRow[3] == Rows[3]) {
						strcpy(SelectedSetting, SettingFirst);
						SelectedValue = Setting->second;
						if (SelectedColumn >= 3) {
							if (EditingMode) {
								T = Setting->first + " = " + EditingValue;
								Text = T.c_str();
								FontSelected->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorEditing);
								FontSelected->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorEditing);
							}
							else {
								FontSelected->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorSelected);
								FontSelected->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorSelected);
							}
						}
						else {
							FontNormal->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorNormal);
							FontNormal->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorNormal);
						}
					}
					else {
						FontNormal->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorNormal);
						FontNormal->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorNormal);
					}
					Rows[3]++;
				}
				Setting++;
			}
		}
		if (TheSettingManager->SettingsMain.Main.FPSOverlay) {
			char Text[4];
			
			FrameCounter++;
			if (FrameCounter == 10) {
				FrameCounter = 0;
				FrameRate = 1.0 / TheFrameRateManager->ElapsedTime;
			}
			sprintf(Text, "%d", FrameRate);
			SetRect(&Rect, 9, 6, 109, 6 + TextSize);
			SetRect(&RectShadow, Rect.left + 1, Rect.top + 1, Rect.right + 1, Rect.bottom + 1);
			FontSelected->DrawTextA(NULL, Text, -1, &RectShadow, DT_LEFT, TextShadowColorSelected);
			FontSelected->DrawTextA(NULL, Text, -1, &Rect, DT_LEFT, TextColorSelected);
		}
	}

}
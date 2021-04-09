#include "FormHook.h"
#include "WeatherMode.h"

#if defined(NEWVEGAS)
#define kLoadForm 0x004601D0
static const UInt32 kSetRegionEditorName = 0x004F0D29;
static const UInt32 kSetRegionEditorNameReturn = 0x004F0E1B;
#elif defined(OBLIVION)
#define kLoadForm 0x00447050
static const UInt32 kSetRegionEditorName = 0x004A32A6;
static const UInt32 kSetRegionEditorNameReturn = 0x004A33A6;
#elif defined(SKYRIM)
#define kLoadForm 0x0043B4A0
static const UInt32 kSetRegionEditorName = 0x0048BEE4;
static const UInt32 kSetRegionEditorNameReturn = 0x0048BEEA;
#endif

bool (__cdecl* LoadForm)(TESForm*, UInt32) = (bool (__cdecl*)(TESForm*, UInt32))kLoadForm;
bool __cdecl TrackLoadForm(TESForm* Form, UInt32 ModEntry) {
	
	bool r = LoadForm(Form, ModEntry);
	switch (Form->formType) {
		case TESForm::FormType::kFormType_Weather: {
				if (TheSettingManager->SettingsMain.WeatherMode.Enabled) {
					TESWeatherEx* Weather = (TESWeatherEx*)Form;
					memcpy(Weather->colorsb, Weather->colors, WeatherColorsSize);
					TheSettingManager->SetSettingsWeather(Weather);
				}
			}
			break;
		case TESForm::FormType::kFormType_Water: {
				TESWaterForm* Water = (TESWaterForm*)Form;
				#if defined(OBLIVION)
				switch (Water->waterType) {
					case TESWaterForm::WaterType::kWaterType_Blood:
						Water->texture.ddsPath.Set("");
						Water->textureBlend = 25;
						break;
					case TESWaterForm::WaterType::kWaterType_Lava:
						Water->texture.ddsPath.Set("Water\\alternatelavaX.dds");
						Water->textureBlend = 50;
						break;
					case TESWaterForm::WaterType::kWaterType_Normal:
						Water->texture.ddsPath.Set("");
						Water->textureBlend = 0;
						break;
					default:
						Water->texture.ddsPath.Set("");
						Water->textureBlend = 0;
						break;
				}
				#endif
				if (TheSettingManager->SettingsMain.Main.RemoveUnderwater) Water->RemoveUnderwaterFog();
			}
			break;
		default:
			break;
	}
	return r;

}

void SetEditorName(TESRegionEx* Region, const char* Name) {

	strcpy(Region->EditorName, Name);

}

static __declspec(naked) void SetEditorNameHook() {

	__asm
	{
		push	ecx
		call	SetEditorName
		add		esp, 8
#if defined(OBLIVION)
		xor		esi, esi
#elif defined(SKYRIM)
		lea     ecx, [esp+0x20]
#endif
		jmp		kSetRegionEditorNameReturn
	}

}

void CreateFormLoadHook() {
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)LoadForm,		&TrackLoadForm);
	DetourTransactionCommit();

#if defined(NEWVEGAS)
	// Extends the TESRegion allocation (for each constructor call) to store additional data
	SafeWrite8(0x00466877, sizeof(TESRegionEx));
	SafeWrite8(0x004F1107, sizeof(TESRegionEx));
#elif defined(OBLIVION)
	// Extends the TESRegion allocation (for each constructor call) to store additional data
	SafeWrite8(0x00448843, sizeof(TESRegionEx));
	SafeWrite8(0x004A2EFF, sizeof(TESRegionEx));
#elif defined(SKYRIM)
	// Extends the TESRegion allocation (for each constructor call) to store additional data
	SafeWrite8(0x0048BC15, sizeof(TESRegionEx));
#endif
	SafeWriteJump(kSetRegionEditorName, (UInt32)SetEditorNameHook);

}
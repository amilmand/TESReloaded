#include "GrassMode.h"

static const UInt32 kGrassHook = 0x004EBF87;
static const UInt32 kGrassReturn = 0x004EC4E8;

void UpdateGrass(TESObjectCELL* Cell, NiNode* GrassNode, float CameraPosX, float CameraPosY, float CameraPosZ, float CameraForwardX, float CameraForwardY, int Arg8, float StartFadingDistance, float EndDistance, float Arg11) {

	for (UInt32 x = 0 ; x < *SettingGridsToLoad ; x++) {
		for (UInt32 y = 0 ; y < *SettingGridsToLoad ; y++) {
			CreateGrass(Tes->gridCellArray->GetCell(x, y), GrassNode, CameraPosX, CameraPosY, CameraPosZ, CameraForwardX, CameraForwardY, Arg8, StartFadingDistance, EndDistance, Arg11);
		}
	}

}

static __declspec(naked) void GrassHook() {

	__asm
	{
		call	UpdateGrass
		jmp		kGrassReturn
	}

}

void CreateGrassHook() {
	
	WriteRelJump(kGrassHook, (UInt32)GrassHook);

}
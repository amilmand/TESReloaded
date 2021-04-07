#include "FrameRateManager.h"

static const UInt32 kUpdateTimeInfoHook = 0x0040D8AB;
static const UInt32 kUpdateTimeInfoReturn = 0x0040D8B0;
static float* MPF = (float*)0x00B33E94;

FrameRateManager::FrameRateManager() {

	Logger::Log("Starting the framerate manager...");
	TheFrameRateManager = this;
	
	LARGE_INTEGER Frequency;
	LARGE_INTEGER PerformanceCounter;
	
	Time = 0.0;
	LastTime = 0.0;
	ElapsedTime = 0.0;
	LastPerformance = 0;
	SmartControlMPF = 1000.0 / TheSettingManager->SettingsMain.FrameRate.SmartControlFPS;
	QueryPerformanceFrequency(&Frequency); PerformanceFrequency = Frequency.QuadPart;
	QueryPerformanceCounter(&PerformanceCounter); PerformanceCounterStart = PerformanceCounter.QuadPart;

}

void FrameRateManager::UpdatePerformance() {
	
	LARGE_INTEGER PerformanceCounterEnd;
	
	QueryPerformanceCounter(&PerformanceCounterEnd);
	Time = (double)(PerformanceCounterEnd.QuadPart - PerformanceCounterStart) / (double)PerformanceFrequency;
	ElapsedTime = Time - LastTime;
	LastTime = Time;

}

double FrameRateManager::GetPerformance() {

	LARGE_INTEGER PerformanceCounterEnd;

	QueryPerformanceCounter(&PerformanceCounterEnd);
	return (double)(PerformanceCounterEnd.QuadPart - TheFrameRateManager->PerformanceCounterStart) * 1000.0 / (double)TheFrameRateManager->PerformanceFrequency;

}

void FrameRateManager::PerformSync() {

	double CurrentPerformance = GetPerformance();
	double FrameTime = CurrentPerformance - LastPerformance;

	if (FrameTime < SmartControlMPF) Sleep(SmartControlMPF - FrameTime);
	LastPerformance = GetPerformance();
	*MPF = (float)(FrameTime + TheSettingManager->SettingsMain.FrameRate.FlowControl + (LastPerformance - CurrentPerformance));

}

static __declspec(naked) void UpdateTimeInfoHook() {

	__asm {
		pushad
		mov		ecx, TheFrameRateManager
		call	FrameRateManager::PerformSync
		popad
		mov		eax, 0x0047D170
		call	eax
		jmp		kUpdateTimeInfoReturn
	}

}

void EndProcess() {
	
	void* SettingCollection = (void*)0x00B07BF0;
	char* SettingName = (char*)0x00B07BF4;

	ThisCall(0x0040C180, SettingCollection, SettingName);
	TerminateProcess(GetCurrentProcess(), 0);

}

void CreateFrameRateHook() {
	
	if (TheSettingManager->SettingsMain.FrameRate.SmartControl) WriteRelJump(kUpdateTimeInfoHook,	(UInt32)UpdateTimeInfoHook);
	WriteRelJump(0x0040F488, (UInt32)EndProcess);

}
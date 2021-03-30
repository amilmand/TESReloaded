#include "CameraMode.h"

#if defined(NEWVEGAS)
#define kToggleCamera 0x00950110
#define kToggleBody 0x00951A10
#define kSetDialogCamera 0x00953060
#define kSetAimingZoom 0x0095DE30
#define kUpdateCameraCollisions 0x0094A0C0
#define PlayerNode Player->renderData->niNode
#define ToggleCameraEcx [ebp - 0x18]
#define PlayerEx Player
static const UInt32 kUpdateCameraHook = 0x0094BDDA;
static const UInt32 kUpdateCameraReturn = 0x0094BDDF;
static const UInt32 kSwitchCameraHook = 0x00945C2D;
static const UInt32 kSwitchCameraReturn = 0x00945C34;
static const UInt32 kSwitchCameraPOVHook = 0x00942DC5;
static const UInt32 kSwitchCameraPOVReturn = 0x00942DCE;
#elif defined(OBLIVION)
#define kToggleCamera 0x0066C580
#define kToggleBody 0x00664F70
#define kSetDialogCamera 0x0066C6F0
#define kSetAimingZoom 0x00666670
#define kUpdateCameraCollisions 0x0065F080
#define PlayerNode Player->niNode
#define ToggleCameraEcx ebx
#define PlayerEx ((PlayerCharacterEx*)Player)
static const UInt32 kUpdateCameraHook = 0x0066BE6E;
static const UInt32 kUpdateCameraReturn = 0x0066BE7C;
static const UInt32 kSwitchCameraHook = 0x00671AC9;
static const UInt32 kSwitchCameraReturn = 0x00671AD0;
static const UInt32 kSwitchCameraPOVHook = 0x00672FDA;
static const UInt32 kSwitchCameraPOVReturn = 0x00672FE2;
#endif

#if defined(OBLIVION) || defined(NEWVEGAS)
static MobileObject* DialogObject = NULL;
static NiPoint3 From = { 0.0f, 0.0f, 0.0f };
static NiPoint3 FromOffset = { 0.0f, 0.0f, 0.0f };

class CameraMode {
public:
	void TrackToggleCamera(UInt8);
	void TrackToggleBody(UInt8);
	void TrackSetDialogCamera(MobileObject*, float, UInt8);
	void TrackUpdateCameraCollisions(NiPoint3*, NiPoint3*, UInt8);
	void TrackSetAimingZoom(float);
};

void (__thiscall CameraMode::* ToggleCamera)(UInt8);
void (__thiscall CameraMode::* TrackToggleCamera)(UInt8);
void CameraMode::TrackToggleCamera(UInt8 FirstPerson) {
	
	SettingsMainStruct::CameraModeStruct* CameraMode = &TheSettingManager->SettingsMain.CameraMode;

#if defined(NEWVEGAS)
	if (TheKeyboardManager->OnControlDown(13)) return (this->*ToggleCamera)(FirstPerson);
	if (Player->isSpecialView && Player->isSpecialViewPrevThird) Player->isSpecialViewPrevThird = ((UInt8)TheRenderManager->FirstPersonView) + 1;
	if (!Player->isSpecialView && Player->isSpecialViewPrevThird) FirstPerson = Player->isSpecialViewPrevThird - 1;
#endif
	TheRenderManager->FirstPersonView = FirstPerson;
#if defined(NEWVEGAS)
	if (Player->isSpecialView) { (this->*ToggleCamera)(1); return; }
#endif
	if (CameraMode->FirstPersonAiming && Player->IsAiming()) { (this->*ToggleCamera)(1); return; }
	(this->*ToggleCamera)(0);

}

void (__thiscall CameraMode::* ToggleBody)(UInt8);
void (__thiscall CameraMode::* TrackToggleBody)(UInt8);
void CameraMode::TrackToggleBody(UInt8 FirstPerson) {
	
	SettingsMainStruct::CameraModeStruct* CameraMode = &TheSettingManager->SettingsMain.CameraMode;

#if defined(NEWVEGAS)
	if (TheSettingManager->GameLoading) Player->unkThirdPersonPrev = Player->unkThirdPerson = Player->isThirdPersonBody = Player->isThirdPerson = 1;
	if (Player->isSpecialView) { (this->*ToggleBody)(1); return; }
#endif
	if (CameraMode->FirstPersonAiming && Player->IsAiming()) { (this->*ToggleBody)(1); return; }
	(this->*ToggleBody)(0);

}

void (__thiscall CameraMode::* SetDialogCamera)(MobileObject*, float, UInt8);
void (__thiscall CameraMode::* TrackSetDialogCamera)(MobileObject*, float, UInt8);
void CameraMode::TrackSetDialogCamera(MobileObject* Object, float Arg2, UInt8 Arg3) {

	DialogObject = Object;

}

void (__thiscall CameraMode::* UpdateCameraCollisions)(NiPoint3*, NiPoint3*, UInt8);
void (__thiscall CameraMode::* TrackUpdateCameraCollisions)(NiPoint3*, NiPoint3*, UInt8);
void CameraMode::TrackUpdateCameraCollisions(NiPoint3* CameraPosition, NiPoint3* PlayerPosition, UInt8 CameraChasing) {
	
	SettingsMainStruct::CameraModeStruct* CameraMode = &TheSettingManager->SettingsMain.CameraMode;
	UInt8 Crosshair = TheSettingManager->SettingsMain.CameraMode.Crosshair;
	UInt8 DisableFading = Player->DisableFading;
	UInt8 SitSleepState = Player->GetSitSleepState();
	bool IsThirdPerson = Player->IsThirdPersonView(CameraMode->Enabled, TheRenderManager->FirstPersonView);

	if (SitSleepState >= HighProcess::kSitSleep_SleepingIn && SitSleepState <= HighProcess::kSitSleep_SleepingOut) Player->DisableFading = 1;
	if (IsThirdPerson) {
		CameraChasing = !CameraMode->ChasingThird;
	}
	else {
		Player->DisableFading = 1;
		CameraChasing = !CameraMode->ChasingFirst;
	}
	(this->*UpdateCameraCollisions)(CameraPosition, PlayerPosition, CameraChasing);
	Player->DisableFading = DisableFading;
	if (Crosshair == 0) {
		if (IsThirdPerson) MenuManager->SetCrosshair(0); else MenuManager->SetCrosshair(1);
	}
	else {
		if (Crosshair == 1) {
			MenuManager->SetCrosshair(0);
		}
		else {
			if (Player->IsAiming())
				MenuManager->SetCrosshair(1);
			else
				MenuManager->SetCrosshair(0);
		}
	}

}

void (__thiscall CameraMode::* SetAimingZoom)(float);
void (__thiscall CameraMode::* TrackSetAimingZoom)(float);
void CameraMode::TrackSetAimingZoom(float Arg1) {
	
	SettingsMainStruct::CameraModeStruct* CameraMode = &TheSettingManager->SettingsMain.CameraMode;

	if (TheRenderManager->FirstPersonView) {
		if (CameraMode->FirstPersonAiming) {
			if ((Player->isThirdPerson && Player->IsAiming()) || (!Player->isThirdPerson && !Player->IsAiming())) ThisCall(kToggleCamera, Player, 1);
		}
		(this->*SetAimingZoom)(Arg1);
	}

}

void UpdateCamera(NiAVObject* CameraNode, NiPoint3* LocalPosition) {
	
	SettingsMainStruct::CameraModeStruct* CameraMode = &TheSettingManager->SettingsMain.CameraMode;
	NiMatrix33* CameraRotation = &CameraNode->m_localTransform.rot;
	NiPoint3* CameraPosition = &CameraNode->m_localTransform.pos;
	UInt8 SitSleepState = Player->GetSitSleepState();
	HighProcess* Process = (HighProcess*)Player->process;
	NiPoint3* HeadPosition = &Process->animData->nHead->m_worldTransform.pos;
	bool IsThirdPerson = Player->IsThirdPersonView(CameraMode->Enabled, TheRenderManager->FirstPersonView);
	NiPoint3 Rot = { 0.0f, 0.0f, 0.0f };
	NiMatrix33 mw, ml;
	NiPoint3 v;
	float x, y, z, r;
	
	if (SitSleepState < HighProcess::kSitSleep_SleepingIn || SitSleepState > HighProcess::kSitSleep_SleepingOut) From.x = 0.0f;
	if (IsThirdPerson && SitSleepState >= HighProcess::kSitSleep_SleepingIn && SitSleepState <= HighProcess::kSitSleep_SleepingOut) {
		if (From.x == 0.0f) {
			From.x = CameraPosition->x;
			From.y = CameraPosition->y;
			From.z = CameraPosition->z - 20.0f;
		}
		else {
			FromOffset = { 0.0f, 0.0f, 0.0f };
			if (TheKeyboardManager->OnControlPressed(2))
				FromOffset.x -= 5.0f;
			else if (TheKeyboardManager->OnControlPressed(3))
				FromOffset.x += 5.0f;
			if (TheKeyboardManager->OnControlPressed(0))
				FromOffset.y += 5.0f;
			else if (TheKeyboardManager->OnControlPressed(1))
				FromOffset.y -= 5.0f;
			if (TheKeyboardManager->OnControlPressed(4))
				FromOffset.z += 5.0f;
			else if (TheKeyboardManager->OnControlPressed(6))
				FromOffset.z -= 5.0f;
			if (FromOffset.x != 0.0f || FromOffset.y != 0.0f || FromOffset.z != 0.0f) {
				v = CameraNode->m_worldTransform.rot * FromOffset;
				From.x += v.x;
				From.y += v.y;
				From.z += v.z;
			}
		}
		CameraPosition->x = From.x;
		CameraPosition->y = From.y;
		CameraPosition->z = From.z;
		mw.GenerateRotationMatrixZXY(&Rot, 1);
		x = CameraPosition->x - HeadPosition->x;
		y = CameraPosition->y - HeadPosition->y;
		z = CameraPosition->z - HeadPosition->z;
		r = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
		Rot.x = D3DXToDegree(atan2(y, x)) + 90.0f;
		Rot.y = D3DXToDegree(acos(z / r)) - 90.0f;
		Rot.z = 0;
		ml.GenerateRotationMatrixZXY(&Rot, 1);
		CameraNode->m_worldTransform.rot = mw;
		CameraNode->m_localTransform.rot = ml;
	}
	else if (MenuManager->IsActive(Menu::MenuType::kMenuType_Dialog) || MenuManager->IsActive(Menu::MenuType::kMenuType_Persuasion)) {
		HighProcess* DialogObjectProcess = (HighProcess*)DialogObject->process;
		v = PlayerNode->m_worldTransform.rot * CameraMode->DialogOffset;
		CameraPosition->x = HeadPosition->x + v.x;
		CameraPosition->y = HeadPosition->y + v.y;
		CameraPosition->z = HeadPosition->z + v.z;
		if (DialogObjectProcess->animData)
			HeadPosition = &DialogObjectProcess->animData->nHead->m_worldTransform.pos;
		else
			HeadPosition = &DialogObject->pos;
		mw.GenerateRotationMatrixZXY(&Rot, 1);
		x = CameraPosition->x - HeadPosition->x;
		y = CameraPosition->y - HeadPosition->y;
		z = CameraPosition->z - HeadPosition->z;
		r = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
		Rot.x = D3DXToDegree(atan2(y, x)) + 90.0f;
		Rot.y = D3DXToDegree(acos(z / r)) - 90.0f;
		Rot.z = 0;
		ml.GenerateRotationMatrixZXY(&Rot, 1);
		CameraNode->m_worldTransform.rot = mw;
		CameraNode->m_localTransform.rot = ml;
	}
	else if (!IsThirdPerson && !Player->IsVanityView()) {
		if (SitSleepState != 3 && SitSleepState != 5) {
			NiPoint3 o = { CameraMode->Offset.z, CameraMode->Offset.y, CameraMode->Offset.x };
			if (Player->IsAiming()) {
				switch (Player->ActorSkinInfo->WeaponForm->weaponType) {
#if defined(NEWVEGAS)
					case TESObjectWEAP::WeaponType::kWeapType_OneHandPistol:
					case TESObjectWEAP::WeaponType::kWeapType_OneHandPistolEnergy:
						o.x = CameraMode->OneHandAimingOffset.z;
						o.y = CameraMode->OneHandAimingOffset.y;
						o.z = CameraMode->OneHandAimingOffset.x;
						break;
					case TESObjectWEAP::WeaponType::kWeapType_TwoHandRifle:
					case TESObjectWEAP::WeaponType::kWeapType_TwoHandAutomatic:
					case TESObjectWEAP::WeaponType::kWeapType_TwoHandRifleEnergy:
					case TESObjectWEAP::WeaponType::kWeapType_TwoHandHandle:
					case TESObjectWEAP::WeaponType::kWeapType_TwoHandLauncher:
						o.x = CameraMode->TwoHandAimingOffset.z;
						o.y = CameraMode->TwoHandAimingOffset.y;
						o.z = CameraMode->TwoHandAimingOffset.x;
						break;
#elif defined(OBLIVION)
					case TESObjectWEAP::WeaponType::kWeapType_Bow:
						o.x = CameraMode->BowAimingOffset.z;
						o.y = CameraMode->BowAimingOffset.y;
						o.z = CameraMode->BowAimingOffset.x;
						break;
#endif
					default:
						break;
				}
			}
			v = Process->animData->nHead->m_worldTransform.rot * o;
			CameraPosition->x = HeadPosition->x + v.x;
			CameraPosition->y = HeadPosition->y + v.y;
			CameraPosition->z = HeadPosition->z + v.z;
			PlayerEx->ReticleOffset.x = v.x;
			PlayerEx->ReticleOffset.y = v.y;
			PlayerEx->ReticleOffset.z = v.z;
		}
		BSAnimGroupSequence* AnimSequence = Process->animData->animSequences[0];
		TESAnimGroup* AnimGroup = (AnimSequence ? AnimSequence->animGroup : NULL);
		if ((AnimGroup && AnimGroup->animGroup >= TESAnimGroup::kAnimGroup_DodgeForward && AnimGroup->animGroup <= TESAnimGroup::kAnimGroup_DodgeRight && AnimGroup->animType == 0) || (SitSleepState >= HighProcess::kSitSleep_SleepingIn && SitSleepState <= HighProcess::kSitSleep_SleepingOut) || !Player->IsAlive() || Process->KnockedState) {
			Rot.z = 90.0f;
			ml.GenerateRotationMatrixZXY(&Rot, 1);
			ml = Process->animData->nHead->m_worldTransform.rot * ml;
			memcpy(CameraRotation, &ml, 0x24);
		}
	}
	LocalPosition->x = CameraPosition->x;
	LocalPosition->y = CameraPosition->y;
	LocalPosition->z = CameraPosition->z;

}

static __declspec(naked) void UpdateCameraHook() {

	__asm {
#if defined(NEWVEGAS)
		push	0
		push	0
		push	ecx
		pushad
		lea		edx, [ebp - 0x60]
		push	edx
		push	eax
		call	UpdateCamera
		pop		eax
		pop		edx
		popad
#elif defined (OBLIVION)
		pushad
		lea		ecx, [esp + 0x58]
		push	ecx
		push	eax
		call	UpdateCamera
		pop		eax
		pop		ecx
		popad
#endif
		jmp		kUpdateCameraReturn
	}

}

static __declspec(naked) void SwitchCameraHook() {

	__asm {
		mov		eax, TheRenderManager
		movzx	edx, byte ptr[eax + RenderManager::FirstPersonView]
#if defined(NEWVEGAS)
		xor		edx, 1
#elif defined(OBLIVION)
		cmp		edx, 1
#endif
		jmp		kSwitchCameraReturn
	}

}

static const UInt32 GameToggleCamera = kToggleCamera;
static __declspec(naked) void SwitchCameraPOVHook() {

	__asm {
		mov		eax, TheRenderManager
		movzx	ecx, byte ptr [eax + RenderManager::FirstPersonView]
		xor		ecx, 1
		push	ecx
		mov		ecx, ToggleCameraEcx
		call	GameToggleCamera
		jmp		kSwitchCameraPOVReturn
	}

}

#if defined(OBLIVION)
void SetReticleOffset(NiPoint3* CameraPos) {
	
	CameraPos->x += PlayerEx->ReticleOffset.x;
	CameraPos->y += PlayerEx->ReticleOffset.y;
	CameraPos->z += PlayerEx->ReticleOffset.z;

}

static const UInt32 kSetReticleOffsetHook = 0x00580796;
static const UInt32 kSetReticleOffsetReturn = 0x0058079C;
static __declspec(naked) void SetReticleOffsetHook() {

	__asm {
		lea		eax, [esp + 0x20]
		push	eax
		call	SetReticleOffset
		pop		eax
		mov     ecx, Player
		jmp		kSetReticleOffsetReturn
	}

}
#endif

void CreateCameraModeHook() {

	*((int*)&ToggleCamera)					= kToggleCamera;
	TrackToggleCamera						= &CameraMode::TrackToggleCamera;
	*((int*)&ToggleBody)					= kToggleBody;
	TrackToggleBody							= &CameraMode::TrackToggleBody;
	*((int *)&SetDialogCamera)				= kSetDialogCamera;
	TrackSetDialogCamera					= &CameraMode::TrackSetDialogCamera;
	*((int *)&SetAimingZoom)				= kSetAimingZoom;
	TrackSetAimingZoom						= &CameraMode::TrackSetAimingZoom;
	*((int *)&UpdateCameraCollisions)		= kUpdateCameraCollisions;
	TrackUpdateCameraCollisions				= &CameraMode::TrackUpdateCameraCollisions;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)ToggleCamera,				*((PVOID*)&TrackToggleCamera));
	DetourAttach(&(PVOID&)ToggleBody,				*((PVOID*)&TrackToggleBody));
	DetourAttach(&(PVOID&)SetDialogCamera,			*((PVOID*)&TrackSetDialogCamera));
	DetourAttach(&(PVOID&)SetAimingZoom,			*((PVOID*)&TrackSetAimingZoom));
	DetourAttach(&(PVOID&)UpdateCameraCollisions,	*((PVOID*)&TrackUpdateCameraCollisions));
	DetourTransactionCommit();

	WriteRelJump(kUpdateCameraHook,		 (UInt32)UpdateCameraHook);
	WriteRelJump(kSwitchCameraHook,		 (UInt32)SwitchCameraHook);
	WriteRelJump(kSwitchCameraPOVHook,   (UInt32)SwitchCameraPOVHook);
#if defined(OBLIVION)
	WriteRelJump(kSetReticleOffsetHook,  (UInt32)SetReticleOffsetHook);
	WriteRelJump(0x0066B769, 0x0066B795); // Does not lower the player Z axis value (fixes the bug of the camera on feet after resurrection)
	WriteRelJump(0x00666704, 0x0066672D); // Enables the zoom with the bow

	// Extends the PlayerCharacter allocation (for each constructor call) to store additional data
	SafeWrite32(0x00406775, sizeof(PlayerCharacterEx));
	SafeWrite32(0x0046451E, sizeof(PlayerCharacterEx));
#endif

}
#elif defined (SKYRIM)
static bool TogglePOV = false;

class CameraMode {
public:
	int TrackSetCameraState(TESCameraState*);
	void TrackManageButtonEvent(ButtonEvent*, int);
	void TrackSetCameraPosition();
};

int (__thiscall CameraMode::* SetCameraState)(TESCameraState*);
int (__thiscall CameraMode::* TrackSetCameraState)(TESCameraState*);
int CameraMode::TrackSetCameraState(TESCameraState* CameraState) {
	
	PlayerCamera* Camera = (PlayerCamera*)this;
	bool IsWeaponOut = false;

	if (Camera->cameraNode->m_pcName && CameraState->camera->thirdPersonState2 != NULL) {
		if (CameraState->stateId == TESCameraState::CameraState::kCameraState_FirstPerson) {
			if (TheRenderManager->FirstPersonView && TogglePOV) {
				CameraState = Camera->thirdPersonState2;
				TheRenderManager->FirstPersonView = false;
			}
			else {
				CameraState = Camera->thirdPersonState2;
				TheRenderManager->FirstPersonView = true;
			}
		}
		else if (CameraState->stateId == TESCameraState::CameraState::kCameraState_ThirdPerson2) TheRenderManager->FirstPersonView = false;
		if (TheRenderManager->FirstPersonView && CameraState->stateId != TESCameraState::CameraState::kCameraState_ThirdPerson2) TheRenderManager->FirstPersonView = false;
		if (!TheRenderManager->FirstPersonView && CameraState->stateId == TESCameraState::CameraState::kCameraState_ThirdPerson2) {
			IsWeaponOut = Player->actorState.IsWeaponOut();
			Camera->AllowVanityMode = !IsWeaponOut;
			Camera->UpdateOverShoulder(IsWeaponOut);
		}
		TogglePOV = false;
	}
	return (this->*SetCameraState)(CameraState);

}

void (__thiscall CameraMode::* ManageButtonEvent)(ButtonEvent*, int);
void (__thiscall CameraMode::* TrackManageButtonEvent)(ButtonEvent*, int);
void CameraMode::TrackManageButtonEvent(ButtonEvent* Event, int Arg2) {
	
	ThirdPersonState* State = (ThirdPersonState*)(this - 0x10); //ecx is ThirdPersonState for PlayerInputHandler (class is "shifted" due to the multi inheritance)
	
	(this->*ManageButtonEvent)(Event, Arg2);
	if (State->stateId == TESCameraState::CameraState::kCameraState_ThirdPerson2) {
		if (PlayerControls::Get()->IsCamSwitchControlEnabled()) {
			if (State->TogglePOV) TogglePOV = true;
			if (TheRenderManager->FirstPersonView && *Event->GetControlID() == InputStringHolder::Get()->zoomOut) State->camera->SetCameraState(State->camera->thirdPersonState2);
		}
	}

}

void (__thiscall CameraMode::* SetCameraPosition)();
void (__thiscall CameraMode::* TrackSetCameraPosition)();
void CameraMode::TrackSetCameraPosition() {

	ThirdPersonState* State = (ThirdPersonState*)this;
	BSFixedString Head;

	if (TheRenderManager->FirstPersonView) {
		Head.Create("NPC Head [Head]");
		NiNode* ActorNode = Player->GetNiRootNode(0);
		NiPoint3* HeadPosition = &ActorNode->GetObjectByName(&Head)->m_worldTransform.pos;
		NiPoint3 v = ActorNode->m_worldTransform.rot * TheSettingManager->SettingsMain.CameraMode.Offset;
		State->CameraPosition.x = HeadPosition->x + v.x;
		State->CameraPosition.y = HeadPosition->y + v.y;
		State->CameraPosition.z = HeadPosition->z + v.z;
		State->OverShoulderPosX = State->OverShoulderPosY = State->OverShoulderPosZ = 0.0f;
		State->camera->AllowVanityMode = 0;
		Head.Dispose();
	}
	(this->*SetCameraPosition)();
	
}

void CreateCameraModeHook()
{

	*((int *)&SetCameraState)			= 0x006533D0;
	TrackSetCameraState					= &CameraMode::TrackSetCameraState;
	*((int *)&ManageButtonEvent)		= 0x00840BE0;
	TrackManageButtonEvent				= &CameraMode::TrackManageButtonEvent;
	*((int *)&SetCameraPosition)		= 0x0083F690;
	TrackSetCameraPosition				= &CameraMode::TrackSetCameraPosition;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)SetCameraState,		*((PVOID*)&TrackSetCameraState));
	DetourAttach(&(PVOID&)ManageButtonEvent,	*((PVOID*)&TrackManageButtonEvent));
	DetourAttach(&(PVOID&)SetCameraPosition,	*((PVOID*)&TrackSetCameraPosition));
	DetourTransactionCommit();
	
	SafeWrite8(0x0083F69B, 0); // Stops PlayerCharacter fading

}
#endif
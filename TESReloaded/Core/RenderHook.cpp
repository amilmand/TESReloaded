#include <CommCtrl.h>
#include "RenderHook.h"

#if defined(NEWVEGAS)
#define kRender 0x008706B0
#define kProcessImageSpaceShaders 0x00B55AC0
#define kBeginScene 0x00E74F40
#define kMenuRenderedTexture 0x011DED3C
#define kDetectorWindowScale 0x004D67A5
#define kDetectorWindowScaleReturn 0x004D67E9
static const UInt32 kRenderInterface = 0x007144D3;
static const UInt32 kRenderInterfaceReturn = 0x007144D8;
static const UInt32 kRenderInterfaceMethod = 0x007134D0;
static const UInt32 kSetTileShaderConstants = 0x00BCAAD7;
static const UInt32 kSetTileShaderConstantsReturn = 0x00BCAADE;
static const UInt32 kMultiBoundWaterHeightFix1 = 0x006FAE04;
static const UInt32 kMultiBoundWaterHeightFix2 = 0x006FAF50;
static const UInt32 kDetectorWindowSetNodeName = 0x004D678A;
static const UInt32 kDetectorWindowCreateTreeView = 0x004D6476;
static const UInt32 kDetectorWindowCreateTreeViewReturn = 0x004D647F;
static const UInt32 kDetectorWindowDumpAttributes = 0x004D6A2E;
static const UInt32 kDetectorWindowDumpAttributesReturn = 0x004D69C5;
#elif defined(OBLIVION)
#define kRender 0x0040C830
#define kProcessImageSpaceShaders 0x007B48E0
#define kBeginScene 0x0076BE00
#define kMenuRenderedTexture 0x00B333E8
#define kDetectorWindowScale 0x004965A8
#define kDetectorWindowScaleReturn 0x0049660F
static const UInt32 kRenderInterface = 0x0057F3F3;
static const UInt32 kRenderInterfaceReturn = 0x0057F3F8;
static const UInt32 kRenderInterfaceMethod = 0x0070E0A0;
static const UInt32 kDetectorWindowSetNodeName = 0x0049658E;
static const UInt32 kDetectorWindowCreateTreeView = 0x00495E1F;
static const UInt32 kDetectorWindowCreateTreeViewReturn = 0x00495E27;
static const UInt32 kDetectorWindowDumpAttributes = 0x004967C7;
static const UInt32 kDetectorWindowDumpAttributesReturn = 0x004967CD;
static bool ProcessReflectionsGeos = false;
#elif defined(SKYRIM)
#define kRender 0x0069BDF0
#define kProcessImageSpaceShaders 0x00C70DA0
#define kBeginScene 0x00CDA620
#define kMenuRenderedTexture 0x01B2E8D8
static const UInt32 kRenderInterface = 0x00A5CB57;
static const UInt32 kRenderInterfaceReturn = 0x00A5CB5C;
static const UInt32 kRenderInterfaceMethod = 0x00A623F0;
static const UInt32 kRenderingGeometry = 0x00CAFFC0;
static const UInt32 kRenderingGeometryReturn = 0x00CAFFC5;
static const UInt32 kSetShadowDistance = 0x00CBB140;
static const UInt32 kSetShadowDistanceReturn = 0x00CBB146;
static const UInt32 kSetShadowDistanceShader = 0x00CB7365;
static const UInt32 kSetShadowDistanceShaderReturn = 0x00CB736D;
static NiGeometry* Geometry = NULL;
#endif

class RenderHook {
public:
	bool	TrackBeginScene();
#if defined(NEWVEGAS)
	void*	TrackShowDetectorWindow(HWND, HINSTANCE, NiNode*, char*, int, int, int, int);
	void	TrackRender(BSRenderedTexture*, int, int);
	void	TrackRenderWorldSceneGraph(Sun*, UInt8, UInt8, UInt8);
	void	TrackRenderFirstPerson(NiDX9Renderer*, NiGeometry*, Sun*, BSRenderedTexture*);
	void	TrackRenderPipboyScreen(NiGeometry*, NiDX9Renderer*);
	float	TrackGetWaterHeightLOD();
#elif defined (OBLIVION)
	void*	TrackShowDetectorWindow(HWND, HINSTANCE, NiNode*, char*, int, int, int, int);
	void	TrackRender(BSRenderedTexture*);
	bool	TrackEndTargetGroup(NiCamera*, NiRenderTargetGroup*);
	void	TrackHDRRender(NiScreenElements*, BSRenderedTexture**, BSRenderedTexture**, UInt8);
	UInt32	TrackSetupShaderPrograms(NiGeometry*, NiSkinInstance*, NiSkinPartition::Partition*, NiGeometryBufferData*, NiPropertyState*, NiDynamicEffectState*, NiTransform*, UInt32);
	void	TrackBSFadeNodeOnVisible(NiCullingProcess*);
	float	TrackFarPlane();
	HRESULT TrackSetSamplerState(UInt32, D3DSAMPLERSTATETYPE, UInt32, UInt8);
	void	TrackRenderReflections(NiCamera*, ShadowSceneNode*);
	void	TrackWaterCullingProcess(NiAVObject*);
#elif defined(SKYRIM)
	void	TrackRender(BSRenderedTexture*, int, int);
	void	TrackRenderWorldSceneGraph(Sun*, UInt8, UInt8);
	void	TrackRenderFirstPerson(NiDX9Renderer*, NiGeometry*, Sun*, BSRenderedTexture*, UInt8);
#endif

};

bool (__thiscall RenderHook::* BeginScene)();
bool (__thiscall RenderHook::* TrackBeginScene)();
bool RenderHook::TrackBeginScene() {

	TheShaderManager->BeginScene();
	return (this->*BeginScene)();

}

#if defined(NEWVEGAS) || defined(OBLIVION)
void* (__thiscall RenderHook::* ShowDetectorWindow)(HWND, HINSTANCE, NiNode*, char*, int, int, int, int);
void* (__thiscall RenderHook::* TrackShowDetectorWindow)(HWND, HINSTANCE, NiNode*, char*, int, int, int, int);
void* RenderHook::TrackShowDetectorWindow(HWND Handle, HINSTANCE Instance, NiNode* RootNode, char* FormCaption, int X, int Y, int Width, int Height) {

	NiAVObject* Object = NULL;
	void* r = NULL;

	r = (this->*ShowDetectorWindow)(Handle, Instance, RootNode, "Pipeline detector by Alenet", X, Y, 1024, 1024);
	for (int i = 0; i < RootNode->m_children.end; i++) {
		NiNode* Node = (NiNode*)RootNode->m_children.data[i];
		Node->m_children.data[0] = NULL;
		Node->m_children.data[1] = NULL;
		Node->m_children.end = 0;
		Node->m_children.numObjs = 0;
	}
	return r;

}
#endif

#if defined(NEWVEGAS)
void (__thiscall RenderHook::* Render)(BSRenderedTexture*, int, int);
void (__thiscall RenderHook::* TrackRender)(BSRenderedTexture*, int, int);
void RenderHook::TrackRender(BSRenderedTexture* RenderedTexture, int Arg2, int Arg3) {
	
	TheRenderManager->SetSceneGraph();
	TheShaderManager->UpdateConstants();
	if (TheSettingManager->SettingsMain.Develop.TraceShaders && DWNode::Get() == NULL && TheKeyboardManager->OnKeyDown(TheSettingManager->SettingsMain.Develop.TraceShaders)) DWNode::Create();
	(this->*Render)(RenderedTexture, Arg2, Arg3);

}

void (__cdecl * SetupRenderingPass)(UInt32, NiD3DShader*) = (void (__cdecl *)(UInt32, NiD3DShader*))0x00B99390;
void __cdecl TrackSetupRenderingPass(UInt32 PassIndex, NiD3DShader* Shader) {
	
	SetupRenderingPass(PassIndex, Shader);

	NiGeometry* Geometry = *(NiGeometry**)(*(void**)0x011F91E0);
	NiD3DPass* Pass = *(NiD3DPass**)0x0126F74C;
	NiD3DVertexShaderEx* VertexShader = (NiD3DVertexShaderEx*)Pass->VertexShader;
	NiD3DPixelShaderEx* PixelShader = (NiD3DPixelShaderEx*)Pass->PixelShader;

	if (VertexShader && PixelShader) {
		if (VertexShader->ShaderProg) VertexShader->ShaderProg->SetCT();
		if (PixelShader->ShaderProg) PixelShader->ShaderProg->SetCT();
		if (DWNode::Get()) {
			char Name[256];
			sprintf(Name, "Pass %s (%s %s)", Geometry->m_pcName, VertexShader->ShaderName, PixelShader->ShaderName);
			if (!VertexShader->ShaderProg) strcat(Name, " - Vertex: vanilla");
			if (!PixelShader->ShaderProg) strcat(Name, " - Pixel: vanilla");
			DWNode::AddNode(Name, Geometry->m_parent, Geometry);
		}
		if (TheSettingManager->SettingsMain.Develop.LogShaders && TheKeyboardManager->OnKeyDown(TheSettingManager->SettingsMain.Develop.LogShaders)) Logger::Log("Pass %s (%s %s)", Geometry->m_pcName, VertexShader->ShaderName, PixelShader->ShaderName);
	}

}

void (__thiscall RenderHook::* RenderWorldSceneGraph)(Sun*, UInt8, UInt8, UInt8);
void (__thiscall RenderHook::* TrackRenderWorldSceneGraph)(Sun*, UInt8, UInt8, UInt8);
void RenderHook::TrackRenderWorldSceneGraph(Sun* SkySun, UInt8 IsFirstPerson, UInt8 WireFrame, UInt8 Arg4) {
	
	bool CameraMode = TheSettingManager->SettingsMain.CameraMode.Enabled;

	(this->*RenderWorldSceneGraph)(SkySun, IsFirstPerson, WireFrame, Arg4);
	if (CameraMode || Player->IsThirdPersonView(CameraMode, TheRenderManager->FirstPersonView)) TheRenderManager->ResolveDepthBuffer();

}

void (__thiscall RenderHook::* RenderFirstPerson)(NiDX9Renderer*, NiGeometry*, Sun*, BSRenderedTexture*);
void (__thiscall RenderHook::* TrackRenderFirstPerson)(NiDX9Renderer*, NiGeometry*, Sun*, BSRenderedTexture*);
void RenderHook::TrackRenderFirstPerson(NiDX9Renderer* Renderer, NiGeometry* Geo, Sun* SkySun, BSRenderedTexture* RenderedTexture) {
	
	(this->*RenderFirstPerson)(Renderer, Geo, SkySun, RenderedTexture);
	TheRenderManager->ResolveDepthBuffer();
	TheRenderManager->Clear(NULL, NiRenderer::kClear_ZBUFFER);
	ThisCall(0x00874C10, Global);
	(this->*RenderFirstPerson)(Renderer, Geo, SkySun, RenderedTexture);

}

void (__thiscall RenderHook::* RenderPipboyScreen)(NiGeometry*, NiDX9Renderer*);
void (__thiscall RenderHook::* TrackRenderPipboyScreen)(NiGeometry*, NiDX9Renderer*);
void RenderHook::TrackRenderPipboyScreen(NiGeometry* Geo, NiDX9Renderer* Renderer) {
	
	WorldSceneGraph->SetCameraFOV(Player->firstPersonFoV);
	(this->*RenderPipboyScreen)(Geo, Renderer);

}

float (__thiscall RenderHook::* GetWaterHeightLOD)();
float (__thiscall RenderHook::* TrackGetWaterHeightLOD)();
float RenderHook::TrackGetWaterHeightLOD() {
	
	TESWorldSpace* Worldspace = (TESWorldSpace*)this;
	float r = Worldspace->waterHeight;

	// Due to compiler optimization, the function GetWaterHeightLOD is "shared" as general method that returns ECX + 0x07C
	if (*(void**)Worldspace == (void*)0x0103195C) r = Tes->GetWaterHeight(Player);
	return r;

}

void (__cdecl * SetShaderPackage)(int, int, UInt8, int, char*, int) = (void (__cdecl *)(int, int, UInt8, int, char*, int))0x00B4F710;
void __cdecl TrackSetShaderPackage(int Arg1, int Arg2, UInt8 Force1XShaders, int Arg4, char* GraphicsName, int Arg6) {

	SetShaderPackage(Arg1, Arg2, Force1XShaders, Arg4, GraphicsName, Arg6);

}

void RenderMainMenuMovie() {

	if (TheSettingManager->SettingsMain.Main.ReplaceIntro && MenuManager->IsActive(Menu::MenuType::kMenuType_Main))
		Binker::Render(TheRenderManager->device, TheSettingManager->CurrentPath, MainMenuMovie, TheRenderManager->width, TheRenderManager->height);
	else
		Binker::Close(TheRenderManager->device);

}

void SetTileShaderAlpha() {
	
	float ViewProj[16];
	NiVector4 TintColor = { 1.0f, 1.0f, 1.0f, 0.0f };

	if (MenuManager->IsActive(Menu::MenuType::kMenuType_Main)) {	
		TheRenderManager->device->GetVertexShaderConstantF(0, ViewProj, 4);
		if ((int)ViewProj[3] == -1 && (int)ViewProj[7] == 1 && (int)ViewProj[15] == 1) TheRenderManager->device->SetPixelShaderConstantF(0, (const float*)&TintColor, 1);
	}

}

static __declspec(naked) void SetTileShaderConstants() {

	__asm {
		pushad
		call	SetTileShaderAlpha
		popad
		cmp		byte ptr [esi + 0xAC], 0
		jmp		kSetTileShaderConstantsReturn
	}

}

float MultiBoundWaterHeightFix() {

	return Player->pos.z;

}

#elif defined(OBLIVION)
void (__thiscall RenderHook::* Render)(BSRenderedTexture*);
void (__thiscall RenderHook::* TrackRender)(BSRenderedTexture*);
void RenderHook::TrackRender(BSRenderedTexture* RenderedTexture) {
	
	TheRenderManager->SetSceneGraph();
	TheShaderManager->UpdateConstants();
	if (TheRenderManager->BackBuffer) TheRenderManager->defaultRTGroup->RenderTargets[0]->data->Surface = TheRenderManager->defaultRTGroup->RenderTargets[1]->data->Surface;
	if (TheSettingManager->SettingsMain.Develop.TraceShaders && DWNode::Get() == NULL && TheKeyboardManager->OnKeyDown(TheSettingManager->SettingsMain.Develop.TraceShaders)) DWNode::Create();
	if (TheSettingManager->SettingsMain.Develop.LogShaders && TheKeyboardManager->OnKeyDown(TheSettingManager->SettingsMain.Develop.LogShaders)) Logger::Log("START FRAME LOG");
	(this->*Render)(RenderedTexture);

}

bool (__thiscall RenderHook::* EndTargetGroup)(NiCamera*, NiRenderTargetGroup*);
bool (__thiscall RenderHook::* TrackEndTargetGroup)(NiCamera*, NiRenderTargetGroup*);
bool RenderHook::TrackEndTargetGroup(NiCamera* Camera, NiRenderTargetGroup* TargetGroup) {

	if (TheRenderManager->BackBuffer) TargetGroup = TheRenderManager->defaultRTGroup;
	return (this->*EndTargetGroup)(Camera, TargetGroup);

}

void (__thiscall RenderHook::* HDRRender)(NiScreenElements*, BSRenderedTexture**, BSRenderedTexture**, UInt8);
void (__thiscall RenderHook::* TrackHDRRender)(NiScreenElements*, BSRenderedTexture**, BSRenderedTexture**, UInt8);
void RenderHook::TrackHDRRender(NiScreenElements* ScreenElements, BSRenderedTexture** RenderedTexture1, BSRenderedTexture** RenderedTexture2, UInt8 Arg4) {
	
	TheRenderManager->clearColor = D3DCOLOR_ARGB(0, 0, 0, 0);
	(this->*HDRRender)(ScreenElements, RenderedTexture1, RenderedTexture2, Arg4);

}

void (__thiscall RenderHook::* BSFadeNodeOnVisible)(NiCullingProcess*);
void (__thiscall RenderHook::* TrackBSFadeNodeOnVisible)(NiCullingProcess*);
void RenderHook::TrackBSFadeNodeOnVisible(NiCullingProcess* Process) {
	
	BSFadeNode* Node = (BSFadeNode*)this;

	if (TheSettingManager->SettingsMain.FrameRate.Enabled && Player->GetWorldSpace()) {
		if (Node->MultType == 6 && !strstr(Node->m_pcName, "ImperialCity") && TheFrameRateManager->IsOutGrid(Node)) return;
	}
	(this->*BSFadeNodeOnVisible)(Process);

}

float (__thiscall RenderHook::* FarPlane)();
float (__thiscall RenderHook::* TrackFarPlane)();
float RenderHook::TrackFarPlane() {
	
	float r = (this->*FarPlane)();

	if (TheSettingManager->SettingsMain.Main.FarPlaneDistance && r == 283840.0f) r = TheSettingManager->SettingsMain.Main.FarPlaneDistance;
	return r;

}

UInt32 (__thiscall RenderHook::* SetupShaderPrograms)(NiGeometry*, NiSkinInstance*, NiSkinPartition::Partition*, NiGeometryBufferData*, NiPropertyState*, NiDynamicEffectState*, NiTransform*, UInt32);
UInt32 (__thiscall RenderHook::* TrackSetupShaderPrograms)(NiGeometry*, NiSkinInstance*, NiSkinPartition::Partition*, NiGeometryBufferData*, NiPropertyState*, NiDynamicEffectState*, NiTransform*, UInt32);
UInt32 RenderHook::TrackSetupShaderPrograms(NiGeometry* Geometry, NiSkinInstance* SkinInstance, NiSkinPartition::Partition* SkinPartition, NiGeometryBufferData* GeometryBufferData, NiPropertyState* PropertyState, NiDynamicEffectState* EffectState, NiTransform* WorldTransform, UInt32 WorldBound) {

	NiD3DPass* Pass = ((NiD3DShader*)this)->CurrentPass;
	NiD3DVertexShaderEx* VertexShader = (NiD3DVertexShaderEx*)Pass->VertexShader;
	NiD3DPixelShaderEx* PixelShader = (NiD3DPixelShaderEx*)Pass->PixelShader;

	if (VertexShader && PixelShader) {
		if (PixelShader->ShaderProg && Pass->Stages.numObjs && Pass->Stages.data[0]->Texture) {
			TheShaderManager->ShaderConst.TextureData.x = Pass->Stages.data[0]->Texture->GetWidth();
			TheShaderManager->ShaderConst.TextureData.y = Pass->Stages.data[0]->Texture->GetHeight();
		}
		if (VertexShader->ShaderProg && TheRenderManager->renderState->GetVertexShader() != VertexShader->ShaderHandle) VertexShader->ShaderProg->SetCT();
		if (PixelShader->ShaderProg && TheRenderManager->renderState->GetPixelShader() != PixelShader->ShaderHandle) PixelShader->ShaderProg->SetCT();
		if (DWNode::Get()) {
			char Name[256];
			sprintf(Name, "Pass %s (%s %s)", Geometry->m_pcName, VertexShader->ShaderName, PixelShader->ShaderName);
			if (!VertexShader->ShaderProg) strcat(Name, " - Vertex: vanilla");
			if (!PixelShader->ShaderProg) strcat(Name, " - Pixel: vanilla");
			DWNode::AddNode(Name, Geometry->m_parent, Geometry);
		}
		if (TheSettingManager->SettingsMain.Develop.LogShaders && TheKeyboardManager->OnKeyDown(TheSettingManager->SettingsMain.Develop.LogShaders)) Logger::Log("Pass %s (%s %s)", Geometry->m_pcName, VertexShader->ShaderName, PixelShader->ShaderName);
	}
	return (this->*SetupShaderPrograms)(Geometry, SkinInstance, SkinPartition, GeometryBufferData, PropertyState, EffectState, WorldTransform, WorldBound);
	
}

HRESULT (__thiscall RenderHook::* SetSamplerState)(UInt32, D3DSAMPLERSTATETYPE, UInt32, UInt8);
HRESULT (__thiscall RenderHook::* TrackSetSamplerState)(UInt32, D3DSAMPLERSTATETYPE, UInt32, UInt8);
HRESULT RenderHook::TrackSetSamplerState(UInt32 Sampler, D3DSAMPLERSTATETYPE Type, UInt32 Value, UInt8 Save) {

	if (TheSettingManager->SettingsMain.Main.AnisotropicFilter >= 2) {
		if (Type == D3DSAMP_MAGFILTER) {
			if (Value != D3DTEXF_NONE && Value != D3DTEXF_POINT) Value = D3DTEXF_LINEAR;
		}
		if (Type == D3DSAMP_MINFILTER) {
			if (Value != D3DTEXF_NONE && Value != D3DTEXF_POINT) Value = D3DTEXF_ANISOTROPIC;
		}
		if ((Type == D3DSAMP_MIPFILTER) && ((Value == D3DTEXF_POINT) || (Value == D3DTEXF_LINEAR))) {
			Value = D3DTEXF_LINEAR;
		}
	}
	return (this->*SetSamplerState)(Sampler, Type, Value, Save);

}

void (__thiscall RenderHook::* RenderReflections)(NiCamera*, ShadowSceneNode*);
void (__thiscall RenderHook::* TrackRenderReflections)(NiCamera*, ShadowSceneNode*);
void RenderHook::TrackRenderReflections(NiCamera* Camera, ShadowSceneNode* SceneNode) {
	
	D3DXVECTOR4* ShadowData = &TheShaderManager->ShaderConst.Shadow.Data;
	D3DXVECTOR4* ParallaxData = &TheShaderManager->ShaderConst.POM.ParallaxData;
	float ShadowDataBackup = ShadowData->x;
	float ParallaxDataBackup = ParallaxData->y;

	if (DWNode::Get()) DWNode::AddNode("BEGIN REFLECTIONS RENDERING", NULL, NULL);
	ShadowData->x = -1.0f; // Disables the shadows rendering for water reflections (the geo is rendered with the same shaders used in the normal scene!)
	ParallaxData->y = -1.0f; // Disables the additional POM computation (the geo is rendered with the same shaders used in the normal scene!)
	(this->*RenderReflections)(Camera, SceneNode);
	ShadowData->x = ShadowDataBackup;
	ParallaxData->y = ParallaxDataBackup;
	if (DWNode::Get()) DWNode::AddNode("END REFLECTIONS RENDERING", NULL, NULL);

}

void (__thiscall RenderHook::* WaterCullingProcess)(NiAVObject*);
void (__thiscall RenderHook::* TrackWaterCullingProcess)(NiAVObject*);
void RenderHook::TrackWaterCullingProcess(NiAVObject* Object) {
	
	if (Object == Tes->sky->nodeSkyRoot) ProcessReflectionsGeos = false;
	if (Object == SceneNode) ProcessReflectionsGeos = true;
	if (ProcessReflectionsGeos) {
		float Radius = Object->GetWorldBoundRadius();

		if (Radius < 100.0f || Object->m_worldTransform.pos.z + Radius < TheShaderManager->ShaderConst.Water.waterSettings.x) return;
	}
	(this->*WaterCullingProcess)(Object);

}

NiPixelData* (__cdecl * SaveGameScreenshot)(int*, int*) = (NiPixelData* (__cdecl *)(int*, int*))0x00411B70;
NiPixelData* __cdecl TrackSaveGameScreenshot(int* pWidth, int* pHeight) {
	
	NiPixelData* r = NULL;
	
	TheRenderManager->IsSaveGameScreenShot = 1;
	r = SaveGameScreenshot(pWidth, pHeight);
	TheRenderManager->IsSaveGameScreenShot = 0;
	return r;

}

void (__cdecl * SetShaderPackage)(int, int, UInt8, int, char*, int) = (void (__cdecl *)(int, int, UInt8, int, char*, int))0x007B45F0;
void __cdecl TrackSetShaderPackage(int Arg1, int Arg2, UInt8 Force1XShaders, int Arg4, char* GraphicsName, int Arg6) {
	
	UInt32* ShaderPackage = (UInt32*)0x00B42F48;
	UInt32* ShaderPackageMax = (UInt32*)0x00B42D74;

	SetShaderPackage(Arg1, Arg2, Force1XShaders, Arg4, GraphicsName, Arg6);
	
	*ShaderPackage = 7;
	*ShaderPackageMax = 7;

}

void (__cdecl * RenderObject)(NiCamera*, NiNode*, NiCullingProcess*, NiVisibleArray*) = (void (__cdecl *)(NiCamera*, NiNode*, NiCullingProcess*, NiVisibleArray*))0x0070C0B0;
void __cdecl TrackRenderObject(NiCamera* Camera, NiNode* Object, NiCullingProcess* CullingProcess, NiVisibleArray* VisibleArray) {
	
	bool CameraMode = TheSettingManager->SettingsMain.CameraMode.Enabled;

	RenderObject(Camera, Object, CullingProcess, VisibleArray);
	if (Object == WorldSceneGraph && (CameraMode || Player->IsThirdPersonView(CameraMode, TheRenderManager->FirstPersonView))) {
		TheRenderManager->ResolveDepthBuffer();
	}
	else if (Object == Player->firstPersonNiNode) {
		TheRenderManager->ResolveDepthBuffer();
		TheRenderManager->Clear(NULL, NiRenderer::kClear_ZBUFFER);
		RenderObject(Camera, Object, CullingProcess, VisibleArray);
	}

}

#elif defined(SKYRIM)
void (__thiscall RenderHook::* Render)(BSRenderedTexture*, int, int);
void (__thiscall RenderHook::* TrackRender)(BSRenderedTexture*, int, int);
void RenderHook::TrackRender(BSRenderedTexture* RenderedTexture, int Arg2, int Arg3) {
	
	TheRenderManager->SetSceneGraph();
	TheShaderManager->UpdateConstants();
	if (TheSettingManager->SettingsMain.Develop.TraceShaders && TheKeyboardManager->OnKeyDown(TheSettingManager->SettingsMain.Develop.TraceShaders)) Logger::Log("START FRAME LOG");
	if (TheSettingManager->SettingsMain.Develop.LogShaders && TheKeyboardManager->OnKeyDown(TheSettingManager->SettingsMain.Develop.LogShaders)) Logger::Log("START FRAME LOG");
	(this->*Render)(RenderedTexture, Arg2, Arg3);

}

bool (__cdecl * SetupRenderingPass)(UInt32, BSShader*) = (bool (__cdecl *)(UInt32, BSShader*))0x00CAF9C0;
bool __cdecl TrackSetupRenderingPass(UInt32 PassIndex, BSShader* Shader) {
	
	bool r = SetupRenderingPass(PassIndex, Shader);

	NiD3DVertexShaderEx* VertexShader = *(NiD3DVertexShaderEx**)0x01BABFB4;
	NiD3DPixelShaderEx* PixelShader = *(NiD3DPixelShaderEx**)0x01BABFB0;

	if (VertexShader && PixelShader) {
		if (VertexShader->ShaderProg) VertexShader->ShaderProg->SetCT();
		if (PixelShader->ShaderProg) PixelShader->ShaderProg->SetCT();
		if (TheSettingManager->SettingsMain.Develop.LogShaders && TheKeyboardManager->OnKeyDown(TheSettingManager->SettingsMain.Develop.LogShaders)) Logger::Log("Pass %s (%s %s)", Geometry->m_pcName, VertexShader->ShaderName, PixelShader->ShaderName);
	}
	return r;

}

void (__thiscall RenderHook::* RenderWorldSceneGraph)(Sun*, UInt8, UInt8);
void (__thiscall RenderHook::* TrackRenderWorldSceneGraph)(Sun*, UInt8, UInt8);
void RenderHook::TrackRenderWorldSceneGraph(Sun* SkySun, UInt8 IsFirstPerson, UInt8 WireFrame) {
	
	bool CameraMode = TheSettingManager->SettingsMain.CameraMode.Enabled;

	(this->*RenderWorldSceneGraph)(SkySun, IsFirstPerson, WireFrame);
	if (CameraMode || Player->IsThirdPersonView(CameraMode, TheRenderManager->FirstPersonView)) TheRenderManager->ResolveDepthBuffer();

}

void (__thiscall RenderHook::* RenderFirstPerson)(NiDX9Renderer*, NiGeometry*, Sun*, BSRenderedTexture*, UInt8);
void (__thiscall RenderHook::* TrackRenderFirstPerson)(NiDX9Renderer*, NiGeometry*, Sun*, BSRenderedTexture*, UInt8);
void RenderHook::TrackRenderFirstPerson(NiDX9Renderer* Renderer, NiGeometry* Geo, Sun* SkySun, BSRenderedTexture* RenderedTexture, UInt8 IsMultiSample) {
	
	(this->*RenderFirstPerson)(Renderer, Geo, SkySun, RenderedTexture, IsMultiSample);
	TheRenderManager->ResolveDepthBuffer();
	TheRenderManager->Clear(NULL, NiRenderer::kClear_ZBUFFER);
	ThisCall(0x00697E50, Global);
	(this->*RenderFirstPerson)(Renderer, Geo, SkySun, RenderedTexture, IsMultiSample);

}

static __declspec(naked) void RenderingGeometryHook() {

	__asm {
		mov     ebx, [esi]
		mov     ebp, [esi + 8]
		mov     Geometry, ebp
		jmp		kRenderingGeometryReturn
	}

}

void SetShadowDistanceValue(float* Distance, UInt32 Pass) {

	if (Pass == 0) *Distance /= TheSettingManager->SettingsMain.ShadowMode.NearQuality;

}

static __declspec(naked) void SetShadowDistance()
{
	__asm
	{
		faddp   st(1), st
		fstp	dword ptr [esp + 0x4C]
		lea		ecx, [esp + 0x4C]
		mov		edx, [esp + 0x74]
		pushad
		pushfd
		push	edx
		push	ecx
		call	SetShadowDistanceValue
		add		esp, 8
		popfd
		popad
		jmp		kSetShadowDistanceReturn
	}
}

static __declspec(naked) void SetShadowDistanceShader()
{
	__asm
	{
		lea		ecx, [esp + 0xE0 - 0xC4 + 4]
		pushad
		pushfd
		push	0
		push	ecx
		call	SetShadowDistanceValue
		add		esp, 8
		popfd
		popad
		mov		ecx, [esp + 0xE0 - 0xC4 + 4]
		mov		[esp + esi * 4 + 0xE0 - 0x98], ecx
		jmp		kSetShadowDistanceShaderReturn
	}
}
#endif

void (__cdecl * ProcessImageSpaceShaders)(NiDX9Renderer*, BSRenderedTexture*, BSRenderedTexture*) = (void (__cdecl *)(NiDX9Renderer*, BSRenderedTexture*, BSRenderedTexture*))kProcessImageSpaceShaders;
void __cdecl TrackProcessImageSpaceShaders(NiDX9Renderer* Renderer, BSRenderedTexture* RenderedTexture1, BSRenderedTexture* RenderedTexture2) {
	
	BSRenderedTexture* MenuRenderedTexture = *(BSRenderedTexture**)kMenuRenderedTexture;
	
	if (TheRenderManager->BackBuffer) TheRenderManager->defaultRTGroup->RenderTargets[0]->data->Surface = TheRenderManager->BackBuffer;
	ProcessImageSpaceShaders(Renderer, RenderedTexture1, RenderedTexture2);
	if ((!RenderedTexture2 || MenuRenderedTexture) && TheRenderManager->currentRTGroup) TheShaderManager->RenderEffects(TheRenderManager->currentRTGroup->RenderTargets[0]->data->Surface);
	if (TheRenderManager->IsSaveGameScreenShot) {
		if (MenuRenderedTexture)
			TheRenderManager->device->StretchRect(MenuRenderedTexture->RenderTargetGroup->RenderTargets[0]->data->Surface, NULL, TheRenderManager->currentRTGroup->RenderTargets[0]->data->Surface, &TheRenderManager->SaveGameScreenShotRECT, D3DTEXF_NONE);
		else
			TheRenderManager->device->StretchRect(TheRenderManager->defaultRTGroup->RenderTargets[0]->data->Surface, NULL, TheRenderManager->currentRTGroup->RenderTargets[0]->data->Surface, &TheRenderManager->SaveGameScreenShotRECT, D3DTEXF_NONE);
	}

}

static __declspec(naked) void RenderInterface() {
	
	__asm {
#if defined(NEWVEGAS)
		pushad
		call	RenderMainMenuMovie
		popad
#endif
		call	kRenderInterfaceMethod
		pushad
		mov		ecx, TheGameMenuManager
		call	GameMenuManager::Render
		popad
		jmp		kRenderInterfaceReturn
	}

}

#if defined(NEWVEGAS) || defined(OBLIVION)
#if defined(OBLIVION)
void DetectorWindowSetNodeName(char* Buffer, char* Format, char* ClassName, char* Name, float LPosX, float LPosY, float LPosZ) {
	
	sprintf(Buffer, "%s", Name);

}
#elif defined(NEWVEGAS)
void DetectorWindowSetNodeName(char* Buffer, int Size, char* Format, char* ClassName, char* Name, float LPosX, float LPosY, float LPosZ) {

	sprintf(Buffer, "%s", Name);

}
#endif

void DetectorWindowCreateTreeView(HWND TreeView) {
	
	HFONT Font = CreateFontA(14, 0, 0, 0, FW_DONTCARE, NULL, NULL, NULL, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
	SendMessageA(TreeView, WM_SETFONT, (WPARAM)Font, TRUE);
	SendMessageA(TreeView, TVM_SETBKCOLOR, NULL, 0x001E1E1E);
	SendMessageA(TreeView, TVM_SETTEXTCOLOR, NULL, 0x00DCDCDC);
	
}

static __declspec(naked) void DetectorWindowCreateTreeViewHook() {

	__asm {
		pushad
		push	eax
		call	DetectorWindowCreateTreeView
		pop		eax
		popad
#if defined(OBLIVION)
		pop     edi
		mov		[esi + 0x0C], eax
		pop     esi
		add     esp, 0x40
#elif defined(NEWVEGAS)
		mov     ecx, [ebp - 0x48]
		mov		[ecx + 0x0C], eax
		mov     esp, ebp
		pop     ebp
#endif
		jmp		kDetectorWindowCreateTreeViewReturn
	}

}

void DetectorWindowDumpAttributes(HWND TreeView, UInt32 Msg, WPARAM wParam, LPTVINSERTSTRUCTA lParam) {

	TVITEMEXA Item;
	char T[260] = { '\0' };

	Item.pszText = T;
	Item.mask = TVIF_TEXT;
	Item.hItem = (HTREEITEM)SendMessageA(TreeView, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)lParam->hParent);
	Item.cchTextMax = 260;
	SendMessageA(TreeView, TVM_GETITEMA, 0, (LPARAM)&Item);
	if (!memcmp(Item.pszText, "Pass", 4))
		SendMessageA(TreeView, TVM_DELETEITEM, 0, (LPARAM)lParam->hParent);
	else
		if (strlen(Item.pszText)) SendMessageA(TreeView, Msg, wParam, (LPARAM)lParam);

}

static __declspec(naked) void DetectorWindowDumpAttributesHook() {

	__asm {
		call	DetectorWindowDumpAttributes
		add		esp, 16
#if defined(OBLIVION)
		movzx   ecx, word ptr [esi + 0x0A]
#endif
		jmp		kDetectorWindowDumpAttributesReturn
	}

}
#endif

void CreateRenderHook() {

	*((int*)&Render)						= kRender;
	TrackRender								= &RenderHook::TrackRender;
	*((int*)&BeginScene)					= kBeginScene;
	TrackBeginScene							= &RenderHook::TrackBeginScene;
#if defined(NEWVEGAS)
	*((int*)&ShowDetectorWindow)			= 0x004D61B0;
	TrackShowDetectorWindow					= &RenderHook::TrackShowDetectorWindow;
	*((int*)&RenderWorldSceneGraph)			= 0x00873200;
	TrackRenderWorldSceneGraph				= &RenderHook::TrackRenderWorldSceneGraph;
	*((int*)&RenderFirstPerson)				= 0x00875110;
	TrackRenderFirstPerson					= &RenderHook::TrackRenderFirstPerson;
	*((int*)&RenderPipboyScreen)			= 0x008761E0;
	TrackRenderPipboyScreen					= &RenderHook::TrackRenderPipboyScreen;
	*((int*)&GetWaterHeightLOD)				= 0x0045CD80;
	TrackGetWaterHeightLOD					= &RenderHook::TrackGetWaterHeightLOD;
#elif defined(OBLIVION)
	*((int*)&ShowDetectorWindow)			= 0x00496CB0;
	TrackShowDetectorWindow					= &RenderHook::TrackShowDetectorWindow;
	*((int*)&SetupShaderPrograms)			= 0x0077A1F0;
	TrackSetupShaderPrograms				= &RenderHook::TrackSetupShaderPrograms;
	*((int*)&EndTargetGroup)				= 0x007AAA30;
	TrackEndTargetGroup						= &RenderHook::TrackEndTargetGroup;
	*((int*)&HDRRender)						= 0x007BDFC0;
	TrackHDRRender							= &RenderHook::TrackHDRRender;
	*((int*)&BSFadeNodeOnVisible)			= 0x004A0920;
	TrackBSFadeNodeOnVisible				= &RenderHook::TrackBSFadeNodeOnVisible;
	*((int*)&FarPlane)						= 0x00410EE0;
	TrackFarPlane							= &RenderHook::TrackFarPlane;
	*((int*)&SetSamplerState)				= 0x0077B610;
	TrackSetSamplerState					= &RenderHook::TrackSetSamplerState;
	*((int*)&RenderReflections)				= 0x0049BEF0;
	TrackRenderReflections					= &RenderHook::TrackRenderReflections;
	*((int*)&WaterCullingProcess)			= 0x0049CBF0;
	TrackWaterCullingProcess				= &RenderHook::TrackWaterCullingProcess;
#elif defined(SKYRIM)
	*((int*)&RenderWorldSceneGraph)			= 0x00692290;
	TrackRenderWorldSceneGraph				= &RenderHook::TrackRenderWorldSceneGraph;
	*((int*)&RenderFirstPerson)				= 0x00698B30;
	TrackRenderFirstPerson					= &RenderHook::TrackRenderFirstPerson;
#endif

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)Render,						*((PVOID*)&TrackRender));
	DetourAttach(&(PVOID&)BeginScene,					*((PVOID*)&TrackBeginScene));
	DetourAttach(&(PVOID&)ProcessImageSpaceShaders,				  &TrackProcessImageSpaceShaders);
#if defined(NEWVEGAS)
	DetourAttach(&(PVOID&)ShowDetectorWindow,			*((PVOID*)&TrackShowDetectorWindow));
	DetourAttach(&(PVOID&)RenderWorldSceneGraph,		*((PVOID*)&TrackRenderWorldSceneGraph));
	DetourAttach(&(PVOID&)RenderFirstPerson,			*((PVOID*)&TrackRenderFirstPerson));
	DetourAttach(&(PVOID&)RenderPipboyScreen,			*((PVOID*)&TrackRenderPipboyScreen));
	DetourAttach(&(PVOID&)GetWaterHeightLOD,			*((PVOID*)&TrackGetWaterHeightLOD));
	DetourAttach(&(PVOID&)SetupRenderingPass,					  &TrackSetupRenderingPass);
	DetourAttach(&(PVOID&)SetShaderPackage,						  &TrackSetShaderPackage);
#elif defined(OBLIVION)
	DetourAttach(&(PVOID&)ShowDetectorWindow,			*((PVOID*)&TrackShowDetectorWindow));
	DetourAttach(&(PVOID&)SetupShaderPrograms,			*((PVOID*)&TrackSetupShaderPrograms));
	DetourAttach(&(PVOID&)EndTargetGroup,				*((PVOID*)&TrackEndTargetGroup));
	DetourAttach(&(PVOID&)HDRRender,					*((PVOID*)&TrackHDRRender));
	DetourAttach(&(PVOID&)BSFadeNodeOnVisible,			*((PVOID*)&TrackBSFadeNodeOnVisible));
	DetourAttach(&(PVOID&)FarPlane,						*((PVOID*)&TrackFarPlane));
	DetourAttach(&(PVOID&)SetSamplerState,				*((PVOID*)&TrackSetSamplerState));
	DetourAttach(&(PVOID&)RenderReflections,			*((PVOID*)&TrackRenderReflections));
	DetourAttach(&(PVOID&)WaterCullingProcess,			*((PVOID*)&TrackWaterCullingProcess));
	DetourAttach(&(PVOID&)SaveGameScreenshot,					  &TrackSaveGameScreenshot);
	DetourAttach(&(PVOID&)SetShaderPackage,						  &TrackSetShaderPackage);
	DetourAttach(&(PVOID&)RenderObject,							  &TrackRenderObject);
#elif defined(SKYRIM)
	DetourAttach(&(PVOID&)RenderWorldSceneGraph,		*((PVOID*)&TrackRenderWorldSceneGraph));
	DetourAttach(&(PVOID&)RenderFirstPerson,			*((PVOID*)&TrackRenderFirstPerson));
	DetourAttach(&(PVOID&)SetupRenderingPass,					  &TrackSetupRenderingPass);
#endif
    DetourTransactionCommit();

	WriteRelJump(kRenderInterface,				(UInt32)RenderInterface);
#if defined(NEWVEGAS) || defined(OBLIVION)
	WriteRelCall(kDetectorWindowSetNodeName,    (UInt32)DetectorWindowSetNodeName);
	WriteRelJump(kDetectorWindowCreateTreeView, (UInt32)DetectorWindowCreateTreeViewHook);
	WriteRelJump(kDetectorWindowDumpAttributes, (UInt32)DetectorWindowDumpAttributesHook);
	WriteRelJump(kDetectorWindowScale,			kDetectorWindowScaleReturn); // Avoids to add the scale to the node description in the detector window
#endif
#if defined(NEWVEGAS)
	WriteRelJump(0x004E4C3B, 0x004E4C42); // Fixes reflections when cell water height is not like worldspace water height
	WriteRelJump(0x004E4DA4, 0x004E4DAC); // Fixes reflections on distant water
	WriteRelCall(0x00875B86, 0x00710AB0); // Sets the world fov at the end of 1st person rendering instead of 1st person fov
	WriteRelCall(0x00875B9D, 0x00710AB0); // Sets the world fov at the end of 1st person rendering instead of 1st person fov
	WriteRelCall(kMultiBoundWaterHeightFix1,	(UInt32)MultiBoundWaterHeightFix);
	WriteRelCall(kMultiBoundWaterHeightFix2,	(UInt32)MultiBoundWaterHeightFix);
	if (TheSettingManager->SettingsMain.Main.ReplaceIntro) WriteRelJump(kSetTileShaderConstants, (UInt32)SetTileShaderConstants);
	if (TheSettingManager->SettingsMain.Main.RemovePrecipitations) {
		WriteRelJump(0x0063AFC4, 0x0063AFD8);
		WriteRelJump(0x0063A5CB, 0x0063A5DE);
	}
	SafeWrite8(0x008751C0, 0); // Stops to clear the depth buffer when rendering the 1st person node
#elif defined(OBLIVION)
	SafeWrite32(0x0049BFAF, TheSettingManager->SettingsMain.Main.WaterReflectionMapSize); // Constructor
	SafeWrite32(0x007C1045, TheSettingManager->SettingsMain.Main.WaterReflectionMapSize); // RenderedSurface
	SafeWrite32(0x007C104F, TheSettingManager->SettingsMain.Main.WaterReflectionMapSize); // RenderedSurface
	SafeWrite32(0x007C10F9, TheSettingManager->SettingsMain.Main.WaterReflectionMapSize); // RenderedSurface
	SafeWrite32(0x007C1103, TheSettingManager->SettingsMain.Main.WaterReflectionMapSize); // RenderedSurface
	SafeWrite8(0x00A38280, 0x5A); // Fixes the "purple water bug"
	SafeWrite8(0x0040CE11, 0); // Stops to clear the depth buffer when rendering the 1st person node
	WriteRelJump(0x00553EAC, 0x00553EB2); // Patches the use of Lighting30Shader only for the hair
	WriteRelJump(0x007D1BC4, 0x007D1BFD); // Patches the use of Lighting30Shader only for the hair
	WriteRelJump(0x007D1BCD, 0x007D1BFD); // Patches the use of Lighting30Shader only for the hair
	WriteRelJump(0x0049C3A2, 0x0049C41D); // Avoids to manage the cells culling for reflections
	WriteRelJump(0x0049C8CB, 0x0049C931); // Avoids to manage the cells culling for reflections
	if (TheSettingManager->SettingsMain.Main.AnisotropicFilter >= 2) {
		SafeWrite8(0x007BE1D3, TheSettingManager->SettingsMain.Main.AnisotropicFilter);
		SafeWrite8(0x007BE32B, TheSettingManager->SettingsMain.Main.AnisotropicFilter);
	}
	if (TheSettingManager->SettingsMain.Main.RemovePrecipitations) WriteRelJump(0x00543167, 0x00543176);
	*LocalWaterHiRes = 1; // Fixes a bug on the WaterHeightMapConstructor
#elif defined(SKYRIM)
	WriteRelJump(kRenderingGeometry,		(UInt32)RenderingGeometryHook);
	if (TheSettingManager->SettingsMain.ShadowMode.NearQuality) {
		WriteRelJump(kSetShadowDistance,		(UInt32)SetShadowDistance);
		WriteRelJump(kSetShadowDistanceShader,	(UInt32)SetShadowDistanceShader);
	}
	SafeWrite8(0x00698BBB, 0); // Stops to clear the depth buffer when rendering the 1st person node
#endif

}

#include "OcclusionManager.h"

#if defined(NEWVEGAS)
#define RenderStateArgs 0, 0
#elif defined(OBLIVION)
#define RenderStateArgs 0
#define kFormType_MoveableStatic kFormType_Stat
static const UInt32 kNew1CollisionObjectHook = 0x00564529;
static const UInt32 kNew1CollisionObjectReturn = 0x0056452E;
static const UInt32 kNew2CollisionObjectHook = 0x0089E989;
static const UInt32 kNew2CollisionObjectReturn = 0x0089E98E;
static const UInt32 kNew3CollisionObjectHook = 0x0089EA1C;
static const UInt32 kNew3CollisionObjectReturn = 0x0089EA21;
static const UInt32 kDisposeCollisionObjectHook = 0x00532DD1;
static const UInt32 kDisposeCollisionObjectReturn = 0x00532DD8;
static const UInt32 kMaterialPropertyHook = 0x0089F7C6;
static const UInt32 kMaterialPropertyReturn1 = 0x0089F7CE;
static const UInt32 kMaterialPropertyReturn2 = 0x0089F8A0;
static const UInt32 kCoordinateJackHook = 0x008A3101;
static const UInt32 kCoordinateJackReturn1 = 0x008A3107;
static const UInt32 kCoordinateJackReturn2 = 0x008A3165;
static const UInt32 kObjectCullHook = 0x007073D6;
static const UInt32 kObjectCullReturn1 = 0x007073DC;
static const UInt32 kObjectCullReturn2 = 0x007073E7;
static const void* VFTNiNode = (void*)0x00A7E38C;
static const void* VFTBSFadeNode = (void*)0x00A3F944;
static const void* VFTNiTriShape = (void*)0x00A7ED5C;
static const void* VFTNiTriStrips = (void*)0x00A7F27C;
static const void* VFTbhkCollisionObject = (void*)0x00A55FCC;
#endif

OcclusionManager::OcclusionManager() {
	
	Logger::Log("Starting the occlusion manager...");
	TheOcclusionManager = this;
	
	Tex = NULL;
	WaterOccluded = false;

	IDirect3DDevice9* Device = TheRenderManager->device;
	UINT OcclusionMapSizeX = TheRenderManager->width / TheSettingManager->SettingsMain.OcclusionCulling.OcclusionMapRatio;
	UINT OcclusionMapSizeY = TheRenderManager->height / TheSettingManager->SettingsMain.OcclusionCulling.OcclusionMapRatio;

	OcclusionMapVertex = new ShaderRecord();
	if (OcclusionMapVertex->LoadShader("OcclusionMap.vso")) Device->CreateVertexShader((const DWORD*)OcclusionMapVertex->Function, &OcclusionMapVertexShader);
	OcclusionMapPixel = new ShaderRecord();
	if (OcclusionMapPixel->LoadShader("OcclusionMap.pso")) Device->CreatePixelShader((const DWORD*)OcclusionMapPixel->Function, &OcclusionMapPixelShader);

	Device->CreateQuery(D3DQUERYTYPE_OCCLUSION, &OcclusionQuery);
	Device->CreateTexture(OcclusionMapSizeX, OcclusionMapSizeY, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &OcclusionMapTexture, NULL);
	OcclusionMapTexture->GetSurfaceLevel(0, &OcclusionMapSurface);
	Device->CreateDepthStencilSurface(OcclusionMapSizeX, OcclusionMapSizeY, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, true, &OcclusionMapDepthSurface, NULL);
	OcclusionMapViewPort = { 0, 0, OcclusionMapSizeX, OcclusionMapSizeY, 0.0f, 1.0f };

}

bool OcclusionManager::InFrustum(NiNode* Node) {
	
	NiCullingProcess* Process = WorldSceneGraph->cullingProcess;
	UInt32 ActivePlanes = Process->Planes.ActivePlanes;
	NiBound* Bound = Node->GetWorldBound();
	UInt32 Side = 0;
	UInt32 i = 0;
	bool Result = false;

	if (ActivePlanes > 0) {
		for (i = 0; i < NiFrustumPlanes::MaxPlanes; i++) {
			if (ActivePlanes & (1 << i)) {
				Side = Bound->WhichSide(&Process->Planes.CullingPlanes[i]);
				if (Side == NiPlane::NegativeSide) break;
				if (Side == NiPlane::PositiveSide) Process->Planes.ActivePlanes &= ~(1 << i);
			}
		}
		if (i == NiFrustumPlanes::MaxPlanes) Result = true;
		Process->Planes.ActivePlanes = ActivePlanes;
	}
	return Result;

}

TESObjectREFR* OcclusionManager::GetRef(TESObjectREFR* Ref) {
	
	TESObjectREFR* R = NULL;

	if (Ref && Ref->GetNode()) {
		TESForm* Form = Ref->baseForm;
		UInt8 TypeID = Form->formType;
		if ((TypeID == TESForm::FormType::kFormType_Activator && 0) ||
			(TypeID == TESForm::FormType::kFormType_Apparatus && 0) ||
			(TypeID == TESForm::FormType::kFormType_Book && 0) ||
			(TypeID == TESForm::FormType::kFormType_Container && 0) ||
			(TypeID == TESForm::FormType::kFormType_Door && 0) ||
			(TypeID == TESForm::FormType::kFormType_Misc && 0) ||
			(TypeID >= TESForm::FormType::kFormType_Stat && TypeID <= TESForm::FormType::kFormType_MoveableStatic && 1) ||
			(TypeID == TESForm::FormType::kFormType_Furniture && 0) ||
			(TypeID >= TESForm::FormType::kFormType_NPC && TypeID <= TESForm::FormType::kFormType_LeveledCreature && 0))
			R = Ref;
	}
	return R;

}

void OcclusionManager::RenderStatic(NiAVObject* Object, float MinBoundSize, float MaxBoundSize, bool PerformOcclusion) {
	
	DWORD Pixels = 0;
	NiPoint2 BoundSize;
	NiBound* Bound = NULL;
	float BoundBox = 0.0f;

	if (Object) {
		Bound = Object->GetWorldBound();
		TheRenderManager->GetScreenSpaceBoundSize(&BoundSize, Bound);
		BoundBox = (BoundSize.x * 100.f) * (BoundSize.y * 100.0f);
		if (!(Object->m_flags & NiAVObject::kFlag_AppCulled) && BoundBox >= MinBoundSize && BoundBox <= MaxBoundSize && Object->m_worldTransform.pos.z + Bound->Radius > TheShaderManager->ShaderConst.Water.waterSettings.x) {
			void* VFT = *(void**)Object;
			if (VFT == VFTNiNode || VFT == VFTBSFadeNode) {
				if (VFT == VFTBSFadeNode && ((BSFadeNode*)Object)->FadeAlpha < 0.9f) return;
				NiNode* Node = (NiNode*)Object;
				if (bhkCollisionObjectEx* CollisionObject = (bhkCollisionObjectEx*)Node->m_spCollision) {
					VFT = *(void**)CollisionObject;
					if (VFT == VFTbhkCollisionObject) {
						if (!CollisionObject->GeoNode) {
							if (bhkRigidBody* RigidBody = CollisionObject->bRigidBody) {
								NiNode* GeoNode = (NiNode*)MemoryAlloc(sizeof(NiNode)); GeoNode->New(1); GeoNode->m_flags |= NiAVObject::kFlag_IsOCNode;
								Node->AddObject(GeoNode, 1);
								GeoNode->Update();
								RigidBody->CreateNiGeometry(GeoNode);
								GeoNode->Update();
								Node->RemoveObject((NiAVObject**)&GeoNode, GeoNode);
								CollisionObject->GeoNode = GeoNode;
							}
						}
						if (NiNode* GeoNode = CollisionObject->GeoNode) {
							for (int i = 0; i < GeoNode->m_children.end; i++) {
								NiGeometry* Geo = (NiGeometry*)GeoNode->m_children.data[i];
								if (!Geo->geomData->BuffData) TheRenderManager->unsharedGeometryGroup->AddObject(Geo->geomData, NULL, NULL);
								if (PerformOcclusion) OcclusionQuery->Issue(D3DISSUE_BEGIN);
								Render(Geo);
								if (PerformOcclusion) {
									OcclusionQuery->Issue(D3DISSUE_END);
									while (OcclusionQuery->GetData((void*)&Pixels, sizeof(DWORD), D3DGETDATA_FLUSH) == S_FALSE);
									if (Pixels <= 10)
										Geo->m_flags |= NiAVObject::kFlag_IsOccluded;
									else
										Geo->m_flags &= ~NiAVObject::kFlag_IsOccluded;
								}
							}
						}
					}
				}
				else {
					for (int i = 0; i < Node->m_children.end; i++) {
						RenderStatic(Node->m_children.data[i], MinBoundSize, MaxBoundSize, PerformOcclusion);
					}
				}
			}
		}
	}

}

void OcclusionManager::RenderTerrain(NiAVObject* Object) {

	if (Object && !(Object->m_flags & NiAVObject::kFlag_AppCulled)) {
		void* VFT = *(void**)Object;
		if (VFT == VFTNiNode) {
			NiNode* Node = (NiNode*)Object;
			if (InFrustum(Node)) {
				for (int i = 0; i < Node->m_children.end; i++) {
					RenderTerrain(Node->m_children.data[i]);
				}
			}
		}
		else if (VFT == VFTNiTriShape || VFT == VFTNiTriStrips) {
			NiGeometry* Geo = (NiGeometry*)Object;
			if (Geo->geomData->BuffData) Render(Geo);
		}
	}

}

void OcclusionManager::RenderWater(NiAVObject* Object) {
	
	DWORD Pixels = 0;

	if (Object) {
		if (!(Object->m_flags & NiAVObject::kFlag_AppCulled)) {
			void* VFT = *(void**)Object;
			if (VFT == VFTNiNode) {
				NiNode* Node = (NiNode*)Object;
				if (InFrustum(Node)) {
					for (int i = 0; i < Node->m_children.end; i++) {
						RenderWater(Node->m_children.data[i]);
					}
				}
			}
			else if (VFT == VFTNiTriShape || VFT == VFTNiTriStrips) {
				NiGeometry* Geo = (NiGeometry*)Object;
				if (Geo->geomData->BuffData) {
					OcclusionQuery->Issue(D3DISSUE_BEGIN);
					Render(Geo);
					OcclusionQuery->Issue(D3DISSUE_END);
					while (OcclusionQuery->GetData((void*)&Pixels, sizeof(DWORD), D3DGETDATA_FLUSH) == S_FALSE);
					if (Pixels <= 10) {
						Geo->m_flags |= NiAVObject::kFlag_IsOccluded;
					}
					else {
						WaterOccluded = false;
						Geo->m_flags &= ~NiAVObject::kFlag_IsOccluded;
					}
				}
			}
		}
	}

}

void OcclusionManager::Render(NiGeometry* Geo) {

	IDirect3DDevice9* Device = TheRenderManager->device;
	NiDX9RenderState* RenderState = TheRenderManager->renderState;
	int StartIndex = 0;
	int PrimitiveCount = 0;
	int StartRegister = 9;
	NiGeometryData* ModelData = Geo->geomData;
	NiGeometryBufferData* GeoData = ModelData->BuffData;
	NiD3DShaderDeclaration* ShaderDeclaration = (Geo->shader ? Geo->shader->ShaderDeclaration : NULL);
	D3DXMATRIX WorldMatrix;

	TheRenderManager->CreateD3DMatrix(&WorldMatrix, &Geo->m_worldTransform);
	D3DXMatrixMultiplyTranspose(&TheShaderManager->ShaderConst.OcclusionMap.OcclusionWorldViewProj, &WorldMatrix, &TheRenderManager->ViewProjMatrix);
	BSShaderProperty* ShaderProperty = (BSShaderProperty*)Geo->GetProperty(NiProperty::PropertyType::kType_Shade);
	if (ShaderProperty) {
		if (ShaderProperty->IsLightingProperty()) {
			if (NiTexture* Texture = *((BSShaderPPLightingProperty*)ShaderProperty)->textures[0]) { // Only for testing, remove the diffuse, we do not need it.
				RenderState->SetTexture(0, Texture->rendererData->dTexture);
				RenderState->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP, false);
				RenderState->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP, false);
				RenderState->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT, false);
				RenderState->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT, false);
				RenderState->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT, false);
			}
		}
		else if (ShaderProperty->IsWaterProperty()) {
			if (!Tex) D3DXCreateTextureFromFileA(TheRenderManager->device, "C:\\Bethesda Softworks\\Oblivion\\Data\\Textures\\Water\\water00.dds", &Tex); // Only for testing, remove the diffuse, we do not need it.
			RenderState->SetTexture(0, Tex);
			RenderState->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP, false);
			RenderState->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP, false);
			RenderState->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT, false);
			RenderState->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT, false);
			RenderState->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT, false);
		}
		else {
			return;
		}
	}
	TheRenderManager->PackGeometryBuffer(GeoData, ModelData, NULL, ShaderDeclaration);
	for (UInt32 i = 0; i < GeoData->StreamCount; i++) {
		Device->SetStreamSource(i, GeoData->VBChip[i]->VB, 0, GeoData->VertexStride[i]);
	}
	Device->SetIndices(GeoData->IB);
	if (GeoData->FVF)
		Device->SetFVF(GeoData->FVF);
	else
		Device->SetVertexDeclaration(GeoData->VertexDeclaration);
	OcclusionMapVertex->SetCT();
	OcclusionMapPixel->SetCT();
	for (UInt32 i = 0; i < GeoData->NumArrays; i++) {
		if (GeoData->ArrayLengths)
			PrimitiveCount = GeoData->ArrayLengths[i] - 2;
		else
			PrimitiveCount = GeoData->TriCount;
		Device->DrawIndexedPrimitive(GeoData->PrimitiveType, GeoData->BaseVertexIndex, 0, GeoData->VertCount, StartIndex, PrimitiveCount);
		StartIndex += PrimitiveCount + 2;
	}

}

void OcclusionManager::RenderOcclusionMap() {

	NiNode* WaterRoot = *(NiNode**)0x00B35230;
	IDirect3DDevice9* Device = TheRenderManager->device;
	NiDX9RenderState* RenderState = TheRenderManager->renderState;
	GridCellArray* CellArray = Tes->gridCellArray;
	SettingsMainStruct::OcclusionCullingStruct* OcclusionCulling = &TheSettingManager->SettingsMain.OcclusionCulling;
	float OccludingStaticMin = OcclusionCulling->OccludingStaticMin;
	float OccludingStaticMax = OcclusionCulling->OccludingStaticMax;
	float OccludedStaticMin = OcclusionCulling->OccludedStaticMin;
	float OccludedStaticMax = OcclusionCulling->OccludedStaticMax;

	Device->SetRenderTarget(0, OcclusionMapSurface);
	Device->SetDepthStencilSurface(OcclusionMapDepthSurface);
	Device->SetViewport(&OcclusionMapViewPort);
	Device->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0L);
	RenderState->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE, RenderStateArgs);
	RenderState->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE, RenderStateArgs);
	RenderState->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE, RenderStateArgs);
	RenderState->SetRenderState(D3DRS_ALPHABLENDENABLE, 0, RenderStateArgs);
	RenderState->SetVertexShader(OcclusionMapVertexShader, false);
	RenderState->SetPixelShader(OcclusionMapPixelShader, false);
	Device->BeginScene();
	//RenderState->SetRenderState(D3DRS_COLORWRITEENABLE, D3DZB_FALSE, RenderStateArgs);
	for (UInt32 i = 0; i < CellArray->size * CellArray->size; i++) {
		TESObjectCELL* Cell = CellArray->grid[i].cell;
		NiNode* CellNode = Cell->niNode;
		for (int i = 2; i < 6; i++) {
			NiNode* ChildNode = (NiNode*)CellNode->m_children.data[i];
			if (ChildNode->m_children.end) {
				RenderTerrain(ChildNode->m_children.data[0]);
			}
		}
		if (OcclusionCulling->OccludingStatic) {
			TList<TESObjectREFR>::Entry* Entry = &Cell->objectList.First;
			while (Entry) {
				if (TESObjectREFR* Ref = GetRef(Entry->item)) {
					NiNode* RefNode = Ref->GetNode();
					if (InFrustum(RefNode)) RenderStatic(RefNode, OccludingStaticMin, OccludingStaticMax, false);
				}
				Entry = Entry->next;
			}
		}
	}
	RenderState->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_FALSE, RenderStateArgs);
	WaterOccluded = true;
	RenderWater(WaterRoot);
	if (OcclusionCulling->OccludedStatic) {
		for (UInt32 i = 0; i < CellArray->size * CellArray->size; i++) {
			TESObjectCELL* Cell = CellArray->grid[i].cell;
			TList<TESObjectREFR>::Entry* Entry = &Cell->objectList.First;
			while (Entry) {
				if (TESObjectREFR* Ref = GetRef(Entry->item)) {
					NiNode* RefNode = Ref->GetNode();
					if (InFrustum(RefNode)) RenderStatic(RefNode, OccludedStaticMin, OccludedStaticMax, true);
				}
				Entry = Entry->next;
			}
		}
	}
	Device->EndScene();

}

void OcclusionManager::PerformOcclusionCulling() {

	IDirect3DDevice9* Device = TheRenderManager->device;
	IDirect3DSurface9* DepthSurface = NULL;

	if (Player->GetWorldSpace()) {
		Device->GetDepthStencilSurface(&DepthSurface);
		TheRenderManager->SetupSceneCamera();
		RenderOcclusionMap();
		Device->SetDepthStencilSurface(DepthSurface);
	}

	if (TheKeyboardManager->OnKeyDown(26)) {
		D3DXSaveSurfaceToFileA("C:\\Archivio\\Downloads\\occlusionmap.jpg", D3DXIFF_JPG, OcclusionMapSurface, NULL, NULL);
	}

}

static __declspec(naked) void New1CollisionObjectHook() {

	__asm {
		mov     edi, eax
		add     esp, 4
		mov		dword ptr [edi + bhkCollisionObjectEx::GeoNode], 0
		jmp		kNew1CollisionObjectReturn
	}

}

static __declspec(naked) void New2CollisionObjectHook() {

	__asm {
		mov     esi, eax
		add     esp, 4
		mov		dword ptr [esi + bhkCollisionObjectEx::GeoNode], 0
		jmp		kNew2CollisionObjectReturn
	}

}

static __declspec(naked) void New3CollisionObjectHook() {

	__asm {
		mov     esi, eax
		add     esp, 4
		mov		dword ptr [esi + bhkCollisionObjectEx::GeoNode], 0
		jmp		kNew3CollisionObjectReturn
	}

}

void DisposeCollisionObject(bhkCollisionObjectEx* bCollisionObject) {
	
	void* VFT = *(void**)bCollisionObject;
	
	if (VFT == VFTbhkCollisionObject) {
		if (NiNode* GeoNode = bCollisionObject->GeoNode) {
			GeoNode->Destructor(1);
			bCollisionObject->GeoNode = NULL;
		}
	}

}

static __declspec(naked) void DisposeCollisionObjectHook() {

	__asm {
		pushad
		push	ecx
		call	DisposeCollisionObject
		pop		ecx
		popad
		mov     esi, ecx
		mov		eax, 0x00897B00
		call	eax
		jmp		kDisposeCollisionObjectReturn
	}

}

static __declspec(naked) void MaterialPropertyHook() {

	__asm {
		test    word ptr [esi + 0x18], 0x400
		jnz     short loc_return
		test    edi, edi
		fldz
		fst		[esp + 0x18]
		jmp		kMaterialPropertyReturn1
	loc_return:
		jmp		kMaterialPropertyReturn2
	}

}

static __declspec(naked) void CoordinateJackHook() {

	__asm {
		test    word ptr [eax + 0x18], 0x400
		jnz     short loc_return
		mov     esi, eax
		jmp		kCoordinateJackReturn1
	loc_return:
		jmp		kCoordinateJackReturn2
	}

}

static __declspec(naked) void ObjectCullHook() {

	__asm {
		test    word ptr [ecx + 0x18], 0x200
		jnz     short loc_return
		mov     eax, [esp + 0x04]
		mov     edx, [eax]
		jmp		kObjectCullReturn1
	loc_return:
		jmp		kObjectCullReturn2
	}

}

void CreateOcclusionCullingHook() {

	UInt32 bhkCollisionObjectExSize = sizeof(bhkCollisionObjectEx);
	SafeWrite8(0x00564523, bhkCollisionObjectExSize);
	SafeWrite8(0x0089E983, bhkCollisionObjectExSize);
	SafeWrite8(0x0089EA16, bhkCollisionObjectExSize);

	WriteRelJump(kNew1CollisionObjectHook,		(UInt32)New1CollisionObjectHook);
	WriteRelJump(kNew2CollisionObjectHook,		(UInt32)New2CollisionObjectHook);
	WriteRelJump(kNew3CollisionObjectHook,		(UInt32)New3CollisionObjectHook);
	WriteRelJump(kDisposeCollisionObjectHook,	(UInt32)DisposeCollisionObjectHook);
	WriteRelJump(kMaterialPropertyHook,			(UInt32)MaterialPropertyHook);
	WriteRelJump(kCoordinateJackHook,			(UInt32)CoordinateJackHook);
	WriteRelJump(kObjectCullHook,				(UInt32)ObjectCullHook);
	
}
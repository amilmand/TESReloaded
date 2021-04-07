#define DEBUGSH 0
#include <algorithm>

#if defined(NEWVEGAS)
#define RenderStateArgs 0, 0
#define kRockParams 0x01200658
#define kRustleParams 0x01200668
#define kWindMatrixes 0x01200688
static const UInt32 kRenderShadowMapHook = 0x00870C39;
static const UInt32 kRenderShadowMapReturn = 0x00870C41;
static const UInt32 kAddCastShadowFlagHook = 0x0050DD06;
static const UInt32 kAddCastShadowFlagReturn = 0x0050DD0B;
static const UInt32 kEditorCastShadowFlagHook = 0x00000000;
static const UInt32 kEditorCastShadowFlagReturn = 0x00000000;
static const UInt32 kLeavesNodeName = 0x0066A115;
static const UInt32 kLeavesNodeNameReturn = 0x0066A11E;
static const void* VFTNiNode = (void*)0x0109B5AC;
static const void* VFTBSFadeNode = (void*)0x010A8F90;
static const void* VFTBSFaceGenNiNode = (void*)0x010660DC;
static const void* VFTBSTreeNode = (void*)0x010668E4;
static const void* VFTNiTriShape = (void*)0x0109D454;
static const void* VFTNiTriStrips = (void*)0x0109CD44;
#elif defined(OBLIVION)
#define RenderStateArgs 0
#define kFormType_MoveableStatic kFormType_Stat
#define kRockParams 0x00B46778
#define kRustleParams 0x00B46788
#define kWindMatrixes 0x00B467B8
static const UInt32 kRenderShadowMapHook = 0x0040C919;
static const UInt32 kRenderShadowMapReturn = 0x0040C920;
static const UInt32 kAddCastShadowFlagHook = 0x004B1A25;
static const UInt32 kAddCastShadowFlagReturn = 0x004B1A2A;
static const UInt32 kEditorCastShadowFlagHook = 0x005498DD;
static const UInt32 kEditorCastShadowFlagReturn = 0x005498E3;
static const void* VFTNiNode = (void*)0x00A7E38C;
static const void* VFTBSFadeNode = (void*)0x00A3F944;
static const void* VFTBSFaceGenNiNode = (void*)0x00A64F5C;
static const void* VFTBSTreeNode = (void*)0x00A65854;
static const void* VFTNiTriShape = (void*)0x00A7ED5C;
static const void* VFTNiTriStrips = (void*)0x00A7F27C;
#endif

#if defined(NEWVEGAS) || defined(OBLIVION)
ShadowManager::ShadowManager() {
	
	Logger::Log("Starting the shadows manager...");
	TheShadowManager = this;
	
	IDirect3DDevice9* Device = TheRenderManager->device;
	SettingsShadowStruct::ExteriorsStruct* ShadowsExteriors = &TheSettingManager->SettingsShadows.Exteriors;
	SettingsShadowStruct::InteriorsStruct* ShadowsInteriors = &TheSettingManager->SettingsShadows.Interiors;
	UINT ShadowMapSize = 0;
	UINT ShadowCubeMapSize = ShadowsInteriors->ShadowCubeMapSize;

	CurrentCell = NULL;
	ShadowCubeMapState = ShadowCubeMapStateEnum::None;

	ShadowMapVertex = new ShaderRecord();
	if (ShadowMapVertex->LoadShader("ShadowMap.vso")) Device->CreateVertexShader((const DWORD*)ShadowMapVertex->Function, &ShadowMapVertexShader);
	ShadowMapPixel = new ShaderRecord();
	if (ShadowMapPixel->LoadShader("ShadowMap.pso")) Device->CreatePixelShader((const DWORD*)ShadowMapPixel->Function, &ShadowMapPixelShader);

	ShadowCubeMapVertex = new ShaderRecord();
	if (ShadowCubeMapVertex->LoadShader("ShadowCubeMap.vso")) Device->CreateVertexShader((const DWORD*)ShadowCubeMapVertex->Function, &ShadowCubeMapVertexShader);
	ShadowCubeMapPixel = new ShaderRecord();
	if (ShadowCubeMapPixel->LoadShader("ShadowCubeMap.pso")) Device->CreatePixelShader((const DWORD*)ShadowCubeMapPixel->Function, &ShadowCubeMapPixelShader);

	for (int i = 0; i < 3; i++) {
		UINT ShadowMapSize = ShadowsExteriors->ShadowMapSize[i];
		Device->CreateTexture(ShadowMapSize, ShadowMapSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &ShadowMapTexture[i], NULL);
		ShadowMapTexture[i]->GetSurfaceLevel(0, &ShadowMapSurface[i]);
		Device->CreateDepthStencilSurface(ShadowMapSize, ShadowMapSize, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, true, &ShadowMapDepthSurface[i], NULL);
		ShadowMapViewPort[i] = { 0, 0, ShadowMapSize, ShadowMapSize, 0.0f, 1.0f };
	}
	for (int i = 0; i < 4; i++) {
		Device->CreateCubeTexture(ShadowCubeMapSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &ShadowCubeMapTexture[i], NULL);
		for (int j = 0; j < 6; j++) {
			ShadowCubeMapTexture[i]->GetCubeMapSurface((D3DCUBEMAP_FACES)j, 0, &ShadowCubeMapSurface[i][j]);
		}
	}
	Device->CreateDepthStencilSurface(ShadowCubeMapSize, ShadowCubeMapSize, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, true, &ShadowCubeMapDepthSurface, NULL);
	ShadowCubeMapViewPort = { 0, 0, ShadowCubeMapSize, ShadowCubeMapSize, 0.0f, 1.0f };
	
	memset(ShadowCubeMapLights, NULL, sizeof(ShadowCubeMapLights));

}

void ShadowManager::SetFrustum(ShadowMapTypeEnum ShadowMapType, D3DMATRIX* Matrix) {

	ShadowMapFrustum[ShadowMapType][PlaneNear].a = Matrix->_13;
	ShadowMapFrustum[ShadowMapType][PlaneNear].b = Matrix->_23;
	ShadowMapFrustum[ShadowMapType][PlaneNear].c = Matrix->_33;
	ShadowMapFrustum[ShadowMapType][PlaneNear].d = Matrix->_43;
	ShadowMapFrustum[ShadowMapType][PlaneFar].a = Matrix->_14 - Matrix->_13;
	ShadowMapFrustum[ShadowMapType][PlaneFar].b = Matrix->_24 - Matrix->_23;
	ShadowMapFrustum[ShadowMapType][PlaneFar].c = Matrix->_34 - Matrix->_33;
	ShadowMapFrustum[ShadowMapType][PlaneFar].d = Matrix->_44 - Matrix->_43;
	ShadowMapFrustum[ShadowMapType][PlaneLeft].a = Matrix->_14 + Matrix->_11;
	ShadowMapFrustum[ShadowMapType][PlaneLeft].b = Matrix->_24 + Matrix->_21;
	ShadowMapFrustum[ShadowMapType][PlaneLeft].c = Matrix->_34 + Matrix->_31;
	ShadowMapFrustum[ShadowMapType][PlaneLeft].d = Matrix->_44 + Matrix->_41;
	ShadowMapFrustum[ShadowMapType][PlaneRight].a = Matrix->_14 - Matrix->_11;
	ShadowMapFrustum[ShadowMapType][PlaneRight].b = Matrix->_24 - Matrix->_21;
	ShadowMapFrustum[ShadowMapType][PlaneRight].c = Matrix->_34 - Matrix->_31;
	ShadowMapFrustum[ShadowMapType][PlaneRight].d = Matrix->_44 - Matrix->_41;
	ShadowMapFrustum[ShadowMapType][PlaneTop].a = Matrix->_14 - Matrix->_12;
	ShadowMapFrustum[ShadowMapType][PlaneTop].b = Matrix->_24 - Matrix->_22;
	ShadowMapFrustum[ShadowMapType][PlaneTop].c = Matrix->_34 - Matrix->_32;
	ShadowMapFrustum[ShadowMapType][PlaneTop].d = Matrix->_44 - Matrix->_42;
	ShadowMapFrustum[ShadowMapType][PlaneBottom].a = Matrix->_14 + Matrix->_12;
	ShadowMapFrustum[ShadowMapType][PlaneBottom].b = Matrix->_24 + Matrix->_22;
	ShadowMapFrustum[ShadowMapType][PlaneBottom].c = Matrix->_34 + Matrix->_32;
	ShadowMapFrustum[ShadowMapType][PlaneBottom].d = Matrix->_44 + Matrix->_42;
	for (int i = 0; i < 6; ++i) {
		D3DXPLANE Plane(ShadowMapFrustum[ShadowMapType][i]);
		D3DXPlaneNormalize(&ShadowMapFrustum[ShadowMapType][i], &Plane);
	}

}

bool ShadowManager::InFrustum(ShadowMapTypeEnum ShadowMapType, NiNode* Node) {
	
	float Distance = 0.0f;
	bool R = false;
	NiBound* Bound = Node->GetWorldBound();

	if (Bound) {
		D3DXVECTOR3 Position = { Bound->Center.x - TheRenderManager->CameraPosition.x, Bound->Center.y - TheRenderManager->CameraPosition.y, Bound->Center.z - TheRenderManager->CameraPosition.z };
		
		R = true;
		for (int i = 0; i < 6; ++i) {
			Distance = D3DXPlaneDotCoord(&ShadowMapFrustum[ShadowMapType][i], &Position);
			if (Distance <= -Bound->Radius) {
				R = false;
				break;
			}
		}
		if (ShadowMapType == MapFar && R) { // Ensures to not be fully in the near frustum
			for (int i = 0; i < 6; ++i) {
				Distance = D3DXPlaneDotCoord(&ShadowMapFrustum[MapNear][i], &Position);
				if (Distance <= -Bound->Radius || std::fabs(Distance) < Bound->Radius) {
					R = false;
					break;
				}
			}
			R = !R;
		}
	}
	return R;

}

TESObjectREFR* ShadowManager::GetRef(TESObjectREFR* Ref, SettingsShadowStruct::FormsStruct* Forms, SettingsShadowStruct::ExcludedFormsList* ExcludedForms) {
	
	TESObjectREFR* R = NULL;

	if (Ref && Ref->GetNode()) {
		TESForm* Form = Ref->baseForm;
		if (!(Ref->flags & TESForm::FormFlags::kFormFlags_NotCastShadows)) {
			UInt8 TypeID = Form->formType;
			if ((TypeID == TESForm::FormType::kFormType_Activator && Forms->Activators) ||
				(TypeID == TESForm::FormType::kFormType_Apparatus && Forms->Apparatus) ||
				(TypeID == TESForm::FormType::kFormType_Book && Forms->Books) ||
				(TypeID == TESForm::FormType::kFormType_Container && Forms->Containers) ||
				(TypeID == TESForm::FormType::kFormType_Door && Forms->Doors) ||
				(TypeID == TESForm::FormType::kFormType_Misc && Forms->Misc) ||
				(TypeID >= TESForm::FormType::kFormType_Stat && TypeID <= TESForm::FormType::kFormType_MoveableStatic && Forms->Statics) ||
				(TypeID == TESForm::FormType::kFormType_Tree && Forms->Trees) ||
				(TypeID == TESForm::FormType::kFormType_Furniture && Forms->Furniture) ||
				(TypeID >= TESForm::FormType::kFormType_NPC && TypeID <= TESForm::FormType::kFormType_LeveledCreature && Forms->Actors))
				R = Ref;
			if (R && ExcludedForms->size() > 0 && std::binary_search(ExcludedForms->begin(), ExcludedForms->end(), Form->refID)) R = NULL;
		}
	}
	return R;

}

void ShadowManager::RenderExterior(NiAVObject* Object, float MinRadius) {
	
	if (Object) {
		float Radius = Object->GetWorldBoundRadius();
		if (!(Object->m_flags & NiAVObject::kFlag_AppCulled) && Radius >= MinRadius && Object->m_worldTransform.pos.z + Radius > TheShaderManager->ShaderConst.Water.waterSettings.x) {
			void* VFT = *(void**)Object;
			if (VFT == VFTNiNode || VFT == VFTBSFadeNode || VFT == VFTBSFaceGenNiNode || VFT == VFTBSTreeNode) {
				if (VFT == VFTBSFadeNode && ((BSFadeNode*)Object)->FadeAlpha < 0.75f) return;
				NiNode* Node = (NiNode*)Object;
				for (int i = 0; i < Node->m_children.end; i++) {
					RenderExterior(Node->m_children.data[i], MinRadius);
				}
			}
			else if (VFT == VFTNiTriShape || VFT == VFTNiTriStrips) {
				RenderGeometry((NiGeometry*)Object);
			}
		}
	}

}

void ShadowManager::RenderInterior(NiAVObject* Object, float MinRadius) {
	
	if (Object) {
		float Radius = Object->GetWorldBoundRadius();
		if (!(Object->m_flags & NiAVObject::kFlag_AppCulled) && Radius >= MinRadius) {
			void* VFT = *(void**)Object;
			if (VFT == VFTNiNode || VFT == VFTBSFaceGenNiNode) {
				NiNode* Node = (NiNode*)Object;
				for (int i = 0; i < Node->m_children.end; i++) {
					RenderInterior(Node->m_children.data[i], MinRadius);
				}
			}
			else if (VFT == VFTNiTriShape || VFT == VFTNiTriStrips) {
				RenderGeometry((NiGeometry*)Object);
			}
		}
	}

}

void ShadowManager::RenderTerrain(NiAVObject* Object, ShadowMapTypeEnum ShadowMapType) {

	if (Object && !(Object->m_flags & NiAVObject::kFlag_AppCulled)) {
		void* VFT = *(void**)Object;
		if (VFT == VFTNiNode) {
			NiNode* Node = (NiNode*)Object;
			if (InFrustum(ShadowMapType, Node)) {
				for (int i = 0; i < Node->m_children.end; i++) {
					RenderTerrain(Node->m_children.data[i], ShadowMapType);
				}
			}
		}
		else if (VFT == VFTNiTriShape || VFT == VFTNiTriStrips) {
			RenderGeometry((NiGeometry*)Object);
		}
	}

}

void ShadowManager::RenderGeometry(NiGeometry* Geo) {

	NiGeometryBufferData* GeoData = NULL;

	if (Geo->shader) {
		GeoData = Geo->geomData->BuffData;
		if (GeoData) {
			Render(Geo);
		}
		else if (Geo->skinInstance && Geo->skinInstance->SkinPartition && Geo->skinInstance->SkinPartition->Partitions) {
			GeoData = Geo->skinInstance->SkinPartition->Partitions[0].BuffData;
			if (GeoData) Render(Geo);
		}
	}

}

void ShadowManager::Render(NiGeometry* Geo) {
	
	IDirect3DDevice9* Device = TheRenderManager->device;
	NiDX9RenderState* RenderState = TheRenderManager->renderState;
	int StartIndex = 0;
	int PrimitiveCount = 0;
	int StartRegister = 9;
	NiGeometryData* ModelData = Geo->geomData;
	NiGeometryBufferData* GeoData = ModelData->BuffData;
	NiSkinInstance* SkinInstance = Geo->skinInstance;
	NiD3DShaderDeclaration* ShaderDeclaration = (Geo->shader ? Geo->shader->ShaderDeclaration : NULL);

	if (Geo->m_pcName && !memcmp(Geo->m_pcName, "Torch", 5)) return; // No torch geo, it is too near the light and a bad square is rendered.
	
	TheShaderManager->ShaderConst.Shadow.Data.x = 0.0f; // Type of geo (0 normal, 1 actors (skinned), 2 speedtree leaves)
	TheShaderManager->ShaderConst.Shadow.Data.y = 0.0f; // Alpha control
	if (GeoData) {
		TheRenderManager->CreateD3DMatrix(&TheShaderManager->ShaderConst.ShadowMap.ShadowWorld, &Geo->m_worldTransform);
		if (Geo->m_parent->m_pcName && !memcmp(Geo->m_parent->m_pcName, "Leaves", 6)) {
			NiVector4* RockParams = (NiVector4*)kRockParams;
			NiVector4* RustleParams = (NiVector4*)kRustleParams;
			NiVector4* WindMatrixes = (NiVector4*)kWindMatrixes;
			SpeedTreeLeafShaderProperty* STProp = (SpeedTreeLeafShaderProperty*)Geo->GetProperty(NiProperty::PropertyType::kType_Shade);
			BSTreeNode* Node = (BSTreeNode*)Geo->m_parent->m_parent;
			NiDX9SourceTextureData* Texture = (NiDX9SourceTextureData*)Node->TreeModel->LeavesTexture->rendererData;

			TheShaderManager->ShaderConst.Shadow.Data.x = 2.0f;
			Device->SetVertexShaderConstantF(63, (float*)&BillboardRight, 1);
			Device->SetVertexShaderConstantF(64, (float*)&BillboardUp, 1);
			Device->SetVertexShaderConstantF(65, (float*)RockParams, 1);
			Device->SetVertexShaderConstantF(66, (float*)RustleParams, 1);
			Device->SetVertexShaderConstantF(67, (float*)WindMatrixes, 16);
			Device->SetVertexShaderConstantF(83, STProp->leafData->leafBase, 48);
			RenderState->SetTexture(0, Texture->dTexture);
			RenderState->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP, false);
			RenderState->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP, false);
			RenderState->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT, false);
			RenderState->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT, false);
			RenderState->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT, false);
		}
		else {
			BSShaderProperty* ShaderProperty = (BSShaderProperty*)Geo->GetProperty(NiProperty::PropertyType::kType_Shade);
			if (!ShaderProperty || !ShaderProperty->IsLightingProperty()) return;
			if (AlphaEnabled) {
				NiAlphaProperty* AProp = (NiAlphaProperty*)Geo->GetProperty(NiProperty::PropertyType::kType_Alpha);
				if (AProp->flags & NiAlphaProperty::AlphaFlags::ALPHA_BLEND_MASK || AProp->flags & NiAlphaProperty::AlphaFlags::TEST_ENABLE_MASK) {
					if (NiTexture* Texture = *((BSShaderPPLightingProperty*)ShaderProperty)->textures[0]) {
						TheShaderManager->ShaderConst.Shadow.Data.y = 1.0f;
						RenderState->SetTexture(0, Texture->rendererData->dTexture);
						RenderState->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP, false);
						RenderState->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP, false);
						RenderState->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT, false);
						RenderState->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT, false);
						RenderState->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT, false);
					}
				}
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
		CurrentVertex->SetCT();
		CurrentPixel->SetCT();
		for (UInt32 i = 0; i < GeoData->NumArrays; i++) {
			if (GeoData->ArrayLengths)
				PrimitiveCount = GeoData->ArrayLengths[i] - 2;
			else
				PrimitiveCount = GeoData->TriCount;
			Device->DrawIndexedPrimitive(GeoData->PrimitiveType, GeoData->BaseVertexIndex, 0, GeoData->VertCount, StartIndex, PrimitiveCount);
			StartIndex += PrimitiveCount + 2;
		}
	}
	else {
		TheShaderManager->ShaderConst.Shadow.Data.x = 1.0f;
		NiSkinPartition* SkinPartition = SkinInstance->SkinPartition;
		D3DPRIMITIVETYPE PrimitiveType = (SkinPartition->Partitions[0].Strips == 0) ? D3DPT_TRIANGLELIST : D3DPT_TRIANGLESTRIP;
		TheRenderManager->CalculateBoneMatrixes(SkinInstance, &Geo->m_worldTransform);
		if (SkinInstance->SkinToWorldWorldToSkin) memcpy(&TheShaderManager->ShaderConst.ShadowMap.ShadowWorld, SkinInstance->SkinToWorldWorldToSkin, 0x40);
		for (UInt32 p = 0; p < SkinPartition->PartitionsCount; p++) {
			StartIndex = 0;
			StartRegister = 9;
			NiSkinPartition::Partition* Partition = &SkinPartition->Partitions[p];
			for (int i = 0; i < Partition->Bones; i++) {
				UInt16 NewIndex = (Partition->pBones == NULL) ? i : Partition->pBones[i];
				Device->SetVertexShaderConstantF(StartRegister, ((float*)SkinInstance->BoneMatrixes) + (NewIndex * 3 * 4), 3);
				StartRegister += 3;
			}
			GeoData = Partition->BuffData;
			TheRenderManager->PackSkinnedGeometryBuffer(GeoData, ModelData, SkinInstance, Partition, ShaderDeclaration);
			for (UInt32 i = 0; i < GeoData->StreamCount; i++) {
				Device->SetStreamSource(i, GeoData->VBChip[i]->VB, 0, GeoData->VertexStride[i]);
			}
			Device->SetIndices(GeoData->IB);
			if (GeoData->FVF)
				Device->SetFVF(GeoData->FVF);
			else
				Device->SetVertexDeclaration(GeoData->VertexDeclaration);
			CurrentVertex->SetCT();
			CurrentPixel->SetCT();
			for (UInt32 i = 0; i < GeoData->NumArrays; i++) {
				if (GeoData->ArrayLengths)
					PrimitiveCount = GeoData->ArrayLengths[i] - 2;
				else
					PrimitiveCount = GeoData->TriCount;
				Device->DrawIndexedPrimitive(PrimitiveType, GeoData->BaseVertexIndex, 0, Partition->Vertices, StartIndex, PrimitiveCount);
				StartIndex += PrimitiveCount + 2;
			}
		}
	}

}

void ShadowManager::RenderShadowMap(ShadowMapTypeEnum ShadowMapType, SettingsShadowStruct::ExteriorsStruct* ShadowsExteriors, D3DXVECTOR3* At, D3DXVECTOR4* SunDir) {
	
	ShaderConstants::ShadowMapStruct* ShadowMap = &TheShaderManager->ShaderConst.ShadowMap;
	IDirect3DDevice9* Device = TheRenderManager->device;
	NiDX9RenderState* RenderState = TheRenderManager->renderState;
	GridCellArray* CellArray = Tes->gridCellArray;
	UInt32 CellArraySize = CellArray->size * CellArray->size;
	float FarPlane = ShadowsExteriors->ShadowMapFarPlane;
	float Radius = ShadowsExteriors->ShadowMapRadius[ShadowMapType];
	float MinRadius = ShadowsExteriors->Forms[ShadowMapType].MinRadius;
	D3DXVECTOR3 Up = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	D3DXMATRIX View, Proj;
	D3DXVECTOR3 Eye;

	AlphaEnabled = ShadowsExteriors->AlphaEnabled[ShadowMapType];

	Eye.x = At->x - FarPlane * SunDir->x * -1;
	Eye.y = At->y - FarPlane * SunDir->y * -1;
	Eye.z = At->z - FarPlane * SunDir->z * -1;
	D3DXMatrixLookAtRH(&View, &Eye, At, &Up);
	D3DXMatrixOrthoRH(&Proj, 2.0f * Radius, (1 + SunDir->z) * Radius, 0.0f, 2.0f * FarPlane);
	ShadowMap->ShadowViewProj = View * Proj;
	ShadowMap->ShadowCameraToLight[ShadowMapType] = TheRenderManager->InvViewProjMatrix * ShadowMap->ShadowViewProj;
	BillboardRight = { View._11, View._21, View._31, 0.0f };
	BillboardUp = { View._12, View._22, View._32, 0.0f };
	SetFrustum(ShadowMapType, &ShadowMap->ShadowViewProj);
	Device->SetRenderTarget(0, ShadowMapSurface[ShadowMapType]);
	Device->SetDepthStencilSurface(ShadowMapDepthSurface[ShadowMapType]);
	Device->SetViewport(&ShadowMapViewPort[ShadowMapType]);
	Device->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(1.0f, 0.25f, 0.25f, 0.55f), 1.0f, 0L);
	if (ShadowsExteriors->Enabled[ShadowMapType] && SunDir->z > 0.0f) {
		RenderState->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE, RenderStateArgs);
		RenderState->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE, RenderStateArgs);
		RenderState->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE, RenderStateArgs);
		RenderState->SetRenderState(D3DRS_ALPHABLENDENABLE, 0, RenderStateArgs);
		RenderState->SetVertexShader(ShadowMapVertexShader, false);
		RenderState->SetPixelShader(ShadowMapPixelShader, false);
		Device->BeginScene();
		for (UInt32 i = 0; i < CellArraySize; i++) {
			if (TESObjectCELL* Cell = CellArray->grid[i].cell) {
				if (ShadowsExteriors->Forms[ShadowMapType].Terrain) {
					NiNode* CellNode = Cell->niNode;
					for (int i = 2; i < 6; i++) {
						NiNode* TerrainNode = (NiNode*)CellNode->m_children.data[i];
						if (TerrainNode->m_children.end) RenderTerrain(TerrainNode->m_children.data[0], ShadowMapType);
					}
				}
				TList<TESObjectREFR>::Entry* Entry = &Cell->objectList.First;
				while (Entry) {
					if (TESObjectREFR* Ref = GetRef(Entry->item, &ShadowsExteriors->Forms[ShadowMapType], &ShadowsExteriors->ExcludedForms)) {
						NiNode* RefNode = Ref->GetNode();
						if (InFrustum(ShadowMapType, RefNode)) RenderExterior(RefNode, MinRadius);
					}
					Entry = Entry->next;
				}
			}
		}
		Device->EndScene();
	}

}

void ShadowManager::RenderShadowCubeMap(NiPointLight** Lights, int LightIndex, SettingsShadowStruct::InteriorsStruct* ShadowsInteriors) {
	
	ShaderConstants::ShadowMapStruct* ShadowMap = &TheShaderManager->ShaderConst.ShadowMap;
	IDirect3DDevice9* Device = TheRenderManager->device;
	NiDX9RenderState* RenderState = TheRenderManager->renderState;
	float Radius = 0.0f;
	float MinRadius = ShadowsInteriors->Forms.MinRadius;
	NiPoint3* LightPos = NULL;
	D3DXMATRIX View, Proj;
	D3DXVECTOR3 Eye, At, Up;

	Device->SetDepthStencilSurface(ShadowCubeMapDepthSurface);
	for (int L = 0; L <= LightIndex; L++) {
		LightPos = &Lights[L]->m_worldTransform.pos;
		Radius = Lights[L]->Spec.r;
		if (Lights[L]->CanCarry) Radius = 256.0f;
		Eye.x = LightPos->x - TheRenderManager->CameraPosition.x;
		Eye.y = LightPos->y - TheRenderManager->CameraPosition.y;
		Eye.z = LightPos->z - TheRenderManager->CameraPosition.z;
		ShadowMap->ShadowCubeMapLightPosition.x = ShadowMap->ShadowLightPosition[L].x = Eye.x;
		ShadowMap->ShadowCubeMapLightPosition.y = ShadowMap->ShadowLightPosition[L].y = Eye.y;
		ShadowMap->ShadowCubeMapLightPosition.z = ShadowMap->ShadowLightPosition[L].z = Eye.z;
		ShadowMap->ShadowCubeMapLightPosition.w = ShadowMap->ShadowLightPosition[L].w = Radius;
		TheShaderManager->ShaderConst.Shadow.Data.z = Radius;
		D3DXMatrixPerspectiveFovRH(&Proj, D3DXToRadian(90.0f), 1.0f, 1.0f, Radius);
		for (int Face = 0; Face < 6; Face++) {
			At.x = Eye.x;
			At.y = Eye.y;
			At.z = Eye.z;
			switch (Face) {
				case D3DCUBEMAP_FACE_POSITIVE_X:
					At += D3DXVECTOR3(1.0f, 0.0f, 0.0f);
					Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
					break;
				case D3DCUBEMAP_FACE_NEGATIVE_X:
					At += D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
					Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
					break;
				case D3DCUBEMAP_FACE_POSITIVE_Y:
					At += D3DXVECTOR3(0.0f, 1.0f, 0.0f);
					Up = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
					break;
				case D3DCUBEMAP_FACE_NEGATIVE_Y:
					At += D3DXVECTOR3(0.0f, -1.0f, 0.0f);
					Up = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
					break;
				case D3DCUBEMAP_FACE_POSITIVE_Z:
					At += D3DXVECTOR3(0.0f, 0.0f, -1.0f);
					Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
					break;
				case D3DCUBEMAP_FACE_NEGATIVE_Z:
					At += D3DXVECTOR3(0.0f, 0.0f, 1.0f);
					Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
					break;
			}
			D3DXMatrixLookAtRH(&View, &Eye, &At, &Up);
			ShadowMap->ShadowViewProj = View * Proj;
			Device->SetRenderTarget(0, ShadowCubeMapSurface[L][Face]);
			Device->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(1.0f, 0.25f, 0.25f, 0.55f), 1.0f, 0L);
			if (ShadowsInteriors->Enabled) {
				Device->BeginScene();
				RenderState->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE, RenderStateArgs);
				RenderState->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE, RenderStateArgs);
				RenderState->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE, RenderStateArgs);
				RenderState->SetRenderState(D3DRS_ALPHABLENDENABLE, 0, RenderStateArgs);
				Device->SetViewport(&ShadowCubeMapViewPort);
				RenderState->SetVertexShader(ShadowCubeMapVertexShader, false);
				RenderState->SetPixelShader(ShadowCubeMapPixelShader, false);
				TList<TESObjectREFR>::Entry* Entry = &Player->parentCell->objectList.First;
				while (Entry) {
					if (TESObjectREFR* Ref = GetRef(Entry->item, &ShadowsInteriors->Forms, &ShadowsInteriors->ExcludedForms)) {
						NiNode* RefNode = Ref->GetNode();
						if (RefNode->GetDistance(LightPos) <= Radius * 1.2f) RenderInterior(RefNode, MinRadius);
					}
					Entry = Entry->next;
				}
				Device->EndScene();
			}
		}
	}
	
}

void ShadowManager::RenderShadowMaps() {
	
	SettingsMainStruct::EquipmentModeStruct* EquipmentModeSettings = &TheSettingManager->SettingsMain.EquipmentMode;
	SettingsShadowStruct::ExteriorsStruct* ShadowsExteriors = &TheSettingManager->SettingsShadows.Exteriors;
	SettingsShadowStruct::InteriorsStruct* ShadowsInteriors = &TheSettingManager->SettingsShadows.Interiors;
	SettingsShadowStruct::FormsStruct* ShadowsInteriorsForms = &ShadowsInteriors->Forms;
	SettingsShadowStruct::ExcludedFormsList* ShadowsInteriorsExcludedForms = &ShadowsInteriors->ExcludedForms;
	IDirect3DDevice9* Device = TheRenderManager->device;
	NiDX9RenderState* RenderState = TheRenderManager->renderState;
	IDirect3DSurface9* DepthSurface = NULL;
	D3DXVECTOR4* ShadowData = &TheShaderManager->ShaderConst.Shadow.Data;
	D3DXVECTOR4* OrthoData = &TheShaderManager->ShaderConst.Shadow.OrthoData;

	Device->GetDepthStencilSurface(&DepthSurface);
	TheRenderManager->SetupSceneCamera();
	if (Player->GetWorldSpace()) {
		D3DXVECTOR4* SunDir = &TheShaderManager->ShaderConst.SunDir;
		D3DXVECTOR4 OrthoDir = D3DXVECTOR3(0.05f, 0.05f, 1.0f);
		NiNode* PlayerNode = Player->GetNode();
		D3DXVECTOR3 At;

		CurrentVertex = ShadowMapVertex;
		CurrentPixel = ShadowMapPixel;
		ClearShadowCubeMaps(Device, -1, ShadowCubeMapStateEnum::Exterior);

		At.x = PlayerNode->m_worldTransform.pos.x - TheRenderManager->CameraPosition.x;
		At.y = PlayerNode->m_worldTransform.pos.y - TheRenderManager->CameraPosition.y;
		At.z = PlayerNode->m_worldTransform.pos.z - TheRenderManager->CameraPosition.z;
		
		RenderShadowMap(MapNear, ShadowsExteriors, &At, SunDir);
		RenderShadowMap(MapFar, ShadowsExteriors, &At, SunDir);
		RenderShadowMap(MapOrtho, ShadowsExteriors, &At, &OrthoDir);

		ShadowData->x = ShadowsExteriors->Quality;
		ShadowData->y = ShadowsExteriors->Darkness;
		if (SunDir->z < 0.1f) {
			if (ShadowData->y == 0.0f) ShadowData->y = 0.1f;
			ShadowData->y += log(SunDir->z) / -10.0f;
			if (ShadowData->y > 1.0f) ShadowData->y = 1.0f;
		}
		ShadowData->z = 1.0f / (float)ShadowsExteriors->ShadowMapSize[MapNear];
		ShadowData->w = 1.0f / (float)ShadowsExteriors->ShadowMapSize[MapFar];
		
		OrthoData->z = 1.0f / (float)ShadowsExteriors->ShadowMapSize[MapOrtho];
	}
	else {
		std::map<int, NiPointLight*> SceneLights;
		NiPointLight* Lights[4] = { NULL };
		int LightIndex = -1;
		bool TorchOnBeltEnabled = EquipmentModeSettings->Enabled && EquipmentModeSettings->TorchKey != 255;

		NiTList<ShadowSceneLight>::Entry* Entry = SceneNode->lights.start;
		while (Entry) {
			NiPointLight* Light = Entry->data->sourceLight;
			int Distance = (int)Light->GetDistance(&Player->pos);
			while (SceneLights[Distance]) { --Distance; }
			SceneLights[Distance] = Light;
			Entry = Entry->next;
		}

		std::map<int, NiPointLight*>::iterator v = SceneLights.begin();
		while (v != SceneLights.end()) {
			NiPointLight* Light = v->second;
			bool CastShadow = true;
			if (TorchOnBeltEnabled && Light->CanCarry == 2) {
				HighProcessEx* Process = (HighProcessEx*)Player->process;
				if (Process->OnBeltState == HighProcessEx::State::In) CastShadow = false;
			}
			if (Light->CastShadows && CastShadow) {
				LightIndex += 1;
				Lights[LightIndex] = Light;
			}
			if (LightIndex == ShadowsInteriors->LightPoints - 1 || LightIndex == 3) break;
			v++;
		}

		CurrentVertex = ShadowCubeMapVertex;
		CurrentPixel = ShadowCubeMapPixel;
		AlphaEnabled = ShadowsInteriors->AlphaEnabled;
		if (CurrentCell != Player->parentCell) { ShadowCubeMapState = ShadowCubeMapStateEnum::None; CurrentCell = Player->parentCell; }

		RenderShadowCubeMap(Lights, LightIndex, ShadowsInteriors);

		ClearShadowCubeMaps(Device, LightIndex, ShadowCubeMapStateEnum::Interior);
		CalculateBlend(Lights, LightIndex);

		ShadowData->x = ShadowsInteriors->Quality;
		ShadowData->y = ShadowsInteriors->Darkness;
		ShadowData->z = 1.0f / (float)ShadowsInteriors->ShadowCubeMapSize;
	}
	Device->SetDepthStencilSurface(DepthSurface);

#if DEBUGSH
	if (TheKeyboardManager->OnKeyDown(26)) {
		D3DXSaveSurfaceToFileA("C:\\Archivio\\Downloads\\shadowmap0.jpg", D3DXIFF_JPG, ShadowMapSurface[0], NULL, NULL);
		D3DXSaveSurfaceToFileA("C:\\Archivio\\Downloads\\shadowmap1.jpg", D3DXIFF_JPG, ShadowMapSurface[1], NULL, NULL);
		D3DXSaveSurfaceToFileA("C:\\Archivio\\Downloads\\shadowmap2.jpg", D3DXIFF_JPG, ShadowMapSurface[2], NULL, NULL);
	}
	if (TheKeyboardManager->OnKeyDown(26)) {
		D3DXSaveSurfaceToFileA("C:\\Archivio\\Downloads\\shadowcubemap0.jpg", D3DXIFF_JPG, ShadowCubeMapSurface[0][0], NULL, NULL);
		D3DXSaveSurfaceToFileA("C:\\Archivio\\Downloads\\shadowcubemap1.jpg", D3DXIFF_JPG, ShadowCubeMapSurface[0][1], NULL, NULL);
		D3DXSaveSurfaceToFileA("C:\\Archivio\\Downloads\\shadowcubemap2.jpg", D3DXIFF_JPG, ShadowCubeMapSurface[0][2], NULL, NULL);
		D3DXSaveSurfaceToFileA("C:\\Archivio\\Downloads\\shadowcubemap3.jpg", D3DXIFF_JPG, ShadowCubeMapSurface[0][3], NULL, NULL);
		D3DXSaveSurfaceToFileA("C:\\Archivio\\Downloads\\shadowcubemap4.jpg", D3DXIFF_JPG, ShadowCubeMapSurface[0][4], NULL, NULL);
		D3DXSaveSurfaceToFileA("C:\\Archivio\\Downloads\\shadowcubemap5.jpg", D3DXIFF_JPG, ShadowCubeMapSurface[0][5], NULL, NULL);
	}
#endif

}

void ShadowManager::ClearShadowCubeMaps(IDirect3DDevice9* Device, int From, ShadowCubeMapStateEnum NewState) {

	if (ShadowCubeMapState != NewState) {
		for (int L = From + 1; L < 4; L++) {
			TheShaderManager->ShaderConst.ShadowMap.ShadowLightPosition[L].w = 0.0f;
			for (int Face = 0; Face < 6; Face++) {
				Device->SetRenderTarget(0, ShadowCubeMapSurface[L][Face]);
				Device->Clear(0L, NULL, D3DCLEAR_TARGET, D3DXCOLOR(1.0f, 0.25f, 0.25f, 0.55f), 1.0f, 0L);
			}
		}
		ShadowCubeMapState = NewState;
	}
	
}

void ShadowManager::CalculateBlend(NiPointLight** Lights, int LightIndex) {

	D3DXVECTOR4* ShadowCubeMapBlend = &TheShaderManager->ShaderConst.ShadowMap.ShadowCubeMapBlend;
	float* Blend = NULL;
	bool Found = false;

	if (memcmp(Lights, ShadowCubeMapLights, 16)) {
		for (int i = 0; i <= LightIndex; i++) {
			for (int j = 0; j <= LightIndex; j++) {
				if (Lights[i] == ShadowCubeMapLights[j]) {
					Found = true;
					break;
				}
			}
			if (i == 0)
				Blend = &ShadowCubeMapBlend->x;
			else if (i == 1)
				Blend = &ShadowCubeMapBlend->y;
			else if (i == 2)
				Blend = &ShadowCubeMapBlend->z;
			else if (i == 3)
				Blend = &ShadowCubeMapBlend->w;
			if (!Found) *Blend = 0.0f;
			Found = false;
		}
		memcpy(ShadowCubeMapLights, Lights, 16);
	}
	else {
		if (ShadowCubeMapBlend->x < 1.0f) ShadowCubeMapBlend->x += 0.1f;
		if (ShadowCubeMapBlend->y < 1.0f) ShadowCubeMapBlend->y += 0.1f;
		if (ShadowCubeMapBlend->z < 1.0f) ShadowCubeMapBlend->z += 0.1f;
		if (ShadowCubeMapBlend->w < 1.0f) ShadowCubeMapBlend->w += 0.1f;
	}
	
}

static __declspec(naked) void RenderShadowMapHook() {

	__asm {
		pushad
		mov		ecx, TheShadowManager
		call	ShadowManager::RenderShadowMaps
		popad
		jmp		kRenderShadowMapReturn
	}

}

void AddCastShadowFlag(TESObjectREFR* Ref, TESObjectLIGH* Light, NiPointLight* LightPoint) {
	
	SettingsShadowStruct::InteriorsStruct* ShadowsInteriors = &TheSettingManager->SettingsShadows.Interiors;

	if (Light->lightFlags & TESObjectLIGH::LightFlags::kLightFlags_CanCarry) {
		LightPoint->CastShadows = ShadowsInteriors->TorchesCastShadows;
		LightPoint->CanCarry = 1;
		if (Ref == Player) {
			if (Player->isThirdPerson) {
				if (Player->firstPersonSkinInfo->LightForm == Light) LightPoint->CastShadows = 0;
			}
			else {
				if (Player->ActorSkinInfo->LightForm == NULL && Player->firstPersonSkinInfo->LightForm == Light) LightPoint->CastShadows = 0;
			}
			LightPoint->CanCarry = 2;
		}
	}		
	else if (Ref->baseForm->formType == TESForm::FormType::kFormType_Light) {
		LightPoint->CastShadows = !(Ref->flags & TESForm::FormFlags::kFormFlags_NotCastShadows);
		LightPoint->CanCarry = 0;
	}

}

static __declspec(naked) void AddCastShadowFlagHook() {

	__asm {
#if defined(OBLIVION)
		mov     ecx, [esp + 0x158]
		pushad
		push	esi
		push	edi
		push	ecx
		call	AddCastShadowFlag
		pop		ecx
		pop		edi
		pop		esi
		popad
		pop		ecx
		pop		edi
		pop		esi
		pop		ebp
		pop		ebx
#elif defined(NEWVEGAS)
		mov     esi, [ebp + 0x008]
		pushad
		mov		ecx, [ebp - 0x164]
		push	eax
		push	ecx
		push	esi
		call	AddCastShadowFlag
		pop		esi
		pop		ecx
		pop		eax
		popad
		pop		ecx
		pop		esi
		mov     ecx, [ebp - 0x34]
#endif
		jmp		kAddCastShadowFlagReturn
	}

}

#if defined(NEWVEGAS)
void LeavesNodeName(BSTreeNode* TreeNode) {
	
	TreeNode->m_children.data[1]->SetName("Leaves");

}

static __declspec(naked) void LeavesNodeNameHook() {

	__asm {
		pushad
		push	eax 
		call	LeavesNodeName
		pop		eax
		popad
		jmp		kLeavesNodeNameReturn
	}

}
#endif

void CreateShadowsHook() {
	
	WriteRelJump(kRenderShadowMapHook,		(UInt32)RenderShadowMapHook);
	WriteRelJump(kAddCastShadowFlagHook,	(UInt32)AddCastShadowFlagHook);

#if defined(NEWVEGAS)
	WriteRelJump(kLeavesNodeName, (UInt32)LeavesNodeNameHook);
#endif
}

void EditorCastShadowFlag(HWND Window, TESForm* Form) {
	
	if (Window && Form) {
		SetDlgItemTextA(Window, 0x697, "Does Not Cast Shadows");
		SetWindowPos(GetDlgItem(Window, 0x697), HWND_BOTTOM, 0, 0, 140, 15, SWP_NOMOVE | SWP_NOZORDER);
	}

}

static __declspec(naked) void EditorCastShadowFlagHook() {

	__asm {
		pushad
		push	eax
		push	edi
		call	EditorCastShadowFlag
		pop		edi
		pop		eax
		popad
		jmp		kEditorCastShadowFlagReturn
	}

}

void CreateEditorShadowsHook() {

	WriteRelJump(kEditorCastShadowFlagHook, (UInt32)EditorCastShadowFlagHook);

}
#elif defined(SKYRIM)
ShadowManager::ShadowManager() {

	Logger::Log("Starting the shadows manager...");
	TheShadowManager = this;

}
#endif
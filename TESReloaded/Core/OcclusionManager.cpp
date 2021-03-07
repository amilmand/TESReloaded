#include "OcclusionManager.h"

#if defined(NEWVEGAS)
#define RenderStateArgs 0, 0
#elif defined(OBLIVION)
#define RenderStateArgs 0
static const void* VFTNiNode = (void*)0x00A7E38C;
static const void* VFTBSFadeNode = (void*)0x00A3F944;
static const void* VFTNiTriShape = (void*)0x00A7ED5C;
static const void* VFTNiTriStrips = (void*)0x00A7F27C;
#endif
#define OcclusionMapSizeX 1920
#define OcclusionMapSizeY 1080
//#define OcclusionMapSizeX 480
//#define OcclusionMapSizeY 270

OcclusionManager::OcclusionManager() {
	
	Logger::Log("Starting the occlusion manager...");
	TheOcclusionManager = this;
	
	Tex = NULL;

	IDirect3DDevice9* Device = TheRenderManager->device;

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
				if (Side == NiPlane::PositiveSide) Process->Planes.ActivePlanes & ~(1 << i);
			}
		}
		if (i == NiFrustumPlanes::MaxPlanes) Result = true;
		Process->Planes.ActivePlanes = ActivePlanes;
	}
	return Result;

}

void OcclusionManager::RenderObject(NiAVObject* Object, bool Query) {

	if (Object) {
		if (!(Object->m_flags & NiAVObject::kFlag_AppCulled)) {
			void* VFT = *(void**)Object;
			if (VFT == VFTNiNode || VFT == VFTBSFadeNode) {
				NiNode* Node = (NiNode*)Object;
				if (InFrustum(Node)) {
					for (int i = 0; i < Node->m_children.end; i++) {
						RenderObject(Node->m_children.data[i], Query);
					}
				}
			}
			else if (VFT == VFTNiTriShape || VFT == VFTNiTriStrips) {
				NiGeometry* Geo = (NiGeometry*)Object;
				if (Geo->shader && Geo->geomData->BuffData) {
					if (Query) OcclusionQuery->Issue(D3DISSUE_BEGIN);
					Render(Geo);
					if (Query) {
						UInt32 Pixels = 0;
						OcclusionQuery->Issue(D3DISSUE_END);
						while (OcclusionQuery->GetData((void*)&Pixels, sizeof(DWORD), D3DGETDATA_FLUSH) == S_FALSE);
						if (Pixels == 0)
							Geo->m_flags |= NiAVObject::kFlag_AppCulled; add a new flag to the geo and manage it in the pipeline in the culling process, i cannot use the culled flag otherwise i cannot re-enable it due to the if at the start of the function!
						else
							Geo->m_flags &= ~NiAVObject::kFlag_AppCulled;
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
	D3DXMATRIX WorldMatrix;

	TheRenderManager->CreateD3DMatrix(&WorldMatrix, &Geo->m_worldTransform);
	D3DXMatrixMultiplyTranspose(&TheShaderManager->ShaderConst.OcclusionMap.OcclusionWorldViewProj, &WorldMatrix, &TheRenderManager->ViewProjMatrix);
	BSShaderProperty* ShaderProperty = (BSShaderProperty*)Geo->GetProperty(NiProperty::PropertyType::kType_Shade);
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

	IDirect3DDevice9* Device = TheRenderManager->device;
	NiDX9RenderState* RenderState = TheRenderManager->renderState;
	NiNode* ObjectLODRoot = Tes->ObjectLODRoot;
	
	Device->SetRenderTarget(0, OcclusionMapSurface);
	Device->SetDepthStencilSurface(OcclusionMapDepthSurface);
	Device->SetViewport(&OcclusionMapViewPort);
	Device->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(1.0f, 1.00f, 1.00f, 1.00f), 1.0f, 0L);
	RenderState->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE, RenderStateArgs);
	RenderState->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE, RenderStateArgs);
	RenderState->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE, RenderStateArgs);
	RenderState->SetRenderState(D3DRS_ALPHABLENDENABLE, 0, RenderStateArgs);
	RenderState->SetVertexShader(OcclusionMapVertexShader, false);
	RenderState->SetPixelShader(OcclusionMapPixelShader, false);
	Device->BeginScene();
	for (int i = 0; i < ObjectLODRoot->m_children.end; i++) {
		RenderObject(ObjectLODRoot->m_children.data[i], false);
	}
	for (int i = 0; i < ObjectLODRoot->m_children.end; i++) {
		RenderObject(ObjectLODRoot->m_children.data[i], true);
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

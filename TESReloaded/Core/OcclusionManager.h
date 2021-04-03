#pragma once

class bhkCollisionObjectEx : public bhkCollisionObject {
public:
	NiNode*		GeoNode;		// 014
};
assert(sizeof(bhkCollisionObjectEx) == 0x18);

class OcclusionManager { // Never disposed
public:
	OcclusionManager();

	bool					InFrustum(NiNode* Node);
	TESObjectREFR*			GetRef(TESObjectREFR* Ref);
	void					RenderStatic(NiAVObject* Object, float MinBoundSize, float MaxBoundSize, bool PerformOcclusion);
	void					RenderImmediate(NiAVObject* Object, bool PerformOcclusion);
	void					RenderTerrain(NiAVObject* Object);
	void					RenderWater(NiAVObject* Object);
	void					Render(NiGeometry* Geo);
	void					ManageDistantStatic(NiAVObject* Object, float MaxBoundSize);
	void					RenderDistantStatic(NiAVObject* Object);
	void					RenderOcclusionMap(SettingsMainStruct::OcclusionCullingStruct* OcclusionCulling);
	void					PerformOcclusionCulling();
	
	ShaderRecord*			OcclusionMapVertex;
	ShaderRecord*			OcclusionMapPixel;
	IDirect3DVertexShader9* OcclusionMapVertexShader;
	IDirect3DPixelShader9*	OcclusionMapPixelShader;
	IDirect3DQuery9*		OcclusionQuery;
	IDirect3DTexture9*		OcclusionMapTexture;
	IDirect3DSurface9*		OcclusionMapSurface;
	IDirect3DSurface9*		OcclusionMapDepthSurface;
	D3DVIEWPORT9			OcclusionMapViewPort;
	IDirect3DTexture9*		WaterTexture;
	bool					WaterOccluded;
};

void CreateOcclusionCullingHook();
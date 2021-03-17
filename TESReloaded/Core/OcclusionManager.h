#pragma once

class bhkCollisionObjectEx : public bhkCollisionObject {
public:
	NiGeometry*		Geometry;		// 014
};
assert(sizeof(bhkCollisionObjectEx) == 0x18);

class OcclusionManager { // Never disposed
public:
	OcclusionManager();

	bool					InFrustum(NiNode* Node);
	void					RenderStatic(NiAVObject* Object, float MinRadius);
	void					RenderTerrain(NiAVObject* Object);
	void					RenderWater(NiAVObject* Object);
	void					Render(NiGeometry* Geo);
	void					RenderOcclusionMap();
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
	bool					WaterOccluded;
	IDirect3DTexture9*		Tex;
};

void CreateOcclusionCullingHook();
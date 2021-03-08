#pragma once

class OcclusionManager { // Never disposed
public:
	OcclusionManager();

	bool					InFrustum(NiNode* Node);
	void					RenderObject(NiAVObject* Object, bool Query);
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

	IDirect3DTexture9*		Tex;
};

void CreateOcclusionCullingHook();
#pragma once
#include <d3dx9mesh.h>

enum EffectRecordType
{
	EffectRecordType_Underwater,
	EffectRecordType_WaterLens,
	EffectRecordType_GodRays,
	EffectRecordType_DepthOfField,
	EffectRecordType_AmbientOcclusion,
	EffectRecordType_Coloring,
	EffectRecordType_Cinema,
	EffectRecordType_Bloom,
	EffectRecordType_SnowAccumulation,
	EffectRecordType_BloodLens,
	EffectRecordType_SMAA,
	EffectRecordType_MotionBlur,
	EffectRecordType_LowHF,
	EffectRecordType_WetWorld,
	EffectRecordType_Sharpening,
	EffectRecordType_VolumetricFog,
	EffectRecordType_Precipitations,
	EffectRecordType_ShadowsExteriors,
	EffectRecordType_ShadowsInteriors,
	EffectRecordType_Extra,
};

struct ShaderConstants {
	
	struct ShadowMapStruct {
		D3DXMATRIXA16	ShadowWorld;
		D3DXMATRIX		ShadowViewProj;
		D3DXMATRIX		ShadowCameraToLight[3];
		D3DXVECTOR4		ShadowCubeMapLightPosition;
		D3DXVECTOR4		ShadowLightPosition[4];
		D3DXVECTOR4		ShadowCubeMapBlend;
	};
	struct OcclusionMapStruct {
		D3DXMATRIX		OcclusionWorldViewProj;
	};
	struct WaterStruct {
		D3DXVECTOR4		waterCoefficients;
		D3DXVECTOR4		waveParams;
		D3DXVECTOR4		waterVolume;
		D3DXVECTOR4		waterSettings;
		D3DXVECTOR4		deepColor;
		D3DXVECTOR4		shallowColor;
		D3DXVECTOR4		shorelineParams;
	};
	struct HDRStruct {
		D3DXVECTOR4		ToneMapping;
	};
	struct GrassStruct {
		D3DXVECTOR4		Scale;
	};
	struct POMStruct {
		D3DXVECTOR4		ParallaxData;
	};
	struct TerrainStruct {
		D3DXVECTOR4		Data;
	};
	struct SkinStruct {
		D3DXVECTOR4		SkinData;
		D3DXVECTOR4		SkinColor;
	};
	struct ShadowStruct {
		D3DXVECTOR4		Data;
		D3DXVECTOR4		OrthoData;
	};
	struct PrecipitationsStruct {
		D3DXVECTOR4		RainData;
		D3DXVECTOR4		SnowData;
	};
	struct WaterLensStruct {
		D3DXVECTOR4		Time;
		float			TimeAmount;
		float			Percent;
	};
	struct GodRaysStruct {
		D3DXVECTOR4		Ray;
		D3DXVECTOR4		RayColor;
		D3DXVECTOR4		Data;
	};
	struct DepthOfFieldStruct {
		bool			Enabled;
		D3DXVECTOR4		Blur;
		D3DXVECTOR4		Data;
	};
	struct AmbientOcclusionStruct {
		bool			Enabled;
		D3DXVECTOR4		AOData;
		D3DXVECTOR4		Data;
	};
	struct ColoringStruct {
		D3DXVECTOR4		ColorCurve;
		D3DXVECTOR4		EffectGamma;
		D3DXVECTOR4		Data;
		D3DXVECTOR4		Values;
	};
	struct CinemaStruct {
		D3DXVECTOR4		Data;
	};
	struct BloomStruct {
		D3DXVECTOR4		BloomData;
		D3DXVECTOR4		BloomValues;
	};
	struct SnowAccumulationStruct {
		D3DXVECTOR4		Params;
	};
	struct BloodLensStruct {
		D3DXVECTOR4		Params;
		D3DXVECTOR4		BloodColor;
		D3DXVECTOR4		Time;
		float			Percent;
	};
	struct MotionBlurStruct {
		D3DXVECTOR4		BlurParams;
		D3DXVECTOR4		Data;
		float			oldAngleX;
		float			oldAngleZ;
		float			oldAmountX;
		float			oldAmountY;
		float			oldoldAmountX;
		float			oldoldAmountY;
	};
	struct LowHFStruct {
		D3DXVECTOR4		Data;
		float			HealthCoeff;
		float			FatigueCoeff;
	};
	struct WetWorldStruct {
		D3DXVECTOR4		Coeffs;
		D3DXVECTOR4		Data;
	};
	struct SharpeningStruct {
		D3DXVECTOR4		Data;
	};
	struct VolumetricFogStruct {
		D3DXVECTOR4		Data;
	};

	D3DXVECTOR4				ReciprocalResolution;
	D3DXVECTOR4				ReciprocalResolutionWater;
	D3DXVECTOR4				SunDir;
	D3DXVECTOR4				SunTiming;
	D3DXVECTOR4				SunAmount;
	D3DXVECTOR4				GameTime;
	TESWeather*				pWeather;
	float					currentsunGlare;
	float					currentwindSpeed;
	UInt8					oldsunGlare;
	UInt8					oldwindSpeed;
	D3DXVECTOR4				fogColor;
	D3DXVECTOR4				oldfogColor;
	D3DXVECTOR4				sunColor;
	D3DXVECTOR4				oldsunColor;
	D3DXVECTOR4				fogData;
	float					currentfogStart;
	float					oldfogStart;
	float					currentfogEnd;
	float					oldfogEnd;
	ShadowMapStruct			ShadowMap;
	OcclusionMapStruct		OcclusionMap;
	WaterStruct				Water;
	HDRStruct				HDR;
	GrassStruct				Grass;
	POMStruct				POM;
	TerrainStruct			Terrain;
	SkinStruct				Skin;
	ShadowStruct			Shadow;
	PrecipitationsStruct	Precipitations;
	WaterLensStruct			WaterLens;
	GodRaysStruct			GodRays;
	DepthOfFieldStruct		DepthOfField;
	AmbientOcclusionStruct	AmbientOcclusion;
	ColoringStruct			Coloring;
	CinemaStruct			Cinema;
	BloomStruct				Bloom;
	SnowAccumulationStruct	SnowAccumulation;
	BloodLensStruct			BloodLens;
	MotionBlurStruct		MotionBlur;
	LowHFStruct				LowHF;
	WetWorldStruct			WetWorld;
	SharpeningStruct		Sharpening;
	VolumetricFogStruct		VolumetricFog;
};

struct ShaderValue {
	UInt32				RegisterIndex;
	UInt32				RegisterCount;
	union {
	D3DXVECTOR4*		Value;
	TextureRecord*		Texture;
	};
};

class ShaderProgram {
public:
	ShaderProgram();
	virtual ~ShaderProgram();
	
	void					SetConstantTableValue(LPCSTR Name, UInt32 Index);

	ShaderValue*			FloatShaderValues;
	UInt32					FloatShaderValuesCount;
	ShaderValue*			TextureShaderValues;
	UInt32					TextureShaderValuesCount;
	bool					Enabled;
};

class ShaderRecord : public ShaderProgram {
public:
	ShaderRecord();
	virtual ~ShaderRecord();
	
	virtual void			SetShaderConstantF(UInt32 RegisterIndex, D3DXVECTOR4* Value, UInt32 RegisterCount) = 0;

	static ShaderRecord*	LoadShader(const char* Name, const char* SubPath);

	void					CreateCT(const char* ShaderSource, ID3DXConstantTable* ConstantTable);
	void					SetCT();
	
	bool					HasCT;
	bool					HasRB; 
	bool					HasDB;
};

class ShaderRecordVertex : public ShaderRecord {
public:
	ShaderRecordVertex();
	virtual ~ShaderRecordVertex();
	
	virtual void			SetShaderConstantF(UInt32 RegisterIndex, D3DXVECTOR4* Value, UInt32 RegisterCount);

	IDirect3DVertexShader9* ShaderHandle;
};

class ShaderRecordPixel : public ShaderRecord {
public:
	ShaderRecordPixel();
	virtual ~ShaderRecordPixel();
	
	virtual void			SetShaderConstantF(UInt32 RegisterIndex, D3DXVECTOR4* Value, UInt32 RegisterCount);

	IDirect3DPixelShader9*	ShaderHandle;
};

class EffectRecord : public ShaderProgram {
public:
	EffectRecord();
	virtual ~EffectRecord();
	
	bool					LoadEffect(const char* Name);
	void					CreateCT();
	void					SetCT();
	void					Render(IDirect3DDevice9* Device, IDirect3DSurface9* RenderTarget, IDirect3DSurface9* RenderedSurface, bool ClearRenderTarget);

	char*	 				Source;
	ID3DXBuffer*			Errors;
	ID3DXEffect*			Effect;
};

typedef std::map<std::string, EffectRecord*> ExtraEffectsList;
typedef std::map<std::string, D3DXVECTOR4> CustomConstants;

class ShaderManager { // Never disposed
public:
	ShaderManager();

	void					CreateFrameVertex(UInt32 Width, UInt32 Height, IDirect3DVertexBuffer9** FrameVertex);
	void					CreateEffects();
	void					InitializeConstants();
	void					UpdateConstants();
	void					BeginScene();
	void					CreateShader(const char *Name);
	void					LoadShader(NiD3DVertexShader* Shader);
	void					LoadShader(NiD3DPixelShader* Shader);
	void					DisposeShader(const char* Name);
	void					CreateEffect(EffectRecordType EffectType);
	bool					LoadEffect(EffectRecordType EffectType, EffectRecord* TheEffect, char* Filename);
	void					DisposeEffect(EffectRecordType EffectType);
	void					RenderEffects(IDirect3DSurface9* RenderTarget);
	void					SwitchShaderStatus(const char* Name);
	void					SetCustomConstant(const char* Name, D3DXVECTOR4 Value);
	void					SetExtraEffectEnabled(const char* Name, bool Value);
	
	struct					FrameVS { float x, y, z, u, v; };

	ShaderConstants			ShaderConst;
	CustomConstants			CustomConst;
	IDirect3DTexture9*		SourceTexture;
	IDirect3DSurface9*		SourceSurface;
	IDirect3DTexture9* 		RenderedTexture;
	IDirect3DSurface9*		RenderedSurface;
	IDirect3DTexture9*		RenderTextureSMAA;
	IDirect3DSurface9*		RenderSurfaceSMAA;
	bool					RenderedBufferFilled;
	bool					DepthBufferFilled;
	IDirect3DVertexBuffer9*	FrameVertex;
	EffectRecord*			UnderwaterEffect;
	EffectRecord*			WaterLensEffect;
	EffectRecord*			GodRaysEffect;
	EffectRecord*			DepthOfFieldEffect;
	EffectRecord*			AmbientOcclusionEffect;
	EffectRecord*			ColoringEffect;
	EffectRecord*			CinemaEffect;
	EffectRecord*			BloomEffect;
	EffectRecord*			SnowAccumulationEffect;
	EffectRecord*			BloodLensEffect;
	EffectRecord*			SMAAEffect;
	EffectRecord*			MotionBlurEffect;
	EffectRecord*			LowHFEffect;
	EffectRecord*			WetWorldEffect;
	EffectRecord*			SharpeningEffect;
	EffectRecord*			VolumetricFogEffect;
	EffectRecord*			RainEffect;
	EffectRecord*			SnowEffect;
	EffectRecord*			ShadowsExteriorsEffect;
	EffectRecord*			ShadowsInteriorsEffect;
	ExtraEffectsList		ExtraEffects;
	NiD3DVertexShader*		WaterHeightMapVertexShader;
	NiD3DPixelShader*		WaterHeightMapPixelShader;
	NiD3DVertexShader*		WaterVertexShaders[51];
	NiD3DPixelShader*		WaterPixelShaders[51];
};
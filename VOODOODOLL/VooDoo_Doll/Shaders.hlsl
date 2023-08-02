struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
	matrix					gmtxView : packoffset(c0);
	matrix					gmtxProjection : packoffset(c4);
	float3					gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix					gmtxGameObject : packoffset(c0);
	float3					texMat: packoffset(c4);
};

cbuffer cbMaterialInfo : register(b3)
{
	MATERIAL				gMaterial : packoffset(c0);
	uint					gnTexturesMask : packoffset(c4);
};

#include "Light.hlsl"

cbuffer cbDrawOptions : register(b5)
{
	int4 gvDrawOptions : packoffset(c0);
};


struct CB_TOOBJECTSPACE
{
	matrix		mtxToTexture;
	float4		f4Position;
};

cbuffer cbToLightSpace : register(b6)
{
	CB_TOOBJECTSPACE gcbToLightSpaces[MAX_SHADOW_LIGHTS];
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtPrevFrame : register(t11);
Texture2D gtxtIlluminationTexture : register(t13);
Texture2D gtxtzDepthTexture : register(t14);
Texture2D gtxtDepthTexture : register(t15);
Texture2D gtxtInput : register(t0);
Texture2DArray gtxtTextureArray : register(t1);
RWTexture2D<float4> gtxtRWOutput : register(u0);

SamplerState gssWrap : register(s0);


struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);

	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

	float3 normalW;
	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}

	float4 cIllumination = Lighting(input.positionW, normalW);
	
	if (cColor.x == 1 && cColor.y == 1 && cColor.z == 1)
		discard;

	return lerp(cColor, cIllumination, 0.5f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

cbuffer cbBoneOffsets : register(b7)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b8)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;
	for (int i=0; i < MAX_VERTEX_INFLUENCES; ++i)
	{
		mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	}


	output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	output.normalW = mul(input.normal, (float3x3)mtxVertexToBoneWorld).xyz;
	output.tangentW = mul(input.tangent, (float3x3)mtxVertexToBoneWorld).xyz;
	output.bitangentW = mul(input.bitangent, (float3x3)mtxVertexToBoneWorld).xyz;

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	return(output);
}


float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
	input.normalW = normalize(input.normalW);

	return(float4(input.normalW * 0.5f + 0.5f, 1.0f));
}
//===============================================================================================================

struct PS_DEPTH_OUTPUT
{
	float fzPosition : SV_Target;
	float fDepth : SV_Depth;
};

PS_DEPTH_OUTPUT PSDepthWriteShader(VS_LIGHTING_OUTPUT input)
{
	PS_DEPTH_OUTPUT output;

	output.fzPosition = input.position.z;
	output.fDepth = input.position.z;

	return(output);
}

//=======================================================================================================================
struct VS_SHADOW_MAP_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;

	float4 uvs[MAX_SHADOW_LIGHTS] : TEXCOORD0;
};

VS_SHADOW_MAP_OUTPUT VSShadowMapShadow(VS_LIGHTING_INPUT input)
{
	VS_SHADOW_MAP_OUTPUT output = (VS_SHADOW_MAP_OUTPUT)0;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.positionW = positionW.xyz;
	output.position = mul(mul(positionW, gmtxView), gmtxProjection);
	output.normalW = mul(float4(input.normal, 0.0f), gmtxGameObject).xyz;


	for (int i = 0; i < MAX_SHADOW_LIGHTS; ++i)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f)
			output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}

	return(output);
}

float4 PSShadowMapShadow(VS_SHADOW_MAP_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uvs[0].xy);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uvs[0].xy);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uvs[0].xy);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uvs[0].xy);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uvs[0].xy);

	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;

	float4 cIllumination = shadowLighting(input.positionW, normalize(input.normalW), true, input.uvs);

	return(lerp(cColor, cIllumination, 0.5f));
}
//===================================================================================================================================
struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextureToViewport(uint nVertexID : SV_VertexID)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return(output);
}

float4 PSTextureToViewport(VS_TEXTURED_OUTPUT input) : SV_Target
{
	return float4(1.f,0.f,0.f,0.f);
}
//=========================================================================================
struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSSpriteAnimation(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;
	
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	
	if (texMat.z == 6 )//연기 
	{
		output.uv.x = (input.uv.x) / texMat.z + texMat.x;
		output.uv.y = input.uv.y / texMat.z + texMat.y;
	}
	else if (texMat.z == 4)//로딩 파티클
	{
		output.uv.x = (input.uv.x) / texMat.z + texMat.x;
		output.uv.y = input.uv.y / (texMat.z*1.5f) + texMat.y;
	}
	else//피 화면
		output.uv = input.uv;

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;

	if (texMat.z == 6)
	{
		if (cColor.x <= 0.05f&& cColor.y <= 0.05f && cColor.z <= 0.05f)
			discard;
		if (cColor.x > 0.25f && cColor.x <= 0.26f &&
			cColor.y > 0.25f && cColor.y <= 0.26f &&
			cColor.z > 0.25f && cColor.z <= 0.26f)
			discard;
	}
	else if (texMat.z == 1 || texMat.z == 4 || texMat.z == 3)
	{
		if(texMat.z == 3)
			cColor.a = 0.7f;

		if (cColor.x < 0.4f)
			discard;
	}

	return(cColor);
}



#define _WITH_2D_GAUSSIAN_BLUR
#define _WITH_GROUPSHARED_MEMORY

#ifdef _WITH_2D_GAUSSIAN_BLUR
groupshared float4 gf4GroupSharedCache[2 + 32 + 2][2 + 32 + 2];

static float gfGaussianBlurMask2D[5][5] = {
	{ 1.0f / 273.0f, 4.0f / 273.0f, 7.0f / 273.0f, 4.0f / 273.0f, 1.0f / 273.0f },
	{ 4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f },
	{ 7.0f / 273.0f, 26.0f / 273.0f, 41.0f / 273.0f, 26.0f / 273.0f, 7.0f / 273.0f },
	{ 4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f },
	{ 1.0f / 273.0f, 4.0f / 273.0f, 7.0f / 273.0f, 4.0f / 273.0f, 1.0f / 273.0f }
};

#define MotionBlurStrength 2.1f // 모션 블러 강도

[numthreads(32, 32, 1)]
//void CSGaussian2DBlur(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
//{
//	if ((n3DispatchThreadID.x < 2) || (n3DispatchThreadID.x >= int(gtxtInput.Length.x - 2)) || (n3DispatchThreadID.y < 2) || (n3DispatchThreadID.y >= int(gtxtInput.Length.y - 2)))
//	{
//		gtxtRWOutput[n3DispatchThreadID.xy] = gtxtInput[n3DispatchThreadID.xy];
//	}
//	else
//	{
//		// 모션 벡터 계산
//		float2 motionVector = gtxtInput[n3DispatchThreadID.xy].xy - gtxtPrevFrame[n3DispatchThreadID.xy].xy;
//		//normalize(round(motionVector));
//		motionVector *= MotionBlurStrength;
//
//		float4 f4Color = float4(0, 0, 0, 0);
//		
//		for (int i =-2; i <= 2; i++)
//		{
//			for (int j = -2; j <= 2; j++)
//			{
//				//float2 offset = float2(0, 0);
//				//if(motionVector.x>0 && motionVector.y>0)
//				//	offset = float2(i + 1, j + 1);// *motionVector;
//				//else if (motionVector.x <= 0 && motionVector.y <= 0)
//				//	offset = float2(i - 1, j - 1);// *motionVector;
//				//else if (motionVector.x > 0 && motionVector.y <= 0)
//				//	offset = float2(i + 1, j -1);// *motionVector;
//				//else if (motionVector.x <= 0 && motionVector.y > 0)
//				//	offset = float2(i - 1, j + 1);// *motionVector;
//				float2 offset = float2(i, j) * motionVector;
//				//float2 offset = float2( motionVector.x, motionVector.y);// *motionVector;
//				f4Color += gfGaussianBlurMask2D[i+2 ][j +2] * gtxtInput[n3DispatchThreadID.xy + offset];
//				//f4Color += gfGaussianBlurMask2D[i + 2][j + 2] * gtxtInput.Sample(gssWrap, gtxtInput[n3DispatchThreadID.xy].xy + offset);
//			}
//		}
//		gtxtRWOutput[n3DispatchThreadID.xy] = f4Color;
//	}
//}
void CSGaussian2DBlur(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
{
	if ((n3DispatchThreadID.x < 2) || (n3DispatchThreadID.x >= int(gtxtInput.Length.x - 2)) || (n3DispatchThreadID.y < 2) || (n3DispatchThreadID.y >= int(gtxtInput.Length.y - 2)))
	{
		gtxtRWOutput[n3DispatchThreadID.xy] = gtxtInput[n3DispatchThreadID.xy];
	}
	else
	{
		float4 f4Color = float4(0, 0, 0, 0);
		for (int i = -2; i <= 2; ++i)
		{
			for (int j = -2; j <= 2; ++j)
			{
				float2 offset = float2(i, j) * MotionBlurStrength;
				f4Color += gfGaussianBlurMask2D[i + 2][j + 2]/float(1.03) * gtxtInput[n3DispatchThreadID.xy + offset];
			}
		}

		gtxtRWOutput[n3DispatchThreadID.xy] = f4Color;
	}
}

#endif

Texture2D gtxtOutput : register(t1);


float4 PSTextureToFullScreen(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtInput.Sample(gssWrap, input.uv);
	float4 cEdgeColor = gtxtOutput.Sample(gssWrap, input.uv) * 1.25f;//gtxtPrevFrame
	//float4 cEdgeColor = gtxtPrevFrame.Sample(gssWrap, input.uv) * 1.25f;//gtxtPrevFrame

	return(cEdgeColor);
	//return(cColor * cEdgeColor);
	//return(cColor + cEdgeColor);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 


VS_TEXTURED_OUTPUT VSNewTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSNewTextured(VS_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID): SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtTextureArray.Sample(gssWrap, uvw);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


struct VS_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_LIGHTING_OUTPUT VSTexturedLighting(VS_TEXTURED_LIGHTING_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTexturedLighting(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID): SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtTextureArray.Sample(gssWrap, uvw);
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);

	return(cColor * cIllumination);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//



struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
	float4 color : SV_TARGET0;

	float4 cTexture : SV_TARGET1;
	float4 cIllumination : SV_TARGET2;
	float4 normal : SV_TARGET3;
	float zDepth : SV_TARGET4;
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTexturedLightingToMultipleRTs(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	output.cTexture = gtxtTextureArray.Sample(gssWrap, uvw);

	input.normalW = normalize(input.normalW);
	output.cIllumination = Lighting(input.positionW, input.normalW);

	output.color = output.cIllumination * output.cTexture;

	output.normal = float4(input.normalW.xyz * 0.5f + 0.5f, 1.0f);

	output.zDepth = input.position.z;

	return(output);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//
float4 VSPostProcessing(uint nVertexID : SV_VertexID): SV_POSITION
{
	if (nVertexID == 0)	return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 1)	return(float4(+1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 2)	return(float4(+1.0f, -1.0f, 0.0f, 1.0f));

	if (nVertexID == 3)	return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 4)	return(float4(+1.0f, -1.0f, 0.0f, 1.0f));
	if (nVertexID == 5)	return(float4(-1.0f, -1.0f, 0.0f, 1.0f));

	return(float4(0, 0, 0, 0));
}

float4 PSPostProcessing(float4 position : SV_POSITION): SV_Target
{
	return(float4(0.0f, 0.0f, 0.0f, 1.0f));
}


///////////////////////////////////////////////////////////////////////////////
//
struct VS_SCREEN_RECT_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_SCREEN_RECT_TEXTURED_OUTPUT VSScreenRectSamplingTextured(uint nVertexID : SV_VertexID)
{
	VS_SCREEN_RECT_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	else if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
							 
	else if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return(output);
}

float4 GetColorFromDepth(float fDepth)
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	if (fDepth >= 1.0f) cColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.00625f) cColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	else if (fDepth < 0.0125f) cColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.025f) cColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.05f) cColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.075f) cColor = float4(0.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.1f) cColor = float4(1.0f, 0.5f, 0.5f, 1.0f);
	else if (fDepth < 0.4f) cColor = float4(0.5f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.6f) cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.8f) cColor = float4(0.5f, 0.5f, 1.0f, 1.0f);
	else if (fDepth < 0.9f) cColor = float4(0.5f, 1.0f, 0.5f, 1.0f);
	else cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	return(cColor);
}

float4 PSScreenRectSamplingTextured(VS_TEXTURED_OUTPUT input): SV_Target
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	switch (gvDrawOptions.x)
	{
		case 84: //'T'
		{
			cColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
			break;
		}
		case 89: //'Y'
		{
			cColor = gtxtIlluminationTexture.Sample(gssWrap, input.uv);
			break;
		}
		case 85: //'U'
		{
			cColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
			break;
		}
		case 73: //'I'
		{
			float fDepth = gtxtDepthTexture.Load(uint3((uint)input.position.x, (uint)input.position.y, 0));
			cColor = fDepth;
//			cColor = GetColorFromDepth(fDepth);
			break; 
		}
		case 79: //'O'
		{
			float fzDepth = gtxtzDepthTexture.Load(uint3((uint)input.position.x, (uint)input.position.y, 0));
			cColor = fzDepth;
//			cColor = GetColorFromDepth(fDepth);
			break;
		}
	}
	return(cColor);
}
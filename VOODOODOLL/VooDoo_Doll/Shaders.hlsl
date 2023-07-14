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

struct CB_TOOBJECTSPACE
{
	matrix		mtxToTexture;
	float4		f4Position;
};

cbuffer cbToLightSpace : register(b6)
{
	CB_TOOBJECTSPACE gcbToLightSpaces[MAX_SHADOW_LIGHTS];
};

#define NUM_RENDER_TARGETS 5

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
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);

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

struct PS_STANDARD_OUTPUT
{
	  float4 renderTarget[NUM_RENDER_TARGETS] : SV_Target;
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

PS_STANDARD_OUTPUT PSStandard(VS_STANDARD_OUTPUT input)
{
	PS_STANDARD_OUTPUT output;

     for (int i = 0; i < NUM_RENDER_TARGETS; i++)
    {
        output.renderTarget[i] = float4(0.0f, 0.0f, 0.0f, 1.0f);
    }

	if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
		output.renderTarget[0] = gtxtAlbedoTexture.Sample(gssWrap, input.uv);

	if (gnTexturesMask & MATERIAL_SPECULAR_MAP)
		output.renderTarget[1] = gtxtSpecularTexture.Sample(gssWrap, input.uv);

	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
		output.renderTarget[2] = gtxtNormalTexture.Sample(gssWrap, input.uv);

	if (gnTexturesMask & MATERIAL_METALLIC_MAP)
		output.renderTarget[3] = gtxtMetallicTexture.Sample(gssWrap, input.uv);

	if (gnTexturesMask & MATERIAL_EMISSION_MAP)
		output.renderTarget[4] = gtxtEmissionTexture.Sample(gssWrap, input.uv);

	return output;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// 기본 셰이더로 변경 예정 

struct GBuffer
{
    float4 albedo : SV_Target0;
    float4 specular : SV_Target1;
    float4 normal : SV_Target2;
    float4 metallic : SV_Target3;
    float4 emission : SV_Target4;
};

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

struct PS_STANDARD_OUTPUT
{
	GBuffer gbuffer : SV_Target;
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

PS_STANDARD_OUTPUT PSStandard(VS_STANDARD_OUTPUT input)
{
	  PS_STANDARD_OUTPUT output;

    output.gbuffer.albedo = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.gbuffer.specular = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.gbuffer.normal = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.gbuffer.metallic = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.gbuffer.emission = float4(0.0f, 0.0f, 0.0f, 1.0f);

    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
        output.gbuffer.albedo = gtxtAlbedo.Sample(gssWrap, input.uv);

    if (gnTexturesMask & MATERIAL_SPECULAR_MAP)
        output.gbuffer.specular = gtxtSpecular.Sample(gssWrap, input.uv);

    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
        output.gbuffer.normal = gtxtNormal.Sample(gssWrap, input.uv);

    if (gnTexturesMask & MATERIAL_METALLIC_MAP)
        output.gbuffer.metallic = gtxtMetallic.Sample(gssWrap, input.uv);

    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
        output.gbuffer.emission = gtxtEmission.Sample(gssWrap, input.uv);

    return output;
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
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
		//		mtxVertexToBoneWorld += input.weights[i] * gpmtxBoneTransforms[input.indices[i]];
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


	for (int i = 0; i < MAX_SHADOW_LIGHTS; i++)
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
	
	if (texMat.z ==8 || texMat.z == 6)//연기 파티클
	{
		output.uv.x = (input.uv.x) / texMat.z + texMat.x;
		output.uv.y = input.uv.y / texMat.z + texMat.y;
	}
	else if (texMat.z == 4)//로딩
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

	if (texMat.z == 8 || texMat.z == 6)
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


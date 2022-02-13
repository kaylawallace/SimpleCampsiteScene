//
// Skybox effect common
// Includes information used by both skybox_vs and skybox_ps
// 
// All skybox files adapted from https://github.com/microsoft/DirectXTK/wiki/Authoring-an-Effect

#ifndef __SKYBOXEFFECT_COMMON_HLSLI__
#define __SKYBOXEFFECT_COMMON_HLSLI__

cbuffer SkyboxConstants : register(b0)
{
    float4x4 WorldViewProjection;
}

struct OutputType
{
    float3 texCoord : TEXCOORD0;
    float4 pos : SV_Position;
};

#endif

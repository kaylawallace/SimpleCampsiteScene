//
// Skybox pixel shader 
//

#include "skybox_common.hlsli"

TextureCube<float4> CubeMap : register(t0);
SamplerState Sampler : register(s0);

struct InputType
{
    float3 texCoord : TEXCOORD0;
};
float4 main(InputType input) : SV_TARGET0
{
    return CubeMap.Sample(Sampler, normalize(input.texCoord));
}
    
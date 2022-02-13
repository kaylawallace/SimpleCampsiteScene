//
// Skybox vertex shader 
// Calculates the texture coordinates on the assumption that the skybox is centred around the origin 
// Uses vertex position information to do this 
//

#include "skybox_common.hlsli"

struct InputType
{
    float4 pos : SV_Position;
};

OutputType main(InputType input)
{
    OutputType output;
    
    output.pos = mul(input.pos, WorldViewProjection).xyww; // Set z so that it is drawn on the far plane  
    output.texCoord.xyz = input.pos.xyz;
    
    return output;
}
    
#pragma once
#include "pch.h"

//
// Skybox effect class 
// Adapted from  https://github.com/microsoft/DirectXTK/wiki/Authoring-an-Effect
//

#include "pch.h"
#include "SkyboxEffect.h"
#include "DirectXHelpers.h"
#include "GraphicsMemory.h"
#include "ReadData.h"
#include <stdexcept>


using namespace DirectX;
using namespace DX;

namespace
{
    constexpr uint32_t DirtyConstantBuffer = 0x1;
    constexpr uint32_t DirtyWVPMatrix = 0x2;
}

SkyboxEffect::SkyboxEffect(_In_ ID3D11Device* device) :
    m_dirtyFlags(uint32_t(-1)),
    m_constBuffer(device)
{
    static_assert((sizeof(SkyboxEffect::SkyboxEffectConstants) % 16) == 0, "CB size alignment");

    // Get shaders
    m_vsBlob = DX::ReadData(L"skybox_vs.cso");
    DX::ThrowIfFailed(device->CreateVertexShader(m_vsBlob.data(), m_vsBlob.size(), nullptr, m_vs.ReleaseAndGetAddressOf()));

    auto psBlob = DX::ReadData(L"skybox_ps.cso");
    DX::ThrowIfFailed(device->CreatePixelShader(psBlob.data(), psBlob.size(), nullptr, m_ps.ReleaseAndGetAddressOf()));
}


// IEffect methods.
void SkyboxEffect::Apply(_In_ ID3D11DeviceContext* context)
{
    if (m_dirtyFlags & DirtyWVPMatrix)
    {
        // Skybox doesn't use m_world matrix and ignores the translation of m_view
        XMMATRIX view = m_view;
        view.r[3] = g_XMIdentityR3;
        m_wvp = XMMatrixMultiply(view, m_proj);

        // Set the dirty bit to indicate the WVP matrix has changed
        m_dirtyFlags &= ~DirtyWVPMatrix;
        m_dirtyFlags |= DirtyConstantBuffer;
    }
    if (m_dirtyFlags & DirtyConstantBuffer)
    {
        // Define the skybox effect constants
        SkyboxEffectConstants constants;
        constants.worldViewProj = XMMatrixTranspose(m_wvp);
        m_constBuffer.SetData(context, constants);

        m_dirtyFlags &= ~DirtyConstantBuffer;
    }
    // Create the constant buffers for the vertex and pixel shaders
    auto constBuf = m_constBuffer.GetBuffer();
    context->VSSetConstantBuffers(0, 1, &constBuf);
    context->PSSetShaderResources(0, 1, m_texture.GetAddressOf());

    // Set each shader 
    context->VSSetShader(m_vs.Get(), nullptr, 0);
    context->PSSetShader(m_ps.Get(), nullptr, 0);
}

void SkyboxEffect::GetVertexShaderBytecode(_Out_ void const** pShaderByteCode, _Out_ size_t* pByteCodeLength)
{
    assert(pShaderByteCode != nullptr && pByteCodeLength != nullptr);
    *pShaderByteCode = m_vsBlob.data();
    *pByteCodeLength = m_vsBlob.size();
}


// Camera settings.
void SkyboxEffect::SetWorld(FXMMATRIX /*value*/)
{
}

// Set view matrix & assign the dirty bit 
void SkyboxEffect::SetView(FXMMATRIX value)
{
    m_view = value;
    m_dirtyFlags |= DirtyWVPMatrix;
}

// Set projection matrix & assign the dirty bit 
void SkyboxEffect::SetProjection(FXMMATRIX value)
{
    m_proj = value;
    m_dirtyFlags |= DirtyWVPMatrix;
}

// Set the view and projection matrices (skybox doesn't use world matrix) & assign the dirty bit 
void SkyboxEffect::SetMatrices(FXMMATRIX /*world*/, CXMMATRIX view, CXMMATRIX projection)
{
    m_view = view;
    m_proj = projection;
    m_dirtyFlags |= DirtyWVPMatrix;
}


// Texture settings.
void SkyboxEffect::SetTexture(_In_opt_ ID3D11ShaderResourceView* value)
{
    m_texture = value;
}

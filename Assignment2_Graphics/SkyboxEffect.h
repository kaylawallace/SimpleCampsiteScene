#pragma once


//
// Skybox Effect header 
//

#include "pch.h"
#include <vector>

namespace DX
{
    class SkyboxEffect : public DirectX::IEffect, public DirectX::IEffectMatrices
    {
    public:
        explicit SkyboxEffect(ID3D11Device* device);

        void XM_CALLCONV SetWorld(DirectX::FXMMATRIX value) override;
        void XM_CALLCONV SetView(DirectX::FXMMATRIX value) override;
        void XM_CALLCONV SetProjection(DirectX::FXMMATRIX value) override;
        void XM_CALLCONV SetMatrices(DirectX::FXMMATRIX world, DirectX::CXMMATRIX view, DirectX::CXMMATRIX projection) override;

        virtual void __cdecl Apply(ID3D11DeviceContext* context);

        virtual void __cdecl GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength);

        void SetTexture(ID3D11ShaderResourceView* val);

    private:
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
        std::vector<uint8_t> m_vsBlob;

        // Matrices for calculations (minus world as skyboxes do not require the world matrix)
        DirectX::SimpleMath::Matrix m_view;
        DirectX::SimpleMath::Matrix m_proj;
        DirectX::SimpleMath::Matrix m_wvp; // WorldViewProjection matrix 

        uint32_t m_dirtyFlags;
        struct __declspec(align(16)) SkyboxEffectConstants
        {
            DirectX::XMMATRIX worldViewProj;
        };

        DirectX::ConstantBuffer<SkyboxEffectConstants> m_constBuffer;
    };
}

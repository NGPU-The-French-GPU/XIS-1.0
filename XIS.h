#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <string>

class XIS {
public:
    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height);
    void Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView* sceneSRV);
    void Cleanup();

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_IntermediateTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_IntermediateRTV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_IntermediateSRV;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerState;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_FullscreenQuadVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_BicubicUpsamplePS;

    void CreateIntermediateRenderTarget(ID3D11Device* device, int width, int height);
    void LoadShaders(ID3D11Device* device);
    void LoadSettings(int& width, int& height, float& upscalePercentage); // Lecture des fichiers JSON
};

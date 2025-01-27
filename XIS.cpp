#include "XIS.h"
#include <d3dcompiler.h>

bool XIS::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height) {
    CreateIntermediateRenderTarget(device, width, height);
    LoadShaders(device);

    // Create sampler state
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    device->CreateSamplerState(&samplerDesc, m_SamplerState.GetAddressOf());

    return true;
}

void XIS::CreateIntermediateRenderTarget(ID3D11Device* device, int width, int height) {
    // Create texture
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width / 2; // Render at half resolution
    textureDesc.Height = height / 2;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    device->CreateTexture2D(&textureDesc, nullptr, m_IntermediateTexture.GetAddressOf());

    // Create render target view
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = textureDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    device->CreateRenderTargetView(m_IntermediateTexture.Get(), &rtvDesc, m_IntermediateRTV.GetAddressOf());

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(m_IntermediateTexture.Get(), &srvDesc, m_IntermediateSRV.GetAddressOf());
}

void XIS::LoadShaders(ID3D11Device* device) {
    // Compile and load shaders
    ID3DBlob* vsBlob;
    D3DCompileFromFile(L"BicubicUpscale.hlsl", nullptr, nullptr, "FullscreenQuadVS", "vs_5_0", 0, 0, &vsBlob, nullptr);
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_FullscreenQuadVS.GetAddressOf());

    ID3DBlob* psBlob;
    D3DCompileFromFile(L"BicubicUpscale.hlsl", nullptr, nullptr, "BicubicUpsamplePS", "ps_5_0", 0, 0, &psBlob, nullptr);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_BicubicUpsamplePS.GetAddressOf());
}

void XIS::Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView* sceneSRV) {
    // Bind intermediate render target
    ID3D11RenderTargetView* rtvs[] = { m_IntermediateRTV.Get() };
    context->OMSetRenderTargets(1, rtvs, nullptr);

    // Render scene at low resolution
    context->PSSetShaderResources(0, 1, &sceneSRV);
    // [Call your scene rendering function here]

    // Apply bicubic upscaling
    context->OMSetRenderTargets(1, nullptr, nullptr); // Switch back to the screen render target
    context->PSSetShaderResources(0, 1, m_IntermediateSRV.GetAddressOf());
    context->VSSetShader(m_FullscreenQuadVS.Get(), nullptr, 0);
    context->PSSetShader(m_BicubicUpsamplePS.Get(), nullptr, 0);
    context->Draw(6, 0); // Render fullscreen quad
}

void XIS::Cleanup() {
    m_IntermediateTexture.Reset();
    m_IntermediateRTV.Reset();
    m_IntermediateSRV.Reset();
    m_FullscreenQuadVS.Reset();
    m_BicubicUpsamplePS.Reset();
    m_SamplerState.Reset();
}

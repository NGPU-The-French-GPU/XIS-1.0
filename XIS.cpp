#include "XIS.h"
#include <d3dcompiler.h>
#include <fstream>
#include <nlohmann/json.hpp> // Bibliothèque JSON
using json = nlohmann::json;

bool XIS::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height) {
    float upscalePercentage = 1.0f;
    LoadSettings(width, height, upscalePercentage); // Charger les paramètres JSON

    // Calculer la résolution ajustée
    int scaledWidth = static_cast<int>(width * upscalePercentage);
    int scaledHeight = static_cast<int>(height * upscalePercentage);

    CreateIntermediateRenderTarget(device, scaledWidth, scaledHeight);
    LoadShaders(device);

    // Créer un sampler state
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    device->CreateSamplerState(&samplerDesc, m_SamplerState.GetAddressOf());

    return true;
}

void XIS::CreateIntermediateRenderTarget(ID3D11Device* device, int width, int height) {
    // Créer une texture intermédiaire
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    device->CreateTexture2D(&textureDesc, nullptr, m_IntermediateTexture.GetAddressOf());

    // Créer un Render Target View
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = textureDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    device->CreateRenderTargetView(m_IntermediateTexture.Get(), &rtvDesc, m_IntermediateRTV.GetAddressOf());

    // Créer un Shader Resource View
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(m_IntermediateTexture.Get(), &srvDesc, m_IntermediateSRV.GetAddressOf());
}

void XIS::LoadShaders(ID3D11Device* device) {
    // Compiler et charger les shaders
    ID3DBlob* vsBlob;
    D3DCompileFromFile(L"BicubicUpscale.hlsl", nullptr, nullptr, "FullscreenQuadVS", "vs_5_0", 0, 0, &vsBlob, nullptr);
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_FullscreenQuadVS.GetAddressOf());

    ID3DBlob* psBlob;
    D3DCompileFromFile(L"BicubicUpscale.hlsl", nullptr, nullptr, "BicubicUpsamplePS", "ps_5_0", 0, 0, &psBlob, nullptr);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_BicubicUpsamplePS.GetAddressOf());
}

void XIS::LoadSettings(int& width, int& height, float& upscalePercentage) {
    // Lire les paramètres de résolution
    std::ifstream resFile("res.json");
    if (resFile.is_open()) {
        json resJson;
        resFile >> resJson;
        width = resJson.value("width", 1920); // Par défaut 1920 si absent
        height = resJson.value("height", 1080); // Par défaut 1080 si absent
    } else {
        width = 1920; // Valeur par défaut si le fichier est absent
        height = 1080;
    }

    // Lire les paramètres d'upscaling
    std::ifstream usresFile("usres.json");
    if (usresFile.is_open()) {
        json usresJson;
        usresFile >> usresJson;
        upscalePercentage = usresJson.value("upscale_percentage", 100.0f) / 100.0f;
    } else {
        upscalePercentage = 1.0f; // 100% par défaut
    }
}

void XIS::Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView* sceneSRV) {
    // Attacher le Render Target View intermédiaire
    ID3D11RenderTargetView* rtvs[] = { m_IntermediateRTV.Get() };
    context->OMSetRenderTargets(1, rtvs, nullptr);

    // Rendu de la scène à basse résolution
    context->PSSetShaderResources(0, 1, &sceneSRV);
    // [Appeler ici votre fonction de rendu de la scène]

    // Appliquer le suréchantillonnage bicubique
    context->OMSetRenderTargets(1, nullptr, nullptr); // Revenir au Render Target principal
    context->PSSetShaderResources(0, 1, m_IntermediateSRV.GetAddressOf());
    context->VSSetShader(m_FullscreenQuadVS.Get(), nullptr, 0);
    context->PSSetShader(m_BicubicUpsamplePS.Get(), nullptr, 0);
    context->Draw(6, 0); // Affichage en plein écran
}

void XIS::Cleanup() {
    m_IntermediateTexture.Reset();
    m_IntermediateRTV.Reset();
    m_IntermediateSRV.Reset();
    m_FullscreenQuadVS.Reset();
    m_BicubicUpsamplePS.Reset();
    m_SamplerState.Reset();
}

#include "Shader.h"
#include <d3dcompiler.h>

Shader::Shader() : m_vertexShader(nullptr), m_pixelShader(nullptr), m_layout(nullptr) {}

Shader::~Shader() {
    if (m_layout) m_layout->Release();
    if (m_pixelShader) m_pixelShader->Release();
    if (m_vertexShader) m_vertexShader->Release();
}

bool Shader::CompileShader(const WCHAR* filename, const char* entryPoint, const char* shaderModel, ID3DBlob** outBlob) {
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(filename, nullptr, nullptr, entryPoint, shaderModel, 0, 0, outBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) errorBlob->Release();
        return false;
    }
    return true;
}

bool Shader::Initialize(ID3D11Device* device) {
    ID3DBlob* vertexBlob = nullptr;
    if (!CompileShader(L"shader.fx", "VSMain", "vs_4_0", &vertexBlob)) {
        return false;
    }
    
    device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &m_vertexShader);
    
    ID3DBlob* pixelBlob = nullptr;
    if (!CompileShader(L"shader.fx", "PSMain", "ps_4_0", &pixelBlob)) {
        return false;
    }
    
    device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &m_pixelShader);
    
    vertexBlob->Release();
    
    return true;
}

void Shader::Render(ID3D11DeviceContext* context) {
    context->VSSetShader(m_vertexShader, nullptr, 0);
    context->PSSetShader(m_pixelShader, nullptr, 0);
}

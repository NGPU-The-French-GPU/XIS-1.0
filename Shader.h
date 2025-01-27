#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

class Shader {
public:
    Shader();
    ~Shader();

    bool CompileShader(const WCHAR* filename, const char* entryPoint, const char* shaderModel, ID3DBlob** outBlob);
    bool Initialize(ID3D11Device* device);
    void Render(ID3D11DeviceContext* context);
    
private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
};

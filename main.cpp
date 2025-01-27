#include <d3d11.h>
#include <DirectXMath.h>
#include <Windows.h>
#include <chrono>
#include <string>
#include <sstream>
#include <d3dcompiler.h>
#include "Shader.h"

using namespace DirectX;

// DirectX 11 Variables
ID3D11Device* g_device = nullptr;
ID3D11DeviceContext* g_context = nullptr;
IDXGISwapChain* g_swapChain = nullptr;
ID3D11RenderTargetView* g_renderTargetView = nullptr;
ID3D11DepthStencilView* g_depthStencilView = nullptr;
ID3D11Buffer* g_vertexBuffer = nullptr;
ID3D11Buffer* g_indexBuffer = nullptr;

XMMATRIX g_worldMatrix;
XMMATRIX g_viewMatrix;
XMMATRIX g_projectionMatrix;

// FPS variables
std::chrono::high_resolution_clock::time_point g_lastFrameTime;
int g_frameCount = 0;
float g_fps = 0.0f;

// Cube rotation variables
float g_rotationX = 0.0f;
float g_rotationY = 0.0f;
float g_rotationZ = 0.0f;

// XIS status
bool g_xisEnabled = true;

// Create a simple cube
struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

Vertex g_vertices[] = {
    { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
    { XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
    { XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
    { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
    { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
    { XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
    { XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
    { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f) }
};

unsigned int g_indices[] = {
    0, 1, 2, 0, 2, 3, // Front face
    4, 5, 6, 4, 6, 7, // Back face
    4, 5, 1, 4, 1, 0, // Bottom face
    3, 2, 6, 3, 6, 7, // Top face
    0, 3, 7, 0, 7, 4, // Left face
    1, 2, 6, 1, 6, 5  // Right face
};

// Window procedure to process messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SIZE:
        if (g_device) {
            // Handle window resizing
        }
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void InitializeDirectX(HWND hwnd) {
    // Create a DirectX device, context, swap chain, etc.

    D3D_FEATURE_LEVEL featureLevel;
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = 800;
    swapChainDesc.BufferDesc.Height = 600;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    // Create device, context, and swap chain
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, &g_swapChain, &g_device, &featureLevel, &g_context);

    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create device and swap chain", L"Error", MB_OK);
        exit(1);
    }

    // Create render target view and depth stencil view
    ID3D11Texture2D* pBackBuffer;
    g_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_device->CreateRenderTargetView(pBackBuffer, nullptr, &g_renderTargetView);
    pBackBuffer->Release();

    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = 800;
    depthStencilDesc.Height = 600;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* pDepthStencil;
    g_device->CreateTexture2D(&depthStencilDesc, nullptr, &pDepthStencil);
    g_device->CreateDepthStencilView(pDepthStencil, nullptr, &g_depthStencilView);
    pDepthStencil->Release();
}

void UpdateFPS() {
    auto now = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(now - g_lastFrameTime).count();
    g_lastFrameTime = now;

    g_frameCount++;
    if (deltaTime >= 1.0f) {
        g_fps = g_frameCount / deltaTime;
        g_frameCount = 0;
    }
}

void Render() {
    g_context->ClearRenderTargetView(g_renderTargetView, Colors::MidnightBlue);
    g_context->ClearDepthStencilView(g_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Cube rotation logic
    g_rotationX += 0.01f;
    g_rotationY += 0.01f;
    g_rotationZ += 0.01f;

    // Apply world transformation
    g_worldMatrix = XMMatrixRotationX(g_rotationX) * XMMatrixRotationY(g_rotationY) * XMMatrixRotationZ(g_rotationZ);

    // Update FPS counter
    UpdateFPS();

    // Display FPS and XIS status
    DisplayText("FPS: " + std::to_string(g_fps), 10, 10);
    DisplayText("XIS: " + std::string(g_xisEnabled ? "Activé" : "Désactivé"), 700, 10);

    // Present the scene
    g_swapChain->Present(0, 0);
}

int main() {
    HWND hwnd = CreateWindowEx(0, L"WindowClass", L"DirectX Test", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, GetModuleHandle(0), nullptr);

    InitializeDirectX(hwnd);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Main message loop
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Render();
        }
    }

    return 0;
}

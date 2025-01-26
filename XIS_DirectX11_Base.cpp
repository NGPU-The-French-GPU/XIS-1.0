#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <iostream>
#include <fstream>
#include <string>
#include <json/json.h>
#include <windows.h>
#include <chrono>

// Global variables
IDXGISwapChain* g_swapChain = nullptr;
ID3D11Device* g_device = nullptr;
ID3D11DeviceContext* g_deviceContext = nullptr;
ID3D11RenderTargetView* g_renderTargetView = nullptr;
ID3D11DepthStencilView* g_depthStencilView = nullptr;
ID3D11Buffer* g_vertexBuffer = nullptr;
ID3D11Buffer* g_indexBuffer = nullptr;
ID3D11VertexShader* g_vertexShader = nullptr;
ID3D11PixelShader* g_pixelShader = nullptr;
ID3D11InputLayout* g_inputLayout = nullptr;
float g_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Default background color (black)

// FPS counter variables
std::chrono::steady_clock::time_point g_lastTime;
int g_frameCount = 0;
float g_fps = 0.0f;

// Fonction pour charger la résolution depuis le fichier res.json
std::string loadResolutionFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    Json::Value jsonData;
    file >> jsonData;
    return jsonData["resolution"].asString();
}

// Fonction pour appliquer la résolution
void applyResolution(int width, int height) {
    // Ici tu peux redimensionner la fenêtre DirectX ou utiliser cette info
    // Pour simplifier, cette partie peut juste être une indication de changement de résolution.
    std::cout << "Resolution set to: " << width << "x" << height << std::endl;
}

// Fonction pour initialiser DirectX
bool initDirectX(HWND hwnd) {
    // Création de la chaîne de swap et du périphérique DirectX 11
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = 1920;
    scd.BufferDesc.Height = 1080;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &g_swapChain, &g_device, nullptr, &g_deviceContext);
    if (FAILED(hr)) {
        std::cerr << "DirectX Device creation failed!" << std::endl;
        return false;
    }

    // Création de la cible de rendu
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_device->CreateRenderTargetView(pBackBuffer, nullptr, &g_renderTargetView);
    pBackBuffer->Release();

    // Création de la vue de profondeur
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = 1920;
    depthDesc.Height = 1080;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* pDepthStencil = nullptr;
    g_device->CreateTexture2D(&depthDesc, nullptr, &pDepthStencil);
    g_device->CreateDepthStencilView(pDepthStencil, nullptr, &g_depthStencilView);
    pDepthStencil->Release();

    g_deviceContext->OMSetRenderTargets(1, &g_renderTargetView, g_depthStencilView);

    // Configuration des propriétés de la vue (Viewport)
    D3D11_VIEWPORT viewport = {};
    viewport.Width = 1920.0f;
    viewport.Height = 1080.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    g_deviceContext->RSSetViewports(1, &viewport);

    return true;
}

// Fonction pour afficher les FPS
void displayFPS(HWND hwnd) {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> duration = now - g_lastTime;

    g_frameCount++;
    if (duration.count() >= 1.0f) {
        g_fps = g_frameCount / duration.count();
        g_frameCount = 0;
        g_lastTime = now;
        std::cout << "FPS: " << g_fps << std::endl;
    }
}

// Fonction pour dessiner la scène (simple cube)
void drawScene() {
    // Clear screen
    g_deviceContext->ClearRenderTargetView(g_renderTargetView, g_clearColor);
    g_deviceContext->ClearDepthStencilView(g_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Dessiner des objets 3D ici, par exemple un cube ou une simple géométrie

    // Swap buffers
    g_swapChain->Present(0, 0);
}

// Fonction principale
int main() {
    // Initialisation de la fenêtre Windows
    HWND hwnd = CreateWindowEx(0, L"STATIC", L"DirectX 11", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1920, 1080, nullptr, nullptr, nullptr, nullptr);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Initialiser DirectX 11
    if (!initDirectX(hwnd)) {
        return -1;
    }

    // Charger la résolution depuis le fichier JSON
    std::string resolution = loadResolutionFromFile("res.json");

    // Appliquer la résolution
    applyResolution(1920, 1080); // Résolution par défaut

    // FPS timer
    g_lastTime = std::chrono::steady_clock::now();

    // Boucle principale
    bool xisEnabled = false;
    while (true) {
        MSG msg = {};
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Logique d'activation/désactivation de XIS
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            xisEnabled = !xisEnabled;
            std::cout << "XIS " << (xisEnabled ? "activé" : "désactivé") << std::endl;
        }

        // Affichage des FPS
        displayFPS(hwnd);

        // Dessiner la scène
        drawScene();
    }

    // Libérer les ressources DirectX
    if (g_deviceContext) g_deviceContext->ClearState();
    if (g_renderTargetView) g_renderTargetView->Release();
    if (g_depthStencilView) g_depthStencilView->Release();
    if (g_deviceContext) g_deviceContext->Release();
    if (g_device) g_device->Release();
    if (g_swapChain) g_swapChain->Release();

    return 0;
}

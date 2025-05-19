#include "BicubicUpscaler.h"
#include "../Utils/Logger.h"
#include "../Renderer/IRenderer.h"
#include "../Shaders/ShaderManager.h"
#include <memory>

namespace XIS {

struct BicubicUpscaler::BicubicUpscalerData {
    bool initialized;
    
    // Shader resources
    void* bicubicShader;
    
    // Constant buffer for bicubic parameters
    struct BicubicConstants {
        int inputWidth;
        int inputHeight;
        int outputWidth;
        int outputHeight;
        float sharpnessFactor;  // Controls the 'a' parameter of bicubic (-0.5 to -1.0)
        float padding[3];
    };
    
    void* constantBuffer;
    
    // Weight buffer for precomputed bicubic weights
    void* weightBuffer;
};

BicubicUpscaler::BicubicUpscaler() 
    : m_data(std::make_unique<BicubicUpscalerData>())
{
    m_data->initialized = false;
    m_data->bicubicShader = nullptr;
    m_data->constantBuffer = nullptr;
    m_data->weightBuffer = nullptr;
}

BicubicUpscaler::~BicubicUpscaler() {
    Shutdown();
}

bool BicubicUpscaler::Initialize(const XISContext* context) {
    if (!context) {
        Logger::Error("BicubicUpscaler: Invalid context provided");
        return false;
    }
    
    // Initialize shaders
    if (!InitializeShaders(context)) {
        Logger::Error("BicubicUpscaler: Failed to initialize shaders");
        return false;
    }
    
    // Create resources
    if (!CreateResources(context)) {
        Logger::Error("BicubicUpscaler: Failed to create resources");
        return false;
    }
    
    m_data->initialized = true;
    Logger::Info("BicubicUpscaler: Successfully initialized");
    return true;
}

void BicubicUpscaler::Shutdown() {
    if (!m_data->initialized) {
        return;
    }
    
    // Release shader resources
    if (m_data->bicubicShader) {
        m_data->bicubicShader = nullptr;
    }
    
    // Release constant buffer
    if (m_data->constantBuffer) {
        m_data->constantBuffer = nullptr;
    }
    
    // Release weight buffer
    if (m_data->weightBuffer) {
        m_data->weightBuffer = nullptr;
    }
    
    m_data->initialized = false;
    Logger::Info("BicubicUpscaler: Successfully shut down");
}

bool BicubicUpscaler::Upscale(
    const XISContext* context,
    void* inputTexture,
    void* outputTexture,
    int inputWidth,
    int inputHeight,
    int outputWidth,
    int outputHeight,
    float sharpnessFactor) {
    
    if (!m_data->initialized) {
        Logger::Error("BicubicUpscaler: Not initialized");
        return false;
    }
    
    if (!inputTexture || !outputTexture) {
        Logger::Error("BicubicUpscaler: Invalid input or output texture");
        return false;
    }
    
    IRenderer* renderer = context->GetRenderer();
    
    // Update constant buffer with upscaling parameters
    BicubicUpscalerData::BicubicConstants constants;
    constants.inputWidth = inputWidth;
    constants.inputHeight = inputHeight;
    constants.outputWidth = outputWidth;
    constants.outputHeight = outputHeight;
    
    // Clamp sharpness factor to valid range (-1.0 to -0.5)
    // -0.5 is smoother (Mitchell), -1.0 is sharper (Spline)
    sharpnessFactor = std::max(-1.0f, std::min(-0.5f, sharpnessFactor));
    constants.sharpnessFactor = sharpnessFactor;
    
    if (!renderer->UpdateConstantBuffer(m_data->constantBuffer, &constants, sizeof(constants))) {
        Logger::Error("BicubicUpscaler: Failed to update constant buffer");
        return false;
    }
    
    // Calculate and update bicubic filter weights
    CalculateWeights(sharpnessFactor);
    
    // Set shader resources
    renderer->SetComputeShader(m_data->bicubicShader);
    renderer->SetComputeConstantBuffer(0, m_data->constantBuffer);
    renderer->SetComputeShaderResource(0, inputTexture);
    renderer->SetComputeShaderResource(1, m_data->weightBuffer);
    renderer->SetComputeUnorderedAccessView(0, outputTexture);
    
    // Calculate dispatch dimensions
    // Use 8x8 thread groups (64 threads per group)
    int dispatchX = (outputWidth + 7) / 8;
    int dispatchY = (outputHeight + 7) / 8;
    
    // Dispatch compute shader
    renderer->DispatchCompute(dispatchX, dispatchY, 1);
    
    // Wait for compute to finish
    renderer->SyncCompute();
    
    return true;
}

bool BicubicUpscaler::InitializeShaders(const XISContext* context) {
    ShaderManager* shaderManager = context->GetShaderManager();
    if (!shaderManager) {
        Logger::Error("BicubicUpscaler: Failed to get shader manager");
        return false;
    }
    
    // Load bicubic upscaling compute shader
    m_data->bicubicShader = shaderManager->LoadComputeShader(
        "BicubicUpscale.hlsl", 
        "BicubicUpscaleCS", 
        "cs_5_0"
    );
    
    if (!m_data->bicubicShader) {
        Logger::Error("BicubicUpscaler: Failed to load bicubic upscale shader");
        return false;
    }
    
    return true;
}

bool BicubicUpscaler::CreateResources(const XISContext* context) {
    IRenderer* renderer = context->GetRenderer();
    if (!renderer) {
        Logger::Error("BicubicUpscaler: Failed to get renderer");
        return false;
    }
    
    // Create constant buffer
    BicubicUpscalerData::BicubicConstants constants;
    constants.inputWidth = 0;   // Will be updated in Upscale()
    constants.inputHeight = 0;
    constants.outputWidth = 0;
    constants.outputHeight = 0;
    constants.sharpnessFactor = -0.5f; // Default: Mitchell filter (balanced)
    
    m_data->constantBuffer = renderer->CreateConstantBuffer(
        sizeof(BicubicUpscalerData::BicubicConstants),
        &constants,
        "BicubicConstantBuffer"
    );
    
    if (!m_data->constantBuffer) {
        Logger::Error("BicubicUpscaler: Failed to create constant buffer");
        return false;
    }
    
    // Create buffer for precalculated bicubic weights
    // We store 4 weights for each of 256 fractional positions (1024 floats)
    const int WEIGHT_COUNT = 1024; // 256 * 4
    
    m_data->weightBuffer = renderer->CreateStructuredBuffer(
        WEIGHT_COUNT,
        sizeof(float),
        false, // No UAV needed, read-only
        "BicubicWeightBuffer"
    );
    
    if (!m_data->weightBuffer) {
        Logger::Error("BicubicUpscaler: Failed to create weight buffer");
        return false;
    }
    
    // Initialize weights with default value
    CalculateWeights(-0.5f); // Mitchell filter by default
    
    return true;
}

void BicubicUpscaler::CalculateWeights(float a) {
    // Bicubic weight function with parameter 'a'
    // Common values: -0.5 for Mitchell filter, -0.75 for Catmull-Rom, -1.0 for B-Spline
    auto bicubicWeight = [a](float x) -> float {
        x = std::abs(x);
        
        if (x < 1.0f) {
            return ((a + 2.0f) * x - (a + 3.0f)) * x * x + 1.0f;
        } else if (x < 2.0f) {
            return ((a * x - 5.0f * a) * x + 8.0f * a) * x - 4.0f * a;
        } else {
            return 0.0f;
        }
    };
    
    // Compute weights for 256 fractional positions
    const int PRECISION = 256;
    const int WEIGHT_COUNT = PRECISION * 4; // 4 weights per position
    
    std::vector<float> weights(WEIGHT_COUNT);
    
    for (int i = 0; i < PRECISION; ++i) {
        float frac = static_cast<float>(i) / PRECISION;
        
        // Calculate weights for samples at [-1, 0, 1, 2] relative to frac
        weights[i * 4 + 0] = bicubicWeight(1.0f + frac);    // Sample at -1
        weights[i * 4 + 1] = bicubicWeight(frac);           // Sample at 0
        weights[i * 4 + 2] = bicubicWeight(1.0f - frac);    // Sample at 1
        weights[i * 4 + 3] = bicubicWeight(2.0f - frac);    // Sample at 2
        
        // Normalize weights to ensure they sum to 1.0
        float sum = weights[i * 4 + 0] + weights[i * 4 + 1] + weights[i * 4 + 2] + weights[i * 4 + 3];
        weights[i * 4 + 0] /= sum;
        weights[i * 4 + 1] /= sum;
        weights[i * 4 + 2] /= sum;
        weights[i * 4 + 3] /= sum;
    }
    
    // Update weight buffer with calculated values
    IRenderer* renderer = XISContext::GetCurrentContext()->GetRenderer();
    renderer->UpdateBuffer(m_data->weightBuffer, weights.data(), weights.size() * sizeof(float));
}

} // namespace XIS

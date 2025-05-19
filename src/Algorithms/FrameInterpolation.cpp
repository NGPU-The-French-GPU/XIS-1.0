#include "FrameInterpolation.h"
#include "../Utils/Logger.h"
#include "../Renderer/IRenderer.h"
#include "../Core/XISDevice.h"
#include "../Shaders/ShaderManager.h"

namespace XIS {

struct FrameInterpolation::FrameInterpolationData {
    bool initialized;
    
    // Shader resources
    void* motionEstimationShader;
    void* motionRefinementShader;
    void* frameInterpolationShader;
    
    // Compute resources
    void* blockMotionBuffer;
    void* occlusionBuffer;
    
    // Shader constants
    struct MotionShaderConstants {
        int frameWidth;
        int frameHeight;
        int blockSize;
        int searchRadius;
        float temporalWeight;
        float spatialWeight;
        int padding[2];
    };
    
    struct InterpolationShaderConstants {
        int frameWidth;
        int frameHeight;
        float timePosition;
        float qualityFactor;
        int useOcclusion;
        int padding[3];
    };
    
    void* motionConstantBuffer;
    void* interpolationConstantBuffer;
    
    // Settings
    int blockSize;
    int searchRadius;
};

FrameInterpolation::FrameInterpolation() 
    : m_data(std::make_unique<FrameInterpolationData>())
{
    m_data->initialized = false;
    m_data->motionEstimationShader = nullptr;
    m_data->motionRefinementShader = nullptr;
    m_data->frameInterpolationShader = nullptr;
    m_data->blockMotionBuffer = nullptr;
    m_data->occlusionBuffer = nullptr;
    m_data->motionConstantBuffer = nullptr;
    m_data->interpolationConstantBuffer = nullptr;
    
    // Default settings
    m_data->blockSize = 16;    // 16x16 pixel blocks for motion estimation
    m_data->searchRadius = 32; // 32 pixel search radius
}

FrameInterpolation::~FrameInterpolation() {
    Shutdown();
}

bool FrameInterpolation::Initialize(const XISContext* context) {
    if (!context) {
        Logger::Error("FrameInterpolation: Invalid context provided");
        return false;
    }
    
    // Initialize shaders
    if (!InitializeShaders(context)) {
        Logger::Error("FrameInterpolation: Failed to initialize shaders");
        return false;
    }
    
    // Create compute buffers
    if (!CreateComputeBuffers(context)) {
        Logger::Error("FrameInterpolation: Failed to create compute buffers");
        return false;
    }
    
    m_data->initialized = true;
    Logger::Info("FrameInterpolation: Successfully initialized");
    return true;
}

void FrameInterpolation::Shutdown() {
    if (!m_data->initialized) {
        return;
    }
    
    // Release shader resources
    if (m_data->motionEstimationShader) {
        m_data->motionEstimationShader = nullptr;
    }
    
    if (m_data->motionRefinementShader) {
        m_data->motionRefinementShader = nullptr;
    }
    
    if (m_data->frameInterpolationShader) {
        m_data->frameInterpolationShader = nullptr;
    }
    
    // Release compute resources
    if (m_data->blockMotionBuffer) {
        m_data->blockMotionBuffer = nullptr;
    }
    
    if (m_data->occlusionBuffer) {
        m_data->occlusionBuffer = nullptr;
    }
    
    // Release constant buffers
    if (m_data->motionConstantBuffer) {
        m_data->motionConstantBuffer = nullptr;
    }
    
    if (m_data->interpolationConstantBuffer) {
        m_data->interpolationConstantBuffer = nullptr;
    }
    
    m_data->initialized = false;
    Logger::Info("FrameInterpolation: Successfully shut down");
}

bool FrameInterpolation::CalculateMotionVectors(
    const XISContext* context,
    void* previousFrame,
    void* currentFrame,
    void* motionVectorTexture) {
    
    if (!m_data->initialized) {
        Logger::Error("FrameInterpolation: Not initialized");
        return false;
    }
    
    if (!previousFrame || !currentFrame || !motionVectorTexture) {
        Logger::Error("FrameInterpolation: Invalid input textures");
        return false;
    }
    
    // Step 1: Block-based motion estimation
    if (!CalculateBlockMotion(context, previousFrame, currentFrame, m_data->blockMotionBuffer)) {
        Logger::Error("FrameInterpolation: Failed to calculate block motion");
        return false;
    }
    
    // Step 2: Refine motion vectors using optical flow
    if (!RefineMotionVectors(context, m_data->blockMotionBuffer, motionVectorTexture)) {
        Logger::Error("FrameInterpolation: Failed to refine motion vectors");
        return false;
    }
    
    return true;
}

bool FrameInterpolation::GenerateFrames(
    const XISContext* context,
    void* previousFrame,
    void* currentFrame,
    void* motionVectorTexture,
    void* outputFrameBuffer,
    int generationFactor,
    float qualityFactor) {
    
    if (!m_data->initialized) {
        Logger::Error("FrameInterpolation: Not initialized");
        return false;
    }
    
    if (!previousFrame || !currentFrame || !motionVectorTexture || !outputFrameBuffer) {
        Logger::Error("FrameInterpolation: Invalid input textures");
        return false;
    }
    
    // Validate generation factor
    if (generationFactor < 1) {
        generationFactor = 1;
        Logger::Warning("FrameInterpolation: Invalid generation factor, using default (1)");
    }
    
    // Adjust quality factor to valid range [0.0, 1.0]
    if (qualityFactor < 0.0f) qualityFactor = 0.0f;
    if (qualityFactor > 1.0f) qualityFactor = 1.0f;
    
    // Generate intermediate frames
    bool success = true;
    for (int i = 0; i < generationFactor; i++) {
        // Calculate time position between frames (0.0 to 1.0)
        float timePosition = static_cast<float>(i + 1) / (generationFactor + 1);
        
        // Generate intermediate frame
        if (!GenerateIntermediateFrame(
            context, 
            previousFrame, 
            currentFrame, 
            motionVectorTexture,
            outputFrameBuffer,
            timePosition,
            qualityFactor)) {
            
            Logger::Error("FrameInterpolation: Failed to generate intermediate frame at time position %.2f", timePosition);
            success = false;
            break;
        }
        
        // TODO: Store the generated frame in the output buffer
        // This would depend on the actual implementation of the frame buffer
    }
    
    return success;
}

bool FrameInterpolation::InitializeShaders(const XISContext* context) {
    ShaderManager* shaderManager = context->GetShaderManager();
    if (!shaderManager) {
        Logger::Error("FrameInterpolation: Failed to get shader manager");
        return false;
    }
    
    // Load motion estimation compute shader
    m_data->motionEstimationShader = shaderManager->LoadComputeShader(
        "FrameGeneration.hlsl", 
        "MotionEstimationCS", 
        "cs_5_0"
    );
    
    if (!m_data->motionEstimationShader) {
        Logger::Error("FrameInterpolation: Failed to load motion estimation shader");
        return false;
    }
    
    // Load motion refinement compute shader
    m_data->motionRefinementShader = shaderManager->LoadComputeShader(
        "FrameGeneration.hlsl", 
        "MotionRefinementCS", 
        "cs_5_0"
    );
    
    if (!m_data->motionRefinementShader) {
        Logger::Error("FrameInterpolation: Failed to load motion refinement shader");
        return false;
    }
    
    // Load frame interpolation compute shader
    m_data->frameInterpolationShader = shaderManager->LoadComputeShader(
        "FrameGeneration.hlsl", 
        "FrameInterpolationCS", 
        "cs_5_0"
    );
    
    if (!m_data->frameInterpolationShader) {
        Logger::Error("FrameInterpolation: Failed to load frame interpolation shader");
        return false;
    }
    
    return true;
}

bool FrameInterpolation::CreateComputeBuffers(const XISContext* context) {
    IRenderer* renderer = context->GetRenderer();
    if (!renderer) {
        Logger::Error("FrameInterpolation: Failed to get renderer");
        return false;
    }
    
    // Get frame dimensions
    int frameWidth = context->GetBackBufferWidth();
    int frameHeight = context->GetBackBufferHeight();
    
    // Calculate block grid dimensions
    int blockGridWidth = (frameWidth + m_data->blockSize - 1) / m_data->blockSize;
    int blockGridHeight = (frameHeight + m_data->blockSize - 1) / m_data->blockSize;
    
    // Create block motion buffer
    m_data->blockMotionBuffer = renderer->CreateStructuredBuffer(
        blockGridWidth * blockGridHeight, // Number of blocks
        sizeof(float) * 4,                // Vector4: x, y motion vectors + confidence + occlusion
        true,                             // Allow UAV
        "BlockMotionBuffer"
    );
    
    if (!m_data->blockMotionBuffer) {
        Logger::Error("FrameInterpolation: Failed to create block motion buffer");
        return false;
    }
    
    // Create occlusion buffer
    m_data->occlusionBuffer = renderer->CreateTexture2D(
        frameWidth,
        frameHeight,
        renderer->GetFloatTextureFormat(), // Use appropriate format for occlusion map
        true,                              // Allow UAV
        "OcclusionBuffer"
    );
    
    if (!m_data->occlusionBuffer) {
        Logger::Error("FrameInterpolation: Failed to create occlusion buffer");
        return false;
    }
    
    // Create motion constant buffer
    FrameInterpolationData::MotionShaderConstants motionConstants;
    motionConstants.frameWidth = frameWidth;
    motionConstants.frameHeight = frameHeight;
    motionConstants.blockSize = m_data->blockSize;
    motionConstants.searchRadius = m_data->searchRadius;
    motionConstants.temporalWeight = 0.7f;  // Weight for temporal coherence
    motionConstants.spatialWeight = 0.3f;   // Weight for spatial coherence
    
    m_data->motionConstantBuffer = renderer->CreateConstantBuffer(
        sizeof(FrameInterpolationData::MotionShaderConstants),
        &motionConstants,
        "MotionConstantBuffer"
    );
    
    if (!m_data->motionConstantBuffer) {
        Logger::Error("FrameInterpolation: Failed to create motion constant buffer");
        return false;
    }
    
    // Create interpolation constant buffer
    FrameInterpolationData::InterpolationShaderConstants interpolationConstants;
    interpolationConstants.frameWidth = frameWidth;
    interpolationConstants.frameHeight = frameHeight;
    interpolationConstants.timePosition = 0.5f;     // Default middle position
    interpolationConstants.qualityFactor = 0.8f;    // Default high quality
    interpolationConstants.useOcclusion = 1;        // Enable occlusion handling by default
    
    m_data->interpolationConstantBuffer = renderer->CreateConstantBuffer(
        sizeof(FrameInterpolationData::InterpolationShaderConstants),
        &interpolationConstants,
        "InterpolationConstantBuffer"
    );
    
    if (!m_data->interpolationConstantBuffer) {
        Logger::Error("FrameInterpolation: Failed to create interpolation constant buffer");
        return false;
    }
    
    return true;
}

bool FrameInterpolation::CalculateBlockMotion(
    const XISContext* context,
    void* previousFrame,
    void* currentFrame,
    void* blockMotionBuffer) {
    
    IRenderer* renderer = context->GetRenderer();
    
    // Update frame dimensions in constant buffer if needed
    FrameInterpolationData::MotionShaderConstants constants;
    constants.frameWidth = context->GetBackBufferWidth();
    constants.frameHeight = context->GetBackBufferHeight();
    constants.blockSize = m_data->blockSize;
    constants.searchRadius = m_data->searchRadius;
    constants.temporalWeight = 0.7f;
    constants.spatialWeight = 0.3f;
    
    if (!renderer->UpdateConstantBuffer(m_data->motionConstantBuffer, &constants, sizeof(constants))) {
        Logger::Error("FrameInterpolation: Failed to update motion constant buffer");
        return false;
    }
    
    // Set shader resources
    renderer->SetComputeShader(m_data->motionEstimationShader);
    renderer->SetComputeConstantBuffer(0, m_data->motionConstantBuffer);
    renderer->SetComputeShaderResource(0, previousFrame);
    renderer->SetComputeShaderResource(1, currentFrame);
    renderer->SetComputeUnorderedAccessView(0, blockMotionBuffer);
    
    // Calculate dispatch dimensions
    int blockGridWidth = (constants.frameWidth + m_data->blockSize - 1) / m_data->blockSize;
    int blockGridHeight = (constants.frameHeight + m_data->blockSize - 1) / m_data->blockSize;
    
    // Dispatch compute shader
    renderer->DispatchCompute(
        (blockGridWidth + 7) / 8,  // Round up to multiple of 8 for thread groups
        (blockGridHeight + 7) / 8,
        1
    );
    
    // Wait for compute shader to finish
    renderer->SyncCompute();
    
    return true;
}

bool FrameInterpolation::RefineMotionVectors(
    const XISContext* context,
    void* blockMotionBuffer,
    void* motionVectorTexture) {
    
    IRenderer* renderer = context->GetRenderer();
    
    // Set shader resources
    renderer->SetComputeShader(m_data->motionRefinementShader);
    renderer->SetComputeConstantBuffer(0, m_data->motionConstantBuffer);
    renderer->SetComputeShaderResource(0, blockMotionBuffer);
    renderer->SetComputeUnorderedAccessView(0, motionVectorTexture);
    
    // Calculate dispatch dimensions for full-resolution processing
    int frameWidth = context->GetBackBufferWidth();
    int frameHeight = context->GetBackBufferHeight();
    
    // Dispatch compute shader (8x8 thread groups)
    renderer->DispatchCompute(
        (frameWidth + 7) / 8,
        (frameHeight + 7) / 8,
        1
    );
    
    // Wait for compute shader to finish
    renderer->SyncCompute();
    
    return true;
}

bool FrameInterpolation::GenerateIntermediateFrame(
    const XISContext* context,
    void* previousFrame,
    void* currentFrame,
    void* motionVectorTexture,
    void* outputTexture,
    float timePosition,
    float qualityFactor) {
    
    IRenderer* renderer = context->GetRenderer();
    
    // Update interpolation constant buffer
    FrameInterpolationData::InterpolationShaderConstants constants;
    constants.frameWidth = context->GetBackBufferWidth();
    constants.frameHeight = context->GetBackBufferHeight();
    constants.timePosition = timePosition;
    constants.qualityFactor = qualityFactor;
    constants.useOcclusion = qualityFactor > 0.5f ? 1 : 0; // Use occlusion for higher quality
    
    if (!renderer->UpdateConstantBuffer(m_data->interpolationConstantBuffer, &constants, sizeof(constants))) {
        Logger::Error("FrameInterpolation: Failed to update interpolation constant buffer");
        return false;
    }
    
    // Set shader resources
    renderer->SetComputeShader(m_data->frameInterpolationShader);
    renderer->SetComputeConstantBuffer(0, m_data->interpolationConstantBuffer);
    renderer->SetComputeShaderResource(0, previousFrame);
    renderer->SetComputeShaderResource(1, currentFrame);
    renderer->SetComputeShaderResource(2, motionVectorTexture);
    renderer->SetComputeUnorderedAccessView(0, outputTexture);
    
    if (constants.useOcclusion) {
        renderer->SetComputeUnorderedAccessView(1, m_data->occlusionBuffer);
    }
    
    // Calculate dispatch dimensions for full-resolution processing
    int frameWidth = constants.frameWidth;
    int frameHeight = constants.frameHeight;
    
    // Dispatch compute shader (8x8 thread groups)
    renderer->DispatchCompute(
        (frameWidth + 7) / 8,
        (frameHeight + 7) / 8,
        1
    );
    
    // Wait for compute shader to finish
    renderer->SyncCompute();
    
    return true;
}

}
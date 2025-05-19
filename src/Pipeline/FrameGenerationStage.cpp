#include "FrameGenerationStage.h"
#include "../Algorithms/FrameInterpolation.h"
#include "../Utils/Logger.h"
#include "../Renderer/IRenderer.h"

namespace XIS {

struct FrameGenerationStage::FrameGenerationStageData {
    bool initialized;
    FrameInterpolation frameInterpolator;
    void* motionVectorTexture;
    int frameWidth;
    int frameHeight;
    int format;
};

FrameGenerationStage::FrameGenerationStage() 
    : m_data(std::make_unique<FrameGenerationStageData>())
    , m_generatedFrameBuffer(nullptr)
    , m_generationFactor(1) // Default to 1 intermediate frame (doubles framerate)
{
    m_data->initialized = false;
    m_data->motionVectorTexture = nullptr;
    m_data->frameWidth = 0;
    m_data->frameHeight = 0;
    m_data->format = 0;

    // Reserve space for previous frames (keeping last 2 frames)
    m_previousFrames.resize(2, nullptr);
}

FrameGenerationStage::~FrameGenerationStage() {
    Shutdown();
}

bool FrameGenerationStage::Initialize(const XISContext* context) {
    if (!context) {
        Logger::Error("FrameGenerationStage: Invalid context provided");
        return false;
    }

    // Get frame dimensions from context
    m_data->frameWidth = context->GetBackBufferWidth();
    m_data->frameHeight = context->GetBackBufferHeight();
    m_data->format = context->GetBackBufferFormat();

    // Initialize the frame interpolator
    if (!m_data->frameInterpolator.Initialize(context)) {
        Logger::Error("FrameGenerationStage: Failed to initialize frame interpolator");
        return false;
    }

    // Initialize resources
    if (!InitializeResources(context)) {
        Logger::Error("FrameGenerationStage: Failed to initialize resources");
        return false;
    }

    m_data->initialized = true;
    Logger::Info("FrameGenerationStage: Successfully initialized");
    return true;
}

void FrameGenerationStage::Shutdown() {
    if (!m_data->initialized) {
        return;
    }

    // Clean up frame interpolator
    m_data->frameInterpolator.Shutdown();

    // Release previous frames
    for (auto& frame : m_previousFrames) {
        if (frame) {
            // Release resource based on renderer type
            frame = nullptr;
        }
    }

    // Release motion vector texture
    if (m_data->motionVectorTexture) {
        m_data->motionVectorTexture = nullptr;
    }

    // Release generated frame buffer
    if (m_generatedFrameBuffer) {
        m_generatedFrameBuffer = nullptr;
    }

    m_data->initialized = false;
    Logger::Info("FrameGenerationStage: Successfully shut down");
}

bool FrameGenerationStage::Process(const XISContext* context, 
                                  void* inputTexture, 
                                  void* outputTexture,
                                  const XISParameters& params) {
    if (!m_data->initialized) {
        Logger::Error("FrameGenerationStage: Not initialized");
        return false;
    }

    if (!inputTexture || !outputTexture) {
        Logger::Error("FrameGenerationStage: Invalid input or output texture");
        return false;
    }

    // Update motion vectors between current frame and previous frame
    if (!UpdateMotionVectors(context, inputTexture)) {
        Logger::Warning("FrameGenerationStage: Failed to update motion vectors");
        // Continue processing even if motion vector update fails
    }

    // Get previous frame
    void* previousFrame = m_previousFrames[0];
    
    // Generate intermediate frames if we have a previous frame
    if (previousFrame) {
        if (!GenerateIntermediateFrames(context, previousFrame, inputTexture, params)) {
            Logger::Error("FrameGenerationStage: Failed to generate intermediate frames");
            return false;
        }
    }

    // Update frame history:
    // Shift frames: previousFrames[1] -> previousFrames[0]
    m_previousFrames[0] = m_previousFrames[1];
    
    // Store current frame as the most recent frame in history
    m_previousFrames[1] = inputTexture;

    // Copy current frame to output texture
    IRenderer* renderer = context->GetRenderer();
    if (!renderer->CopyResource(outputTexture, inputTexture)) {
        Logger::Error("FrameGenerationStage: Failed to copy resource");
        return false;
    }

    return true;
}

void FrameGenerationStage::SetGenerationFactor(int factor) {
    if (factor < 1) {
        Logger::Warning("FrameGenerationStage: Invalid generation factor, using default (1)");
        m_generationFactor = 1;
        return;
    }
    
    m_generationFactor = factor;
    Logger::Info("FrameGenerationStage: Generation factor set to %d", m_generationFactor);
}

void* FrameGenerationStage::GetGeneratedFrameBuffer() const {
    return m_generatedFrameBuffer;
}

bool FrameGenerationStage::IsReady() const {
    // We need at least two frames in history to generate intermediate frames
    return m_data->initialized && m_previousFrames[0] != nullptr;
}

bool FrameGenerationStage::InitializeResources(const XISContext* context) {
    IRenderer* renderer = context->GetRenderer();
    
    // Create motion vector texture
    m_data->motionVectorTexture = renderer->CreateTexture2D(
        m_data->frameWidth, 
        m_data->frameHeight, 
        m_data->format,
        true, // Allow UAV access for compute shader
        "MotionVectorTexture"
    );
    
    if (!m_data->motionVectorTexture) {
        Logger::Error("FrameGenerationStage: Failed to create motion vector texture");
        return false;
    }
    
    // Create generated frame buffer
    m_generatedFrameBuffer = renderer->CreateTexture2D(
        m_data->frameWidth, 
        m_data->frameHeight, 
        m_data->format,
        true, // Allow UAV access for compute shader
        "GeneratedFrameBuffer"
    );
    
    if (!m_generatedFrameBuffer) {
        Logger::Error("FrameGenerationStage: Failed to create generated frame buffer");
        return false;
    }
    
    return true;
}

bool FrameGenerationStage::UpdateMotionVectors(const XISContext* context, void* currentFrame) {
    // Get previous frame - if none, return success (nothing to do yet)
    void* previousFrame = m_previousFrames[0];
    if (!previousFrame) {
        return true;
    }
    
    // Calculate motion vectors between previous and current frame
    return m_data->frameInterpolator.CalculateMotionVectors(
        context,
        previousFrame,
        currentFrame,
        m_data->motionVectorTexture
    );
}

bool FrameGenerationStage::GenerateIntermediateFrames(
    const XISContext* context, 
    void* previousFrame, 
    void* currentFrame,
    const XISParameters& params) {
    
    // Use frame interpolator to generate frames
    return m_data->frameInterpolator.GenerateFrames(
        context,
        previousFrame,
        currentFrame,
        m_data->motionVectorTexture,
        m_generatedFrameBuffer,
        m_generationFactor,
        params.frameGenerationQuality
    );
}

} // namespace XIS
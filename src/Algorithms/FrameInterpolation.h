#pragma once

#include "../Core/XISContext.h"
#include <memory>

namespace XIS {

class FrameInterpolation {
public:
    FrameInterpolation();
    ~FrameInterpolation();

    bool Initialize(const XISContext* context);
    void Shutdown();

    // Calculate motion vectors between two frames
    bool CalculateMotionVectors(
        const XISContext* context,
        void* previousFrame,
        void* currentFrame,
        void* motionVectorTexture
    );

    // Generate intermediate frames between two input frames
    bool GenerateFrames(
        const XISContext* context,
        void* previousFrame,
        void* currentFrame,
        void* motionVectorTexture,
        void* outputFrameBuffer,
        int generationFactor,
        float qualityFactor
    );

private:
    struct FrameInterpolationData;
    std::unique_ptr<FrameInterpolationData> m_data;

    // Initialize shader resources
    bool InitializeShaders(const XISContext* context);

    // Create compute buffers for intermediate computations
    bool CreateComputeBuffers(const XISContext* context);
    
    // Helper method to calculate block-based motion estimation
    bool CalculateBlockMotion(
        const XISContext* context,
        void* previousFrame,
        void* currentFrame,
        void* blockMotionBuffer
    );
    
    // Refine motion vectors using optical flow techniques
    bool RefineMotionVectors(
        const XISContext* context,
        void* blockMotionBuffer,
        void* motionVectorTexture
    );
    
    // Generate a single intermediate frame at the specified time position
    bool GenerateIntermediateFrame(
        const XISContext* context,
        void* previousFrame,
        void* currentFrame,
        void* motionVectorTexture,
        void* outputTexture,
        float timePosition, // 0.0 = previous frame, 1.0 = current frame
        float qualityFactor
    );
};

} // namespace XIS
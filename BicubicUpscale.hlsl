Texture2D inputTexture : register(t0);
SamplerState samplerState : register(s0);

float4 BicubicUpsamplePS(float4 position : SV_POSITION, float2 texCoord : TEXCOORD) : SV_Target {
    float2 texSize = float2(1.0 / inputTexture.GetDimensions());
    float2 coord = texCoord * inputTexture.GetDimensions() - 0.5;
    float2 floorCoord = floor(coord);
    float2 fractCoord = coord - floorCoord;

    float4 result = float4(0, 0, 0, 0);

    for (int x = -1; x <= 2; x++) {
        for (int y = -1; y <= 2; y++) {
            float2 offset = float2(x, y);
            float2 sampleCoord = (floorCoord + offset + 0.5) * texSize;
            float weight = CubicKernel(fractCoord - offset);
            result += inputTexture.Sample(samplerState, sampleCoord) * weight;
        }
    }

    return result;
}

float CubicKernel(float x) {
    x = abs(x);
    if (x < 1.0) {
        return (1.5 * x - 2.5) * x * x + 1.0;
    } else if (x < 2.0) {
        return ((-0.5 * x + 2.5) * x - 4.0) * x + 2.0;
    }
    return 0.0;
}

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

VS_OUTPUT FullscreenQuadVS(float4 position : POSITION, float2 texCoord : TEXCOORD) {
    VS_OUTPUT output;
    output.position = position;
    output.texCoord = texCoord;
    return output;
}

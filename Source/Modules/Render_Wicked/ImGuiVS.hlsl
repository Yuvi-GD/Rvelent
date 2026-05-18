// Wicked Engine ImGui Vertex Shader
// MUST use the same root signature as globals.hlsli so that the DX12 backend
// can create its internal multi-draw indirect command signatures.

struct VertexInput {
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

struct VertexOutput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

// Use Wicked Engine's required root signature layout.
// The key requirement is "RootConstants(num32BitConstants=12, b999)"
// which Wicked Engine uses internally for push constants / indirect drawing.
#define WICKED_IMGUI_ROOTSIG \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    "RootConstants(num32BitConstants=12, b999), " \
    "CBV(b0, visibility=SHADER_VISIBILITY_VERTEX), " \
    "DescriptorTable(SRV(t0, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(Sampler(s0, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)"

cbuffer vertexBuffer : register(b0) {
    float4x4 ProjectionMatrix;
};

[RootSignature(WICKED_IMGUI_ROOTSIG)]
VertexOutput main(VertexInput input) {
    VertexOutput output;
    output.pos = mul(ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
    output.uv = input.uv;
    output.col = input.col;
    return output;
}

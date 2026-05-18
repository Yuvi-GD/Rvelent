// Wicked Engine ImGui Pixel Shader

struct VertexOutput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

#define WICKED_IMGUI_ROOTSIG \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    "RootConstants(num32BitConstants=12, b999), " \
    "CBV(b0, visibility=SHADER_VISIBILITY_VERTEX), " \
    "DescriptorTable(SRV(t0, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(Sampler(s0, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)"

[RootSignature(WICKED_IMGUI_ROOTSIG)]
float4 main(VertexOutput input) : SV_TARGET {
    return input.col * texture0.Sample(sampler0, input.uv);
}

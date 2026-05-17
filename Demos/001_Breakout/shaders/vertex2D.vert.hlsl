#include "_common.hlsl"

VSOutput main(VSInput input)
{
    VSOutput output;

    // Convert static unit quad vertices into world-sized local coordinates.
    float2 localPos = input.Position * g_Push.Size;

    // Model should contain position + rotation, but not scale.
    float4 worldPos = mul(float4(localPos, 0.0, 1.0), g_Push.Model);
    float4 clipPos  = mul(worldPos, g_SceneData.viewProj);

    output.Position = clipPos;
    output.Color = g_Push.color;
    output.LocalPos = localPos;

    return output;
}

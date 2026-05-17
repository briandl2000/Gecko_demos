#include "_common.hlsl"

float4 main(PSInput input) : SV_Target
{
    // For a real circle, Size.x and Size.y should be equal.
    float radius = min(g_Push.Size.x, g_Push.Size.y) * 0.5;

    float sdf = sdCircle(input.LocalPos, radius);

    float alpha = calcAlpha(sdf);

    return float4(input.Color, alpha);
}

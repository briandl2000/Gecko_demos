#include "_common.hlsl"

float4 main(PSInput input) : SV_Target
{
    // For a real circle, Size.x and Size.y should be equal.
    float radius = min(g_Push.Size.x, g_Push.Size.y) * 0.5;

    float outer = sdCircle(input.LocalPos, radius);

    float innerRadius = max(radius - g_Push.StrokeWidth, 0.0);
    float inner = sdCircle(input.LocalPos, innerRadius);

    // Keep pixels inside outer circle but outside inner circle.
    float sdf = max(outer, -inner);

    float alpha = clamp(0, g_Push.alpha, calcAlpha(sdf));

    return float4(input.Color, alpha);
}

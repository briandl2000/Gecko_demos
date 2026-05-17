#include "_common.hlsl"

float4 main(PSInput input) : SV_Target
{
    float2 halfSize = g_Push.Size * 0.5;

    float outer = sdBox(input.LocalPos, halfSize);

    float2 innerHalfSize = halfSize - g_Push.StrokeWidth;
    innerHalfSize = max(innerHalfSize, float2(0.0, 0.0));

    float inner = sdBox(input.LocalPos, innerHalfSize);

    // Keep pixels inside the outer rect but outside the inner rect.
    float sdf = max(outer, -inner);

    float alpha = clamp(0, g_Push.alpha, calcAlpha(sdf));

    return float4(input.Color, alpha);
}

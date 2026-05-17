#include "_common.hlsl"

float4 main(PSInput input) : SV_Target
{
    float2 halfSize = g_Push.Size * 0.5;

    float sdf = sdBox(input.LocalPos, halfSize);

    float alpha = calcAlpha(sdf);

    return float4(input.Color, alpha);
}

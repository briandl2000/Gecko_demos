[[vk::binding(0, 0)]] Texture2D    g_Source  : register(t0);
[[vk::binding(1, 0)]] SamplerState g_Sampler : register(s0);

struct PSInput
{
  float4 Position : SV_Position;
  float2 UV       : TEXCOORD0;
};

float4 main(PSInput input) : SV_Target
{
  float2 uv = input.UV;
  uv.y *= -1.;
  uv.y += 1.;
  return g_Source.Sample(g_Sampler, uv);
}

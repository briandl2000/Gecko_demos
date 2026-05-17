struct PushConstants
{
  float4x4 Model;

  float3 color;
  float StrokeWidth;

  // World-space size of the primitive.
  // Rect: width/height.
  // Circle: use Size.x == Size.y, diameter.
  float2 Size;

  // Padding so the C++ side can safely align this to 16-byte boundaries.
  float2 _Padding;
};

struct VSInput
{
  float2 Position : POSITION;
};

struct VSOutput
{
  float4 Position : SV_Position;
  float3 Color    : COLOR0;

  // World-sized local position.
  // For a rect of Size = float2(4, 2), this ranges:
  // x: -2 to +2
  // y: -1 to +1
  float2 LocalPos : TEXCOORD0;
};

typedef VSOutput PSInput;

struct SceneData
{
  float4x4 viewProj;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> g_Push;

[[vk::binding(0)]]
ConstantBuffer<SceneData> g_SceneData : register(t0);

float sdCircle(float2 p, float r)
{
  return length(p) - r;
}

float sdBox(float2 p, float2 b)
{
    float2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

float calcAlpha(float sdf)
{
  return smoothstep(0.0, -0.015, sdf);
}

// Vertex shader for the coloured triangle drawn into the offscreen RT.
// Compiled with: glslc -x hlsl -fshader-stage=vert -fentry-point=main
//
// Uses a push constant (Time) to rotate the triangle about the origin.

struct PushConstants
{
  float4x4 viewProjModel;
  float3 color;
  float pad;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> g_Push;

struct VSInput
{
  float2 Position : POSITION;
};

struct VSOutput
{
  float4 Position : SV_Position;
  float3 Color    : COLOR;
};

VSOutput main(VSInput input)
{

  float4 position = mul(float4(input.Position, 0.0, 1.0), g_Push.viewProjModel);

  VSOutput output;
  output.Position = position;
  output.Color = g_Push.color;
  return output;
}

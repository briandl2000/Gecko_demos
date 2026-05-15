#include "Renderer.h"
#include <gecko/graphics/graphics_module.h>
#include <gecko/math/matrix.h>
#include <gecko/math/vector.h>
#include "shaders.h"
#include <gecko/core/utility/random.h>

namespace
{
  constexpr gk::Label Renderer_Label = gk::MakeLabel("app.breakout.Renderer");

  struct Vertex
  {
    gm::Float2 Position;
  };

  constexpr Vertex RectVertices[] = {
      {{-0.5, -0.5}},
      {{ 0.5, -0.5}},
      {{-0.5,  0.5}},
      {{ 0.5,  0.5}}
  };

  constexpr u32 RectIndices[] = {
    0, 1, 2, 1, 3, 2
  };

}

struct pushConst {
  gm::Float4x4 ViewProjModel {};
  gm::Float3 Color { };
  f32 pad1;
};

bool Renderer::Init()
{
  return CreateRenderResources() && CreatePipelines();
}

void Renderer::BeginRender(gk::graphics::ICommandList* cmd, gm::Float4x4* viewProjection)
{
  m_ViewProjection = viewProjection;
  m_Cmd = cmd;
}

void Renderer::EndRender()
{
  m_ViewProjection = nullptr;
  m_Cmd = nullptr;
}

bool Renderer::CreateRenderResources()
{
  auto device = gk::graphics::GetGraphicsDevice();
  if(!device)
    return false;

  gk::graphics::VertexBufferDesc vertexBufferDesc;
  vertexBufferDesc.NumVertices = 4;
  vertexBufferDesc.VertexSize = sizeof(Vertex);
  vertexBufferDesc.Memory = gk::graphics::MemoryType::Dedicated;
  m_RectVertexBuffer = device->CreateVertexBuffer(vertexBufferDesc);
  if (m_RectVertexBuffer.IsValid())
  {
    const auto* raw = reinterpret_cast<const gk::byte*>(RectVertices);
    device->UploadBufferData(m_RectVertexBuffer, {raw, sizeof(RectVertices)});
  }

  gk::graphics::IndexBufferDesc indicesBufferDesc;
  indicesBufferDesc.NumIndices = 6;
  indicesBufferDesc.Memory = gk::graphics::MemoryType::Dedicated;
  m_RectIndexBuffer = device->CreateIndexBuffer(indicesBufferDesc);
  if (m_RectIndexBuffer.IsValid())
  {
    const auto* raw = reinterpret_cast<const gk::byte*>(RectIndices);
    device->UploadBufferData(m_RectIndexBuffer, {raw, sizeof(RectIndices)});
  }

  return true;
}

bool Renderer::CreatePipelines()
{
  auto device = gk::graphics::GetGraphicsDevice();
  if(!device)
    return false;

  gk::graphics::VertexLayout rectLayout;
  rectLayout.AddAttribute(gk::graphics::DataFormat::R32G32_FLOAT, "a_Position");

  gk::graphics::GraphicsPipelineDesc rectPDesc;
  rectPDesc.VertexShader = gk::graphics::ShaderCode {
      .Format = gk::graphics::ShaderFormat::SPIRV,
      .Bytes = {reinterpret_cast<const gk::byte*>(shaders::RectVert), sizeof(shaders::RectVert)},
  };
  rectPDesc.PixelShader = gk::graphics::ShaderCode {
      .Format = gk::graphics::ShaderFormat::SPIRV,
      .Bytes = {reinterpret_cast<const gk::byte*>(shaders::RectFrag), sizeof(shaders::RectFrag)},
  };
  rectPDesc.Layout = rectLayout;
  rectPDesc.NumRenderTargets = 1;
  rectPDesc.RenderTargetFormats[0] = gk::graphics::DataFormat::R8G8B8A8_UNORM;
  rectPDesc.Culling = gk::graphics::CullMode::None;
  rectPDesc.PushConstantBytes = sizeof(pushConst);
  rectPDesc.DebugName = "RectPipeline";
  m_RectPipeline = device->CreateGraphicsPipeline(rectPDesc);

  if (!m_RectPipeline.IsValid())
  {
    GECKO_WARN(Renderer_Label, "One or more pipelines failed to build - rendering disabled");
  }
  else
  {
    GECKO_INFO(Renderer_Label, "Pipelines ready");
  }
  return true;
}

void Renderer::DrawRect(gm::Float2 position, gm::Float3 Color, gm::Float2 size, f32 rotation)
{
  if(!m_ViewProjection || !m_Cmd)
  {
    GECKO_WARN(Renderer_Label, "No view projection or cmd is initialized!");
    return;
  }

  gm::Float4x4 posMat = gm::Float4x4::Translation({position.X, position.Y, 0.f});
  gm::Float4x4 scaleMat = gm::Float4x4::Scale({size.X, size.Y, 1.0f});
  gm::Float4x4 rotMat = gm::Float4x4::RotationZ(rotation);
  gm::Float4x4 model = posMat * rotMat * scaleMat;

  pushConst pc {};
  pc.ViewProjModel = *m_ViewProjection * model;
  pc.Color = Color;

  m_Cmd->BindPipeline(m_RectPipeline);
  m_Cmd->BindVertexBuffer(m_RectVertexBuffer);
  m_Cmd->BindIndexBuffer(m_RectIndexBuffer);
  m_Cmd->SetConstants(0, {reinterpret_cast<const gk::byte*>(&pc), sizeof(pc)});
  m_Cmd->DrawIndexed(m_RectIndexBuffer.IndexDesc.NumIndices);
}

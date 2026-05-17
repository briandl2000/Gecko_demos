#include "Renderer.h"
#include <gecko/graphics/graphics_module.h>
#include <gecko/graphics/graphics_types.h>
#include <gecko/math/matrix.h>
#include <gecko/math/vector.h>
#include "Scene.h"
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
  struct pushConst {
    gm::Float4x4 Model {};
    gm::Float3 Color { };
    f32 StrokeWidth {0.04};
    gm::Float2 Size;
    gm::Float2 Padding;
  };

  struct SceneData
  {
    gm::Float4x4 ViewProjection;
  };

  // RGBA float palette, 0 -> 1
}

bool Renderer::Init(u32 numBackBuffers, u32 width, u32 height)
{
  m_FillRectCommands.DrawCommands.resize(MaxCommands);
  m_FillCircleCommands.DrawCommands.resize(MaxCommands);
  m_StrokeRectCommands.DrawCommands.resize(MaxCommands);
  m_StrokeCircleCommands.DrawCommands.resize(MaxCommands);

  m_Width = width;
  m_Height = height;

  m_IsValid = CreateRenderResources(numBackBuffers) && CreatePipelines();

  return m_IsValid;
}

void Renderer::BeginRender(gk::graphics::ICommandList* cmd, gm::Float4x4* viewProjection, u32 frameIndex)
{
  auto device = gk::graphics::GetGraphicsDevice();

  m_ViewProjection = viewProjection;
  m_Cmd = cmd;

  m_FillRectCommands.Offset = 0;
  m_FillCircleCommands.Offset = 0;
  m_StrokeRectCommands.Offset = 0;
  m_StrokeCircleCommands.Offset = 0;
  m_FrameIndex = frameIndex;

  SceneData sceneData {};
  sceneData.ViewProjection = *viewProjection;
  if(m_FrameSlot[m_FrameIndex].SceneBuffer.IsValid())
  {
    SceneData data {};
    data.ViewProjection = *viewProjection;
    const auto* raw = reinterpret_cast<const gk::byte*>(&data);
    device->UploadBufferData(m_FrameSlot[m_FrameIndex].SceneBuffer, {raw, sizeof(SceneData)});
  }
}

void Renderer::EndRender()
{
  gk::graphics::ClearValue rtClear =
    gk::graphics::ClearValue::RenderTarget(
      Colors::bg_dark.X,
      Colors::bg_dark.Y,
      Colors::bg_dark.Z,
      1.0
    );
  gk::graphics::BeginRenderingInfo bri = {
      .Colors = {&m_FrameSlot[m_FrameIndex].RenderTarget, 1},
      .ClearColors = {&rtClear, 1}
  };
  m_Cmd->BeginRendering(bri);
  m_Cmd->SetViewport(0.0F, 0.0F, static_cast<::gecko::f32>(m_Width), static_cast<::gecko::f32>(m_Height));
  m_Cmd->SetScissor(0, 0, m_Width, m_Height);

  auto DrawList = [&](const DrawCommandList& list, const gk::graphics::GraphicsPipeline& pipeline){
    m_Cmd->BindPipeline(pipeline);
    m_Cmd->BindConstantBuffer(0, m_FrameSlot[m_FrameIndex].SceneBuffer);
    m_Cmd->BindVertexBuffer(m_RectVertexBuffer);
    m_Cmd->BindIndexBuffer(m_RectIndexBuffer);
    for (u32 i = 0; i < list.Offset; i++)
    {
      pushConst pc {};
      pc.Model = list.DrawCommands[i].Model;
      pc.Color = list.DrawCommands[i].Color;
      pc.Size = list.DrawCommands[i].Size;
      m_Cmd->SetConstants(0, {reinterpret_cast<const gk::byte*>(&pc), sizeof(pc)});
      m_Cmd->DrawIndexed(m_RectIndexBuffer.IndexDesc.NumIndices);
    }
  };

  DrawList(m_FillRectCommands, m_FillRectPipeline);
  DrawList(m_FillCircleCommands, m_FillCirclePipeline);
  DrawList(m_StrokeRectCommands, m_StrokeRectPipeline);
  DrawList(m_StrokeCircleCommands, m_StrokeCirclePipeline);
  m_Cmd->EndRendering();

  m_ViewProjection = nullptr;
  m_Cmd = nullptr;
}

void Renderer::DrawToScreen(gk::graphics::ICommandList* cmd)
{
  cmd->BindPipeline(m_FullscreenPipeline);
  cmd->BindTexture(0, m_FrameSlot[m_FrameIndex].RenderTarget.BackingTexture);
  cmd->BindSampler(1, m_FullscreenSampler);
  cmd->Draw(3);
}

bool Renderer::CreateRenderResources(u32 numBackBuffers)
{
  auto device = gk::graphics::GetGraphicsDevice();
  if(!device)
    return false;

  gk::graphics::VertexBufferDesc vertexBufferDesc;
  vertexBufferDesc.NumVertices = 4;
  vertexBufferDesc.VertexSize = sizeof(Vertex);
  vertexBufferDesc.Memory = gk::graphics::MemoryType::Dedicated;
  m_RectVertexBuffer = device->CreateVertexBuffer(vertexBufferDesc);
  if (!m_RectVertexBuffer.IsValid())
  {
    GECKO_WARN(Renderer_Label, "Failed to create vertex buffer");
    return false;
  }
  device->UploadBufferData(m_RectVertexBuffer,
    {
      reinterpret_cast<const gk::byte*>(RectVertices), sizeof(RectVertices)
    });

  gk::graphics::IndexBufferDesc indicesBufferDesc;
  indicesBufferDesc.NumIndices = 6;
  indicesBufferDesc.Memory = gk::graphics::MemoryType::Dedicated;
  m_RectIndexBuffer = device->CreateIndexBuffer(indicesBufferDesc);
  if (!m_RectIndexBuffer.IsValid())
  {
    GECKO_WARN(Renderer_Label, "Failed to create index buffer");
    return false;
  }
  device->UploadBufferData(m_RectIndexBuffer,
    {
      reinterpret_cast<const gk::byte*>(RectIndices), sizeof(RectIndices)
    });

  m_FrameSlot.resize(numBackBuffers);
  for(u32 i = 0; i < numBackBuffers; i++)
  {
    m_FrameSlot[i] = {};
    FrameSlot& frameSlot = m_FrameSlot[i];
    gk::graphics::ConstantBufferDesc sceneDataBufferDesc;
    sceneDataBufferDesc.Memory = gk::graphics::MemoryType::Dedicated;
    sceneDataBufferDesc.SizeInBytes = sizeof(SceneData);
    sceneDataBufferDesc.DebugName = "SceneDataBuffer";
    frameSlot.SceneBuffer = device->CreateConstantBuffer(sceneDataBufferDesc);
    if(!frameSlot.SceneBuffer.IsValid())
    {
      GECKO_WARN(Renderer_Label, "Failed to create index buffer");
      return false;
    }
    SceneData sceneData {};
    sceneData.ViewProjection = gm::float4x4::Identity();
    device->UploadBufferData(frameSlot.SceneBuffer,
      {
        reinterpret_cast<const gk::byte*>(&sceneData), sizeof(SceneData)
      });

    gk::graphics::RenderTargetDesc rtDesc;
    rtDesc.Width = m_Width;
    rtDesc.Height = m_Height;
    rtDesc.Format = gk::graphics::DataFormat::B8G8R8A8_UNORM;
    rtDesc.DebugName = "OffscreenRT";
    frameSlot.RenderTarget = device->CreateRenderTarget(rtDesc);
    if (!frameSlot.RenderTarget.IsValid())
    {
      GECKO_INFO(Renderer_Label, "Failed to create offscreen buffer!");
    }
  }

  gk::graphics::SamplerDesc fullscreenSamplerDesc {};
  fullscreenSamplerDesc.Filter = gk::graphics::SamplerFilter::Point;
  fullscreenSamplerDesc.WrapMode = gk::graphics::SamplerWrapMode::Clamp;
  fullscreenSamplerDesc.DebugName = "FullscreenSampler";
  m_FullscreenSampler = device->CreateSampler(fullscreenSamplerDesc);
  return true;

  return true;
}

bool Renderer::CreatePipelines()
{
  auto device = gk::graphics::GetGraphicsDevice();
  if(!device)
    return false;


  gk::graphics::GraphicsPipelineDesc pipelineDesc;
  gk::graphics::VertexLayout vertLayout;
  vertLayout.AddAttribute(gk::graphics::DataFormat::R32G32_FLOAT, "a_Position");

  pipelineDesc.VertexShader = gk::graphics::ShaderCode {
      .Format = gk::graphics::ShaderFormat::SPIRV,
      .Bytes = {reinterpret_cast<const gk::byte*>(shaders::Vertex2D), sizeof(shaders::Vertex2D)},
  };

  pipelineDesc.Layout = vertLayout;
  pipelineDesc.NumPipelineResources = 1;
  pipelineDesc.NumRenderTargets = 1;
  pipelineDesc.RenderTargetFormats[0] = gk::graphics::DataFormat::R8G8B8A8_UNORM;
  pipelineDesc.Culling = gk::graphics::CullMode::None;
  pipelineDesc.PipelineResources[0] =
  {
    .Type = gk::graphics::ResourceType::ConstantBuffer,
    .ShaderVisibility = gk::graphics::ShaderType::All,
  };
  pipelineDesc.BlendStates[0] = {
    .BlendEnable = true,
    .SrcColor = gk::graphics::BlendFactor::SrcAlpha,
    .DstColor = gk::graphics::BlendFactor::InvSrcAlpha,
    .ColorOp = gk::graphics::BlendOp::Add,
    .SrcAlpha = gk::graphics::BlendFactor::One,
    .DstAlpha = gk::graphics::BlendFactor::InvSrcAlpha,
    .AlphaOp = gk::graphics::BlendOp::Add,
  };
  pipelineDesc.PushConstantBytes = sizeof(pushConst);

  // Fill Rect pipeline
  {
    pipelineDesc.PixelShader = gk::graphics::ShaderCode {
        .Format = gk::graphics::ShaderFormat::SPIRV,
        .Bytes = {reinterpret_cast<const gk::byte*>(shaders::FillRectFrag), sizeof(shaders::FillRectFrag)},
    };
    pipelineDesc.DebugName = "RectFillPipeline";
    m_FillRectPipeline = device->CreateGraphicsPipeline(pipelineDesc);
    if (!m_FillRectPipeline.IsValid())
    {
      GECKO_WARN(Renderer_Label, "One or more pipelines failed to build - rendering disabled");
      return false;
    }
  }

  // Fill Circle pipeline
  {
    pipelineDesc.PixelShader = gk::graphics::ShaderCode {
        .Format = gk::graphics::ShaderFormat::SPIRV,
        .Bytes = {reinterpret_cast<const gk::byte*>(shaders::FillCircleFrag), sizeof(shaders::FillCircleFrag)},
    };
    pipelineDesc.DebugName = "circlePipeline";
    m_FillCirclePipeline = device->CreateGraphicsPipeline(pipelineDesc);
    if (!m_FillCirclePipeline.IsValid())
    {
      GECKO_WARN(Renderer_Label, "One or more pipelines failed to build - rendering disabled");
      return false;
    }
  }

  // Stroke Rect pipeline
  {
    pipelineDesc.PixelShader = gk::graphics::ShaderCode {
        .Format = gk::graphics::ShaderFormat::SPIRV,
        .Bytes = {reinterpret_cast<const gk::byte*>(shaders::StrokeRectFrag), sizeof(shaders::StrokeRectFrag)},
    };
    pipelineDesc.DebugName = "RectPipeline";
    m_StrokeRectPipeline = device->CreateGraphicsPipeline(pipelineDesc);
    if (!m_StrokeRectPipeline.IsValid())
    {
      GECKO_WARN(Renderer_Label, "One or more pipelines failed to build - rendering disabled");
      return false;
    }
  }

  // Circle pipeline
  {
    pipelineDesc.PixelShader = gk::graphics::ShaderCode {
        .Format = gk::graphics::ShaderFormat::SPIRV,
        .Bytes = {reinterpret_cast<const gk::byte*>(shaders::StrokeCircleFrag), sizeof(shaders::StrokeCircleFrag)},
    };
    pipelineDesc.DebugName = "circlePipeline";
    m_StrokeCirclePipeline = device->CreateGraphicsPipeline(pipelineDesc);
    if (!m_StrokeCirclePipeline.IsValid())
    {
      GECKO_WARN(Renderer_Label, "One or more pipelines failed to build - rendering disabled");
      return false;
    }
  }

  {
    gk::graphics::GraphicsPipelineDesc fullscreenPipelineDesc;
    fullscreenPipelineDesc.VertexShader = gk::graphics::ShaderCode {
        .Format = gk::graphics::ShaderFormat::SPIRV,
        .Bytes = {reinterpret_cast<const ::gecko::byte*>(shaders::FullscreenVert), sizeof(shaders::FullscreenVert)},
    };
    fullscreenPipelineDesc.PixelShader = gk::graphics::ShaderCode {
        .Format = gk::graphics::ShaderFormat::SPIRV,
        .Bytes = {reinterpret_cast<const ::gecko::byte*>(shaders::FullscreenFrag), sizeof(shaders::FullscreenFrag)},
    };
    fullscreenPipelineDesc.Layout = {};
    fullscreenPipelineDesc.NumRenderTargets = 1;
    fullscreenPipelineDesc.RenderTargetFormats[0] = gk::graphics::DataFormat::R8G8B8A8_UNORM;
    fullscreenPipelineDesc.Culling = gk::graphics::CullMode::None;
    fullscreenPipelineDesc.PipelineResources[0] =
      gk::graphics::PipelineResource::TextureBinding(1, gk::graphics::ShaderType::Pixel);
    fullscreenPipelineDesc.PipelineResources[1] =
      gk::graphics::PipelineResource::SamplerBinding(1, gk::graphics::ShaderType::Pixel);
    fullscreenPipelineDesc.NumPipelineResources = 2;
    fullscreenPipelineDesc.DebugName = "fullscreenPipelineipeline";
    m_FullscreenPipeline = device->CreateGraphicsPipeline(fullscreenPipelineDesc);
  }

  GECKO_INFO(Renderer_Label, "Pipelines ready");
  return true;
}

// Draw commands ------------------------------

void Renderer::WriteDrawCommand(DrawCommandList& CommandList, gm::Float2 position, gm::Float3 Color, gm::Float2 size, f32 rotation)
{
  if(!m_ViewProjection || !m_Cmd)
  {
    GECKO_WARN(Renderer_Label, "No view projection or cmd is initialized!");
    return;
  }
  int index = CommandList.Offset++;
  if (index >= CommandList.DrawCommands.size())
    return;

  gm::Float4x4 posMat = gm::Float4x4::Translation({position.X, position.Y, 0.f});
  gm::Float4x4 scaleMat = gm::Float4x4::Scale({size.X, size.Y, 1.0f});
  gm::Float4x4 rotMat = gm::Float4x4::RotationZ(rotation);
  gm::Float4x4 model = posMat * rotMat;

  CommandList.DrawCommands[index].Color = Color;
  CommandList.DrawCommands[index].Model = model;
  CommandList.DrawCommands[index].Size = size;
}

void Renderer::FillRect(gm::Float2 position, gm::Float3 Color, gm::Float2 size, f32 rotation)
{
  WriteDrawCommand(m_FillRectCommands, position, Color, size, rotation);
}

void Renderer::FillCircle(gm::Float2 position, gm::Float3 Color, gm::Float2 size)
{
  WriteDrawCommand(m_FillCircleCommands, position, Color, size, 0.);
}

void Renderer::StrokeRect(gm::Float2 position, gm::Float3 Color, gm::Float2 size, f32 rotation)
{
  WriteDrawCommand(m_StrokeRectCommands, position, Color, size, 0.);
}

void Renderer::StrokeCircle(gm::Float2 position, gm::Float3 Color, gm::Float2 size)
{
  WriteDrawCommand(m_StrokeCircleCommands, position, Color, size, 0.);
}

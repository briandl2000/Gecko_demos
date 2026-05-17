#pragma once
#include "Common.h"
#include <gecko/graphics/command_list.h>
#include <gecko/graphics/graphics_types.h>
#include <gecko/math/matrix.h>

namespace Colors
{
  constexpr gm::float3 deep_navy      = { 0.0392f, 0.1059f, 0.1725f}; // #0A1B2C
  constexpr gm::float3 cream          = { 0.9490f, 0.9137f, 0.8510f}; // #F2E9D9
  constexpr gm::float3 warm_white     = { 0.9686f, 0.9490f, 0.9020f}; // #F7F2E6
  constexpr gm::float3 blue           = { 0.2392f, 0.4510f, 0.8784f}; // #3D73E0
  constexpr gm::float3 yellow         = { 0.9765f, 0.7373f, 0.1569f}; // #F9BC28
  constexpr gm::float3 red            = { 0.8863f, 0.2196f, 0.1922f}; // #E23831
  constexpr gm::float3 green          = { 0.4275f, 0.6157f, 0.2000f}; // #6D9D33
  constexpr gm::float3 purple         = { 0.5490f, 0.3333f, 0.7137f}; // #8C55B6
  constexpr gm::float3 orange         = { 0.9216f, 0.4314f, 0.1373f}; // #EB6E23
  constexpr gm::float3 near_black     = { 0.0902f, 0.0902f, 0.0863f}; // #171716

  constexpr gm::float3 bg_dark        = deep_navy;
  constexpr gm::float3 ui_panel       = cream;
  constexpr gm::float3 ui_highlight   = warm_white;

  constexpr gm::float3 brick_normal   = blue;
  constexpr gm::float3 brick_core     = yellow;
  constexpr gm::float3 brick_bomb     = red;
  constexpr gm::float3 brick_splitter = green;
  constexpr gm::float3 brick_shield   = purple;
  constexpr gm::float3 brick_mover    = orange;
  constexpr gm::float3 brick_hazard   = near_black;

  constexpr gm::float3 ball_color     = warm_white;
  constexpr gm::float3 paddle_color   = cream;
  constexpr gm::float3 outline_color  = near_black;
  constexpr gm::float3 text_dark      = near_black;
  constexpr gm::float3 text_light     = warm_white;

}

struct DrawCommand
{
  gm::Float4x4 Model;
  gm::Float3 Color;
  gm::Float2 Size;
};

class Renderer
{
public:
  Renderer() = default;

  bool Init(u32 numBackBuffers, u32 width, u32 height);

  void BeginRender(gk::graphics::ICommandList* cmd, gm::Float4x4* viewProjection, u32 frameIndex);
  void EndRender();

  void DrawToScreen(gk::graphics::ICommandList* cmd);

  void FillRect(gm::Float2 position, gm::Float3 Color, gm::Float2 size = {1., 1.}, f32 rotation = 0.);
  void FillCircle(gm::Float2 position, gm::Float3 Color, gm::Float2 size = {1., 1.});
  void StrokeRect(gm::Float2 position, gm::Float3 Color, gm::Float2 size = {1., 1.}, f32 rotation = 0.);
  void StrokeCircle(gm::Float2 position, gm::Float3 Color, gm::Float2 size = {1., 1.});
private:
  static constexpr u32 MaxCommands {1028*4};

  bool CreateRenderResources(u32 numBackBuffers);
  bool CreatePipelines();


  // A frame slot is data that is unique per the frame, this is so that new frames don't overwrite
  // data being used by the previous frame
  struct FrameSlot
  {
    gk::graphics::Buffer SceneBuffer {};
    gk::graphics::RenderTarget RenderTarget {};
  };

  std::vector<FrameSlot> m_FrameSlot {};

  gk::graphics::Buffer m_RectVertexBuffer {};
  gk::graphics::Buffer m_RectIndexBuffer {};

  gk::graphics::GraphicsPipeline m_FillRectPipeline {};
  gk::graphics::GraphicsPipeline m_FillCirclePipeline {};
  gk::graphics::GraphicsPipeline m_StrokeRectPipeline {};
  gk::graphics::GraphicsPipeline m_StrokeCirclePipeline {};
  gk::graphics::GraphicsPipeline m_FullscreenPipeline {};

  gk::graphics::Sampler m_FullscreenSampler;

  gm::Float4x4* m_ViewProjection {nullptr};
  gk::graphics::ICommandList* m_Cmd {nullptr};

  struct DrawCommandList
  {
    std::vector<DrawCommand> DrawCommands;
    u32 Offset {0};
  };

  void WriteDrawCommand(DrawCommandList& CommandList, gm::Float2 position, gm::Float3 Color, gm::Float2 size, f32 rotation);

  DrawCommandList m_FillRectCommands;
  DrawCommandList m_FillCircleCommands;
  DrawCommandList m_StrokeRectCommands;
  DrawCommandList m_StrokeCircleCommands;

  bool m_IsValid {false};
  u32 m_FrameIndex { 0 };

  u32 m_Width {0};
  u32 m_Height {0};

};

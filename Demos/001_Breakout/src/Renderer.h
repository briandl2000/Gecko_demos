#pragma once
#include "Common.h"
#include <gecko/graphics/command_list.h>
#include <gecko/math/matrix.h>

class Renderer
{
public:
  Renderer() = default;

  bool Init();

  void BeginRender(gk::graphics::ICommandList* cmd, gm::Float4x4* viewProjection);
  void EndRender();

  void DrawRect(gm::Float2 position, gm::Float3 Color, gm::Float2 size = {1., 1.}, f32 rotation = 0.);
private:
  bool CreateRenderResources();
  bool CreatePipelines();

  gk::graphics::Buffer m_RectVertexBuffer {};
  gk::graphics::Buffer m_RectIndexBuffer {};
  gk::graphics::GraphicsPipeline m_RectPipeline {};


  gm::Float4x4* m_ViewProjection {nullptr};
  gk::graphics::ICommandList* m_Cmd {nullptr};

};

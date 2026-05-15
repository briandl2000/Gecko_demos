#pragma once
#include "Common.h"


class Camera
{
public:
  Camera() = default;

  void Update(f32 dt, f32 width, f32 height);

  gm::Float4x4 GetViewProjection();

  gm::Float2 Position {0., 0.};
  f32 Rotation {0.};

private:
  gm::Float4x4 m_Projection {};
  gm::Float4x4 m_View {};
};

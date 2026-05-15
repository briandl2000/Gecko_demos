#include "Camera.h"
#include <gecko/math/matrix.h>
#include <gecko/math/quat.h>

void Camera::Update(f32 dt, f32 width, f32 height)
{
  f32 aspect = width/height;
  f32 halfBoxSize = 5.0f;

  f32 horizontal = halfBoxSize * aspect;
  f32 vertical = halfBoxSize;

  m_Projection = gm::Float4x4::Orthographic(
    -horizontal, horizontal,
    -vertical, vertical,
    -halfBoxSize, halfBoxSize
  );
  // No inversion of the matrices yet, so pass on the negative position.
  gm::Float3 position {Position.X, Position.Y, 0.f};
  m_View = gm::Float4x4::RotationZ(Rotation) * gm::Float4x4::Translation(-1.f * position);
}

gm::Float4x4 Camera::GetViewProjection()
{
  return m_Projection * m_View;
}

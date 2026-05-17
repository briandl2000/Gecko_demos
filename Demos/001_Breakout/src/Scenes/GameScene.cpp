#include "GameScene.h"
#include <algorithm>
#include <gecko/core/ptr.h>
#include <gecko/platform/input.h>
#include <gecko/platform/input_codes.h>
#include <gecko/platform/window.h>
#include "../Renderer.h"

void GameScene::Start()
{
  GECKO_INFO(Scene_Label, "Main menu start");

  WallStorage.resize(1028);
  BallStorage.resize(1028);
  PaddleStorage.resize(1028);
  BrickStorage.resize(1028);

  PaddleStorage.push_back({});
  m_Entities.push_back(&PaddleStorage[PaddleStorage.size()-1]);
  BallStorage.push_back({});
  m_Entities.push_back(&BallStorage[BallStorage.size()-1]);
  WallStorage.push_back({gm::Float2 {-5., 0.}, gm::Float2 {0.05, 8.}});
  m_Entities.push_back(&WallStorage[WallStorage.size()-1]);
  WallStorage.push_back({gm::Float2 { 5., 0.}, gm::Float2 {0.05, 8.}});
  m_Entities.push_back(&WallStorage[WallStorage.size()-1]);
  WallStorage.push_back({gm::Float2 { 0., 4.}, gm::Float2 {10.0, 0.05}});
  m_Entities.push_back(&WallStorage[WallStorage.size()-1]);

  BrickStorage.resize(1028);

  u32 brickIdx = 0;

  for (i32 j = 0; j < 6; j++)
  {
    f32 y = j * 0.6;
    {
      i32 count = 16;
      for(i32 i = 0; i < count; i++)
      {
        if (brickIdx >= 1028) break;
        f32 pacing = 0.53;
        f32 x = static_cast<f32>(i-(count>>1)) * pacing + pacing/2;
        BrickStorage.push_back({gm::Float2 { x,  y+0.3f}});
        m_Entities.push_back(&BrickStorage[BrickStorage.size()-1]);
      }
    }
    {
      i32 count = 15;
      for(i32 i = 0; i < count; i++)
      {
        if (brickIdx >= 1028) break;
        f32 pacing = 0.53;
        f32 x = static_cast<f32>(i-(count>>1)) * pacing;
        BrickStorage.push_back({gm::Float2 { x, y+0.0f}});
        m_Entities.push_back(&BrickStorage[BrickStorage.size()-1]);
      }
    }
  }
}

void GameScene::Stop()
{
}

void GameScene::Tick(f32 dt)
{
  for (auto e : m_Entities)
  {
    e->BeginCollisionChecks();
  }

  for (u32 i = 0; i < m_Entities.size(); i++)
  {
    auto entity = m_Entities[i];
    for (u32 j = i+1; j < m_Entities.size(); j++)
    {
      auto other = m_Entities[j];
      entity->CheckCollision(other);
      other->CheckCollision(entity);
    }
  }

  for (auto e : m_Entities)
  {
    e->Tick(dt);
  }

  m_Entities.erase(
      std::remove_if(m_Entities.begin(), m_Entities.end(),
          [](auto& e)
          {
              return e->IsDead();
          }),
      m_Entities.end()
  );
}

SceneRequest GameScene::Update(f32 dt, const gk::platform::Extent2D& windowSize)
{
  if(gk::platform::GetInput()->WasKeyPressed(gk::platform::KeyCode::Enter))
  {
    return SceneRequest::SwitchTo(SceneID::GameOver);
  }

  u32 width = windowSize.Width;
  u32 height = windowSize.Height;

  f32 zoom = 1.f;
  if(gk::platform::GetInput()->IsKeyDown(gk::platform::KeyCode::Up))
  {
      zoom -= 0.9*dt;
  }
  if(gk::platform::GetInput()->IsKeyDown(gk::platform::KeyCode::Down))
  {
      zoom += 0.9*dt;
  }
  m_Camera.Zoom *= zoom;

  m_Camera.Update(dt, static_cast<f32>(width), static_cast<f32>(height));

  return SceneRequest::None();
}

void GameScene::Render(gk::graphics::ICommandList* cmd, Renderer& renderer, u32 frameIndex)
{
  gm::Float4x4 viewProj = m_Camera.GetViewProjection();
  renderer.BeginRender(cmd, &viewProj, frameIndex);
  for (auto e : m_Entities)
  {
    e->Render(&renderer);
  }
  renderer.EndRender();

}

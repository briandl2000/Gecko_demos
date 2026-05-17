#include "GameOverScene.h"
#include <algorithm>
#include <gecko/core/ptr.h>
#include <gecko/platform/input.h>
#include <gecko/platform/input_codes.h>
#include <gecko/platform/window.h>
#include "../Renderer.h"

void GameOverScene::Start()
{
  GECKO_INFO(Scene_Label, "Main menu start");

  WallStorage.resize(1028);
  BallStorage.resize(1028);
  PaddleStorage.resize(1028);
  BrickStorage.resize(1028);

  PaddleStorage.push_back({});
  m_Entities.push_back(&PaddleStorage[PaddleStorage.size()-1]);
  WallStorage.push_back({gm::Float2 {-5., 0.}, gm::Float2 {0.05, 8.}});
  m_Entities.push_back(&WallStorage[WallStorage.size()-1]);
  WallStorage.push_back({gm::Float2 { 5., 0.}, gm::Float2 {0.05, 8.}});
  m_Entities.push_back(&WallStorage[WallStorage.size()-1]);
  WallStorage.push_back({gm::Float2 { 0., 4.}, gm::Float2 {10.0, 0.05}});
  m_Entities.push_back(&WallStorage[WallStorage.size()-1]);
}

void GameOverScene::Stop()
{
}

void GameOverScene::Tick(f32 dt)
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

SceneRequest GameOverScene::Update(f32 dt, const gk::platform::Extent2D& windowSize)
{

  if(gk::platform::GetInput()->WasKeyPressed(gk::platform::KeyCode::Enter))
  {
    return SceneRequest::SwitchTo(SceneID::MainMenu);
  }

  u32 width = windowSize.Width;
  u32 height = windowSize.Height;

  f32 zoom = 1.f;
  if(gk::platform::GetInput()->IsKeyDown(gk::platform::KeyCode::Up))
  {
      zoom -= 0.1*dt;
  }
  if(gk::platform::GetInput()->IsKeyDown(gk::platform::KeyCode::Down))
  {
      zoom += 0.1*dt;
  }
  m_Camera.Zoom *= zoom;

  m_Camera.Update(dt, static_cast<f32>(width), static_cast<f32>(height));

  return SceneRequest::None();
}


void GameOverScene::Render(gk::graphics::ICommandList* cmd, Renderer& renderer, u32 frameIndex)
{
  gm::Float4x4 viewProj = m_Camera.GetViewProjection();
  renderer.BeginRender(cmd, &viewProj, frameIndex);
  for (auto e : m_Entities)
  {
    e->Render(&renderer);
  }
  renderer.EndRender();

}

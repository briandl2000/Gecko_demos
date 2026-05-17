#pragma once

#include "Common.h"

#include "Camera.h"
#include "Entity.h"
#include <gecko/core/labels.h>
#include <gecko/graphics/command_list.h>
#include <gecko/platform/window.h>

class Renderer;

constexpr gk::Label Scene_Label = gk::MakeLabel("app.breakout.scene");

enum class SceneID : u8
{
  MainMenu = 0,
  Game,
  GameOver
};

struct SceneRequest
{
  enum class Type : u8
  {
    None = 0,
    Switch,
    Quit
  };

  Type type = Type::None;
  SceneID target = SceneID::MainMenu;

  static SceneRequest None() { return {}; }

  static SceneRequest SwitchTo(SceneID scene)
  {
    SceneRequest request;
    request.type = Type::Switch;
    request.target = scene;
    return request;
  }

  static SceneRequest Quit(SceneID scene)
  {
    SceneRequest request;
    request.type = Type::Quit;
    return request;
  }

};

class Scene
{
public:
  Scene() = default;
  virtual ~Scene() = default;

  virtual void Start() = 0;
  virtual void Stop() = 0;
  virtual void Tick(f32 dt) = 0;
  [[nodiscard]]
  virtual SceneRequest Update(f32 dt, const gk::platform::Extent2D& windowSize) = 0;
  virtual void Render(gk::graphics::ICommandList* cmd, Renderer& renderer, u32 frameIndex) = 0;

protected:
  Camera m_Camera;
  std::vector<Entity*> m_Entities;

};

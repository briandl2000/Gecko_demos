#pragma once
#include "../Common.h"
#include "../Scene.h"

class GameOverScene : public Scene
{
public:
  GameOverScene() = default;
  virtual ~GameOverScene() = default;

  virtual void Start() override;
  virtual void Stop() override;
  virtual void Tick(f32 dt) override;
  [[nodiscard]]
  virtual SceneRequest Update(f32 dt, const gk::platform::Extent2D& windowSize) override;
  virtual void Render(gk::graphics::ICommandList* cmd, Renderer& renderer, u32 frameIndex) override;

private:

  std::vector<Wall> WallStorage;
  std::vector<Ball> BallStorage;
  std::vector<Paddle> PaddleStorage;
  std::vector<Brick> BrickStorage;

};

#pragma once

#include "Common.h"
#include <gecko/math/aabb.h>
#include <vector>

class Renderer;

enum class EntityType : u8
{
  None = 0,
  Paddle,
  Ball,
  Brick,
  Wall
};

struct CollisionInfo
{
  EntityType otherType;
  gm::Aabb2 otherBounds;
  gm::Aabb2 bounds;
  gm::Float2 otherPosition;
};

class Entity
{
public:

  virtual ~Entity() = default;

  virtual void Render(Renderer* renderer);
  virtual void Tick(f32 dt) {};
  virtual void Update(f32 dt) {};
  virtual EntityType GetType() { return EntityType::None; }

  bool IsDead() { return m_IsDead; }

  void BeginCollisionChecks();
  void CheckCollision(Entity* other);

  gm::Aabb2 GetBounds() {return {Position-Size*0.5, Position+Size*0.5}; }

  gm::Float2 Position {};
  gm::Float2 Size {};
  gm::Float3 Color {};

protected:
  ::std::vector<CollisionInfo> m_LastCollisionBounds;
  bool m_IsDead {false};

private:
};

class Paddle : public Entity
{
public:
  Paddle();

  virtual void Tick(f32 dt) override;
  virtual EntityType GetType() override { return EntityType::Paddle; }

  f32 Speed {10.f};
private:
  void HandleCollision(const CollisionInfo& collision);

};

class Ball : public Entity
{
public:
  Ball();

  virtual void Render(Renderer* renderer) override;
  virtual void Tick(f32 dt) override;
  virtual EntityType GetType() override { return EntityType::Ball; }

private:
void HandleCollision(const CollisionInfo& collision);

  std::vector<gm::Float2> m_PreviousPositions;
  u32 m_PreviousPositionsHead = 0;
  u32 m_PreviousPositionsCount = 0;

  static constexpr u32 MaxPreviousPositions = 20;

  f32 m_PositionSampleTimer = 0.0f;

  gm::Float2 m_Velocity {};
};

class Brick : public Entity
{
public:
  Brick() = default;
  Brick(gm::Float2 position);

  virtual void Tick(f32 dt) override;
  virtual EntityType GetType() override { return EntityType::Brick; }

private:
};

class Wall : public Entity
{
public:
  Wall() = default;
  Wall(gm::Float2 position, gm::Float2 size);

  virtual EntityType GetType() override { return EntityType::Wall; }


private:
};

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
  virtual void Update(f32 dt) = 0;
  virtual EntityType GetType() { return EntityType::None; }

  void BeginCollisionChecks();
  void CheckCollision(Entity* other);

  gm::Aabb2 GetBounds() {return {Position-Size*0.5, Position+Size*0.5}; }

  gm::Float2 Position {};
  gm::Float2 Size {};
  gm::Float3 Color {};

protected:
  ::std::vector<CollisionInfo> m_LastCollisionBounds;

private:
};

class Paddle : public Entity
{
public:
  Paddle();

  virtual void Update(f32 dt) override;
  virtual EntityType GetType() override { return EntityType::Paddle; }

  f32 Speed {10.f};
private:

};

class Ball : public Entity
{
public:
  Ball();

  virtual void Update(f32 dt) override;
  virtual EntityType GetType() override { return EntityType::Ball; }

private:
  gm::Float2 m_Velocity {};
};

class Brick : public Entity
{
public:

  virtual EntityType GetType() override { return EntityType::Brick; }

private:
};

#include "Entity.h"
#include "Renderer.h"
#include <gecko/core/services/jobs.h>
#include <gecko/math/aabb.h>
#include <gecko/math/vector.h>
#include <gecko/platform/input.h>
#include <gecko/platform/input_codes.h>
#include <gecko/core/utility/random.h>

void Entity::Render(Renderer* renderer)
{
  renderer->FillRect(Position, Color, Size, 0.);
}

void Entity::BeginCollisionChecks()
{
  m_LastCollisionBounds.clear();
}

void Entity::CheckCollision(Entity* other)
{
  gm::Aabb2 myBounds = GetBounds();
  gm::Aabb2 otherBounds = other->GetBounds();

  if(gm::Intersects(myBounds, otherBounds))
  {
    gm::Aabb2 overlappedBounds {
      {
        std::max(myBounds.Min.X, otherBounds.Min.X),
        std::max(myBounds.Min.Y, otherBounds.Min.Y)
      },
      {
        std::min(myBounds.Max.X, otherBounds.Max.X),
        std::min(myBounds.Max.Y, otherBounds.Max.Y)
      }
    };
    m_LastCollisionBounds.push_back({
      other->GetType(),
      otherBounds,
      overlappedBounds,
      other->Position
    });
  }
}

Paddle::Paddle()
{
  Position = {0., -3};
  Size = {1., 0.3};
  Color = Colors::paddle_color;
}

void Paddle::HandleCollision(const CollisionInfo& collision)
{
  f32 overlapX = collision.bounds.Max.X - collision.bounds.Min.X;
  f32 overlapY = collision.bounds.Max.Y - collision.bounds.Min.Y;

  gm::Aabb2 myBounds = GetBounds();
  gm::Float2 otherPosition = collision.otherPosition;

  if (overlapY < overlapX)
  {
    if (Position.Y < otherPosition.Y)
    {
      Position.Y -= overlapY;
    }
    else
    {
      Position.Y += overlapY;
    }
  }
  else
  {
    if (Position.X < otherPosition.X)
    {
      Position.X -= overlapX;
    }
    else
    {
      Position.X += overlapX;
    }
  }
}

void Paddle::Update(f32 dt)
{
  gm::Float2 movement {};
  if(gk::platform::GetInput()->IsKeyDown(gk::platform::KeyCode::D))
  {
    movement.X += 1.f;
  }
  if(gk::platform::GetInput()->IsKeyDown(gk::platform::KeyCode::A))
  {
    movement.X -= 1.f;
  }
  if (movement.X != 0.f || movement.Y != 0.f)
  {
    movement = gm::Normalized(movement);
  }
  for(const auto collision : m_LastCollisionBounds)
  {
    if (collision.otherType == EntityType::Wall)
    {
      HandleCollision(collision);
    }
  }

  Position = Position + movement * Speed * dt;

}

Ball::Ball()
{
  Position = {0., -1.};
  Size = {0.3, 0.3};
  Color = Colors::paddle_color;
  m_Velocity = {.3, -2.f};
}

void Ball::Render(Renderer* renderer)
{
  renderer->FillCircle(Position, Color, Size);
}

void Ball::HandleCollision(const CollisionInfo& collision)
{
  f32 overlapX = collision.bounds.Max.X - collision.bounds.Min.X;
  f32 overlapY = collision.bounds.Max.Y - collision.bounds.Min.Y;

  gm::Aabb2 myBounds = GetBounds();
  gm::Float2 otherPosition = collision.otherPosition;

  if (overlapY < overlapX)
  {
    if (Position.Y < otherPosition.Y)
    {
      Position.Y -= overlapY;
    }
    else
    {
      Position.Y += overlapY;
    }

    m_Velocity.Y = -m_Velocity.Y;
  }
  else
  {
    if (Position.X < otherPosition.X)
    {
      Position.X -= overlapX;
    }
    else
    {
      Position.X += overlapX;
    }
    m_Velocity.X = -m_Velocity.X;
  }
}

void Ball::Update(f32 dt)
{
  for(const auto collision : m_LastCollisionBounds)
  {
    HandleCollision(collision);
  }

  Position = Position + m_Velocity * dt;
}

Brick::Brick(gm::Float2 postion)
{
  Position = postion;
  Size = gm::Float2(0.5, 0.2);
  Color = Colors::brick_normal;
}

void Brick::Update(f32 dt)
{
  for(const auto collision : m_LastCollisionBounds)
  {
    if(collision.otherType == EntityType::Ball)
    {
      m_IsDead = true;
    }
  }
}

Wall::Wall(gm::Float2 position, gm::Float2 size)
{
  Position = position;
  Size = size;
  Color = Colors::yellow;
}

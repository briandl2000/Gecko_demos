#include "Entity.h"
#include "Renderer.h"
#include <gecko/math/aabb.h>
#include <gecko/math/vector.h>
#include <gecko/platform/input.h>
#include <gecko/platform/input_codes.h>

void Entity::Render(Renderer* renderer)
{
  renderer->DrawRect(Position, Color, Size, 0.);
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
  Position = {0., -4};
  Size = {1., 0.3};
  Color = {0.4, 0.5, 0.9};
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
  Position = Position + movement * Speed * dt;
}

Ball::Ball()
{
  Position = {0., 0.};
  Size = {0.3, 0.3};
  Color = {0.7, 0.4, 0.2};
  m_Velocity = {.3, -2.f};
}

void Ball::Update(f32 dt)
{
  for(auto collision : m_LastCollisionBounds)
  {
    if (collision.otherType == EntityType::Paddle)
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
  }

  Position = Position + m_Velocity * dt;
}

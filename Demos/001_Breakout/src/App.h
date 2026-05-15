#pragma once
#include "Common.h"
#include "Camera.h"
#include "Renderer.h"
#include "Entity.h"

class App
{
public:
  App();
  ~App();
  App(const App&) = delete;
  App& operator=(const App&) = delete;

  [[nodiscard]] bool IsValid() const noexcept
  {
    return m_Engine.has_value() && m_Device != nullptr;
  }

  int Run();

private:
  class BreakoutModule final : public gk::IModule
  {
  public:
    [[nodiscard]] gk::Label RootLabel() const noexcept override;
    [[nodiscard]] bool Startup(gk::IModuleRegistry&) noexcept override;
    void Shutdown(gk::IModuleRegistry&) noexcept override;
  };

  static gk::graphics::GraphicsConfig MakeGraphicsConfig() noexcept;

  bool CreateWindow();
  bool CreateSwapchain();
  void ConfigureProfiler();
  void SubscribeEvents();

  void RenderFrame();
  void Tick(f32 dt);
  void Start();
  void Update();
  void OnKey(gk::platform::KeyCode key);
  void PrintHudIfDue(f32 drawCalls);

  gk::runtime::TrackingAllocator m_Allocator;
  gk::AllocatorScope m_AllocScope {m_Allocator};

  gk::runtime::RuntimeModule m_RuntimeModule;
  gk::platform::PlatformModule m_PlatformModule;
  gk::graphics::GraphicsModule m_GraphicsModule;
  BreakoutModule m_AppModule;

  ::std::optional<gk::Engine> m_Engine;
  ::std::optional<gk::runtime::StandardLogSinks> m_LogSinks;

  gk::graphics::GraphicsDevice* m_Device = nullptr;
  gk::graphics::IGpuSampler* m_GpuSampler = nullptr;

  gk::platform::WindowHandle m_WindowHandle {};
  gk::graphics::Swapchain m_SwapChain {};

  gk::EventSubscription m_CloseSub {};
  gk::EventSubscription m_ResizeSub {};
  gk::EventSubscription m_KeySub {};

  // Loop state.
  bool m_Running = true;
  u64 m_FrameIndex = 0;
  u64 m_LastHudPrintNs = 0;

  Camera m_Camera;
  Renderer m_Renderer;

  std::vector<Entity*> m_Entities;
};

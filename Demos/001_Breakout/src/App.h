#pragma once
#include "Common.h"
#include "Renderer.h"
#include "Scene.h"
#include <gecko/core/ptr.h>

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

  static gk::graphics::GraphicsConfig MakeGraphicsConfig() noexcept;

  void ConfigureProfiler();
  bool CreateWindow();
  bool CreateSwapchain();

  void SubscribeEvents();
  void OnKey(gk::platform::KeyCode key);

  void Start();
  void Update();
  void Tick(f32 dt);
  void RenderFrame();

  void PrintHudIfDue(f32 drawCalls);

  gk::Unique<Scene> CreateScene(SceneID id);
  void SwitchScene(SceneID id);

  gk::runtime::TrackingAllocator m_Allocator;
  gk::AllocatorScope m_AllocScope {m_Allocator};

  gk::runtime::RuntimeModule m_RuntimeModule;
  gk::platform::PlatformModule m_PlatformModule;
  gk::graphics::GraphicsModule m_GraphicsModule;

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

  Renderer m_Renderer;

  gk::Unique<Scene> m_Scene;

};

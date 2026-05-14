#pragma once

#include <chrono>
#include <gecko/core/engine.h>
#include <gecko/core/ptr.h>
#include <gecko/core/scope.h>
#include <gecko/core/services/events.h>
#include <gecko/core/services/memory.h>
#include <gecko/core/services/modules.h>
#include <gecko/core/types.h>
#include <gecko/graphics/gpu_profiler.h>
#include <gecko/graphics/graphics_device.h>
#include <gecko/graphics/graphics_module.h>
#include <gecko/platform/platform_module.h>
#include <gecko/runtime/runtime_module.h>
#include <gecko/runtime/standard_log_sinks.h>
#include <gecko/runtime/tracking_allocator.h>
#include <optional>

namespace gk = gecko;
using u8 = gk::u8;
using u16 = gk::u16;
using u32 = gk::u32;
using u64 = gk::u64;
using i8 = gk::i8;
using i16 = gk::i16;
using i32 = gk::i32;
using i64 = gk::i64;
using f32 = gk::f32;
using f64 = gk::f64;

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
  bool CreateRenderResources();
  bool CreatePipelines();
  void ConfigureProfiler();
  void SubscribeEvents();

  void RecordTrianglePass(gk::graphics::ICommandList& cmd, f32 time, gk::graphics::RenderTarget target);
  void RenderFrame();
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
  bool m_ResizeDirty = false;
  u32 m_ResizeW = 0;
  u32 m_ResizeH = 0;

  gk::graphics::Buffer m_VertexBuffer {};
  gk::graphics::Buffer m_IndexBuffer {};

  gk::graphics::GraphicsPipeline m_TrianglePipeline {};

  gk::EventSubscription m_CloseSub {};
  gk::EventSubscription m_ResizeSub {};
  gk::EventSubscription m_KeySub {};

  // Loop state.
  bool m_Running = true;
  ::std::chrono::steady_clock::time_point m_StartTime {};
  u64 m_FrameIndex = 0;
  u64 m_LastHudPrintNs = 0;
};

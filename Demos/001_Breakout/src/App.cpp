#include "App.h"
#include "Entity.h"
#include "Scene.h"
#include "Scenes/GameOverScene.h"
#include "Scenes/GameScene.h"
#include "Scenes/MainMenuScene.h"


#include <chrono>
#include <gecko/core/ptr.h>
#include <gecko/math/vector.h>
#include <gecko/platform/window.h>
#include <gecko/platform/platform_module.h>
#include <gecko/core/labels.h>
#include <gecko/core/services.h>
#include <gecko/core/services/log.h>
#include <gecko/core/services/memory.h>
#include <gecko/core/services/profiler.h>
#include <gecko/core/utility/thread.h>
#include <gecko/core/version.h>
#include <gecko/graphics/graphics_types.h>
#include <gecko/math/matrix.h>
#include <gecko/platform/input.h>
#include <gecko/platform/input_codes.h>
#include <gecko/platform/platform_events.h>
#include <gecko/platform/windows_interface.h>
#include <gecko/platform/input.h>
#include <utility>

namespace
{
  constexpr gk::Label App_Label = gk::MakeLabel("app.breakout");
  constexpr gk::Label Main_Label = gk::MakeLabel("app.breakout.main");
}

// Constructor destructor ------------------------

App::App() : m_GraphicsModule(MakeGraphicsConfig())
{
  if (!m_AllocScope)
  {
    return;
  }

  m_Engine = gk::Engine::Create({&m_RuntimeModule, &m_PlatformModule, &m_GraphicsModule});
  if (!m_Engine)
  {
    return;
  }

  m_Device = gk::graphics::GetGraphicsDevice();
  m_GpuSampler = gk::graphics::GetGpuSampler();
  if (!m_Device)
  {
    GECKO_ERROR(Main_Label, "GraphicsModule did not publish a device");
    return;
  }

  ConfigureProfiler();
  m_LogSinks.emplace();
  gk::SetThreadProfilerName("main");

  GECKO_INFO(Main_Label, gk::VersionFullString());

  if (!CreateWindow())
    return;
  if (!CreateSwapchain())
    return;
  if(!m_Renderer.Init(m_SwapChain.Desc.NumBackBuffers, m_SwapChain.Desc.Width, m_SwapChain.Desc.Height))
    return;
  SubscribeEvents();
}

App::~App()
{
  if (m_Device)
  {
    if (m_SwapChain.IsValid())
    {
      m_Device->DestroySwapchain(m_SwapChain);
    }
  }
  if (m_Engine)
  {
    if (m_WindowHandle.IsValid())
    {
      gk::platform::GetWindows()->DestroyWindow(m_WindowHandle);
    }
  }
}

// Creation stuff ------------------------

void App::ConfigureProfiler()
{
  gk::GetProfiler()->SetMinLevel(gk::ProfLevel::Normal);
  if (auto* prof = gk::GetProfiler())
  {
    prof->WatchScope("frame", 240);
    prof->WatchScope("frameGPU", 240, gk::ProfSource::GPU);
    prof->WatchScope("renderFrame", 240);
    prof->WatchScope("tick", 240);
    prof->SetStatsResetIntervalMs(500);
  }
}

gk::graphics::GraphicsConfig App::MakeGraphicsConfig() noexcept
{
  gk::graphics::GraphicsConfig cfg {};
  cfg.Backend = gk::graphics::GraphicsBackend::Vulkan;
  cfg.Debug = false;
  cfg.AppName = "Breakout";
  cfg.Sampler.MaxZonesPerFrame = 64;
  cfg.Sampler.FramesInFlight = 3;
  cfg.Sampler.GpuThreadName = "GPU.Graphics";
  return cfg;
}

bool App::CreateWindow()
{
  static const char* WindowTitle = "Breakout";

  gk::platform::WindowDesc windowDesc;
  windowDesc.Title = WindowTitle;
  windowDesc.Size = {1280, 720};
  windowDesc.Visible = true;
  windowDesc.Resizable = false;
  windowDesc.Mode = gk::platform::WindowMode::Windowed;
  m_WindowHandle = gk::platform::GetWindows()->CreateWindow(windowDesc);
  if (!m_WindowHandle.IsValid())
  {
    GECKO_ERROR(Main_Label, "Failed to create window");
    return false;
  }

  GECKO_INFO(Main_Label, "Window created");
  return true;
}

bool App::CreateSwapchain()
{
  gk::platform::NativeWindowHandle native = gk::platform::GetWindows()->GetNativeWindowHandle(m_WindowHandle);
  gk::platform::Extent2D size = gk::platform::GetWindows()->GetClientSize(m_WindowHandle);

  gk::graphics::SwapchainDesc swapchainDesc;
  swapchainDesc.Width = size.Width;
  swapchainDesc.Height = size.Height;
  swapchainDesc.NumBackBuffers = 2;
  swapchainDesc.Format = gk::graphics::DataFormat::R8G8B8A8_UNORM;
  swapchainDesc.VSync = false;

  m_SwapChain = m_Device->CreateSwapchain(native, swapchainDesc);
  if (!m_SwapChain.IsValid())
  {
    GECKO_WARN(Main_Label, "Swapchain %u not created (NullDevice?)");
  }
  return true;
}

// Events stuff ----------------------------

void App::SubscribeEvents()
{
  m_CloseSub = gk::SubscribeEvent(
      gk::platform::events::WindowCloseRequested,
      [](void* user, const gk::EventMeta&, gk::EventView) { static_cast<App*>(user)->m_Running = false; },
      this
  );

  m_ResizeSub = gk::SubscribeEvent(
      gk::platform::events::WindowResized,
      [](void* user, const gk::EventMeta&, gk::EventView view) {
        const auto* payload = static_cast<const gk::platform::events::WindowResizedPayload*>(view.Data());
        auto* self = static_cast<App*>(user);
        if (payload->Width > 0 && payload->Height > 0)
        {
          self->m_SwapChain.Desc.Width = payload->Width;
          self->m_SwapChain.Desc.Height = payload->Height;
          self->m_Device->ResizeSwapchain(self->m_SwapChain);
        }
      },
      this
  );

  m_KeySub = gk::SubscribeEvent(
      gk::platform::events::WindowKey,
      [](void* user, const gk::EventMeta&, gk::EventView view) {
        const auto* p = static_cast<const gk::platform::events::WindowKeyPayload*>(view.Data());
        if (!p->Down || p->Repeat)
          return;
        static_cast<App*>(user)->OnKey(p->Key);
      },
      this
  );

  gk::platform::SetModalFrameCallback([](void* ud) { static_cast<App*>(ud)->Update(); }, this);
}

void App::OnKey(gk::platform::KeyCode key)
{
  switch (key)
  {
  case gk::platform::KeyCode::Escape:
    m_Running = false;
    break;
  case gk::platform::KeyCode::F4:
    if (auto* prof = gk::GetProfiler())
      prof->DumpStats(Main_Label);
    break;
  default:
    break;
  }
}

// Update stuff ----------------------------

int App::Run()
{
  if (!IsValid())
    return 1;

  GECKO_PUSH_LABEL(Main_Label);
  GECKO_SCOPE_ALWAYS_NAMED(Main_Label, "AppRun");

  GECKO_INFO(Main_Label, "Entering frame loop - Escape or close to quit");

  Start();
  while (m_Running)
  {
    gk::platform::PumpEvents();
    Update();
  }

  gk::platform::SetModalFrameCallback(nullptr, nullptr);
  GECKO_INFO(Main_Label, "Shutdown complete");
  return 0;
}

constexpr f32 FIXED_DT = 1.0f/240.f;

void App::Update()
{
  GECKO_SCOPE_ALWAYS_NAMED(Main_Label, "frame");
  static f32 lastTime = ::std::chrono::duration<f32>(::std::chrono::steady_clock::now().time_since_epoch()).count();
  f32 currentTime = ::std::chrono::duration<f32>(::std::chrono::steady_clock::now().time_since_epoch()).count();
  f32 dt = currentTime - lastTime;
  lastTime = currentTime;

  static f32 accumulator = 0.0f;
  accumulator += dt;

  (void)gk::DispatchEvents();

  while(accumulator >= FIXED_DT)
  {
    Tick(FIXED_DT);
    accumulator -= FIXED_DT;
  }

  SceneRequest request = m_Scene->Update(dt, gk::platform::GetWindows()->GetClientSize(m_WindowHandle));

  if(request.type == SceneRequest::Type::Quit)
  {
    m_Running = false;
    return;
  }

  if(request.type == SceneRequest::Type::Switch)
  {
    SwitchScene(request.target);
    return;
  }

  RenderFrame();
  GECKO_FRAME(Main_Label, "Frame");
}

void App::Start()
{
  SwitchScene(SceneID::MainMenu);
}

void App::Tick(f32 dt)
{
  GECKO_SCOPE_ALWAYS_NAMED(Main_Label, "tick");

  m_Scene->Tick(dt);
}

void App::RenderFrame()
{
  GECKO_SCOPE_ALWAYS_NAMED(Main_Label, "renderFrame");

  gk::graphics::FrameContext frame {};
  if (m_SwapChain.IsValid())
  {
    frame = m_Device->BeginFrame(m_SwapChain);
  }

  if (!frame.Valid)
    return;

  auto cmd = m_Device->CreateGraphicsCommandList();
  cmd->Begin();

  if (m_GpuSampler)
  {
    m_GpuSampler->BeginFrame(*cmd);
    cmd->AttachGpuSampler(m_GpuSampler, Main_Label);
  }
  {
    GECKO_GPU_SCOPE_ALWAYS_NAMED(*cmd, Main_Label, "frameGPU");

    m_Scene->Render(cmd.get(), m_Renderer, frame.FrameIndex);

    u32 width = frame.BackBuffer.Desc.Width;
    u32 height = frame.BackBuffer.Desc.Height;
    gk::graphics::ClearValue clearValue = gk::graphics::ClearValue::RenderTarget(0.18F, 0.18F, 0.3F, 1.0F);
    gk::graphics::BeginRenderingInfo beginRenderInfo = {
        .Colors = {&frame.BackBuffer, 1},
        .ClearColors = {&clearValue, 1},
    };
    cmd->BeginRendering(beginRenderInfo);
    cmd->SetViewport(0.0F, 0.0F, static_cast<f32>(width), static_cast<f32>(height));
    cmd->SetScissor(0, 0, width, height);

    m_Renderer.DrawToScreen(cmd.get());

    cmd->EndRendering();

  }
  if (m_GpuSampler)
    m_GpuSampler->EndFrame(*cmd);

  cmd->End();
  m_Device->ExecuteGraphicsCommandList(::std::move(cmd));

  gk::graphics::FrameContext toPresent {};
  if (frame.Valid)
    toPresent = ::std::move(frame);
  m_Device->Present(::std::span<const gk::graphics::FrameContext> {&toPresent, 1});

  f32 drawCalls = 1u;
  if (frame.Valid)
    ++drawCalls;
  GECKO_COUNTER(Main_Label, "DrawCalls", drawCalls);
  GECKO_COUNTER(Main_Label, "AllocLiveBytes", m_Allocator.TotalLiveBytes());

  PrintHudIfDue(drawCalls);
  ++m_FrameIndex;
}

// Scene Stuff ----------------------------

gk::Unique<Scene> App::CreateScene(SceneID id)
{
  switch (id)
  {
    case SceneID::MainMenu:
      return gk::CreateUnique<MainMenuScene>();
    case SceneID::Game:
      return gk::CreateUnique<GameScene>();
    case SceneID::GameOver:
      return gk::CreateUnique<GameOverScene>();
    default:
      break;
  }
  GECKO_WARN(App_Label, "Unkown scene type!");
  return nullptr;
}

void App::SwitchScene(SceneID id)
{
  if(m_Scene)
  {
    m_Scene->Stop();
  }

  m_Scene = CreateScene(id);

  if(m_Scene)
  {
    m_Scene->Start();
  }
}


// Utility stuff --------------------------

void App::PrintHudIfDue(f32 drawCalls)
{
  auto* prof = gk::GetProfiler();
  if (!prof)
    return;
  const u64 nowNs = prof->NowNs();
  if (nowNs - m_LastHudPrintNs <= 1'000'000'000ULL)
    return;
  m_LastHudPrintNs = nowNs;

  const auto frame = prof->GetStats("frame");
  const auto rend = prof->GetStats("renderFrame");
  const auto gpuFrame = prof->GetStats("frameGPU", gk::ProfSource::GPU);
  const auto tick = prof->GetStats("tick");
  const f64 fps = (frame.AvgNs > 0) ? 1.0e9 / static_cast<f64>(frame.AvgNs) : 0.0;
  const u64 live = m_Allocator.TotalLiveBytes();
  GECKO_INFO(Main_Label,
             "\n\tHUD FRAME STATS:\n\t\tframe=%.2fms (%.1f fps) \n\t\tcpu_render=%.2fms  "
             "\n\t\tgpu_frame=%.3fms  "
             "\n\t\ttick=%.3fms  "
             "\n\t\talloc_live=%llu KB\n",
             frame.AvgNs / 1.0e6, fps, rend.AvgNs / 1.0e6, gpuFrame.AvgNs / 1.0e6, tick.AvgNs / 1.0e6,
             static_cast<unsigned long long>(live / 1024));
}

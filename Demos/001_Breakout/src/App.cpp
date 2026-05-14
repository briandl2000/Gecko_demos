#include "App.h"

#include "shaders.h"

#include <cmath>
#include <gecko/core/labels.h>
#include <gecko/core/services.h>
#include <gecko/core/services/log.h>
#include <gecko/core/services/memory.h>
#include <gecko/core/services/profiler.h>
#include <gecko/core/utility/thread.h>
#include <gecko/core/version.h>
#include <gecko/graphics/graphics_types.h>
#include <gecko/platform/platform_events.h>
#include <gecko/platform/windows_interface.h>
#include <utility>

namespace
{

  constexpr gk::Label App_Label = gk::MakeLabel("app.breakout");
  constexpr gk::Label Main_Label = gk::MakeLabel("app.breakout.main");

  struct Vertex
  {
    float Position[3];
    float Color[3];
  };

  constexpr Vertex TriangleVertices[] = {
      {{0.0F, 0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
      {{0.5F, -0.5F, 0.0F}, {0.0F, 1.0F, 0.0F}},
      {{-0.5F, -0.5F, 0.0F}, {0.0F, 0.0F, 1.0F}},
  };

}

gk::Label App::BreakoutModule::RootLabel() const noexcept
{
  return App_Label;
}

bool App::BreakoutModule::Startup(gk::IModuleRegistry&) noexcept
{
  return true;
}

void App::BreakoutModule::Shutdown(gk::IModuleRegistry&) noexcept
{}

App::App() : m_GraphicsModule(MakeGraphicsConfig())
{
  if (!m_AllocScope)
  {
    return;
  }

  m_Engine = gk::Engine::Create({&m_RuntimeModule, &m_PlatformModule, &m_GraphicsModule, &m_AppModule});
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
  if (!CreateRenderResources())
    return;
  if (!CreatePipelines())
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

void App::ConfigureProfiler()
{
  gk::GetProfiler()->SetMinLevel(gk::ProfLevel::Normal);
  if (auto* prof = gk::GetProfiler())
  {
    prof->WatchScope("frame", 240);
    prof->WatchScope("frameGPU", 240, gk::ProfSource::GPU);
    prof->WatchScope("renderFrame", 240);
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

bool App::CreateRenderResources()
{
  gk::graphics::VertexBufferDesc vertexBufferDesc;
  vertexBufferDesc.NumVertices = 3;
  vertexBufferDesc.VertexSize = sizeof(Vertex);
  vertexBufferDesc.Memory = gk::graphics::MemoryType::Dedicated;
  m_VertexBuffer = m_Device->CreateVertexBuffer(vertexBufferDesc);
  if (m_VertexBuffer.IsValid())
  {
    const auto* raw = reinterpret_cast<const gk::byte*>(TriangleVertices);
    m_Device->UploadBufferData(m_VertexBuffer, {raw, sizeof(TriangleVertices)});
  }
  return true;
}

bool App::CreatePipelines()
{
  gk::graphics::VertexLayout triLayout;
  triLayout.AddAttribute(gk::graphics::DataFormat::R32G32B32_FLOAT, "a_Position");
  triLayout.AddAttribute(gk::graphics::DataFormat::R32G32B32_FLOAT, "a_Color");

  gk::graphics::GraphicsPipelineDesc triPDesc;
  triPDesc.VertexShader = gk::graphics::ShaderCode {
      .Format = gk::graphics::ShaderFormat::SPIRV,
      .Bytes = {reinterpret_cast<const gk::byte*>(shaders::TriangleVert), sizeof(shaders::TriangleVert)},
  };
  triPDesc.PixelShader = gk::graphics::ShaderCode {
      .Format = gk::graphics::ShaderFormat::SPIRV,
      .Bytes = {reinterpret_cast<const gk::byte*>(shaders::TriangleFrag), sizeof(shaders::TriangleFrag)},
  };
  triPDesc.Layout = triLayout;
  triPDesc.NumRenderTargets = 1;
  triPDesc.RenderTargetFormats[0] = gk::graphics::DataFormat::R8G8B8A8_UNORM;
  triPDesc.DepthStencilFormat = gk::graphics::DataFormat::D32_FLOAT;
  triPDesc.Culling = gk::graphics::CullMode::None;
  triPDesc.PushConstantBytes = 16;
  triPDesc.DebugName = "TrianglePipeline";
  m_TrianglePipeline = m_Device->CreateGraphicsPipeline(triPDesc);

  if (!m_TrianglePipeline.IsValid())
  {
    GECKO_WARN(Main_Label, "One or more pipelines failed to build - rendering disabled");
  }
  else
  {
    GECKO_INFO(Main_Label, "Pipelines ready");
  }
  return true;
}

void App::SubscribeEvents()
{
  m_CloseSub = gk::SubscribeEvent(
      gk::platform::events::WindowCloseRequested,
      [](void* user, const gk::EventMeta&, gk::EventView) { static_cast<App*>(user)->m_Running = false; },
      this
  );

  // m_ResizeSub = gk::SubscribeEvent(
  //     gk::platform::events::WindowResized,
  //     [](void* user, const gk::EventMeta&, gk::EventView view) {
  //       const auto* payload = static_cast<const gk::platform::events::WindowResizedPayload*>(view.Data());
  //       auto* self = static_cast<App*>(user);
  //       if (payload->Width > 0 && payload->Height > 0)
  //       {
  //         self->m_SwapChain.Desc.Width = payload->Width;
  //         self->m_SwapChain.Desc.Height = payload->Height;
  //         self->m_Device->ResizeSwapchain(self->m_SwapChain);
  //       }
  //     },
  //     this
  // );

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

void App::RecordTrianglePass(gk::graphics::ICommandList& cmd, f32 time, gk::graphics::RenderTarget target)
{
  cmd.BindPipeline(m_TrianglePipeline);
  cmd.BindVertexBuffer(m_VertexBuffer);
  const f32 pc[4] = {time, 0.0F, 0.0F, 0.0F};
  cmd.SetConstants(0, {reinterpret_cast<const gk::byte*>(pc), sizeof(pc)});
  cmd.Draw(3);
}

void App::RenderFrame()
{
  GECKO_SCOPE_ALWAYS_NAMED(Main_Label, "renderFrame");

  if (!m_TrianglePipeline.IsValid())
    return;

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
    u32 width = frame.BackBuffer.Desc.Width;
    u32 height = frame.BackBuffer.Desc.Height;
    gk::graphics::ClearValue clearValue = gk::graphics::ClearValue::RenderTarget(0.08F, 0.08F, 0.12F, 1.0F);
    gk::graphics::BeginRenderingInfo beginRenderInfo = {
        .Colors = {&frame.BackBuffer, 1},
        .ClearColors = {&clearValue, 1},
    };
    cmd->BeginRendering(beginRenderInfo);
    cmd->SetViewport(0.0F, 0.0F, static_cast<f32>(width), static_cast<f32>(height));
    cmd->SetScissor(0, 0, width, height);

    const f32 time =
        ::std::chrono::duration<f32>(::std::chrono::steady_clock::now() - m_StartTime).count();

    RecordTrianglePass(*cmd, time, frame.BackBuffer);

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
  const f64 fps = (frame.AvgNs > 0) ? 1.0e9 / static_cast<f64>(frame.AvgNs) : 0.0;
  const u64 live = m_Allocator.TotalLiveBytes();
  GECKO_INFO(Main_Label,
             "HUD frame=%.2fms (%.1f fps)  cpu_render=%.2fms  "
             "gpu_frame=%.3fms  "
             "alloc_live=%llu KB  frames=%llu",
             frame.AvgNs / 1.0e6, fps, rend.AvgNs / 1.0e6, gpuFrame.AvgNs / 1.0e6,
             static_cast<unsigned long long>(live / 1024), static_cast<unsigned long long>(m_FrameIndex));
}

void App::Update()
{
  GECKO_SCOPE_ALWAYS_NAMED(Main_Label, "frame");
  (void)gk::DispatchEvents();
  RenderFrame();
  GECKO_FRAME(Main_Label, "Frame");
}

int App::Run()
{
  if (!IsValid())
    return 1;

  GECKO_PUSH_LABEL(Main_Label);
  GECKO_SCOPE_ALWAYS_NAMED(Main_Label, "AppRun");

  GECKO_INFO(Main_Label, "Entering frame loop - Escape or close to quit");
  m_StartTime = ::std::chrono::steady_clock::now();

  gk::platform::SetModalFrameCallback([](void* ud) { static_cast<App*>(ud)->Update(); }, this);

  while (m_Running)
  {
    gk::platform::PumpEvents();
    Update();
  }

  gk::platform::SetModalFrameCallback(nullptr, nullptr);
  GECKO_INFO(Main_Label, "Shutdown complete");
  return 0;
}

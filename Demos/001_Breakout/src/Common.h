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
#include <gecko/math/matrix.h>
#include <gecko/platform/platform_module.h>
#include <gecko/runtime/runtime_module.h>
#include <gecko/runtime/standard_log_sinks.h>
#include <gecko/runtime/tracking_allocator.h>
#include <optional>
#include <gecko/math/math.h>

namespace gk = gecko;
namespace gm = gecko::math;
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

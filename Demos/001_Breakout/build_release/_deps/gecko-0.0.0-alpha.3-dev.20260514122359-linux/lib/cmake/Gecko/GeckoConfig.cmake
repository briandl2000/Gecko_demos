
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was GeckoConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include(CMakeFindDependencyMacro)

# Linux transitive deps. The exported Gecko::Platform / Gecko::Graphics
# targets reference these as imported targets (X11::*, PkgConfig::*,
# Vulkan::Vulkan) rather than absolute distro-specific library paths,
# so the same Linux .zip is consumable on Ubuntu, Arch, Fedora, etc.
if(UNIX AND NOT APPLE)
    # FindX11 in CMake does not accept a "X11" component; passing it makes
    # find_dependency fail with "missing: X11" on Arch. Use the bare form
    # which probes everything and exposes X11::X11 / X11::Xrandr targets.
    find_dependency(X11)

    find_dependency(PkgConfig)

    if(NOT TARGET PkgConfig::Wayland)
        pkg_check_modules(Wayland REQUIRED IMPORTED_TARGET wayland-client)
    endif()

    if(NOT TARGET PkgConfig::WaylandCursor)
        pkg_check_modules(WaylandCursor REQUIRED IMPORTED_TARGET wayland-cursor)
    endif()

    if(NOT TARGET PkgConfig::XkbCommon)
        pkg_check_modules(XkbCommon REQUIRED IMPORTED_TARGET xkbcommon)
    endif()

    # Graphics module Wayland surface uses a separate prefix.
    if(NOT TARGET PkgConfig::WAYLAND_CLIENT)
        pkg_check_modules(WAYLAND_CLIENT IMPORTED_TARGET wayland-client)
    endif()
endif()

# Vulkan is required transitively by Gecko::Graphics when the shipped
# package was built with Vulkan support. QUIET so consumers that only
# use Core/Math/Runtime/Platform don't fail if Vulkan is missing.
find_dependency(Vulkan QUIET)

include("${CMAKE_CURRENT_LIST_DIR}/GeckoTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/GeckoShaders.cmake")

check_required_components("Gecko")

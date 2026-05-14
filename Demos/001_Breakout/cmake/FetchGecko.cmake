# Downloads and extracts a tagged Gecko release, then calls find_package().
#
# Required input:
#   GECKO_VERSION  -- e.g. "0.0.0-alpha.3" or "0.0.0-alpha.3-dev.20260512183646"
#                     (without the leading "v")
#
# After this file is included, the following targets are available:
#   Gecko::Core, Gecko::CoreServices, Gecko::Math,
#   Gecko::Runtime, Gecko::Platform, Gecko::Graphics

if(NOT DEFINED GECKO_VERSION)
  message(FATAL_ERROR "FetchGecko: set GECKO_VERSION before including this file")
endif()

set(GECKO_TAG "v${GECKO_VERSION}")

if(WIN32)
  set(_gecko_platform "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(_gecko_platform "linux")
else()
  message(FATAL_ERROR "Gecko binary releases are only published for Windows and Linux.")
endif()

set(_gecko_pkg     "gecko-${GECKO_VERSION}-${_gecko_platform}.zip")
set(_gecko_url     "https://github.com/briandl2000/Gecko/releases/download/${GECKO_TAG}/${_gecko_pkg}")
set(_gecko_deps    "${CMAKE_BINARY_DIR}/_deps")
set(_gecko_zip     "${_gecko_deps}/${_gecko_pkg}")
set(_gecko_extract "${_gecko_deps}/gecko-${GECKO_VERSION}-${_gecko_platform}")

file(MAKE_DIRECTORY "${_gecko_deps}")

if(NOT EXISTS "${_gecko_zip}")
  message(STATUS "Downloading Gecko: ${_gecko_url}")
  file(DOWNLOAD "${_gecko_url}" "${_gecko_zip}"
        SHOW_PROGRESS
        TLS_VERIFY ON
        STATUS _gecko_dl_status)
  list(GET _gecko_dl_status 0 _gecko_dl_code)
  if(NOT _gecko_dl_code EQUAL 0)
    file(REMOVE "${_gecko_zip}")
    message(FATAL_ERROR "Failed to download Gecko: ${_gecko_dl_status}")
  endif()
endif()

if(NOT EXISTS "${_gecko_extract}/lib/cmake/Gecko/GeckoConfig.cmake")
  message(STATUS "Extracting Gecko to: ${_gecko_extract}")
  file(MAKE_DIRECTORY "${_gecko_extract}")
  file(ARCHIVE_EXTRACT INPUT "${_gecko_zip}" DESTINATION "${_gecko_extract}")
endif()

# The zip extracts to <extract>/gecko-<version>/<install tree>.
file(GLOB _gecko_config "${_gecko_extract}/*/lib/cmake/Gecko/GeckoConfig.cmake")
if(NOT _gecko_config)
  file(GLOB _gecko_config "${_gecko_extract}/lib/cmake/Gecko/GeckoConfig.cmake")
endif()
if(NOT _gecko_config)
  message(FATAL_ERROR "GeckoConfig.cmake not found under ${_gecko_extract}")
endif()
list(GET _gecko_config 0 _gecko_config)
get_filename_component(Gecko_DIR "${_gecko_config}" DIRECTORY)

# Put Gecko's bin/ on the runtime DLL search path so $<TARGET_RUNTIME_DLLS:...>
# can find GeckoCoreServices.dll / .so on Windows and Linux respectively.
get_filename_component(_gecko_root "${Gecko_DIR}/../../.." ABSOLUTE)
list(APPEND CMAKE_PREFIX_PATH "${_gecko_root}")

find_package(Gecko CONFIG REQUIRED)

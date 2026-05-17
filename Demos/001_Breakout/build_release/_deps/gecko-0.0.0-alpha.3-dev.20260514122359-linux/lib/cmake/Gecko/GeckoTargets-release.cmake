#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Gecko::Core" for configuration "Release"
set_property(TARGET Gecko::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Gecko::Core PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libGeckoCore.a"
  )

list(APPEND _cmake_import_check_targets Gecko::Core )
list(APPEND _cmake_import_check_files_for_Gecko::Core "${_IMPORT_PREFIX}/lib/libGeckoCore.a" )

# Import target "Gecko::CoreServices" for configuration "Release"
set_property(TARGET Gecko::CoreServices APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Gecko::CoreServices PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libGeckoCoreServices.so"
  IMPORTED_SONAME_RELEASE "libGeckoCoreServices.so"
  )

list(APPEND _cmake_import_check_targets Gecko::CoreServices )
list(APPEND _cmake_import_check_files_for_Gecko::CoreServices "${_IMPORT_PREFIX}/lib/libGeckoCoreServices.so" )

# Import target "Gecko::Platform" for configuration "Release"
set_property(TARGET Gecko::Platform APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Gecko::Platform PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libGeckoPlatform.a"
  )

list(APPEND _cmake_import_check_targets Gecko::Platform )
list(APPEND _cmake_import_check_files_for_Gecko::Platform "${_IMPORT_PREFIX}/lib/libGeckoPlatform.a" )

# Import target "Gecko::Runtime" for configuration "Release"
set_property(TARGET Gecko::Runtime APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Gecko::Runtime PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libGeckoRuntime.a"
  )

list(APPEND _cmake_import_check_targets Gecko::Runtime )
list(APPEND _cmake_import_check_files_for_Gecko::Runtime "${_IMPORT_PREFIX}/lib/libGeckoRuntime.a" )

# Import target "Gecko::Graphics" for configuration "Release"
set_property(TARGET Gecko::Graphics APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Gecko::Graphics PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libGeckoGraphics.a"
  )

list(APPEND _cmake_import_check_targets Gecko::Graphics )
list(APPEND _cmake_import_check_files_for_Gecko::Graphics "${_IMPORT_PREFIX}/lib/libGeckoGraphics.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

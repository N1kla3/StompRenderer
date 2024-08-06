include(CMakeDependentOption)
include(CheckCXXCompilerFlag)

macro(renderer_setup_options)

  if(NOT PROJECT_IS_TOP_LEVEL)
    option(renderer_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(renderer_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(renderer_ENABLE_PCH "Enable precompiled headers" OFF)
    option(renderer_ENABLE_CACHE "Enable ccache" OFF)
    option(renderer_ENABLE_TESTS "Enable renderer tests" ON)
  else()
    option(renderer_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(renderer_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(renderer_ENABLE_PCH "Enable precompiled headers" OFF)
    option(renderer_ENABLE_CACHE "Enable ccache" OFF)
    option(renderer_ENABLE_TESTS "Enable renderer tests" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      renderer_WARNINGS_AS_ERRORS
      renderer_ENABLE_UNITY_BUILD
      renderer_ENABLE_COVERAGE
      renderer_ENABLE_PCH
      renderer_ENABLE_CACHE)
  endif()

endmacro()

macro(renderer_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(CMake/StandardProjectSettings.cmake)
  endif()

  add_library(renderer_warnings INTERFACE)
  add_library(renderer_options INTERFACE)

  include(CMake/CompilerWarnings.cmake)
  renderer_set_project_warnings(
    renderer_warnings
    ${renderer_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  set_target_properties(renderer_options PROPERTIES UNITY_BUILD ${renderer_ENABLE_UNITY_BUILD})

  if(renderer_ENABLE_PCH)
    target_precompile_headers(
      renderer_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(renderer_ENABLE_CACHE)
    include(CMake/Cache.cmake)
    renderer_enable_cache()
  endif()

  if(renderer_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    renderer_enable_coverage(renderer_options)
  endif()

  if(renderer_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(renderer_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

endmacro()

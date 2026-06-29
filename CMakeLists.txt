cmake_minimum_required(VERSION 3.16)
project(OpenHABTestSuiteC VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# ── c-openhab-rest-client ─────────────────────────────────────────────────────
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/c-openhab-rest-client/CMakeLists.txt")
  add_subdirectory(extern/c-openhab-rest-client)
else()
  find_package(OpenHABRestClientC REQUIRED)
endif()

find_package(CURL REQUIRED)
find_package(Threads)

# ── Sources ───────────────────────────────────────────────────────────────────
set(LIB_SOURCES
  src/openhab_item_tester.c
  src/openhab_thing_tester.c
  src/openhab_rule_tester.c
  src/openhab_channel_tester.c
  src/openhab_persistence_tester.c
  src/openhab_sitemap_tester.c
)

# ── Static library ────────────────────────────────────────────────────────────
add_library(openhab_test_suite_static STATIC ${LIB_SOURCES})
set_target_properties(openhab_test_suite_static PROPERTIES OUTPUT_NAME openhab_test_suite)
target_compile_definitions(openhab_test_suite_static PUBLIC OPENHAB_STATIC)
target_include_directories(openhab_test_suite_static PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(openhab_test_suite_static PUBLIC
  openhab_rest_client_static ${CURL_LIBRARIES}
  $<$<BOOL:${Threads_FOUND}>:Threads::Threads>)

add_library(OpenHABTestSuiteC::Static ALIAS openhab_test_suite_static)

# ── Test executable ───────────────────────────────────────────────────────────
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/test.c")
  add_executable(openhab_suite_test test/test.c)
  target_link_libraries(openhab_suite_test PRIVATE openhab_test_suite_static)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS openhab_test_suite_static ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

message(STATUS "──────────────────────────────────────────")
message(STATUS " c-openhab-test-suite ${PROJECT_VERSION}")
message(STATUS "──────────────────────────────────────────")

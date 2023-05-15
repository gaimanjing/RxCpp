include(FetchContent)
message(STATUS "Downloading gtest...")
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
  play_test.cpp
)
target_include_directories(unit_tests PRIVATE
  ../Rx/v2/src
)

target_link_libraries(unit_tests PRIVATE gtest_main gmock)
include(GoogleTest)
gtest_discover_tests(unit_tests)
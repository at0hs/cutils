cmake_minimum_required(VERSION 3.18)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

project(C_Utilisties C CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/a7f443b80b105f940225332ed3c31f2790092f47.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
if (WIN32)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif ()
FetchContent_MakeAvailable(googletest)

#
# definition common variables
#
set(LIBRARY_NAME CUtils)
set(HOME ${PROJECT_SOURCE_DIR})
set(SOURCE_DIRECTROY ${HOME}/lib)
set(INCLUDE_DIRECTROY ${HOME}/include)

message(STATUS "Project home: " ${HOME})
message(STATUS "Source direcotry: " ${SOURCE_DIRECTROY})
message(STATUS "Inlcude directory: " ${INCLUDE_DIRECTROY})

# compile options
set(DEBUG_OPTIONS
	-O0
	-ggdb
	-g3
)
set(NODEBUG_OPTIONS
	-O2
)
set(WARNING_OPTIONS
	-Wall
	-Wextra
	-Wno-unused-variable
	-Wno-unused-function
	-Wno-unused-parameter
)

# 外部依存ライブラリ (pkg-config 経由で取得し、アーキテクチャ固有のパスを直書きしない)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
set(EXTERNAL_LIBRARY
	PkgConfig::GLIB
)

enable_testing()
include(GoogleTest)

add_subdirectory(lib)
add_subdirectory(tests)

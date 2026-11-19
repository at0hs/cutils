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

# 外部依存ライブラリ (POSIX スレッドのみ)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(EXTERNAL_LIBRARY
	Threads::Threads
)

enable_testing()
include(GoogleTest)

add_subdirectory(lib)
add_subdirectory(tests)

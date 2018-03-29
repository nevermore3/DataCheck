CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

project(autohdmap-datacheck C CXX)

set(CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX d)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(BuildType)

message("project[${PROJECT_NAME}] build type=${CMAKE_BUILD_TYPE}")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/installed"  CACHE PATH "default install path" FORCE )
endif()

message("project[${PROJECT_NAME}] install path=${CMAKE_INSTALL_PREFIX}")

#ignore the shape lib warning
if ( CMAKE_HOST_APPLE )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-tautological-compare")
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-tautological-compare")
endif ( CMAKE_HOST_APPLE )
#set msvc /utf-8
if(MSVC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} /utf-8")
endif(MSVC)


set(HDMAP_LIBS_DIR ${CMAKE_SOURCE_DIR}/../autohdmap-libs)
message("project[${PROJECT_NAME}] depends libs path=${HDMAP_LIBS_DIR}")


#link 目录不能省略
if(CMAKE_HOST_WIN32)
    link_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/win64/poco/lib)
    link_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/win64/opencv3/lib)
    link_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/win64/geos/lib)
    link_directories(${HDMAP_LIBS_DIR}/modules/api/lib/win64)
    link_directories(${HDMAP_LIBS_DIR}/modules/core/lib/win64)
    link_directories(${HDMAP_LIBS_DIR}/modules/server/lib/win64)
elseif(CMAKE_HOST_APPLE)
    link_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/darwin/poco/lib)
    link_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/darwin/opencv/lib)
    link_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/darwin/geos/lib)
    link_directories(${HDMAP_LIBS_DIR}/modules/api/lib/darwin)
    link_directories(${HDMAP_LIBS_DIR}/modules/core/lib/darwin)
    link_directories(${HDMAP_LIBS_DIR}/modules/server/lib/darwin)
elseif(CMAKE_HOST_UNIX)
    link_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/linux/poco/lib)
    link_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/linux/opencv3/lib)
    link_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/linux/geos/lib)
    link_directories(${HDMAP_LIBS_DIR}/modules/api/lib/linux)
    link_directories(${HDMAP_LIBS_DIR}/modules/core/lib/linux)
    link_directories(${HDMAP_LIBS_DIR}/modules/server/lib/linux)
endif()
#thirdparty include
include_directories(${HDMAP_LIBS_DIR}/thirdparty/include)
if(CMAKE_HOST_WIN32)
    include_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/win64/poco/include)
    include_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/win64/opencv3/include)
    include_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/win64/geos/include)
elseif(CMAKE_HOST_APPLE)
    include_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/darwin/poco/include)
    include_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/darwin/opencv/include)
    include_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/darwin/geos/include)
elseif(CMAKE_HOST_UNIX)
    include_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/linux/poco/include)
    include_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/linux/opencv3/include)
    include_directories(${HDMAP_LIBS_DIR}/thirdparty/platform/linux/geos/include)
endif()
#
add_subdirectory(${CMAKE_SOURCE_DIR}/modules)
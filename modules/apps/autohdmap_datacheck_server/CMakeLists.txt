#include and src file
file(GLOB_RECURSE autohdmap_datacheck_server_files_header *.h)
file(GLOB_RECURSE autohdmap_datacheck__server_files_cpp *.cpp)

#
add_executable(autohdmap_datacheck_server  ${autohdmap_datacheck_server_files_header} ${autohdmap_datacheck__server_files_cpp} )
target_link_libraries(autohdmap_datacheck_server autohdmap_checkcore)
IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    target_link_libraries(autohdmap_datacheck_server general autohdmap_serverd autohdmap_cored autohdmap_apid  pthread)
ELSE()
    target_link_libraries(autohdmap_datacheck_server general autohdmap_server autohdmap_core autohdmap_api  pthread)
ENDIF()

target_link_libraries(autohdmap_datacheck_server general PocoNet PocoUtil PocoXML PocoFoundation PocoJSON)
target_link_libraries(autohdmap_datacheck_server general geos)
#include self include
target_include_directories(autohdmap_datacheck_server PUBLIC include)
target_include_directories(autohdmap_datacheck_server PUBLIC ${HDMAP_LIBS_DIR}/modules/api/include)
target_include_directories(autohdmap_datacheck_server PUBLIC ${HDMAP_LIBS_DIR}/modules/core/include)
target_include_directories(autohdmap_datacheck_server PUBLIC ${HDMAP_LIBS_DIR}/modules/server/include)

if (CMAKE_HOST_WIN32)
    INSTALL(TARGETS autohdmap_datacheck_server DESTINATION lane/bin/win64)
elseif (CMAKE_HOST_APPLE)
    INSTALL(TARGETS autohdmap_datacheck_server DESTINATION lane/bin/darwin)
elseif (CMAKE_HOST_UNIX)
    INSTALL(TARGETS autohdmap_datacheck_server DESTINATION lane/bin/linux)
endif ()
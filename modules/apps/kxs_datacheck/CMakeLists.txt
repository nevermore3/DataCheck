#include and src file
file(GLOB_RECURSE check_app_files_header *.hpp *.h)
file(GLOB_RECURSE check_app_files_cpp *.cpp *.c)

#exe
add_executable(kxs_datacheck ${check_app_files_cpp})

#
target_link_libraries(kxs_datacheck autohdmap_checkcore)
target_link_libraries(kxs_datacheck general autohdmap_core autohdmap_api)
target_link_libraries(kxs_datacheck general PocoJSON PocoFoundation PocoNet PocoUtil pthread)
target_link_libraries(kxs_datacheck general geos glog cppsqlite3 nds_sqlite3 tinyxml)

#target_link_libraries(kxs_datacheck general PocoFoundation opencv_core opencv_imgproc opencv_imgcodecs)
#target_link_libraries(kxs_datacheck general opencv_features2d opencv_xfeatures2d)
#include self include
target_include_directories(kxs_datacheck PUBLIC include)
target_include_directories(kxs_datacheck PUBLIC ${HDMAP_LIBS_DIR}/modules/api/include)
target_include_directories(kxs_datacheck PUBLIC ${HDMAP_LIBS_DIR}/modules/core/include)
target_include_directories(kxs_datacheck PUBLIC ${HDMAP_LIBS_DIR}/modules/server/include)
#target_include_directories(kxs_datacheck PUBLIC ${HDMAP_LIBS_DIR}/modules/fusion/include)

if (CMAKE_HOST_WIN32)
    INSTALL(TARGETS kxs_datacheck DESTINATION download/bin/win64)
elseif (CMAKE_HOST_APPLE)
    INSTALL(TARGETS kxs_datacheck DESTINATION download/bin/darwin)
elseif (CMAKE_HOST_UNIX)
    INSTALL(TARGETS kxs_datacheck DESTINATION download/bin/linux)
endif ()

##
##include self include
#target_include_directories(autohdmap_fusion_app PUBLIC ${CMAKE_SOURCE_DIR}/modules/api/include)
#target_include_directories(autohdmap_fusion_app PUBLIC ${CMAKE_SOURCE_DIR}/modules/core/include)
#target_include_directories(autohdmap_fusion_app PUBLIC ${CMAKE_SOURCE_DIR}/modules/apps/track_merge)
#
##link lib
#target_link_libraries(autohdmap_fusion_app PUBLIC general autohdmap_api)
#target_link_libraries(autohdmap_fusion_app PUBLIC general autohdmap_core)
#target_link_libraries(autohdmap_fusion_app PUBLIC general track_merge)

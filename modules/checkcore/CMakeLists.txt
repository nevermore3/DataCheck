#include and src file
file(GLOB_RECURSE checkcore_files_header include/*.hpp include/*.h)
file(GLOB_RECURSE checkcore_files_cpp src/*.cpp src/*.c)
#static library
add_library(autohdmap_checkcore ${checkcore_files_header} ${checkcore_files_cpp})

#include self include
target_include_directories(autohdmap_checkcore PUBLIC include)
target_include_directories(autohdmap_checkcore PUBLIC ${HDMAP_LIBS_DIR}/modules/api/include)
target_include_directories(autohdmap_checkcore PUBLIC ${HDMAP_LIBS_DIR}/modules/core/include)
target_include_directories(autohdmap_checkcore PUBLIC ${HDMAP_LIBS_DIR}/modules/clothoid/include)

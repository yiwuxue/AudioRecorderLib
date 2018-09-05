
cmake_minimum_required(VERSION 3.4.1)

set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/main/cpp)

#指定头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR})


#加载编译的源文件
file (GLOB ALL_FILES "${PROJECT_SOURCE_DIR}/*.cpp" "${PROJECT_SOURCE_DIR}/audiorecord/*.cpp" "${PROJECT_SOURCE_DIR}/opensles/*.cpp" "${PROJECT_SOURCE_DIR}/base/*.cpp")
add_library( audiorecord
             SHARED
             ${ALL_FILES}
             )

#设置so目录,为在主app工程的libs位置，根据实际情况调整distribution_DIR的值
set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties( audiorecord
                       PROPERTIES
                       LIBRARY_OUTPUT_DIRECTORY
                      ${distribution_DIR}/libs/${ANDROID_ABI})

find_library( log-lib
              log )

find_library( OpenSLES-lib
                OpenSLES )

target_link_libraries( audiorecord
                       ${log-lib}
                       ${OpenSLES-lib}
                     )
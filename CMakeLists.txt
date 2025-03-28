cmake_minimum_required(VERSION 3.10)
project(Muoduo_CPP)

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#设置可执行文件最终存储的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)
include_directories(${PROJECT_SOURCE_DIR}/include/server/UDP)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)

# 添加 muduo 库的头文件搜索路径
include_directories(/usr/local/include)
# 添加 muduo 库的库文件搜索路径
link_directories(/usr/local/lib)


# 加载子目录
add_subdirectory(src)

#把.指定路径下的所有源文件名字放入变量名SRC_LIST里面
#aux_source_directory(. SRC_LIST)
# moduoServer可执行文件
# add_executable(Muoduo_CPP src/moduoServer.cpp)
# 表示Muoduo_CPP这个目标程序，链接 muduo 库
# target_link_libraries(Muoduo_CPP muduo_net muduo_base pthread)

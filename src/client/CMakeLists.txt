#定义了一个变量包含当前目录下所有的源文件
aux_source_directory(. SRC_LIST)

# 指定生成的可执行文件
add_executable(ChatClient ${SRC_LIST})
#指定可执行文件链接时需要以来的库文件
target_link_libraries(ChatClient pthread)

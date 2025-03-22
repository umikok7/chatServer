#定义了一个变量包含当前目录下所有的源文件
aux_source_directory(. SRC_LIST)
#当前目录的db目录下的源文件
aux_source_directory(./db DB_LIST)
#当前目录的model目录下的源文件
aux_source_directory(./model MODEL_LIST)
#当前目录的redis目录下的源文件
aux_source_directory(./redis REDIS_LIST)

# 指定生成的可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})
#指定可执行文件链接时需要以来的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)

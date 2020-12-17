cmake_minimum_required(VERSION 2.6)

project(cppnp)

set(SOURCES 
    src/perror.cc 
    src/epoll.cc 
    src/channel.cc 
    src/buffer.cc
    src/tcp_connection.cc 
    src/timestamp.cc
    src/timerqueue.cc
    src/eventloop.cc 
    src/acceptor.cc 
    src/tcp_server.cc 
    src/echo_sv.cc 
    src/main.cc 
)


add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include 
)

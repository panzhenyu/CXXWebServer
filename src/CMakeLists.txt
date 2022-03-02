# set source file
set(SRCS
    Epoll.cc
    Event.cc
    IOEventHandler.cc
    ListenEventHandler.cc
    main.cc
    Server.cc
    Worker.cc
)

# add the executable
add_executable(server ${SRCS})

# add libs for test
add_library(WebServerUtils ${SRCS})

# find and add system libs
# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)
target_link_libraries(server pthread)
target_link_libraries(server HttpUtils)
target_link_libraries(server Log)

# add sub dir
add_subdirectory(HttpUtils)
add_subdirectory(Log)
add_subdirectory(test)

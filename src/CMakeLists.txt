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

# add third-party include path
include_directories(ThirdParty)

# add libs for test
add_library(WebServerUtils ${SRCS})

# find and add system libs
# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)
target_link_libraries(server pthread HttpUtils Log Cache)
# add ThirdPartyLibs
target_link_libraries(server hiredis)

# add sub dir
add_subdirectory(HttpUtils)
add_subdirectory(Log)
add_subdirectory(test)
add_subdirectory(ThirdParty)
add_subdirectory(Cache)

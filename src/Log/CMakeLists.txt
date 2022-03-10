set(SRCS
    AsyncLogger.cc
    Logger.cc
    LogStream.cc
)

add_library(Log ${SRCS})
target_link_libraries(Log pthread)

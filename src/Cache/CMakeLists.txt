set(SRCS
    RedisCache.cc
)

add_library(Cache ${SRCS})
target_link_libraries(Cache pthread)

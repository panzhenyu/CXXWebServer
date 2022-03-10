add_executable(SocketStreamBufferTest SocketStreamBufferTest.cc)
target_link_libraries(SocketStreamBufferTest HttpUtils WebServerUtils)

add_executable(RouterTest RouterTest.cc)
target_link_libraries(RouterTest HttpUtils)

add_executable(HttpUtilTest HttpUtilTest.cc)
target_link_libraries(HttpUtilTest HttpUtils)

add_executable(RedisConnTest RedisConnTest.cc)
target_link_libraries(RedisConnTest Cache hiredis)

add_executable(RedisCacheTest RedisCacheTest.cc)
target_link_libraries(RedisCacheTest Cache hiredis)

add_executable(BufferTest BufferTest.cc)
target_link_libraries(BufferTest Log)

add_executable(LoggerTest LoggerTest.cc)
target_link_libraries(LoggerTest Log pthread)
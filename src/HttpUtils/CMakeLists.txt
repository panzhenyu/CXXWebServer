set(SRCS
    HttpConfig.cc
    SocketStream.cc
    HttpRequest.cc
    Resource.cc
    ResourceAccessor.cc
    Router.cc
    HttpResponse.cc
)

add_library(HttpUtils ${SRCS})

set(SRCS
    HttpConfig.cc
    SocketStream.cc
    HttpRequest.cc
    Resource.cc
    Router.cc
    HttpResponse.cc
)

add_library(HttpUtils ${SRCS})

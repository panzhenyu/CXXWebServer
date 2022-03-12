# Web Server

## 准备

    安装redis
    拷贝hiredis目录至src/ThirdParty

## 配置

由于没有完成配置文件的解析，因此目前的配置需要在代码里添加。

在src/Server.cc的defaultCfg变量中可以设置服务器相关的属性。

    修改defaultCfg.__redisConf以配置你的redis_server端口号、密码
    
    修改defaultCfg.__redisCacheEnable控制是否激活redis缓存功能

在src/HttpUtils/Router.cc的doLoad方法中配置路由信息。

    错误页路由配置：
    __errMapper[错误码] = std::make_shared<StaticResource>(错误页路径, "");

    其他静态资源路由配置：
    __uriMapper[URI] = std::make_shared<StaticResource>(静态资源路径, URI);

## 构建
    cd ${WebServer_ROOT_DIR}
    mkdir build && cd build
    cmake ..
    cd src/

## 启动
    启动redis_server
    cd ${WebServer_ROOT_DIR}/build/src
    ./server

## 前言
* 这是一个只能返回静态资源的Web服务器，能够根据HTTP请求中的URI回送对应的资源。

* 本项目主要由并发控制、HTTP协议工具、资源缓存、日志四模块构成。其中并发模型与日志模块的设计参考了linyacool的WebServer，详见：```https://github.com/linyacool/WebServer```

## 模块描述
### 并发控制

* 本模块主要由Server、Worker、Epoll、Event、EventHandler五个类构成并发模型，模块相关代码见src文件夹下所有.cc/.hpp文件。原想借鉴Reactor的思路，将IO与计算分离，但在设计与实现过程中没有想到较好的处理方法，最终IO与计算(Http请求读取与Http响应构造)都交由一类线程完成。

* Server的工作包含三部分，init、start、stop、clean。init负责初始化所有资源；start主要负责启动Listen与IO两类Worker以协助Server完成整个HTTP请求处理过程；stop负责暂停所有Worker；clean负责清理所有资源。

* Worker借助Epoll实现对一个或多个套接字事件的监听，并借助EventHandler函数响应每个发生的事件。

* Event类用于描述一个事件，包括当事人(文件描述符)，以及期望的事件类型等信息，Event是Woker、Epoll、EventHandler相互交互的元数据类型。

* Epoll类是线程安全的，它管理了它所关心的事件信息，并向上为Worker提供已响应的事件信息。其内部主要封装了epoll_create1、epoll_ctl、epoll_wait与Event对象管理单元(具体是file descriptor->Event的哈希表)。

* EventHandler是一类抽象类，用于描述不同Worker对事件的处理方法。

### Http协议工具

* 本模块为HTTP请求解析->HTTP响应回送过程提供所有与HTTP协议相关的工具类，代码见src/HttpUtils。

* HttpConfig负责提供与HTTP协议内容相关的宏及响应的宏-字符串转化函数，包括HTTP请求类型、HTTP响应状态码等。

* HttpRequest负责从客户端连接中读取Http请求，并将其封装为HttpRequest对象。

* HttpResponse负责根据HttpRequest对象构造HttpResponse对象，并提供将HttpResponse对象回送客户端的接口。

* SocketStream继承了stream_buf，并提供SocketInputStream与SocketOuputStream两类流，用以向上支持HttpRequest的读取与HttpResponse的写入工作。

* Router负责提供路由功能，用于将HttpRequest中的uri映射为一个具体的网络资源(Resource对象)。

* ResourceAccessor负责获取具体的网络资源，并给出资源的读取接口(是一个istream对象)。

* Resource是网络资源的描述，一个Resource对象代表一个具体的网络资源。Resource是HttpRequest、HttpResponse、Router、ResourceAccessor进行信息交互的媒介。

### Cache

* Cache模块负责为CachedResourceAccessor提供缓冲功能，代码见src/Cache。

* RedisCache为ICache的实现类，对外提供了资源访问(get)、资源缓存(put)接口。

* RedisConnection为Redis数据库连接类，用以提供对Redis数据库访问的接口(主要提供了get与set)。

* RedisConnectionPool为Redis数据库连接池，用以减小数据库连接的建立开销。RedisConnectionPoll向上为RedisCache提供了RedisConnection的获取方法。

### 日志

* 日志模块采用双缓冲队列方式实现异步写入，代码见src/Log。

* AsyncLogger为单例模式，负责日志的异步写入，其成员主要分为日志文件写入句柄、生产者缓冲区、消费者缓冲区、同步变量四类。生产者负责向生产者缓冲区中追加日志信息，当缓冲区满后将日志内容以追加形式移动到消费者缓冲区，并通过条件变量通知消费者(AsyncLogger线程)。消费者将消费者缓冲区的内容搬到一个本地缓冲区后，解除对消费者缓冲区的控制，并将本地缓冲区的内容写入日志文件。特别地，若经过timeout事件后仍然没有收到通知，消费者也会自动醒来，并将生产者缓冲区、消费者缓冲区的内容一并搬运到本地缓冲区后写入日志。

* Buffer为AsyncLogger缓冲日志信息的元数据结构。

* LogStream对外提供了类似流的方式用以将日志信息写入到Buffer。

* Logger是LogStream的封装，用于在Log开始与结束时打上对应的标记。

## 后记

> 其实还有很多想做的内容，但一方面能力不够，另一方面是时间与精力都比较有限，只能先将想法放在这里，以待后续了...

* 设计配置文件，在Server中根据配置文件初始化各类资源。

* 重新组织Router内部的实现方式，使其能够根据配置文件，将URI直接映射到一个资源目录中，并能根据预设的正则规则寻找对应的资源文件。

* 重新构造并发模型，将IO Worker的工作分离，一部分用于处理IO，一部分用于接收请求与构造响应。

* 将HttpResponse的响应构造过程从HttpResponsor中分离出来，交给Worker去处理，HttpResponsor专门负责序列化HttpResponse对象，将数据回送给客户端。

* 添加大文件传输的支持。目前的想法是若检测到HttpRequest请求的资源过大，则生成多个HttpResponse对象，但这要求HttpResponse类的body部分能够感知到偏移量与文件体长度(因为目前的设计是body部分用一个istream对象表示，而不是string)。或许不需要修改body部分：通过获取多个istream对象+预设文件指针+提前设置HttpResponse中的Content-Length字段告知Responsor每个HttpResponse应该从body的何处开始读取以及具体读取多少字节。

* 添加反向代理与CGI的支持。目前没有太好的想法，可能是让处理HTTP请求的Worker多负责监听一组与远程服务已经建立好连接的套接字池，每次发现资源被映射到对应的代理上时，根据负载均衡策略从从套接字池中取出一个套接字，建立远程服务端套接字-客户端套接字的映射，并将客户端请求转发后监听该套接字，并将该套接字回送的Http响应转发给客户端，最后归还远程服务端套接字。


struct Resource {
    enum Type { STATIC, CGI };
    using path_t = std::string;
    Type    __type;
    path_t  __path;
};

class ResourceAccessor {

};


struct Resource {
    using path_t    = std::string;
    using uri_t     = std::string;
    enum Type { STATIC, CGI };

    Type    __type;
    path_t  __path;
    uri_t   __uri;
};

// singleton
class ResourceAccessor {

};

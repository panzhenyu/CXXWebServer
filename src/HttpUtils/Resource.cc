#include "Resource.hpp"

GeneralResource::GeneralResource(ResourceType _type): __type(_type) {}

GeneralResource::ResourceType GeneralResource::getType() const { return __type; }

InvalidResource::InvalidResource(): GeneralResource(INVALID) {}

ValidResource::ValidResource(ResourceType _t, const path_t& _path, const uri_t& _uri): 
    GeneralResource(_t), __path(_path), __uri(_uri) {}

ValidResource::path_t ValidResource::getPath() const { return __path; }

ValidResource::uri_t ValidResource::getURI() const { return __uri; }

StaticResource::StaticResource(const path_t& _path, const uri_t& _uri): 
    ValidResource(STATIC, _path, _uri) {}

CGIResource::CGIResource(const path_t& _path, const uri_t& _uri): 
    ValidResource(CGI, _path, _uri) {}

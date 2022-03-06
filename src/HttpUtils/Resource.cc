#include "Resource.hpp"

GeneralResource::GeneralResource(ResourceType _type): __type(_type) {}

GeneralResource::ResourceType GeneralResource::getType() const { return __type; }

InvalidResource::InvalidResource(): GeneralResource(INVALID) {}

ValidResource::ValidResource(ResourceType _t, const std::string& _path, const std::string& _uri): 
    GeneralResource(_t), __path(_path), __uri(_uri) {}

std::string ValidResource::getPath() const { return __path; }

std::string ValidResource::getURI() const { return __uri; }

StaticResource::StaticResource(const std::string& _path, const std::string& _uri): 
    ValidResource(STATIC, _path, _uri) {}

CGIResource::CGIResource(const std::string& _path, const std::string& _uri): 
    ValidResource(CGI, _path, _uri) {}

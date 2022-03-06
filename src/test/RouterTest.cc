#include <mutex>
#include <string>
#include <iostream>
#include "../HttpUtils/Router.hpp"
using namespace std;

ostream& operator<<(ostream& _out, GeneralResource& _gs) {
    auto type = _gs.getType();
    _out << "Type: ";
    if (type == GeneralResource::INVALID) _out << "INVAILD";
    else {
        ValidResource& vs = static_cast<ValidResource&>(_gs);
        if (type == GeneralResource::STATIC) _out << "STATIC";
        else _out << "CGI";
        _out << " URI: " << vs.getURI() << " Path: " << vs.getPath(); 
    }
    return _out;   
}

int main() {
    Router& router = Router::getRouter();
    cout << *router[404] << endl;
    cout << *router["/hello"] << endl;
    cout << *router[505] << endl;
    return 0;
}

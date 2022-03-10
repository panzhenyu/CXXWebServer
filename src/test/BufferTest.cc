#include <iostream>
#include <algorithm>
#include "../Log/Buffer.hpp"

using namespace std;

int main() {
    Buffer<char> buff;
    string s("FDSFSF");
    while (1) {
        buff.append(s.begin(), s.end());
    }
    return 0;
}

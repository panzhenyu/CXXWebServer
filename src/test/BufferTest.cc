#include <iostream>
#include <algorithm>
#include "../Log/Buffer.hpp"

using namespace std;

struct Test {
    Test() { cout << "default" << endl; }
    Test(const Test&) { cout << "copy" << endl; }
    Test(Test&&) { cout << "move" << endl; }
};

int main() {

    return 0;
}

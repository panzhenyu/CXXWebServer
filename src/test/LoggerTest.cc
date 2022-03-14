#include <thread>
#include <chrono>
#include <string>
#include <iostream>
#include "../Log/Logger.hpp"

using namespace std;

void logThread() {
    string s(123, 'a');
    while (true) {
        LOG2DIARY << this_thread::get_id() << " " << s << endl;
        // this_thread::sleep_for(chrono::milliseconds(5000));
    }
}

int main() {
    AsyncLogger::getAsyncLogger().setLogFile("/dev/null");
    for (int i=0; i<8; ++i) {
        thread t(logThread);
        t.detach();
    }
    while (true) {
        this_thread::sleep_for(chrono::seconds(5));
        cout << AsyncLogger::getAsyncLogger().isRunning() << endl;
    }
    return 0;
}

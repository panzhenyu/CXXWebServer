#include <thread>
#include <chrono>
#include <string>
#include <iostream>
#include "../Log/Logger.hpp"

using namespace std;

void logThread() {
    while (true) {
        LOG2DIARY << this_thread::get_id() << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

int main() {
    int sleepCount = 4;
    for (int i=0; i<1; ++i) {
        thread t(logThread);
        t.detach();
    }
    while (true) {
        this_thread::sleep_for(chrono::seconds(1));
        if (--sleepCount == 0) AsyncLogger::getAsyncLogger().setLogFile(DEFAULT_LOG);
        cout << AsyncLogger::getAsyncLogger().isRunning() << endl;
    }
    return 0;
}

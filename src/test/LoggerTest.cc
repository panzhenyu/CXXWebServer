#include <thread>
#include <chrono>
#include <string>
#include <iostream>
#include "../Log/Logger.hpp"

using namespace std;

void logThread() {
    LOG2DIARY << this_thread::get_id();
}

int main() {
    for (int i=0; i<100; ++i) {
        thread t(logThread);
        t.detach();
    }
    while (true) {
        // LOG2DIARY << s;
        this_thread::sleep_for(chrono::seconds(1));
        cout << AsyncLogger::getAsyncLogger().isRunning() << endl;
    }
    
    return 0;
}
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

mutex x;
condition_variable to[3];
string out[3]{ "A", "B", "C" };

int main()
{
    auto f = [](int idx) {
        for (int i = 0; i < 3; ++i) {
            unique_lock<mutex> lock(x);
            to[idx].wait(lock);
            cout << this_thread::get_id() << "\tprint\t" << out[idx] << endl;
            lock.unlock();
            to[(idx + 1) % 3].notify_all();
        }
    };
    vector<thread> pool;
    for (int i = 0; i < 3; ++i) {
        pool.emplace_back(thread(f, i));
    }
    to[0].notify_all();
    for (int i = 0; i < 3; ++i) {
        pool[i].join();
    }
    return 0;
}

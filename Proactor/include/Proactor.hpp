#include <functional>
#include <map>
#include <mutex>
#include <thread>

using std::function;
using std::mutex;
using std::thread;

typedef  std::function<void*(int, std::mutex&)> proactorFunc;

class proactor {
private:
    std::mutex mtx;
    std::thread eventLoopThread;

public:
    void start_proactor(int listener, proactorFunc client_handler);
    void stop_proactor();
    ~proactor();
};
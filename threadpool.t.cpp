#include <threadpool.h>

#include <iostream>
#include <functional>
#include <thread>

void work(int t)
{
    std::cout << "Job " << t 
              << " working for " << t
              << " seconds... " << "\n";

    std::chrono::seconds secs(t);
    std::this_thread::sleep_for(secs);

    std::cout << "Job " << t << " done.\n";
}

int main(int argc, char * argv[])
{

    ThreadPool tp(4);
    tp.start();

    for (int i = 0; i < 10; ++i) {
        tp.enqueue(std::bind(work, i));
    }

    tp.drain();

}

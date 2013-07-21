#include <threadpool.h>

#include <iostream>

namespace {
    const int DEFAULT_NUM_THREADS = 4;

} // anon namespace

int ThreadPool::numThreads() const
{
    return d_numThreads;
}

int ThreadPool::availableThreads() const
{
    return d_availableThreads;
}

int ThreadPool::numTasks() const
{
    std::lock_guard<std::mutex> guard(d_mutex);
    return d_queue.size();
}

int ThreadPool::status() const
{
    return d_state;
}

void ThreadPool::startThread()
{
    while (d_state == STARTING ||
           d_state == ACTIVE ||
           d_state == DRAINING) {

        ++d_availableThreads;

        std::unique_lock<std::mutex> lock(d_mutex);

        while (d_queue.empty() && (d_state == STARTING ||
                                   d_state == ACTIVE)) {
            d_qNotEmpty.wait(lock);
        }

        if (d_state == STOPPING || d_state == STOPPED) {
            --d_availableThreads;
            lock.unlock();
            return;
        }

        if (d_queue.empty() && d_state == DRAINING) {
            --d_availableThreads;
            lock.unlock();
            return;
        }

        --d_availableThreads;
        Task t = d_queue.front();
        d_queue.pop_front();
        lock.unlock();

        t();
    }
}

bool ThreadPool::enqueue(const Task & task)
{
    if (d_state != ACTIVE) {
        return false;
    }

    std::lock_guard<std::mutex> guard(d_mutex);

    d_queue.push_back(task);
    d_qNotEmpty.notify_one();

    return true;
}

bool ThreadPool::start()
{
    if (d_state != STOPPED) {
        return false;
    }

    d_state = STARTING;

    for (int i = 0; i < d_numThreads; ++i) {
        d_threads.push_back(
               std::thread(&ThreadPool::startThread, this));
    }

    d_state = ACTIVE;

    return true;
}

void ThreadPool::stop()
{
    if (d_state != ACTIVE) {
        return;
    }

    std::cout << "Stopping threadpool.\n";

    d_state = STOPPING;
    d_qNotEmpty.notify_all();

    std::cout << "Waiting for threads to finish...\n";

    for (auto & th : d_threads) {
        th.join();
    }

    std::cout << "Threadpool stopped.\n";

    d_state = STOPPED;
}

void ThreadPool::drain()
{
    if (d_state != ACTIVE) {
        return;
    }

    std::cout << "Draining threadpool queue.\n";

    d_state = DRAINING;
    d_qNotEmpty.notify_all();

    for (auto & th : d_threads) {
        th.join();
    }

    std::cout << "Threadpool drained.\n";

    d_state = STOPPED;
}

ThreadPool::ThreadPool(int numThreads) :
    d_numThreads(numThreads),
    d_availableThreads(0),
    d_state(STOPPED)
{
}

ThreadPool::ThreadPool():
    d_numThreads(DEFAULT_NUM_THREADS),
    d_availableThreads(0),
    d_state(STOPPED)
{
}

ThreadPool::~ThreadPool()
{
    stop();
}


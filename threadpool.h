#ifndef INCLUDED_THREADPOOL_H
#define INCLUDED_THREADPOOL_H

#include <vector>
#include <deque>
#include <atomic>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>


class ThreadPool {


public:

    // Construct threadpool with the default number
    // of threads
    ThreadPool();

    // Construct threadpool with specified numThreads.
    // Results undefined if numThreads is <= 0.
    explicit ThreadPool(int numThreads);

    // DTOR calls the stop() method. If processing of
    // remaining tasks on the queue is required, drain()
    // must be called before the threadpool is destroyed.
    ~ThreadPool();

public:
    typedef std::function<void()> Task;
    enum Status {STOPPED  = 0,
                 STARTING = 1,
                 ACTIVE   = 2,
                 DRAINING = 3,
                 STOPPING = 4};

public:
    // Must be called before enqueueing any tasks. Returns
    // true if threadpool is successfully started, false otherwise.
    // If true is returned, the threadpool's state is ACTIVE.
    bool start();

    // Enqueues specified task onto the processing queue. This method
    // will fail unless the threadpool is in the ACTIVE state.
    bool enqueue(const Task & task);

    // Those threads that are still processing will be allowed to finish,
    // whereas any waiting threads are woken up and joined immediately.
    // Any items still on the queue will not be processed. Threadpool's
    // state after stop returns is STOPPED provided it was called
    // while the threadpool was in ACTIVE state.
    void stop();

    // Similar to stop(), but the remaining items on the queue will be
    // processed while any additional items will not be allowed to
    // be enqueued. Threadpool will be in the STOPPED state after drain()
    // returns provided it was called while the threadpool was in ACTIVE
    // state.
    void drain();

    // Returns the number of threads configured for this threadpool
    int  numThreads() const;

    // Returns the number of available threads for this threadpool
    int  availableThreads() const;

    // Returns the number of tasks currently in the queue
    int  numTasks() const;

    // Returns the current status of the threadpool
    int  status() const;

private:
    void startThread();

    // non-copyable, non-assignable
    ThreadPool(const ThreadPool & tp);
    ThreadPool & operator=(const ThreadPool & rhs);

private:
    mutable std::mutex       d_mutex;
    std::condition_variable  d_qNotEmpty;
    std::deque<Task>         d_queue;
    std::vector<std::thread> d_threads;
    int                      d_numThreads;
    std::atomic<int>         d_availableThreads;
    std::atomic<int>         d_state;
};


#endif

#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
private:
    std::vector<std::thread> threads;        // vector of threads
    std::queue<std::function<void()>> tasks; // queue of void-function objects to assing task to

    std::condition_variable
        cond; // condition variable to tell threads that queue is reachable to take task from
    std::mutex qMutex; // to block acces to the queue for more than one thread simultaneously
    bool stop;

public:
    explicit ThreadPool(int numThr); // create Thread Pool
    ~ThreadPool();
    void enqueue(std::function<void()> task);
    /*template<class F>
    void enqueue(F &&task) // perfect-forward function to enqueue tasks
    {
        {
            std::unique_lock<std::mutex> lock(qMutex);
            tasks.emplace(std::forward<F>(task));
        }
        cond.notify_one();
    }*/
    void stopped();
};

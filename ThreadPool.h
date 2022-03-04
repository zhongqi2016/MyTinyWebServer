//
// Created by zhongqi on 20.02.2022.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <cstdio>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
    explicit ThreadPool(int _numThreads = 7);

    template<class T>
    void append(T &&task){
        std::lock_guard<std::mutex> locker(pool->mtx);
        pool->workQueue.emplace(std::forward<T>(task));

        //随机唤醒一个线程
        pool->cond.notify_one();
    }

    ~ThreadPool() {
        if (pool) {
            std::lock_guard<std::mutex> locker(pool->mtx);
            pool->isClosed = true;
        }
        pool->cond.notify_all();
    }

private:
    struct Pool {
        std::mutex mtx;
        bool isClosed;
        std::queue<std::function<void()>> workQueue;
        std::condition_variable cond;
    };

    static void worker(const std::shared_ptr<Pool>& pool);

private:
    int threadNumber;
    //线程池数组
    std::shared_ptr<Pool> pool;

};


#endif //WEBSERVER_THREADPOOL_H

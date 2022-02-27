//
// Created by zhongqi on 20.02.2022.
//

#include "ThreadPool.h"

ThreadPool::ThreadPool(int _numThreads) {
    pool = std::make_shared<Pool>();
    threadNumber = _numThreads;
    for (int i = 0; i < threadNumber; ++i) {
        std::thread(&ThreadPool::worker, pool).detach();
    }
}

void ThreadPool::worker(const std::shared_ptr<Pool>& pool) {
    std::unique_lock<std::mutex> locker(pool->mtx);
    while (true) {
        if (!pool->workQueue.empty()) {
            auto task = std::move(pool->workQueue.front());
            pool->workQueue.pop();
            locker.unlock();
            task();
            locker.lock();
        } else if (pool->isClosed) break;
        else {
            pool->cond.wait(locker);
        }
    }
}

//template<class T>
//inline void ThreadPool::append(T &&task) {
//    std::lock_guard<std::mutex> locker(pool->mtx);
//    pool->workQueue.emplace(std::forward<T>(task));
//
//    //随机唤醒一个线程
//    pool->cond.notify_one();
//}
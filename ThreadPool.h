//
// Created by zhongqi on 20.02.2022.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <list>
#include <pthread.h>
#include <cstdio>



class ThreadPool {
public:


    ThreadPool(int _numThreads=8, int _maxRequest=1000);

    template<class T>
    bool append(T* request);
private:
    int threadNumber;
    int maxRequest;

    //线程池数组
    pthread_t *threads;

    std::list<T *> workQueue;
};


#endif //WEBSERVER_THREADPOOL_H

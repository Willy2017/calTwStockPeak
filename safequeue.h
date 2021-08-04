// -*- C++ -*-

//=============================================================================
/**
 * @file safequeue.h
 *
 * @author Bofu Huang
 *
 * @date 2021-04-21
 */
//=============================================================================

#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

#include "_DECL_NAMESPACE_BEGIN.h"

template <class T>
class SafeQueue
{
public:
    SafeQueue() {}

    ~SafeQueue() {}

    void enqueue(T t)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.push(t);
        mCV.notify_one();
    }

    T dequeue(void)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        while (mQueue.empty())
        {
            mCV.wait(lock);
        }
        T val = mQueue.front();
        mQueue.pop();
        return val;
    }

private:
    std::condition_variable mCV;
    std::queue<T> mQueue;
    mutable std::mutex mMutex;
};

#include "_DECL_NAMESPACE_END.h"
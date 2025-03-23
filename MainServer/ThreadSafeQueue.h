#pragma once

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue()
        : mSRWLock()
    {
        InitializeSRWLock(&mSRWLock);
    }

    void Push(T value) 
    {
        AcquireSRWLockExclusive(&mSRWLock);
        queue.push(value);
        ReleaseSRWLockExclusive(&mSRWLock);
    }

    T Pop() 
    {
        AcquireSRWLockExclusive(&mSRWLock);

        if (queue.empty() == true)
        {
            ReleaseSRWLockExclusive(&mSRWLock);
            return nullptr;
        }

        T value = queue.front();
        queue.pop();
        ReleaseSRWLockExclusive(&mSRWLock);
        return value;
    }

    bool IsQueueEmpty() 
    {
        bool bisQueueEmpty = false;

        AcquireSRWLockExclusive(&mSRWLock);
        bisQueueEmpty = queue.empty();
        ReleaseSRWLockExclusive(&mSRWLock);
        
        return bisQueueEmpty;
    }

    uint32_t size() const
    {
        return queue.size();
    }

private:
    std::queue<T> queue;
    SRWLOCK mSRWLock;
};
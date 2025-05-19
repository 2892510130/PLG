#pragma once

#include <thread>
#include <atomic>
#include <iostream>

#include "thread_safe_container.h"

template<typename ClassType, typename QueType>
class ActorSingle {
public:
    static ClassType& Inst() {
        static ClassType as;
        return as;
    }

    ~ ActorSingle() {}

    void PostMsg(const QueType& data) {
        _que.push(data);
    }

protected:
    
    ActorSingle() : _bstop(false) {}

    ActorSingle(const ActorSingle&) = delete;
    ActorSingle(ActorSingle&&) = delete;
    ActorSingle& operator = (const ActorSingle&) = delete;

    std::atomic<bool> _bstop;
    ThreadSafeQueueHeadTail<QueType>  _que;
    std::thread _thread;
};
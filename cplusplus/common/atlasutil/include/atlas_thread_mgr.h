#pragma once
#include <iostream>
#include <memory>
#include <thread>
#include <unistd.h>
#include "atlas_utils.h"
#include "thread_safe_queue.h"
#include "atlas_thread.h"

enum AtlasThreadStatus {    
    THREAD_READY = 0,
    THREAD_RUNNING = 1,
    THREAD_EXITING = 2,
    THREAD_EXITED = 3,
    THREAD_ERROR = 4,
};

class AtlasThreadMgr{
public:
    // 构造函数
    AtlasThreadMgr(AtlasThread* userThreadInstance, 
                   const std::string& threadName);
    ~AtlasThreadMgr();
    // 线程函数
    static void ThreadEntry(void* data);

    AtlasThread* GetUserInstance() { return this->userInstance_; }
    const std::string& GetThreadName(){ return name_; }
    // 将 AtlasMessage 数据发送到队列中
    AtlasError PushMsgToQueue(shared_ptr<AtlasMessage>& pMessage);
    // 从队列中将 AtlasMessage 数据取出
    shared_ptr<AtlasMessage> PopMsgFromQueue(){ return this->msgQueue_.Pop(); }
    void CreateThread();
    void SetStatus(AtlasThreadStatus status) { status_ = status; }
    AtlasThreadStatus GetStatus() { return status_; }
    AtlasError WaitThreadInitEnd();
 
public:
    std::string name_;
    AtlasThread* userInstance_;

    bool isExit_;
    AtlasThreadStatus status_;
    ThreadSafeQueue<shared_ptr<AtlasMessage>> msgQueue_;
};




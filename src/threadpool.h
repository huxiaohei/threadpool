/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/11/23
 * Copyright (c) 2023 虎小黑
 ****************************************************************/

#ifndef __THREAD_THREADPOOL_H__
#define __THREAD_THREADPOOL_H__

#include "utils.h"
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <vector>

static std::atomic<size_t> s_task_id{0};

class ThreadPool
{

  protected:
    struct TaskFunc
    {
        typedef std::shared_ptr<TaskFunc> ptr;

        TaskFunc(uint64_t expire_time) : m_expire_time(expire_time), m_id(s_task_id++), m_func(nullptr){};
        uint64_t m_expire_time;
        int m_id;
        std::function<void()> m_func;
    };

  protected:
    void run();

  public:
    typedef std::shared_ptr<ThreadPool> ptr;

    ThreadPool(size_t thread_num = 2);
    virtual ~ThreadPool();

    bool start();
    void stop();
    bool wait_for_all_done(uint64_t timeout_ms = 1000);

    template <typename F, typename... Args> auto exec(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        return exec(0, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <typename F, typename... Args>
    auto exec(uint64_t expire_time_ms, F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        expire_time_ms = expire_time_ms == 0 ? 0 : TimeUtils::get_current_time_milliseconds() + expire_time_ms;
        auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        auto task_func = std::make_shared<TaskFunc>(expire_time_ms);
        task_func->m_func = [task]() { (*task)(); };
        std::unique_lock<std::mutex> lock(m_mutex);
        m_tasks.emplace(task_func);
        m_cond.notify_one();
        return task->get_future();
    }

  public:
    int get_thread_num()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_threads.size();
    }

    int get_task_num()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_tasks.size();
    }

    bool is_exit() const
    {
        return m_exit;
    }

  private:
    std::mutex m_mutex;
    std::condition_variable m_cond;

  private:
    size_t m_thread_num;
    bool m_exit;
    std::atomic<int> m_runing_task;
    std::queue<TaskFunc::ptr> m_tasks;
    std::vector<decltype(std::make_shared<std::thread>())> m_threads;
};

#endif //__THREAD_THREADPOOL_H__
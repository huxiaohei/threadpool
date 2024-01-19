/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/11/23
 * Copyright (c) 2023 虎小黑
 ****************************************************************/

#include "threadpool.h"
#include <iostream>

ThreadPool::ThreadPool(size_t thread_num) : m_thread_num(thread_num), m_exit(false), m_runing_task(0)
{
}

ThreadPool::~ThreadPool()
{
    stop();
}

bool ThreadPool::wait_for_all_done(uint64_t timeout_ms)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_tasks.empty() && m_runing_task == 0)
    {
        return true;
    }
    if (timeout_ms <= 0)
    {
        m_cond.wait(lock, [this] { return m_tasks.empty() && m_runing_task == 0; });
        return true;
    }
    else
    {
        return m_cond.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                               [this] { return m_tasks.empty() && m_runing_task == 0; });
    }
}

bool ThreadPool::start()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_threads.empty())
    {
        return false;
    }
    for (size_t i = 0; i < m_thread_num; ++i)
    {
        m_threads.emplace_back(std::make_shared<std::thread>(&ThreadPool::run, this));
    }
    return true;
}

void ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_exit = true;
        m_cond.notify_all();
    }
    for (auto &thread : m_threads)
    {
        if (thread->joinable())
        {
            thread->join();
        }
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    m_threads.clear();
}

void ThreadPool::run()
{
    while (!m_exit)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        try
        {
            if (!m_tasks.empty())
            {
                m_runing_task++;
                auto task = m_tasks.front();
                m_tasks.pop();
                lock.unlock();
                if (task->m_expire_time == 0 || task->m_expire_time > TimeUtils::get_current_time_milliseconds())
                {
                    task->m_func();
                }
                m_runing_task--;
                continue;
            }
            if (m_exit)
            {
                break;
            }
            m_cond.wait(lock);
        }
        catch (const std::exception &e)
        {
            m_runing_task--;
            std::cout << e.what() << std::endl;
        }
    }
}

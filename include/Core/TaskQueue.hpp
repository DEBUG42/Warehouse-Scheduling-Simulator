#pragma once
#include <queue>
#include <vector>
#include "Task.hpp"

// 任务队列类
class TaskQueue
{
private:
    std::queue<Task> m_tasks;

public:
    // 添加任务到队列
    void enqueue(const Task &task)
    {
        m_tasks.push(task);
    }

    // 获取并移除队首任务
    Task dequeue()
    {
        Task task = m_tasks.front();
        m_tasks.pop();
        return task;
    }

    // 查看队首任务（不移除）
    const Task *peek() const
    {
        return m_tasks.empty() ? nullptr : &m_tasks.front();
    }

    // 是否为空
    bool isEmpty() const
    {
        return m_tasks.empty();
    }

    // 获取任务数量
    size_t size() const
    {
        return m_tasks.size();
    }

    // 获取所有任务的副本（用于UI显示）
    std::vector<Task> getAllTasks() const
    {
        std::vector<Task> result;

        // 复制队列中的所有任务到向量
        std::queue<Task> tempQueue = m_tasks;
        while (!tempQueue.empty())
        {
            result.push_back(tempQueue.front());
            tempQueue.pop();
        }

        return result;
    }
};

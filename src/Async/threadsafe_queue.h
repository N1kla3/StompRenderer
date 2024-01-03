#pragma once
#include <memory>
#include <mutex>

namespace omp
{
    template< class T >
    class ThreadSafeQueue
    {
    private:
        struct Node
        {
            std::shared_ptr<T> data;
            std::unique_ptr<Node> next;
        };
        std::mutex m_HeadMutex;
        std::mutex m_TailMutex;

        std::unique_ptr<Node> m_Head;

        Node *m_Tail;
    public:
        ThreadSafeQueue()
                : m_Head(std::make_unique<Node>()), m_Tail(m_Head.get())
        {
        }
        ThreadSafeQueue(const ThreadSafeQueue &other) = delete;
        ThreadSafeQueue &operator=(const ThreadSafeQueue &other) = delete;

    private:
        Node* getTail()
        {
            std::lock_guard<std::mutex> tail_lock(m_TailMutex);
            return m_Tail;
        }
        std::unique_ptr<Node> popHead()
        {
            std::unique_ptr<Node> old_head = std::move(m_Head);
            m_Head = std::move(old_head->next);
            return old_head;
        }

        std::unique_ptr<Node> tryPopHead()
        {
            std::lock_guard<std::mutex> head_lock(m_HeadMutex);
            if (m_Head.get() == getTail())
            {
                return std::unique_ptr<Node>();
            }
            return popHead();
        }
        std::unique_ptr<Node> tryPopHead(T& value)
        {
            std::lock_guard<std::mutex> head_lock(m_HeadMutex);
            if (m_Head.get() == getTail())
            {
                return std::unique_ptr<Node>();
            }
            value = std::move(*m_Head->data);
            return popHead();
        }

    public:
        void push(T newValue);

        std::shared_ptr<T> tryPop()
        {
            std::unique_ptr<Node> old_head = tryPopHead();
            return old_head ? old_head->data : std::shared_ptr<T>();
        }
        bool tryPop(T& value)
        {
            std::unique_ptr<Node> const old_head = tryPopHead(value);
            return static_cast<bool>(old_head);
        }
        bool empty()
        {
            std::lock_guard<std::mutex> head_lock(m_HeadMutex);
            return (m_Head.get() == getTail());
        }
    };

    template< class T >
    void ThreadSafeQueue<T>::push(T newValue)
    {
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(newValue)));
        std::unique_ptr<Node> new_node(std::make_unique<Node>());
        {
            std::lock_guard<std::mutex> tail_lock(m_TailMutex);
            m_Tail->data = new_data;
            Node* const new_tail = new_node.get();
            m_Tail->next = std::move(new_node);
            m_Tail = new_tail;
        }
    }
}
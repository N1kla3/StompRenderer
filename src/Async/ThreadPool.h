#pragma once
#include <thread>
#include <vector>
#include <functional>
#include <deque>
#include <future>
#include "threadsafe_queue.h"

namespace omp
{
    class FunctionWrapper
    {
    private:
        struct ImplBase
        {
            virtual void Call() = 0;
            virtual ~ImplBase() = default;
        };
        std::unique_ptr<ImplBase> m_Callable;

        template< class Functor >
        struct Impl : ImplBase
        {
            Functor functor;
            explicit Impl(Functor&& inF) : functor(std::move(inF)) {}

            Impl() = delete;
            Impl(Impl&& other) = delete;
            Impl& operator=(Impl&& other) = delete;
            Impl(const Impl& other) = delete;
            Impl& operator=(const Impl& other) = delete;

            virtual void Call() override
            {
                std::invoke(functor);
            }
        };

    public:
        template< class F >
        FunctionWrapper(F&& f)
                : m_Callable(std::make_unique<Impl<F>>(std::move(f)))
        {}
        FunctionWrapper() = default;
        FunctionWrapper(FunctionWrapper&& other)
                : m_Callable(std::move(other.m_Callable))
        {

        }
        FunctionWrapper& operator=(FunctionWrapper&& other)
        {
            m_Callable = std::move(other.m_Callable);
            return *this;
        }

        void operator()()
        {
            m_Callable->Call();
        }

        // deleted
        FunctionWrapper(const FunctionWrapper&) = delete;
        FunctionWrapper(FunctionWrapper&) = delete;
        FunctionWrapper& operator=(const FunctionWrapper&) = delete;
    };

    class WorkStealingQueue
    {
    private:
        using DataType = FunctionWrapper;
        std::deque<DataType> m_Queue;
        mutable std::mutex m_Mutex;
    public:
        WorkStealingQueue() = default;
        WorkStealingQueue(const WorkStealingQueue&) = delete;
        WorkStealingQueue& operator=(const WorkStealingQueue&) = delete;

        void push(DataType data)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Queue.push_front(std::move(data));
        }

        bool empty()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Queue.empty();
        }

        bool tryPop(DataType& data)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_Queue.empty())
            {
                return false;
            }
            data = std::move(m_Queue.front());
            m_Queue.pop_front();
            return true;
        }

        bool trySteal(DataType& outRes)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_Queue.empty())
            {
                return false;
            }
            outRes = std::move(m_Queue.back());
            m_Queue.pop_back();
            return true;
        }
    };

    class JoinThreads
    {
        std::vector<std::thread>& m_Threads;

    public:
        explicit JoinThreads(std::vector<std::thread>& inThreads)
                : m_Threads(inThreads)
        {}
        ~JoinThreads()
        {
            for(size_t index = 0; index < m_Threads.size(); index++)
            {
                if(m_Threads[index].joinable())
                    m_Threads[index].join();
            }
        }
    };

    void InterruptionPoint();

    class InterruptFlag
    {
    public:
        InterruptFlag()
                : m_ThreadCond(nullptr)
                , m_ThreadCondAny(nullptr)
        {}
        void set()
        {
            m_Flag.store(true, std::memory_order_relaxed);
            std::lock_guard<std::mutex> lock(m_SetClearMutex);
            if (m_ThreadCond)
            {
                m_ThreadCond->notify_all();
            }
            else if (m_ThreadCondAny)
            {
                m_ThreadCondAny->notify_all();
            }
        }
        bool isSet() const
        {
            return m_Flag.load(std::memory_order_relaxed);
        }

        template< typename Lockable >
        void wait(std::condition_variable_any& conditionVar, Lockable& lockable)
        {
            struct CustomLock
            {
                InterruptFlag* self;
                Lockable& m_lock;

                CustomLock(InterruptFlag* inSelf, std::condition_variable_any& cond, Lockable& inLock)
                        : self(inSelf)
                        , m_lock(inLock)
                {
                    self->m_SetClearMutex.lock();
                    self->m_ThreadCondAny = &cond;
                }

                void Unlock()
                {
                    m_lock.unlock();
                    self->m_SetClearMutex.unlock();
                }

                void Lock()
                {
                    std::lock(self->m_SetClearMutex, m_lock);
                }

                ~CustomLock()
                {
                    self->m_ThreadCondAny = nullptr;
                    self->m_SetClearMutex.unlock();
                }
            };

            CustomLock custom(this, conditionVar, lockable);
            InterruptionPoint();
            conditionVar.wait(custom);
            InterruptionPoint();
        }

    private:
        std::atomic<bool> m_Flag;
        std::condition_variable* m_ThreadCond;
        std::condition_variable_any* m_ThreadCondAny;
        std::mutex m_SetClearMutex;
    };

    inline thread_local InterruptFlag g_ThisThreadInterruptFlag;

    class ThreadInterruptedException final : public std::exception
    {
        virtual const char* what() const throw()
        {
            return "Thread interrupted";
        }
    };

    template< typename Lockable >
    void InterruptibleWait(std::condition_variable_any& cond, Lockable& lockable)
    {
        g_ThisThreadInterruptFlag.wait(cond, lockable);
    }

    class InterruptibleThread
    {
    public:
        template< typename FunctionType >
        InterruptibleThread(FunctionType &&function)
        {
            std::promise<InterruptFlag*> interrupt_flag_promise;
            // Packaged task issue, because it does not have non-const call
            m_InternalThread = std::thread([func = std::forward<FunctionType>(function), &interrupt_flag_promise]() mutable {
                interrupt_flag_promise.set_value(&g_ThisThreadInterruptFlag);
                func();
            });
            m_Flag = interrupt_flag_promise.get_future().get();
        }
        void interrupt()
        {
            if (m_Flag)
            {
                m_Flag->set();
            }
        }
        void join()
        {
            m_InternalThread.join();
        }
        bool joinable() const noexcept
        {
            return m_InternalThread.joinable();
        }
        void detach()
        {
            m_InternalThread.detach();
        }
    private:
        std::thread m_InternalThread;
        InterruptFlag* m_Flag;
    };

    class ThreadPool {
    private:
        // Data //
        // ==== //
        using TaskType = FunctionWrapper;

        std::atomic_bool m_Done;
        omp::ThreadSafeQueue<TaskType> m_PoolWorkQueue;
        std::vector<std::unique_ptr<WorkStealingQueue>> m_Queues;

        inline static thread_local WorkStealingQueue* s_LocalQueue;
        inline static thread_local size_t s_Index;

        std::vector<std::thread> m_Threads;

        JoinThreads m_Joiner;

        // Function //
        // ======== //
        void workerThread(size_t inIndex)
        {
            s_Index = inIndex;
            s_LocalQueue = m_Queues[s_Index].get();
            while (!m_Done)
            {
                runPendingTask();
            }
        }

        bool popTaskFromLocalQueue(TaskType& task)
        {
            return s_LocalQueue && s_LocalQueue->tryPop(task);
        }

        bool popTaskFromPoolQueue(TaskType& task)
        {
            return m_PoolWorkQueue.tryPop(task);
        }

        bool popTaskFromOtherThread(TaskType& task)
        {
            for (size_t index = 0; index < m_Queues.size(); index++)
            {
                const size_t i = (s_Index + index + 1) % m_Queues.size();
                if (m_Queues[i]->trySteal(task))
                {
                    return true;
                }
            }
            return false;
        }

    public:
        ThreadPool()
                : m_Done{ false }
                , m_Joiner(m_Threads)
        {
            unsigned const thread_count = std::thread::hardware_concurrency();
            try
            {
                for (size_t i = 0; i < thread_count; i++)
                {
                    m_Queues.push_back(std::unique_ptr<WorkStealingQueue>(std::make_unique<WorkStealingQueue>()));
                }
                for (size_t i = 0; i < thread_count; i++)
                {
                    m_Threads.push_back(std::thread(&ThreadPool::workerThread, this, i));
                }
            }
            catch (...)
            {
                m_Done = true;
            }
        }
        ~ThreadPool()
        {
            m_Done = true;
        }

        template< typename FunctionType, typename ...Args >
        std::future<typename std::invoke_result_t<FunctionType, Args...>> submit(FunctionType&& f, Args&&... args)
        {
            using ResultType = std::invoke_result_t<FunctionType, Args...>;
            std::packaged_task < ResultType() > task(std::bind(std::forward<FunctionType>(f), std::forward<Args>(args)...));
            std::future<ResultType> result(task.get_future());
            if (s_LocalQueue)
            {
                s_LocalQueue->push(std::move(task));
            }
            else
            {
                m_PoolWorkQueue.push(std::move(task));
            }
            return result;
        }

        void runPendingTask()
        {
            TaskType task;
            if (popTaskFromLocalQueue(task)
                || popTaskFromPoolQueue(task)
                || popTaskFromOtherThread(task))
            {
                task();
            }
            else
            {
                std::this_thread::yield();
            }
        }
    };
}

#pragma once
#include <thread>
#include <vector>
#include <functional>
#include <deque>
#include <future>
#include "threadsafe_queue.h"

class function_wrapper
{
private:
    struct impl_base
    {
        virtual void call() = 0;
        virtual ~impl_base() = default;
    };
    std::unique_ptr<impl_base> callable;

    template< class F >
    struct implemented : impl_base
    {
        F f;
        implemented() = delete;
        explicit implemented(F&& inF) : f(std::move(inF))
        {

        }
        virtual void call() override
        {
            std::invoke(f);
        }
    };

public:
    template< class F >
    function_wrapper(F&& f)
        : callable(std::make_unique<implemented<F>>(std::move(f)))
    {}
    void operator()()
    {
        callable->call();
    }
    function_wrapper() = default;
    function_wrapper(function_wrapper&& other)
        : callable(std::move(other.callable))
    {

    }
    function_wrapper& operator=(function_wrapper&& other)
    {
        callable = std::move(other.callable);
        return *this;
    }

    // deleted
    function_wrapper(const function_wrapper&) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&) = delete;
};

class ThreadJoin
{
private:
    std::vector<std::thread> m_Threads;
public:
    explicit ThreadJoin(const std::vector<std::thread>& inThreads)
    {
        //m_Threads = inThreads;
    }
    ~ThreadJoin()
    {
        for (auto& thr : m_Threads)
        {
            thr.join();
        }
    }
};

class work_stealing_queue
{
private:
    using data_type = function_wrapper;
    std::deque<data_type> m_Queue;
    mutable std::mutex m_Mutex;
public:
    work_stealing_queue() = default;
    work_stealing_queue(const work_stealing_queue&) = delete;
    work_stealing_queue& operator=(const work_stealing_queue&) = delete;

    void push(data_type data)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Queue.push_front(std::move(data));
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Queue.empty();
    }

    bool try_pop(data_type& data)
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

    bool try_steal(data_type& res)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_Queue.empty())
        {
            return false;
        }
        res = std::move(m_Queue.back());
        m_Queue.pop_back();
        return true;
    }
};

class join_threads
{
    std::vector<std::thread>& m_Threads;

public:
    explicit join_threads(std::vector<std::thread>& inThreads)
        : m_Threads(inThreads)
    {}
    ~join_threads()
    {
        for(size_t index = 0; index < m_Threads.size(); index++)
        {
            if(m_Threads[index].joinable())
                m_Threads[index].join();
        }
    }
};

void interruption_point();

class interrupt_flag
{
public:
    interrupt_flag()
        : thread_cond(nullptr)
        , thread_cond_any(nullptr)
    {}
    void set()
    {
        flag.store(true, std::memory_order_relaxed);
        std::lock_guard<std::mutex> lock(set_clear_mutex);
        if (thread_cond)
        {
            thread_cond->notify_all();
        }
        else if (thread_cond_any)
        {
            thread_cond_any->notify_all();
        }
    }
    bool is_set() const
    {
        return flag.load(std::memory_order_relaxed);
    }

    template< typename Lockable >
    void wait(std::condition_variable_any& condition_var, Lockable& lockable)
    {
        struct custom_lock
        {
            interrupt_flag* self;
            Lockable& m_lock;

            custom_lock(interrupt_flag* inSelf, std::condition_variable_any& cond, Lockable& inLock)
                : self(inSelf)
                , m_lock(inLock)
            {
                self->set_clear_mutex.lock();
                self->thread_cond_any = &cond;
            }

            void unlock()
            {
                m_lock.unlock();
                self->set_clear_mutex.unlock();
            }

            void lock()
            {
                std::lock(self->set_clear_mutex, m_lock);
            }

            ~custom_lock()
            {
                self->thread_cond_any = nullptr;
                self->set_clear_mutex.unlock();
            }
        };

        custom_lock custom(this, condition_var, lockable);
        interruption_point();
        condition_var.wait(custom);
        interruption_point();
    }

private:
    std::atomic<bool> flag;
    std::condition_variable* thread_cond;
    std::condition_variable_any* thread_cond_any;
    std::mutex set_clear_mutex;
};

inline thread_local interrupt_flag this_thread_interrupt_flag;

class thread_interrupted final : public std::exception
{
    virtual const char* what() const throw()
    {
        return "Thread interrupted";
    }
};

template< typename Lockable >
void interruptible_wait(std::condition_variable_any& cond, Lockable& lockable)
{
    this_thread_interrupt_flag.wait(cond, lockable);
}

class interruptible_thread
{
public:
    template< typename FunctionType >
    interruptible_thread(FunctionType &&f)
    {
        std::promise<interrupt_flag*> p;
        // Packaged task issue, because it does not have non-const call
        m_InternalThread = std::thread([c = std::forward<FunctionType>(f), &p]() mutable {
            p.set_value(&this_thread_interrupt_flag);
            c();
        });
        m_Flag = p.get_future().get();
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
    interrupt_flag* m_Flag;
};

class ThreadPool {
private:
    // Data //
    // ==== //
    using task_type = function_wrapper;

    std::atomic_bool m_Done;
    ThreadSafeQueue<task_type> m_PoolWorkQ;
    std::vector<std::unique_ptr<work_stealing_queue>> m_Queues;


    inline static thread_local work_stealing_queue* m_LocalQueue;
    inline static thread_local size_t m_Index;

    std::vector<std::thread> m_Threads;

    join_threads joiner;

    // Function //
    // ======== //
    void worker_thread(size_t inIndex)
    {
        m_Index = inIndex;
        m_LocalQueue = m_Queues[m_Index].get();
        while (!m_Done)
        {
            run_pending_task();
        }
    }

	bool pop_task_from_local_queue(task_type& task)
	{
		return m_LocalQueue && m_LocalQueue->try_pop(task);
	}

	bool pop_task_from_pool_queue(task_type& task)
	{
		return m_PoolWorkQ.try_pop(task);
	}

	bool pop_task_from_other_thread(task_type& task)
	{
		for (size_t index = 0; index < m_Queues.size(); index++)
		{
		    const size_t i = (m_Index + index + 1) % m_Queues.size();
			if (m_Queues[i]->try_steal(task))
			{
				return true;
			}
		}
		return false;
	}

public:
    ThreadPool()
        : m_Done{ false }
        , joiner(m_Threads)
    {
        unsigned const thread_count = std::thread::hardware_concurrency();
        try
        {
            for (size_t i = 0; i < thread_count; i++)
            {
                m_Queues.push_back(std::unique_ptr<work_stealing_queue>(std::make_unique<work_stealing_queue>()));
            }
            for (size_t i = 0; i < thread_count; i++)
            {
                m_Threads.push_back(std::thread(&ThreadPool::worker_thread, this, i));
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

    template< typename FunctionType >
    std::future<typename std::invoke_result_t<FunctionType>> submit(FunctionType f)
    {
        using result_type = std::invoke_result_t<FunctionType>;
        std::packaged_task<result_type()> task(f);
        std::future<result_type> result(task.get_future());
        if (m_LocalQueue)
        {
            m_LocalQueue->push(std::move(task));
        }
        else
        {
            m_PoolWorkQ.push(std::move(task));
        }
        return result;
    }

    void run_pending_task()
    {
        task_type task;
        if (pop_task_from_local_queue(task)
            || pop_task_from_pool_queue(task)
            || pop_task_from_other_thread(task))
        {
            task();
        }
        else
        {
            std::this_thread::yield();
        }
    }
};

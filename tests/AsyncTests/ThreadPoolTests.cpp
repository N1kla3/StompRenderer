#include "gtest/gtest.h"
#include <iostream>
#include <future>
#include "Async/ThreadPool.h"
#include "Logs.h"

class ThreadPoolSuite : public ::testing::Test
{
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
    }
};

class testmember
{
public:
    void func(std::shared_future<void> fut) { fut.wait();}
    float funcret(std::shared_future<void> fut) { fut.wait(); return 5.f; }
    float withparam(std::shared_future<void> fut, float a, float b) { fut.wait(); return a + b; }
    void consfunc(std::shared_future<void> fut) const { fut.wait();}
};

void myfunc(std::shared_future<void> fut) { fut.wait();  }
void ues() { int i = 4; i++;}

TEST_F(ThreadPoolSuite, ThreadPool_one)
{
    std::promise<void> start;
    std::shared_future<void> ready(start.get_future());
    std::future<void> a,b, z, x;
    std::future<int> first_done, q,w,e,r,t,y,u;
    std::future<float> second_done,s,d,f,g,h,j,k;

    omp::ThreadPool pool{};

    try
    {
        testmember member;
        a = pool.submit(&testmember::func, &member, ready);
        b = pool.submit(myfunc, ready);
        z = pool.submit(ues);
        s = pool.submit(&testmember::funcret, member, ready);
        d = pool.submit(&testmember::withparam, member, ready, 3.f, 5.f);
        x = pool.submit(&testmember::consfunc, member, ready);
        auto lymd = [ready]() -> int {
            ready.wait();
            return 2;
        };
        first_done = pool.submit(lymd);
        auto lamd = [ready]() -> float {
            ready.wait();

            return 0.f;
        };
        second_done = pool.submit(lamd);

        q = pool.submit(lymd);
        f = pool.submit(lamd);
        w = pool.submit(lymd);
        g = pool.submit(lamd);
        e = pool.submit(lymd);
        h = pool.submit(lamd);
        r = pool.submit(lymd);
        t = pool.submit(lymd);
        j = pool.submit(lamd);
        y = pool.submit(lymd);
        k = pool.submit(lamd);
        u = pool.submit(lymd);

        start.set_value();
        // VOIDS
        EXPECT_NO_THROW(a.get());
        EXPECT_NO_THROW(b.get());
        EXPECT_NO_THROW(z.get());
        EXPECT_NO_THROW(x.get());

        // INTS
        EXPECT_TRUE(first_done.get() == 2);
        EXPECT_EQ(q.get(), 2);
        EXPECT_EQ(w.get(), 2);
        EXPECT_EQ(e.get(), 2);
        EXPECT_EQ(r.get(), 2);
        EXPECT_EQ(t.get(), 2);
        EXPECT_EQ(y.get(), 2);
        EXPECT_EQ(u.get(), 2);

        // FLOATS
        EXPECT_TRUE(second_done.get() == 0.f);
        EXPECT_EQ(s.get(), 5.f);
        EXPECT_EQ(d.get(), 8.f);
        EXPECT_EQ(f.get(), 0.f);
        EXPECT_EQ(g.get(), 0.f);
        EXPECT_EQ(h.get(), 0.f);
        EXPECT_EQ(j.get(), 0.f);
        EXPECT_EQ(k.get(), 0.f);
    }
    catch ( ... )
    {
        ERROR(AssetManager, "this is EXCEPTION");
    }
    ASSERT_TRUE(true);
}

TEST_F(ThreadPoolSuite, ThreadPool_two)
{
    std::packaged_task<void()> task([]()
    {
        while (true)
        {
            omp::InterruptionPoint();
        }
    });
    omp::InterruptibleThread inter(std::move(task));

    inter.interrupt();
    inter.join();

    ASSERT_TRUE(true);
}
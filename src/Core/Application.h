#pragma once
#include <string>

namespace omp
{
    class Application
    {
    private:
        virtual void preInit();
        virtual void init();
        virtual void preDestroy();
        virtual void tick(float delta);

    public:
        Application(const std::string& flags);
    };
}

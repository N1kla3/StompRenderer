#pragma once
#include <string>
#include "Renderer.h"
#include "AssetSystem/AssetManager.h"

namespace omp
{
    class Application
    {
    public:
        Application(const std::string& flags);
        void start();

        Renderer* GetRenderer() const { return m_Renderer.get(); }
        omp::AssetManager* GetAssetManager() const { return m_AssetManager.get(); }
        GLFWwindow* GetWindow() const { return m_Window; }
        omp::ThreadPool* GetThreadPool() const { return m_ThreadPool.get(); }

    private:
        virtual void preInit();
        virtual void init();
        virtual void preDestroy();
        virtual void tick(float delta);

    // Data //
    // ==== //
    private:
        std::unique_ptr<Renderer> m_Renderer;
        std::unique_ptr<omp::AssetManager> m_AssetManager;
        GLFWwindow* m_Window;
        std::unique_ptr<omp::ThreadPool> m_ThreadPool;

        std::vector<std::string> m_Flags;

    private:
        void parseFlags(const std::string& commands);
    };
}

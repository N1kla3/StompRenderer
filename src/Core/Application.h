#pragma once
#include <string>
#include "Renderer.h"
#include "Scene.h"
#include "AssetSystem/AssetManager.h"

namespace omp
{
    class Application
    {
    public:
        Application(const std::string& flags);
        void start();
        void requestExit();

        Renderer* getRenderer() const { return m_Renderer.get(); }
        omp::AssetManager* getAssetManager() const { return m_AssetManager.get(); }
        omp::ThreadPool* getThreadPool() const { return m_ThreadPool.get(); }
        omp::ObjectFactory* getObjectFactory() const { return m_Factory.get(); }
        omp::Scene* getCurrentScene() const { return m_CurrentScene.get(); }
        GLFWwindow* getWindow() const { return m_Window; }

    private:
        virtual void preInit();
        virtual void init();
        virtual void preDestroy();
        virtual void tick(float delta);

    // Data //
    // ==== //
    private:
        std::unordered_map<std::string, std::string> m_Flags;

        std::unique_ptr<Renderer> m_Renderer;
        std::unique_ptr<omp::AssetManager> m_AssetManager;
        std::unique_ptr<omp::ThreadPool> m_ThreadPool;
        std::unique_ptr<omp::ObjectFactory> m_Factory;
        std::unique_ptr<omp::Scene> m_CurrentScene;
        GLFWwindow* m_Window;

        uint32_t m_Width = 1280;
        uint32_t m_Height = 720;
        int m_FrameLimit = -1;
        int m_ThreadCount = -1;
        bool m_RequestExit = false;
        

    private:
        void parseFlags(const std::string& commands);
        inline static void windowResizeCallback(GLFWwindow* window, int width, int height);
    };
}

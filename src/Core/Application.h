#pragma once
#include <string>
#include "Renderer.h"
#include "Scene.h"
#include "Core/ICommand.h"
#include "AssetSystem/AssetManager.h"

namespace omp
{
    class Application
    {
    public:
        Application(const std::string& flags);
        virtual ~Application() = default;

        void start();
        void requestExit();

        Renderer* getRenderer() const { return m_Renderer.get(); }
        omp::AssetManager* getAssetManager() const { return m_AssetManager.get(); }
        omp::ThreadPool* getThreadPool() const { return m_ThreadPool.get(); }
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

        std::shared_ptr<omp::CommandStack> m_CommandStack;
        std::unique_ptr<Renderer> m_Renderer;
        std::unique_ptr<omp::AssetManager> m_AssetManager;
        std::unique_ptr<omp::ThreadPool> m_ThreadPool;
        std::shared_ptr<omp::Scene> m_CurrentScene;
        GLFWwindow* m_Window;

        int m_Width = 1280;
        int m_Height = 720;
        int m_FrameLimit = -1;
        int m_ThreadCount = 5;//-1;
        bool m_RequestExit = false;
        

    private:
        void parseFlags(const std::string& commands);
        inline static void windowResizeCallback(GLFWwindow* window, int width, int height);

        void debug_createSceneManually();
        void debug_addLightToScene();
    };
}

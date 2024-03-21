#pragma once
#include <string>
#include "Renderer.h"
#include "AssetSystem/AssetManager.h"

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

        Renderer* GetRenderer() const { return m_Renderer.get(); }
        omp::AssetManager* GetAssetManager() const { return m_AssetManager.get(); }

    // Data //
    // ==== //
    private:
        std::unique_ptr<Renderer> m_Renderer;
        std::unique_ptr<omp::AssetManager> m_AssetManager;
    };
}

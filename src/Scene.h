#pragma once
#include <vector>
#include "Model.h"

namespace omp{
class Scene
{
public:
    Scene() = default;

private:
    // State //
    // ===== //
    bool m_StateDirty = false;

    std::vector<omp::Model> m_Models;

public:
    // Methods //
    // ======= //
    void AddModelToScene(const omp::Model& ModelToAdd);

    const std::vector<omp::Model>& GetModels() const;
    bool IsDirty() const { return m_StateDirty; }
    void ConfirmRendering() { m_StateDirty = true; };
};
} // omp

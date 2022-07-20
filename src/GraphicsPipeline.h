#pragma once


namespace omp{
class GraphicsPipeline
{
public:
    GraphicsPipeline() = default;

    void StartCreation();
    void ConfirmCreation();

private:
    bool m_IsCreated;
};
}

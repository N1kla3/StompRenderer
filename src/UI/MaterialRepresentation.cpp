#include "MaterialRepresentation.h"

omp::MaterialRepresentation::MaterialRepresentation(const std::shared_ptr<Texture> &texture)
    : m_Texture(texture)
{

}

void omp::MaterialRepresentation::SetDependency(const std::shared_ptr<Texture> &texture)
{
    m_Texture = texture;
}

void omp::MaterialRepresentation::renderUIonPanel()
{

}

void omp::MaterialRepresentation::renderUIonContentBrowser()
{

}

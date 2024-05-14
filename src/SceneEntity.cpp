#include "Scene.h"
#include "imgui.h"
#include <memory>
#include "Core/CoreLib.h"
#include "SceneEntity.h"

omp::SceneEntity::SceneEntity()
    : SceneEntity("None", nullptr)
{
    m_Id = omp::CoreLib::generateId32();
}

omp::SceneEntity::SceneEntity(const std::string& inName, const std::shared_ptr<omp::ModelInstance>& inModel)
    : m_Name(inName)
    , m_ModelInstance(inModel)
{
    m_Id = omp::CoreLib::generateId32();
}

void omp::SceneEntity::TryLoadToGpu(const std::shared_ptr<omp::VulkanContext>& context)
{
    m_ModelInstance->tryLoad(context, false);
}

void omp::SceneEntity::setTranslation(const glm::vec3& trans)
{
    m_ModelInstance->getPosition() = trans;
}

void omp::SceneEntity::setRotation(const glm::vec3& rot)
{
    m_ModelInstance->getRotation() = rot;
}

void omp::SceneEntity::setScale(const glm::vec3& scale)
{
    m_ModelInstance->getScale() = scale;
}

void omp::SceneEntity::draw()
{
    if (m_ModelInstance)
    {
        ImGui::Text("%s", m_ModelInstance->getName().c_str());
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {1.0f, 0.5f, 0.5f, 0.5f});
        ImGui::DragFloat3("Position", &m_ModelInstance->getPosition()[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::PopStyleColor(1);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.0f, 1.0f, 0.5f, 0.5f});
        ImGui::DragFloat3("Rotation", &m_ModelInstance->getRotation()[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::PopStyleColor(1);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.6f, 0.5f, 1.0f, 0.5f});
        ImGui::DragFloat3("Scale", &m_ModelInstance->getScale()[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::PopStyleColor(1);
    }
    else
    {
        ImGui::BulletText("No Entity");
    }
}
void omp::SceneEntity::onSceneSave(JsonParser<>& parser, omp::Scene* scene)
{
    parser.writeValue("Id", m_Id);
    parser.writeValue("Name", m_Name);

    glm::vec3& pos = m_ModelInstance->getPosition();
    parser.writeValue("translation_x", pos.x);
    parser.writeValue("translation_y", pos.y);
    parser.writeValue("translation_z", pos.z);

    glm::vec3& rot = m_ModelInstance->getRotation();
    parser.writeValue("rotation_x", rot.x);
    parser.writeValue("rotation_y", rot.y);
    parser.writeValue("rotation_z", rot.z);

    glm::vec3& scale = m_ModelInstance->getScale();
    parser.writeValue("scale_x", scale.x);
    parser.writeValue("scale_y", scale.y);
    parser.writeValue("scale_z", scale.z);

    if (!m_ModelInstance->getModel().expired())
    {
        parser.writeValue("model_id", scene->serializeDependency(m_ModelInstance->getModel().lock().get()));
    }
    if (m_ModelInstance && m_ModelInstance->getMaterialInstance() && m_ModelInstance->getMaterialInstance()->getStaticMaterial().lock().get()) 
    {
        parser.writeValue("material_id", scene->serializeDependency(m_ModelInstance->getMaterialInstance()->getStaticMaterial().lock().get()));
    }

    glm::vec4 ambient = m_ModelInstance->getMaterialInstance()->getAmbient();
    parser.writeValue("ambient_x", ambient.x);
    parser.writeValue("ambient_y", ambient.y);
    parser.writeValue("ambient_z", ambient.z);
    parser.writeValue("ambient_w", ambient.w);

    glm::vec4 diffusive = m_ModelInstance->getMaterialInstance()->getDiffusive();
    parser.writeValue("diffusive_x", diffusive.x);
    parser.writeValue("diffusive_y", diffusive.y);
    parser.writeValue("diffusive_z", diffusive.z);
    parser.writeValue("diffusive_w", diffusive.w);

    glm::vec4 specualr = m_ModelInstance->getMaterialInstance()->getSpecular();
    parser.writeValue("specular_x", specualr.x);
    parser.writeValue("specular_y", specualr.y);
    parser.writeValue("specular_z", specualr.z);
    parser.writeValue("specular_w", specualr.w);
}

void omp::SceneEntity::onSceneLoad(JsonParser<>& parser, omp::Scene* scene)
{
    m_Id = parser.readValue<uint32_t>("Id").value_or(0);
    m_Name = parser.readValue<std::string>("Name").value_or("");

    glm::vec3 pos;
    pos.x = parser.readValue<float>("translation_x").value();
    pos.y = parser.readValue<float>("translation_y").value();
    pos.z = parser.readValue<float>("translation_z").value();

    glm::vec3 rot;
    rot.x = parser.readValue<float>("rotation_x").value();
    rot.y = parser.readValue<float>("rotation_y").value();
    rot.z = parser.readValue<float>("rotation_z").value();

    glm::vec3 scale;
    scale.x = parser.readValue<float>("scale_x").value();
    scale.y = parser.readValue<float>("scale_y").value();
    scale.z = parser.readValue<float>("scale_z").value();

    glm::vec4 ambient;
    ambient.x = parser.readValue<float>("ambient_x").value();
    ambient.y = parser.readValue<float>("ambient_y").value();
    ambient.z = parser.readValue<float>("ambient_z").value();
    ambient.w = parser.readValue<float>("ambient_w").value();

    glm::vec4 diffusive;
    diffusive.x = parser.readValue<float>("diffusive_x").value();
    diffusive.y = parser.readValue<float>("diffusive_y").value();
    diffusive.z = parser.readValue<float>("diffusive_z").value();
    diffusive.w = parser.readValue<float>("diffusive_w").value();

    glm::vec4 specular;
    specular.x = parser.readValue<float>("specular_x").value();
    specular.y = parser.readValue<float>("specular_y").value();
    specular.z = parser.readValue<float>("specular_z").value();
    specular.w = parser.readValue<float>("specular_w").value();

    omp::SerializableObject::SerializationId model_id = parser.readValue<omp::SerializableObject::SerializationId>("model_id").value();
    omp::SerializableObject::SerializationId mat_id = parser.readValue<omp::SerializableObject::SerializationId>("material_id").value();
    std::shared_ptr<omp::Model> model_casted = std::dynamic_pointer_cast<omp::Model>(scene->getDependency(model_id));
    if (model_casted)
    {
        m_ModelInstance->setModel(model_casted);
        glm::vec3& old_pos = m_ModelInstance->getPosition();
        old_pos = pos;
        glm::vec3& old_rot = m_ModelInstance->getRotation();
        old_rot = rot;
        glm::vec3& old_scale = m_ModelInstance->getScale();
        old_scale = scale;
        std::shared_ptr<omp::Material> mat_casted = std::dynamic_pointer_cast<omp::Material>(scene->getDependency(mat_id));
        if (mat_casted)
        {
            std::shared_ptr<omp::MaterialInstance> mat_inst = std::make_shared<omp::MaterialInstance>(mat_casted);
            m_ModelInstance->setMaterialInstance(mat_inst);
            mat_inst->setAmbient(ambient);
            mat_inst->setDiffusive(diffusive);
            mat_inst->setSpecular(specular);
        }
        else
        {
            ERROR(LogAssetManager, "Invalid asset type retrieved from id: {}", model_id);
        }
    }
    else
    {
        ERROR(LogAssetManager, "Invalid asset type retrieved from id: {}", model_id);
    }
}


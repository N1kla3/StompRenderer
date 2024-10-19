#pragma once
#include "IO/SerializableObject.h"

#define OMP_READ_SETTING(type, variable, key)\
    auto variable##_opt = parser.readValue<type>(key);\
    if (variable##_opt.has_value()) variable = variable##_opt.value()

namespace omp
{
    class ProjectSettings : public omp::SerializableObject
    {
    public:

        int32_t version = 100;
        std::string project_name = "None";
        std::string default_map = "../assets/main_scene.json";

        virtual void serialize(omp::JsonParser<> &parser) override
        {
            parser.writeValue("ProjectVersion", version);
            parser.writeValue("ProjectName", project_name);
            parser.writeValue("DefaultMap", default_map);
        }

        virtual void deserialize(omp::JsonParser<> &parser) override
        {
            OMP_READ_SETTING(int32_t, version, "ProjectVersion");
            OMP_READ_SETTING(std::string, project_name, "ProjectName");
            OMP_READ_SETTING(std::string, default_map, "DefaultMap");
        }
    };
}

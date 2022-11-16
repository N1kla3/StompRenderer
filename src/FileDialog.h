#pragma once


#include <string>
#include <optional>

class FileDialog
{
public:
    static std::optional<std::string> openDir();
};

#pragma once
#include <string>
#include <memory>

#include "World/Assets/Mesh.h"

class MeshLoader
{
public:
    static std::shared_ptr<Mesh> LoadFromFile(const std::string& path);
};
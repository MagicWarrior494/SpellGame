#pragma once
#include <string>

class Asset
{
public:
    virtual ~Asset() = default;

    void        SetName(const std::string& name) { m_name = name; }
    std::string GetName() const { return m_name; }

private:
    std::string m_name;
};
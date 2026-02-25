#pragma once
#include <glm.hpp>
#include <functional>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    bool operator==(const Vertex& other) const
    {
        return position == other.position &&
            normal == other.normal &&
            uv == other.uv;
    }
};

inline void hashCombine(std::size_t& seed, std::size_t hash)
{
    seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std
{
    template<>
    struct hash<Vertex>
    {
        size_t operator()(const Vertex& v) const noexcept
        {
            size_t seed = 0;

            hashCombine(seed, hash<float>()(v.position.x));
            hashCombine(seed, hash<float>()(v.position.y));
            hashCombine(seed, hash<float>()(v.position.z));

            hashCombine(seed, hash<float>()(v.normal.x));
            hashCombine(seed, hash<float>()(v.normal.y));
            hashCombine(seed, hash<float>()(v.normal.z));

            hashCombine(seed, hash<float>()(v.uv.x));
            hashCombine(seed, hash<float>()(v.uv.y));

            return seed;
        }
    };
}
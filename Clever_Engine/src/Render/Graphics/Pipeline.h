#pragma once
#include <cstdint>
#include "World/Assets/ShaderReflected.h"

enum class Cullmode
{
    Back,
    Front,
    None
};

enum class FrontFace
{
    CounterClockwise,
    Clockwise
};

enum class PolygonMode
{
    Fill,
    Line,
    Point
};

enum class PrimitiveTopology
{
    TriangleList,
    LineList,
    PointList
};

enum class SampleCount
{
    Sample1 = 1,
    Sample2 = 2,
    Sample4 = 4,
    Sample8 = 8,
    Sample16 = 16,
    Sample32 = 32,
    Sample64 = 64
};

struct VertexInputBindingDescription
{
    uint32_t binding;
    uint32_t stride;
    VertexInputRate inputRate; // uses the one from ShaderReflected.h
};

struct PipelineCreationInfo
{
    Cullmode  cullMode = Cullmode::Back;
    FrontFace frontFace = FrontFace::CounterClockwise;
    PolygonMode polygonMode = PolygonMode::Fill;

    bool enableBlending = false;
    bool enableDepthTest = true;

    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    SampleCount       samples = SampleCount::Sample1;

    VertexInputBindingDescription bindingDescription{};
    ShaderReflection              reflectedShader{};
};

struct PipelineLayoutInfo {};

class PipelineLayout {};
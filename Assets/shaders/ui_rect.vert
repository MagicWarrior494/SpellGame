#version 450

// Quad push constants — position and size in pixel-space [0..width, 0..height],
// converted to NDC inside the shader using the scene dimensions.
layout(push_constant) uniform PushConstants
{
    vec2  position;    // top-left corner in pixels
    vec2  size;        // width and height in pixels
    vec4  color;       // RGBA [0..1]
    vec2  resolution;  // scene width and height in pixels
} pc;

// Generate a quad from gl_VertexIndex — no vertex buffer needed.
// Two triangles, CCW winding:
//  0?2
//  ???
//  1?3
void main()
{
    // Build the four corners from position + size
    vec2 corners[4];
    corners[0] = pc.position;                          // top-left
    corners[1] = pc.position + vec2(0.0, pc.size.y);  // bottom-left
    corners[2] = pc.position + vec2(pc.size.x, 0.0);  // top-right
    corners[3] = pc.position + pc.size;                // bottom-right

    // Triangle list indices: 0,1,2, 1,3,2
    int indices[6] = int[](0, 1, 2, 1, 3, 2);
    vec2 pos = corners[indices[gl_VertexIndex]];

    // Convert from pixel-space to NDC: [0,W]x[0,H] ? [-1,1]x[-1,1]
    vec2 ndc = (pos / pc.resolution) * 2.0 - 1.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
}

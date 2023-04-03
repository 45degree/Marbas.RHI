#version 450

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

layout (location = 0) out vec2 TexCoords;

layout(std140, binding = 0, set = 0) uniform Matrices {
  mat4 model;
  mat4 view;
  mat4 projection;
};

void main() {
  TexCoords = aTexCoords;
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  gl_Position.y = -gl_Position.y;
}

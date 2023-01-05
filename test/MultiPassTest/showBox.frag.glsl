#version 450

layout(location=0) out vec4 FragColor;
layout(location=0) in vec2 TexCoords;

layout(binding = 0) uniform sampler2D texture1;

void main() {
  FragColor = vec4(texture(texture1, TexCoords).rgb, 1);
}

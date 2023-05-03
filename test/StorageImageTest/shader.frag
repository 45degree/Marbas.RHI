#version 450

layout(location = 0) in vec2 inTex;

layout (binding = 0, set = 0, rgba8) writeonly uniform image2D outputImage;

void main() {
  vec4 color = vec4(gl_FragCoord.xy / 300, 1, 1);
  imageStore(outputImage, ivec2(gl_FragCoord.xy), color);
}

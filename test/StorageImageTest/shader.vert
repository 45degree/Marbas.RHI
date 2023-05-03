#version 450

layout(location = 0) out vec2 aTex;

vec3 gridPlane[6] = vec3[] (
  vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
  vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

vec2 TexCoor[6] = vec2[] (
  vec2(1, 1), vec2(0, 0), vec2(0, 1),
  vec2(0, 0), vec2(1, 1), vec2(1, 0)
);

void main() {
  vec3 p = gridPlane[gl_VertexIndex].xyz;
  aTex = TexCoor[gl_VertexIndex];
  gl_Position = vec4(p, 1.0);
}

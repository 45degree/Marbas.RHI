#version 450

layout(local_size_x = 5, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0, set = 0) buffer calcBuf {
  int data[5];
};

void main() {
  data[gl_LocalInvocationIndex] = data[gl_LocalInvocationIndex] * data[gl_LocalInvocationIndex];
}

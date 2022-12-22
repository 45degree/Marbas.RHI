#include <fstream>
#include <iostream>

#include "RHIFactory.hpp"

int
main(void) {
  std::ifstream file("shader.vert.glsl.spv", std::ios::binary | std::ios::in);
  std::vector<char> code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  auto targetStr = Marbas::RHIFactory::ConvertShader(code, Marbas::RendererType::DirectX12);

  std::cout << targetStr << std::endl;
}

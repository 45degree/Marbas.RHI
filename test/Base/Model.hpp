#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <vector>

#include "RHIFactory.hpp"

namespace Marbas {

struct Vertex {
  glm::vec3 position = glm::vec3(0, 0, 0);
  glm::vec2 texCoord = glm::vec2(0, 0);

  bool
  operator==(const Vertex& other) const {
    return position == other.position && texCoord == other.texCoord;
  }

  constexpr static std::vector<InputElementDesc>
  GetInputElementDesc() {
    InputElementDesc posAttribute, texCoordAttribute;
    posAttribute.binding = 0;
    posAttribute.attribute = 0;
    posAttribute.format = Marbas::ElementType::R32G32B32_SFLOAT;
    posAttribute.offset = offsetof(Vertex, position);
    posAttribute.instanceStepRate = 0;

    texCoordAttribute.binding = 0;
    texCoordAttribute.attribute = 1;
    texCoordAttribute.format = Marbas::ElementType::R32G32_SFLOAT;
    texCoordAttribute.offset = offsetof(Vertex, texCoord);

    return {posAttribute, texCoordAttribute};
  }
};

class Model final {
 public:
  Model() = default;
  explicit Model(const std::string& modelPath);
  Model(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
      : m_vertices(vertices), m_indices(indices) {}
  ~Model() = default;

 public:
  std::vector<Vertex> m_vertices;
  std::vector<uint32_t> m_indices;
};

}  // namespace Marbas

namespace std {
template <>
struct hash<Marbas::Vertex> {
  size_t
  operator()(Marbas::Vertex const& vertex) const {
    return (hash<glm::vec3>()(vertex.position) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
  }
};
}  // namespace std

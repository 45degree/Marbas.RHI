---@diagnostic disable: undefined-global

if has_config("buildTest") then
  includes("Base")
  includes("WindowTest")
  includes("DrawTriangleTest")
  includes("VertexAndIndexTest")
  includes("ImguiTest")
  includes("UniformBufferTest")
  includes("Image2DTextureTest")
  includes("DepthBufferTest")
  includes("MipmapTest")
  includes("MsaaTest")
  includes("MultiPassTest")
end

%VULKAN_SDK%/Bin/glslc.exe shaders/shader.vert -o SPRV/vert.spv
%VULKAN_SDK%/Bin/glslc.exe shaders/shader.frag -o SPRV/frag.spv
%VULKAN_SDK%/Bin/glslc.exe shaders/shaderLight.vert -o SPRV/vertLight.spv
%VULKAN_SDK%/Bin/glslc.exe shaders/shaderLight.frag -o SPRV/fragLight.spv
%VULKAN_SDK%/Bin/glslc.exe shaders/outline.vert -o SPRV/vertOutline.spv
%VULKAN_SDK%/Bin/glslc.exe shaders/outline.frag -o SPRV/fragOutline.spv
%VULKAN_SDK%/Bin/glslc.exe shaders/shaderLightBlend.vert -o SPRV/vertLightBlend.spv
%VULKAN_SDK%/Bin/glslc.exe shaders/shaderLightBlend.frag -o SPRV/fragLightBlend.spv

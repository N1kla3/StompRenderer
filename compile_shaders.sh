dir=SPRV
mkdir -p $dir
glslc shaders/shader.frag -o $dir/frag.spv
glslc shaders/shader.vert -o $dir/vert.spv
glslc shaders/shaderLight.frag -o $dir/fragLight.spv
glslc shaders/shaderLight.vert -o $dir/vertLight.spv
glslc shaders/outline.frag -o $dir/fragOutline.spv
glslc shaders/outline.vert -o $dir/vertOutline.spv


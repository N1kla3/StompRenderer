dir=SPRV
mkdir -p $dir
glslc shaders/shader.frag -o $dir/frag.spv
glslc shaders/shader.vert -o $dir/vert.spv


dir=SPRV
mkdir -p $dir
# glslc shaders/shader.frag -o $dir/frag.spv
# glslc shaders/shader.vert -o $dir/vert.spv
# glslc shaders/shaderLight.frag -o $dir/fragLight.spv
# glslc shaders/shaderLight.vert -o $dir/vertLight.spv
# glslc shaders/outline.frag -o $dir/fragOutline.spv
# glslc shaders/outline.vert -o $dir/vertOutline.spv
# glslc shaders/shaderLightBlend.frag -o $dir/fragLightBlend.spv
# glslc shaders/shaderLightBlend.vert -o $dir/vertLightBlend.spv
# glslc shaders/skybox.frag -o $dir/fragSkybox.spv
# glslc shaders/skybox.vert -o $dir/vertSkybox.spv

cd shaders
for f in *.vert 
do 
	glslc $f -o ../$dir/${f%.*}vert.spv
	echo ../$dir/${f%.*}vert.spv
done

for v in *.frag 
do 
	glslc $v -o ../$dir/${v%.*}frag.spv
	echo ../$dir/${v%.*}frag.spv
done

cd ..

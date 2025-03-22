shader_type spatial;
render_mode world_vertex_coords;

//#include "res://Addons/CodeJar/Shaders/Functions/triplanarSampling.gdshaderinc"
#include "res://addons/jar_voxel_terrain/src/shaders/color_transformation.gdshaderinc"


uniform sampler2D noise;
uniform float scale = 0.25;
uniform float normalScale = 0.15;
uniform vec4 rock_color : source_color;
uniform vec4 dirt_color : source_color;

//varying vec3 global_normal;
varying vec3 global_position;


void vertex() {
	global_position = VERTEX;
}

void fragment() {
	vec3 global_normal = normalize((INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz);
	//vec3 sampled_noise = sampleFractalTriPlanar(noise, global_position, global_normal, scale, CAMERA_POSITION_WORLD).rgb;

	//vec3 sampled_noise = sampleTriplanar(noise, global_position, global_normal, scale).rgb;

	//global_normal = noiseNormalTransformer(global_normal, sampled_noise, normalScale);
	SPECULAR = 0.1;
	ROUGHNESS = 0.8;

	//float grass_normal_factor = slopeStep(global_position.y, biome.ground_range.x, biome.ground_range.y, 100.0);

	vec3 color = rock_color.rgb;
	//if (global_normal.y > mix(1.0, 0.72f, grass_normal_factor)) {
		////color = biome.ground_color;
	//}

	// Edited color
	if(COLOR.r > 0.5) {
		color = dirt_color.rgb;
		SPECULAR = 0.1;
		ROUGHNESS = 0.8;
	}

	NORMAL = normalize((VIEW_MATRIX * vec4(global_normal, 0.0)).xyz);
	ALBEDO = color;
	//ALBEDO = noiseColorTransformer(color.rgb, sampled_noise);
}
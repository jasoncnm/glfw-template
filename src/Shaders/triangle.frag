#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

float logisticDepth(float depth, float steepness, float offset)
{
	float zVal = LinearizeDepth(depth);
	return (1 / (1 + exp(-steepness * (zVal - offset))));
}

void main()
{
	float depth = logisticDepth(gl_FragCoord.z, 0.5f, 5.0f);

    vec4 texel = texture(texSampler, fragTexCoord);
	vec4 depth_color = vec4(depth * vec3(0.85f, 0.85f, 0.9f), 1.0f);

    outColor = texel * (1.0f - depth) + depth_color;
}

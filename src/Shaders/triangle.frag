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

void main()
{
    vec4 texels = texture(texSampler, fragTexCoord);
    // outColor = vec4(fragColor * texels.rgb, texels.a);
    
    // outColor = texels;

	float depth = LinearizeDepth(gl_FragCoord.z) / far;
	outColor = vec4(vec3(depth), 1.0);
}

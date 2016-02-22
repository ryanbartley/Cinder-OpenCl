#version 150

// inputs passed from the vertex shader
in vec4				vVertex;
in vec3				vNormal;
in vec3				vTangent;
in vec3				vBiTangent;
in vec2				vTexCoord0;

// output a single color
out vec4			oColor;

// textures and settings
uniform	sampler2D	uDiffuseMap;
uniform	sampler2D	uSpecularMap;
uniform	sampler2D	uNormalMap;

// light source information
#define MAX_NUM_LIGHTS 16

struct LightSource
{
	vec4 position;
	vec4 diffuse;
	vec4 specular;
};

uniform LightSource	uLights[MAX_NUM_LIGHTS];
uniform int			uNumOfLights;

void main()
{
	// fetch the normal from the normal map
	vec3	vMappedNormal = texture(uNormalMap, vTexCoord0.st).rgb * 2.0 - 1.0;

	// modify it using the normal & tangents from the 3D mesh (normal mapping)
	vec3	vSurfaceNormal = normalize((vTangent * vMappedNormal.x) + (vBiTangent * vMappedNormal.y) + (vNormal * vMappedNormal.z));
  
	// apply each of our light sources
	vec4	vDiffuseColor	= vec4(0, 0, 0, 1);
	vec4	vSpecularColor	= vec4(0, 0, 0, 1);

	vec3	vToCamera = normalize(-vVertex.xyz);

	for(int i=0; i<uNumOfLights; ++i)
	{
		// calculate view space light vectors
		vec3	vToLight = normalize(uLights[i].position.xyz - vVertex.xyz); 
		vec3	vReflect = normalize(-reflect(vToLight, vSurfaceNormal));

		// calculate diffuse term
		float	fDiffuse = max(dot(vSurfaceNormal, vToLight), 0.0);
		fDiffuse = clamp(fDiffuse, 0.1, 1.0);  

		// calculate specular term
		float	fSpecularPower = 100.0;
		float	fSpecular = pow( max( dot(vReflect, vToCamera), 0.0), fSpecularPower );
		fSpecular = clamp(fSpecular, 0.0, 1.0);

		// calculate final colors
		vDiffuseColor += texture(uDiffuseMap, vTexCoord0.st) * uLights[i].diffuse * fDiffuse;
		vSpecularColor += texture(uSpecularMap, vTexCoord0.st) * uLights[i].specular * fSpecular;
	}

	// output colors to buffer
	oColor.rgb = (vDiffuseColor + vSpecularColor).rgb;
	oColor.a = 1.0;
}
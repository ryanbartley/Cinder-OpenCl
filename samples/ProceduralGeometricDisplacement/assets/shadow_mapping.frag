#version 410

uniform float ciElapsedSeconds;

in vec4 vColor;
in vec4 vPosition;
in vec3 vNormal;
in vec4 vModelPosition;
in vec3 vModelNormal;
in vec4	vShadowCoord;

uniform sampler2DShadow	uShadowMap;
uniform float			uDepthBias;

uniform vec3	uMatAmbient;
uniform vec3	uMatDiffuse;
uniform vec3	uMatSpecular;
uniform float   uMatShininess;

uniform struct Light {
	vec3 position;
	vec3 intensities;
	float attenuation;
	float ambientCoefficient;
};

uniform Light light;

out vec4 fragColor;

float samplePCF3x3( vec4 sc )
{
	const int s = 2;
	
	float shadow = 0.0;
	shadow += textureProjOffset( uShadowMap, sc, ivec2(-s,-s) );
	shadow += textureProjOffset( uShadowMap, sc, ivec2(-s, 0) );
	shadow += textureProjOffset( uShadowMap, sc, ivec2(-s, s) );
	shadow += textureProjOffset( uShadowMap, sc, ivec2( 0,-s) );
	shadow += textureProjOffset( uShadowMap, sc, ivec2( 0, 0) );
	shadow += textureProjOffset( uShadowMap, sc, ivec2( 0, s) );
	shadow += textureProjOffset( uShadowMap, sc, ivec2( s,-s) );
	shadow += textureProjOffset( uShadowMap, sc, ivec2( s, 0) );
	shadow += textureProjOffset( uShadowMap, sc, ivec2( s, s) );
	return shadow/9.0;;
}

void main()
{
	// Normal in view space
	vec3	N = normalize( vNormal );
	// Light direction
	vec3	L = normalize( light.position - vPosition.xyz );
	// To camera vector
	vec3	C = normalize( -vPosition.xyz );
	// Surface reflection vector
	vec3	R = normalize( -reflect( L, N ) );
	
	//ambient
	vec3 ambient = light.ambientCoefficient * uMatAmbient * light.intensities;
	
	// Diffuse factor
	float diffuseCoefficient = max( dot( N, L ), 0.0 );
	vec3 diffuse = diffuseCoefficient * uMatDiffuse * light.intensities;
	
	// Specular factor
	float specularCoefficient = 0.0;
	if(diffuseCoefficient > 0.0)
		specularCoefficient = pow( max( dot( R, C ), 0.0 ), uMatShininess );
	vec3 specular = specularCoefficient * uMatSpecular * light.intensities;
	
	// Attenuation Factor
	float distanceToLight = length(light.position - vPosition.xyz);
	float attenuation = 1.0 / (1.0 + light.attenuation * pow(distanceToLight, 2));
		
	// Sample the shadowmap to compute the shadow value
	float shadow = 1.0f;
	vec4 sc = vShadowCoord;
	sc.z += uDepthBias;
	
	shadow = samplePCF3x3( sc );

	// linear color (color before gamma correction)
	vec3 linearColor = ambient + ( shadow * ( attenuation * ( diffuse + specular ) ) );
	
	// final color (after gamma correction)
	vec3 gamma = vec3( 1.0 / 2.2 );
	fragColor = vec4( pow( linearColor, gamma ), 1.0f );
}

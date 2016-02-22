#version 330

in vec4 ciPosition;

uniform ivec2 ciWindowSize;
uniform mat4  ciViewMatrix;
uniform mat4  ciProjectionMatrix;
uniform mat4  ciModelViewProjection;
uniform float uParticleDiameter;
uniform float uMaxLife;
uniform float uParticleSizeFactor;

out float vLife;
out vec4  vPosition;

void main(){
    
    vec3 viewSpacePosition = vec3( ciViewMatrix  *  vec4( ciPosition.xyz, 1.0 ) );
    
    vec4 corner = vec4(uParticleDiameter * 0.5, uParticleDiameter * 0.5, viewSpacePosition.z, 1.0);
    
    float projectedCornerX = dot( vec4(ciProjectionMatrix[0][0],
                                       ciProjectionMatrix[1][0],
                                       ciProjectionMatrix[2][0],
                                       ciProjectionMatrix[3][0]
                                       )
                                 , corner );
    
    float projectedCornerW = dot( vec4(ciProjectionMatrix[0][3],
                                       ciProjectionMatrix[1][3],
                                       ciProjectionMatrix[2][3],
                                       ciProjectionMatrix[3][3]
                                       )
                                 , corner);
    
    gl_PointSize = 2.;//float(ciWindowSize.x) * 0.5 * projectedCornerX * uParticleSizeFactor / projectedCornerW * smoothstep(0.,1., ciPosition.w/uMaxLife);

    vPosition = ciPosition;
    gl_Position = ciModelViewProjection * vec4(ciPosition.xyz, 1.);
    vLife = ciPosition.w;
}
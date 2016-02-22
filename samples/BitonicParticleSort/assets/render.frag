#version 330

in float vLife;
in vec4 vPosition;
uniform float uMaxLife;
uniform vec3 uColor;
out vec4 FragColor;

float lmap( float val, float inMin, float inMax, float outMin, float outMax ){
    return outMin + (outMax - outMin) * ((val - inMin) / (inMax - inMin));
}

void main(){
    float alpha = pow( exp( -pow( length( gl_PointCoord.xy - vec2(.5) ), 4. ) ), 16. );
    //float z = clamp(lmap( vPosition.z, -2., 2., 0.,1 ),0.,1.);
    FragColor = vec4( uColor , alpha );
}
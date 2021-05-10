#version 430

uniform sampler2D texture1;
in vec2 texcoord;
out vec4 fragmentColor;

uniform float gamma;

void main() {
    fragmentColor = texture( texture1, texcoord );
    fragmentColor.rgb = pow( fragmentColor.rgb, vec3( 1.0 / gamma));
}
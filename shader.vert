#version 430

layout( location = 0) in vec2 vertexPosition;

out vec2 texcoord;


void main() {
    gl_Position = vec4( vertexPosition, 0, 1 );
    texcoord = (vertexPosition + vec2( 1, 1 )) / 2;
}
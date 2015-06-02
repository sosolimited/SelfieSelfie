#version 100

precision highp float;

attribute vec4  ciPosition;
attribute vec2  ciTexCoord0;

varying vec2 vTexCoord;

void main()
{
	gl_Position = ciPosition; // we specify position in NDC
  vTexCoord = ciTexCoord0; // offsetTextureCoordinate( ciTexCoord0, uFrameIndex );
}

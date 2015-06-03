#version 100

precision mediump float;

// Position in Normalized Device Coordinates.
attribute vec4  ciPosition;
attribute vec2  ciTexCoord0;

varying vec2 vTexCoord;

void main()
{
	gl_Position = ciPosition;
  vTexCoord = ciTexCoord0;
}

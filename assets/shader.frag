#version 460 core

out vec4 FragColor;

in float vertexColor;

uniform vec4 triangColor;
uniform bool calcAlpha;

void main()
{
	if (calcAlpha)
		FragColor = triangColor * vertexColor;
	else
		FragColor = vec4(triangColor.rgb * vertexColor, triangColor.a);
}
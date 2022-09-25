#shader vertex
#version 330 core

layout( location = 0 ) in vec4 position;
layout( location = 0 ) in vec3 translate;

void main()
{
	gl_Position = position;
};

#shader fragment
#version 330 core

layout( location = 0 ) out vec4 fragcolor;

uniform vec4 color;

void main()
{
	fragcolor = color;
};
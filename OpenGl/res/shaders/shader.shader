#shader vertex
#version 330 core

layout( location = 0 ) in vec4 position;

uniform mat4 u_MVP;

void main()
{
	gl_Position = u_MVP * position;
};

#shader fragment
#version 330 core

layout( location = 0 ) out vec4 fragcolor;

uniform vec4 u_Color;

void main()
{
	fragcolor = u_Color;
};

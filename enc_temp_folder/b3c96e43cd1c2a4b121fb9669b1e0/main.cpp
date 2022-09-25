#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader( const std::string& filepath )
{
	std::ifstream stream( filepath );

	enum class ShaderType
	{
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1
	};

	std::string line;
	std::stringstream sstring[2];

	ShaderType stype = ShaderType::NONE;

	while ( getline( stream, line ) )
	{
		if ( line.find( "#shader" ) != std::string::npos )
		{
			if ( line.find( "vertex" ) != std::string::npos )
			{
				stype = ShaderType::VERTEX;
			}
			if ( line.find( "fragment" ) != std::string::npos )
			{
				stype = ShaderType::FRAGMENT;
			}
		}
		else
		{
			sstring[(int)stype] << line << '\n';
		}
	}

	return { sstring[0].str(), sstring[1].str() };
}


float triangleVertices[6] = {
	0.0f, 0.4f
	,0.4f, -0.4f
	,-0.4f, -0.4f,
};

float squareVertices[12] = {
	-0.4f, -0.4f
	,0.4f, -0.4f
	,0.4f, 0.4f

	,0.4f, 0.4f
	,-0.4f, 0.4f
	,-0.4f, -0.4f
};

float position[4] = {
	0.0f, 0.0f, 0.0f, 1.0f
};


float color[4] = { 
	1, 0.76, 0, 1.0 
};

float bgcolor[4] = {
	0, 0, 0, 0
};

bool drawTriangle = false;
bool drawSquare = false;

bool* pdrawTriangle = &drawTriangle;
bool* pdrawSquare = &drawSquare;

static unsigned int CompileShader( unsigned int type, const std::string& source )
{
	unsigned int id = glCreateShader( type );
	const char* src = source.c_str();
	glShaderSource( id, 1, &src, nullptr );
	glCompileShader( id );

	int result;

	glGetShaderiv( id, GL_COMPILE_STATUS, &result );
	if ( result == GL_FALSE )
	{
		int length;

		glGetShaderiv( id, GL_INFO_LOG_LENGTH, &length );
		char* message = (char*)alloca( length * sizeof( char ) );
		glGetShaderInfoLog( id, length, &length, message );
		std::cout << "Failed to compile " <<
			(type == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< std::endl;
		std::cout << message << std::endl;

		glDeleteShader( id );

		return 0;
	}

	return id;
}

static unsigned int CreateShader( const std::string vertexShader, const std::string& fragmentShader )
{
	unsigned int program = glCreateProgram();

	unsigned int vs = CompileShader( GL_VERTEX_SHADER, vertexShader );
	unsigned int fs = CompileShader( GL_FRAGMENT_SHADER, fragmentShader );

	glAttachShader( program, vs );
	glAttachShader( program, fs );
	glLinkProgram( program );
	glValidateProgram( program );

	glDeleteShader( fs );
	glDeleteShader( vs );

	return program;
}


int main( void )
{
	glEnable( GL_DEPTH_TEST );

	GLFWwindow* window;

	/* Initialize the library */
	if ( !glfwInit() )
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow( 1280, 960, "Spinning Cube", NULL, NULL );
	if ( !window )
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent( window );

	if ( glewInit() != GLEW_OK )
		return -1;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL( window, true );
	ImGui_ImplOpenGL3_Init( "#version 330" );
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();




	unsigned int buffer;

	glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, 6 * 2 * sizeof( float ), squareVertices, GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 2, 0 );


	ShaderProgramSource source = ParseShader( "res/shaders/shader.shader" );

	unsigned int shader = CreateShader( source.VertexSource, source.FragmentSource );

	glUseProgram( shader );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );


	/* Loop until the user closes the window */
	while ( !glfwWindowShouldClose( window ) )
	{
		/* Render here */
		glClearColor( bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3] );
		glClear( GL_COLOR_BUFFER_BIT );


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin( "Edit" );
		ImGui::ColorEdit4( "Object Color", color );
		ImGui::ColorEdit4( "Background Color", bgcolor );
		ImGui::SliderFloat3( "Transform", position, -5.0f, 5.0f);
		ImGui::Checkbox( "Draw Triangle", pdrawTriangle );
		ImGui::Checkbox( "Draw Square", pdrawSquare );

		glDrawArrays( GL_TRIANGLES, 0, 6 );

		ImGui::End();

		glUseProgram( shader );
		glUniform4f( glGetUniformLocation( shader, "color" ), color[0], color[1], color[2], color[3] );
		glUniform4f( glGetUniformLocation( shader, "translate" ), position[0], position[1], position[2], position[3] );
		glTranslated( position[0],position[1], position[2]);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

		/* Swap front and back buffers */
		glfwSwapBuffers( window );

		/* Poll for and process events */
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteProgram( shader );

	glfwTerminate();
	return 0;
}
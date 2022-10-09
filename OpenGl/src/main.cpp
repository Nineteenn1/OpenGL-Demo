#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

//float squareVertices[12] = {
//	-0.4f, -0.4f
//	,0.4f, -0.4f
//	,0.4f, 0.4f
//
//	,0.4f, 0.4f
//	,-0.4f, 0.4f
//	,-0.4f, -0.4f
//};

float squareVertices[12] = {
	200.0f, 200.0f
	,400.0f, 200.0f
	,400.0f, 400.0f
	,400.0f, 400.0f
	,200.0f, 400.0f
	,200.0f, 200.0f
};


unsigned int indices[] = {
	0, 1, 2,
	2, 3, 0
};

float position[4] = {
	0.0f, 0.0f, 0.0f, 1.0f
};


float color[4] = {
	1.0f, 0.76f, 0.0f, 1.0f
};

float bgcolor[4] = {
	0, 0, 0, 0
};

float scale[3] = {
	1.0f, 1.0f, 1.0f
};


bool drawUIElements = true;
bool drawShapes = true;

bool drawTriangle = false;
bool drawSquare = true;
bool drawCircle = false;

bool drawPyramid = false;
bool drawCube = false;
bool drawSphere = false;

bool rotate = false;
bool rotateRight = false;
bool rotateLeft = false;


float( *currentVertices )[12] = &squareVertices;


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
		char* message = (char*)_malloca( length * sizeof( char ) );
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

glm::mat4 proj = glm::ortho( 0.0f, 1280.0f, 0.0f, 1280.0f, -1.0f, 1.0f );
glm::mat4 view = glm::translate( glm::mat4( 1.0f ), glm::vec3( -100.0f, 0.0f, 0.0f ) );

int main( void )
{
	glEnable( GL_DEPTH_TEST );

	GLFWwindow* window;

	/* Initialize the library */
	if ( !glfwInit() )
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow( 1280, 1280, "Spinning Cube", NULL, NULL );
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

	glm::vec3 translation( 0.0f, 0.0f, 0.0f );
	/* Loop until the user closes the window */
	while ( !glfwWindowShouldClose( window ) )
	{
		/* Render here */
		glClearColor( bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3] );
		glClear( GL_COLOR_BUFFER_BIT );

		glm::mat4 model = glm::translate( glm::mat4( 1.0f ), translation);

		glm::mat4 mvp = proj * view * model;

		if ( drawSquare )
			glDrawArrays( GL_TRIANGLES, 0, 6 );

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if ( ImGui::IsKeyDown( ImGuiKey_T ) )
			drawUIElements = !drawUIElements;
		else if ( ImGui::IsKeyDown( ImGuiKey_F1 ) )
			drawUIElements = !drawUIElements;
		else if ( ImGui::IsKeyDown( ImGuiKey_Escape ) )
			exit( 0 );

		if ( drawUIElements )
		{
			ImGui::Begin( "Edit" );

			if ( ImGui::CollapsingHeader( "Properties" ) )
			{
				ImGui::ColorEdit4( "Object Color", color );
				ImGui::ColorEdit4( "Background Color", bgcolor );
				ImGui::SliderFloat2( "Translate", &translation.x, 0.0f, 1280.0f );
				ImGui::SliderFloat3( "Scale", scale, -5.0f, 5.0f );
			}

			ImGui::BeginChild( "Draw Shapes" );
			if ( ImGui::CollapsingHeader( "2D Shapes" ) )
			{
				ImGui::Checkbox( "Draw Triangle", &drawTriangle );
				ImGui::Checkbox( "Draw Square", &drawSquare );
				ImGui::Checkbox( "Draw Circle", &drawCircle );
			}
			if ( ImGui::CollapsingHeader( "3D Shapes" ) )
			{
				ImGui::Checkbox( "Draw Pyramid", &drawPyramid );
				ImGui::Checkbox( "Draw Cube", &drawCube );
				ImGui::Checkbox( "Draw Sphere", &drawSphere );
			}
			ImGui::EndChild();

			ImGui::BeginChild( "Rotation" );
			if ( ImGui::CollapsingHeader( "Rotation" ) )
			{
				ImGui::Checkbox( "Rotate", &rotate );
				ImGui::Checkbox( "Rotate Left", &rotateRight );
				ImGui::Checkbox( "Rotate Right", &rotateLeft );
			}
			ImGui::EndChild();

			ImGui::BeginChild( "Menus" );
			if ( ImGui::BeginMainMenuBar() )
			{
				if ( ImGui::BeginMenu( "File" ) )
				{
					if ( ImGui::MenuItem( "Exit" ) )
					{
						exit( 0 );
					}

					ImGui::EndMenu();
				}
				
				if ( ImGui::BeginMenu( "Help" ) )
				{
					ImGui::Text( "Press F1 or T For UI Toggling" );

					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
			ImGui::EndChild();

			ImGui::End();
		}


		glUseProgram( shader );
		glUniform4f( glGetUniformLocation( shader, "u_Color" ), color[0], color[1], color[2], color[3] );
		glUniform4f( glGetUniformLocation( shader, "position" ), position[0], position[1], position[2], position[3] );
		glUniformMatrix4fv( glGetUniformLocation( shader, "u_MVP" ), 1, GL_FALSE, &mvp[0][0] );

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
#include "glew.h"		// include GL Extension Wrangler

#include "glfw3.h"  // include GLFW helper library

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/constants.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cctype>
#include <sstream>
#include <gtx\rotate_vector.hpp>

using namespace std;

#define M_PI        3.14159265358979323846264338327950288   /* pi */
#define DEG_TO_RAD	M_PI/180.0f

GLFWwindow* window = 0x00;

GLuint shader_program = 0;

GLuint view_matrix_id = 0;
GLuint model_matrix_id = 0;
GLuint proj_matrix_id = 0;


///Transformations
glm::mat4 proj_matrix;
glm::mat4 view_matrix;
glm::mat4 model_matrix;

glm::mat4 up_matrix = {0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0};

enum ArrowKeys {none, left, right, up, down};
ArrowKeys arrowKey;

GLuint VBO, VAO, EBO;

GLfloat point_size = 3.0f;

// An array of 3 vectors which represents 3 vertices
GLfloat* g_vertex_buffer_data;

const GLuint WIDTH = 800, HEIGHT = 800;

int mode;

GLfloat* points_buffer;
int points_buffer_size;
GLfloat* lines_buffer;
int lines_buffer_size;
GLfloat* triangles_buffer;
int triangles_buffer_size;
int numStrips;
int numStripLength;

string vec3tostring(glm::vec3 vec)
{
	std::stringstream ss;
	ss << vec.x << ' ' << vec.y << ' ' << vec.z;
	return ss.str();
}

ifstream file;

///Handle the keyboard input
void keyPressed(GLFWwindow *_window, int key, int scancode, int action, int mods) {

	if (action == GLFW_RELEASE)
	{
		arrowKey = ArrowKeys::none;
	}
	else if(action == GLFW_PRESS)
	{
		switch (key) {
		case GLFW_KEY_LEFT:
			arrowKey = ArrowKeys::left;
			break;
		case GLFW_KEY_RIGHT:
			arrowKey = ArrowKeys::right;
			break;
		case GLFW_KEY_UP:
			arrowKey = ArrowKeys::up;
			break;
		case GLFW_KEY_DOWN:
			arrowKey = ArrowKeys::down;
			break;

		default: break;
		}
	}

	
	return;
}


bool initialize() {
	/// Initialize GL context and O/S window using the GLFW helper library
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	/// Create a window of size 640x480 and with title "Lecture 2: First Triangle"
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	window = glfwCreateWindow(800, 800, "COMP371: Assignment 1", NULL, NULL);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	///Register the keyboard callback function: keyPressed(...)
	glfwSetKeyCallback(window, keyPressed);

	glfwMakeContextCurrent(window);

	/// Initialize GLEW extension handler
	glewExperimental = GL_TRUE;	///Needed to get the latest version of OpenGL
	glewInit();

	/// Get the current OpenGL version
	const GLubyte* renderer = glGetString(GL_RENDERER); /// Get renderer string
	const GLubyte* version = glGetString(GL_VERSION); /// Version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	/// Enable the depth test i.e. draw a pixel if it's closer to the viewer
	glEnable(GL_DEPTH_TEST); /// Enable depth-testing
	glDepthFunc(GL_LESS);	/// The type of testing i.e. a smaller value as "closer"

	return true;
}

bool cleanUp() {
	glDisableVertexAttribArray(0);
	//Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// Close GL context and any other GLFW resources
	glfwTerminate();

	return true;
}

GLuint loadShaders(std::string vertex_shader_path, std::string fragment_shader_path) {
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_shader_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_shader_path.c_str());
		getchar();
		exit(-1);
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_shader_path.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_shader_path.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);

	glBindAttribLocation(ProgramID, 0, "in_Position");

	//appearing in the vertex shader.
	glBindAttribLocation(ProgramID, 1, "in_Color");

	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	//The three variables below hold the id of each of the variables in the shader
	//If you read the vertex shader file you'll see that the same variable names are used.
	view_matrix_id = glGetUniformLocation(ProgramID, "view_matrix");
	model_matrix_id = glGetUniformLocation(ProgramID, "model_matrix");
	proj_matrix_id = glGetUniformLocation(ProgramID, "proj_matrix");

	return ProgramID;
}


int main() {

	cout << "Enter the name of the file";

	string filename;
	//cin >> filename;

	//filename = "translational_shape.out";
	//filename = "translational_hollow_box.out";
	filename = "rotational_cup.out";
	//filename = "rotational_hat.out";

	file.open(filename, ios::in);

	if (!file.is_open())
	{
		cout << "File name is invalid";
		system("pause");
		return 1;
	}

	file >> mode;

	//Translational Sweep Surface Definition
	if (mode == 0)
	{
		vector<glm::vec3> profile;
		vector<glm::vec3> trajectory;
		vector<glm::vec3> points;

		int profileSize;
		file >> profileSize;

		// Collect points for the profile curve
		for (int i = 0; i < profileSize; i++)
		{
			GLfloat x, y, z;
			file >> x >> y >> z;
			profile.push_back(glm::vec3(x, y, z));
		}

		int tragectorySize;
		file >> tragectorySize;

		// Collect points for the trajectory curve
		for (int i = 0; i < tragectorySize; i++)
		{
			GLfloat x, y, z;
			file >> x >> y >> z;
			trajectory.push_back(glm::vec3(x, y, z));
		}

		glm::vec3 offset(trajectory[0]);

		// Move the trajectory curve to the origin
		for (unsigned i = 0; i < trajectory.size(); i++)
		{
			trajectory[i] -= offset;
		}

		// Calculate all the points
		for (unsigned i = 0; i < profile.size(); i++)
		{
			for (unsigned j = 0; j < trajectory.size(); j++)
			{
				points.push_back(glm::vec3(profile[i] + trajectory[j]));
			}
		}

		points_buffer_size = points.size() * 3;
		points_buffer = new GLfloat[points_buffer_size];

		cout << endl;

		// Covert all the point vectors into a single array
		for (unsigned i = 0; i < points.size(); i++)
		{
			points_buffer[i * 3] = points[i].x;
			points_buffer[i * 3 + 1] = points[i].y;
			points_buffer[i * 3 + 2] = points[i].z;

			// Debug output
			cout << vec3tostring(points[i]) << endl;
		}

		lines_buffer_size = (trajectory.size() - 1) * profile.size() * 6;
		lines_buffer = new GLfloat[lines_buffer_size];

		int ts = (trajectory.size() - 1) * 6;

		for (unsigned i = 0; i < profile.size(); i++)
		{
			for (unsigned j = 0; j < trajectory.size() - 1; j++)
			{
				lines_buffer[i * ts + j * 6] = profile[i].x + trajectory[j].x;
				lines_buffer[i * ts + j * 6 + 1] = profile[i].y + trajectory[j].y;
				lines_buffer[i * ts + j * 6 + 2] = profile[i].z + trajectory[j].z;

				lines_buffer[i * ts + j * 6 + 3] = profile[i].x + trajectory[j + 1].x;
				lines_buffer[i * ts + j * 6 + 4] = profile[i].y + trajectory[j + 1].y;
				lines_buffer[i * ts + j * 6 + 5] = profile[i].z + trajectory[j + 1].z;

				cout << lines_buffer[i * ts + j * 6] << " " << lines_buffer[i * ts + j * 6 + 1] << " " << lines_buffer[i * ts + j * 6 + 2] << " to " << lines_buffer[i * ts + j * 6 + 3] << " " << lines_buffer[i * ts + j * 6 + 4] << " " << lines_buffer[i * ts + j * 6 + 5] << " " << endl;
			}
		}
		
		//cout << "full"

		for (int i = 0; i < lines_buffer_size / 6; i++)
		{
			cout << lines_buffer[i + 0] << " " << lines_buffer[i + 1] << " " << lines_buffer[i + 2] << " to " << lines_buffer[i + 3] << " " << lines_buffer[i + 4] << " " << lines_buffer[i + 5] << endl;
		}

		vector<glm::vec3> trianglePoints;

		numStrips = profile.size() - 1;
		numStripLength = trajectory.size() * 2;

		cout << endl << " TRIANGLE DATA " << endl;

		for (unsigned i = 0; i < profile.size() - 1; i++)
		{
			for (unsigned j = 0; j < trajectory.size(); j++)
			{
				trianglePoints.push_back(glm::vec3(profile[i] + trajectory[j]));
				trianglePoints.push_back(glm::vec3(profile[i + 1] + trajectory[j]));

				cout << vec3tostring(trianglePoints[trianglePoints.size() - 2]) << " " << vec3tostring(trianglePoints[trianglePoints.size() - 1]) << endl;
			}
		}

		triangles_buffer_size = trianglePoints.size() * 3;
		triangles_buffer = new GLfloat[triangles_buffer_size];

		for (int i = 0; i < triangles_buffer_size / 3; i++)
		{
			triangles_buffer[i * 3] = trianglePoints[i].x;
			triangles_buffer[i * 3 + 1] = trianglePoints[i].y;
			triangles_buffer[i * 3 + 2] = trianglePoints[i].z;
		}
	}
	else
	{
		int spans = 10;
		//spans = mode;
		GLfloat angle = 2 * M_PI / spans;

		int profileSize;
		file >> profileSize;
		
		vector<glm::vec3> profile;

		for (int i = 0; i < profileSize; i++)
		{
			GLfloat x, y, z;
			file >> x >> y >> z;
			profile.push_back(glm::vec3(x, y, z));
		}

		vector<glm::vec3> points(profile);

		for (int i = 1; i < spans; i++)
		{
			for (unsigned j = 0; j < profile.size(); j++)
			{
				points.push_back(glm::rotateY(points[(i - 1)*profile.size() + j], angle));
			}
		}

		points_buffer_size = points.size();
		points_buffer = new GLfloat[points_buffer_size];

		// Covert all the point vectors into a single array
		for (unsigned i = 0; i < points.size(); i++)
		{
			points_buffer[i * 3] = points[i].x;
			points_buffer[i * 3 + 1] = points[i].y;
			points_buffer[i * 3 + 2] = points[i].z;
		}

		lines_buffer_size = (profile.size() - 1) * spans * 6;
		lines_buffer = new GLfloat[lines_buffer_size];

		int ts = (profile.size() - 1) * 6;

		for (unsigned i = 0; i < spans; i++)
		{
			for (unsigned j = 0; j < profile.size() - 1; j++)
			{

				//assert(i * ts + j * 6 > lines_buffer_size);
				if (i * ts + j * 6 >= lines_buffer_size)
				{
					cout << "foo";
				}

				lines_buffer[i * ts + j * 6] = points[i * profile.size() + j].x;
				lines_buffer[i * ts + j * 6 + 1] = points[i * profile.size() + j].y;
				lines_buffer[i * ts + j * 6 + 2] = points[i * profile.size() + j].z;

				lines_buffer[i * ts + j * 6 + 3] = points[i * profile.size() + j + 1].x;
				lines_buffer[i * ts + j * 6 + 4] = points[i * profile.size() + j + 1].y;
				lines_buffer[i * ts + j * 6 + 5] = points[i * profile.size() + j + 1].z;

				cout << lines_buffer[i * ts + j * 6] << " " << lines_buffer[i * ts + j * 6 + 1] << " " << lines_buffer[i * ts + j * 6 + 2] << " to " << lines_buffer[i * ts + j * 6 + 3] << " " << lines_buffer[i * ts + j * 6 + 4] << " " << lines_buffer[i * ts + j * 6 + 5] << " " << endl;
			}
		}

		vector<glm::vec3> trianglePoints;

		numStrips = spans;
		numStripLength = profile.size() * 2;

		cout << endl << " TRIANGLE DATA " << endl;

		for (unsigned i = 0; i < spans; i++)
		{
			for (unsigned j = 0; j < profile.size(); j++)
			{
				trianglePoints.push_back(glm::vec3(points[i*profile.size() + j]));

				unsigned neighbourPoint;
				if (i == spans - 1)
				{
					neighbourPoint = j;
				}
				else
				{
					neighbourPoint = i*profile.size() + j + profile.size();
				}

				trianglePoints.push_back(glm::vec3(points[neighbourPoint]));

				cout << vec3tostring(trianglePoints[trianglePoints.size() - 2]) << " " << vec3tostring(trianglePoints[trianglePoints.size() - 1]) << endl;
			}
		}

		triangles_buffer_size = trianglePoints.size() * 3;
		triangles_buffer = new GLfloat[triangles_buffer_size];

		for (int i = 0; i < triangles_buffer_size / 3; i++)
		{
			triangles_buffer[i * 3] = trianglePoints[i].x;
			triangles_buffer[i * 3 + 1] = trianglePoints[i].y;
			triangles_buffer[i * 3 + 2] = trianglePoints[i].z;
		}
	}


	int vertex_buffer_size = points_buffer_size + lines_buffer_size + triangles_buffer_size;
	int linesOffset = points_buffer_size;
	int trianglesOffset = points_buffer_size + lines_buffer_size;

	g_vertex_buffer_data = new GLfloat[vertex_buffer_size];

	for (int i = 0; i < points_buffer_size; i++)
	{
		g_vertex_buffer_data[i] = points_buffer[i];
	}

	for (int i = 0; i < lines_buffer_size; i++)
	{
		g_vertex_buffer_data[linesOffset + i] = lines_buffer[i];
	}

	for (int i = 0; i < triangles_buffer_size; i++)
	{
		g_vertex_buffer_data[trianglesOffset + i] = triangles_buffer[i];
	}

	initialize();


	///Load the shaders
	shader_program = loadShaders("COMP371_hw1.vs", "COMP371_hw1.fs");

	// This will identify our vertex buffer
	GLuint vertexbuffer;

	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);

	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data[0]) * vertex_buffer_size, g_vertex_buffer_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);


	view_matrix = glm::translate(view_matrix, glm::vec3(0.0f, 0.0f, -3.0f));

	while (!glfwWindowShouldClose(window)) {
		// wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
		glPointSize(point_size);

		glUseProgram(shader_program);

		//proj_matrix = glm::perspective(200.0f, 1.0f, -5.0f, 5.0f);

		proj_matrix = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
		
		

		switch (arrowKey)
		{
		case ArrowKeys::none:
			break;
		case ArrowKeys::left:
			model_matrix =  glm::rotate(model_matrix, 0.05f, glm::vec3(glm::vec4(0.0f, -1.0f, 0.0f, 0.0f) * model_matrix));
			break;
		case ArrowKeys::right:
			model_matrix = glm::rotate(model_matrix, 0.05f, glm::vec3(glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * model_matrix));
			break;
		case ArrowKeys::up:
			model_matrix = glm::rotate(model_matrix, 0.05f, glm::vec3(glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f) * model_matrix));
			break;
		case ArrowKeys::down:
			model_matrix = glm::rotate(model_matrix, 0.05f, glm::vec3(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) * model_matrix));
			break;
		default:
			break;
		}

		

		//Pass the values of the three matrices to the shaders
		glUniformMatrix4fv(proj_matrix_id, 1, GL_FALSE, glm::value_ptr(proj_matrix));
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(model_matrix));

		glBindVertexArray(VAO);

		//glDrawArrays(GL_POINTS, 0, points_buffer_size / 3);

		//glDrawArrays(GL_LINES, linesOffset, lines_buffer_size / 3);

		for (int i = 0; i < numStrips; i++)
		{
			//glDrawArrays(GL_LINE_STRIP, trianglesOffset + (i * numStripLength), numStripLength);
		}

		/*
		for (int i = 0; i < numStrips; i++)
		{
			glDrawArrays(GL_TRIANGLE_STRIP, i * numStripLength, numStripLength);
		};*/

		//glDrawArrays(GL_LINES, 0, lines_buffer_size / 3); // Starting from vertex 0; 3 vertices total -> 1 triangle

		glBindVertexArray(0);

		// update other events like input handling
		glfwPollEvents();
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
	}

	cleanUp();
	return 0;
}
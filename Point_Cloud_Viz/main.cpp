#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
#include <string>
#include <stdio.h>
#include <math.h>
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <cstdio>
//#include <ctime>

//#include <boost/spirit/home/x3.hpp>
//#include <boost/iostreams/device/mapped_file.hpp> 
#include "DebugTimer.h"
#include "bcdEncoder.h"
//#define MYPI 3.14159265357f

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void do_movement();
void reset();
GLuint loadShaderFromFile(const char* path, GLenum shaderType);
void readData(const char* fileName, std::vector<GLfloat> &vec);
int countLines(const char* fileName);
//void readData_mmap_spirit(const char* fileName, std::vector<GLfloat> &vec);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Starting positions
glm::vec3 cameraPosStart = glm::vec3(-45.0f, 15.0f, 5.0f);
glm::vec3 cameraFrontStart = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 cameraUpStart = glm::vec3(0.0f, 0.0f, 1.0f);
GLfloat yawStart = 0.0f;
GLfloat pitchStart = 0.0f;
GLfloat lastXStart = WIDTH / 2.0;
GLfloat lastYStart = HEIGHT / 2.0;
GLfloat fovStart = 50.0f;

// Camera vars
glm::vec3 cameraPos   = cameraPosStart;
glm::vec3 cameraFront = cameraFrontStart;
glm::vec3 cameraUp    = cameraUpStart;
GLfloat yaw = yawStart;
GLfloat pitch  = pitchStart;
GLfloat lastX  = lastXStart;
GLfloat lastY  = lastYStart;
GLfloat fov = fovStart;
bool keys[1024];

// Deltatime - normalizes between systems
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// Shader files
const char* vertFile = "..\\PointCloud.vert";
const char* fragFile = "..\\PointCloud.frag";

// INPUTS

//const char* inFile = "..\\backlot_every1000.xyz";
//const int numVertices = 148300;

//const char* inFile = "..\\backlot_every100.xyz";
//const int numVertices = 1483002;

//const char* inFile = "..\\backlot_every10.xyz";
//const int numVertices = 14830000;
//const int numVertices = 14830024;

const char* inFile = "..\\data\\riseBacklot_pointcloud.xyz";
const int numVertices = 148300241;

// Main Function
int main()
{   
	//DebugTimer::Begin("READ");

    // Instantiate the GLFW window
    glfwInit();
    
    // Set all required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for mac
    
    // Create a window object called 'LearnOpenGL'
    GLFWwindow* window = glfwCreateWindow(800, 600, "Point Cloud Visualization", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // Options
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    
    // Initialize GLEW to setup the OpenGL Function pointers
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // Define viewpoint dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height); // location of lower left corner of window
    
    // Create vertex shader object
    GLuint vertexShader;
    vertexShader = loadShaderFromFile(vertFile, GL_VERTEX_SHADER);
    //vertexShader = glCreateShader(GL_VERTEX_SHADER);
    
    // Attach shader source code to shader object + compile shader
    //glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    //glCompileShader(vertexShader);
    
    // Create fragment shader object
    GLuint fragmentShader;
    fragmentShader = loadShaderFromFile(fragFile, GL_FRAGMENT_SHADER);
    
    // Create a shader program object
    GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    
    // Attach compiled shaders to shader program object
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check if shader program failed
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // Delete shader objects once they've been linked to program object
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Define test point cloud data - 41 points
	/*
    GLfloat vertices[] = {
        1.68420000, 0.12080000, 9.66800000, 254, 254, 254,
        1.67520000, 0.05110000, 9.64720000, 251, 251, 251,
        1.65530000, -0.11230000, 9.67500000, 254, 254, 254,
        1.65810000, -0.11240000, 9.65540000, 254, 253, 254,
        1.65830000, -0.11340000, 9.65750000, 254, 253, 254,
        1.65690000, -0.11430000, 9.65090000, 254, 253, 254,
        1.65580000, -0.13080000, 9.66450000, 254, 254, 253,
        1.66460000, -0.11480000, 9.65970000, 238, 234, 254,
        1.65250000, -0.17290000, 9.66500000, 250, 251, 243,
        1.65090000, -0.17670000, 9.66140000, 254, 252, 254,
        1.65030000, -0.18370000, 9.66770000, 254, 254, 254,
        1.71850000, 0.16760000, 9.71490000, 65, 53, 117,
        1.71870000, 0.16660000, 9.71610000, 65, 54, 117,
        1.71880000, 0.16560000, 9.71670000, 65, 54, 117,
        1.71850000, 0.16460000, 9.71540000, 81, 68, 135,
        1.71850000, 0.16360000, 9.71510000, 81, 68, 135,
        1.71900000, 0.16270000, 9.71810000, 81, 68, 135,
        1.72020000, 0.15180000, 9.72720000, 78, 70, 117,
        1.72020000, 0.15080000, 9.72720000, 63, 50, 118,
        1.71980000, 0.14980000, 9.72540000, 62, 50, 117,
        1.71990000, 0.14880000, 9.72590000, 63, 50, 117,
        1.71990000, 0.14770000, 9.72630000, 71, 59, 124,
        1.72000000, 0.14580000, 9.72710000, 71, 59, 124,
        1.71990000, 0.14480000, 9.72710000, 70, 58, 124,
        1.71970000, 0.14380000, 9.72580000, 73, 61, 125,
        1.72000000, 0.13980000, 9.72880000, 63, 51, 112,
        1.72010000, 0.13870000, 9.72940000, 63, 51, 112,
        1.71130000, 0.14530000, 9.71150000, 122, 116, 153,
        1.71140000, 0.14430000, 9.71260000, 122, 116, 153,
        1.71150000, 0.14330000, 9.71340000, 122, 116, 153,
        1.71130000, 0.14230000, 9.71230000, 122, 116, 153,
        1.71210000, 0.14140000, 9.71680000, 122, 116, 153,
        1.71220000, 0.14040000, 9.71760000, 102, 92, 149,
        1.71230000, 0.13740000, 9.71910000, 102, 92, 149,
        1.71200000, 0.13640000, 9.71790000, 102, 92, 149,
        1.71180000, 0.13530000, 9.71700000, 102, 92, 149,
        1.71200000, 0.13420000, 9.71820000, 134, 125, 176,
        1.71250000, 0.13130000, 9.72200000, 135, 125, 176,
        1.71280000, 0.13030000, 9.72400000, 135, 125, 176,
        1.71270000, 0.12830000, 9.72410000, 82, 72, 120,
        1.71250000, 0.12730000, 9.72280000, 82, 72, 120
    };
    */
    //Count lines in input file
	//int numLines = countLines(inFile);
	//std:cout << "Lines: " << numLines << std::endl;


	/*
	GLfloat* vertices = NULL;
	
	vertices = new GLfloat[numVertices*6];
	
	std::ifstream source(inFile);
	GLfloat val;
	int counter = 0;
	if (source) {
		while (source >> val) {
			vertices[counter] = val;
		}
	}
	*/
	// Read input data
	//std::vector<GLfloat> vertices;
	//vertices.reserve(numLines * 6);
	//vertices = readData(inFile);

	// Read data as array
	/*
	GLfloat* vertices = NULL;
	//int numVertices = countLines(inFile);
	vertices = new GLfloat[numVertices*6];
	std::ifstream source(inFile);
	for (int i = 0; i < numVertices * 6; i++) {
		source >> vertices[i];
	}
	std::cout << vertices[0] << vertices[1] << vertices[2] << std::endl;
	DebugTimer::End("aaa");
	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
	std::cout << "Read Time: " << duration << std::endl;
	*/

	// Read data as vector
	
	bool splitArray = false;
	std::vector<GLfloat> vertices (1);
	try {
		//std::vector<GLfloat> vertices(numVertices * 6);
		vertices.resize(numVertices * 6);
		std::string s(inFile);
		std::string bFile = bcdEncoder::writeBinaryFile(s, numVertices*6);
		bcdEncoder::readBinaryFile(bFile, numVertices * 6, vertices); // read from binary rep
	}
	catch (const std::bad_alloc& e) {
		std::cout << "Allocation failed" << std::endl;
		splitArray = true;
	}
	
	// split up read operation for big data
	std::vector<int> splits;
	if (splitArray) {
		int splitElements = (numVertices * 6) / 10;
		int lastSplit = splitElements + ((numVertices * 6) % 10);

		for (int i = 0; i = 9; i++) {
			if (i == 9)
				splits.push_back(lastSplit);
			else
				splits.push_back(splitElements);
		}


	}
	//DebugTimer::End("READ");
	//std::vector<GLfloat> vertices;
	//vertices.reserve(numVertices);

	

	//Debug data load
	/*
	std::cout << vertices[0] << std::endl;
	std::cout << vertices[1] << std::endl;
	std::cout << vertices[2] << std::endl;
	std::cout << vertices[3] << std::endl;
	std::cout << vertices[4] << std::endl;
	std::cout << vertices[5] << std::endl;
	std::cout << vertices[148300 - 6] << std::endl;
	std::cout << vertices[148300 - 5] << std::endl;
	std::cout << vertices[148300 - 4] << std::endl;
	std::cout << vertices[148300 - 3] << std::endl;
	std::cout << vertices[148300 - 2] << std::endl;
	std::cout << vertices[148300 - 1] << std::endl;
	std::cout << vertices.size() << std::endl;
	*/

	//readData(inFile, vertices); //faster than mmap
	
	//readData_mmap_spirit(inFile, vertices); // read data w/ memory mapped file & spirit parser
	//duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
	//DebugTimer::End("aaa");
	//std::cout << "Read Time: " << duration << std::endl;
	
	// Generate a vertex buffer object - manages memory on GPU
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    // Bind Vertex array object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
	if (!splitArray) {
		glBufferData(GL_ARRAY_BUFFER, numVertices * 6 * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW); 
	}
	else {
		for (int i = 0; i = 9; i++) {
			vertices.resize(splits[i]);
			std::string s(inFile);
			std::string bFile = bcdEncoder::writeBinaryFile(s, splits[i]);
			bcdEncoder::readBinaryFile(bFile, splits[i], vertices); // read from binary rep

			glBufferSubData(GL_ARRAY_BUFFER, i * splits[i] * sizeof(GLfloat), splits[i] * sizeof(GLfloat), &vertices[0]);
		}
		
		
	}
	
	//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW); //vector
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);    
	//glBufferData(GL_ARRAY_BUFFER, numVertices * 6 * sizeof(GLfloat), vertices, GL_STATIC_DRAW); // heap array
	//delete [] vertices;
	//vertices = NULL;
	std::vector<GLfloat>().swap(vertices); // deallocate mem

    // Tell OpenGL how to interpret vertex data
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Unbind the VAO
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Game loop (so window doesn't close)
    while (!glfwWindowShouldClose(window))
    {
		glfwSwapInterval(0); //for timer
		DebugTimer::Begin("LOOP");
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // check if any events have been activated
        glfwPollEvents();
        do_movement();
        
        // Render
        // Clear the colorbuffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Every shader and rendering call after this will use this shader program obj
        glUseProgram(shaderProgram);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        
        // camera
        glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        //glm::mat4 projMatrix = glm::perspective(fov / 180.f * MYPI, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
		glm::mat4 projMatrix = glm::perspective(glm::radians(fov), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        glm::mat4 modelMatrix = glm::mat4();
        
        // Get uniform locations
        GLint modelLoc = glGetUniformLocation(shaderProgram, "ModelMatrix");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "ViewMatrix");
        GLint projLoc = glGetUniformLocation(shaderProgram, "ProjectionMatrix");

        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        
        // Draw with vertex array
        glBindVertexArray(VAO);
    
        //glDrawArrays(GL_POINTS, 0, 3);
        glDrawArrays(GL_POINTS, 0, numVertices);

        glBindVertexArray(0);
        
        glfwSwapBuffers(window);
		
		DebugTimer::End("LOOP");
    }
    
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    
    // Clean resources
    glfwTerminate();
    
    return 0;
}

// Moves/alters the camera positions based on user input
void do_movement()
{
    // Camera controls
    //GLfloat cameraSpeed = 0.08f;
    GLfloat cameraSpeed = 5.0f * deltaTime;
    if (keys[GLFW_KEY_UP])
        cameraPos += cameraSpeed * cameraFront;
    if (keys[GLFW_KEY_DOWN])
        cameraPos -= cameraSpeed * cameraFront;
    if (keys[GLFW_KEY_LEFT])
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (keys[GLFW_KEY_RIGHT])
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void reset()
{
	cameraPos = cameraPosStart;
	cameraFront = cameraFrontStart;
	cameraUp = cameraUpStart;
	yaw = yawStart;
	pitch = pitchStart;
	lastX = lastXStart;
	lastY = lastYStart;
	fov = fovStart;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		reset();
    
    if (key >= 0 && key < 1024)
    {
        if(action == GLFW_PRESS)
            keys[key] = true;
        else if(action == GLFW_RELEASE)
            keys[key] = false;
    }
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	GLint mb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1);

	if (mb) {
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		GLfloat xoffset = xpos - lastX;
		GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
		lastX = xpos;
		lastY = ypos;

		GLfloat sensitivity = 1.0;	// Change this value to your liking
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		pitch += yoffset;
		yaw -= xoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		front.z = sin(glm::radians(pitch));
		front.y = sin(glm::radians(yaw));
		cameraFront = glm::normalize(front);

		//Debug
		/*
		std::cout << "yoffset: " << yoffset << "; xoffset: " << xoffset << std::endl;
		std::cout << "pitch: " << pitch << "; yaw: " << yaw << std::endl;
		std::cout << "front-x: " << front.x << std::endl;
		std::cout << "front-y: " << front.y << std::endl;
		std::cout << "front-z: " << front.z << std::endl;
		*/
	}
	else {
		firstMouse = true;
	}

}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

    if (fov >= 1.0f && fov <= fovStart)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= fovStart)
        fov = fovStart;
}


GLuint loadShaderFromFile(const char* path, GLenum shaderType)
{
    //Open file
    GLuint shaderID = 0;
    std::string shaderString;
    std::ifstream sourceFile(path);
    
    //Source file loaded
    if (sourceFile)
    {
        //Get shader source
        shaderString.assign((std::istreambuf_iterator< char >(sourceFile)), std::istreambuf_iterator< char >());
        
        //Create shader ID
        shaderID = glCreateShader(shaderType);
        
        //Set shader source
        const GLchar* shaderSource = shaderString.c_str();
        glShaderSource(shaderID, 1, (const GLchar**)&shaderSource, NULL);
        
        //Compile shader source
        glCompileShader(shaderID);
        
        //Check shader for errors
        GLint shaderCompiled = GL_FALSE;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
        if (shaderCompiled != GL_TRUE)
        {
            printf("Unable to compile shader %d!\n\nSource:\n%s\n", shaderID, shaderSource);
            glDeleteShader(shaderID);
            shaderID = 0;
        }
    }
    else
    {
        printf("Unable to open file %s\n", path);
    }
    
    return shaderID;
}

/*
Reads space delimited file into 1-D vector.
*/
void readData(const char* fileName, std::vector<GLfloat> &vec)
{
	GLfloat val;
	std::ifstream source(fileName);

	if (source) {
		while (source >> val) {
			vec.push_back(val);
		}
	}
	else {
		std::cout << "Unable to open file " << fileName << std::endl;
	}
    
}

int countLines(const char* fileName)
{
	int numLines = 0;
	std::string line;
	std::ifstream source(fileName);

	while (std::getline(source, line))
		++numLines;

	return numLines;
}
/*
void readData_mmap_spirit(const char* fileName, std::vector<GLfloat> &vec)
{
	// memory map the input file
	boost::iostreams::mapped_file mmap(fileName, boost::iostreams::mapped_file::readonly);
	auto f = mmap.const_data();
	auto l = f + mmap.size();

	// parse with spirit x3
	namespace x3 = boost::spirit::x3;
	using namespace x3;
	bool ok = x3::phrase_parse(f, l, *x3::double_, x3::space, vec);

	// check success
	if (ok)
		std::cout << "parse success\n";
	else
		std::cerr << "parse failed: '" << std::string(f, l) << "'\n";
}
*/
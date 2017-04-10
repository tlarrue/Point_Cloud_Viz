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

#include "DebugTimer.h"
#include "bcdEncoder.h"


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void do_movement();
void reset();
GLuint loadShaderFromFile(const char* path, GLenum shaderType);

// Window dimensions
const GLuint WIDTH = 1600, HEIGHT = 1200;

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

// Input Data
//const char* inFile = "..\\data\\backlot_every1000.xyz";
//const int numVertices = 148300;

const char* inFile = "..\\data\\backlot_every100.xyz";
const int numVertices = 1483002;

//const char* inFile = "..\\data\\backlot_every10.xyz";
//const int numVertices = 14830024;

//const char* inFile = "..\\data\\riseBacklot_pointcloud.xyz";
//const int numVertices = 148300241;

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
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Point Cloud Visualization", nullptr, nullptr);
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
   
	// Read data as vector
	std::vector<GLfloat> vertices (1);
	try {
		float datasize = (numVertices * 6 * sizeof(GLfloat)) / 1000000000.f;
		std::cout << "Input data size: " << datasize << " gb" <<std::endl;
		vertices.resize(numVertices * 6);
		std::string s(inFile);
		std::string bFile = bcdEncoder::writeBinaryFile(s);
		bcdEncoder::readBinaryFile(bFile, numVertices * 6, vertices); // read from binary rep
	}
	catch (const std::bad_alloc& e) {
		std::cout << "Mem Allocation failed. Exiting." << std::endl;
		return -1;
	}
	//DebugTimer::End("READ");
	
	// Generate a vertex buffer object - manages memory on GPU
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    // Bind Vertex array object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, numVertices * 6 * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW); 

	//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW); //vector
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);   //const array 
	//glBufferData(GL_ARRAY_BUFFER, numVertices * 6 * sizeof(GLfloat), vertices, GL_STATIC_DRAW); //heap array
	//delete [] vertices; // heap array
	//vertices = NULL; // heap array
	std::vector<GLfloat>().swap(vertices); // deallocate mem - vector

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
		// Measure render time
		glfwSwapInterval(0); //vertical sync
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
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //black
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //bluish-grey
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Every shader and rendering call after this will use this shader program obj
        glUseProgram(shaderProgram);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        
        // camera
        glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
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
    
    // De-allocate resources
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

/*
Look around in direction of mouse when mouse button is clicked.
*/
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

		GLfloat sensitivity = 1.0;	// adjust
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		pitch += yoffset;
		yaw -= xoffset;

		// Make sure when pitch is out of bounds, screen doesn't get flipped
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		front.z = sin(glm::radians(pitch));
		front.y = sin(glm::radians(yaw));
		cameraFront = glm::normalize(front);

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
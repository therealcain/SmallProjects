// Author: Eviatar Mor, 05/10/2020
// Many classes and functions should be probably seperated to their own files,
// rather than making a big messy single file.
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/quaternion.hpp>
// #include <glm/gtx/quaternion.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <complex>
#include <sstream>
#include <cmath>

#include <stdio.h>
#include <stdlib.h>
// ------------------------------------------------------------------------

static constexpr unsigned int Width  = 800;
static constexpr unsigned int Height = 800;
// ------------------------------------------------------------------------

// Macro to print which line has an error
// because OpenGL error handling is quite bad.
#ifdef NDEBUG

    // X MACRO to define all of the OpenGL errors
    #define _LIST_OF_OPENGL_ERRORS           \
        X(GL_NO_ERROR)                       \
        X(GL_INVALID_VALUE)                  \
        X(GL_INVALID_OPERATION)              \
        X(GL_STACK_OVERFLOW)                 \
        X(GL_STACK_UNDERFLOW)                \
        X(GL_OUT_OF_MEMORY)                  \
        X(GL_INVALID_FRAMEBUFFER_OPERATION)  \
        X(GL_CONTEXT_LOST)                   \
        X(GL_TABLE_TOO_LARGE)

    // Returns a string of the X Macro Error
    static const char* _glStringError(const GLenum err)
    {
        switch (err)
        {
#           define X(name) case name: return #name;
                _LIST_OF_OPENGL_ERRORS
#           undef X
        }
        return "UNDEFINED";
    }

    // Fetching all of the errors and printing them
    static void _glPrintErrors(const char* func, const char* file, const int line) 
    {
        GLenum err = glGetError();
        while(err != GL_NO_ERROR)
        {
            std::cerr << "\nOpenGL ERROR:"
                         "\n  Func - " << func << 
                         "\n  File - " << file << 
                         "\n  Line - " << line << 
                         "\n  Type - " << _glStringError(err) 
                         << std::endl;
            err = glGetError();
        }
    }
    
#   define glLog(x)  x; _glPrintErrors(#x, __FILE__, __LINE__)  
#else
#   define glLog(x) x
#endif
// ------------------------------------------------------------------------

// NOT MINE - Modified
// https://github.com/opengl-tutorials/ogl/blob/master/common/shader.cpp
static GLuint LoadShaders(const std::string& VertexShaderCode, const std::string& FragmentShaderCode)
{
    // Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader Vertex Shader\n");
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader Fragment Shader\n");
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

// ------------------------------------------------------------------------

// Shaders
static const std::string VertexShader =
    "#version 330 core                          \n"
    "layout(location = 0) in vec3 modelSpace;   \n"
    "layout(location = 1) in vec3 vertexColor;  \n"
    "out vec3 fragmentColor;                    \n"
    "uniform mat4 MVP;                          \n"
    "void main(){	                            \n"
	"gl_Position =  MVP * vec4(modelSpace, 1);  \n"
	"fragmentColor = vertexColor;               \n"
    "}                                          \n";

static const std::string FragmentShader = 
    "#version 330 core          \n"
    "in vec3 fragmentColor;     \n"
    "out vec3 color;            \n"
    "void main() {              \n"
	"   color = fragmentColor;  \n"
    "}                          \n";
// ------------------------------------------------------------------------

static const GLfloat cube_vertex_data[] = { 
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f
};

static const GLfloat cube_color_data[] = { 
    0.1f,  0.1f,  0.1f,
    0.1f,  0.1f,  0.1f,
    0.1f,  0.1f,  0.1f,
    0.1f,  0.1f,  0.1f,
    0.2f,  0.2f,  0.2f,
    0.2f,  0.2f,  0.2f,
    0.2f,  0.2f,  0.2f,
    0.2f,  0.2f,  0.2f,
    0.3f,  0.3f,  0.3f,
    0.3f,  0.3f,  0.3f,
    0.3f,  0.3f,  0.3f,
    0.3f,  0.3f,  0.3f,
    0.4f,  0.4f,  0.4f,
    0.4f,  0.4f,  0.4f,
    0.4f,  0.4f,  0.4f,
    0.4f,  0.4f,  0.4f,
    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    0.6f,  0.6f,  0.6f,
    0.6f,  0.6f,  0.6f,
    0.6f,  0.6f,  0.6f,
    0.6f,  0.6f,  0.6f,
    0.7f,  0.7f,  0.7f,
    0.7f,  0.7f,  0.7f,
    0.7f,  0.7f,  0.7f,
    0.7f,  0.7f,  0.7f,
    0.8f,  0.8f,  0.8f,
    0.8f,  0.8f,  0.8f,
    0.8f,  0.8f,  0.8f,
    0.8f,  0.8f,  0.8f,
    0.9f,  0.9f,  0.9f,
    0.9f,  0.9f,  0.9f,
    0.9f,  0.9f,  0.9f,
    0.9f,  0.9f,  0.9f
};
// ------------------------------------------------------------------------

// I DID NOT COMPLETE THIS CLASS
// When i do rotation it's converting this coordinates to euler and transform
// them into the screen.
// this is definitely not right, but i don't care about performance or anything
// like that, i just want to see the difference between euler to quaternions.
struct Quaternion 
{
    float w, x, y, z;

    // https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#Quaternion_to_Euler_angles_conversion
    static Quaternion convertFromEuler(const float yaw, const float pitch, const float roll)
    {
        const float cy = cosf(yaw * 0.5f);
        const float sy = sinf(yaw * 0.5f);
        const float cp = cosf(pitch * 0.5f);
        const float sp = sinf(pitch * 0.5f);
        const float cr = cosf(roll * 0.5f);
        const float sr = sinf(roll * 0.5f);

        Quaternion q;
        q.w = cr * cp * cy + sr * sp * sy;
        q.x = sr * cp * cy - cr * sp * sy;
        q.y = cr * sp * cy + sr * cp * sy;
        q.z = cr * cp * sy - sr * sp * cy;

        return q;
    }
};
// ------------------------------------------------------------------------

// Rotation_matrix
// A Replacment would be using glm::rotate
struct EulerAngle
{
    float x, y, z;

    static void rotateX(const float theta, glm::mat4& model)
    {
        glm::mat4 xmat(0.f);
        xmat[0][0] = 1.f;

        xmat[1][1] = cosf(theta);
        xmat[1][2] = -sinf(theta);

        xmat[2][1] = sinf(theta);
        xmat[2][2] = cosf(theta);

        xmat[3][3] = 1.f;

        model *= xmat;
    }

    static void rotateY(const float theta, glm::mat4& model)
    {
        glm::mat4 ymat(0.f);

        ymat[0][0] = cosf(theta);
        ymat[0][2] = sinf(theta);

        ymat[2][0] = -sinf(theta);
        ymat[2][2] = cosf(theta);

        ymat[1][1] = 1.f;
        ymat[3][3] = 1.f;

        model *= ymat;
    }

    static void rotateZ(const float theta, glm::mat4& model)
    {
        glm::mat4 zmat(0.f);

        zmat[0][0] = cosf(theta);
        zmat[0][1] = -sinf(theta);

        zmat[1][0] = sinf(theta);
        zmat[1][1] = cosf(theta);

        zmat[2][2] = 1.f;
        zmat[3][3] = 1.f;

        model *= zmat;
    }

    // https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#Quaternion_to_Euler_angles_conversion
    static EulerAngle convertFromQuaternion(const Quaternion& q)
    {
        EulerAngle ea;

        float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
        float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
        ea.x = atan2f(sinr_cosp, cosr_cosp);

        float sinp = 2 * (q.w * q.y - q.z * q.x);
        if(fabs(sinp) >= 1)
            ea.y = copysignf(M_PI / 2, sinp);
        else
            ea.y = asinf(sinp);

        float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
        float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
        ea.z = atan2f(siny_cosp, cosy_cosp);

        return ea;
    }
};
// ------------------------------------------------------------------------

int main(int, char**)
{
    // Setup window
    if (!glfwInit())
    {
        glfwTerminate();
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return EXIT_FAILURE;
    }

    // GL 3.2 + GLSL 1.5
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(Width, Height, "Rotational Methods", nullptr, nullptr);
    
    if (window == nullptr)
    {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window!" << std::endl;
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync

    // Initialize OpenGL loader
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        return EXIT_FAILURE;
    }

    glLog(glEnable(GL_CULL_FACE));
    glLog(glClearColor(0.1f, 0.0f, 0.4f, 0.0f));

    GLuint VertexArrayID;
	glLog(glGenVertexArrays(1, &VertexArrayID));
	glLog(glBindVertexArray(VertexArrayID));

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    [[maybe_unused]] ImGuiIO& io = ImGui::GetIO();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // MVP
    const glm::mat4 Projection = glm::perspective(glm::radians(60.0f), (float)Width / Height, 0.1f, 150.0f);
    const glm::mat4 View       = glm::lookAt(glm::vec3(4,3,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 Model(1.0f);

    // Shader
    GLuint programID = LoadShaders(VertexShader, FragmentShader);
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    // Cube Vertices
    GLuint vertexbuffer;
	glLog(glGenBuffers(1, &vertexbuffer));
	glLog(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
	glLog(glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_data), cube_vertex_data, GL_STATIC_DRAW));

    // Cube Color
	GLuint colorbuffer;
	glLog(glGenBuffers(1, &colorbuffer));
	glLog(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
	glLog(glBufferData(GL_ARRAY_BUFFER, sizeof(cube_color_data), cube_color_data, GL_STATIC_DRAW));

    // Enabling the vertex attribute
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));

    // Enabling the color attribute
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Handle events
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Rotation Window");

            // Angle Measurment
            static bool as_radians = true;
            if(ImGui::Button("Radians")) as_radians = true;
            ImGui::SameLine();
            if(ImGui::Button("Degrees")) as_radians = false;

            if(as_radians) ImGui::Text("Angle Measure: Radians");
            else           ImGui::Text("Angle Measure: Degrees");

            ImGui::NewLine();
            
            // Euler angles
            ImGui::Text("Euler Angle:");
            
            static float euler[3];
            static float quat[4];
            float euler_rad[3];
            euler_rad[0] = as_radians ? euler[0] : glm::radians(euler[0]);
            euler_rad[1] = as_radians ? euler[1] : glm::radians(euler[1]); 
            euler_rad[2] = as_radians ? euler[2] : glm::radians(euler[2]);

            ImGui::Text("Roll(X), Pitch(Y), Yaw(Z)");
            ImGui::SliderFloat3("Euler", euler, as_radians ? -3.141f : 0.f, as_radians ? 3.141f : 360.f);
            if(ImGui::IsItemActive())
            {
                Model = glm::mat4(1.f);

                Quaternion q = Quaternion::convertFromEuler(euler_rad[0], euler_rad[1], euler_rad[2]);
                quat[0] = q.w; quat[1] = q.x; quat[2] = q.y; quat[3] = q.z;

                EulerAngle::rotateX(euler_rad[0], Model);
                EulerAngle::rotateY(euler_rad[1], Model);
                EulerAngle::rotateZ(euler_rad[2], Model);
            }

            ImGui::NewLine();
            ImGui::Text("w, xI, yJ, zK = -1");
            ImGui::SliderFloat4("Quaternion", quat, -1.f, 1.f);
            if(ImGui::IsItemActive())
            {
                Model = glm::mat4(1.f);
                
                Quaternion quter;
                quter.w = quat[0]; quter.x = quat[1]; quter.y = quat[2]; quter.z = quat[3];

                auto converted_euler = EulerAngle::convertFromQuaternion(quter);
                euler[0] = converted_euler.x;
                euler[1] = converted_euler.y;
                euler[2] = converted_euler.z;

                EulerAngle::rotateX(euler[0], Model);
                EulerAngle::rotateY(euler[1], Model);
                EulerAngle::rotateZ(euler[2], Model);
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        
        // Clears Buffers
        glLog(glClear(GL_COLOR_BUFFER_BIT));

        // Using the current shader
		glUseProgram(programID);

        glm::mat4 MVP = Projection * View * Model;  

        // Passing the transformation matrix
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // Cube has 6 faces each face has 2 triangles meaning 6 * 2 = 12
		glDrawArrays(GL_TRIANGLES, 0, 12*3); 

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap GPU Buffers
        glfwSwapBuffers(window);
    }

    // Disabling the vertex and color attributes 
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// Author: Eviatar Mor, 03/10/2020

#define GLEW_STATIC
#include <GL/glew.h>

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <iostream>
#include <random>
#include <memory>
#include <vector>
#include <iterator>
#include <algorithm>

#include <cmath>
#include <ctime>
#include <cstring>
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

// Canvas Dimensions
constexpr float Width  = 800;
constexpr float Height = 800;

// PI
#ifdef M_PI
constexpr float Pi = M_PI;
#else
constexpr float Pi = 3.1415926f;
#endif
// ------------------------------------------------------------------------

// Simply generate a random number between min and max.
template<typename T>
static T random(T min, T max) 
{
    // There is a problem with the MinGW compiler.
#if !defined(__MINGW32__)
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(min, max);

	return static_cast<T>(dis(gen));
#else
    float scale = rand() / static_cast<float>(RAND_MAX);
    return min + scale * (max - min);
#endif
}
// ------------------------------------------------------------------------

// Converting a vector from one type to another.
// For example: Vector2<float> ---> Vector2<unsigned int>
template<typename T, typename U>
static inline sf::Vector2<T> vector_cast(const sf::Vector2<U> vec) {
    return sf::Vector2<T>(static_cast<T>(vec.x), static_cast<T>(vec.y));
}
// ------------------------------------------------------------------------

// Returns the orthogonal projection matrix
// Probably using an OpenGL mathematics library (glm) is way better.
// https://en.wikipedia.org/wiki/Orthographic_projection
static sf::Glsl::Mat4 get_projection() 
{
    // Boundaries
    const float Left   = 0.f;
    const float Right  = Width;
    const float Top    = 0.f;
    const float Bottom = Height;

    // Matrix 4x4
    float mat[4][4] = {
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}
    };
    
    // Orthographic projection matrix
    mat[0][0] = 2.f / (Right - Left);
    mat[1][1] = 2.f / (Top - Bottom);
    mat[2][2] = -1.f;
    mat[3][0] = -(Right + Left) / (Right - Left);
    mat[3][1] = -(Top + Bottom) / (Top - Bottom);

    return sf::Glsl::Mat4(&mat[0][0]);
}
// ------------------------------------------------------------------------

// Default Shaders
static const std::string VertexShader = 
    "#version 330 core                          \n"
    "layout(location = 0) in vec2 vert;         \n"
    "layout(location = 1) in vec3 color;        \n"
    "out vec3 fragment_color;                   \n"
    "uniform mat4 proj;                         \n"
    "void main()                                \n"
    "{                                          \n"
    "   gl_Position = proj * vec4(vert, 0, 1);  \n"
    "   fragment_color = color;                 \n"
    "}                                          \n";

static const std::string FragmentShader =
    "#version 330 core           \n"
    "in vec3 fragment_color;     \n"
    "out vec3 color;             \n"
    "void main()                 \n"
    "{                           \n"
    "   color = fragment_color;  \n"
    "}                           \n";
// ------------------------------------------------------------------------

class ObjectsContainer
{
public:
    ObjectsContainer(const unsigned int size)
    {
        // Allocate memory for the vertex buffer
        glLog(glGenBuffers(1, &vertex_buffer));
        glLog(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer));
        glLog(glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), nullptr, GL_DYNAMIC_DRAW));

        // Allocate memory for the color buffer
        glLog(glGenBuffers(1, &color_buffer));
        glLog(glBindBuffer(GL_ARRAY_BUFFER, color_buffer));
        glLog(glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), nullptr, GL_DYNAMIC_DRAW));
    }

    void append_vertices(const void* data, const unsigned int size)
    {
        // Operand on the vertex buffer
        glLog(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer));

        // Enabling Vertices - Location 0
        glLog(glEnableVertexAttribArray(0));

#ifdef NDEBUG
        std::cout << "Vertex Offset: " << vertex_offset << std::endl;
#endif

        if(should_resize_buffer(vertex_offset, size))
        {
            resize_buffer();
        }

        // "Pushing" the data to the GPU
        glLog(glBufferSubData(GL_ARRAY_BUFFER, vertex_offset, size, data));

        // Points to the vertex
        glLog(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0)));

        // Pushing vertices
        shape_start.push_back(vertex_offset / sizeof(sf::Vector2f));

        // Pushing the segments
        shape_segments.push_back(size / sizeof(sf::Vector2f));

        // Setting up the next variables
        vertex_offset += size;
        shapes_counter++;
    }

    void append_colors(const void* data, const unsigned int size)
    {
        // Operate on the color buffer
        glLog(glBindBuffer(GL_ARRAY_BUFFER, color_buffer));

        // Enabling Colors - Location 1
        glLog(glEnableVertexAttribArray(1));

#ifdef NDEBUG
        std::cout << "Color Offset:" << color_offset << std::endl;
#endif

        if(should_resize_buffer(color_offset, size))
        {
            resize_buffer();
        }

        // "Pushing" the data to the GPU
        glLog(glBufferSubData(GL_ARRAY_BUFFER, color_offset, size, data));

        // Points to the Color
        glLog(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0)));

        // Setting up the next variable
        color_offset += size;
    }

    void draw(const unsigned int Mode)
    {
        if(shapes_counter > 0)
        {
            // Draws
            glLog(glMultiDrawArrays(Mode, shape_start.data(), 
                shape_segments.data(), shapes_counter));
        }
    } 

    void clear()
    {
        // Resetting variables
        vertex_offset  = 0;
        color_offset   = 0;
        shapes_counter = 0;

        // Clearing vectors
        shape_start.clear();
        shape_segments.clear();

        {
            // Operate on the vertex buffer
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

            // Getting the buffer size
            GLint size = 0;
            glLog(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size));

            // Overwriting the buffer memory
            glLog(glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
        }

        {
            // Operate on the color buffer
            glBindBuffer(GL_ARRAY_BUFFER, color_buffer);

            // Getting the buffer size
            GLint size = 0;
            glLog(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size));

            // Overwriting the buffer memory
            glLog(glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
        }
    }

    ~ObjectsContainer()
    {
        // Disabling Vertices
        glLog(glDisableVertexAttribArray(0));
        // Disabling Colors
        glLog(glDisableVertexAttribArray(1));

        // Cleanup the buffers
        glLog(glDeleteBuffers(1, &vertex_buffer));
        glLog(glDeleteBuffers(1, &color_buffer));
    }

private:
    static bool should_resize_buffer(const unsigned int offset, const unsigned int add_bytes)
    {
        // Getting the buffer size
        GLint size = 0;
        glLog(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size));

        // If there is not enough room for the buffer.
        // ask for resize.
        return offset + add_bytes >= size;
    }

    static void resize_buffer()
    {
        // Getting the buffer size
        GLint size = 0;
        glLog(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size));

        // Copy all of the GPU data into the RAM
        const void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
        std::vector<uint8_t> copied_data(size);
        memcpy(copied_data.data(), ptr, size);
        glLog(glUnmapBuffer(GL_ARRAY_BUFFER));

        // Uploaded the data and resized the GPU buffer
        glLog(glBufferData(GL_ARRAY_BUFFER, size * 2, copied_data.data(), GL_DYNAMIC_DRAW));
    }

private:
    GLuint vertex_buffer, color_buffer;
    unsigned int vertex_offset  = 0;
    unsigned int color_offset   = 0;
    unsigned int shapes_counter = 0;

    std::vector<GLint>   shape_start;
    std::vector<GLsizei> shape_segments;
};
// ------------------------------------------------------------------------

class Polygon
{
public:
    Polygon(ObjectsContainer& cont, const float radius, const unsigned int num_segments, const sf::Vector2f position, const sf::Color color)
    {
        // Points are never going to be over num_segments
        points.reserve(num_segments);

        // How many segments does this polygon have
        for(float i = 0; i < num_segments; i += 1.f)
        {
            // How much to rotate?
            const float theta = 2.f * Pi * i / (float)num_segments;

            // Rotate along the x-axis and y-axis
            const float x = radius * cosf(theta);
            const float y = radius * sinf(theta);

            // Pushing the data and positioning it correctly
            points.push_back(sf::Vector2f(x, y) + position);
        }
 
        // Pushing the points
        cont.append_vertices(static_cast<void*>(points.data()), num_segments * sizeof(sf::Vector2f));

        // Setting the color
        float clr[3];
        clr[0] = (float)color.r / 255.f; // Red
        clr[1] = (float)color.g / 255.f; // Green
        clr[2] = (float)color.b / 255.f; // Blue

        // Pushing the color
        std::vector<float> indices_color;
        for(size_t i = 0; i < num_segments; i++)
        {
            for(const auto c : clr)
            {
                indices_color.push_back(c);
            }
        }
        cont.append_colors(indices_color.data(), indices_color.size() * sizeof(float));
    }

    const auto& get_points()   const { return points; }

private:
    std::vector<sf::Vector2f> points;
};
// ------------------------------------------------------------------------

int main()
{
#ifdef __MINGW32__
    srand(time(NULL));
#endif

    // Creates a window with AA
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    sf::Window window(sf::VideoMode(Width, Height), "Raycast 2D Test", sf::Style::Default, settings);

    // Initializing OpenGL
    glewExperimental = true;
    if(glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW\n";
        return EXIT_FAILURE;
    }

    // Creates a default shader
    sf::Shader shader;
    if(!shader.loadFromMemory(VertexShader, FragmentShader))
    {
        std::cerr << "Failed to create a shader!\n";
        return EXIT_FAILURE;
    }

    // Set background to white
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    std::vector<Polygon> shapes;

    // Binding the current shader
    sf::Shader::bind(&shader);

    // Projecting the coordinates
    shader.setUniform("proj", get_projection());

    // Objects Scene
    ObjectsContainer shapes_cont(1000);

    // The main loop - ends as soon as the window is closed
    bool running = true;
    while (running)
    {
        const auto MousePos = sf::Mouse::getPosition(window);

        // Event processing
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Request for closing the window
            if (event.type == sf::Event::Closed)
            {
                running = false;
            }

            // Adjust the viewport when the window has been resized            
            else if(event.type == sf::Event::Resized)
            {
                glViewport(0, 0, event.size.width, event.size.height);
            }

            // When button has been pressed create a random object at mouse position
            else if(event.type == sf::Event::MouseButtonPressed)
            {
                shapes.emplace_back(shapes_cont, random(10, 130), random(3, 8), 
                    vector_cast<float>(MousePos), sf::Color(random(0, 255), 
                        random(0, 255), random(0, 255)));
            }
        }
        
        // Clears the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw shapes
        shapes_cont.draw(GL_TRIANGLE_FAN);

        // Displaying everything to the screen.
        window.display();
        
    }
}

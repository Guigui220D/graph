#include "graph.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

static const char * fragment_shader_source = NULL; // loaded by init_resources()
static const char * vertex_shader_source = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
    "}\0"
;

static GLfloat quad_vertices[] = {
    -1.f, 1.f,
    1.f, 1.f,
    1.f, -1.f,
    -1.f, -1.f,
};

static GLuint quad_elements[] = {
    0, 1, 2,
    2, 3, 0
};

float upperBound(const float * samples, size_t size) {
    if (size == 0)
        return 0.f;
    float max = samples[0];
    for (int i = 1; i < size; i++) {
        if (samples[i] > max)
            max = samples[i];
    }
    return max;
}

float lowerBound(const float * samples, size_t size) {
    if (size == 0)
        return 0.f;
    float min = samples[0];
    for (int i = 1; i < size; i++) {
        if (samples[i] < min)
            min = samples[i];
    }
    return min;
}

float gradPeriod(float span) {
    float b10 = pow(10, floorf(log10f(span)));
    if (span / b10 <= 3)
        b10 /= 2;
    return b10;
}

int inog_init_resources() 
{
    glfwInit();

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    {
        char* buffer = 0;
        long length;
        FILE* f = fopen("graph_shader.fs", "rb");

        if (!f) {
            printf("Could not load shader file\n");
            return -1;
        }
        
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length + 1);
        if (!buffer) {
            printf("Could not load shader file\n");
            return -1;
        }
        fread(buffer, 1, length, f);
        fclose(f);
        buffer[length] = '\0';

        fragment_shader_source = buffer;
    }
    return 0;
}

void inog_deinit_resources() 
{
    glfwTerminate();
    free((void*)fragment_shader_source);
}

void* window_thread_function(void* arg) 
{
    GraphContext* context = (GraphContext*)arg;

    float upper = upperBound(context->data, context->size);
    float lower = lowerBound(context->data, context->size);
    upper += (upper - lower) / 10;
    lower += (lower - upper) / 10;
    float grad = gradPeriod(upper - lower);

    int success;
    char infoLog[512];
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Graph window", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window");
        return NULL;
    }
    glfwMakeContextCurrent(window);

    // TODO: figure out if that works
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

    glViewport(0, 0, 800, 600);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_elements), quad_elements, GL_STATIC_DRAW);

    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("Failed to compile vertex shader:\n%s\n", infoLog);
        return NULL;
    }

    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("Failed to compile fragment shader:\n%s\n", infoLog);
        return NULL;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Failed to link shader program:\n%s\n", infoLog);
        return NULL;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderProgram);

    glUniform2f(glGetUniformLocation(shaderProgram, "resolution"), 800, 600);
    glUniform1f(glGetUniformLocation(shaderProgram, "upperLimit"), upper);
    glUniform1f(glGetUniformLocation(shaderProgram, "lowerLimit"), lower);
    glUniform1f(glGetUniformLocation(shaderProgram, "gradPeriod"), grad);
    //TODO: modify shader to tolerate more than 1000 samples
    if (context->size > 1000)
        context->size = 1000;
    glUniform1fv(glGetUniformLocation(shaderProgram, "data"), context->size, context->data);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

    while(!glfwWindowShouldClose(window) && !context->must_stop)
    {
        glClearColor(1.f, 0.f, 1.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();    
    }
    return NULL;
}

GraphContext* inog_make_graph(const float* data, size_t size) 
{
    // TODO: dealloc on error
    GraphContext* context = (GraphContext*)malloc(sizeof(GraphContext));

    context->must_stop = 0;
    context->data = (float*)malloc(size);
    for (unsigned int i = 0; i < size; i++)
        context->data[i] = data[i];
    context->size = size;

    pthread_create(&context->thread, NULL, window_thread_function, (void*)context);

    return context;
}

void inog_destroy_graph(GraphContext* context) {
    context->must_stop = 1;
    pthread_join(context->thread, NULL);
    free(context->data);
    free(context);
}
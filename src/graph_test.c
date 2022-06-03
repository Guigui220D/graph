#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height); 

volatile unsigned int shaderProgram = 0;
unsigned int resolutionUniform;

void testFn(float * buffer, size_t size) {
    for (int i = 0; i < size; i++) {
        float x = (float)i / 1000.f;
        x *= 10;
        //buffer[i] = cosf(x * x) * x * x;
        buffer[i] = (i / 50) % 3;
    }
}

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

int main(int argc, char* argv[]) 
{
    const size_t SIZE = 1000;
    float samples[SIZE];

    testFn(samples, SIZE);
    float upper = upperBound(samples, SIZE);
    float lower = lowerBound(samples, SIZE);
    upper += (upper - lower) / 10;
    lower += (lower - upper) / 10;
    float grad = gradPeriod(upper - lower);

    printf("Upper: %f, Lower: %f\n", upper, lower);
    printf("Grad: %f\n", grad);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Graph window", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD");
        return -1;
    }  

    glViewport(0, 0, 800, 600);

    // Create Vertex Array Object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create a Vertex Buffer Object and copy the vertex data to it
    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {
        -1.f,  1.f,
         1.f,  1.f,
         1.f, -1.f,
        -1.f, -1.f,
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create an element array
    GLuint ebo;
    glGenBuffers(1, &ebo);

    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    const char *vertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
    "}\0"
    ;
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        return -1;
    }

    char * buffer = 0;
    long length;
    FILE * f = fopen("graph_shader.fs", "rb");

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

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &buffer, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader); 

    free(buffer);

    glUseProgram(shaderProgram);

    resolutionUniform = glGetUniformLocation(shaderProgram, "resolution");
    glUniform2f(resolutionUniform, 800, 600);
    glUniform1f(glGetUniformLocation(shaderProgram, "upperLimit"), upper);
    glUniform1f(glGetUniformLocation(shaderProgram, "lowerLimit"), lower);
    glUniform1f(glGetUniformLocation(shaderProgram, "gradPeriod"), grad);
    glUniform1fv(glGetUniformLocation(shaderProgram, "data"), 1000, samples);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

    while(!glfwWindowShouldClose(window))
    {
        glClearColor(1.f, 0.f, 1.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    shaderProgram = 0;

    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
    glViewport(0, 0, width, height);

    if (shaderProgram != 0 && resolutionUniform != 0) {
        glUniform2f(resolutionUniform, width, height);
    }
} 
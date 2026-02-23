#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstdint>

// #include "definitions.hpp"
#include "help.hpp"

namespace GL {
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
};
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>

// Function to check shader compilation errors
bool checkShaderCompilation(GL::GLuint shader, const char* type) {
    GL::GLint success;
    GL::GLchar infoLog[1024];
    GL::glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GL::glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cerr << "Shader compilation error (" << type << "): " << infoLog << std::endl;
        return false;
    }
    return true;
}

// Function to check program linking errors
bool checkProgramLinking(GL::GLuint program) {
    GL::GLint success;
    GL::GLchar infoLog[1024];
    GL::glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GL::glGetProgramInfoLog(program, 1024, NULL, infoLog);
        std::cerr << "Program linking error: " << infoLog << std::endl;
        return false;
    }
    return true;
}

float cube_vertexes[] = {
    // Positions          // Normals
    // Front
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

    // Back
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    // Left
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,

    // Right
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,

    // Bottom
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    // Top
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};


glm::vec3 color = {0.5, 0.5, 0.8};

glm::vec3 camera_position = {-1, 1, -1};
glm::vec3 camera_direction = {1, -1, 1};

glm::vec3 lighsource_position = {2, 1.5, -2};
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
float ambientStrength = 0.2f;
float specularStrength = 0.5f;
int shininess = 32;

float rotation = 0;

int main(int argc, char** argv) {
    (void)(argc); (void)(argv);

    if (!GL::glfwInit()) {
        std::perror("GLFW initialization");
        return -1;
    }
    
    GL::GLFWwindow* window = GL::glfwCreateWindow(800, 600, "Window", NULL, NULL);

    if (!window) {
        std::perror("GLFW window creation");
        return -1;
    }

    GL::glfwMakeContextCurrent(window);
    GL::glViewport(0, 0, 800, 600);
    GL::glfwSwapInterval(1);



    GL::glewExperimental = GL_TRUE;
    if (GL::glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    GL::glEnable(GL_DEPTH_TEST);




    
    std::string vertexShaderSource = read_entire_file("./shaders/vertex.vert");
    const char* vertexShaderSource_cstr = vertexShaderSource.c_str();
    GL::GLuint vertexShader = GL::glCreateShader(GL_VERTEX_SHADER);
    GL::glShaderSource(vertexShader, 1, &vertexShaderSource_cstr, NULL);
    GL::glCompileShader(vertexShader);
    if (!checkShaderCompilation(vertexShader, "VERTEX")) return 1;
    
    std::string fragmentShaderSource = read_entire_file("./shaders/fragment.frag");
    const char* fragmentShaderSource_cstr = fragmentShaderSource.c_str();
    GL::GLuint fragmentShader = GL::glCreateShader(GL_FRAGMENT_SHADER);
    GL::glShaderSource(fragmentShader, 1, &fragmentShaderSource_cstr, NULL);
    GL::glCompileShader(fragmentShader);
    if (!checkShaderCompilation(fragmentShader, "FRAGMENT")) return 1;

    GL::GLuint shaderProgram = GL::glCreateProgram();
    GL::glAttachShader(shaderProgram, vertexShader);
    GL::glAttachShader(shaderProgram, fragmentShader);
    GL::glLinkProgram(shaderProgram);
    if (!checkProgramLinking(shaderProgram)) return 1;

    GL::glDeleteShader(vertexShader);
    GL::glDeleteShader(fragmentShader);





    
    GL::GLuint VAO, VBO;
    GL::glGenVertexArrays(1, &VAO);
    GL::glGenBuffers(1, &VBO);

    GL::glBindVertexArray(VAO);

    GL::glBindBuffer(GL_ARRAY_BUFFER, VBO);
    GL::glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertexes), cube_vertexes, GL_STATIC_DRAW);


    
    GL::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    GL::glEnableVertexAttribArray(0);

    GL::glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    GL::glEnableVertexAttribArray(1);

    GL::glBindBuffer(GL_ARRAY_BUFFER, 0);
    GL::glBindVertexArray(0);


    while (!GL::glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        GL::glClearColor(0.1, 0.1, 0.1, 1);
        GL::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GL::glUseProgram(shaderProgram);

        rotation += 0.002;

        lighsource_position = glm::vec3(
            2 * cos(rotation*3),
            1,
            -2 * sin(rotation*3)
        );


        glm::mat4 model(1);
        
        // model = glm::rotate(model, rotation, glm::vec3(0.5f, 1.0f, 0.3f));

        glm::mat4 view = glm::lookAt(camera_position, camera_position + camera_direction, glm::vec3(0,1,0));

        glm::mat4 projection = glm::perspective(45.0f, 800.0f/600.0f, 0.1f, 100.0f);
        
        GL::glUniformMatrix4fv(GL::glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        GL::glUniformMatrix4fv(GL::glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        GL::glUniformMatrix4fv(GL::glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "lightPosition"), 1, glm::value_ptr(lighsource_position));
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
        GL::glUniform1f(GL::glGetUniformLocation(shaderProgram, "ambientStrength"), ambientStrength);
        GL::glUniform1f(GL::glGetUniformLocation(shaderProgram, "specularStrength"), specularStrength);
        GL::glUniform1i(GL::glGetUniformLocation(shaderProgram, "shininess"), shininess);
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera_position));

        GL::glBindVertexArray(VAO);
        GL::glDrawArrays(GL_TRIANGLES, 0, 36);
        GL::glBindVertexArray(0);


        GL::glfwSwapBuffers(window);
        GL::glfwPollEvents();
    }


    GL::glfwDestroyWindow(window);
    GL::glfwTerminate();

    return 0;
}
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

enum {
    FRONT = 0,
    BACK,
    RIGHT,
    LEFT,
    BOTTOM,
    TOP,

    COUNT
};

float cube_vertexes[6][36] = {
    // Positions          // Normals
    // Front
    {-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,},

    // Back
    {-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,},

    // Left
    {-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,},

    // Right
    { 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,},

    // Bottom
    {-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,},

    // Top
    {-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f}
};

std::vector< float > generate_sphere(glm::vec3 center, float radius, int smooth) {
    std::vector< float > result;

    float delta_phi = 2*glm::pi<float>() / smooth;
    float delta_theta = 2*glm::pi<float>() / smooth;

    float phi = 0;
    float theta = 0;
    for (int i=0; i<smooth + 1; i++) {
        for (int j=0; j<smooth + 1; j++) {
            glm::vec3 dir = {
                radius*glm::cos(phi)*glm::cos(theta), 
                radius*glm::sin(phi)*glm::cos(theta),
                radius*glm::sin(theta)
            };
            glm::vec3 dir1 = {
                radius*glm::cos(phi + delta_phi)*glm::cos(theta), 
                radius*glm::sin(phi + delta_phi)*glm::cos(theta),
                radius*glm::sin(theta)
            };
            glm::vec3 dir2 = {
                radius*glm::cos(phi)*glm::cos(theta + delta_theta), 
                radius*glm::sin(phi)*glm::cos(theta + delta_theta),
                radius*glm::sin(theta + delta_theta)
            };
            glm::vec3 dir3 = {
                radius*glm::cos(phi + delta_phi)*glm::cos(theta + delta_theta), 
                radius*glm::sin(phi + delta_phi)*glm::cos(theta + delta_theta),
                radius*glm::sin(theta + delta_theta)
            };

            glm::vec3 point = center + dir;
            glm::vec3 point1 = center + dir1;
            glm::vec3 point2 = center + dir2;
            glm::vec3 point3 = center + dir3;

            dir = glm::normalize(dir);
            dir1 = glm::normalize(dir1);
            dir2 = glm::normalize(dir2);
            dir3 = glm::normalize(dir3);

            result.push_back(point.x);
            result.push_back(point.y);
            result.push_back(point.z);
            result.push_back(dir.x);
            result.push_back(dir.y);
            result.push_back(dir.z);

            result.push_back(point1.x);
            result.push_back(point1.y);
            result.push_back(point1.z);
            result.push_back(dir1.x);
            result.push_back(dir1.y);
            result.push_back(dir1.z);

            result.push_back(point2.x);
            result.push_back(point2.y);
            result.push_back(point2.z);
            result.push_back(dir2.x);
            result.push_back(dir2.y);
            result.push_back(dir2.z);



            result.push_back(point1.x);
            result.push_back(point1.y);
            result.push_back(point1.z);
            result.push_back(dir1.x);
            result.push_back(dir1.y);
            result.push_back(dir1.z);

            result.push_back(point2.x);
            result.push_back(point2.y);
            result.push_back(point2.z);
            result.push_back(dir2.x);
            result.push_back(dir2.y);
            result.push_back(dir2.z);
            
            result.push_back(point3.x);
            result.push_back(point3.y);
            result.push_back(point3.z);
            result.push_back(dir3.x);
            result.push_back(dir3.y);
            result.push_back(dir3.z);

            theta += delta_theta;
        }
        phi += delta_phi;
    }

    return result;
}

glm::vec3 color = {0.5, 0.5, 0.8};
float scale_from_origin = 1.0f;

glm::vec3 camera_position = {-2, 2, -2};
glm::vec3 camera_direction = {1, -1, 1};

glm::vec3 lighsource_position = {1, 1.25, 0};
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
float ambientStrength = 0.45f;
float specularStrength = 0.55f;
int shininess = 64;
float sphere_radius = 0.25;

float light_rotation = 0;
float object_rotation_x = 0;
float object_rotation_y = 0;

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

    GL::GLuint VAO[COUNT], VBO[COUNT];
    for (int i=0; i<COUNT; i++) {
        GL::glGenVertexArrays(1, &VAO[i]);
        GL::glGenBuffers(1, &VBO[i]);
        GL::glBindVertexArray(VAO[i]);
        
        GL::glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        GL::glBufferData(GL_ARRAY_BUFFER, sizeof(float)*36, cube_vertexes[i], GL_STATIC_DRAW);
        
        GL::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        GL::glEnableVertexAttribArray(0);
        
        GL::glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        GL::glEnableVertexAttribArray(1);
        
        GL::glBindBuffer(GL_ARRAY_BUFFER, 0);
        GL::glBindVertexArray(0);
    }
    
    
    GL::GLuint sphere_VAO, sphere_VBO;
    GL::glGenVertexArrays(1, &sphere_VAO);
    GL::glGenBuffers(1, &sphere_VBO);

    auto sphere_vertex = generate_sphere(lighsource_position, sphere_radius, 100);
    GL::glBindVertexArray(sphere_VAO);
        
    GL::glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO);
    GL::glBufferData(GL_ARRAY_BUFFER, sizeof(float)*sphere_vertex.size(), sphere_vertex.data(), GL_STATIC_DRAW);
    
    GL::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    GL::glEnableVertexAttribArray(0);
    
    GL::glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    GL::glEnableVertexAttribArray(1);
    
    GL::glBindBuffer(GL_ARRAY_BUFFER, 0);
    GL::glBindVertexArray(0);

    while (!GL::glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
        
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
            scale_from_origin += 0.02f;
        }
        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
            scale_from_origin -= 0.02f;
            if (scale_from_origin < 0.5f) scale_from_origin = 0.5f;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            object_rotation_x += 0.006;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            object_rotation_x -= 0.006;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            object_rotation_y += 0.002;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            object_rotation_y -= 0.002;
        }

        // std::cout << scale_from_origin << std::endl;

        GL::glClearColor(0.1, 0.1, 0.1, 1);
        GL::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GL::glUseProgram(shaderProgram);

        light_rotation += 0.005;
                
        glm::mat4 model(1);
        
        model = glm::rotate(model, object_rotation_x, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, object_rotation_y, glm::vec3(0.0f, 0.0f, 1.0f));
        
        glm::mat4 light_model(1);
        
        light_model = glm::rotate(light_model, -light_rotation, glm::vec3(0.0f, 1.0f, 0.0f));
        light_model = model*light_model;

        glm::vec4 temp = glm::vec4(lighsource_position, 1.0f);
        auto real_light_position = glm::vec3(light_model*temp);
        

        std::cout << real_light_position.x << "," << real_light_position.y << "," << real_light_position.z << std::endl;
        
        
        glm::mat4 view = glm::lookAt(camera_position, camera_position + camera_direction, glm::vec3(0,1,0));
        
        glm::mat4 projection = glm::perspective(45.0f, 800.0f/600.0f, 0.1f, 100.0f);
        
        GL::glUniformMatrix4fv(GL::glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        GL::glUniformMatrix4fv(GL::glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        GL::glUniformMatrix4fv(GL::glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        GL::glUniform1f(GL::glGetUniformLocation(shaderProgram, "scaleFromOrigin"), scale_from_origin);
        
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "lightPosition"), 1, glm::value_ptr(real_light_position));
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
        GL::glUniform1f(GL::glGetUniformLocation(shaderProgram, "ambientStrength"), ambientStrength);
        GL::glUniform1f(GL::glGetUniformLocation(shaderProgram, "specularStrength"), specularStrength);
        GL::glUniform1i(GL::glGetUniformLocation(shaderProgram, "shininess"), shininess);
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera_position));

        for (int i=0; i<COUNT; i++) {
            GL::glBindVertexArray(VAO[i]);
            GL::glDrawArrays(GL_TRIANGLES, 0, 36);
            GL::glBindVertexArray(0);
        }

        
        GL::glUniformMatrix4fv(GL::glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(light_model));
        GL::glUniformMatrix4fv(GL::glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        GL::glUniformMatrix4fv(GL::glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        GL::glUniform1f(GL::glGetUniformLocation(shaderProgram, "scaleFromOrigin"), 0);
        
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "lightPosition"), 1, glm::value_ptr(real_light_position));
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(lightColor));
        GL::glUniform1f(GL::glGetUniformLocation(shaderProgram, "ambientStrength"), ambientStrength);
        GL::glUniform1f(GL::glGetUniformLocation(shaderProgram, "specularStrength"), specularStrength);
        GL::glUniform1i(GL::glGetUniformLocation(shaderProgram, "shininess"), shininess);
        GL::glUniform3fv(GL::glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera_position));

        
        GL::glBindVertexArray(sphere_VAO);
        GL::glDrawArrays(GL_TRIANGLES, 0, sphere_vertex.size());
        GL::glBindVertexArray(0);
    

        GL::glfwSwapBuffers(window);
        GL::glfwPollEvents();
    }


    GL::glfwDestroyWindow(window);
    GL::glfwTerminate();

    return 0;
}
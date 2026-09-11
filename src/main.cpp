#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <vector>
#include <cmath>
#include <cstddef>
#include <glm/gtc/constants.hpp>
#include "DebugControls.h"
#include "QualityControls.h"
#include "GpuTimer.h"
#include "Benchmark.h"

namespace
{
    constexpr int WindowWidth = 900;
    constexpr int WindowHeight = 600;

    constexpr unsigned int SphereLatitudeSegments = 64;
    constexpr unsigned int SphereLongitudeSegments = 64;
    constexpr float SphereRadius = 1.0f;

    constexpr float LodFullDistance = 5.0f;
    constexpr float LodSimpleDistance = 6.0f;

    constexpr float CameraMoveSpeed = 2.0f;

    constexpr float FieldOfViewDegrees = 45.0f;
    constexpr float NearPlane = 0.1f;
    constexpr float FarPlane = 100.0f;

    constexpr int SimpleQualityLevel = 0;
    constexpr int FullQualityLevel = 1;

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
    };

    struct SphereMesh 
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };


    SphereMesh createSphere(unsigned int latitudeSegments, unsigned int longitudeSegments, float radius)
    {
        SphereMesh mesh;

        for (unsigned int y = 0; y <= latitudeSegments; ++y)
        {
            float v = static_cast<float>(y) / static_cast<float>(latitudeSegments);

            float theta = v * glm::pi<float>();

            for (unsigned int x = 0; x <= longitudeSegments; ++x)
            {
                float u = static_cast<float>(x) / static_cast<float>(longitudeSegments);

                float phi = u * glm::two_pi<float>();

                const glm::vec3 normal(
                    std::sin(theta) * std::cos(phi),
                    std::cos(theta),
                    std::sin(theta) * std::sin(phi)
                );

                mesh.vertices.push_back({normal * radius, normal});
            }
        }

        for (unsigned int y = 0; y < latitudeSegments; ++y)
        {
            for (unsigned int x = 0; x < longitudeSegments; ++x)
            {
                unsigned int first = y * (longitudeSegments + 1) + x;

                unsigned int second = first + longitudeSegments + 1;

                mesh.indices.push_back(first);
                mesh.indices.push_back(first + 1);
                mesh.indices.push_back(second);

                mesh.indices.push_back(second);
                mesh.indices.push_back(first + 1);
                mesh.indices.push_back(second + 1);
            }
        }

        return mesh;
    }

    void glfwErrorCallback(int error, const char* description)
    {
        std::cerr << "GLFW error (" << error << "): " << description << '\n';
    }

    std::string readTextFile(const std::string& path)
    {
        std::ifstream file(path);
        if (!file)
        {
            throw std::runtime_error("Could not open file: " + path);
        }

        std::ostringstream contents;
        contents << file.rdbuf();
        return contents.str();
    }

    GLuint compileShader(GLenum type, const std::string& source, const std::string& label)
    {
        const GLuint shader = glCreateShader(type);
        const char* sourcePtr = source.c_str();

        glShaderSource(shader, 1, &sourcePtr, nullptr);
        glCompileShader(shader);

        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (success == GL_FALSE)
        {
            GLint logLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

            std::string log(static_cast<std::size_t>(logLength), '\0');
            glGetShaderInfoLog(shader, logLength, nullptr, log.data());

            glDeleteShader(shader);
            throw std::runtime_error("Shader compilation failed (" + label + "):\n" + log);
        }

        return shader;
    }

    GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath)
    {
        const std::string vertexSource = readTextFile(vertexPath);
        const std::string fragmentSource = readTextFile(fragmentPath);

        const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource, vertexPath);
        GLuint fragmentShader = 0;

    try
    {
        fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
    }
    catch (...)
    {
        glDeleteShader(vertexShader);
        throw;
    }

        const GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        GLint success = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (success == GL_FALSE)
        {
            GLint logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

            std::string log(static_cast<std::size_t>(logLength), '\0');
            glGetProgramInfoLog(program, logLength, nullptr, log.data());

            glDeleteProgram(program);
            throw std::runtime_error("Shader program link failed:\n" + log);
        }

        return program;
    }

    void framebufferSizeCallback(GLFWwindow*, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    void processInput(GLFWwindow* window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

int main()
{
	glfwSetErrorCallback(glfwErrorCallback);
	// Need to initialize GLFW before calling any GLFW functions
    if (glfwInit() != GLFW_TRUE)
    {
        std::cerr << "Failed to initialize GLFW.\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, "3D and Shader Programming Yagiz Selek", nullptr, nullptr);

    if (window == nullptr)
    {
        std::cerr << "Failed to create a GLFW window.\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSwapInterval(1);

    const int loadedVersion = gladLoadGL(glfwGetProcAddress);
    if (loadedVersion == 0)
    {
        std::cerr << "Failed to load OpenGL functions with GLAD.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';


	// Create a sphere mesh with 64 latitude and longitude segments and a radius of 1.0
    const SphereMesh sphere = createSphere(
        SphereLatitudeSegments,
        SphereLongitudeSegments,
        SphereRadius
    );

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;


    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 
        sizeof(Vertex) * sphere.vertices.size()
        , sphere.vertices.data(),
		GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); 

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sphere.indices.size() * sizeof(unsigned int),
        sphere.indices.data(),
        GL_STATIC_DRAW
	);


    constexpr GLsizei stride = sizeof(Vertex);


    glVertexAttribPointer(
        0, 
        3, 
        GL_FLOAT, 
        GL_FALSE, 
        stride, 
		reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3, 
        GL_FLOAT, 
        GL_FALSE, 
        stride, 
        reinterpret_cast<void*>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    GLuint shaderProgram = 0;

    try
    {
        shaderProgram = createShaderProgram("shaders/basic.vert", "shaders/basic.frag");
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    const GLint modelLocation = glGetUniformLocation(shaderProgram, "model");
    const GLint viewLocation = glGetUniformLocation(shaderProgram, "view");
    const GLint projectionLocation = glGetUniformLocation(shaderProgram, "projection");
    const GLint normalMatrixLocation = glGetUniformLocation(shaderProgram, "normalMatrix");
    const GLint viewPositionLocation = glGetUniformLocation(shaderProgram, "viewPosition");
    const GLint timeLocation = glGetUniformLocation(shaderProgram, "time");
    const GLint debugModeLocation = glGetUniformLocation(shaderProgram, "debugMode");
    const GLint qualityLevelLocation = glGetUniformLocation(shaderProgram, "qualityLevel");

    if (modelLocation == -1 || viewLocation == -1 || projectionLocation == -1)
    {
        std::cerr
            << "Note: one or more matrix uniforms are inactive. "
            << "This is expected if the current shader experiment does not use them.\n";
    }

    if (timeLocation == -1)
    {
        std::cerr
            << "Note: the time uniform is inactive. "
            << "This is expected if the current shader experiment does not use it.\n";
    }

    const glm::mat4 model(1.0f);

    const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    glm::vec3 viewPosition(0.0f, 0.0f, 3.0f);

    DebugMode debugMode = DebugMode::Final;

    // Start in auto quality
    QualityMode qualityMode = QualityMode::Auto;

    int autoQualityLevel = FullQualityLevel;

    float previousTime = static_cast<float>(glfwGetTime());

    bool benchmarkKeyWasPressed = false;
    bool benchmarkWasRunning = false;
    {

            GpuTimer gpuTimer;
            Benchmark benchmark;
        while (glfwWindowShouldClose(window) == GLFW_FALSE)
        {
            const float currentTime = static_cast<float>(glfwGetTime());

            const float deltaTime = currentTime - previousTime;

            previousTime = currentTime;

            processInput(window);

            const bool benchmarkKeyPressed = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;

		    //Don't change debug or quality mode while benchmarking, as it would invalidate the results.
            if (!benchmark.isRunning())
            {
                updateDebugControls(window, debugMode);

                updateQualityControls(window, qualityMode);
            }

            if (benchmarkKeyPressed && !benchmarkKeyWasPressed && !benchmark.isRunning())
            {
                if (debugMode != DebugMode::Final)
                {
                    std::cout << "Benchmark requires Debug Mode 0 (Final).\n";
                }
                else if (qualityMode == QualityMode::Auto)
                {
                    std::cout << "Benchmark requires Simple or Full mode. Select F1 or F2 first.\n";
                }
                else
                {
                    const std::string label = qualityMode == QualityMode::Full ? "Full" : "Simple";

                    // Every benchmark begins from exactly the same
                    // camera position and therefore the same screen coverage.
                    viewPosition = glm::vec3(0.0f, 0.0f, 3.0f);

                    // Disable VSync so presentation rate does not limit the benchmark.
                    glfwSwapInterval(0);

                    benchmark.start(label);

                    benchmarkWasRunning = true;
                }
            }

            benchmarkKeyWasPressed = benchmarkKeyPressed;

            if (!benchmark.isRunning())
            {
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                {
                    viewPosition.z -= CameraMoveSpeed * deltaTime;
                }

                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                {
                    viewPosition.z += CameraMoveSpeed * deltaTime;
                }

                viewPosition.z = glm::max(viewPosition.z, SphereRadius + 0.2f);
            }

            const glm::mat4 view =glm::translate(glm::mat4(1.0f), -viewPosition);

            // Sphere centre in world space.
            const glm::vec3 sphereCenter = glm::vec3(model[3]);

            // Distance between the camera and the sphere centre.
            const float distanceToSphere = glm::length(viewPosition - sphereCenter);

            int framebufferWidth = 0;
            int framebufferHeight = 0;
            glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

            if (framebufferWidth == 0 || framebufferHeight == 0)
            {
                glfwPollEvents();
                continue;
            }

            int qualityLevel = 1;

            const int previousAutoQualityLevel = autoQualityLevel;

            switch (qualityMode)
            {
            case QualityMode::Simple:qualityLevel = SimpleQualityLevel;
            break;

            case QualityMode::Full:qualityLevel = FullQualityLevel;
            break;

            case QualityMode::Auto:
            {
                if (distanceToSphere > LodSimpleDistance)
                {
                    autoQualityLevel = SimpleQualityLevel;
                }
                else if (distanceToSphere < LodFullDistance)
                {
                    autoQualityLevel = FullQualityLevel;
                }
            }
                qualityLevel = autoQualityLevel;
                break;
            }

            if (autoQualityLevel != previousAutoQualityLevel)
            {
                std::cout
                    << "Auto LOD: "
                    << (autoQualityLevel == 1 ? "Full" : "Simple")
                    << " | distance: "
                    << distanceToSphere
                    << '\n';
            }

            const float aspectRatio = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);

            const glm::mat4 projection = glm::perspective(glm::radians(FieldOfViewDegrees), aspectRatio, NearPlane, FarPlane);

            glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(shaderProgram);

            glUniform1i(debugModeLocation, static_cast<int>(debugMode));
            glUniform1i(qualityLevelLocation, qualityLevel);

            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            glUniform3fv(viewPositionLocation, 1, glm::value_ptr(viewPosition));
            glUniform1f(timeLocation, currentTime);

            glBindVertexArray(vao);

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Transparent object should not write into depth buffer
            glDepthMask(GL_FALSE);

            glEnable(GL_CULL_FACE);

            double gpuMilliseconds = 0.0;

            if (gpuTimer.tryGetElapsedMilliseconds(gpuMilliseconds))
            {
                benchmark.addSample(gpuMilliseconds);
            }

            bool measuringGpu = false;

            if (benchmark.isWarmingUp())
            {
                benchmark.updateWarmup();
            }
            else if (benchmark.isRunning())
            {
                measuringGpu = gpuTimer.begin();
            }

            // 1. Render back faces first
            glCullFace(GL_FRONT);

            glDrawElements(
                GL_TRIANGLES,
                static_cast<GLsizei>(sphere.indices.size()),
                GL_UNSIGNED_INT,
                nullptr
            );

            // 2. Render front faces afterwards
            glCullFace(GL_BACK);

            glDrawElements(
                GL_TRIANGLES,
                static_cast<GLsizei>(sphere.indices.size()),
                GL_UNSIGNED_INT,
                nullptr
            );

            if (measuringGpu)
            {
                gpuTimer.end();
            }

            // Restore normal state
            glDepthMask(GL_TRUE);
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);

            if (benchmarkWasRunning && !benchmark.isRunning())
            {
                glfwSwapInterval(1);

                benchmarkWasRunning = false;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

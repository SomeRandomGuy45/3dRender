#include "main.h"

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "ERROR::GLFW Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set the required OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(800, 600, "Main App", NULL, NULL);
    if (!window) {
        std::cerr << "ERROR::GLFW Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Check OpenGL version
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "INFO::GLFW Renderer: " << renderer << std::endl;
    std::cout << "INFO::GLFW OpenGL version supported: " << version << std::endl;

    // Initialize GLEW
    glewExperimental = GL_TRUE; 
    if (glewInit() != GLEW_OK) {
        std::cerr << "ERROR::GLEW Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST); // Enable depth testing

    // Set up camera
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
    glfwSetWindowUserPointer(window, &camera);
    glfwSetCursorPosCallback(window, mouseCallback);

    // Set the framebuffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Compile shaders and create shader program
    GLuint shaderProgram = compileShaders();

    // Load models into a vector
    //Example: models.push_back(loadModel("pathToObj.obj", glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(255.0f,0.0f,0.0f), glm::vec3(1.0f), glm::vec3(90.0f, 45.0f, 90.0f)));
    models.push_back(loadModel("test.obj", glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(255.0f,0.0f,0.0f), glm::vec3(1.0f), glm::vec3(90.0f, 45.0f, 90.0f)));

    // Set up projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glEnable(GL_DEPTH_TEST); // Enable depth testing
    glEnable(GL_CULL_FACE);  // Enable backface culling
    glCullFace(GL_BACK);      // Cull back faces

    // Main loop
    float lastFrameTime = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        // Process input
        float currentTime = glfwGetTime();
        currentDeltaTime = currentTime - lastFrameTime; // Calculate delta time
        lastFrameTime = currentTime; // Update last frame time
        camera.processKeyboard(currentDeltaTime); // Adjust deltaTime as needed

        // Clear the buffers
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use the shader program
        glUseProgram(shaderProgram);

        // Set the view and projection matrices
        glm::mat4 view = camera.getViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        
        glm::mat4 projection = camera.getProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Render all loaded models
        renderModels(shaderProgram);
        DoAllTweenRotate();
        DoAllTweenMove();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    for (const auto& model : models) {
        GLuint VAO = std::get<0>(model); // Extract the VAO from the model tuple
        glDeleteVertexArrays(1, &VAO);
    }
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
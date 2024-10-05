#include "Render.h"

// Mouse callback to capture mouse movement for camera control
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!locked) return;
    static float lastX = 400, lastY = 300;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY;

    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    camera->processMouseMovement(xoffset, yoffset);

    lastX = xpos;
    lastY = ypos;
}

void RemoveModel(int index)
{
    auto model = models[index];
    GLuint VAO = std::get<0>(model); // Extract the VAO from the model tuple
    glDeleteVertexArrays(1, &VAO);
    models.erase(std::remove(models.begin(), models.end(), model), models.end());
}

void MoveModel(int index, glm::vec3 newPosition, bool tween, float duration)
{
    if (index < 0 || index >= models.size())
    {
        std::cerr << "Invalid model index: " << index << std::endl;
        return;
    }

    auto& model = models[index];
    glm::vec3 currentPosition = std::get<1>(model).second;
    glm::vec3 direction = newPosition - currentPosition;
    float totalDistance = glm::length(direction);
    if (tween)
    {
        needToTween_POS[index] = {currentPosition, newPosition, duration, 0.0f, totalDistance/duration};
    }
    else
    {
        std::get<1>(model).second = newPosition;
    }
}

void MoveModel(int index, glm::vec3 newPosition) {MoveModel(index, newPosition, false, 0);};
void MoveModel(int index, glm::vec3 newPosition, bool tween) {MoveModel(index, newPosition, tween, 0);};

void RotateModel(int index, glm::vec3 newRotation, bool tween, float duration)
{
    if (index < 0 || index >= models.size()) {
        std::cerr << "Invalid model index: " << index << std::endl;
        return;
    }

    // Get the current model's rotation
    auto& model = models[index];
    glm::vec3 currentRotation = std::get<2>(std::get<2>(model)); // Assuming the rotation is stored here
    glm::vec3 direction = newRotation - currentRotation;
    float totalDistance = glm::length(direction);

    if (tween) {
        // Initialize tweening parameters
        needToTween[index] = {currentRotation, newRotation, duration, 0.0f, totalDistance};
    } else {
        // Directly set the new rotation if not tweening
        std::get<2>(std::get<2>(model)) = newRotation;
    }
}

void RotateModel(int index, glm::vec3 newRotation) {RotateModel(index, newRotation, false, 0);};
void RotateModel(int index, glm::vec3 newRotation, bool tween) {RotateModel(index, newRotation, tween, 0);};

void DoAllTweenRotate()
{
    for (auto it = needToTween.begin(); it != needToTween.end(); ) {
        auto& [index, tween] = *it;

        // If this is the first update, calculate the angular speed
        if (tween.elapsedTime == 0.0f) {
            // Calculate the angular distance to rotate
            glm::vec3 direction = tween.endRotation - tween.startRotation;
            float totalAngle = glm::length(direction); // Total angular distance
            tween.speed = totalAngle / tween.duration; // Compute angular speed
        }

        // Update elapsed time
        tween.elapsedTime += currentDeltaTime;

        // Calculate the angular distance to rotate this frame
        float angleToRotate = tween.speed * currentDeltaTime;

        // Calculate the current rotation
        glm::vec3 currentRotation = std::get<2>(std::get<2>(models[index]));

        // Determine if we can reach the target in this frame
        if (glm::length(tween.endRotation - currentRotation) > angleToRotate) {
            // Move towards the target by the calculated angle
            glm::vec3 rotationStep = glm::normalize(tween.endRotation - currentRotation) * angleToRotate;
            currentRotation += rotationStep; // Update current rotation
        } else {
            // Snap to the target rotation when within the distance threshold
            currentRotation = tween.endRotation;
            std::cout << "Tween complete for model " << index << std::endl;
            it = needToTween.erase(it); // Remove the completed tween
            continue; // Skip the increment as we've erased the element
        }

        // Update model rotation
        auto& model = models[index];
        std::get<2>(std::get<2>(model)) = currentRotation;

        // Debug output
        std::cout << "Current Rotation for model " << index << ": " << glm::to_string(currentRotation) << std::endl;

        ++it; // Move to the next tween
    }
}

void DoAllTweenMove()
{
    for (auto it = needToTween_POS.begin(); it != needToTween_POS.end(); ) {
        auto& [index, tween] = *it;

        // Get the current position
        glm::vec3 currentPos = std::get<1>(models[index]).second;

        // Calculate the total distance to the target position
        glm::vec3 direction = tween.endRotation - tween.startRotation;
        float totalDistance = glm::length(direction);

        // If speed is not already set, calculate it based on duration
        if (tween.elapsedTime == 0.0f) {
            tween.speed = totalDistance / tween.duration; // Compute speed
        }

        // Normalize the direction
        direction = glm::normalize(direction);

        // Calculate the distance to move this frame
        float distanceToMove = tween.speed * currentDeltaTime;

        // Check if we can reach the target in this frame
        if (glm::distance(currentPos, tween.endRotation) > distanceToMove) {
            // Move the object a fixed distance in the direction of the target
            currentPos += direction * distanceToMove;
        } else {
            // Snap to the target position when within the distance threshold
            currentPos = tween.endRotation; 
            std::cout << "Tween complete for model " << index << std::endl;
            it = needToTween_POS.erase(it); // Remove the completed tween
            continue; // Skip the increment, as we have erased the element
        }

        // Update model position
        std::get<1>(models[index]).second = currentPos;

        // Update elapsed time
        tween.elapsedTime += currentDeltaTime;

        ++it; // Move to the next tween
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Adjust the viewport based on the new width and height
    glViewport(0, 0, width, height);

    // Get camera from user pointer
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    
    // Update the projection matrix
    camera->setProjection((float)width / (float)height);
}
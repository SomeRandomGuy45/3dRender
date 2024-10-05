#include "compileShaders.h"

//deltaTime
float currentDeltaTime;

// Tween parameters
struct Tween {
    glm::vec3 startRotation; // Starting rotation (in degrees)
    glm::vec3 endRotation;   // Target rotation (in degrees)
    float duration;          // Duration of the tween in seconds
    float elapsedTime;       // Time elapsed since the tween started
    float speed; // Speed of the tween
};

// Map to hold tween data for models
std::map<int, Tween> needToTween;
std::map<int, Tween> needToTween_POS;

float LinearEase(float t) {
    return t; // Linear interpolation
}

float EaseIn(float t) {
    return t * t; // Accelerating from zero velocity
}

float EaseOut(float t) {
    return t * (2 - t); // Decelerating to zero velocity
}

float EaseInOut(float t) {
    return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t; // Acceleration until halfway, then deceleration
}

glm::vec3 DoTweenFunc(const glm::vec3& start, const glm::vec3& end, float duration, float (*easeFunc)(float)) {
    float t = currentDeltaTime / duration; // Calculate the progress
    t = glm::clamp(t, 0.0f, 1.0f); // Clamp to [0, 1]
    
    // Apply the easing function
    float easedT = easeFunc(t);
    
    // Interpolate
    return start + (end - start) * easedT; 
}

// Function to render all loaded models
void renderModels(GLuint shaderProgram) {
    for (const auto& model : models) {
        GLuint VAO = std::get<0>(model);
        unsigned int indexCount = std::get<1>(model).first;
        glm::vec3 position = std::get<1>(model).second;
        glm::vec3 color = std::get<0>(std::get<2>(model));
        glm::vec3 scale = std::get<1>(std::get<2>(model));
        glm::vec3 rotationAxis = std::get<2>(std::get<2>(model));
        GLuint textureID = std::get<3>(model); // Get textureID

        // Normalize the axis and calculate the angle
        float rotationAngle = glm::length(rotationAxis);
        if (rotationAngle > 0.0f) {
            rotationAxis = glm::normalize(rotationAxis);
        }

        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
        modelMatrix = glm::rotate(modelMatrix, rotationAngle, rotationAxis); // Rotation
        modelMatrix = glm::scale(modelMatrix, scale); // Scaling
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

        // Check if the texture is used
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), textureID != 0); // Set the useTexture uniform

        // Set color for the fragment shader
        glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, glm::value_ptr(color));

        // Bind the texture if textureID is not zero
        if (textureID != 0) {
            glActiveTexture(GL_TEXTURE0); // Activate texture unit
            glBindTexture(GL_TEXTURE_2D, textureID); // Bind texture
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Unbind the texture
        if (textureID != 0) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}
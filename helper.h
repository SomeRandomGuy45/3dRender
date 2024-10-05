#include <iostream>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <map>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Vertex Shader
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords; // Add texture coordinates input

out vec2 fragTexCoords; // Pass texture coordinates to fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    fragTexCoords = texCoords; // Pass texture coordinates to fragment shader
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
#version 330 core
in vec2 fragTexCoords; // Receive texture coordinates from vertex shader
out vec4 outColor;

uniform sampler2D texture1; // Texture sampler
uniform vec3 color; // Color uniform
uniform bool useTexture; // Boolean to determine whether to use texture or color

void main() {
    if (useTexture) {
        vec4 textureColor = texture(texture1, fragTexCoords); // Sample the texture
        outColor = textureColor; // Use texture color
    } else {
        outColor = vec4(color, 1.0); // Use the specified color
    }
}
)";

GLFWwindow* window;

bool locked = true;

std::vector<std::tuple<GLuint, std::pair<unsigned int, glm::vec3>, std::tuple<glm::vec3, glm::vec3, glm::vec3>, GLuint>> models;

GLuint loadTexture(const std::string& path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    // Load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Flip loaded texture coordinates
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format = (nrChannels == 1) ? GL_RED : (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // Generate texture
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cerr << "ERROR::IMAGE-LOADING Failed to load texture: " << path << std::endl;
    }

    stbi_image_free(data); // Free the image data
    return textureID;
}

// Function to load an OBJ file using Assimp
std::tuple<GLuint, std::pair<unsigned int, glm::vec3>, std::tuple<glm::vec3, glm::vec3, glm::vec3>, GLuint> loadModel(
    const std::string& path, 
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f), 
    glm::vec3 scale = glm::vec3(1.0f), 
    glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 0.0f),
    const std::string& texturePath = ""
)
{
    GLuint textureID = 0;
    std::vector<GLuint> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;

    color.x /= 255.0f;
    color.y /= 255.0f;
    color.z /= 255.0f;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FixInfacingNormals | aiProcess_SortByPType);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return { 0, { 0, glm::vec3(0.0f) }, { glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(0.0f) }, 0 }; 
    }

    // Process each mesh in the scene
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];

        // Process vertices and texture coordinates
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            aiVector3D pos = mesh->mVertices[j];
            vertices.emplace_back(pos.x, pos.y, pos.z);

            if (mesh->mTextureCoords[0]) {
                aiVector3D texCoord = mesh->mTextureCoords[0][j];
                texCoords.emplace_back(texCoord.x, texCoord.y);
            } else {
                texCoords.emplace_back(0.0f, 0.0f);
            }
        }

        // Process indices
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                indices.push_back(face.mIndices[k]);
            }
        }

        // Load material properties
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiColor3D diffuse;
            material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
            color = glm::vec3(diffuse.r, diffuse.g, diffuse.b); // Use the diffuse color

            // Load the texture if available
            aiString texturePath;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                // Load texture
                std::string fullPath = std::string(texturePath.C_Str());
                std::cout << "INFO::IMAGE Loading Image Path:" << fullPath << "\n";
                textureID = loadTexture(fullPath);
                // Save the textureID or use it directly if needed
            }
        }
    }

    std::cout << "INFO::IMAGE Loaded " << vertices.size() << " vertices and " << indices.size() << " indices." << std::endl;

    GLuint VAO, VBO, EBO, TBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &TBO);

    glBindVertexArray(VAO);

    // Vertex Buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Texture Coordinate Buffer
    glBindBuffer(GL_ARRAY_BUFFER, TBO);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)0);
    glEnableVertexAttribArray(1);

    // Element Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (!texturePath.empty() && textureID == 0) {
        textureID = loadTexture(texturePath);
    }

    return { VAO, { static_cast<unsigned int>(indices.size()), position }, { color, scale, rotationAxis }, textureID };
}
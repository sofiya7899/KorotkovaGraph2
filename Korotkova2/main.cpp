//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include <stdio.h>

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>


static const GLsizei WIDTH = 1024, HEIGHT = 1024; //размеры окна
static int filling = 0;
static bool keys[1024]; //массив состояний кнопок - нажата/не нажата
static GLfloat lastX = 400, lastY = 300; //исходное положение мыши
static bool firstMouse = true;
static bool g_captureMouse = true;  // Мышка захвачена нашим приложением или нет?
static bool g_capturedMouseJustNow = false;
static int g_shaderProgram = 0;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Camera camera(float3(0.0f, 0.0f, 5.0f));

//функция для обработки нажатий на кнопки клавиатуры
void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    //std::cout << key << std::endl;
    switch (key)
    {
    case GLFW_KEY_ESCAPE: //на Esc выходим из программы
        if (action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        break;
    case GLFW_KEY_SPACE: //на пробел переключение в каркасный режим и обратно
        if (action == GLFW_PRESS)
        {
            if (filling == 0)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                filling = 1;
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                filling = 0;
            }
        }
        break;
    case GLFW_KEY_1:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case GLFW_KEY_Q:
        keys[GLFW_KEY_E] = false;
        keys[key] = true;
        break;
    case GLFW_KEY_E:
        keys[GLFW_KEY_Q] = false;
        keys[key] = true;
        break;
    case GLFW_KEY_2:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
    default:
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

//функция для обработки клавиш мыши
void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        g_captureMouse = !g_captureMouse;


    if (g_captureMouse)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        g_capturedMouseJustNow = true;
    }
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

//функция для обработки перемещения мыши
void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = float(xpos);
        lastY = float(ypos);
        firstMouse = false;
    }

    GLfloat xoffset = float(xpos) - lastX;
    GLfloat yoffset = lastY - float(ypos);

    lastX = float(xpos);
    lastY = float(ypos);

    if (g_captureMouse)
        camera.ProcessMouseMove(xoffset, yoffset);
}


void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(GLfloat(yoffset));
}

void doCameraMovement(Camera& camera, GLfloat deltaTime)
{
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}


GLsizei CreateSphere(float radius, int numberSlices, GLuint& vao)
{
    int i, j;

    int numberParallels = numberSlices;
    int numberVertices = (numberParallels + 1) * (numberSlices + 1);
    int numberIndices = numberParallels * numberSlices * 3;

    float angleStep = (2.0f * 3.14159265358979323846f) / ((float)numberSlices);
    //float helpVector[3] = {0.0f, 1.0f, 0.0f};

    std::vector<float> pos(numberVertices * 4, 0.0f);
    std::vector<float> norm(numberVertices * 4, 0.0f);
    std::vector<float> texcoords(numberVertices * 2, 0.0f);

    std::vector<int> indices(numberIndices, -1);

    for (i = 0; i < numberParallels + 1; i++)
    {
        for (j = 0; j < numberSlices + 1; j++)
        {
            int vertexIndex = (i * (numberSlices + 1) + j) * 4;
            int normalIndex = (i * (numberSlices + 1) + j) * 4;
            int texCoordsIndex = (i * (numberSlices + 1) + j) * 2;

            pos.at(vertexIndex + 0) = radius * sinf(angleStep * (float)i) * sinf(angleStep * (float)j);
            pos.at(vertexIndex + 1) = radius * cosf(angleStep * (float)i);
            pos.at(vertexIndex + 2) = radius * sinf(angleStep * (float)i) * cosf(angleStep * (float)j);
            pos.at(vertexIndex + 3) = 1.0f;

            norm.at(normalIndex + 0) = pos.at(vertexIndex + 0) / radius;
            norm.at(normalIndex + 1) = pos.at(vertexIndex + 1) / radius;
            norm.at(normalIndex + 2) = pos.at(vertexIndex + 2) / radius;
            norm.at(normalIndex + 3) = 1.0f;

            texcoords.at(texCoordsIndex + 0) = (float)j / (float)numberSlices;
            texcoords.at(texCoordsIndex + 1) = (1.0f - (float)i) / (float)(numberParallels - 1);
        }
    }

    int* indexBuf = &indices[0];

    for (i = 0; i < numberParallels; i++)
    {
        for (j = 0; j < numberSlices; j++)
        {
            *indexBuf++ = i * (numberSlices + 1) + j;
            *indexBuf++ = (i + 1) * (numberSlices + 1) + j;
            *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);

            *indexBuf++ = i * (numberSlices + 1) + j;
            *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);
            *indexBuf++ = i * (numberSlices + 1) + (j + 1);

            int diff = int(indexBuf - &indices[0]);
            if (diff >= numberIndices)
                break;
        }
        int diff = int(indexBuf - &indices[0]);
        if (diff >= numberIndices)
            break;
    }

    GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(GLfloat), &pos[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(GLfloat), &norm[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    //glGenBuffers(1, &vboTexCoords);
    //glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
    //glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GLfloat), &texcoords[0], GL_STATIC_DRAW);
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    //glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}



GLsizei CreateTriangle(GLuint& vao)
{
    std::vector<float> positions = { -1.0f, 0.0f, 0.0f, 1.0f,
                                      1.0f, 0.0f, 0.0f, 1.0f,
                                      0.0f, 2.0f, 0.0f, 1.0f };

    std::vector<float> normals = { 1.0f, 0.0f, 0.0f, 0.1f,
                                    1.0f, 0.0f, 0.0f, 0.1f,
                                    1.0f, 0.0f, 0.0f, 0.1f };

    std::vector<float> texCoords = { 0.0f, 0.0f,
                                     0.5f, 1.0f,
                                     1.0f, 0.0f };

    std::vector<uint32_t> indices = { 0u, 1u, 2u };

    GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    /* glGenBuffers(1, &vboTexCoords);
     glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
     glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(GLfloat), texCoords.data(), GL_STATIC_DRAW);
     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
     glEnableVertexAttribArray(2);*/

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLuint CreateCube(GLuint& vao, float4 a, float4 b, float4 c, float4 d, float4 c1) {
    std::vector<float> positions = {
        a.x, a.y, a.z, a.w,
        b.x, b.y, b.z, b.w,
        c.x, c.y, c.z, c.w,
        d.x, d.y, d.z, d.w,
        a.x, c1.y, a.z, a.w,
        b.x, c1.y, b.z, a.w,
        c1.x, c1.y, c1.z, c1.w,
        d.x, c1.y, d.z, d.w
    };

    std::vector<float> normals = {
        -1.0f, -1.0f,  1.0f, 1.0f,
         1.0f, -1.0f, -1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f,
         1.0f, -1.0f, -1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f };

    std::vector<uint32_t> indices = {
        1u, 4u, 0u,
        1u, 4u, 5u,
        1u, 5u, 2u,
        6u, 5u, 2u,
        6u, 7u, 2u,
        3u, 7u, 2u,
        3u, 7u, 0u,
        4u, 7u, 0u,
        4u, 6u, 5u,
        4u, 6u, 7u,
        0u, 2u, 1u,
        0u, 2u, 3u,
    };

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLuint CreateCone(GLuint& vao, float4 center, float4 vertex, float radius, int slices) {

    std::vector<float> norm(4 * (slices + 2), 0.0f);
    std::vector<float> positions(4 * (slices + 2), 0.0f);
    positions.at(0) = (vertex.x);
    positions.at(1) = (vertex.y);
    positions.at(2) = (vertex.z);
    positions.at(3) = (vertex.w);
    positions.at(4) = (center.x);
    positions.at(5) = (center.y);
    positions.at(6) = (center.z);
    positions.at(7) = (center.w);

    float angleStep = (2.0f * 3.14159265358979323846f) / ((float)slices);

    for (int i = 2; i < slices + 2; i++) {
        positions.at(i * 4) = center.x + cosf(angleStep * (i - 2)) * radius;
        positions.at(i * 4 + 1) = center.y + sinf(angleStep * (i - 2)) * radius;
        positions.at(i * 4 + 2) = center.z;
        positions.at(i * 4 + 3) = 1.0f;

        norm.at(i + 0) = positions.at(i + 0) / radius;
        norm.at(i + 1) = positions.at(i + 1) / radius;
        norm.at(i + 2) = positions.at(i + 2) / radius;
        norm.at(i + 3) = 1.0f;
    }

    std::vector<UINT32>indicies(slices * 3 * 2, 0.0f);

    for (unsigned int i = 2; i < slices + 2; i++) {
        indicies.at((i - 2) * 6 + 0) = 1u;
        indicies.at((i - 2) * 6 + 1) = i;
        indicies.at((i - 2) * 6 + 2) = i >= (slices + 1) ? 2u : i + 1;

        indicies.at((i - 2) * 6 + 3) = 0u;
        indicies.at((i - 2) * 6 + 4) = i;
        indicies.at((i - 2) * 6 + 5) = i >= (slices + 1) ? 2u : i + 1;
    }

    GLuint vboVertices, vboIndices, vboNormals;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(GLfloat), norm.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(int), indicies.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indicies.size();
}

GLuint createCylinder(GLuint& vao, float4 bottonCenter, float4 topCenter, float radius, UINT32 slices) {
    std::vector<float> norm(2 * 4 * (slices)+8, 0.0f);
    std::vector<float> positions(2 * 4 * (slices)+8, 0.0f);
    positions.at(0) = (bottonCenter.x);
    positions.at(1) = (bottonCenter.y);
    positions.at(2) = (bottonCenter.z);
    positions.at(3) = (bottonCenter.w);
    positions.at((slices + 1) * 4) = (topCenter.x);
    positions.at((slices + 1) * 4 + 1) = (topCenter.y);
    positions.at((slices + 1) * 4 + 2) = (topCenter.z);
    positions.at((slices + 1) * 4 + 3) = (topCenter.w);
    norm.at(0) = -1.0f;
    norm.at(1) = 1.0f;
    norm.at(2) = -1.0f;
    norm.at(3) = 1.0f;


    float angleStep = (2.0f * 3.14159265358979323846f) / ((float)slices);

    for (int i = 1; i < (slices + 1) * 2; i++) {
        if (i < (slices + 1)) {
            positions.at(i * 4) = bottonCenter.x + cosf(angleStep * (i - 2)) * radius;
            positions.at(i * 4 + 1) = bottonCenter.y;
            positions.at(i * 4 + 2) = bottonCenter.z + sinf(angleStep * (i - 2)) * radius;
            positions.at(i * 4 + 3) = 1.0f;
        }
        else  if (i != slices + 1) {
            positions.at(i * 4) = topCenter.x + cosf(angleStep * (i - 2)) * radius;
            positions.at(i * 4 + 1) = topCenter.y;
            positions.at(i * 4 + 2) = topCenter.z + sinf(angleStep * (i - 2)) * radius;
            positions.at(i * 4 + 3) = 1.0f;
        }
    }

    for (int i = 0; i < 2 * slices + 2; i++) {
        norm.at(i * 4 + 0) = 0 - i / (slices);
        norm.at(i * 4 + 1) = -1.0f;
        norm.at(i * 4 + 2) = 0.5f + i / (slices);
        norm.at(i * 4 + 3) = 1.0f;
    }
    //for (int i = 0; i < 2 * slices + 2; i++) {
    //    std::cout << i <<": "<< positions.at(4*i)<< " " << positions.at(4*i+1)<<std::endl;
    //}
    std::vector<UINT32>indicies(slices * 12, 0.0f);

    for (unsigned int i = 1; i < slices + 1; i++) {
        indicies.at((i - 1) * 12 + 0) = 0u;
        indicies.at((i - 1) * 12 + 1) = i;
        indicies.at((i - 1) * 12 + 2) = i >= (slices) ? 1u : i + 1;

        indicies.at((i - 1) * 12 + 3) = slices + 1;
        indicies.at((i - 1) * 12 + 4) = i + slices + 1;
        indicies.at((i - 1) * 12 + 5) = i + slices + 1 >= (2 * (slices)+1) ? slices + 2 : slices + i + 2;

        indicies.at((i - 1) * 12 + 6) = i;
        indicies.at((i - 1) * 12 + 7) = slices + i + 1;
        indicies.at((i - 1) * 12 + 8) = i >= (slices) ? 1u : i + 1;

        indicies.at((i - 1) * 12 + 9) = i + slices + 1 >= (2 * (slices)+1) ? slices + 2 : slices + i + 2;
        indicies.at((i - 1) * 12 + 10) = slices + i + 1;
        indicies.at((i - 1) * 12 + 11) = i >= (slices) ? 1u : i + 1;
    }
    //for (int i = 0; i < slices * 4; i++) {
    //    std::cout << indicies.at(i*3)<<" "<<indicies.at(i*3+1)<<" "<<indicies.at(i*3+2)<<std::endl;
    //}
    std::cout << indicies.size();
    GLuint vboVertices, vboIndices, vboNormals;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(GLfloat), norm.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(int), indicies.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indicies.size();
}

GLuint createPlane(GLuint& vao, float4 position1, float4 position2, float4 position3) {
    std::vector<float> positions = {
        position1.x, position1.y, position1.z, position1.w,
        position2.x, position2.y, position2.z, position2.w,
        position3.x, position3.y, position3.z, position3.w,
    };
    positions.push_back(position1.x + position3.x - position2.x);
    positions.push_back(position1.y + position3.y - position2.y);
    positions.push_back(position1.z + position3.z - position2.z);
    positions.push_back(1.0f);
    std::vector<float> norms = {
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
    };
    std::vector<UINT32>indicies = {
        0u, 1u, 2u,
        2u, 3u, 0u
    };
    GLuint vboVertices, vboIndices, vboNormals;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(GLfloat), norms.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(int), indicies.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indicies.size();
}

GLuint loadFile(const char* path, GLuint& vao) {
    std::vector<float> vertices;
    std::vector<float> norm;
    std::vector<float> norms;
    std::vector<float> tex;
    std::vector<UINT32> indicies;
    std::vector<UINT32> normIndicies;
    FILE* file = fopen(path, "rb");

    while (true) {
        char lineHeader[128];

        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break;

        if (strcmp(lineHeader, "v") == 0) {
            float x, y, z;
            fscanf(file, "%f %f %f\n", &x, &y, &z);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(1.0f);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            float x, y;
            fscanf(file, "%f %f\n", &x, &y);
            tex.push_back(x);
            tex.push_back(y);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            float x, y, z;
            fscanf(file, "%f %f %f\n", &x, &y, &z);
            norm.push_back(x);
            norm.push_back(y);
            norm.push_back(z);
            norm.push_back(1.0f);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            //int matches = fscanf(file, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            indicies.push_back(vertexIndex[0] - 1);
            indicies.push_back(vertexIndex[1] - 1);
            indicies.push_back(vertexIndex[2] - 1);
            /*uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);*/
            normIndicies.push_back(normalIndex[0] - 1);
            normIndicies.push_back(normalIndex[1] - 1);
            normIndicies.push_back(normalIndex[2] - 1);
        }

    };

    for (int i = 0; i < normIndicies.size(); i++) {
        norms.push_back(norm.at(normIndicies.at(i) * 4));
        norms.push_back(norm.at(normIndicies.at(i) * 4 + 1));
        norms.push_back(norm.at(normIndicies.at(i) * 4 + 2));
        norms.push_back(1.0f);
    }

    GLuint vboVertices, vboIndices, vboNormals;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(GLfloat), norms.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(int), indicies.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indicies.size();
}

GLuint cave(GLuint& vao) {
    return loadFile("../models/usemtl-issue-68.obj", vao);
}

int initGL()
{
    int res = 0;

    //грузим функции opengl через glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    //выводим в консоль некоторую информацию о драйвере и контексте opengl
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    std::cout << "Controls: " << std::endl;
    std::cout << "press right mouse button to capture/release mouse cursor  " << std::endl;
    std::cout << "press spacebar to alternate between shaded wireframe and fill display modes" << std::endl;
    std::cout << "press ESC to exit" << std::endl;

    return 0;
}


int main(int argc, char** argv)
{
    if (!glfwInit())
        return -1;

    //запрашиваем контекст opengl версии 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    //регистрируем коллбеки для обработки сообщений от пользователя - клавиатура, мышь..
    glfwSetKeyCallback(window, OnKeyboardPressed);
    glfwSetCursorPosCallback(window, OnMouseMove);
    glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
    glfwSetScrollCallback(window, OnMouseScroll);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (initGL() != 0)
        return -1;

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    //создание шейдерной программы из двух файлов с исходниками шейдеров
    //используется класс-обертка ShaderProgram
    std::unordered_map<GLenum, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = "../shaders/vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "../shaders/lambert.frag";
    ShaderProgram lambert(shaders); GL_CHECK_ERRORS;

    //GLuint vaoTriangle;
    //GLsizei triangleIndices = CreateTriangle(vaoTriangle);

    GLuint vaoSphere;
    float radius = 1.0f;
    GLsizei sphereIndices = CreateSphere(radius, 118, vaoSphere);

    GLuint vaoCone;
    float4 center = float4(0.5f, 0.5f, 0.0f, 1.0f);
    float4 vertex = float4(0.5f, 0.5f, 3.0f, 1.0f);
    radius = 1.0f;
    int slices = 118;
    GLsizei coneIndices = CreateCone(vaoCone, center, vertex, radius, slices);

    GLuint vaoCylinder;
    float4 topCenter = float4(0.0f, -1.0f, 0.0f, 1.0f);
    float4 bottonCenter = float4(0.0f, -5.0f, 0.0f, 1.0f);
    radius = 0.2f;
    slices = 116;
    GLsizei cylinderIndices = createCylinder(vaoCylinder, bottonCenter, topCenter, radius, slices);

    GLuint vaoCube;
    GLsizei cubeIndices = CreateCube(vaoCube, float4(0.1f, -5.0f, 0.0f, 1.0f), float4(0.5f, -5.0f, 0.0f, 1.0f), float4(0.5f, -5.0f, 0.3f, 1.0f), float4(0.1f, -5.0f, 0.3f, 1.0f), float4(0.5f, -4.9f, 0.3f, 1.0f));

    GLuint vaoPlane;
    GLsizei planeIndices = createPlane(vaoPlane, float4(-11.0f, -10.0f, -11.0f, 1.0f), float4(-11.0f, -10.0f, 11.0f, 1.0f), float4(11.0f, -10.0f, -11.0f, 1.0f));

    GLuint vaoPlaneWings;
    GLsizei planeWings = CreateCube(vaoPlaneWings, float4(-0.5f, -0.5f, -0.5f, 1.0f), float4(0.5f, -0.5f, -0.5f, 1.0f), float4(0.5f, -0.5f, 0.5f, 1.0f), float4(-0.5f, -0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f));

    GLuint vaoCave;
    GLsizei Cave = cave(vaoCave);

    glViewport(0, 0, WIDTH, HEIGHT);  GL_CHECK_ERRORS;
    glEnable(GL_DEPTH_TEST);  GL_CHECK_ERRORS;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float move = 0.0f;
    //цикл обработки сообщений и отрисовки сцены каждый кадр
    while (!glfwWindowShouldClose(window))
    {

        //считаем сколько времени прошло за кадр
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        doCameraMovement(camera, deltaTime);

        //очищаем экран каждый кадр
        glClearColor(0.9f, 0.95f, 0.97f, 1.0f); GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;

        lambert.StartUseShader(); GL_CHECK_ERRORS;

        float4x4 view = camera.GetViewMatrix();
        float4x4 projection = projectionMatrixTransposed(camera.zoom, float(WIDTH) / float(HEIGHT), 0.1f, 1000.0f);
        float4x4 model;

        lambert.SetUniform("view", view);       GL_CHECK_ERRORS;
        lambert.SetUniform("projection", projection); GL_CHECK_ERRORS;

        //glBindVertexArray(vaoSphere);
        //{
        //  model = transpose(translate4x4(float3(0.0f, -2.0f, 0.0f)));
        //  lambert.SetUniform("model", model); GL_CHECK_ERRORS;
        //  glDrawElements(GL_TRIANGLE_STRIP, sphereIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;

        //  model = transpose(translate4x4(float3(0.0f, 2.0f, 0.0f)));
        //  lambert.SetUniform("model", model); GL_CHECK_ERRORS;
        //  glDrawElements(GL_TRIANGLE_STRIP, sphereIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
        //}
        //glBindVertexArray(0); GL_CHECK_ERRORS;

        ////glBindVertexArray(vaoTriangle); GL_CHECK_ERRORS;
        ////glBindVertexArray(vaoTriangle); GL_CHECK_ERRORS;
        ////{
        ////  model = transpose(mul(translate4x4(float3(0.0f, -1.0f, 0.0f)), rotate_Y_4x4(45.0f)));
        ////  lambert.SetUniform("model", model); GL_CHECK_ERRORS;
        ////  glDrawElements(GL_TRIANGLE_STRIP, triangleIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
        ////}
        ////glBindVertexArray(0); GL_CHECK_ERRORS;


        //glBindVertexArray(vaoCone); GL_CHECK_ERRORS;
        //glBindVertexArray(vaoCone); GL_CHECK_ERRORS;
        //{
        //    model = transpose(mul(translate4x4(float3(0.0f, 1.0f, 0.0f)), rotate_X_4x4(-45.0f)));
        //    lambert.SetUniform("model", model); GL_CHECK_ERRORS;
        //    glDrawElements(GL_TRIANGLE_STRIP, coneIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
        //}


        if (keys[GLFW_KEY_Q]) {
            glBindVertexArray(vaoCylinder); GL_CHECK_ERRORS;
            {
                //model = transpose(mul(translate4x4(float3(0.0f, -1.0f, 0.0f)), rotate_Y_4x4(45.0f)));
                lambert.SetUniform("model", model); GL_CHECK_ERRORS;
                glDrawElements(GL_TRIANGLES, cylinderIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
            }

            glBindVertexArray(vaoCube); GL_CHECK_ERRORS;
            {
                float4x4 model1 = model;
                std::vector<float4> stairs;
                for (int i = 0; i < 30; i++) {
                    model1 = transpose(mul(translate4x4(float3(0.0f, i * 0.14f, 0.0f)), rotate_Y_4x4(0.5f * i)));
                    lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
                    glDrawElements(GL_TRIANGLE_STRIP, cubeIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
                }
            }

            float radius = 2.5;
            float angle = 0.6f;

            float moveX = cosf(angle * move) * radius;
            float moveZ = sinf(angle * move) * radius;
            glBindVertexArray(vaoPlaneWings); GL_CHECK_ERRORS;
            // Рисуем крылья
            {
                float4x4 model1;
                float4x4 timeModel = mul(rotate_Y_4x4(-angle * move), translate4x4(float3(3.5f, -2.0f, 0.0f)));
                model1 = transpose(mul(timeModel, scale4x4(float3(1.5f, 0.05f, 0.3f))));
                lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
                glDrawElements(GL_TRIANGLES, planeWings, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
            }
            // Рисуем тело
            {
                float4x4 model1;
                float4x4 timeModel = mul(rotate_Y_4x4(-angle * move), translate4x4(float3(3.5f, -2.0f, 0.0f)));
                model1 = transpose(mul(timeModel, scale4x4(float3(0.3, 0.2, 1.2f))));
                lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
                glDrawElements(GL_TRIANGLE_STRIP, planeWings, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
            }
            // рисуем пропеллер
            {
                float4x4 model1;
                float4x4 timeModel = mul(translate4x4(float3(3.0f, -2.0f, 0.15f)), rotate_Z_4x4(move * 30));
                timeModel = mul(rotate_Y_4x4(-angle * move), timeModel);
                model1 = transpose(mul(timeModel, scale4x4(float3(0.09, 0.25, 0.02f))));
                lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
                glDrawElements(GL_TRIANGLES, planeWings, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
            }
            // рисуем пропеллер
            {
                float4x4 model1;
                float4x4 timeModel = mul(translate4x4(float3(4.0f, -2.0f, 0.15f)), rotate_Z_4x4(move * 30));
                timeModel = mul(rotate_Y_4x4(-angle * move), timeModel);
                model1 = transpose(mul(timeModel, scale4x4(float3(0.09, 0.25, 0.02f))));
                lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
                glDrawElements(GL_TRIANGLES, planeWings, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
            }

            //glBindVertexArray(vaoPropeller); GL_CHECK_ERRORS;
            //glBindVertexArray(vaoPropeller); GL_CHECK_ERRORS;
            //{
            //    float4x4 model1;
            //    lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
            //    glDrawElements(GL_TRIANGLE_STRIP, propeller, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
            //}


            //glBindVertexArray(vaoPropeller); GL_CHECK_ERRORS; 
            //{
            //    float4x4 model1;
            //    model1 = transpose(mul(translate4x4(float3(0.7f, 0.0f, 0.0f)), rotate_Z_4x4(move * LiteMath::DEG_TO_RAD)));
            //    lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
            //    glDrawElements(GL_TRIANGLE_STRIP, propeller, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
            //}
            move += 0.01f;
        }

        if (keys[GLFW_KEY_E]) {
            glBindVertexArray(vaoCave); GL_CHECK_ERRORS;
            {
                lambert.SetUniform("model", model); GL_CHECK_ERRORS;
                glDrawElements(GL_TRIANGLES, Cave, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
            }
        }

        lambert.StopUseShader(); GL_CHECK_ERRORS;

        glfwSwapBuffers(window);

    }

    glDeleteVertexArrays(1, &vaoCube);
    glDeleteVertexArrays(1, &vaoSphere);
    //glDeleteVertexArrays(1, &vaoTriangle);
    glDeleteVertexArrays(1, &vaoCone);
    glDeleteVertexArrays(1, &vaoCylinder);
    glDeleteVertexArrays(1, &vaoPlane);
    glfwTerminate();
    return 0;
}

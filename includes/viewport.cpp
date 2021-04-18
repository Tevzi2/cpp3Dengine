#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<fstream>
#include<vector>
#include<wtypes.h>
#include "viewport.h"
#include "vecCalc.h"
#include "vecs.h"

Viewport::Viewport(std::vector<float>& vecC, GLenum type)
{
    startVec = vecC;
    drawType = type;
    vecSize = vecC.size();
}

void convertMeshToArray(const vecs::mesh iMesh, std::vector<float>& oMesh)
{
    for (const auto& tri : iMesh.tris)
    {
        //fist vec3
        oMesh.push_back(tri.p[0].x);
        oMesh.push_back(tri.p[0].y);
        oMesh.push_back(tri.p[0].z);
        //colors for the triangle
        oMesh.push_back(tri.color.r);
        oMesh.push_back(tri.color.g);
        oMesh.push_back(tri.color.b);
        //add the normal
        oMesh.push_back(tri.normal.x);
        oMesh.push_back(tri.normal.y);
        oMesh.push_back(tri.normal.z);

        //second vec3
        oMesh.push_back(tri.p[1].x);
        oMesh.push_back(tri.p[1].y);
        oMesh.push_back(tri.p[1].z);
        //colors for the triangle
        oMesh.push_back(tri.color.r);
        oMesh.push_back(tri.color.g);
        oMesh.push_back(tri.color.b);
        //add the normal
        oMesh.push_back(tri.normal.x);
        oMesh.push_back(tri.normal.y);
        oMesh.push_back(tri.normal.z);

        //third vec3
        oMesh.push_back(tri.p[2].x);
        oMesh.push_back(tri.p[2].y);
        oMesh.push_back(tri.p[2].z);
        //colors for the triangle
        oMesh.push_back(tri.color.r);
        oMesh.push_back(tri.color.g);
        oMesh.push_back(tri.color.b);
        //add the normal
        oMesh.push_back(tri.normal.x);
        oMesh.push_back(tri.normal.y);
        oMesh.push_back(tri.normal.z);
    }
};

unsigned int Viewport::bindBuffer(std::vector<ShaderInfo> shaders, bool depthTest)
{
    float* positions = &startVec[0];

    if(depthTest == true)
        glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_FRONT);


    unsigned int buffer;

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, vecSize * sizeof(float), positions, drawType);

    unsigned int shader = createShader(shaders);

    unsigned int coords = glGetAttribLocation(shader, "position");
    unsigned int color = glGetAttribLocation(shader, "iColor");
    unsigned int normal = glGetAttribLocation(shader, "iNormal");

    glEnableVertexAttribArray(0);    
    glEnableVertexAttribArray(color);
    glEnableVertexAttribArray(normal);

    glVertexAttribPointer(
        coords,
        3,
        GL_FLOAT,
        GL_FALSE,
        9 * sizeof(float),
        0
    );

    ptrdiff_t offsetColor = 3 * sizeof(float);

    glVertexAttribPointer(
        color,
        3,
        GL_FLOAT,
        GL_FALSE,
        9 * sizeof(float),
        (void*)offsetColor
    );

    ptrdiff_t offsetNormal = 6 * sizeof(float);

    glVertexAttribPointer(
        normal,
        3,
        GL_FLOAT,
        GL_FALSE,
        9 * sizeof(float),
        (void*)offsetNormal
    );

    return shader;
}

vecs::mat4 Viewport::createWorldMatrix(float xRot, float zRot, float time)
{
    vecs::mat4 mRotX, mRotZ, mTranslation;
    mRotX = vc::rotX(xRot * time);
    mRotZ = vc::rotZ(zRot * time);
    mTranslation = vc::translationMat(0.0f, 0.0f, 4.0f);

    return mRotX * mRotZ * mTranslation;
}

vecs::mat4 Viewport::createViewMatrix(vecs::mat4 mProjMat, vecs::vec3& pos, vecs::vec3& target, vecs::vec3& up, float pitch)
{
    vecs::mat4 mCameraRotX = vc::rotX(pitch);
    vecs::mat4 mView = vc::quickInverse(mCameraRotX * pointAtMatrix(pos, target, up));
    return mView * mProjMat;
}

vecs::mat4 pointAtMatrix(vecs::vec3& pos, vecs::vec3& target, vecs::vec3& up)
{
    vecs::vec3 newForward = vc::normalize(target - pos);
    vecs::vec3 a = newForward * vc::dotPru(up, newForward);
    vecs::vec3 newUp = vc::normalize(up - a);
    vecs::vec3 newRight = vc::crossPru(newUp, newForward);

    vecs::mat4 matrix;
    matrix.r[0][0] = newRight.x;	matrix.r[0][1] = newRight.y;	matrix.r[0][2] = newRight.z;	matrix.r[0][3] = 0.0f;
    matrix.r[1][0] = newUp.x;		matrix.r[1][1] = newUp.y;		matrix.r[1][2] = newUp.z;		matrix.r[1][3] = 0.0f;
    matrix.r[2][0] = newForward.x;	matrix.r[2][1] = newForward.y;	matrix.r[2][2] = newForward.z;	matrix.r[2][3] = 0.0f;
    matrix.r[3][0] = pos.x;			matrix.r[3][1] = pos.y;			matrix.r[3][2] = pos.z;			matrix.r[3][3] = 1.0f;
    return matrix;
}

void readShader(std::string path, std::string& shader)
{
    std::string line;
    std::ifstream shaderFile(path);

    if (shaderFile.is_open())
    {
        while (std::getline(shaderFile, line))
        {
            shader += line + "\n";
        }
    }
};

void screenResolution(float& horizontal, float& vertical)
{
    RECT desktop;

    const HWND hDesktop = GetDesktopWindow();

    GetWindowRect(hDesktop, &desktop);

    horizontal = desktop.right;
    vertical = desktop.bottom;
}

unsigned int compileShader(unsigned int type, const std::string& shader)
{
    unsigned int id = glCreateShader(type);
    const char* src = shader.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        char* message = (char*)malloc(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);

        std::cout << "Shader failed to compile" << (type == GL_VERTEX_SHADER ? "vertex: " : "fragment: ") << std::endl;
        std::cout << message << std::endl;

        glDeleteShader(id);

        return 0;
    }

    return id;
};

unsigned int createShader(std::vector<ShaderInfo> shaders)
{
    unsigned int program = glCreateProgram();

    std::vector<unsigned int> delShader;

    for (const auto& shader : shaders) {
        std::string shaderSrc;

        readShader(shader.shader, shaderSrc);

        unsigned int sh = compileShader(shader.shaderType, shaderSrc);
        glAttachShader(program, sh);
        delShader.push_back(sh);
    }

    glLinkProgram(program);
    glValidateProgram(program);

    int result;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

        char* message = (char*)malloc(length * sizeof(char));
        glGetProgramInfoLog(program, length, &length, message);

        std::cout << "Program failed to validate:" << std::endl;
        std::cout << message << std::endl;

        return 0;
    }

    for (const auto& delSh : delShader)
    {
        glDeleteShader(delSh);
    }

    return program;
};
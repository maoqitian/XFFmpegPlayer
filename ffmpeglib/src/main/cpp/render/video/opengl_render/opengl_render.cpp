//
// Created by maoqitian on 2020/12/18 0018.
// Description: 并不需要自己穿件EGL（EGL 是 OpenGL ES 和本地窗口系统（Native Window System）之间的通信接口）
// 直接利用 Android GLSurfaceView 类创建 OpenGL 渲染环境，GLSurfaceView 类已经封装了 EGL 创建渲染上下文的操作，并启动了一个独立的渲染线程，完全符合渲染视频解码帧的需求
//
#include <thread>
#include <unistd.h>
#include <libavutil/mem.h>
#include <utils/logger.h>
#include "opengl_render.h"
#include <gtc/matrix_transform.hpp>
#include "GLUtils.h"
/**
 * 通过 OpenGL 可实现的加视频滤镜，如加水印、旋转缩放等效果
 */

OpenGLRender* OpenGLRender::s_Instance = nullptr;
std::mutex OpenGLRender::m_Mutex;
////顶点着色器
static char vShaderStr[] =
        "#version 300 es\n"
        "layout(location = 0) in vec4 a_position;\n"
        "layout(location = 1) in vec2 a_texCoord;\n"
        "uniform mat4 u_MVPMatrix;\n"
        "out vec2 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = u_MVPMatrix * a_position;\n"
        "    v_texCoord = a_texCoord;\n"
        "}";
////片段着色器
static char fShaderStr[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec2 v_texCoord;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "uniform sampler2D s_texture0;\n"
        "uniform sampler2D s_texture1;\n"
        "uniform sampler2D s_texture2;\n"
        "uniform int u_nImgType;// 1:RGBA, 2:NV21, 3:NV12, 4:I420\n"
        "\n"
        "void main()\n"
        "{\n"
        "\n"
        "    if(u_nImgType == 1) //RGBA\n"
        "    {\n"
        "        outColor = texture(s_texture0, v_texCoord);\n"
        "    }\n"
        "    else if(u_nImgType == 2) //NV21\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture(s_texture1, v_texCoord).a - 0.5;\n"
        "        yuv.z = texture(s_texture1, v_texCoord).r - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "        0.0, \t-0.344, \t1.770,\n"
        "        1.403,  -0.714,     0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "\n"
        "    }\n"
        "    else if(u_nImgType == 3) //NV12\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture(s_texture1, v_texCoord).r - 0.5;\n"
        "        yuv.z = texture(s_texture1, v_texCoord).a - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "        0.0, \t-0.344, \t1.770,\n"
        "        1.403,  -0.714,     0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "    }\n"
        "    else if(u_nImgType == 4) //I420\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture(s_texture1, v_texCoord).r - 0.5;\n"
        "        yuv.z = texture(s_texture2, v_texCoord).r - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "                              0.0, \t-0.344, \t1.770,\n"
        "                              1.403,  -0.714,     0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        outColor = vec4(1.0);\n"
        "    }\n"
        "}";

static char fMeshShaderStr[] =
        "//dynimic mesh 动态网格\n"
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec2 v_texCoord;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "uniform sampler2D s_texture0;\n"
        "uniform sampler2D s_texture1;\n"
        "uniform sampler2D s_texture2;\n"
        "uniform int u_nImgType;// 1:RGBA, 2:NV21, 3:NV12, 4:I420\n"
        "uniform float u_Offset;\n"
        "uniform vec2 u_TexSize;\n"
        "\n"
        "vec4 sampleImage(vec2 texCoord) {\n"
        "    vec4 outColor;\n"
        "    if(u_nImgType == 1) //RGBA\n"
        "    {\n"
        "        outColor = texture(s_texture0, texCoord);\n"
        "    }\n"
        "    else if(u_nImgType == 2) //NV21\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, texCoord).r;\n"
        "        yuv.y = texture(s_texture1, texCoord).a - 0.5;\n"
        "        yuv.z = texture(s_texture1, texCoord).r - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "        0.0,  -0.344,  1.770,\n"
        "        1.403,  -0.714,     0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "\n"
        "    }\n"
        "    else if(u_nImgType == 3) //NV12\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, texCoord).r;\n"
        "        yuv.y = texture(s_texture1, texCoord).r - 0.5;\n"
        "        yuv.z = texture(s_texture1, texCoord).a - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "        0.0,  -0.344,  1.770,\n"
        "        1.403,  -0.714,     0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "    }\n"
        "    else if(u_nImgType == 4) //I420\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, texCoord).r;\n"
        "        yuv.y = texture(s_texture1, texCoord).r - 0.5;\n"
        "        yuv.z = texture(s_texture2, texCoord).r - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "        0.0,  -0.344,  1.770,\n"
        "        1.403,  -0.714,     0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        outColor = vec4(1.0);\n"
        "    }\n"
        "    return outColor;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    vec2 imgTexCoord = v_texCoord * u_TexSize;\n"
        "    float sideLength = u_TexSize.y / 6.0;\n"
        "    float maxOffset = 0.08 * sideLength;\n"
        "    float x = mod(imgTexCoord.x, floor(sideLength));\n"
        "    float y = mod(imgTexCoord.y, floor(sideLength));\n"
        "\n"
        "    float offset = u_Offset * maxOffset;\n"
        "\n"
        "    if(offset <= x\n"
        "    && x <= sideLength - offset\n"
        "    && offset <= y\n"
        "    && y <= sideLength - offset)\n"
        "    {\n"
        "        outColor = sampleImage(v_texCoord);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        outColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "    }\n"
        "}";

static char fGrayShaderStr[] =
        "//黑白滤镜\n"
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec2 v_texCoord;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "uniform sampler2D s_TextureMap;//采样器\n"
        "void main()\n"
        "{\n"
        "    outColor = texture(s_TextureMap, v_texCoord);\n"
        "    if(v_texCoord.x > 0.5)\n"
        "        outColor = vec4(vec3(outColor.r*0.299 + outColor.g*0.587 + outColor.b*0.114), outColor.a);\n"
        "}";

GLfloat verticesCoords[] = {
        -1.0f,  1.0f, 0.0f,  // Position 0
        -1.0f, -1.0f, 0.0f,  // Position 1
        1.0f,  -1.0f, 0.0f,  // Position 2
        1.0f,   1.0f, 0.0f,  // Position 3
};

GLfloat textureCoords[] = {
        0.0f,  0.0f,        // TexCoord 0
        0.0f,  1.0f,        // TexCoord 1
        1.0f,  1.0f,        // TexCoord 2
        1.0f,  0.0f         // TexCoord 3
};

GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

OpenGLRender::OpenGLRender() : VideoRender(VIDEO_RENDER_OPENGL) {

}

//释放资源
OpenGLRender::~OpenGLRender() {
    NativeImageUtil::FreeNativeImage(&m_RenderImage);
}

//OpenGL 渲染初始化
void OpenGLRender::Init(int videoWidth, int videoHeight, int *dstSize) {
    LOGCATE("OpenGLRender::InitRender video[w, h]=[%d, %d]", videoWidth, videoHeight);
    //初始化视频图像的宽和高

    if (dstSize != nullptr){
        dstSize[0] = videoWidth;
        dstSize[1] = videoHeight;
    }
    m_FrameIndex = 0;
    UpdateMVPMatrix(0,0,1.0f,1.0f);
}

//视频渲染
void OpenGLRender::RenderVideoFrame(NativeImage *pImage) {
    // 接收解码后的视频帧
    LOGCATE("OpenGLRender::RenderVideoFrame pImage=%p", pImage);
    if(pImage == nullptr || pImage->ppPlane[0] == nullptr) return;

    //加互斥锁，解码线程和渲染线程是 2 个不同的线程，避免数据访问冲突
    std::unique_lock<std::mutex> lock(m_Mutex);
    if(pImage -> width != m_RenderImage.width || pImage -> height != m_RenderImage.height ){
        if(m_RenderImage.ppPlane[0] != nullptr){
            NativeImageUtil::FreeNativeImage(&m_RenderImage);
        }
        memset(&m_RenderImage,0,sizeof (NativeImage));

        m_RenderImage.format = pImage->format;
        m_RenderImage.width = pImage->width;
        m_RenderImage.height = pImage->height;
        NativeImageUtil::AllocNativeImage(&m_RenderImage);
    }

    NativeImageUtil::CopyNativeImage(pImage,&m_RenderImage);

}

void OpenGLRender::UnInit() {
     ReleaseInstance();
}

/**
 * 对应 Java 层 GLSurfaceView.Renderer 的三个接口
 */
void OpenGLRender::OnSurfaceCreated() {
    LOGCATE("OpenGLRender::OnSurfaceCreated");

    m_ProgramObj = GLUtils::CreateProgram(vShaderStr,fShaderStr);
    if (!m_ProgramObj){
        LOGCATE("OpenGLRender::OnSurfaceCreated create program fail");
        return;
    }

    glGenTextures(TEXTURE_NUM,m_TextureIds);
    for (int i = 0; i < TEXTURE_NUM ; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }
    // Generate VBO Ids and load the VBOs with data
    glGenBuffers(3, m_VboIds);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Generate VAO Id
    glGenVertexArrays(1, &m_VaoId);
    glBindVertexArray(m_VaoId);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);

    glBindVertexArray(GL_NONE);

    m_TouchXY = vec2(0.5f, 0.5f);
}

void OpenGLRender::OnSurfaceChanged(int w, int h) {
    LOGCATE("OpenGLRender::OnSurfaceChanged [w, h]=[%d, %d]", w, h);

    m_ScreenSize.x = w;
    m_ScreenSize.y = h;
    glViewport(0,0,w,h);
    glClearColor(1.0f,1.0f,1.0f,1.0f);
}

void OpenGLRender::OnDrawFrame() {
    //使用 OpenGL YUV 到 RGBA 的格式转换放到 shader 里，用 GPU 来实现格式转换，提升渲染效率
    glClear(GL_COLOR_BUFFER_BIT);
    if(m_ProgramObj == GL_NONE|| m_RenderImage.ppPlane[0] == nullptr) return;
    LOGCATE("OpenGLRender::OnDrawFrame [w, h]=[%d, %d], format=%d", m_RenderImage.width, m_RenderImage.height, m_RenderImage.format);
    m_FrameIndex++;

    //upload image data 视频图像数据上传
    std::unique_lock<std::mutex> lock(m_Mutex);
    //不同格式处理
    switch (m_RenderImage.format) {
        case IMAGE_FORMAT_RGBA:
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);
            break;
        case IMAGE_FORMAT_NV12:
        case IMAGE_FORMAT_NV21:
            //upload Y plane data
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_RenderImage.width,
                         m_RenderImage.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         m_RenderImage.ppPlane[0]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);

            //update UV plane data
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, m_RenderImage.width >> 1,
                         m_RenderImage.height >> 1, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
                         m_RenderImage.ppPlane[1]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);
            break;
        case IMAGE_FORMAT_I420:
            //upload Y plane data
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_RenderImage.width,
                         m_RenderImage.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         m_RenderImage.ppPlane[0]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);

            //update U plane data
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_RenderImage.width >> 1,
                         m_RenderImage.height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         m_RenderImage.ppPlane[1]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);

            //update V plane data
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[2]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_RenderImage.width >> 1,
                         m_RenderImage.height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         m_RenderImage.ppPlane[2]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);
            break;
        default:
            break;
    }
    lock.unlock();

    // Use the program object
    glUseProgram (m_ProgramObj);

    glBindVertexArray(m_VaoId);

    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

    for (int i = 0; i < TEXTURE_NUM; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        char samplerName[64] = {0};
        sprintf(samplerName, "s_texture%d", i);
        GLUtils::setInt(m_ProgramObj, samplerName, i);
    }

    //float time = static_cast<float>(fmod(m_FrameIndex, 60) / 50);
    //GLUtils::setFloat(m_ProgramObj, "u_Time", time);

    float offset = (sin(m_FrameIndex * MATH_PI / 40) + 1.0f) / 2.0f;
    GLUtils::setFloat(m_ProgramObj, "u_Offset", offset);
    GLUtils::setVec2(m_ProgramObj, "u_TexSize", vec2(m_RenderImage.width, m_RenderImage.height));
    GLUtils::setInt(m_ProgramObj, "u_nImgType", m_RenderImage.format);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);

}

// 单例模式，全局只有一个 OpenGLRender
OpenGLRender *OpenGLRender::GetInstance() {
    if(s_Instance == nullptr){
        std::lock_guard<std::mutex> lock(m_Mutex);
        if(s_Instance == nullptr){

            s_Instance = new OpenGLRender();
        }
    }
    return s_Instance;
}
// 释放静态实例
void OpenGLRender::ReleaseInstance() {
    if(s_Instance != nullptr){
        std::lock_guard<std::mutex> lock(m_Mutex);
        if(s_Instance != nullptr){

           delete s_Instance;
           s_Instance = nullptr;
        }
    }
}
// 设置变换矩阵，控制图像的旋转缩放
void OpenGLRender::UpdateMVPMatrix(int angleX, int angleY, float scaleX, float scaleY) {
    angleX = angleX % 360;
    angleY = angleY % 360;

    //转化为弧度角
    float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
    float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);
    // Projection matrix
    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    //glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
    //glm::mat4 Projection = glm::perspective(45.0f,ratio, 0.1f,100.f);

    // View matrix
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 4), // Camera is at (0,0,1), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(scaleX, scaleY, 1.0f));
    Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
    Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
    Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

    m_MVPMatrix = Projection * View * Model;
}

void OpenGLRender::UpdateMVPMatrix(TransformMatrix *pTransformMatrix) {
    float fFactorX = 1.0f;
    float fFactorY = 1.0f;

    if (pTransformMatrix->mirror == 1) {
        fFactorX = -1.0f;
    } else if (pTransformMatrix->mirror == 2) {
        fFactorY = -1.0f;
    }

    float fRotate = MATH_PI * pTransformMatrix->degree * 1.0f / 180;
    if (pTransformMatrix->mirror == 0) {
        if (pTransformMatrix->degree == 270) {
            fRotate = MATH_PI * 0.5;
        } else if (pTransformMatrix->degree == 180) {
            fRotate = MATH_PI;
        } else if (pTransformMatrix->degree == 90) {
            fRotate = MATH_PI * 1.5;
        }
    } else if (pTransformMatrix->mirror == 1) {
        if (pTransformMatrix->degree == 90) {
            fRotate = MATH_PI * 0.5;
        } else if (pTransformMatrix->degree == 180) {
            fRotate = MATH_PI;
        } else if (pTransformMatrix->degree == 270) {
            fRotate = MATH_PI * 1.5;
        }
    }

    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 1), // Camera is at (0,0,1), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0) // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(fFactorX * pTransformMatrix->scaleX,
                                        fFactorY * pTransformMatrix->scaleY, 1.0f));
    Model = glm::rotate(Model, fRotate, glm::vec3(0.0f, 0.0f, 1.0f));
    Model = glm::translate(Model,
                           glm::vec3(pTransformMatrix->translateX, pTransformMatrix->translateY, 0.0f));

    LOGCATE("OpenGLRender::UpdateMVPMatrix rotate %d,%.2f,%0.5f,%0.5f,%0.5f,%0.5f,", pTransformMatrix->degree, fRotate,
            pTransformMatrix->translateX, pTransformMatrix->translateY,
            fFactorX * pTransformMatrix->scaleX, fFactorY * pTransformMatrix->scaleY);

    m_MVPMatrix = Projection * View * Model;
}

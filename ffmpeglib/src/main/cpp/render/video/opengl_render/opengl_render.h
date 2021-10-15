//
// Created by maoqitian on 2020/12/18 0018.
// Description: 视频 OpenGL ES 渲染器
//

#ifndef FFMPEGPLAYER_OPENGL_RENDER_H
#define FFMPEGPLAYER_OPENGL_RENDER_H
#include <jni.h>
#include "../video_render.h"
#include "BaseGLRender.h"
#include "vec2.hpp"
#include <GLES3/gl3.h>
#include <thread>
#include "ImageDef.h"
#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>


using namespace glm;

#define MATH_PI 3.1415926535897932384626433832802
#define TEXTURE_NUM 3

class OpenGLRender :  public VideoRender,public BaseGLRender{
private:

    OpenGLRender();
    virtual ~OpenGLRender();

    static std::mutex m_Mutex;
    static OpenGLRender* s_Instance;
    GLuint m_ProgramObj = GL_NONE;
    GLuint m_TextureIds[TEXTURE_NUM];
    GLuint m_VaoId;
    GLuint m_VboIds[3];
    NativeImage m_RenderImage;
    //变化矩阵
    glm::mat4 m_MVPMatrix;

    int m_FrameIndex;
    vec2 m_TouchXY;
    vec2 m_ScreenSize;

public:

    virtual void Init(int videoWidth, int videoHeight, int *dstSize); //虚函数 实现子类调用实现
    virtual void RenderVideoFrame(NativeImage *pImage);
    virtual void UnInit();

    //对应 Java 层 GLSurfaceView.Renderer 的三个接口
    void OnSurfaceCreated();
    void OnSurfaceChanged(int w, int h);
    void OnDrawFrame();

    //静态实例管理
    static OpenGLRender *GetInstance();
    static void ReleaseInstance();

    //设置变换矩阵，控制图像的旋转缩放
    virtual void UpdateMVPMatrix(int angleX, int angleY, float scaleX, float scaleY);
    virtual void UpdateMVPMatrix(TransformMatrix * pTransformMatrix);
    virtual void SetTouchLoc(float touchX, float touchY) {
        m_TouchXY.x = touchX / m_ScreenSize.x;
        m_TouchXY.y = touchY / m_ScreenSize.y;
    }
};


#endif //FFMPEGPLAYER_OPENGL_RENDER_H

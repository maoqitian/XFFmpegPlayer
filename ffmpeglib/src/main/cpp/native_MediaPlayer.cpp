//
// Created by llm on 20-3-21.
//

#include <jni.h>
#include <cassert>
#include <android/log.h>
extern "C" {
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <include/libavutil/imgutils.h>
}
#include <memory>
#include <mutex>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "MediaPlayerListener.h"
#include "FFMediaPlayer.h"
#include "macro.h"
#include "FFLog.h"

//#define LOG_TAG "FFMediaPlayer-JNI"
//#define ALOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
//#define ALOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
//#define ALOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
//#define ALOGD(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

using namespace std;

//指定类的路径，通过FindClass 方法来找到对应的类
const char* className  = "com/mao/ffplayer/FFMediaPlayer";

struct fields_t {
    jfieldID    context;     //将native层的player设置给java层
    jfieldID    surface_texture;

    jmethodID   post_event;
};

static fields_t fields;

static mutex sLock;

JavaVM *javaVM = NULL;
jobject jSurface;

void jniThrowException(JNIEnv *env, const char *name, const char *msg)
{
    jclass cls = env->FindClass(name);
    if (cls != NULL) {
        env->ThrowNew(cls, msg);
    }
    env->DeleteLocalRef(cls);
}


//---------------------------------------------------------------------------------------------------------

// ref-counted object for callbacks
class JNIMediaPlayerListener: public MediaPlayerListener
{
public:
    JNIMediaPlayerListener(JNIEnv* env, jobject thiz, jobject weak_thiz);
    ~JNIMediaPlayerListener();
    virtual void notify(int msg, int ext1, int ext2);
private:
    JNIMediaPlayerListener();
    jclass      mClass;     // Reference to MediaPlayer class
    jobject     mObject;    // Weak ref to MediaPlayer Java object to call on
};

JNIMediaPlayerListener::JNIMediaPlayerListener(JNIEnv* env, jobject thiz, jobject weak_thiz)
{

    // Hold onto the MediaPlayer class for use in calling the static method
    // that posts events to the application thread.
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        ALOGE("Can't find com/mao/ffplayer/FFMediaPlayer");
        jniThrowException(env, "java/lang/Exception", NULL);
        return;
    }

    mClass = (jclass)env->NewGlobalRef(clazz);

    // We use a weak reference so the MediaPlayer object can be garbage collected.
    // The reference is only used as a proxy for callbacks.
    mObject  = env->NewGlobalRef(weak_thiz);

    ALOGE("JNIMediaPlayerListener contruct");
}

JNIMediaPlayerListener::~JNIMediaPlayerListener()
{
    JNIEnv *env = NULL;

    int status = javaVM->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (status < 0) {
        javaVM->AttachCurrentThread(&env, NULL);
        ALOGE("AttachCurrentThread");
    }
    // remove global referencesgi
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);

    if (status < 0) {
        javaVM->DetachCurrentThread();
    }

    ALOGE("JNIMediaPlayerListener destruct");
}

void JNIMediaPlayerListener::notify(int msg, int ext1, int ext2) {

    JNIEnv *env = NULL;

    int status = javaVM->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (status < 0) {
        javaVM->AttachCurrentThread(&env, NULL);
        ALOGE("AttachCurrentThread");
    }

    env->CallStaticVoidMethod(mClass, fields.post_event, mObject,
                              msg, ext1, ext2);

    if (env->ExceptionCheck()) {
        ALOGW("An exception occurred while notifying an event.");
        env->ExceptionClear();
    }

    if (status < 0) {
        javaVM->DetachCurrentThread();
        ALOGE("DetachCurrentThread");
    }

}
//---------------------------------------------------------------------------------------------------------


static FFMediaPlayer* getMediaPlayer(JNIEnv* env, jobject thiz)
{
    std::lock_guard<std::mutex> lock(sLock);
    auto* const p = (FFMediaPlayer*)env->GetLongField(thiz, fields.context);
    return p;
}

static void setMediaPlayer(JNIEnv* env, jobject thiz, FFMediaPlayer* player)
{
    std::lock_guard<std::mutex> lock(sLock);
    env->SetLongField(thiz, fields.context, (jlong)player);
}
//---------------------------------------------------------------------------------------------------------------



static void com_mao_ffplayer_FFMediaPlayer_native_init(JNIEnv *env) {
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        return;
    }

    fields.context = env->GetFieldID(clazz, "mNativeContext", "J");
    if (fields.context == NULL) {
        return;
    }

    fields.post_event = env->GetMethodID(clazz, "postEventFromNative",
                                               "(Ljava/lang/Object;III)V");
    if (fields.post_event == NULL) {
        return;
    }

    fields.surface_texture = env->GetFieldID(clazz, "mNativeSurfaceTexture", "J");
    if (fields.surface_texture == NULL) {
        return;
    }

    env->DeleteLocalRef(clazz);
}

static void com_mao_ffplayer_FFMediaPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this) {
    ALOGI("native_setup");

    //TODO:没有用智能指针，需要主动delete
    //c++11 的share_ptr 和Android sp指针有一点差别，无法手动增加计数
    FFMediaPlayer *mp = new FFMediaPlayer();
    if (mp == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return;
    }

    // create new listener and give it to MediaPlayer
    shared_ptr<JNIMediaPlayerListener> listener(new JNIMediaPlayerListener(env, thiz, weak_this));
    shared_ptr<MediaPlayerListener> baseListener = dynamic_pointer_cast<JNIMediaPlayerListener>(listener);
    mp->setListener(baseListener);

    //将FFMediaPlayer指针地址保存到java层对应的对象实例实例中
    // Stow our new C++ MediaPlayer in an opaque field in the Java object.
    setMediaPlayer(env, thiz, mp);
}

static void com_mao_ffplayer_FFMediaPlayer_native_setDataSource(JNIEnv *env, jobject thiz, jstring path) {
    ALOGI("native_setDataSource");

    FFMediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    //将fileDescriptor 转化为native层能识别的文件fd
    if (path == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }

    const char *dataSource = env->GetStringUTFChars(path, 0);

    int ret = mp->setDataSource(const_cast<char *>(dataSource));
    if (ret != STATUS_OK) {
        jniThrowException(env, "java/io/IOException", NULL);
    }

    env->ReleaseStringUTFChars(path, dataSource);
}

static void com_mao_ffplayer_FFMediaPlayer_native_prepareAsync(JNIEnv *env, jobject thiz) {
    ALOGI("native_prepareAsync");

    FFMediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    ANativeWindow* window = (ANativeWindow*) env->GetLongField(thiz, fields.surface_texture);
    mp->setSurface(window);

    int ret = mp->prepareAsync();
    if (ret != STATUS_OK) {
        jniThrowException(env, "java/io/IOException", NULL);
    }

}

static void com_mao_ffplayer_FFMediaPlayer_native_testCallback(JNIEnv *env, jobject thiz, jboolean bNewThread) {
    FFMediaPlayer *mp = getMediaPlayer(env, thiz);
    mp->testCallback(bNewThread);


    if (bNewThread) {
   //     mp->setListener(0);
    }
}

static void com_mao_ffplayer_FFMediaPlayer_native_setSurface(JNIEnv *env, jobject thiz, jobject surface) {
    FFMediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    ANativeWindow *window = (ANativeWindow *)env->GetLongField(thiz, fields.surface_texture);
    if (window != NULL) {
        ANativeWindow_release(window);
    }

    //创建新的窗口用于视频显示
    window = ANativeWindow_fromSurface(env, surface);

    env->SetLongField(thiz, fields.surface_texture, (jlong)window);
    mp->setSurface(window);
}

static void com_mao_ffplayer_FFMediaPlayer_native_start(JNIEnv *env, jobject thiz) {
    FFMediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        ALOGE("mp null");
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    int ret = mp->start();
    if (ret != STATUS_OK) {
        ALOGE("mp start ret=%d", ret);
        jniThrowException(env, "java/io/IOException", NULL);
    }
}

static void com_mao_ffplayer_FFMediaPlayer_native_stop(JNIEnv *env, jobject thiz) {
    FFMediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        ALOGE("mp null");
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    int ret = mp->stop();
    if (ret != STATUS_OK) {
        ALOGE("mp start ret=%d", ret);
        jniThrowException(env, "java/io/IOException", NULL);
    }
}

static void com_mao_ffplayer_FFMediaPlayer_native_reset(JNIEnv *env, jobject thiz) {
    FFMediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        ALOGE("mp null");
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    int ret = mp->reset();
    if (ret != STATUS_OK) {
        ALOGE("mp start ret=%d", ret);
        jniThrowException(env, "java/io/IOException", NULL);
    }

    ANativeWindow *window = (ANativeWindow *)env->GetLongField(thiz, fields.surface_texture);
    if (window != NULL) {
        ANativeWindow_release(window);
        env->SetLongField(thiz, fields.surface_texture, 0);
    }
}

static void com_mao_ffplayer_FFMediaPlayer_native_release(JNIEnv *env, jobject thiz) {

    env->SetLongField(thiz, fields.surface_texture,  0);

    FFMediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp != NULL) {
        mp->setListener(NULL);
        mp->reset();

        ANativeWindow *window = (ANativeWindow *)env->GetLongField(thiz, fields.surface_texture);
        if (window != NULL) {
            ANativeWindow_release(window);
            env->SetLongField(thiz, fields.surface_texture, 0);
        }

        setMediaPlayer(env, thiz, 0);
        delete mp;
    }
}

static void com_mao_ffplayer_FFMediaPlayer_native_pause(JNIEnv *env, jobject thiz) {

    FFMediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        ALOGE("mp null");
        jniThrowException(env, "java/lang/IllegalStateException", 0);
        return;
    }

    int ret = mp->pause();
    if (ret != STATUS_OK) {
        ALOGE("mp pause ret=%d", ret);
        jniThrowException(env, "java/io/IOException", nullptr);
    }

}

//测试视频播放流程
static jint com_mao_ffplayer_FFMediaPlayer_native_playVideo(JNIEnv *env, jobject thiz,jstring url, jstring surface) {
    ALOGI("native_playVideo");
    //视频 url 地址
    const char *dataSource = env->GetStringUTFChars(url, 0);
    ALOGW("start playvideo... url, %s", dataSource);
    //1. 注册 所有容器和格式
    av_register_all();
    avformat_network_init();

    AVFormatContext * mFormatCtx = avformat_alloc_context();

    //2. 打开文件
    if(avformat_open_input(&mFormatCtx,dataSource,nullptr,nullptr)!=0){
        ALOGE("Couldn't open file:%s\n", dataSource);
        return -1;
    }
    //3. 获取流信息
    if(avformat_find_stream_info(mFormatCtx,nullptr) < 0){
       ALOGE("获取流信息失败");
       return -1;
    }
    //4.Find the first video stream 找到第一个视频流
    int videoStream = -1,i;
    for( i = 0; i< mFormatCtx->nb_streams;i++){
        if(mFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStream<0){
            videoStream = i;
        }
    }
    if(videoStream == -1){
        ALOGE("无法获取视频流");
        return -1;
    }
    //获取一个指向视频流的编解码器上下文的指针
    AVCodecContext * mCodecCtx = avcodec_alloc_context3(nullptr);
    //5. 查找解码器
    AVCodec *mCodec = avcodec_find_decoder(mCodecCtx->codec_id);
    if(mCodec == nullptr){
        ALOGE("编码器获取失败");
        return -1;
    }
    //6.打开编码器
    if(avcodec_open2(mCodecCtx,mCodec,nullptr)<0){
        ALOGE("打开编码器失败");
        return -1;
    }
    //获取 native window
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env,surface);

    //视频宽高
    int videoWidth = mCodecCtx -> width;
    int videoHeight = mCodecCtx -> height;

    // 设置native window的buffer大小,可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow,videoWidth,videoHeight,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;

    if(avcodec_open2(mCodecCtx,mCodec,nullptr)<0){
        ALOGE("打开编码器失败");
        return -1;
    }

    //7. 分配视频帧内存
    AVFrame *mFrame = av_frame_alloc();

    //渲染帧
    AVFrame *mFrameRGBA = av_frame_alloc();

    if(mFrame == nullptr || mFrameRGBA == nullptr){
        ALOGE("分配视频帧内存失败");
        return -1;
    }

    // 确定所需的缓冲区大小并分配缓冲区 buffer 数据用于渲染 格式为 WINDOW_FORMAT_RGBA_8888
    int numBytes=av_image_get_buffer_size(AV_PIX_FMT_RGBA, mCodecCtx->width, mCodecCtx->height, 1);
    auto * buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    av_samples_fill_arrays(mFrameRGBA->data, mFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         mFrameRGBA->width, static_cast<AVSampleFormat>(mFrameRGBA->height), 1);

    //渲染前解码格式转换为 RGBA
    struct  SwsContext *swsContext = sws_getContext(mCodecCtx->width,
                                                    mCodecCtx->height,
                                                    mCodecCtx->pix_fmt,
                                                    mCodecCtx->width,
                                                    mCodecCtx->height,
                                                    AV_PIX_FMT_RGBA,
                                                    SWS_BILINEAR,nullptr,nullptr,nullptr);
    // 8.从码流中获取帧数据
    int frameFinished;
    AVPacket packet;

    while(av_read_frame(mFormatCtx,&packet)>=0){
        if(packet.stream_index == videoStream){ //确定是视频流数据
           //9. 解码视频帧
           avcodec_decode_video2(mCodecCtx,mFrame,&frameFinished,&packet);

           if(frameFinished){
               //lock buff
               ANativeWindow_lock(nativeWindow,&windowBuffer,0);

               //format scale
               sws_scale(swsContext,(uint8_t const * const *)mFrame->data,
                         mFrame->linesize, 0, mCodecCtx->height,
                         mFrameRGBA->data, mFrameRGBA->linesize);

               // 获取stride
               auto * dst = windowBuffer.bits;
               int dstStride = windowBuffer.stride * 4;
               auto * src = (uint8_t*) (mFrameRGBA->data[0]);
               int srcStride = mFrameRGBA->linesize[0];

               // 由于window的stride和帧的stride不同,因此需要逐行复制
               int h;
               for (h = 0; h < videoHeight; h++) {
                   memcpy((uint8_t *)dst + h * dstStride, src + h * srcStride, srcStride);
               }
               ANativeWindow_unlockAndPost(nativeWindow);
           }
        }
        av_packet_unref(&packet);
    }
    //10. 资源释放
    av_free(buffer);
    av_free(mFrameRGBA);

    av_free(mFrame);
    //解码器释放
    avcodec_close(mCodecCtx);

    //11.关闭文件输入
    avformat_close_input(&mFormatCtx);
    return 0;
}

//动态注册 native 方法
static JNINativeMethod gMethods[] = {
        {"native_init",         "()V",                              (void *)com_mao_ffplayer_FFMediaPlayer_native_init},
        {"native_setup",        "(Ljava/lang/Object;)V",            (void *)com_mao_ffplayer_FFMediaPlayer_native_setup},
        {"native_setDataSource",        "(Ljava/lang/String;)V",            (void *)com_mao_ffplayer_FFMediaPlayer_native_setDataSource},
        {"native_prepareAsync",        "()V",            (void *)com_mao_ffplayer_FFMediaPlayer_native_prepareAsync},
        {"native_testCallback",        "(Z)V",            (void *)com_mao_ffplayer_FFMediaPlayer_native_testCallback},
        {"native_setSurface",        "(Ljava/lang/Object;)V",            (void *)com_mao_ffplayer_FFMediaPlayer_native_setSurface},
        {"native_start",        "()V",            (void *)com_mao_ffplayer_FFMediaPlayer_native_start},
        {"native_stop",        "()V",            (void *)com_mao_ffplayer_FFMediaPlayer_native_stop},
        {"native_reset",        "()V",            (void *)com_mao_ffplayer_FFMediaPlayer_native_reset},
        {"native_release",        "()V",            (void *)com_mao_ffplayer_FFMediaPlayer_native_release},
        {"native_pause",        "()V",            (void *)com_mao_ffplayer_FFMediaPlayer_native_pause},
        {"native_playVideo",        "(Ljava/lang/String;Ljava/lang/Object;)I",  (void *)com_mao_ffplayer_FFMediaPlayer_native_playVideo},
};

static int registerNativeMethods(JNIEnv* env, const char* className, JNINativeMethod* gMethods, int methodsNum) {
    jclass clazz;
    //找到声明native方法的类
    clazz = env->FindClass(className);
    if(clazz == NULL){
        return JNI_FALSE;
    }
    //注册函数 参数：java类 所要注册的函数数组 注册函数的个数
    if(env->RegisterNatives(clazz, gMethods, methodsNum) < 0){
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static int register_native_MediaPlayer(JNIEnv* env){
    //指定类的路径，通过FindClass 方法来找到对应的类
    const char* className  = "com/mao/ffplayer/FFMediaPlayer";
    return registerNativeMethods(env, className, gMethods, sizeof(gMethods)/ sizeof(gMethods[0]));
}

jint JNI_OnLoad(JavaVM* vm, void* /* reserved */)
{
    JNIEnv* env = NULL;
    jint result = -1;

    javaVM = vm;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_native_MediaPlayer(env) < 0) {
        ALOGE("ERROR: MediaPlayer native registration failed");
        goto bail;
    }
    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

    bail:
    return result;
 }

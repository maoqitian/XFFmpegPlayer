//
// Created by maoqitian on 2020/12/18 0018.
// Description: 绘制代理接口
//

#ifndef FFMPEGPLAYER_DRAWER_PROXY_H
#define FFMPEGPLAYER_DRAWER_PROXY_H

#include "../drawer.h"
class DrawerProxy {
public:
    virtual void AddDrawer(Drawer *drawer) = 0;
    virtual void Draw() = 0;
    virtual void Release() = 0;
    virtual ~DrawerProxy() {}
};

};


#endif //FFMPEGPLAYER_DRAWER_PROXY_H

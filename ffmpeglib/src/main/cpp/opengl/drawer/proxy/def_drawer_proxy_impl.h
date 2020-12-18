//
// Created by maoqitian on 2020/12/18 0018.
// Description: 默认渲染代理
//

#ifndef FFMPEGPLAYER_DEF_DRAWER_PROXY_IMPL_H
#define FFMPEGPLAYER_DEF_DRAWER_PROXY_IMPL_H


#include "drawer_proxy.h"
#include <vector>

class DefDrawerProxyImpl: public DrawerProxy {

private:
    std::vector<Drawer *> m_drawers;

public:
    void AddDrawer(Drawer *drawer);
    void Draw() override;
    void Release() override;
};


#endif //FFMPEGPLAYER_DEF_DRAWER_PROXY_IMPL_H

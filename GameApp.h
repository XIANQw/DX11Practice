#pragma once
#include "d3dApp.h"

class GameApp : public D3DApp
{
public:
    GameApp(HINSTANCE hinstance);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();
};


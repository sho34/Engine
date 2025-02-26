#pragma once
#pragma once

extern void SetupRenderPipeline();
extern void DestroyRenderPipeline();
extern void RunRender();
extern void GameStep();
extern void GameDestroy();
extern void GetAudioListenerVectors(std::function<void(XMFLOAT3 pos, XMFLOAT3 fw, XMFLOAT3 up)>);
extern void WindowResizeReleaseResources();
extern void WindowResize(unsigned int width, unsigned int height);
#pragma once

#include "resource.h"

enum GameStates {
	GS_None,
#if defined(_EDITOR)
	GS_Editor,
	GS_EditorPlaying,
#endif
	GS_Booting,
	GS_Loading,
	GS_Playing,
	GS_Destroy
};

void GameStep();
void GameDestroy();
void RunRender();
void PostRender();
void WindowResizeReleaseResources();
void WindowResize(unsigned int width, unsigned int height);
void RunPreRenderComputeShaders();
void RunPostRenderComputeShaders();
void GetAudioListenerVectors(std::function<void(XMFLOAT3, XMVECTOR)>);

//Booting
void BootScreenCreate(GameStates prevState);
void BootScreenLeave(GameStates nextState);
void BootScreenStep();
void BootScreenRender();

//Loading
void LoadingScreenCreate(GameStates prevState);
void LoadingScreenLeave(GameStates nextState);
void LoadingScreenStep();
void LoadingScreenRender();

//Playing
void PlayModeCreate(GameStates prevState);
void PlayModeLeave(GameStates nextState);
void PlayModeStep();
void PlayModeRender();

#if defined(_EDITOR)
void DestroyEditorModeBindings();
void CreateEditorIndependentCamera();
void SwitchToEditorCamera();
void SwitchToEditorPlayCamera();
void DestroyEditorCameras();
void ReloadSceneFromPrePlay();

//Editor
void EditorModeCreate(GameStates prevState);
void EditorModeStep();
void EditorModeRender();
void EditorModePostRender();
void EditorModeLeave(GameStates nextState);

//EditorPlaying
void EditorPlayingModeCreate(GameStates prevState);
void EditorPlayingModeStep();
void EditorPlayingModeRender();
void EditorPlayingModePostRender();
void EditorPlayingModeLeave(GameStates nextState);
#endif
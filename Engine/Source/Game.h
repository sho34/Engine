#pragma once

#include "GameDecl.h"
#include "Controllers/Controller.h"

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

//Controller
std::vector<std::string> Game::GetGameControllers();
//std::unique_ptr<Game::Controller> Game::GetGameController(std::string name);
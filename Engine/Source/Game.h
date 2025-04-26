#pragma once

#include "GameDecl.h"

enum GameStates {
	GS_None,
#if defined(_EDITOR)
	GS_Editor,
#endif
	GS_Booting,
	GS_MainMenu,
	GS_Loading,
	GS_Playing,
	GS_Destroy
};

//Booting
void BootScreenCreate();
void BootScreenStep();
void BootScreenRender();
void BootScreenLeave();

//Loading
void LoadingScreenCreate();
void LoadingScreenLeave();
void LoadingScreenStep();
void LoadingScreenRender();

//Playing
void PlayModeCreate();
void PlayModeLeave();
void PlayModeStep();
void PlayModeRender();

#if defined(_EDITOR)
//Editor
void EditorModeCreate();
void EditorModeStep();
void EditorModeRender();
void EditorModePostRender();
void EditorModeLeave();
#endif
#pragma once

#include "EquipmentManager.h"
#include "ScriptManager.h"
#include "SettingManager.h"
#include "CommandManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "RenderManager.h"
#include "GameEventManager.h"
#include "GameMenuManager.h"
#include "KeyboardManager.h"
#include "ShadowManager.h"
#include "OcclusionManager.h"

extern SettingManager*		TheSettingManager;
extern CommandManager*		TheCommandManager;
extern TextureManager*		TheTextureManager;
extern ShaderManager*		TheShaderManager;
extern RenderManager*		TheRenderManager;
extern GameMenuManager*		TheGameMenuManager;
extern KeyboardManager*		TheKeyboardManager;
extern GameEventManager*	TheGameEventManager;
extern ShadowManager*		TheShadowManager;
extern OcclusionManager*	TheOcclusionManager;
extern EquipmentManager*	TheEquipmentManager;
extern ScriptManager*		TheScriptManager;

void InitializeManagers();
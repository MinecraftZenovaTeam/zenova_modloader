#pragma once
#include "AppUtils.h"

// Export functions with unmangled names
#define EXPORT extern "C" __declspec(dllexport)

typedef int(__stdcall * Callback)(const int state);

void CreateMinecraftInstance();

EXPORT void LaunchMinecraft(bool forceRestart);
EXPORT int GetMinecraftExecutionState();
EXPORT void OpenMinecraftFolder();
EXPORT void OpenModsFolder();
EXPORT void SetStateChangeCallback(Callback handler);
EXPORT void UnregisterStateChanges();
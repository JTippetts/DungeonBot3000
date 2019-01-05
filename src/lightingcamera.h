// Lighting and Camera
#pragma once

#include <Urho3D/Scene/Component.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Scene/Scene.h>

using namespace Urho3D;

bool LoadLightingAndCamera(Scene *scene, String levelpath);

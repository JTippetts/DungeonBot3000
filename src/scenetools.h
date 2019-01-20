// Scene-building tools
#pragma once

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/Context.h>

using namespace Urho3D;

SharedPtr<Scene> CreateLevel(Context *context, String levelpath);


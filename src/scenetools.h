// Scene-building tools
#pragma once

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/Context.h>

using namespace Urho3D;

Node *SpawnObject(Scene* scene, const String &name, Vector3 pos=Vector3(0,0,0), Quaternion rot=Quaternion(0,Vector3(0,1,0)));
SharedPtr<Scene> CreateLevel(Context *context, String levelpath, unsigned int level, unsigned int previouslevel);


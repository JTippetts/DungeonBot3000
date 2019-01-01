#pragma once
#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/JSONValue.h>
#include <Urho3D/Container/Str.h>

using namespace Urho3D;

// JSON loading utilities
String GetStringFromJSONObject(String name, const JSONObject &obj);
double GetDoubleFromJSONObject(String name, const JSONObject &obj);
bool GetBoolFromJSONObject(String name, const JSONObject &obj);

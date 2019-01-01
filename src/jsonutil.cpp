#include "jsonutil.h"
#include <Urho3D/IO/Log.h>

String GetStringFromJSONObject(String name, const JSONObject &obj)
{
	const JSONValue *v=obj[name];
	if(!v) Log::Write(LOG_DEBUG, String("Could not get String ")+name+String(" from stat file"));
	else return v->GetString();
	return String("");
}

double GetDoubleFromJSONObject(String name, const JSONObject &obj)
{
	const JSONValue *v=obj[name];
	if(!v) Log::Write(LOG_DEBUG, String("Could not get double ")+name+String(" from stat file"));
	else return v->GetDouble();
	return 0.0;
}

bool GetBoolFromJSONObject(String name, const JSONObject &obj)
{
	const JSONValue *v=obj[name];
	if(!v) Log::Write(LOG_DEBUG, String("Could not get bool ")+name+String(" from stat file"));
	else return v->GetBool();
	return false;
}

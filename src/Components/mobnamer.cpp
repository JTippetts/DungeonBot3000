#include "mobnamer.h"
#include "enemynameplate.h"

#include <vector>
#include <Urho3D/IO/Log.h>

int roll(int,int);

void MobNamer::RegisterObject(Context *context)
{
	context->RegisterFactory<MobNamer>();
	URHO3D_ACCESSOR_ATTRIBUTE("Class", GetClass, SetClass, String, "", AM_DEFAULT);
}

MobNamer::MobNamer(Context *context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_NO_EVENT);
}

void MobNamer::DelayedStart()
{
	static std::vector<String> moderators=
	{
		"nsmadsen",
		"Tom Sloper",
		"monalaw",
		"riuthamas",
		"IADaveMark",
		"Hodgman",
		"Kylotan",
		"swiftcoder",
		"frob",
		"Promit",
		"Brother Bob",
		"MJP",
		"hplus0603",
		"JTippetts",
		"SiCrane",
		"All8Up",
		"evolutional",
		"fastcall22",
		"Glass_Knife",
		"grhodes_at_work",
		"Machaira",
		"MadKeithV",
		"Obscure",
		"rip-off",
		"Shannon Barber",
		"Washu",
		"Wavinator",
		"WitchLord"
	};

	static std::vector<String> emeritus=
	{
		"Andrew Russell",
		"ApochPiQ",
		"dbaumgart",
		"Evil Steve",
		"Fruny",
		"Gaiiden",
		"johnhattan",
		"jpetrie",
		"JWalsh",
		"LessBread",
		"Michael Tanczos",
		"Myopic Rhino",
		"Oluseyi",
		"pan narrans",
		"Ravuya",
		"Sneftel",
		"TANSTAAFL",
		"Zahlman"
	};
	auto np=node_->GetComponent<EnemyNameplate>();
	if(np)
	{
		if(class_=="Moderator")
		{
			np->SetName(moderators[roll(0,moderators.size()-1)]);
		}
		else if(class_=="Emeritus")
		{
			np->SetName(emeritus[roll(0,emeritus.size()-1)]);
		}
		else
		{
			Log::Write(LOG_ERROR, "Unknown class.");
		}
	}
}

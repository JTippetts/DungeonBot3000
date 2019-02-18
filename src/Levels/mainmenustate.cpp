#include "mainmenustate.h"
#include "../Components/mainmenu.h"

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Graphics/Octree.h>

MainMenuState::MainMenuState(Context *context) : GameStateBase(context)
{
}

void MainMenuState::Start()
{
	auto cache=context_->GetSubsystem<ResourceCache>();

	scene_=SharedPtr<Scene>(new Scene(context_));
	scene_->CreateComponent<Octree>();
	auto musicsource=scene_->CreateComponent<SoundSource>();
	musicsource->SetSoundType(SOUND_MUSIC);
	auto music=cache->GetResource<Sound>("Music/Gravity Sound - Chase CC BY 4.0_0.ogg");
	if(music)
	{
		music->SetLooped(true);
		musicsource->Play(music);
	}
	auto audio=context_->GetSubsystem<Audio>();
	if(audio)
	{
		audio->SetMasterGain(SOUND_MUSIC, 0.5);
	}

	scene_->CreateComponent<MainMenu>();
}

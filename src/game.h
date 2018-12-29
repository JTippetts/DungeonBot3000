#include <Urho3D/Engine/Application.h>

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

class Game : public Application
{
	URHO3D_OBJECT(Game, Application);

	public:
	explicit Game(Context *context);

	void Setup() override;
	void Start() override;
	void Stop() override;

	private:

	void SetWindowTitleAndIcon();
    void CreateConsoleAndDebugHud();
	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
};


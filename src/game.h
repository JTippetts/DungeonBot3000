#include <Urho3D/Engine/Application.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>

#include <vector>

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
	void HandlePostRenderUpdate(StringHash eventType, VariantMap &eventData);

	void HandleUpdate(StringHash eventType, VariantMap &eventData);

	SharedPtr<Scene> scene_;
	SharedPtr<Node> test_;

	std::vector<Node *> dudes_;

	float x,z;
};


#include <vera/vera.h>

#define RESOURCE_PATH "resource/angry bot/"

int main()
{
	vr::AssetLoader loader;

	// loader.loadScene(RESOURCE_PATH"Models/Player/Player.fbx");
	loader.loadScene(RESOURCE_PATH"Models/BreakDance.fbx");

	return 0;
}
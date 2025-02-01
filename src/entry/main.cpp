#include "../headers/gui.h"
#include "../headers/keyhook.h"

#include <thread>

int __stdcall wWinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	PWSTR arguments,
	int commandShow) {

	gui::CreateHWindow();
	gui::CreateDevice();
	gui::CreateImGui();

	keyhook::RegisterKeyhook();
	keyhook::StartKeyhook();

	while (gui::isRunning) {

		gui::BeginRender();
		gui::Render();
		gui::EndRender();

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	keyhook::RemoveKeyhook();

	gui::DestroyImGui();
	gui::DestroyDevice();
	gui::DestroyHWindow();

	return EXIT_SUCCESS;
}
#include "../headers/gui.h"

#include "../resources/imgui/imgui.h"
#include "../resources/imgui/imgui_impl_dx9.h"
#include "../resources/imgui/imgui_impl_win32.h"


// this function is less reusable so it is not placed in core functionality
void gui::Render() noexcept {
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::Begin(
		"S.T.A.L.K.E.R. 2 HoC - Multihack",
		&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	);

	ImGui::Button("idi nahui pidaras");

	ImGui::End();
}
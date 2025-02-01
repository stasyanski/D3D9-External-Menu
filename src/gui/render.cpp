#include "../headers/gui.h"
#include "../headers/constants.h"

#include "../resources/imgui/imgui.h"
#include "../resources/imgui/imgui_impl_dx9.h"
#include "../resources/imgui/imgui_impl_win32.h"


// this function is less reusable so it is not placed in core functionality
void gui::Render() noexcept {
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ constvar::WIDTH, constvar::HEIGHT });
	ImGui::Begin(
		"S.T.A.L.K.E.R. 2 HoC - Multihack",
		&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	);

	int sliderValueInt = 50;
	bool checkbox = true;
	ImGui::Text("Value: %d", sliderValueInt); 
	ImGui::SameLine();
	if (ImGui::SliderInt("", &sliderValueInt, 0, 100)) {
		
	}
	ImGui::Checkbox("Checkbox", &checkbox);
	ImGui::Text("Press 'INSERT' to hide and bring up the menu.");
	ImGui::Text("");
	ImGui::Text("");
	ImGui::Text("adrian is a big fat bitch");
	ImGui::Text("and he is");

	ImGui::End();
}
#include <windows.h>
#include <psapi.h>
#include <cstdio>  // For sprintf_s


#include "../headers/gui.h"
#include "../headers/constants.h"

#include "../resources/imgui/imgui.h"
#include "../resources/imgui/imgui_impl_dx9.h"
#include "../resources/imgui/imgui_impl_win32.h"

// external to look at link time, pass the L(ong)RESULT to imGui window proc handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window, // the window where event happened
	UINT message, // the event itself as a message e.g. mouse click
	WPARAM wideParameter, // additional parameter of event, e.g. specific key pressed
	LPARAM longParameter // additional parameter of event, e.g. specific coord of mouse press
);

// callback function to process messages sent to window / interaction between OS and window 
long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
) {
	// pass the params to the imgui to update the gui based on end user input
	// returns true if imgui proc handler received msg successfully
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter)) {
		return true;
	}
	// some manual handling required depending on the message, example WM_KEYDOWN or WM_SIZE (within the window)
	switch (message) {
		// wm_size called upon window resizing
		case WM_SIZE: {
			// safety checks to see if device is initialised and available
			// checks if the additional wide param window minimised is not true
			if (gui::device && wideParameter != SIZE_MINIMIZED) {
				// longParameter contains dimensions which are extracted using the lower 16 bits, the width
				gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
				// longParameter contains dimensions which are extracted using the higher 16 bits, the height
				gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
				// this function refreshes the window with the new values / new win size
				gui::ResetDevice();
			}
		} return 0;
		case WM_KEYDOWN: {
			if (wideParameter == VK_INSERT) {
				gui::ToggleVisibility();
			}
			break;
		} return 0;
		// disable ALT to open / switch focus to menu using only the lower 4 bits 11111111 1111000 (within the window)
		case WM_SYSCOMMAND: {
			if ((wideParameter & 0xfff0) == SC_KEYMENU) { // sckeymenu is usually alt in most contexts
				return 0;
			}
		} break;
		// WM_destroy is received when a window is being terminated (within the window)
		case WM_DESTROY: {
			PostQuitMessage(0); // tell the os that the program is exit, exit code 0
		} return 0;
		//on left button click (within the window) 
		case WM_LBUTTONDOWN: {
			gui::position = MAKEPOINTS(longParameter); // set the click coords, extracted by makepoints macro, x,y
		} return 0;
		// on mouse movement (within the window)
		case WM_MOUSEMOVE: {
			if (wideParameter == MK_LBUTTON) { // check if current mouse clicked is lbtn (held down)
				const auto points = MAKEPOINTS(longParameter); // extract the coords of longParameter via makePoints (both x and y)
				auto rect = ::RECT{}; // to hold the windows current pos and size

				GetWindowRect(gui::window, &rect); // gets the window current pos and size and stores it in rect

				// calc how much the mouse has dragged on screen for both x and y
				rect.left += points.x - gui::position.x;
				rect.top += points.y - gui::position.y;

				// safety checks to enable moving overlay / window in a certain region
				if (gui::position.x >= 0 && // within windows width
					gui::position.x <= constvar::WIDTH && // within windows width
					gui::position.y >= 0 && gui::position.y <= 19) { // Y coords must be within  
					SetWindowPos(
						gui::window, // the window to be moved
						HWND_TOPMOST, // make sure window is topmost, wanted behaviour for overlays
						rect.left, // new left edge of window after move
						rect.top, // new top edge
						0, // height/width are NOT changed, so 0 appended
						0, // height/width are NOT changed, so 0 appended
						SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER // the window is shown, but not resized or reordered in Z order of windows
					);
				}
			}
		} return 0;
		}
	return DefWindowProcW(window, message, wideParameter, longParameter); // fallback if none of the above conditions matched
}

// handle window create and die
void gui::CreateHWindow() noexcept {
	windowClass.cbSize = sizeof(WNDCLASSEX); // set the size of the structure
	windowClass.style = CS_CLASSDC; // window style
	windowClass.lpfnWndProc = WindowProcess; // the method / func which handles messages from window
	windowClass.cbClsExtra = 0; // no extra bytes for class
	windowClass.cbWndExtra = 0; // no extra bytes for window
	windowClass.hInstance = GetModuleHandleA(0); //fetches current instance of application
	windowClass.hIcon = (HICON)LoadImage(NULL, constvar::PATH_TO_ICO, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	windowClass.hIconSm = (HICON)LoadImage(NULL, constvar::PATH_TO_ICO, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	windowClass.hCursor = 0; //no custom cursor
	windowClass.hbrBackground = 0; // no custom background color
	windowClass.lpszMenuName = 0; // no menu
	windowClass.lpszClassName = "class001"; //name of window class
	windowClass.hIconSm = 0; // no small icon

	RegisterClassEx(&windowClass); // register the window class

	gui::window = CreateWindowEx( // create the window itself based on the class above
		WS_EX_LAYERED, // extended style WS_EX_LAYERED allows for transparency toggle (SetLayeredWindowAttr)
		"class001", // window class name
		" ", // name of window / title
		WS_POPUP, //ws_popup creates a borderless window
		100,//x pos on screen
		100,//y pos on screen
		constvar::WIDTH,
		constvar::HEIGHT,
		0, // no parent window
		0, // no menu
		windowClass.hInstance, //instance handle
		0 // no additional data passed
	);

	SetLayeredWindowAttributes(gui::window, 0, 255, LWA_ALPHA); // code forces full alpha if using WS_EX_LAYERED 
	ShowWindow(gui::window, SW_SHOWDEFAULT); //show the window
	UpdateWindow(gui::window); //update the window

	// code to keep window always topmost
	SetWindowPos(
		gui::window,
		HWND_TOPMOST, 
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE 
	);
}

void gui::DestroyHWindow() noexcept {
	// method to destroy the windows created earlier
	DestroyWindow(gui::window);
	// unregister the classes from memory / RegisterClassEx
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

// device create and die
bool gui::CreateDevice() noexcept {
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d) {
		return false;
	}
	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		gui::window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept {
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept {
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(gui::window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept {
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept {
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

void gui::ToggleVisibility() noexcept {

	// Get the current extended window style
	LONG style = GetWindowLong(gui::window, GWL_EXSTYLE);

	if (!gui::isVisible) {
		// Make the window fully transparent (use LWA_ALPHA instead of LWA_COLORKEY)
		SetLayeredWindowAttributes(gui::window, 0, 0, LWA_ALPHA); // Alpha set to 0 (fully transparent)
	}
	else {
		// Show the window (set it back to normal)
		SetLayeredWindowAttributes(gui::window, 0, 255, LWA_ALPHA); // Restore visibility with alpha set to 255 (fully opaque)
	}

	// Toggle the visibility flag
	gui::isVisible = !gui::isVisible;
}



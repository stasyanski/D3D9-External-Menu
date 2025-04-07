#include <windows.h> // include windows header for Windows API
#include <psapi.h> // include for process API
#include <cstdio> // include for sprintf_s

#include "../headers/gui.h" // include custom GUI header
#include "../headers/constants.h" // include constants header

#include "../resources/imgui/imgui.h" // include ImGui header
#include "../resources/imgui/imgui_impl_dx9.h" // include ImGui DX9 implementation
#include "../resources/imgui/imgui_impl_win32.h" // include ImGui Win32 implementation

// external declaration for ImGui window procedure handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND window, // the window where the event happened
    UINT message, // the event itself as a message (e.g., mouse click)
    WPARAM wideParameter, // additional parameter of the event (e.g., specific key pressed)
    LPARAM longParameter // additional parameter of the event (e.g., specific coordinate of mouse press)
);

// callback function to process messages sent to the window
long __stdcall WindowProcess(HWND window, UINT message, WPARAM wideParameter, LPARAM longParameter) {
    // pass the parameters to ImGui to update the GUI based on user input
    // returns true if ImGui handler received the message successfully
    if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter)) {
        return true; // if handled by ImGui, exit early
    }

    // manual handling required for specific messages
    switch (message) {
        // wm_size called upon window resizing
    case WM_SIZE: {
        // check if the device is initialized and available
        if (gui::device && wideParameter != SIZE_MINIMIZED) {
            // extract the width and height from longParameter
            gui::presentParameters.BackBufferWidth = LOWORD(longParameter); // width
            gui::presentParameters.BackBufferHeight = HIWORD(longParameter); // height
            // refresh the window with new dimensions
            gui::ResetDevice();
        }
    }
                return 0;

                // handle ALT key to prevent menu from opening
    case WM_SYSCOMMAND: {
        if ((wideParameter & 0xfff0) == SC_KEYMENU) { // SC_KEYMENU is usually the ALT key
            return 0; // prevent the menu from opening
        }
    }
                      break;

                      // WM_DESTROY is received when the window is being terminated
    case WM_DESTROY: {
        PostQuitMessage(0); // tell the OS that the program is exiting
    }
                   return 0;

                   // on left button click
    case WM_LBUTTONDOWN: {
        gui::position = MAKEPOINTS(longParameter); // set the click coordinates
    }
                       return 0;

                       // on mouse movement
    case WM_MOUSEMOVE: {
        if (wideParameter == MK_LBUTTON) { // check if the left button is held down
            const auto points = MAKEPOINTS(longParameter); // extract mouse coordinates
            auto rect = ::RECT{}; // create a rectangle to hold window position
            GetWindowRect(gui::window, &rect); // get the window's current position and size

            // calculate how much the mouse has dragged
            rect.left += points.x - gui::position.x; // update left edge
            rect.top += points.y - gui::position.y; // update top edge

            // check if the window is being moved within a certain region
            if (gui::position.x >= 0 && // within window width
                gui::position.x <= constvar::WIDTH && // within window width
                gui::position.y >= 0 && gui::position.y <= 19) { // Y coords must be within the title bar
                SetWindowPos(
                    gui::window, // the window to be moved
                    HWND_TOPMOST, // keep window on top
                    rect.left, // new left edge of the window
                    rect.top, // new top edge of the window
                    0, // height/width unchanged
                    0, // height/width unchanged
                    SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER // show the window, do not resize or change Z order
                );
            }
        }
    }
                     return 0;
    }

    // fallback if none of the above conditions matched
    return DefWindowProcW(window, message, wideParameter, longParameter);
}

// handle window creation and destruction
void gui::CreateHWindow() noexcept {
    windowClass.cbSize = sizeof(WNDCLASSEX); // set the size of the structure
    windowClass.style = CS_CLASSDC; // window style
    windowClass.lpfnWndProc = WindowProcess; // function to handle messages
    windowClass.cbClsExtra = 0; // no extra bytes for class
    windowClass.cbWndExtra = 0; // no extra bytes for window
    windowClass.hInstance = GetModuleHandleA(0); // get current instance of application
    windowClass.hCursor = 0; // no custom cursor
    windowClass.hbrBackground = 0; // no custom background color
    windowClass.lpszMenuName = 0; // no menu
    windowClass.lpszClassName = "class001"; // name of window class
    windowClass.hIconSm = 0; // no small icon

    RegisterClassEx(&windowClass); // register the window class

    // create the window itself based on the defined class
    gui::window = CreateWindowEx(
        WS_EX_LAYERED, // extended style for transparency
        "class001", // window class name
        " ", // name of window/title
        WS_POPUP, // creates a borderless window
        100, // x pos on screen
        100, // y pos on screen
        constvar::WIDTH, // width of the window
        constvar::HEIGHT, // height of the window
        0, // no parent window
        0, // no menu
        windowClass.hInstance, // instance handle
        0 // no additional data passed
    );

    // set full alpha for transparency
    SetLayeredWindowAttributes(gui::window, 0, 255, LWA_ALPHA);
    ShowWindow(gui::window, SW_SHOWDEFAULT); // show the window
    UpdateWindow(gui::window); // update the window

    // keep the window always topmost
    SetWindowPos(
        gui::window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    );
}

// method to destroy the window
void gui::DestroyHWindow() noexcept {
    DestroyWindow(gui::window); // destroy the window
    UnregisterClass(windowClass.lpszClassName, windowClass.hInstance); // unregister the window class
}

// device creation and destruction
bool gui::CreateDevice() noexcept {
    d3d = Direct3DCreate9(D3D_SDK_VERSION); // create Direct3D object
    if (!d3d) {
        return false; // return false if creation failed
    }
    ZeroMemory(&presentParameters, sizeof(presentParameters)); // zero out presentParameters

    presentParameters.Windowed = TRUE; // run windowed
    presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD; // discard old frames
    presentParameters.BackBufferFormat = D3DFMT_UNKNOWN; // format unknown
    presentParameters.EnableAutoDepthStencil = TRUE; // enable depth stencil
    presentParameters.AutoDepthStencilFormat = D3DFMT_D16; // depth format
    presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // presentation interval

    // create the Direct3D device
    if (d3d->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        gui::window,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &presentParameters,
        &device) < 0) {
        return false; // return false if device creation failed
    }

    return true; // return true on success
}

// reset the Direct3D device
void gui::ResetDevice() noexcept {
    ImGui_ImplDX9_InvalidateDeviceObjects(); // invalidate device objects

    const auto result = device->Reset(&presentParameters); // reset the device

    if (result == D3DERR_INVALIDCALL) IM_ASSERT(0); // check for invalid call

    ImGui_ImplDX9_CreateDeviceObjects(); // create device objects
}

// destroy the Direct3D device
void gui::DestroyDevice() noexcept {
    if (device) {
        device->Release(); // release device
        device = nullptr; // set to nullptr
    }

    if (d3d) {
        d3d->Release(); // release Direct3D
        d3d = nullptr; // set to nullptr
    }
}

// create ImGui context and initialize
void gui::CreateImGui() noexcept {
    IMGUI_CHECKVERSION(); // check ImGui version
    ImGui::CreateContext(); // create ImGui context
    ImGuiIO& io = ::ImGui::GetIO(); // get ImGui IO object

    io.IniFilename = NULL; // disable INI file saving

    ImGui::StyleColorsDark(); // set dark style

    ImGui_ImplWin32_Init(gui::window); // initialize ImGui for Win32
    ImGui_ImplDX9_Init(device); // initialize ImGui for DirectX9
}

// destroy ImGui context and shutdown
void gui::DestroyImGui() noexcept {
    ImGui_ImplDX9_Shutdown(); // shutdown ImGui for DirectX9
    ImGui_ImplWin32_Shutdown(); // shutdown ImGui for Win32
    ImGui::DestroyContext(); // destroy ImGui context
}

// begin rendering process
void gui::BeginRender() noexcept {
    MSG message; // message structure
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) { // process messages
        TranslateMessage(&message); // translate message
        DispatchMessage(&message); // dispatch message

        if (message.message == WM_QUIT) { // check if quit message received
            isRunning = !isRunning; // toggle running state
            return; // exit the loop
        }
    }

    // start a new ImGui frame
    ImGui_ImplDX9_NewFrame(); // start new frame for DirectX9
    ImGui_ImplWin32_NewFrame(); // start new frame for Win32
    ImGui::NewFrame(); // create a new frame
}

// end rendering process
void gui::EndRender() noexcept {
    ImGui::EndFrame(); // end the ImGui frame

    // set render states
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

    // clear the screen
    device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

    // begin the scene
    if (device->BeginScene() >= 0) {
        ImGui::Render(); // render ImGui
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData()); // draw ImGui data
        device->EndScene(); // end the scene
    }

    const auto result = device->Present(0, 0, 0, 0); // present the device

    // handle loss of D3D9 device
    if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
        ResetDevice(); // reset device if lost
    }
}

// toggle the visibility of the GUI window
void gui::ToggleVisibility() noexcept {
    LONG style = GetWindowLong(gui::window, GWL_EXSTYLE); // get the window style
    if (!gui::isVisible) { // if the window is not visible
        // make the window fully transparent
        SetLayeredWindowAttributes(gui::window, 0, 0, LWA_ALPHA); // set alpha to 0 (fully transparent)
    }
    else {
        SetLayeredWindowAttributes(gui::window, 0, 255, LWA_ALPHA); // set alpha to 255 (fully opaque)
    }
    gui::isVisible = !gui::isVisible; // toggle visibility state
}



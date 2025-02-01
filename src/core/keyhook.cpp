#include <windows.h>
#include <thread>

#include "../headers/keyhook.h"
#include "../headers/gui.h"

namespace keyhook {
    // callback function to process keybaord events, long result return, nCode, wideParam and lParam passed by windows api to keyhook proc
    LRESULT CALLBACK KeyhookProc(int nCode, WPARAM wParam, LPARAM lParam) noexcept {
        // ncode hc action checks if the action is valid / keybaord event, and wParam checks that its a keydown event. this just narrows down and improves efficiency technically not needed
        if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
            // this code sts the lParam (pointer to KDBLLHOOKSTRUCT) to a more useful and readable pointer, reveal info about keypress event
            KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
            // check if the keypress is VK_INSERT (insert key) this will toggle the gui visibility
            if (pKeyboard->vkCode == VK_INSERT) {
                gui::ToggleVisibility();
            }
        }
        // calls the next hook in chain
        return CallNextHookEx(keyhook::hook, nCode, wParam, lParam);
    }

    // sets up the keyboard hook registering
    void RegisterKeyhook() noexcept {
        if (!keyhook::hook) {
            //low level keyboard hook allows global monitoring. if imgui not in focus, it will still listen
            // thread 0 is passed meaning global / all threads
            keyhook::hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyhookProc, NULL, 0);
        }
    }

    // unregisters keyhook
    void RemoveKeyhook() noexcept {
        if (keyhook::hook) {
            UnhookWindowsHookEx(keyhook::hook);
            keyhook::hook = NULL;
        }
    }

    // start keyhook and register it
    void StartKeyhook() noexcept {
        // a new thread to run the code inside separate from other
        std::thread([]() {
            MSG msg;
            // if messages are available translate them
            while (GetMessage(&msg, NULL, 0, 0)) {
                TranslateMessage(&msg); // processes message
                DispatchMessage(&msg);
            }
            }).detach(); // a dtached threat running independently from main thread
        // deatch cleans itself once done
    }
}
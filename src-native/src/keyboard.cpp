#include "keyboard.h"
#include <windows.h>
#include <iostream>

#define VK_KEY_Y 0x59
#define VK_KEY_W 0x57
#define VK_ESCAPE 0x1B
#define SCANCODE_W 0x11
#define SCANCODE_SHIFT_LEFT 0x2A

volatile bool isWKeyDown = false;

struct KeyboardData {
    HHOOK keyboardHook;
    HANDLE thread;
    DWORD threadId;

    virtual ~KeyboardData();
};

void SendKey(DWORD scanCode, bool keyDown) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    // input.ki.wVk = keyCode;
    input.ki.wScan = scanCode;
    input.ki.dwFlags = keyDown ? KEYEVENTF_SCANCODE : (KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP);
    SendInput(1, &input, sizeof(INPUT));
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        PKBDLLHOOKSTRUCT pkbhs = (PKBDLLHOOKSTRUCT)lParam;

        // Check for 'Y' key press
        if (pkbhs->vkCode == VK_KEY_Y) {
            if (!isWKeyDown) {
                SendKey(SCANCODE_W, true);
                isWKeyDown = true;
            } else {
                SendKey(SCANCODE_W, false);
                isWKeyDown = false;
            }
            return 1; // Block the 'Y' key from propagating
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

DWORD WINAPI ThreadProc(LPVOID lpParam) {
    Keyboard* keyboard = (Keyboard*)lpParam;
    keyboard->loop();
    return 0; // Thread exit code
}

Keyboard::Keyboard(KeyboardData* d): data(d) {}

Keyboard::~Keyboard() {
    is_running = false;
    SendKey(SCANCODE_W, true);
    delete data;
}

KeyboardData::~KeyboardData() {
    // Wait for thread is finished
    if (thread != nullptr) {
        // Forceful termination (use as last resort)
        if (!TerminateThread(thread, 1)) {
            std::cout << "Failed to terminate thread. Error: " << GetLastError() << std::endl;
        }

        // Wait for the thread to complete
        WaitForSingleObject(thread, INFINITE);

        // Get the thread's exit code for report!
        // DWORD exitCode;
        // GetExitCodeThread(thread, &exitCode);

        // Clean up the thread handle
        CloseHandle(thread);
    }

    // Release 'W' if still down
    if (isWKeyDown) {
        SendKey(SCANCODE_W, false);
    }

    // Clean up the hook
    UnhookWindowsHookEx(keyboardHook);
}

DWORD WINAPI loop(LPVOID lpParam) {
    Keyboard* keyboard = (Keyboard*)lpParam;
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && keyboard->is_running) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

Keyboard* Keyboard::start() {
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (!keyboardHook) {
        std::cout << "[Keyboard::start] Failed to install keyboard hook! Error: " << GetLastError() << std::endl;
        return nullptr;
    }

    KeyboardData *data = new KeyboardData();
    Keyboard *keyboard = new Keyboard(data);
    keyboard->is_running = true;
    data->keyboardHook = keyboardHook;
    data->thread = CreateThread(
        NULL,              // Default security attributes
        0,                 // Default stack size
        ThreadProc,        // Thread function name
        keyboard,          // Parameter to pass to thread
        0,                 // Creation flags (0 = run immediately)
        &data->threadId    // Pointer to receive thread ID
    );

    if (data->thread == nullptr) {
        std::cout << "[Keyboard::start] Failed to create thread. Error: " << GetLastError() << std::endl;
        delete data;
        return nullptr;
    }

    return keyboard;
}

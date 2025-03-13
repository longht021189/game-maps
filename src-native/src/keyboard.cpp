#include "keyboard.h"
#include <windows.h>
#include <iostream>

#define VK_KEY_Y 0x59
#define SCANCODE_W 0x11
#define SCANCODE_SHIFT_LEFT 0x2A

volatile bool isWKeyDown = false;

struct KeyboardData {
    HHOOK keyboardHook = nullptr;
    HANDLE hThread = nullptr;
    DWORD threadId;
    virtual ~KeyboardData();
};

void SendKey(DWORD scanCode, bool keyDown) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
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
                std::cout << "'Y' pressed: Sending 'W' key down" << std::endl;
                SendKey(SCANCODE_W, true);
                SendKey(SCANCODE_SHIFT_LEFT, true);
                isWKeyDown = true;
            } else {
                std::cout << "'Y' pressed again: Sending 'W' key up" << std::endl;
                SendKey(SCANCODE_W, false);
                SendKey(SCANCODE_SHIFT_LEFT, false);
                isWKeyDown = false;
            }
            return 1; // Block the 'Y' key from propagating
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

Keyboard::Keyboard(KeyboardData* d): data(d) {}

Keyboard::~Keyboard() {
    is_running = false;
    SendKey(SCANCODE_W, true);
    delete data;
}

KeyboardData::~KeyboardData() {
    if (TerminateThread(hThread, 1)) {
        std::cout << "Thread forcibly terminated!" << std::endl;
    } else {
        std::cout << "Failed to terminate thread. Error: " << GetLastError() << std::endl;
    }

    // Wait for the thread to complete
    WaitForSingleObject(hThread, INFINITE);

    // Get the thread's exit code
    DWORD exitCode;
    GetExitCodeThread(hThread, &exitCode);
    std::cout << "Main thread: Thread terminated with exit code " << exitCode << std::endl;

    // Clean up the thread handle
    CloseHandle(hThread);

    UnhookWindowsHookEx(keyboardHook);
}

DWORD WINAPI loop(LPVOID lpParam) {
    std::cout << "[Keyboard::loop] Start message loop" << std::endl;

    Keyboard* keyboard = (Keyboard*)lpParam;
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && keyboard->is_running) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if (isWKeyDown) {
        SendKey(SCANCODE_W, false); // Release 'W' if still down
        SendKey(SCANCODE_SHIFT_LEFT, false);
        isWKeyDown = false;
    }

    std::cout << "[Keyboard::loop] End message loop" << std::endl;

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
    data->hThread = CreateThread(
        NULL,              // Default security attributes
        0,                 // Default stack size
        loop,              // Thread function name
        keyboard,          // Parameter to pass to thread
        0,                 // Creation flags (0 = run immediately)
        &data->threadId    // Pointer to receive thread ID
    );

    if (data->hThread == nullptr) {
        delete keyboard;
        std::cout << "Failed to create thread. Error: " << GetLastError() << std::endl;
        return nullptr;
    }

    std::cout << "Main thread: Created thread with ID " << data->threadId << std::endl;


    return keyboard;
}

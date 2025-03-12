#include "keyboard.h"
#include <windows.h>
#include <thread>
#include <iostream>

#define VK_KEY_Y 0x59
#define VK_KEY_W 0x57
#define VK_ESCAPE 0x1B

bool isWKeyDown = false; // Tracks whether 'W' is currently pressed

struct KeyboardData {
    HHOOK keyboardHook = nullptr;
    std::thread *thread = nullptr;
    virtual ~KeyboardData();
};

void SendKey(WORD keyCode, bool keyDown) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = keyDown ? 0 : KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        PKBDLLHOOKSTRUCT pkbhs = (PKBDLLHOOKSTRUCT)lParam;

        // Check for 'Y' key press
        if (pkbhs->vkCode == VK_KEY_Y) {
            if (!isWKeyDown) {
                std::cout << "'Y' pressed: Sending 'W' key down" << std::endl;
                SendKey(VK_KEY_W, true);  // Send 'W' down
                isWKeyDown = true;
            } else {
                std::cout << "'Y' pressed again: Sending 'W' key up" << std::endl;
                SendKey(VK_KEY_W, false); // Send 'W' up
                isWKeyDown = false;
            }
            return 1; // Block the 'Y' key from propagating
        }
        // Exit on Esc
        // else if (pkbhs->vkCode == VK_ESCAPE) {
        //     std::cout << "Esc pressed. Exiting...\n";
        //     if (isWKeyDown) {
        //         SendKey(VK_KEY_W, false); // Release 'W' if still down
        //     }
        //     PostQuitMessage(0);
        //     return 1;
        // }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

Keyboard::Keyboard(KeyboardData* d): data(d) {}

Keyboard::~Keyboard() {
    is_running = false;
    delete data;
}

KeyboardData::~KeyboardData() {
    // TODO Check stop thread
    // if (thread->joinable()) {
    //     thread->join();
    // }
    // delete thread;

    UnhookWindowsHookEx(keyboardHook);
}

void Keyboard::loop() {
    std::cout << "[Keyboard::loop] Start message loop" << std::endl;
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && is_running) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if (isWKeyDown) {
        SendKey(VK_KEY_W, false); // Release 'W' if still down
        isWKeyDown = false;
    }

    std::cout << "[Keyboard::loop] End message loop" << std::endl;
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
    data->thread = new std::thread(&Keyboard::loop, keyboard);

    return keyboard;
}
#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include "mapgenie-data.h"

#define VK_KEY_Y 0x59
#define VK_KEY_W 0x57
#define VK_ESCAPE 0x1B

HHOOK keyboardHook = nullptr;
bool isWKeyDown = false; // Tracks whether 'W' is currently pressed
std::thread* core_main_thread = nullptr;
bool core_main_running = false;
Data *database;
MapgenieData *mapgenie_data = nullptr;

// Function to send 'W' key down or up
void SendKey(WORD keyCode, bool keyDown) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = keyDown ? 0 : KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

// Low-level keyboard hook callback
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        PKBDLLHOOKSTRUCT pkbhs = (PKBDLLHOOKSTRUCT)lParam;

        // Check for 'Y' key press
        if (pkbhs->vkCode == VK_KEY_Y) {
            if (!isWKeyDown) {
                std::cout << "'Y' pressed: Sending 'W' key down\n";
                SendKey(VK_KEY_W, true);  // Send 'W' down
                isWKeyDown = true;
            } else {
                std::cout << "'Y' pressed again: Sending 'W' key up\n";
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

extern "C" void core_start() {
    if (database == nullptr) {
        database = new Data();
        
        if (!database->init()) {
            delete database;
            database = nullptr;
            return;
        }
    }
    if (core_main_thread != nullptr) {
        return;
    }

    // Install the keyboard hook
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    
    if (!keyboardHook) {
        std::cout << "Failed to install keyboard hook! Error: " << GetLastError() << "\n";
        return;
    }

    std::cout << "App started. Press 'Y' to toggle 'W' key.\n";

    core_main_running = true;

    auto loop = []() {
        // Message loop
        MSG msg;
        while (core_main_running && GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (isWKeyDown) {
            SendKey(VK_KEY_W, false); // Release 'W' if still down
            isWKeyDown = false;
        }
    };

    mapgenie_data = new MapgenieData(database);
    core_main_thread = new std::thread(loop);
}

extern "C" void core_end() {
    if (core_main_thread == nullptr) {
        return;
    }

    core_main_running = false;

    if (core_main_thread->joinable()) {
        core_main_thread->join();
    }

    delete mapgenie_data;

    mapgenie_data = nullptr;

    delete database;

    database = nullptr;

    // Cleanup
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
    }

    std::cout << "App terminated.\n";
}

extern "C" void mapgenie_add_location(uint64_t game_id, uint64_t map_id, uint64_t location_id) {
    if (mapgenie_data == nullptr) {
        return;
    }

    mapgenie_data->add_location(game_id, map_id, location_id);
}

extern "C" void mapgenie_remove_location(uint64_t game_id, uint64_t map_id, uint64_t location_id) {
    if (mapgenie_data == nullptr) {
        return;
    }

    mapgenie_data->remove_location(game_id, map_id, location_id);
}

extern "C" char* mapgenie_get_map_data(uint64_t game_id, uint64_t map_id, size_t* length) {
    if (mapgenie_data == nullptr) {
        return nullptr;
    }

    std::string output;
    mapgenie_data->get_map_data(game_id, map_id, output);
    char* buffer = (char*)malloc(output.size() + 1);
    if (buffer) {
        *length = output.size();
        std::memcpy(buffer, output.c_str(), output.size() + 1);
    }
    return buffer;
}

extern "C" void free_buffer(char* ptr) {
    free(ptr);
}
#ifndef GAME_MAPS_CORE_KEYBOARD_H
#define GAME_MAPS_CORE_KEYBOARD_H

struct KeyboardData;

struct Keyboard {
    static Keyboard* start();
    virtual ~Keyboard();
    volatile bool is_running = false;
private:
    Keyboard(KeyboardData* d);
    KeyboardData* data;
};

#endif // GAME_MAPS_CORE_KEYBOARD_H
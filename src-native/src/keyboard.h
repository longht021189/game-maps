#ifndef GAME_MAPS_CORE_KEYBOARD_H
#define GAME_MAPS_CORE_KEYBOARD_H

struct KeyboardData;

struct Keyboard {
    static Keyboard* start();
    virtual ~Keyboard();
private:
    Keyboard(KeyboardData* d);
    void loop();
    KeyboardData* data;
    bool is_running = false;
};

#endif // GAME_MAPS_CORE_KEYBOARD_H
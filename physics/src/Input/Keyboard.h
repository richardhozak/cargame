#pragma once

#include <set>

enum DIK
{
    DIK_UP = 0,
    DIK_DOWN,
    DIK_LEFT,
    DIK_RIGHT,
    DIK_Z,
};

struct Keyboard
{
    Keyboard(std::set<DIK>& pressed)
      : pressed(pressed)
    {}

    bool IsKeyPressed(DIK key)
    {
        return pressed.find(key) != pressed.end();
    }

private:
    std::set<DIK>& pressed;
};
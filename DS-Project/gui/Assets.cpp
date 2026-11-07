#include "gui/Assets.h"
#include <iostream>

bool Assets::load()
{
    if (!m_font.loadFromFile("assets/fonts/Liber-Struct.ttf")) {
        std::cerr << "Could not load assets/fonts/Liber-Struct.ttf\n";
        return false;
    }
    return true;
}
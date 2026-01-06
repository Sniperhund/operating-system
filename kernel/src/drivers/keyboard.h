#pragma once

#include "x86/idt.h"

class Keyboard {
public:
    static void init(bool print);

private:
    static void keyboardHandler(CPUStatus* status);
};
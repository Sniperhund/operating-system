#include "keyboard.h"
#include "sched/scheduler.h"
#include "x86/io.h"
#include "x86/irq.h"
#include <stdio.h>

#define KBD_BUFFER_SIZE 256

char Keyboard::buffer[KBD_BUFFER_SIZE] = {0};
size_t Keyboard::head = 0;
size_t Keyboard::tail = 0;

bool Keyboard::print = false;
uint8_t Keyboard::modKey = 0;

WaitQueue* Keyboard::s_stdinQueue = nullptr;

int Keyboard::init(bool _print) {
    print = _print;

    IRQ::registerIRQ(1, Keyboard::keyboardHandler);

	return 0;
}

bool Keyboard::bufferEmpty() {
  	return head == tail;
}

bool Keyboard::bufferFull() {
  	return ((tail + 1) % KBD_BUFFER_SIZE) == head;
}

char Keyboard::bufferPop() {
	if (bufferEmpty()) return 0;

	char c = buffer[head];
	head = (head + 1) % KBD_BUFFER_SIZE;
	return c;
}

void Keyboard::bufferPush(char c) {
	size_t next = (tail + 1) % KBD_BUFFER_SIZE;

	if (next == head) {
		head = (head + 1) % KBD_BUFFER_SIZE;
	}

	buffer[tail] = c;
	tail = next;
}

#define MOD_NONE  0
#define MOD_CTRL  (1 << 0)
#define MOD_SHIFT (1 << 1)
#define MOD_ALT   (1 << 2)

void Keyboard::keyboardHandler(CPUStatus* status) {
    uint8_t scancode = inb(0x60);

    switch (scancode) {
        case 0x2a: // Left shift pressed
        case 0x36: // Right shift pressed
            modKey |= MOD_SHIFT;
            break;
        case 0xaa: // Left shift released
        case 0xb6: // Right shift released
            modKey &= ~MOD_SHIFT;
            break;
        case 0x1d: // Left ctrl pressed
            modKey |= MOD_CTRL;
            break;
        case 0x9d: // Left ctrl released
            modKey &= ~MOD_CTRL;
            break;
        case 0x38: // Left alt pressed
            modKey |= MOD_ALT;    
            break;
        case 0xb8:
            modKey &= ~MOD_ALT;
            break;
    }

    char c = 0;

    if (scancode & 0x80) return;

    if (modKey == MOD_NONE) {
        c = kbdmix[scancode];
    } else if (modKey == MOD_SHIFT) {
        c = kbdseShift[scancode];
    }

	if (c) {
		bufferPush(c);
    	if (print) printf("%c", c);

        if (s_stdinQueue) Scheduler::wakeOne(s_stdinQueue);
	}
}